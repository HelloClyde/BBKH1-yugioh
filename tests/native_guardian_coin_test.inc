#include "original_guardian_coin_cases.h"
volatile uint32_t native_guardian_coin_test_result[5];
static void guardian_fixture(unsigned side){expansion_fixture(side);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(373);for(unsigned i=0;i<3;++i)test_game.players[side].monsters[i]=slot(simple_record(370+i),side,0);}
static void guardian_coin_tests(void){unsigned initial=checks,matrix=0;DuelCommand c;DuelChoiceOption options[DUEL_OPTION_MAX];
    for(unsigned n=0;n<sizeof(original_guardian_gate_cases)/sizeof(original_guardian_gate_cases[0]);++n){const unsigned *v=original_guardian_gate_cases[n];unsigned side=v[0];expansion_fixture(side);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(373);for(unsigned i=0;i<5;++i){unsigned name=i<3&&(v[1]&(1u<<i))?370+i:v[5]?24:0;if(name)test_game.players[side].monsters[i]=slot(simple_record(name),side,i<3?(v[2+i]==1?2:v[2+i]==2?0:1):0);}if(v[6])expansion_place(1-side,0,1418);uint32_t before=duel_state_hash(&test_game);int result=command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0);
check((result==DUEL_OK)==v[7]);if(!v[7])check(duel_state_hash(&test_game)==before);else check(test_game.choice.kind==DUEL_CHOICE_SPECIAL_HAND_COST&&test_game.special_hand_task.remaining==3&&duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==3);++matrix;}
    for(unsigned side=0;side<2;++side){unsigned enemy=1-side;
        /* Components are three different names, and choices refer to field
           slots. Full fields and face-down components are valid. */
        for(unsigned position=0;position<3;++position)for(unsigned first=0;first<3;++first)for(unsigned second=0;second<3;++second)if(first!=second){unsigned third=3-first-second;guardian_fixture(side);for(unsigned i=0;i<3;++i)test_game.players[side].monsters[i].position=position;test_game.players[side].monsters[3]=slot(simple_record(370),side,2);test_game.players[side].monsters[4]=slot(550,side,0);unsigned owned=owned_cards(&test_game,side);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
check(choose(side,first)==DUEL_OK&&test_game.players[side].grave_count==1);
check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==2+(first!=0));uint32_t before=duel_state_hash(&test_game);
check(choose(side,first)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);if(first==0)check(choose(side,3)==DUEL_BAD_TARGET);
check(choose(side,second)==DUEL_OK&&choose(side,third)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_SPECIAL_HAND_POSITION);
check(choose(side,1)==DUEL_OK&&test_game.players[side].monsters[0].card==simple_record(373)&&test_game.players[side].monsters[0].position==1&&test_game.players[side].monsters[0].properly_summoned&&test_game.players[side].monsters[0].special_summon_marker&&!test_game.players[side].normal_summon_used&&test_game.players[side].grave_count==3&&owned_cards(&test_game,side)==owned);}
        guardian_fixture(side);test_game.players[side].monsters[1].card=simple_record(370);uint32_t before=duel_state_hash(&test_game);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
        guardian_fixture(side);before=duel_state_hash(&test_game);
check(command(DUEL_ACTION_SUMMON,0,0,3,0,7)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
check(command(DUEL_ACTION_SET_MONSTER,0,0,3,0,7)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
        guardian_fixture(side);test_game.players[enemy].monsters[0]=slot(simple_record(1510),enemy,0);before=duel_state_hash(&test_game);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
        guardian_fixture(side);test_game.players[side].monsters[1].owner=enemy;expansion_place(enemy,0,1078);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&choose(side,0)==DUEL_OK&&choose(side,1)==DUEL_OK&&choose(side,2)==DUEL_OK&&choose(side,0)==DUEL_OK);
check(test_game.players[side].grave_count==2&&test_game.players[enemy].grave_count==1&&test_game.players[enemy].grave[0]==simple_record(371)&&test_game.players[side].life==7500);
        guardian_fixture(side);test_game.players[enemy].backrow[0]=slot(simple_record(1028),enemy,2);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&choose(side,0)==DUEL_OK&&choose(side,1)==DUEL_OK&&choose(side,2)==DUEL_OK&&choose(side,0)==DUEL_OK&&test_game.window.kind==DUEL_WINDOW_SUMMON_NEGATION);c=(DuelCommand){enemy,DUEL_ACTION_ACTIVATE_BACKROW,0};
