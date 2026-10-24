#include "original_time_wizard_cases.h"
volatile uint32_t native_time_wizard_test_result[5];
static void time_wizard_fixture(unsigned side){expansion_fixture(side);test_game.players[side].monsters[0]=slot(simple_record(15),side,0);test_game.players[1-side].monsters[0]=slot(24,1-side,0);test_game.random_state=2;}
static void time_wizard_tests(void){unsigned initial=checks,matrix=0;DuelCommand c;DuelChoiceOption options[DUEL_OPTION_MAX];
    for(unsigned n=0;n<sizeof(original_time_wizard_gate_cases)/sizeof(original_time_wizard_gate_cases[0]);++n){const unsigned *v=original_time_wizard_gate_cases[n];time_wizard_fixture(v[0]);test_game.players[v[0]].monsters[0].ignition_turn=v[1]?0:test_game.turn;test_game.players[1-v[0]].monsters[0]=empty_slot();for(unsigned i=0;i<v[2];++i)test_game.players[1-v[0]].monsters[i]=slot(24,1-v[0],i%3);uint32_t before=duel_state_hash(&test_game);int result=command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0);
check((result==DUEL_OK)==v[3]);if(!v[3])check(duel_state_hash(&test_game)==before);++matrix;}
    const int powers[5]={500,1700,3000,1,0};
    for(unsigned n=0;n<sizeof(original_time_wizard_outcome_cases)/sizeof(original_time_wizard_outcome_cases[0]);++n){const unsigned *v=original_time_wizard_outcome_cases[n];unsigned side=v[0],enemy=1-side;time_wizard_fixture(side);for(unsigned i=1;i<v[1];++i){/* The reference stubs supplied ATK values; use face-up slots so injected temporary stats describe a reachable native state. */test_game.players[side].monsters[i]=slot(24,side,i%2);test_game.players[side].monsters[i].temporary_attack=powers[i]-1700;}test_game.players[enemy].monsters[1]=slot(24,enemy,2);test_game.random_state=v[3]?1:2;unsigned own=owned_cards(&test_game,side),opp=owned_cards(&test_game,enemy);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,v[2],0)==DUEL_OK);
