/* Direct H1 PCM services: streamed original music plus two finite effect
 * channels. No Win32, PC sound code, floating point, or audio register writes. */
#include "h1_sdk.h"
#include "duel_audio.h"
#include "duel_ima.h"
#include <stddef.h>

#define AUDIO_RATE 32000u
#define RING_FRAMES 32768u
#define TARGET_FRAMES 16384u
/* Fill half a second before submitting a new scene's music. The first deck
 * canvas/card/font draw can exceed the old quarter-second prefill on H1. */
#define PREFILL_FRAMES TARGET_FRAMES
#define BATCH_FRAMES TARGET_FRAMES
#define EFFECT_SLOTS 2u
#define GAME_VOLUME_PERCENT 10u
typedef struct {int16_t *pcm;uint32_t bytes,private_words[6];} PcmDescriptor;
typedef struct {uint32_t offset,bytes,frames,rate,loop,gain,family,original_id;} AudioEntry;
typedef int (*DescriptorFn)(PcmDescriptor *);
typedef int (*DeviceFn)(void);
typedef int (*OpenFn)(uint32_t *);
typedef int (*SubmitFn)(int,PcmDescriptor *,int,int);
_Static_assert(sizeof(PcmDescriptor)==32,"Full H1 PCM descriptor required");
_Static_assert(sizeof(AudioEntry)==32,"Audio pack index layout changed");
volatile uint32_t joey_audio_diagnostic[32];
volatile uint32_t joey_audio_volume_diagnostic[2];
static AudioEntry entries[JOEY_AUDIO_ASSET_COUNT];
static PcmDescriptor music_descriptor __attribute__((aligned(32)));
static PcmDescriptor effect_descriptors[EFFECT_SLOTS] __attribute__((aligned(32)));
static int16_t music_ring[RING_FRAMES] __attribute__((aligned(32)));
static int16_t decoded_music[JOEY_IMA_BLOCK_FRAMES];
static uint8_t encoded[JOEY_IMA_BLOCK_BYTES];
static void *effect_allocations[EFFECT_SLOTS];
static unsigned effect_assets[EFFECT_SLOTS],effect_live[EFFECT_SLOTS],effect_stamp[EFFECT_SLOTS],stamp;
static h1_file *audio_file;
static DescriptorFn initialize,destroy;
static DeviceFn start,stop,close_device;
static SubmitFn submit;
static volatile uint32_t *queue;
static unsigned device_owned,music_live,music_wanted=JOEY_AUDIO_NONE,music_playing;
static unsigned queued,write_position,read_position,decoded_position,decoded_count,block_index,music_frame,ended;
static unsigned last_tick,retire_tick,progress_tick,progress_position;

