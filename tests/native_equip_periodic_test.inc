#include "original_equip_periodic_cases.h"
volatile uint32_t native_equip_periodic_test_result[5];
static void equip_periodic_fixture(unsigned side,unsigned race,unsigned name){expansion_fixture(side);test_game.cards=equip_fixture_defs;equip_fixture_defs[550]=duel_card_definitions[550];equip_fixture_defs[550].race=race;equip_fixture_defs[550].name_group=name;equip_fixture_defs[550].effect=DUEL_E_NONE;equip_fixture_defs[550].attack=5000;equip_fixture_defs[550].defense=1500;test_game.players[side].monsters[0]=slot(550,side,0);test_game.players[side].monsters[0].serial=100;test_game.next_serial=100;}
static void equip_periodic_tests(void){unsigned initial=checks,matrix=0;DuelCommand c;for(unsigned i=0;i<DUEL_CARD_COUNT;++i)equip_fixture_defs[i]=duel_card_definitions[i];
    for(unsigned n=0;n<sizeof(original_equip_timer_target_cases)/sizeof(original_equip_timer_target_cases[0]);++n){const int *v=original_equip_timer_target_cases[n];equip_periodic_fixture(v[3],v[1],v[2]);test_game.players[v[3]].monsters[0].position=v[4]?0:2;
check(duel_equip_can_target(&test_game,simple_record(v[0]),v[3],0)==v[5]);++matrix;}
    for(unsigned n=0;n<sizeof(original_equip_timer_stat_cases)/sizeof(original_equip_timer_stat_cases[0]);++n){const int *v=original_equip_timer_stat_cases[n];unsigned side=v[1],host=1-side;equip_periodic_fixture(host,v[2],63);dynamic_equip_attach(side,0,simple_record(v[0]),100);test_game.players[side].backrow[0].control_used=v[3];
check(duel_attack_value(&test_game,host,0)==5000+v[4]&&duel_defense_value(&test_game,host,0)==1500);++matrix;}
    for(unsigned n=0;n<sizeof(original_equip_timer_counter_cases)/sizeof(original_equip_timer_counter_cases[0]);++n){const int *v=original_equip_timer_counter_cases[n];unsigned side=v[0],index=v[1];equip_periodic_fixture(side,18,63);dynamic_equip_attach(side,index,simple_record(1060),100);test_game.players[side].backrow[index].control_used=v[2];for(unsigned i=0;i<(unsigned)v[3];++i){test_game.phase=DUEL_DRAW;
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);}check(test_game.players[side].backrow[index].control_used==v[4]&&duel_attack_value(&test_game,side,0)==5700-200*v[4]);++matrix;}
    for(unsigned n=0;n<sizeof(original_equip_timer_scan_cases)/sizeof(original_equip_timer_scan_cases[0]);++n){const int *v=original_equip_timer_scan_cases[n];unsigned side=v[0];equip_periodic_fixture(side,18,63);dynamic_equip_attach(side,0,simple_record(v[1]),100);test_game.players[side].backrow[0].position=v[2]?0:2;test_game.phase=DUEL_DRAW;
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);
check(test_game.players[side].backrow[0].control_used==v[3]);++matrix;}
    for(unsigned n=0;n<sizeof(original_equip_mask_burn_cases)/sizeof(original_equip_mask_burn_cases[0]);++n){const int *v=original_equip_mask_burn_cases[n];unsigned side=v[0],host=v[3];equip_periodic_fixture(host,18,63);test_game.active=side;dynamic_equip_attach(side,v[1],simple_record(1419),v[4]?100:0);test_game.players[side].backrow[v[1]].effect_active=v[2];test_game.phase=DUEL_DRAW;
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);
check(test_game.players[host].life==8000-500*v[5]&&test_game.players[1-host].life==8000);++matrix;}
    for(unsigned n=0;n<sizeof(original_cyclon_pierce_cases)/sizeof(original_cyclon_pierce_cases[0]);++n){const int *v=original_cyclon_pierce_cases[n];unsigned side=v[0],enemy=1-side,host=simple_record(1339),copies=(v[1]&1)+((v[1]>>1)&1);expansion_fixture(side);test_game.cards=equip_fixture_defs;equip_fixture_defs[550]=duel_card_definitions[550];equip_fixture_defs[550].defense=1000;equip_fixture_defs[host].attack=1000+v[2]-300*copies;test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(host,side,0);test_game.players[side].monsters[0].serial=100;test_game.players[enemy].monsters[0]=slot(550,enemy,1);test_game.players[enemy].monsters[0].serial=101;test_game.next_serial=101;for(unsigned owner=0;owner<2;++owner)if(v[1]&(1u<<owner))dynamic_equip_attach(owner,0,simple_record(1540),100);
check(command(DUEL_ACTION_ATTACK,0,enemy,0,0,0)==DUEL_OK);
check(test_game.players[side].life==8000-v[4]&&test_game.players[enemy].life==8000-v[3]);++matrix;}
    for(unsigned n=0;n<sizeof(original_cyclon_stat_cases)/sizeof(original_cyclon_stat_cases[0]);++n){const int *v=original_cyclon_stat_cases[n];equip_periodic_fixture(0,v[0],v[1]);dynamic_equip_attach(1,0,simple_record(1540),100);
check(duel_attack_value(&test_game,0,0)==5000+v[2]&&duel_defense_value(&test_game,0,0)==1500+v[3]);++matrix;}
    for(unsigned i=0;i<DUEL_CARD_COUNT;++i)equip_fixture_defs[i]=duel_card_definitions[i];
    for(unsigned side=0;side<2;++side){unsigned enemy=1-side;
        for(unsigned name=1046;name<=1060;name+=14){equip_periodic_fixture(side,18,63);test_game.active=enemy;dynamic_equip_attach(enemy,0,simple_record(name),100);test_game.phase=DUEL_DRAW;
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK&&test_game.players[enemy].backrow[0].control_used==1);test_game.active=side;test_game.phase=DUEL_DRAW;
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK&&test_game.players[enemy].backrow[0].control_used==1);test_game.active=enemy;expansion_place(side,1,1154);test_game.phase=DUEL_DRAW;
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK&&test_game.players[enemy].backrow[0].control_used==2&&duel_attack_value(&test_game,side,0)==5000);test_game.players[side].backrow[1].card=0;
check(duel_attack_value(&test_game,side,0)==(name==1046?4400:5300));}
        equip_periodic_fixture(side,18,63);dynamic_equip_attach(side,0,simple_record(1060),100);dynamic_equip_attach(enemy,0,simple_record(1046),100);test_game.players[side].backrow[0].control_used=2;test_game.players[enemy].backrow[0].control_used=3;