check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[side].monsters[0].card&&test_game.players[side].grave_count==4&&!test_game.players[side].grave_proper[3]&&!duel_can_revive(&test_game,side,3));
        for(unsigned proper=0;proper<2;++proper){expansion_fixture(side);test_game.players[side].grave_count=1;test_game.players[side].grave[0]=simple_record(373);test_game.players[side].grave_proper[0]=proper;
check(duel_can_revive(&test_game,side,0)==proper);}
    }
    for(unsigned n=0;n<sizeof(original_jirai_coin_cases)/sizeof(original_jirai_coin_cases[0]);++n){const unsigned *v=original_jirai_coin_cases[n];unsigned side=v[0],enemy=1-side;expansion_fixture(side);test_game.phase=DUEL_BATTLE;test_game.players[side].life=v[1];test_game.players[side].monsters[0]=slot(simple_record(366),side,0);test_game.random_state=v[3]?1:2;uint32_t random=test_game.random_state;
check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_ATTACK_COIN&&test_game.players[side].life==(int)v[1]&&!test_game.players[side].monsters[0].attacked&&test_game.random_state==random);
check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==3&&options[0].index==0&&options[1].index==1&&options[2].index==DUEL_NO_SLOT);
check(choose(side,v[2])==DUEL_OK&&!test_game.choice.kind&&test_game.players[side].life==(int)(v[1]-v[4])&&test_game.players[enemy].life==5800&&test_game.players[side].monsters[0].attacked==1&&test_game.random_result.mask==v[3]&&test_game.random_result.guess==v[2]&&test_game.random_result.success==(v[2]==v[3]));++matrix;}
    for(unsigned side=0;side<2;++side){unsigned enemy=1-side;expansion_fixture(side);test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(simple_record(366),side,0);expansion_place(enemy,0,1066);uint32_t random=test_game.random_state;
check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.players[side].life==8000);
check(choose(side,DUEL_NO_SLOT)==DUEL_OK&&!test_game.choice.kind&&!test_game.players[side].monsters[0].attacked&&test_game.players[side].life==8000&&test_game.random_state==random);
check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_OK);random=test_game.random_state;uint32_t before=duel_state_hash(&test_game);
check(choose(side,5)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before&&test_game.random_state==random);
check(choose(side,0)==DUEL_OK&&test_game.random_result.sequence==1);before=duel_state_hash(&test_game);
check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);}
    for(unsigned n=0;n<sizeof(original_jirai_payment_cases)/sizeof(original_jirai_payment_cases[0]);++n){const unsigned *v=original_jirai_payment_cases[n];unsigned side=v[0],enemy=1-side;expansion_fixture(side);test_game.phase=DUEL_BATTLE;test_game.players[side].life=v[1];test_game.players[side].monsters[0]=slot(simple_record(366),side,0);test_game.random_state=v[4]?1:2;for(unsigned i=0;i<v[2];++i)expansion_place(enemy,i,1066);check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.players[side].life==(int)v[1]);check(choose(side,v[3])==DUEL_OK&&test_game.players[side].life==(int)v[6]);if(v[6])check(test_game.winner<0&&test_game.players[enemy].life==5800);else check(test_game.winner==(int)enemy&&test_game.players[enemy].life==8000);++matrix;}
    uint16_t deck[48];const unsigned extra[8]={373,370,371,372,366,366,366,1008};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<8;++i)deck[40+i]=simple_record(extra[i]);unsigned completed=0,coins=0;
    for(unsigned seed=1;seed<=10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,48,deck,48,seed+6000,seed&1)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));coins+=c.action==DUEL_ACTION_CHOOSE&&test_game.choice.kind==DUEL_CHOICE_ATTACK_COIN;int result=duel_command(&test_game,&c);
check(result==DUEL_OK);if(result!=DUEL_OK)break;
check(owned_cards(&test_game,0)==48&&owned_cards(&test_game,1)==48);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(completed==10&&coins);native_guardian_coin_test_result[0]=checks-initial;native_guardian_coin_test_result[1]=matrix;native_guardian_coin_test_result[2]=completed;native_guardian_coin_test_result[3]=coins;
}