static void *service(unsigned offset){return h1_runtime_entry(h1_runtime_table(H1_RUNTIME_SYS_TABLE_SLOT),offset);}
static void update_diagnostic(void){
    joey_audio_volume_diagnostic[0]=GAME_VOLUME_PERCENT;joey_audio_volume_diagnostic[1]=joey_audio_diagnostic[19];
    joey_audio_diagnostic[0]=0x4a415544u;joey_audio_diagnostic[1]=device_owned;joey_audio_diagnostic[5]=music_wanted;joey_audio_diagnostic[6]=music_frame;
    joey_audio_diagnostic[11]=queued;joey_audio_diagnostic[20]=music_playing;joey_audio_diagnostic[21]=effect_live[0]+effect_live[1];
    joey_audio_diagnostic[22]=effect_assets[0];joey_audio_diagnostic[23]=effect_assets[1];joey_audio_diagnostic[25]=AUDIO_RATE;joey_audio_diagnostic[27]=music_live;joey_audio_diagnostic[30]=last_tick;joey_audio_diagnostic[31]=JOEY_AUDIO_MAX_FX_FRAMES;
}
static uint32_t checksum(const void *data,unsigned bytes){const uint8_t *p=data;uint32_t crc=~0u;while(bytes--){crc^=*p++;for(unsigned i=0;i<8;++i)crc=crc>>1^((0u-(crc&1u))&0xedb88320u);}return ~crc;}
static volatile uint32_t *resolve_queue(SubmitFn fn){
    const uint32_t *code=(const uint32_t *)fn;
    if(!code||code[1]!=0x28820008u||(code[0x70/4]&0xffff0000u)!=0x3c060000u||(code[0x74/4]&0xffff0000u)!=0x24c60000u||code[0x78/4]!=0x00c43021u)return 0;
    uint32_t address=((code[0x70/4]&65535u)<<16)+(int16_t)code[0x74/4];if((address&3u)||address<0x80000000u||address>=0x84000000u)return 0;return (volatile uint32_t *)address;
}
static int cancel_music(void){
    if(device_owned&&!submit(0,0,0,0)){++joey_audio_diagnostic[28];joey_audio_diagnostic[2]=14;joey_audio_close();return 0;}
    music_playing=queued=write_position=read_position=0;h1_memset(music_ring,0,sizeof(music_ring));
    return 1;
}
static int load_pack(void){
    audio_file=h1_fopen("A:\\JOEY\\H1SND.PAK","rb");joey_audio_diagnostic[24]=0;
    if(!audio_file){audio_file=h1_fopen("B:\\JOEY\\H1SND.PAK","rb");joey_audio_diagnostic[24]=1;}
    if(!audio_file)return 0;uint32_t header[8];
    h1_fseek(audio_file,0,H1_SEEK_END);unsigned size=h1_ftell(audio_file);h1_fseek(audio_file,0,H1_SEEK_SET);
    if(size!=JOEY_AUDIO_PACK_BYTES||h1_fread(header,4,8,audio_file)!=8||header[0]!=0x59454f4au||header[1]!=0x31444e53u||header[2]!=1||header[3]!=JOEY_AUDIO_ASSET_COUNT||header[4]!=AUDIO_RATE||header[5]!=JOEY_IMA_BLOCK_BYTES||header[6]!=32)return 0;
    if(h1_fread(entries,sizeof(AudioEntry),JOEY_AUDIO_ASSET_COUNT,audio_file)!=JOEY_AUDIO_ASSET_COUNT||checksum(entries,sizeof(entries))!=JOEY_AUDIO_INDEX_CRC)return 0;
    for(unsigned i=0;i<JOEY_AUDIO_ASSET_COUNT;++i){const AudioEntry *e=&entries[i];if(e->offset<header[7]||e->offset>size||e->bytes>size-e->offset||e->offset%JOEY_IMA_BLOCK_BYTES||e->bytes%JOEY_IMA_BLOCK_BYTES||!e->frames||e->frames>e->bytes/JOEY_IMA_BLOCK_BYTES*JOEY_IMA_BLOCK_FRAMES||e->bytes/JOEY_IMA_BLOCK_BYTES*JOEY_IMA_BLOCK_FRAMES-e->frames>=JOEY_IMA_BLOCK_FRAMES||e->rate!=AUDIO_RATE||e->loop>1||e->gain>128||e->family!=(i>=JOEY_AUDIO_MUSIC_COUNT))return 0;}
    return 1;
}
void joey_audio_close(void){
    if(device_owned){stop();submit(0,0,0,0);for(unsigned i=0;i<EFFECT_SLOTS;++i)submit(i+1,0,0,0);close_device();device_owned=0;}
    if(music_live){destroy(&music_descriptor);music_live=0;++joey_audio_diagnostic[4];}
    for(unsigned i=0;i<EFFECT_SLOTS;++i){if(effect_live[i]){destroy(&effect_descriptors[i]);effect_live[i]=0;++joey_audio_diagnostic[4];}if(effect_allocations[i]){h1_free(effect_allocations[i]);effect_allocations[i]=0;}effect_assets[i]=JOEY_AUDIO_NONE;}
    if(audio_file){h1_fclose(audio_file);audio_file=0;}
    music_playing=queued=0;music_wanted=JOEY_AUDIO_NONE;queue=0;++joey_audio_diagnostic[26];update_diagnostic();
}
int joey_audio_open(void){
    if(device_owned||music_live||audio_file)joey_audio_close();
    h1_memset((void *)joey_audio_diagnostic,0,sizeof(joey_audio_diagnostic));music_wanted=JOEY_AUDIO_NONE;for(unsigned i=0;i<EFFECT_SLOTS;++i)effect_assets[i]=JOEY_AUDIO_NONE;
    if(!load_pack()){joey_audio_diagnostic[2]=1;joey_audio_close();return 0;}
    initialize=(DescriptorFn)service(0x50);destroy=(DescriptorFn)service(0x54);OpenFn open_device=(OpenFn)service(0x58);submit=(SubmitFn)service(0x5c);start=(DeviceFn)service(0x60);stop=(DeviceFn)service(0x64);close_device=(DeviceFn)service(0x68);queue=resolve_queue(submit);
    if(!initialize||!destroy||!open_device||!start||!stop||!close_device||!queue){joey_audio_diagnostic[2]=2;joey_audio_close();return 0;}
    const uint32_t *volume=service(0x44);joey_audio_diagnostic[19]=255;
    if(volume&&(volume[0]&0xffff0000u)==0x3c020000u&&volume[1]==0x03e00008u&&(volume[2]&0xffff0000u)==0x8c420000u)joey_audio_diagnostic[19]=((DeviceFn)volume)();
    h1_memset(&music_descriptor,0,sizeof(music_descriptor));h1_memset(music_ring,0,sizeof(music_ring));music_descriptor.pcm=music_ring;music_descriptor.bytes=sizeof(music_ring);
    if(!initialize(&music_descriptor)){joey_audio_diagnostic[2]=3;joey_audio_close();return 0;}music_live=1;++joey_audio_diagnostic[3];
    uint32_t config[9]={AUDIO_RATE,1,4096,0,0,0,0,0,0};if(!open_device(config)){joey_audio_diagnostic[2]=4;joey_audio_close();return 0;}device_owned=1;
    if(!start()){joey_audio_diagnostic[2]=5;joey_audio_close();return 0;}
    last_tick=h1_raw_tick_80hz()-1;retire_tick=progress_tick=last_tick;progress_position=0;update_diagnostic();return 1;
}
void joey_audio_music(unsigned asset){
    if(!device_owned||asset==music_wanted)return;
    if(!cancel_music())return;music_wanted=asset<JOEY_AUDIO_MUSIC_COUNT?asset:JOEY_AUDIO_NONE;block_index=music_frame=decoded_position=decoded_count=ended=0;last_tick=h1_raw_tick_80hz()-1;update_diagnostic();
}
static void reset_music_ring(unsigned tick){cancel_music();retire_tick=progress_tick=tick;progress_position=0;++joey_audio_diagnostic[10];}
static int retire_music(unsigned tick){
    if(!music_playing)return 1;
    volatile uint32_t *node=(volatile uint32_t *)queue[0];uint32_t address=(uint32_t)node,base=(uint32_t)music_ring;
    if(!node||(address&3)||address<0x80000000u||address>=0x84000000u||node[1]!=(uint32_t)&music_descriptor){joey_audio_diagnostic[2]=6;joey_audio_close();return 0;}
    uint32_t cursor=node[2];if(cursor<base||cursor>base+sizeof(music_ring)||(cursor&1)){joey_audio_diagnostic[2]=7;joey_audio_close();return 0;}
    unsigned position=((cursor-base)/2)%RING_FRAMES,consumed=(position+RING_FRAMES-read_position)%RING_FRAMES;
    if((unsigned)(tick-retire_tick)>=RING_FRAMES*80/AUDIO_RATE){++joey_audio_diagnostic[9];reset_music_ring(tick);return 1;}
    retire_tick=tick;
    if(consumed>queued){if(ended){joey_audio_diagnostic[8]+=queued;cancel_music();++joey_audio_diagnostic[16];}else{++joey_audio_diagnostic[9];reset_music_ring(tick);}return 1;}
    for(unsigned i=0;i<consumed;++i)music_ring[(read_position+i)%RING_FRAMES]=0;
    read_position=position;queued-=consumed;joey_audio_diagnostic[8]+=consumed;
    if(position!=progress_position){progress_position=position;progress_tick=tick;}
    else if((unsigned)(tick-progress_tick)>80){joey_audio_diagnostic[2]=8;joey_audio_close();return 0;}
    return 1;
}
static int next_music(int16_t *sample){
    const AudioEntry *e=&entries[music_wanted];
    if(music_frame>=e->frames){if(!e->loop){ended=1;return 0;}music_frame=block_index=decoded_position=decoded_count=0;++joey_audio_diagnostic[12];}
    if(decoded_position>=decoded_count){
        h1_fseek(audio_file,e->offset+block_index*JOEY_IMA_BLOCK_BYTES,H1_SEEK_SET);
        if(h1_fread(encoded,1,JOEY_IMA_BLOCK_BYTES,audio_file)!=JOEY_IMA_BLOCK_BYTES||(decoded_count=joey_ima_decode_block(encoded,JOEY_IMA_BLOCK_BYTES,decoded_music,JOEY_IMA_BLOCK_FRAMES))!=JOEY_IMA_BLOCK_FRAMES){++joey_audio_diagnostic[18];joey_audio_diagnostic[2]=9;ended=1;return 0;}
        ++block_index;decoded_position=0;
    }
    *sample=(int16_t)(((decoded_music[decoded_position++]*(int)e->gain)>>8)*(int)GAME_VOLUME_PERCENT/100);++music_frame;return 1;
}
void joey_audio_tick(void){
    if(!device_owned)return;unsigned tick=h1_raw_tick_80hz();if(tick==last_tick)return;last_tick=tick;++joey_audio_diagnostic[17];
    if(!retire_music(tick)||!device_owned)return;
    if(music_wanted<JOEY_AUDIO_MUSIC_COUNT){unsigned work=0;int16_t sample;
        while(queued<TARGET_FRAMES&&work++<BATCH_FRAMES&&!ended){if(!next_music(&sample))break;music_ring[write_position]=sample;write_position=(write_position+1)%RING_FRAMES;++queued;++joey_audio_diagnostic[7];}
        if(!music_playing&&queued&&(queued>=PREFILL_FRAMES||ended)){
            if(submit(0,&music_descriptor,0,1)){music_playing=1;retire_tick=progress_tick=tick;progress_position=read_position;}
            else{++joey_audio_diagnostic[28];joey_audio_diagnostic[2]=10;joey_audio_close();return;}
        }
        if(ended&&!queued&&music_playing){cancel_music();++joey_audio_diagnostic[16];}
    }update_diagnostic();
}
void joey_audio_effect(unsigned asset){
    if(!device_owned||asset<JOEY_AUDIO_MUSIC_COUNT||asset>=JOEY_AUDIO_ASSET_COUNT)return;
    ++joey_audio_diagnostic[13];joey_audio_diagnostic[29]=asset;const AudioEntry *e=&entries[asset];unsigned slot=0;
    if(effect_assets[1]==asset)slot=1;else if(effect_assets[0]!=asset&&effect_stamp[1]<effect_stamp[0])slot=1;
    if(!submit(slot+1,0,0,0)){++joey_audio_diagnostic[28];joey_audio_diagnostic[2]=14;update_diagnostic();return;} /* Cancellation precedes reusing any PCM memory. */
    if(effect_assets[slot]!=asset){
        if(effect_live[slot]){destroy(&effect_descriptors[slot]);effect_live[slot]=0;++joey_audio_diagnostic[4];}
        effect_assets[slot]=JOEY_AUDIO_NONE;
        if(!effect_allocations[slot])effect_allocations[slot]=h1_alloc(JOEY_AUDIO_MAX_FX_FRAMES*2+31);
        if(!effect_allocations[slot]||e->frames>JOEY_AUDIO_MAX_FX_FRAMES){joey_audio_diagnostic[2]=11;update_diagnostic();return;}
        int16_t *pcm=(int16_t *)(((uintptr_t)effect_allocations[slot]+31)&~(uintptr_t)31);unsigned at=0;
        h1_fseek(audio_file,e->offset,H1_SEEK_SET);
        while(at<e->frames){unsigned n=e->frames-at;if(n>JOEY_IMA_BLOCK_FRAMES)n=JOEY_IMA_BLOCK_FRAMES;
            if(h1_fread(encoded,1,JOEY_IMA_BLOCK_BYTES,audio_file)!=JOEY_IMA_BLOCK_BYTES||joey_ima_decode_block(encoded,JOEY_IMA_BLOCK_BYTES,pcm+at,n)!=n){++joey_audio_diagnostic[18];joey_audio_diagnostic[2]=12;effect_assets[slot]=JOEY_AUDIO_NONE;update_diagnostic();return;}
            for(unsigned i=0;i<n;++i)pcm[at+i]=(int16_t)(((pcm[at+i]*(int)e->gain)>>8)*(int)GAME_VOLUME_PERCENT/100);at+=n;
        }
        h1_memset(&effect_descriptors[slot],0,sizeof(PcmDescriptor));effect_descriptors[slot].pcm=pcm;effect_descriptors[slot].bytes=e->frames*2;
        if(!initialize(&effect_descriptors[slot])){effect_assets[slot]=JOEY_AUDIO_NONE;joey_audio_diagnostic[2]=13;update_diagnostic();return;}
        effect_live[slot]=1;effect_assets[slot]=asset;++joey_audio_diagnostic[3];joey_audio_diagnostic[15]+=e->frames;
    }
    effect_stamp[slot]=++stamp;
    if(submit(slot+1,&effect_descriptors[slot],1,1))++joey_audio_diagnostic[14];else ++joey_audio_diagnostic[28];update_diagnostic();
}