check(test_game.players[side].life==8000-(int)v[5]&&test_game.players[enemy].life==8000&&test_game.random_result.success==v[4]);
check(test_game.players[side].grave_count==(v[4]?0:v[1])&&test_game.players[enemy].grave_count==(v[4]?2:0)&&owned_cards(&test_game,side)==own&&owned_cards(&test_game,enemy)==opp);if(v[4])check(test_game.players[side].monsters[0].ignition_turn==test_game.turn);++matrix;}
    for(unsigned n=0;n<sizeof(original_dark_sage_offer_cases)/sizeof(original_dark_sage_offer_cases[0]);++n){const unsigned *v=original_dark_sage_offer_cases[n];unsigned side=v[0];time_wizard_fixture(side);if(v[1]){test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1202);}if(v[5])test_game.players[side].monsters[1]=slot(simple_record(34),side,0);if(!v[2])for(unsigned i=1;i<5;++i)if(!test_game.players[side].monsters[i].card)test_game.players[side].monsters[i]=slot(24,side,0);if(!v[3])test_game.players[side].monsters[4]=slot(simple_record(1510),side,0);if(v[4])expansion_place(1-side,0,1418);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&((test_game.choice.kind==DUEL_CHOICE_DARK_SAGE_OFFER)==v[6]));++matrix;}
    /* These selectors are const. Reuse their fixed empty-board frame and
       change only the single candidate, instead of clearing an entire duel
       for every one of the original resource records. */
    unsigned selector_side=2,selector_pile=2;
    for(unsigned n=0;n<sizeof(original_dark_sage_select_cases)/sizeof(original_dark_sage_select_cases[0]);++n){const unsigned *v=original_dark_sage_select_cases[n];unsigned side=v[0];if(selector_side!=side||selector_pile!=v[1]){expansion_fixture(side);test_game.players[side].deck_count=test_game.players[side].hand_count=0;test_game.pile_summon_task=(DuelPileSummonTask){simple_record(15),0,side,side,4,1,0};test_game.choice=(DuelTrigger){DUEL_CHOICE_PILE_SUMMON,side,simple_record(15),0};selector_side=side;selector_pile=v[1];}if(v[1]){test_game.players[side].deck_count=1;test_game.players[side].deck[0]=v[2];}else{test_game.players[side].hand_count=1;test_game.players[side].hand[0]=v[2];}unsigned count=duel_choice_options(&test_game,options,DUEL_OPTION_MAX);
check(count==v[3]);if(v[3])check(options[0].card==v[2]&&options[0].index==(v[1]?DUEL_HAND_MAX:0));++matrix;}
    selector_side=2;
    for(unsigned n=0;n<sizeof(original_dark_sage_spell_cases)/sizeof(original_dark_sage_spell_cases[0]);++n){const unsigned *v=original_dark_sage_spell_cases[n];unsigned side=v[0];if(selector_side!=side){expansion_fixture(side);test_game.players[side].deck_count=1;test_game.choice=(DuelTrigger){DUEL_CHOICE_DARK_SAGE_SPELL,side,simple_record(1202),0};selector_side=side;}test_game.players[side].deck[0]=v[1];unsigned count=duel_choice_options(&test_game,options,DUEL_OPTION_MAX);
check(count==v[2]);if(v[2])check(options[0].card==v[1]&&options[0].index==0);++matrix;}
    for(unsigned n=0;n<sizeof(original_dark_sage_revive_cases)/sizeof(original_dark_sage_revive_cases[0]);++n){const unsigned *v=original_dark_sage_revive_cases[n];expansion_fixture(v[0]);test_game.players[v[0]].grave_count=1;test_game.players[v[0]].grave[0]=simple_record(1202);test_game.players[v[0]].grave_proper[0]=v[1];
check(duel_can_revive(&test_game,v[0],0)==v[2]);++matrix;}
    for(unsigned side=0;side<2;++side)for(unsigned pile=0;pile<2;++pile)for(unsigned position=0;position<2;++position)for(unsigned art=0;art<2;++art){time_wizard_fixture(side);DuelPlayer *p=&test_game.players[side];p->monsters[1]=slot(original_dark_magician_records[art],side,0);p->hand_count=pile?0:1;p->hand[0]=pile?0:simple_record(1202);p->deck_count=pile?4:3;p->deck[0]=simple_record(1008);p->deck[1]=simple_record(1079);p->deck[2]=24;if(pile)p->deck[3]=simple_record(1202);unsigned own=owned_cards(&test_game,side);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_DARK_SAGE_OFFER&&duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==2);
check(choose(side,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_DARK_SAGE_TRIBUTE&&duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==1&&options[0].card==original_dark_magician_records[art]);
check(choose(side,1)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_PILE_SUMMON&&!p->monsters[1].card&&p->grave[0]==original_dark_magician_records[art]);
check(choose(side,pile?DUEL_HAND_MAX+3:0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_PILE_SUMMON_POSITION);
check(choose(side,position)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_DARK_SAGE_SPELL&&p->monsters[1].card==simple_record(1202)&&p->monsters[1].position==position&&p->monsters[1].properly_summoned&&p->monsters[1].special_summon_marker&&!p->normal_summon_used);unsigned count=duel_choice_options(&test_game,options,DUEL_OPTION_MAX),index=DUEL_NO_SLOT;
check(count==2);for(unsigned i=0;i<count;++i)if(options[i].card==simple_record(1008))index=options[i].index;
check(index!=DUEL_NO_SLOT&&choose(side,index)==DUEL_OK&&!test_game.choice.kind&&!test_game.resolving&&!test_game.chain_count&&p->hand_count==1&&p->hand[0]==simple_record(1008)&&p->deck_count==2&&owned_cards(&test_game,side)==own);}
    for(unsigned side=0;side<2;++side){time_wizard_fixture(side);test_game.players[side].monsters[1]=slot(simple_record(34),side,0);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1202);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&choose(side,DUEL_NO_SLOT)==DUEL_OK&&!test_game.choice.kind&&test_game.players[side].monsters[1].card==simple_record(34));uint32_t before=duel_state_hash(&test_game);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);test_game.turn+=2;test_game.players[1-side].monsters[0]=slot(24,1-side,0);test_game.random_state=2;
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK);
        time_wizard_fixture(side);test_game.players[side].life=250;test_game.random_state=1;
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&test_game.winner==(int)(1-side)&&!test_game.players[side].life);
        expansion_fixture(side);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1202);before=duel_state_hash(&test_game);
