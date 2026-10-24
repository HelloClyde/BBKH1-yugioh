/* Native deck and collection state. The wire format is explicit little-endian;
 * C padding, pointers, PC save encryption and executable code are never saved. */
#include "duel_profile.h"

static void clear(void *p,unsigned n){uint8_t *b=p;while(n--)*b++=0;}
static void copy(void *to,const void *from,unsigned n){uint8_t *d=to;const uint8_t *s=from;while(n--)*d++=*s++;}
static unsigned read16(const uint8_t *b){return b[0]|(unsigned)b[1]<<8;}
static uint32_t read32(const uint8_t *b){return read16(b)|(uint32_t)read16(b+2)<<16;}
static void write16(uint8_t *b,unsigned v){b[0]=v;b[1]=v>>8;}
static void write32(uint8_t *b,uint32_t v){write16(b,v);write16(b+2,v>>16);}
static uint32_t checksum(const uint8_t *b,unsigned size){uint32_t crc=~0u;for(unsigned i=0;i<size;++i){crc^=i>=20&&i<24?0:b[i];for(unsigned bit=0;bit<8;++bit)crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u);}return ~crc;}
static unsigned card_at(const JoeyProfile *p,unsigned i){if(i<p->main_count)return p->main[i];i-=p->main_count;return i<p->fusion_count?p->fusion[i]:p->side[i-p->fusion_count];}
unsigned joey_profile_count(const JoeyProfile *p,unsigned card){unsigned count=0;for(unsigned i=0;i<p->main_count+p->fusion_count+p->side_count;++i)count+=card_at(p,i)==card;return count;}