check(duel_attack_value(&test_game,side,0)==4400&&duel_defense_value(&test_game,side,0)==1500);test_game.reverse_turn=test_game.turn;
check(duel_attack_value(&test_game,side,0)==5600&&duel_defense_value(&test_game,side,0)==1500);test_game.reverse_turn=0;test_game.players[enemy].backrow[0].card=0;
check(duel_attack_value(&test_game,side,0)==5300);
        equip_periodic_fixture(side,7,63);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1046);uint32_t invalid_before=duel_state_hash(&test_game);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==invalid_before);
        /* Common public attachment, source removal and face-down lifecycle. */
        equip_periodic_fixture(side,18,63);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1060);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK&&duel_attack_value(&test_game,side,0)==5700);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1079);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK&&!test_game.players[side].backrow[0].card&&duel_attack_value(&test_game,side,0)==5000);
        equip_periodic_fixture(side,18,63);test_game.active=enemy;dynamic_equip_attach(enemy,0,simple_record(1419),100);test_game.phase=DUEL_BATTLE;test_game.active=side;uint32_t before=duel_state_hash(&test_game);
check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);expansion_place(side,1,1154);
check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_OK);test_game.active=enemy;test_game.phase=DUEL_DRAW;
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK&&test_game.players[side].life==8000);test_game.players[side].backrow[1].card=0;test_game.phase=DUEL_DRAW;test_game.players[side].life=500;
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK&&test_game.winner==(int)enemy&&!test_game.players[side].life);
        equip_periodic_fixture(side,18,63);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1419);
check(duel_ai_decide(&test_game,side,&c)&&c.action!=DUEL_ACTION_ACTIVATE_HAND);test_game.players[enemy].monsters[0]=slot(24,enemy,0);
check(duel_ai_decide(&test_game,side,&c)&&c.action==DUEL_ACTION_ACTIVATE_HAND&&c.target_player==enemy&&duel_command(&test_game,&c)==DUEL_OK);
    }
    uint16_t deck[48];const unsigned extra[8]={1046,1060,1419,1540,1339,1339,1079,1154};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<8;++i)deck[40+i]=simple_record(extra[i]);unsigned completed=0,activations=0;
    for(unsigned seed=1;seed<=10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,48,deck,48,seed+6100,seed&1)==DUEL_OK);
check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));if(c.action==DUEL_ACTION_ACTIVATE_HAND){unsigned name=duel_card_definitions[test_game.players[c.player].hand[c.source]].name_group;activations+=name==1046||name==1060||name==1419||name==1540;}int result=duel_command(&test_game,&c);
check(result==DUEL_OK);if(result!=DUEL_OK)break;
check(owned_cards(&test_game,0)==48&&owned_cards(&test_game,1)==48);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(completed==10&&activations);native_equip_periodic_test_result[0]=checks-initial;native_equip_periodic_test_result[1]=matrix;native_equip_periodic_test_result[2]=completed;native_equip_periodic_test_result[3]=activations;
}
