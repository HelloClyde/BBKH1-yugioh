#ifndef JOEY_DUEL_PATHS_H
#define JOEY_DUEL_PATHS_H

/* H1 filesystem paths use GBK, independently of the source-file encoding.
 * These bytes spell "应用\数据"; resource filenames remain ASCII. */
#define JOEY_RESOURCE_ROOT_A "A:\\\xd3\xa6\xd3\xc3\\\xca\xfd\xbe\xdd\\yugioh\\"
#define JOEY_RESOURCE_ROOT_B "B:\\\xd3\xa6\xd3\xc3\\\xca\xfd\xbe\xdd\\yugioh\\"
static inline h1_file *joey_resource_open(const char *a,const char *b,unsigned *drive){
    h1_file *file=h1_fopen(a,"rb");
    if(file){if(drive)*drive=0;return file;}
    file=h1_fopen(b,"rb");if(file&&drive)*drive=1;return file;
}
#define JOEY_OPEN_RESOURCE(name,drive) joey_resource_open(JOEY_RESOURCE_ROOT_A name,JOEY_RESOURCE_ROOT_B name,drive)

#endif