void joey_profile_init(JoeyProfile *p,const uint8_t *owned,const uint16_t *main,unsigned n,const uint16_t *fusion,unsigned f){
    clear(p,sizeof(*p));if(n>DUEL_DECK_MAX||f>DUEL_FUSION_MAX)return;
    copy(p->owned,owned,DUEL_CARD_COUNT);copy(p->main,main,n*2);copy(p->fusion,fusion,f*2);p->main_count=n;p->fusion_count=f;
}
int joey_profile_validate(const JoeyProfile *p,const DuelCardDef *cards,const uint8_t *limits,unsigned allow_short){
    if(p->reward_gauge>=20||(p->reward_flags&~7u)||((p->reward_flags&JOEY_REWARD_PENDING)?!p->reward_card||p->reward_card>=DUEL_CARD_COUNT:p->reward_card||p->reward_flags))return JOEY_PROFILE_FORMAT;
    if(p->main_count>DUEL_DECK_MAX||(!allow_short&&p->main_count<40)||p->fusion_count>DUEL_FUSION_MAX||p->side_count>JOEY_SIDE_MAX)return JOEY_PROFILE_SIZE;
    unsigned n=p->main_count+p->fusion_count+p->side_count;
    for(unsigned i=0;i<n;++i){
        unsigned card=card_at(p,i);if(!card||card>=DUEL_CARD_COUNT)return JOEY_PROFILE_CARD;
        const DuelCardDef *c=&cards[card];if(c->race>=23)return JOEY_PROFILE_CARD;unsigned fusion=c->family==DUEL_MONSTER&&c->subtype==2;
        if(i<p->main_count+p->fusion_count&&fusion!=(i>=p->main_count))return JOEY_PROFILE_CARD;
        if(!duel_card_supported(c))return JOEY_PROFILE_UNSUPPORTED;
        unsigned same=0,group=0;
        for(unsigned j=0;j<n;++j){unsigned other=card_at(p,j);if(other>=DUEL_CARD_COUNT)return JOEY_PROFILE_CARD;same+=other==card;group+=cards[other].name_group==c->name_group;}
        if(same>p->owned[card])return JOEY_PROFILE_OWNED;
        if(group>limits[card])return JOEY_PROFILE_LIMIT;
    }
    return JOEY_PROFILE_OK;
}
int joey_profile_change(JoeyProfile *p,const DuelCardDef *cards,const uint8_t *limits,unsigned card,int add){
    if(!card||card>=DUEL_CARD_COUNT)return JOEY_PROFILE_CARD;
    unsigned fusion=cards[card].family==DUEL_MONSTER&&cards[card].subtype==2;
    return joey_profile_change_pile(p,cards,limits,card,add,fusion?JOEY_FUSION:JOEY_MAIN);
}
int joey_profile_change_pile(JoeyProfile *p,const DuelCardDef *cards,const uint8_t *limits,unsigned card,int add,unsigned pile){
    if(!card||card>=DUEL_CARD_COUNT||pile>JOEY_SIDE)return JOEY_PROFILE_CARD;
    uint16_t *deck=pile==JOEY_SIDE?p->side:pile==JOEY_FUSION?p->fusion:p->main,*count=pile==JOEY_SIDE?&p->side_count:pile==JOEY_FUSION?&p->fusion_count:&p->main_count;
    if(add){
        if(cards[card].race>=23)return JOEY_PROFILE_CARD;
        if(*count>=(pile==JOEY_MAIN?DUEL_DECK_MAX:15))return JOEY_PROFILE_SIZE;
        deck[(*count)++]=card;int result=joey_profile_validate(p,cards,limits,1);
        if(result)deck[--*count]=0;return result;
    }
    for(unsigned i=0;i<*count;++i)if(deck[i]==card){for(unsigned j=i+1;j<*count;++j)deck[j-1]=deck[j];deck[--*count]=0;return JOEY_PROFILE_OK;}
    return JOEY_PROFILE_OWNED;
}
int joey_profile_encode(const JoeyProfile *p,const DuelCardDef *cards,const uint8_t *limits,uint8_t *b,unsigned size){
    if(size!=JOEY_PROFILE_SAVE_BYTES)return JOEY_PROFILE_FORMAT;int result=joey_profile_validate(p,cards,limits,0);if(result)return result;
    clear(b,size);copy(b,"JNATSAV4",8);write32(b+8,4);write32(b+12,p->sequence);write32(b+16,size);write16(b+24,p->main_count);write16(b+26,p->fusion_count);
    for(unsigned i=0;i<p->main_count;++i)write16(b+28+2*i,p->main[i]);
    for(unsigned i=0;i<p->fusion_count;++i)write16(b+188+2*i,p->fusion[i]);
    copy(b+218,p->owned,DUEL_CARD_COUNT);write32(b+1333,p->wins);write32(b+1337,p->losses);write16(b+1341,p->reward_card);b[1343]=p->reward_gauge;b[1344]=p->reward_flags;
    write16(b+1345,p->side_count);for(unsigned i=0;i<p->side_count;++i)write16(b+1347+2*i,p->side[i]);write32(b+20,checksum(b,size));return JOEY_PROFILE_OK;
}
int joey_profile_decode(JoeyProfile *p,const DuelCardDef *cards,const uint8_t *limits,const uint8_t *b,unsigned size){
    if(size!=JOEY_PROFILE_SAVE_BYTES&&size!=JOEY_PROFILE_V3_BYTES&&size!=JOEY_PROFILE_V2_BYTES)return JOEY_PROFILE_FORMAT;
    unsigned version=size==JOEY_PROFILE_V2_BYTES?2:size==JOEY_PROFILE_V3_BYTES?3:4;const char *magic=version==2?"JNATSAV2":version==3?"JNATSAV3":"JNATSAV4";for(unsigned i=0;i<8;++i)if(b[i]!=(uint8_t)magic[i])return JOEY_PROFILE_FORMAT;
    if(read32(b+8)!=version||read32(b+16)!=size||read32(b+20)!=checksum(b,size))return JOEY_PROFILE_FORMAT;
    JoeyProfile candidate;clear(&candidate,sizeof(candidate));candidate.sequence=read32(b+12);candidate.main_count=read16(b+24);candidate.fusion_count=read16(b+26);
    if(candidate.main_count>DUEL_DECK_MAX||candidate.fusion_count>DUEL_FUSION_MAX)return JOEY_PROFILE_SIZE;
    for(unsigned i=0;i<candidate.main_count;++i)candidate.main[i]=read16(b+28+2*i);
    for(unsigned i=0;i<candidate.fusion_count;++i)candidate.fusion[i]=read16(b+188+2*i);
    copy(candidate.owned,b+218,DUEL_CARD_COUNT);if(version>=3){candidate.wins=read32(b+1333);candidate.losses=read32(b+1337);candidate.reward_card=read16(b+1341);candidate.reward_gauge=b[1343];candidate.reward_flags=b[1344];}
    if(version==4){candidate.side_count=read16(b+1345);if(candidate.side_count>JOEY_SIDE_MAX)return JOEY_PROFILE_SIZE;for(unsigned i=0;i<candidate.side_count;++i)candidate.side[i]=read16(b+1347+2*i);}
    int result=joey_profile_validate(&candidate,cards,limits,0);if(result)return result;
    copy(p,&candidate,sizeof(candidate));return JOEY_PROFILE_OK;
}
int joey_profile_newest(const JoeyProfile *a,int valid_a,const JoeyProfile *b,int valid_b){
    if(!valid_a)return valid_b?1:-1;if(!valid_b)return 0;return (int32_t)(b->sequence-a->sequence)>0?1:0;
}
/* The original single-duel prize pool is independent of implemented effects.
 * Coverage >=80% / >=90% allows two / three attempts to find an unowned card.
 * A full gauge offers one replacement, not a second awarded copy. */
