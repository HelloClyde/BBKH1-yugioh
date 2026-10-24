#include <stdint.h>
void *memset(void *dst,int value,unsigned size){volatile uint8_t *p=dst;while(size--)*p++=(uint8_t)value;return dst;}
void *memcpy(void *dst,const void *src,unsigned size){volatile uint8_t *d=dst;const volatile uint8_t *s=src;while(size--)*d++=*s++;return dst;}
