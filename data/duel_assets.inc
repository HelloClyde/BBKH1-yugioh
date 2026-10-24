/* Shared native bitmap assets and drawing only. */
typedef struct {uint16_t texture,flags,attack,defense;uint32_t name,description;uint16_t name_length,description_length;} NativeCard;
typedef struct {uint32_t key,stamp;uint16_t palette[256];uint8_t pixels[36*52];} NativeMini;
static h1_file *native_file;
static h1_file *native_art_file;
static uint32_t native_art_header[8];
static uint16_t native_wood[181*32],native_paper[32*32];
static uint16_t native_life_pixels[2][116*9];
static uint32_t native_header[18],native_stamp,native_large_key=0xffffffffu;
static NativeCard native_cards[1115];
static NativeMini native_mini[24];
static uint16_t native_background[480*272],native_large_palette[256];
static uint8_t native_large_pixels[88*128];
static unsigned native_ready,native_attempted,native_selected,native_seen_duel;
static unsigned native_background_key=0xffffffffu;
static unsigned native_menu_focus,native_menu_key=0xffffffffu,native_hand_count=0xffffffffu;
static int native_focus=-1;
static int native_stat_focus=-1;

static int native_load(void){
    if(native_attempted)return native_ready;native_attempted=1;
    native_file=h1_fopen("A:\\JOEY\\H1UI.PAK","rb");if(!native_file)native_file=h1_fopen("B:\\JOEY\\H1UI.PAK","rb");if(!native_file)return 0;
    if(h1_fread(native_header,4,18,native_file)!=18||native_header[0]!=0x4955594au||native_header[1]!=0x31303030u||native_header[2]!=3||native_header[3]!=1115||native_header[10]!=480||native_header[11]!=272)return 0;
    h1_fseek(native_file,native_header[5],H1_SEEK_SET);if(h1_fread(native_cards,20,1115,native_file)!=1115)return 0;
    native_art_file=h1_fopen("B:\\JOEY\\H1ART.PAK","rb");if(!native_art_file)native_art_file=h1_fopen("A:\\JOEY\\H1ART.PAK","rb");
    if(!native_art_file)return 0;
    if(h1_fread(native_art_header,4,8,native_art_file)!=8||native_art_header[0]!=0x5241594au||native_art_header[1]!=0x31303054u||native_art_header[2]!=2||native_art_header[3]!=480||native_art_header[4]!=272||native_art_header[5]!=3||native_art_header[6]!=32)return 0;
    h1_fseek(native_art_file,native_art_header[7],H1_SEEK_SET);
    if(h1_fread(native_wood,2,181*32,native_art_file)!=181*32||h1_fread(native_paper,2,32*32,native_art_file)!=32*32)return 0;
    if(h1_fread(native_life_pixels,2,2*116*9,native_art_file)!=2*116*9)return 0;
    for(unsigned i=0;i<24;++i)native_mini[i].key=0xffffffffu;native_ready=1;return 1;
}
static void native_text(int x,int y,const uint16_t *text,unsigned count,unsigned height,uint32_t color){uint32_t bgr=(color>>16)|(color&0xff00)|((color&255)<<16);native_font_text(screen,480,272,x,y,text,count,bgr,0,height|0x10000);}
static void native_number(int x,int y,unsigned value,unsigned height,uint32_t color){uint16_t text[12],reverse[12];unsigned n=0;do{reverse[n++]='0'+value%10;value/=10;}while(value&&n<12);for(unsigned i=0;i<n;++i)text[i]=reverse[n-1-i];native_text(x,y,text,n,height,color);}
static unsigned native_string(unsigned offset,unsigned length,uint16_t *out,unsigned capacity){if(length>capacity)length=capacity;h1_fseek(native_file,native_header[7]+offset,H1_SEEK_SET);return h1_fread(out,2,length,native_file);}
static void native_wrapped(int x,int y,unsigned width,const uint16_t *text,unsigned count,unsigned height,unsigned max_lines,uint32_t color){
    unsigned line=0,column=0;for(unsigned i=0;i<count&&line<max_lines;++i){unsigned advance=text[i]<128?height/2:height;
        if(text[i]=='\n'||column+advance>width){++line;column=0;if(text[i]=='\n')continue;}if(line>=max_lines)break;
        native_text(x+column,y+line*(height+3),text+i,1,height,color);column+=advance;
    }
}
static int native_texture(unsigned card,unsigned kind,uint16_t **palette,uint8_t **pixels){
    if(card>=1115)card=0;unsigned texture=native_cards[card].texture;uint32_t location[2];
    if(texture>=native_header[4])return 0;
    if(kind==0&&native_large_key==texture){*palette=native_large_palette;*pixels=native_large_pixels;return 1;}
    unsigned key=texture*2+(kind==2),slot=0;
    if(kind){for(unsigned i=0;i<24;++i){if(native_mini[i].key==key){native_mini[i].stamp=++native_stamp;*palette=native_mini[i].palette;*pixels=native_mini[i].pixels;return 1;}if(native_mini[i].stamp<native_mini[slot].stamp)slot=i;}}
    h1_fseek(native_file,native_header[6]+texture*8,H1_SEEK_SET);if(h1_fread(location,4,2,native_file)!=2)return 0;
    unsigned position=native_header[8]+location[0];h1_fseek(native_file,position,H1_SEEK_SET);
    uint16_t *colors=kind?native_mini[slot].palette:native_large_palette;uint8_t *data=kind?native_mini[slot].pixels:native_large_pixels;
    if(h1_fread(colors,2,256,native_file)!=256)return 0;
    unsigned offset=kind==1?88*128:kind==2?88*128+36*52:0,count=kind==0?88*128:kind==1?36*52:32*46;
    h1_fseek(native_file,position+512+offset,H1_SEEK_SET);if(h1_fread(data,1,count,native_file)!=count)return 0;
    if(kind){native_mini[slot].key=key;native_mini[slot].stamp=++native_stamp;}else native_large_key=texture;
    *palette=colors;*pixels=data;return 1;
}
static void native_card(int x,int y,unsigned card,unsigned kind){
    uint16_t *colors;uint8_t *pixels;if(!native_texture(card,kind,&colors,&pixels))return;
    unsigned width=kind==0?88:kind==1?36:32,height=kind==0?128:kind==1?52:46;
    for(unsigned dy=0;dy<height;++dy)for(unsigned dx=0;dx<width;++dx)if(x+(int)dx>=0&&x+(int)dx<480&&y+(int)dy>=0&&y+(int)dy<272)screen[(y+dy)*480+x+dx]=colors[pixels[dy*width+dx]];
}
static void native_card_rotated(int x,int y,unsigned card,unsigned kind){
    uint16_t *colors;uint8_t *pixels;if(!native_texture(card,kind,&colors,&pixels))return;unsigned width=kind==1?36:32,height=kind==1?52:46;
    for(unsigned dy=0;dy<width;++dy)for(unsigned dx=0;dx<height;++dx)if(x+(int)dx>=0&&x+(int)dx<480&&y+(int)dy>=0&&y+(int)dy<272)screen[(y+dy)*480+x+dx]=colors[pixels[(height-1-dx)*width+dy]];
}
static void native_field_card(int x,int y,unsigned card,unsigned quarter){
    uint16_t *colors;uint8_t *pixels;if(!native_texture(card,2,&colors,&pixels))return;
    unsigned width=quarter&1?46:32,height=quarter&1?32:46;
    x+=(32-(int)width)/2;y+=(46-(int)height)/2;
    for(unsigned dy=0;dy<height;++dy)for(unsigned dx=0;dx<width;++dx){
        unsigned source=quarter==0?dy*32+dx:quarter==1?(45-dx)*32+dy:quarter==2?(45-dy)*32+31-dx:dx*32+31-dy;
        if(x+(int)dx>=0&&x+(int)dx<480&&y+(int)dy>=0&&y+(int)dy<272)screen[(y+dy)*480+x+dx]=colors[pixels[source]];
    }
}
static void native_outline(int x,int y,unsigned width,unsigned height,uint16_t color){
    for(unsigned i=0;i<width;++i){if(x+(int)i>=0&&x+(int)i<480){if(y>=0&&y<272)screen[y*480+x+i]=color;if(y+(int)height-1>=0&&y+(int)height-1<272)screen[(y+height-1)*480+x+i]=color;}}
    for(unsigned i=0;i<height;++i)if(y+(int)i>=0&&y+(int)i<272){if(x>=0&&x<480)screen[(y+i)*480+x]=color;if(x+(int)width-1>=0&&x+(int)width-1<480)screen[(y+i)*480+x+width-1]=color;}
}
static void native_backdrop(unsigned key){
    if(native_background_key!=key){h1_fseek(native_art_file,native_art_header[6]+key*sizeof(native_background),H1_SEEK_SET);if(h1_fread(native_background,2,480*272,native_art_file)!=480*272)h1_memset(native_background,0,sizeof(native_background));native_background_key=key;}
    h1_memcpy(screen,native_background,sizeof(screen));
}
static void native_fill(int x,int y,unsigned width,unsigned height,uint16_t color){for(unsigned dy=0;dy<height;++dy)for(unsigned dx=0;dx<width;++dx)if(x+(int)dx>=0&&x+(int)dx<480&&y+(int)dy>=0&&y+(int)dy<272)screen[(y+dy)*480+x+dx]=color;}
static void native_life(unsigned side,unsigned life){
    unsigned filled=(life>8000?8000:life)*116/8000,y=22+side*240;
    for(unsigned dy=0;dy<9;++dy)for(unsigned dx=0;dx<filled;++dx)screen[(y+dy)*480+28+dx]=native_life_pixels[side][dy*116+dx];
    native_number(35,side*240,life,18,0xffda72);
}
static void native_label(int x,int y,const uint16_t *text,unsigned height,uint32_t color){unsigned count=0;while(text[count])++count;native_text(x,y,text,count,height,color);}
static void native_panel(int x,int y,unsigned width,unsigned height){
    for(unsigned dy=0;dy<height;++dy)for(unsigned dx=0;dx<width;++dx)if(x+(int)dx>=0&&x+(int)dx<480&&y+(int)dy>=0&&y+(int)dy<272)screen[(y+dy)*480+x+dx]=native_paper[(dy&31)*32+(dx&31)];
    native_outline(x,y,width,height,0x9c0c);
}
static void native_button(int x,int y,unsigned width,const uint16_t *label){
    for(unsigned dy=0;dy<32;++dy)for(unsigned dx=0;dx<width;++dx){unsigned sx=dx<8?dx:dx>=width-8?181-(width-dx):8+(dx-8)%165;if(x+(int)dx<480&&y+(int)dy<272)screen[(y+dy)*480+x+dx]=native_wood[dy*181+sx];}
    native_label(x+13,y+7,label,18,0x28170a);native_label(x+12,y+6,label,18,0xffe0b1);
}
static void native_menu_button(unsigned index,int x,int y,unsigned width,const uint16_t *label){native_button(x,y,width,label);if(native_menu_focus==index)native_outline(x-2,y-2,width+4,36,0xffe0);}
static void native_title_button(unsigned index,int y,const uint16_t *label){
    for(unsigned dy=0;dy<23;++dy)for(unsigned dx=0;dx<181;++dx)screen[(y+dy)*480+149+dx]=native_wood[(dy*32/23)*181+dx];
    native_label(207,y+3,label,16,0x211509);if(native_menu_focus==index)native_outline(148,y-1,183,25,0xffe0);
}