static int reward_offer(JoeyProfile *p,const uint16_t *pool,unsigned count,JoeyRewardRandom random,void *context,unsigned replacement){
    if(!pool||!count||!random)return JOEY_PROFILE_CARD;unsigned owned=0;
    for(unsigned i=0;i<count;++i){if(!pool[i]||pool[i]>=DUEL_CARD_COUNT)return JOEY_PROFILE_CARD;owned+=p->owned[pool[i]]!=0;}
    unsigned attempts=1+(owned*100/count>=80)+(owned*100/count>=90),card=0,is_new=0;
    for(unsigned i=0;i<attempts;++i){card=pool[random(context)%count];is_new=p->owned[card]==0;if(is_new)break;}
    unsigned gauge=replacement?0:p->reward_gauge+1+random(context)%10,reroll=0;
    if(gauge>=20){if(is_new)gauge=17;else {gauge=0;reroll=1;}}
    p->reward_card=card;p->reward_gauge=gauge;p->reward_flags=JOEY_REWARD_PENDING|(is_new?JOEY_REWARD_NEW:0)|(reroll?JOEY_REWARD_REROLL:0);return JOEY_PROFILE_OK;
}
int joey_profile_finish(JoeyProfile *p,unsigned won,const uint16_t *pool,unsigned count,JoeyRewardRandom random,void *context){
    if(won>1)return JOEY_PROFILE_CARD;if(p->reward_flags&JOEY_REWARD_PENDING)return JOEY_PROFILE_PENDING;
    if(won){int result=reward_offer(p,pool,count,random,context,0);if(result)return result;if(p->wins!=0xffffffffu)++p->wins;}
    else if(p->losses!=0xffffffffu)++p->losses;return JOEY_PROFILE_OK;
}
int joey_profile_reroll(JoeyProfile *p,const uint16_t *pool,unsigned count,JoeyRewardRandom random,void *context){
    if((p->reward_flags&(JOEY_REWARD_PENDING|JOEY_REWARD_REROLL))!=(JOEY_REWARD_PENDING|JOEY_REWARD_REROLL))return JOEY_PROFILE_PENDING;
    return reward_offer(p,pool,count,random,context,1);
}
int joey_profile_claim(JoeyProfile *p){
    if(!(p->reward_flags&JOEY_REWARD_PENDING)||!p->reward_card||p->reward_card>=DUEL_CARD_COUNT)return JOEY_PROFILE_PENDING;
    if(p->owned[p->reward_card]!=255)++p->owned[p->reward_card];p->reward_card=p->reward_flags=0;return JOEY_PROFILE_OK;
}
