/* Full scene canvases share the existing background buffer and cache key. */
#include "duel_attack_assets.h"
static h1_file *scene_file;
static int scene_load(void){
    uint32_t header[8];scene_file=JOEY_OPEN_RESOURCE("H1SCN.PAK",0);
    return scene_file&&h1_fread(header,4,8,scene_file)==8&&header[0]==0x4353594au&&header[1]==0x3130304eu&&header[2]==1&&header[3]==2&&header[4]==480&&header[5]==272&&header[6]==32&&h1_fseek(scene_file,0,H1_SEEK_END)==522272;
}
static void scene_background(unsigned key){
    if(native_background_key!=8+key){h1_fseek(scene_file,32+key*sizeof(native_background),H1_SEEK_SET);if(h1_fread(native_background,2,480*272,scene_file)!=480*272)h1_memset(native_background,0,sizeof(native_background));native_background_key=8+key;}
    h1_memcpy(screen,native_background,sizeof(screen));
}
static void scene_card(int x,int y,unsigned card,unsigned kind,unsigned width,unsigned height){
    uint16_t *colors;uint8_t *pixels;if(!native_texture(card,kind,&colors,&pixels))return;
    unsigned sw=kind==0?88:kind==1?36:32,sh=kind==0?128:kind==1?52:46;
    for(unsigned dy=0;dy<height;++dy)for(unsigned dx=0;dx<width;++dx)if(x+(int)dx>=0&&x+(int)dx<480&&y+(int)dy>=0&&y+(int)dy<272)screen[(y+dy)*480+x+dx]=colors[pixels[(dy*sh/height)*sw+dx*sw/width]];
}
static void scene_menu(void){
    scene_background(0);const uint16_t *labels[]={u"单局决斗",u"卡组整理",u"卡片鉴赏",u"对局记录",u"退出游戏"};
    for(unsigned i=0;i<5;++i){native_label(213,146+i*22,labels[i],14,0x382312);native_label(212,145+i*22,labels[i],14,0xffffff);if(native_menu_focus==i)native_outline(148,143+i*22,184,20,0xffe0);}
    /* The last four source rows include the old menu's fence. Use only wood. */
    for(unsigned y=0;y<24;++y)for(unsigned x=0;x<58;++x)screen[(243+y)*480+414+x]=native_wood[(y*28/24)*181+x*181/58];
    native_label(430,249,u"关于",14,0x382312);native_label(429,248,u"关于",14,0xffffff);if(native_menu_focus==5)native_outline(413,242,60,26,0xffe0);
}
static void scene_about(void){
    native_panel(25,13,430,245);native_label(163,21,u"关于 游戏王",20,0x513819);
    native_label(45,54,u"移植作者：HelloClyde",18,0x35230f);
    native_label(45,83,u"原游戏：混沌力量·城之内篇",14,0x35230f);
    native_label(45,103,u"Yu-Gi-Oh! Power of Chaos: Joey the Passion",12,0x35230f);
    native_label(45,124,u"原版开发与发行：KONAMI",14,0x35230f);
    native_label(45,145,u"原作版权：©1996 KAZUKI TAKAHASHI",12,0x35230f);
    native_label(45,166,u"汉化署名：游戏王汉化制作组",14,0x35230f);
    native_label(45,186,u"署名依据：本版 Load.exe 版本信息（2004）",12,0x513819);
    native_label(45,204,u"原图、音乐、卡片等版权归原权利人所有。",12,0x513819);
    native_button(184,222,112,u"关闭");
}
static void scene_attack_sword(int x,int y,int dx,int dy,unsigned frame){
    unsigned square=dx*dx+dy*dy,length=1;while(length*length<square)++length;
    /* Inverse rotation: source tip points up. A common 3/5 scale preserves
       the original sword's proportions in both axes, without floating point. */
    for(int oy=-24;oy<=24;++oy)for(int ox=-24;ox<=24;++ox){int sx=17+(-ox*dy+oy*dx)*5/((int)length*3),sy=35+(-ox*dx-oy*dy)*5/((int)length*3);
        if(sx<0||sx>=35||sy<0||sy>=71||x+ox<0||x+ox>=480||y+oy<0||y+oy>=272)continue;
        uint16_t color=scene_attack_pixels[frame&1][sy*35+sx];if(color!=0x07e0)screen[(y+oy)*480+x+ox]=color;
    }
}
