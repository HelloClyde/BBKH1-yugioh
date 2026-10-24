/* Integer-only mono Microsoft IMA WAV block decoder. */
#include "duel_ima.h"
static const int steps[89]={7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767};
static const int changes[8]={-1,-1,-1,-1,2,4,6,8};
unsigned joey_ima_decode_block(const uint8_t *data,unsigned bytes,int16_t *out,unsigned capacity){
    if(!data||!out||bytes!=JOEY_IMA_BLOCK_BYTES||data[2]>88||data[3]||!capacity)return 0;
    if(capacity>JOEY_IMA_BLOCK_FRAMES)capacity=JOEY_IMA_BLOCK_FRAMES;
    int predictor=(int16_t)((unsigned)data[0]|(unsigned)data[1]<<8),index=data[2];unsigned count=1;out[0]=predictor;
    for(unsigned i=0;i<JOEY_IMA_BLOCK_FRAMES-1&&count<capacity;++i){unsigned code=(data[4+i/2]>>(i%2*4))&15;int difference=(steps[index]*(2*(int)(code&7)+1))>>3;
        predictor+=code&8?-difference:difference;if(predictor>32767)predictor=32767;if(predictor<-32768)predictor=-32768;
        index+=changes[code&7];if(index<0)index=0;if(index>88)index=88;out[count++]=(int16_t)predictor;
    }return count;
}
