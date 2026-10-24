#include "original_global_state_cases.h"
volatile uint32_t native_global_state_test_result[6];
static void state_prohibition(unsigned owner,unsigned declared,unsigned exempt){expansion_place(owner,0,1162);DuelSlot *s=&test_game.players[owner].backrow[0];s->serial=300;s->link_serial=simple_record(declared);test_game.prohibitions[0]=(DuelProhibition){300,{100},exempt};}
static void state_declare(unsigned side,unsigned record){check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_ACTIVATION_PARAMETER);unsigned page=(record-1)/128;for(unsigned i=0;i<page;++i)check(choose(side,254)==DUEL_OK);
        check(choose(side,record-page*128)==DUEL_OK);pass_chain();}
static void global_state_tests(void){unsigned initial=checks,matrix=0,reference=sizeof(original_cold_activity_cases)/sizeof(original_cold_activity_cases[0])+sizeof(original_trait_reveal_cases)/sizeof(original_trait_reveal_cases[0]);
    for(unsigned n=0;n<sizeof(original_prohibition_cases)/sizeof(original_prohibition_cases[0]);++n){const int *v=original_prohibition_cases[n];if(!v[6]){++reference;continue;}expansion_fixture(v[0]);state_prohibition(v[1],v[4],v[7]);DuelSlot *s=&test_game.players[v[1]].backrow[0];s->position=v[2]==1?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK;if(v[3]==2)s->effect_active=0;if(v[3]==32)expansion_place(1-v[1],4,1154);
        check(duel_card_prohibited(&test_game,simple_record(v[5]),v[6]<0?0:100)==v[8]);++matrix;}
    for(unsigned n=0;n<sizeof(original_cold_gate_cases)/sizeof(original_cold_gate_cases[0]);++n){const int *v=original_cold_gate_cases[n];expansion_fixture(v[0]);test_game.phase=v[1]==2?DUEL_MAIN1:v[1]==4?DUEL_MAIN2:DUEL_STANDBY;test_game.main1_action_used=v[2];test_game.players[v[0]].hand[0]=simple_record(1312);test_game.players[v[0]].hand_count=1;int r=command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0);
        check((r==DUEL_OK)==v[3]);++matrix;}
    for(unsigned n=0;n<sizeof(original_cold_expiry_cases)/sizeof(original_cold_expiry_cases[0]);++n){const int *v=original_cold_expiry_cases[n];expansion_fixture(1-v[1]);test_game.turn=v[2];test_game.cold_wave_until=v[3]+1;test_game.cold_wave_actor=v[0];test_game.players[v[1]].skip_next_draw=v[4];test_game.players[0].hand_count=test_game.players[1].hand_count=0;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);
        check((test_game.cold_wave_until==0)==v[5]);++matrix;}
    for(unsigned n=0;n<sizeof(original_glass_gate_cases)/sizeof(original_glass_gate_cases[0]);++n){const int *v=original_glass_gate_cases[n];if(v[2]){++reference;continue;}expansion_fixture(v[0]);test_game.players[v[0]].backrow[0]=slot(simple_record(1136),v[0],DUEL_FACEDOWN_DEFENSE);test_game.players[v[0]].backrow[0].set_turn=1;test_game.window.kind=v[1]==26?DUEL_WINDOW_EQUIP:DUEL_WINDOW_NONE;
        check((command(DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0)==DUEL_OK)==v[3]);++matrix;}
    for(unsigned n=0;n<sizeof(original_light_position_cases)/sizeof(original_light_position_cases[0]);++n){const int *v=original_light_position_cases[n];if(!v[1]){++reference;continue;}expansion_fixture(v[0]);test_game.players[v[0]].monsters[0]=slot(simple_record(77),v[0],v[1]==1?DUEL_FACEDOWN_DEFENSE:v[1]==2?DUEL_FACEUP_ATTACK:DUEL_FACEUP_DEFENSE);if(v[2])expansion_place(1-v[0],0,1151);if(v[3])expansion_place(v[0],4,1033);duel_change_monster_position(&test_game,v[0],0,DUEL_FACEDOWN_DEFENSE);unsigned expected=v[1]==1?DUEL_FACEDOWN_DEFENSE:v[2]&&!v[3]?DUEL_FACEUP_DEFENSE:DUEL_FACEDOWN_DEFENSE;
        check(test_game.players[v[0]].monsters[0].position==expected);++matrix;}
    for(unsigned side=0;side<2;++side){DuelCommand c;DuelChoiceOption options[DUEL_OPTION_MAX];
        /* Existing physical copies survive the declaration, including a
         * matching card which entered during the chain. New copies do not. */
        expansion_fixture(side);test_game.players[side].monsters[0]=slot(simple_record(77),side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[0].serial=100;test_game.players[side].hand[0]=simple_record(1162);test_game.players[side].hand_count=1;state_declare(side,simple_record(77));
        check(duel_card_prohibited(&test_game,simple_record(77),0)&&!duel_card_prohibited(&test_game,simple_record(77),100));test_game.players[side].monsters[0].serial=101;test_game.phase=DUEL_BATTLE;
        check(command(DUEL_ACTION_ATTACK,0,1-side,DUEL_NO_SLOT,0,0)==DUEL_NOT_ALLOWED);test_game.players[side].backrow[0]=empty_slot();
        check(command(DUEL_ACTION_ATTACK,0,1-side,DUEL_NO_SLOT,0,0)==DUEL_OK);pass_chain();
        expansion_fixture(side);test_game.players[side].hand[0]=simple_record(1162);test_game.players[side].hand_count=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
        check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==130);
        check(choose(side,253)==DUEL_BAD_TARGET);for(unsigned page=0;page<8;++page)check(choose(side,254)==DUEL_OK);
        check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==92);
        check(choose(side,254)==DUEL_BAD_TARGET&&choose(side,DUEL_NO_SLOT)==DUEL_OK&&!test_game.chain_count&&test_game.players[side].hand_count==1);
        /* The original refreshes existing-copy exemptions when a negated
         * Prohibition resumes after Imperial Order leaves. */
        expansion_fixture(side);expansion_place(1-side,4,1154);test_game.players[side].hand[0]=simple_record(1162);test_game.players[side].hand_count=1;state_declare(side,simple_record(77));test_game.players[side].monsters[0]=slot(simple_record(77),side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[0].serial=101;check(!duel_card_prohibited(&test_game,simple_record(77),0));test_game.players[1-side].backrow[4]=empty_slot();c=(DuelCommand){side,DUEL_ACTION_PHASE,0,0,DUEL_MAIN2};check(duel_command(&test_game,&c)==DUEL_OK);check(duel_card_prohibited(&test_game,simple_record(77),0)&&!duel_card_prohibited(&test_game,simple_record(77),101));
        /* A resolved Armored Glass responds to attachment, then suppresses
         * both old and later equipment for the rest of the turn. */
        trait_fixture(side,77,2,0);test_game.players[side].hand[0]=simple_record(1550);test_game.players[side].hand_count=1;test_game.players[1-side].backrow[0]=slot(simple_record(1136),1-side,DUEL_FACEDOWN_DEFENSE);test_game.players[1-side].backrow[0].set_turn=1;int base=duel_attack_value(&test_game,side,0);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK);
        check(test_game.window.kind==DUEL_WINDOW_EQUIP&&test_game.response_player==1-side&&duel_race(&test_game,side,0)==1);c=(DuelCommand){1-side,DUEL_ACTION_ACTIVATE_BACKROW,0};
        check(duel_command(&test_game,&c)==DUEL_OK);pass_chain();
        check(duel_race(&test_game,side,0)==15&&duel_attack_value(&test_game,side,0)==base);test_game.players[side].backrow[1]=slot(simple_record(657),side,DUEL_FACEDOWN_DEFENSE);test_game.players[side].backrow[1].set_turn=1;
        check(command(DUEL_ACTION_ACTIVATE_BACKROW,1,side,0,0,0)==DUEL_OK);pass_chain();
        check(test_game.players[side].backrow[1].effect_active&&duel_attack_value(&test_game,side,0)==base);test_game.turn++;
        check(duel_attack_value(&test_game,side,0)==base+800);
        /* Intervention changes the normal Set procedure to a face-up
         * Defense Normal Summon, with ordinary summon response timing. */
        expansion_fixture(side);expansion_place(1-side,0,1151);test_game.players[side].hand[0]=simple_record(77);test_game.players[side].hand_count=1;
        check(command(DUEL_ACTION_SET_MONSTER,0,0,0,0,0)==DUEL_OK);pass_chain();
        check(test_game.players[side].monsters[0].position==DUEL_FACEUP_DEFENSE&&test_game.players[side].normal_summon_used&&test_game.players[side].monsters[0].normal_summon_marker);
        /* The lock persists without its original Spell card and blocks
         * setting and activation, while ordinary monsters remain usable. */
        expansion_fixture(side);test_game.players[side].hand[0]=simple_record(1312);test_game.players[side].hand_count=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);pass_chain();
        check(test_game.cold_wave_until==test_game.turn+2&&test_game.players[side].grave_count==1);test_game.players[side].hand[0]=simple_record(1079);test_game.players[side].hand_count=1;
        check(command(DUEL_ACTION_SET_BACKROW,0,0,0,0,0)==DUEL_NOT_ALLOWED&&duel_card_activation_blocked(&test_game,simple_record(1079)));test_game.players[side].hand[0]=simple_record(77);
        check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK);pass_chain();
    }
    uint16_t deck[52];const unsigned extra[12]={1145,1146,1448,1550,1136,1151,1162,1312,333,1329,657,1079};unsigned completed=0,plays=0;for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<12;++i)deck[40+i]=simple_record(extra[i]);
    for(unsigned seed=1;seed<=10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,52,deck,52,seed+7800,seed&1)==DUEL_OK);check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK&&duel_field_setup(&test_game,duel_field_rules,DUEL_FIELD_RULE_COUNT)==DUEL_OK&&duel_suppression_setup(&test_game,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT)==DUEL_OK);unsigned actions=0;DuelCommand c;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));plays+=c.action==DUEL_ACTION_ACTIVATE_HAND||c.action==DUEL_ACTION_ACTIVATE_BACKROW;int r=duel_command(&test_game,&c);check(r==DUEL_OK);if(r!=DUEL_OK)break;check(owned_cards(&test_game,0)==52&&owned_cards(&test_game,1)==52);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(completed==10);native_global_state_test_result[2]=completed;native_global_state_test_result[3]=plays;
    native_global_state_test_result[0]=checks-initial;native_global_state_test_result[1]=matrix;native_global_state_test_result[4]=reference;
}
