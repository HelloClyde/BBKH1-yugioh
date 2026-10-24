#include "duel_opening.h"
#include "original_opening_cases.h"
volatile uint32_t native_opening_test_result[4];
static unsigned opening_stream[3],opening_calls;
static uint32_t opening_test_random(void *context){return opening_stream[opening_calls++%3];}
static void opening_tests(void){
    unsigned first_check=checks,draws=0,wins=0,losses=0;
    for(unsigned i=0;i<sizeof(original_opening_cases)/sizeof(original_opening_cases[0]);++i){
        DuelOpening o;duel_opening_begin(&o);check(o.phase==DUEL_OPEN_CHOOSE&&o.human==255&&o.enemy==255&&o.first==255&&o.rounds==0);
        o.rounds=original_opening_cases[i].previous;opening_stream[0]=original_opening_cases[i].random1;opening_stream[1]=original_opening_cases[i].random2;opening_stream[2]=original_opening_cases[i].random3;opening_calls=0;
        check(duel_opening_throw(&o,original_opening_cases[i].human,opening_test_random,0)==0);
        check(o.enemy==original_opening_cases[i].enemy&&o.npc_first==original_opening_cases[i].first&&o.rounds==original_opening_cases[i].rounds&&o.outcome==original_opening_cases[i].outcome&&opening_calls==original_opening_cases[i].calls);
        unsigned calls=opening_calls;check(duel_opening_throw(&o,0,opening_test_random,0)!=0&&opening_calls==calls);
        if(o.outcome==DUEL_OPEN_RESULT_DRAW){++draws;check(o.phase==DUEL_OPEN_DRAW&&duel_opening_order(&o,0)!=0);unsigned rounds=o.rounds;check(duel_opening_continue(&o)==0&&o.phase==DUEL_OPEN_CHOOSE&&o.rounds==rounds&&o.human==255&&o.enemy==255&&o.first==255);}
        else if(o.outcome==DUEL_OPEN_RESULT_WIN){++wins;check(o.phase==DUEL_OPEN_PLAYER_ORDER&&duel_opening_order(&o,2)!=0&&o.first==255&&duel_opening_continue(&o)!=0);
            DuelOpening second=o;check(duel_opening_order(&o,0)==0&&o.phase==DUEL_OPEN_READY&&o.first==0);check(duel_opening_order(&second,1)==0&&second.phase==DUEL_OPEN_READY&&second.first==1);}
        else {++losses;check(o.phase==DUEL_OPEN_NPC_ORDER&&duel_opening_order(&o,0)!=0&&o.first==255);check(duel_opening_continue(&o)==0&&o.phase==DUEL_OPEN_READY&&o.first==original_opening_cases[i].first);}
    }
    DuelOpening o;duel_opening_begin(&o);opening_calls=0;
    check(duel_opening_throw(&o,3,opening_test_random,0)!=0&&!opening_calls&&o.phase==DUEL_OPEN_CHOOSE&&o.rounds==0);
    check(duel_opening_throw(&o,0,0,0)!=0&&!opening_calls&&o.phase==DUEL_OPEN_CHOOSE);
    check(duel_opening_order(&o,0)!=0&&duel_opening_continue(&o)!=0&&o.phase==DUEL_OPEN_CHOOSE);
    o.phase=DUEL_OPEN_NPC_ORDER;o.npc_first=2;check(duel_opening_continue(&o)!=0&&o.phase==DUEL_OPEN_NPC_ORDER&&o.first==255);
    check(draws&&wins&&losses);native_opening_test_result[0]=checks-first_check;native_opening_test_result[1]=draws;native_opening_test_result[2]=wins;native_opening_test_result[3]=losses;
}
