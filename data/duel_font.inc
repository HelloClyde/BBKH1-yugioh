/* Shared native bitmap font renderer. */
static uint32_t native_font_stats[24];
static h1_file *font_file;
static uint32_t font_desc[25][5];
static uint8_t font_index_storage[3000*12],glyph[1024];
static uint8_t *font_index=font_index_storage,*font_books[3]={font_index_storage,0,0};
static unsigned font_book_height[3],font_book_stamp[3],font_stamp;
typedef struct {uint32_t key;uint8_t pixels[128];} FontGlyphCache;
static FontGlyphCache *font_glyph_cache;
static unsigned loaded_height,loaded_count;
static unsigned u16(const uint8_t *p){return p[0]|p[1]<<8;}
static uint32_t u32(const uint8_t *p){return p[0]|p[1]<<8|p[2]<<16|(uint32_t)p[3]<<24;}
static unsigned u24(const uint8_t *p){return p[0]|p[1]<<8|(uint32_t)p[2]<<16;}
static const uint8_t *find_glyph(unsigned cp){unsigned lo=0,hi=loaded_count;while(lo<hi){unsigned mid=(lo+hi)/2;if(u16(font_index+mid*12)<cp)lo=mid+1;else hi=mid;}return lo<loaded_count&&u16(font_index+lo*12)==cp?font_index+lo*12:0;}
static uint16_t color565(uint32_t rgb){return ((rgb&255)>>3)<<11|((rgb>>8&255)>>2)<<5|((rgb>>16&255)>>3);}
static void text_pixel(uint16_t *dest,unsigned w,unsigned h,int x,int y,uint32_t color,unsigned options){
    if(x<0||y<0||(unsigned)x>=w||(unsigned)y>=h)return;unsigned bpp=(options>>17)&63;if(!bpp)bpp=16;
    unsigned stride=options&0x1000000?((w*bpp+31)/32)*4:w*bpp/8,sy=options&0x800000?h-1-y:(unsigned)y;
    uint8_t *p=(uint8_t *)dest+sy*stride+x*(bpp/8);
    if(bpp==16){unsigned pixel=options&0x1000000&&!(options&0x2000000)?((color&255)>>3)<<10|((color>>8&255)>>3)<<5|((color>>16&255)>>3):color565(color);p[0]=pixel;p[1]=pixel>>8;}
    else {p[0]=color>>16;p[1]=color>>8;p[2]=color;if(bpp==32)p[3]=0;}
}
uint32_t native_font_text(uint16_t *dest,uint32_t w,uint32_t h,int32_t x,int32_t y,const uint16_t *text,uint32_t n,uint32_t color,uint32_t background,uint32_t options){
    /* Original text sprites also provide pixel hit masks for buttons. Keep
     * rasterizing them even though the visible H1 screens are independent. */
    unsigned height=options&65535;if(height<8)height=8;if(height>32)height=32;
    if(!font_file){uint8_t header[16];font_file=h1_fopen("B:\\JOEY\\FONT.BIN","rb");if(!font_file)font_file=h1_fopen("A:\\JOEY\\FONT.BIN","rb");if(!font_file)return 0;
        if(h1_fread(header,1,16,font_file)!=16||header[0]!='J'||u32(header+8)!=1||u32(header+12)!=25)return 0;
        if(h1_fread(font_desc,20,25,font_file)!=25)return 0;}
    const uint32_t *desc=font_desc[height-8];
    if(loaded_height!=height){
        if(desc[4]>3000)return 0;unsigned slot=3;
        for(unsigned i=0;i<3;++i)if(font_books[i]&&font_book_height[i]==height){slot=i;break;}
        if(slot==3){
            /* Optional caches are allocated after the large VM banks so they
             * cannot reduce the contiguous heap available to the rules. */
            if(1)for(unsigned i=1;i<3;++i)if(!font_books[i]){font_books[i]=h1_alloc(sizeof(font_index_storage));if(font_books[i])++native_font_stats[20];}
            slot=0;for(unsigned i=1;i<3;++i)if(font_books[i]&&font_book_stamp[i]<font_book_stamp[slot])slot=i;
            h1_fseek(font_file,desc[3],H1_SEEK_SET);if(h1_fread(font_books[slot],12,desc[4],font_file)!=desc[4])return 0;font_book_height[slot]=height;++native_font_stats[17];
        }
        font_index=font_books[slot];font_book_stamp[slot]=++font_stamp;loaded_height=height;loaded_count=desc[4];
    }
    if(!font_glyph_cache&&1){font_glyph_cache=h1_alloc(128*sizeof(FontGlyphCache));if(font_glyph_cache)h1_memset(font_glyph_cache,0,128*sizeof(FontGlyphCache));}
    unsigned advance=0,lines=desc[1]+desc[2];for(unsigned i=0;i<n;++i){const uint8_t *g=find_glyph(text[i]);advance+=g?g[6]:height;}
    if(dest&&!(options&0x10000))for(unsigned dy=0;dy<lines;++dy)for(unsigned dx=0;dx<advance;++dx)text_pixel(dest,w,h,x+dx,y+dy,background,options);
    if(dest)for(unsigned i=0;i<n;++i){const uint8_t *g=find_glyph(text[i]);if(!g){x+=height;continue;}
        unsigned gw=g[2],gh=g[3],stride=(gw+7)/8,bytes=stride*gh;int gx=x+(int8_t)g[4],gy=y+(int8_t)g[5];
        if(bytes<=sizeof(glyph)){uint8_t *bitmap=glyph;unsigned cached=0;uint32_t key=(height<<16)|text[i];FontGlyphCache *entry=font_glyph_cache&&bytes<=128?&font_glyph_cache[(text[i]*31+height*7)&127]:0;
            if(entry&&entry->key==key){bitmap=entry->pixels;cached=1;++native_font_stats[18];}
            if(!cached){h1_fseek(font_file,u32(g+8),H1_SEEK_SET);cached=h1_fread(glyph,1,bytes,font_file)==bytes;++native_font_stats[19];if(cached&&entry){h1_memcpy(entry->pixels,glyph,bytes);entry->key=key;bitmap=entry->pixels;}}
            if(cached)
            for(unsigned dy=0;dy<gh;++dy)for(unsigned dx=0;dx<gw;++dx)if(bitmap[dy*stride+dx/8]&(128>>(dx&7)))text_pixel(dest,w,h,gx+dx,gy+dy,color,options);}
        x+=g[6];}
    return (lines<<16)|(advance&65535);
}
