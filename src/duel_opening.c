/* Native local pre-duel rules. No scene objects, original addresses or CPU
 * instructions are carried into the runtime. Return0 on success,1 on error. */
#include "duel_opening.h"
void duel_opening_begin(DuelOpening *o){if(o)*o=(DuelOpening){0,DUEL_OPEN_CHOOSE,255,255,255,255,255};}
int duel_opening_throw(DuelOpening *o,unsigned hand,DuelOpeningRandom random,void *context){
    if(!o||!random||hand>2||o->phase!=DUEL_OPEN_CHOOSE)return 1;
    unsigned enemy=random(context)%3;
    /* The original permits a first tie. On later rounds it rerolls one
     * matching candidate once; the replacement can still tie. */
    if(o->rounds&&enemy==hand)enemy=random(context)%3;
    unsigned npc_first=random(context)&1,outcome=(enemy+3-hand)%3;
    o->human=hand;o->enemy=enemy;o->outcome=outcome;o->npc_first=npc_first;o->first=255;
    if(o->rounds!=0xffffffffu)++o->rounds;
    o->phase=outcome==DUEL_OPEN_RESULT_DRAW?DUEL_OPEN_DRAW:outcome==DUEL_OPEN_RESULT_WIN?DUEL_OPEN_PLAYER_ORDER:DUEL_OPEN_NPC_ORDER;return 0;
}
int duel_opening_continue(DuelOpening *o){
    if(!o)return 1;
    if(o->phase==DUEL_OPEN_DRAW){o->phase=DUEL_OPEN_CHOOSE;o->human=o->enemy=o->outcome=o->first=o->npc_first=255;return 0;}
    if(o->phase==DUEL_OPEN_NPC_ORDER&&o->npc_first<2){o->first=o->npc_first;o->phase=DUEL_OPEN_READY;return 0;}
    return 1;
}
int duel_opening_order(DuelOpening *o,unsigned first){if(!o||o->phase!=DUEL_OPEN_PLAYER_ORDER||first>1)return 1;o->first=first;o->phase=DUEL_OPEN_READY;return 0;}
