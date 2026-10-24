#include "original_equip_choice_battle_cases.h"
volatile uint32_t native_equip_choice_battle_test_result[5];
static void equip_choice_battle_tests(void){unsigned initial=checks,matrix=0;DuelCommand c;DuelChoiceOption options[DUEL_OPTION_MAX];for(unsigned i=0;i<DUEL_CARD_COUNT;++i)equip_fixture_defs[i]=duel_card_definitions[i];
    for(unsigned n=0;n<sizeof(original_equip_choice_target_cases)/sizeof(original_equip_choice_target_cases[0]);++n){const unsigned *v=original_equip_choice_target_cases[n];equip_periodic_fixture(v[2],v[1],63);test_game.players[v[2]].monsters[0].position=v[3]?0:2;
check(duel_equip_can_target(&test_game,simple_record(v[0]),v[2],0)==v[4]);++matrix;}
    for(unsigned n=0;n<sizeof(original_seven_completed_stat_cases)/sizeof(original_seven_completed_stat_cases[0]);++n){const unsigned *v=original_seven_completed_stat_cases[n];unsigned side=v[0];equip_periodic_fixture(side,v[1],63);dynamic_equip_attach(1-side,0,simple_record(962),100);test_game.players[1-side].backrow[0].control_used=v[2];
check(duel_attack_value(&test_game,side,0)==5000+(int)v[3]&&duel_defense_value(&test_game,side,0)==1500+(int)v[4]);++matrix;}
    for(unsigned n=0;n<sizeof(original_dragon_sword_stat_cases)/sizeof(original_dragon_sword_stat_cases[0]);++n){const unsigned *v=original_dragon_sword_stat_cases[n];equip_periodic_fixture(0,v[0],63);dynamic_equip_attach(1,0,simple_record(1182),100);
check(duel_attack_value(&test_game,0,0)==5000+(int)v[1]&&duel_defense_value(&test_game,0,0)==1500+(int)v[2]);++matrix;}
    for(unsigned n=0;n<sizeof(original_dragon_sword_mark_cases)/sizeof(original_dragon_sword_mark_cases[0]);++n){const unsigned *v=original_dragon_sword_mark_cases[n];unsigned side=v[0],enemy=1-side;expansion_fixture(side);test_game.cards=equip_fixture_defs;for(unsigned owner=0;owner<2;++owner){unsigned card=owner?550:24;equip_fixture_defs[card]=duel_card_definitions[card];equip_fixture_defs[card].race=v[1+owner];equip_fixture_defs[card].effect=DUEL_E_NONE;equip_fixture_defs[card].name_group=63+owner;equip_fixture_defs[card].attack=equip_fixture_defs[card].defense=5000;test_game.players[owner].monsters[0]=slot(card,owner,0);test_game.players[owner].monsters[0].serial=100+owner;test_game.battle_protect_turn[owner]=test_game.turn;}test_game.next_serial=101;for(unsigned owner=0;owner<2;++owner)if((v[3]&(1u<<owner))&&(v[4]||owner==side))dynamic_equip_attach(owner,0,simple_record(1182),100+owner);if(!v[4])test_game.players[enemy].monsters[0]=empty_slot();test_game.phase=DUEL_BATTLE;
check(command(DUEL_ACTION_ATTACK,0,enemy,v[4]?0:DUEL_NO_SLOT,0,0)==DUEL_OK);for(unsigned owner=0;owner<2;++owner){unsigned existed=owner==side||v[4];
check((test_game.players[owner].monsters[0].card!=0)==(existed&&!(v[5]&(1u<<owner))));
check(test_game.players[owner].grave_count==((v[5]&(1u<<owner))?1:0)&&test_game.players[owner].life==8000);}check(!test_game.battle_effect_destroy_count&&!test_game.battle_pending);++matrix;}
    for(unsigned n=0;n<sizeof(original_dragon_sword_execute_cases)/sizeof(original_dragon_sword_execute_cases[0]);++n){const unsigned *v=original_dragon_sword_execute_cases[n];expansion_fixture(v[0]);test_game.phase=DUEL_BATTLE;test_game.battle_pending=1;for(unsigned side=0;side<2;++side){test_game.players[side].monsters[v[1]]=slot(24,side,0);test_game.players[side].monsters[v[1]].serial=100+side;if(v[2]&(1u<<side))test_game.battle_effect_destroy[test_game.battle_effect_destroy_count++]=(DuelTarget){100+side,side,v[1]};}check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK);for(unsigned side=0;side<2;++side)check(test_game.players[side].grave_count==((v[2]&(1u<<side))?1:0));
check(!test_game.battle_effect_destroy_count);++matrix;}
    for(unsigned i=0;i<DUEL_CARD_COUNT;++i)equip_fixture_defs[i]=duel_card_definitions[i];
    for(unsigned side=0;side<2;++side){unsigned enemy=1-side;
        for(unsigned mode=0;mode<2;++mode){equip_periodic_fixture(side,7,63);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(962);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_EQUIP_STAT&&duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==3&&!test_game.resolving&&!test_game.chain_count&&test_game.players[side].hand_count==1&&!test_game.players[side].backrow[0].card);uint32_t before=duel_state_hash(&test_game);
check(choose(enemy,mode)==DUEL_EFFECT_PENDING&&duel_state_hash(&test_game)==before);
check(choose(side,mode)==DUEL_OK&&!test_game.choice.kind&&!test_game.chain_count&&test_game.players[side].backrow[0].control_used==mode+1&&duel_attack_value(&test_game,side,0)==5000+(mode?0:700)&&duel_defense_value(&test_game,side,0)==1500+(mode?700:0));dynamic_equip_attach(enemy,0,simple_record(962),100);test_game.players[enemy].backrow[0].control_used=mode?1:2;
check(duel_attack_value(&test_game,side,0)==5700&&duel_defense_value(&test_game,side,0)==2200);expansion_place(enemy,1,1154);
check(duel_attack_value(&test_game,side,0)==5000&&duel_defense_value(&test_game,side,0)==1500);test_game.players[enemy].backrow[1].card=0;test_game.reverse_turn=test_game.turn;
check(duel_attack_value(&test_game,side,0)==4300&&duel_defense_value(&test_game,side,0)==800);}
        equip_periodic_fixture(side,7,63);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(962);expansion_place(enemy,1,1078);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK&&test_game.players[side].life==8000);