check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
check(command(DUEL_ACTION_SET_MONSTER,0,0,0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
    }
    for(unsigned side=0;side<2;++side){unsigned enemy=1-side;
        time_wizard_fixture(side);test_game.players[side].monsters[1]=slot(simple_record(34),side,2);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1202);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&!test_game.choice.kind);
        time_wizard_fixture(side);test_game.players[side].monsters[1]=slot(simple_record(34),side,0);test_game.players[side].monsters[2]=slot(original_dark_magician_records[1],enemy,2);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1202);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&choose(side,0)==DUEL_OK&&duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==2);
check(choose(side,2)==DUEL_OK&&choose(side,0)==DUEL_OK&&choose(side,0)==DUEL_OK&&!test_game.choice.kind&&test_game.players[side].monsters[2].card==simple_record(1202)&&test_game.players[enemy].grave_count==2&&test_game.players[enemy].grave[1]==original_dark_magician_records[1]);
        time_wizard_fixture(side);test_game.players[side].monsters[1]=slot(simple_record(34),side,0);test_game.players[side].monsters[2]=slot(simple_record(1107),side,0);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1202);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&choose(side,0)==DUEL_OK&&choose(side,1)==DUEL_OK&&choose(side,0)==DUEL_OK&&choose(side,0)==DUEL_OK);
check(test_game.players[side].banished_count==1&&test_game.players[side].banished[0]==simple_record(34)&&test_game.players[side].monsters[1].card==simple_record(1202)&&test_game.players[side].monsters[1].properly_summoned);
        time_wizard_fixture(side);test_game.players[side].monsters[1]=slot(simple_record(34),side,0);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1202);test_game.players[enemy].monsters[0]=slot(simple_record(1510),enemy,0);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_DARK_SAGE_OFFER&&!test_game.players[enemy].monsters[0].card);
        time_wizard_fixture(side);test_game.players[enemy].backrow[0]=slot(simple_record(939),enemy,2);
check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,0)==DUEL_OK&&test_game.response_player==enemy);c=(DuelCommand){enemy,DUEL_ACTION_ACTIVATE_BACKROW,0,side,0};
check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[side].monsters[0].card&&!test_game.players[enemy].monsters[0].card&&test_game.players[side].life==7500&&test_game.players[enemy].life==7500&&!test_game.chain_count);
    }
    uint16_t deck[48];const unsigned extra[8]={15,15,15,1202,34,34,1008,1079};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<8;++i)deck[40+i]=simple_record(extra[i]);unsigned completed=0,activations=0;
    for(unsigned seed=1;seed<=10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,48,deck,48,seed+6200,seed&1)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));activations+=c.action==DUEL_ACTION_ACTIVATE_MONSTER&&test_game.players[c.player].monsters[c.source].card==simple_record(15);int result=duel_command(&test_game,&c);
check(result==DUEL_OK);if(result!=DUEL_OK)break;
check(owned_cards(&test_game,0)==48&&owned_cards(&test_game,1)==48);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(completed==10&&activations);native_time_wizard_test_result[0]=checks-initial;native_time_wizard_test_result[1]=matrix;native_time_wizard_test_result[2]=completed;native_time_wizard_test_result[3]=activations;
}
