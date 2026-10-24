#include "original_lp_mutation_cases.h"
volatile uint32_t native_lp_mutation_test_result[6];
static void lp_recover_fixture(unsigned side,unsigned life,unsigned amount){pair_fixture(side);test_game.players[side].life=life;expansion_place(side,0,1179);test_game.players[side].backrow[0].serial=300;pair_chain(1179,side);test_game.chain[0].source_slot=0;test_game.chain[0].effect_activation=0;test_game.chain[0].target_card=amount;}
static void lp_mutation_tests(void){unsigned initial=checks,matrix=0,reference=0;
    for(unsigned n=0;n<sizeof(original_lp_mutation_cases)/sizeof(original_lp_mutation_cases[0]);++n){const int *v=original_lp_mutation_cases[n];unsigned side=v[0];if(!v[1]){++reference;continue;}if(v[2]>=0){lp_recover_fixture(side,v[1],v[2]);pass_chain();
#ifdef _WIN32
if(test_game.players[side].life!=v[3])fprintf(stderr,"LP row%u side%d life%d delta%d expected%d got%d\n",n,v[0],v[1],v[2],v[3],test_game.players[side].life);
#endif
check(test_game.players[side].life==v[3]);}else if(v[1]){departure_fixture(1-side,77,77);test_game.players[side].monsters[0].card=0;test_game.players[side].life=v[1];test_game.players[1-side].monsters[0].attack_bonus=-v[2]>30000?30000:-v[2]-1700;test_game.players[1-side].monsters[0].temporary_attack=-v[2]>30000?-v[2]-31700:0;check(command(DUEL_ACTION_ATTACK,0,side,DUEL_NO_SLOT,0,0)==DUEL_OK);contact_test_finish();
#ifdef _WIN32
if(test_game.players[side].life!=v[3])fprintf(stderr,"LP row%u side%d life%d delta%d expected%d got%d\n",n,v[0],v[1],v[2],v[3],test_game.players[side].life);
#endif
check(test_game.players[side].life==v[3]);}else{++reference;continue;}++matrix;}
    for(unsigned n=0;n<sizeof(original_lp_recovery_event_cases)/sizeof(original_lp_recovery_event_cases[0]);++n){const int *v=original_lp_recovery_event_cases[n];unsigned side=v[0];/* Sebek snapshots a16-bit damage count; overflow-request vectors remain
         * original-helper evidence until another producer creates that count. */if(v[2]>65535){++reference;continue;}lp_recover_fixture(side,v[1],v[2]);if(v[3]){test_game.players[side].monsters[0]=slot(simple_record(1434),side,v[4]?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK);test_game.players[side].monsters[0].serial=100;}if(v[5])state_prohibition(1-side,1434,0);pass_chain();check(test_game.players[side].life==(v[1]+v[2]>65535?65535:v[1]+v[2]));
#ifdef _WIN32
if(test_game.players[1-side].life!=8000-500*v[7])fprintf(stderr,"LP recovery row%u side%d life%d amount%d present%d hidden%d prohib%d trigger%d got%d\n",n,v[0],v[1],v[2],v[3],v[4],v[5],v[7],test_game.players[1-side].life);
#endif
check(test_game.players[1-side].life==8000-500*v[7]);++matrix;}
    native_lp_mutation_test_result[0]=checks-initial;native_lp_mutation_test_result[1]=matrix;native_lp_mutation_test_result[4]=reference;
}