check(choose(side,DUEL_NO_SLOT)==DUEL_OK&&!test_game.choice.kind&&test_game.players[side].hand_count==1&&test_game.players[side].life==8000);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK&&choose(side,1)==DUEL_OK&&test_game.players[side].life==7500&&test_game.players[side].backrow[0].control_used==2);
        equip_periodic_fixture(side,7,63);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(962);test_game.players[enemy].backrow[0]=slot(simple_record(1079),enemy,2);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK&&test_game.response_player>=2);
check(choose(side,1)==DUEL_OK&&test_game.response_player==enemy&&test_game.chain_count==1&&test_game.chain[0].command.position==1);c=(DuelCommand){enemy,DUEL_ACTION_ACTIVATE_BACKROW,0,side,0};
check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[side].backrow[0].card&&!test_game.choice.kind&&duel_defense_value(&test_game,side,0)==1500);
        /* Destroying the Equip's Warrior in battle does not cancel the
           Dragon's already recorded effect death. */
        expansion_fixture(side);test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(24,side,0);test_game.players[side].monsters[0].serial=100;test_game.players[enemy].monsters[0]=slot(simple_record(0),enemy,0);test_game.players[enemy].monsters[0].serial=101;test_game.next_serial=101;dynamic_equip_attach(side,0,simple_record(1182),100);
check(command(DUEL_ACTION_ATTACK,0,enemy,0,0,0)==DUEL_OK&&!test_game.players[side].monsters[0].card&&!test_game.players[enemy].monsters[0].card&&!test_game.players[side].backrow[0].card&&test_game.players[side].grave_count==2&&test_game.players[enemy].grave_count==1&&test_game.players[side].life==7400);
        /* A normal battle defeat keeps its battle-only Nimble trigger. A
           protected dynamic Dragon is instead destroyed by the Sword effect. */
        for(unsigned protect=0;protect<2;++protect){unsigned nimble=expansion_effect_card(DUEL_E_NIMBLE);expansion_fixture(side);test_game.cards=equip_fixture_defs;equip_fixture_defs[nimble]=duel_card_definitions[nimble];equip_fixture_defs[nimble].race=1;test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(24,side,0);test_game.players[side].monsters[0].serial=100;test_game.players[enemy].monsters[0]=slot(nimble,enemy,0);test_game.players[enemy].monsters[0].serial=101;test_game.next_serial=101;if(protect)test_game.battle_protect_turn[enemy]=test_game.turn;dynamic_equip_attach(side,0,simple_record(1182),100);int difference=2400-duel_attack_value(&test_game,enemy,0);
check(command(DUEL_ACTION_ATTACK,0,enemy,0,0,0)==DUEL_OK&&!test_game.players[enemy].monsters[0].card&&test_game.players[enemy].grave_count==1);
check(test_game.players[enemy].life==(protect?8000:9000-difference));equip_fixture_defs[nimble]=duel_card_definitions[nimble];}
        /* Deferred destruction follows identity, not a reused coordinate. */
        expansion_fixture(side);test_game.phase=DUEL_BATTLE;test_game.battle_pending=1;test_game.players[enemy].monsters[0]=slot(24,enemy,0);test_game.players[enemy].monsters[0].serial=999;test_game.battle_effect_destroy_count=1;test_game.battle_effect_destroy[0]=(DuelTarget){100,enemy,0};
check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK&&test_game.players[enemy].monsters[0].card==24&&!test_game.players[enemy].grave_count&&!test_game.battle_effect_destroy_count);
    }
    unsigned machine=0;for(unsigned i=1;i<DUEL_CARD_COUNT;++i){const DuelCardDef *d=&duel_card_definitions[i];if(d->family==DUEL_MONSTER&&d->subtype==0&&d->race==7&&d->level<=4&&d->attack>=1000){machine=i;break;}}check(machine);uint16_t deck[48];const unsigned extra[8]={962,962,1182,1182,0,0,1079,1154};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<8;++i)deck[40+i]=extra[i]?simple_record(extra[i]):machine;unsigned completed=0,activations=0;
    for(unsigned seed=1;seed<=10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,48,deck,48,seed+6300,seed&1)==DUEL_OK);
check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));if(c.action==DUEL_ACTION_ACTIVATE_HAND){unsigned name=duel_card_definitions[test_game.players[c.player].hand[c.source]].name_group;activations+=name==962||name==1182;}int result=duel_command(&test_game,&c);
check(result==DUEL_OK);if(result!=DUEL_OK)break;
check(owned_cards(&test_game,0)==48&&owned_cards(&test_game,1)==48);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(completed==10&&activations);native_equip_choice_battle_test_result[0]=checks-initial;native_equip_choice_battle_test_result[1]=matrix;native_equip_choice_battle_test_result[2]=completed;native_equip_choice_battle_test_result[3]=activations;
}
