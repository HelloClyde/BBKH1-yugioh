#ifndef JOEY_DUEL_IMA_H
#define JOEY_DUEL_IMA_H
#include <stdint.h>
#define JOEY_IMA_BLOCK_BYTES 256u
#define JOEY_IMA_BLOCK_FRAMES 505u
unsigned joey_ima_decode_block(const uint8_t *,unsigned,int16_t *,unsigned);
#endif
