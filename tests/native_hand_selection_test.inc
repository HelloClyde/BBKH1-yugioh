#include "original_hand_selection_cases.h"
volatile uint32_t native_hand_selection_test_result[6];
static void hand_selection_deck(unsigned side,unsigned index,unsigned name,unsigned serial){DuelPlayer *p=&test_game.players[side];p->deck[index]=simple_record(name);p->deck_state[index]=(DuelCardState){serial,p->deck[index],0,0,side};if(p->deck_count<=index)p->deck_count=index+1;}
static void hand_selection_tests(void){unsigned initial=checks,matrix=0,reference=0;DuelCommand c;DuelChoiceOption options[DUEL_OPTION_MAX];
    for(unsigned n=0;n<sizeof(original_hand_selection_gate_cases)/sizeof(original_hand_selection_gate_cases[0]);++n){const int *v=original_hand_selection_gate_cases[n];unsigned actor=v[0],name=v[1];pair_fixture(actor);test_game.players[actor].deck_count=v[2];for(unsigned i=0;i<(name==1081?1:v[2]);++i)contact_test_hand(actor,i,77,100+i);for(unsigned i=0;i<v[3];++i)contact_test_hand(1-actor,i,77,200+i);if(v[4]){if(!test_game.players[actor].hand_count){++reference;continue;}test_game.players[actor].hand[0]=simple_record(name);test_game.players[actor].hand_state[0].card=simple_record(name);}else{test_game.players[actor].backrow[0]=slot(simple_record(name),actor,DUEL_FACEDOWN_DEFENSE);test_game.players[actor].backrow[0].serial=300;}check((command(v[4]?DUEL_ACTION_ACTIVATE_HAND:DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0)==DUEL_OK)==v[5]);++matrix;}
    for(unsigned n=0;n<sizeof(original_exchange_transport_cases)/sizeof(original_exchange_transport_cases[0]);++n){const int *v=original_exchange_transport_cases[n];unsigned actor=v[0];pair_fixture(actor);DuelCardState before[2][3];for(unsigned side=0;side<2;++side)for(unsigned i=0;i<(unsigned)v[1+side];++i){unsigned name=v[6]?77:(i==0?77:i==1?21:57);contact_test_hand(side,i,name,100+side*10+i);DuelCardState *s=&test_game.players[side].hand_state[i];s->owner=side^v[5];s->proper=i%2;s->history=i%4;before[side][i]=*s;}spirit_fake_spell(1205,actor);pass_chain();
check(test_game.choice.kind==DUEL_CHOICE_EXCHANGE_OTHER&&test_game.choice.player==actor);unsigned other_index=v[3+1-actor],own_index=v[3+actor];
check(choose(actor,other_index)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_EXCHANGE_OWN&&test_game.choice.player==1-actor);
check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==(unsigned)v[1+actor]);
check(choose(1-actor,own_index)==DUEL_OK&&!test_game.choice.kind);for(unsigned side=0;side<2;++side){unsigned at=0,selected=v[3+side];for(unsigned i=0;i<(unsigned)v[1+side];++i)if(i!=selected){const DuelCardState *s=&test_game.players[side].hand_state[at++];
check(s->serial==before[side][i].serial&&s->owner==before[side][i].owner&&s->proper==before[side][i].proper&&s->history==before[side][i].history);}const DuelCardState *s=&test_game.players[side].hand_state[at],*wanted=&before[1-side][v[3+1-side]];
check(test_game.players[side].hand_count==(unsigned)v[1+side]&&s->serial==wanted->serial&&s->card==wanted->card&&s->owner==wanted->owner&&s->proper==wanted->proper&&s->history==wanted->history);}check(identity_unique(&test_game));++matrix;}
    for(unsigned n=0;n<sizeof(original_painful_first_record_cases)/sizeof(original_painful_first_record_cases[0]);++n){const int *v=original_painful_first_record_cases[n];unsigned actor=v[0];const unsigned names[2][5]={{77,21,77,57,1079},{61,1249,61,57,1079}};pair_fixture(actor);test_game.players[actor].deck_count=0;for(unsigned i=0;i<5;++i){DuelPlayer *p=&test_game.players[actor];p->deck[i]=v[4+i];p->deck_state[i]=(DuelCardState){100+i,p->deck[i],0,0,actor};p->deck_count=i+1;}spirit_fake_spell(1081,actor);pass_chain();
check(test_game.choice.kind==DUEL_CHOICE_PAINFUL_DECK);for(unsigned i=0;i<5;++i){check(choose(actor,i)==DUEL_OK);
check(test_game.players[actor].deck_count==5&&!test_game.players[actor].hand_count&&!test_game.players[actor].grave_count);}check(test_game.choice.kind==DUEL_CHOICE_PAINFUL_KEEP&&test_game.choice.player==1-actor);
check(choose(1-actor,v[2])==DUEL_OK);
check(!test_game.players[actor].deck_count&&test_game.players[actor].hand_count==1&&test_game.players[actor].hand_state[0].serial==100+(unsigned)v[3]&&test_game.players[actor].grave_count==5);unsigned at=0;for(unsigned i=0;i<5;++i)if(i!=(unsigned)v[3])check(test_game.players[actor].grave_serial[at++]==100+i);
check(identity_unique(&test_game));++matrix;}
    /* Step return codes and UI/log requests are bounded original evidence.
     * Public native choices above cover the corresponding complete flow. */
    reference+=sizeof(original_exchange_steps_cases)/sizeof(original_exchange_steps_cases[0])+sizeof(original_painful_finish_cases)/sizeof(original_painful_finish_cases[0]);
    for(unsigned actor=0;actor<2;++actor){unsigned enemy=1-actor;
        for(unsigned name_index=0;name_index<2;++name_index){unsigned name=name_index?1081:1205;pair_fixture(actor);contact_test_hand(actor,0,name,500);contact_test_hand(actor,1,77,100);contact_test_hand(enemy,0,21,200);test_game.players[enemy].backrow[0]=slot(simple_record(1029),enemy,DUEL_FACEDOWN_DEFENSE);test_game.players[enemy].backrow[0].serial=300;
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);c=(DuelCommand){enemy,DUEL_ACTION_ACTIVATE_BACKROW,0};
check(duel_command(&test_game,&c)==DUEL_OK);if(test_game.choice.kind==DUEL_CHOICE_HAND)check(choose(enemy,0)==DUEL_OK);pass_chain();
#ifdef _WIN32
if(test_game.choice.kind||test_game.players[actor].hand_count!=1||test_game.players[actor].hand_state[0].serial!=100||test_game.players[actor].deck_count!=34+actor)fprintf(stderr,"negation actor%u name%u choice%u hand%u serial%u deck%u chain%u response%u\n",actor,name,test_game.choice.kind,test_game.players[actor].hand_count,test_game.players[actor].hand_state[0].serial,test_game.players[actor].deck_count,test_game.chain_count,test_game.response_player);
#endif
check(!test_game.choice.kind&&test_game.players[actor].hand_count==1&&test_game.players[actor].hand_state[0].serial==100&&test_game.players[actor].deck_count==34+actor);
            pair_fixture(actor);contact_test_hand(actor,0,77,100);contact_test_hand(enemy,0,21,200);spirit_fake_spell(name,actor);test_game.chain[0].effect_negated=1;pass_chain();
check(!test_game.choice.kind&&test_game.players[actor].hand_state[0].serial==100&&test_game.players[enemy].hand_state[0].serial==200);
        }
        /* Actual Exchange is a normal Spell. The received card is not offered
         * as a candidate to the second chooser; later discard goes to owner. */
        pair_fixture(actor);contact_test_hand(actor,0,1205,500);contact_test_hand(actor,1,77,100);contact_test_hand(enemy,0,21,200);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);pass_chain();
check(test_game.choice.kind==DUEL_CHOICE_EXCHANGE_OTHER&&choose(actor,0)==DUEL_OK);
check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==1&&options[0].card==simple_record(77));
check(choose(enemy,0)==DUEL_OK&&test_game.players[actor].hand_state[0].serial==200&&test_game.players[actor].hand_state[0].owner==enemy);for(unsigned i=1;i<7;++i)contact_test_hand(actor,i,57,600+i);test_game.phase=DUEL_END;test_game.discard_required=1;
check(command(DUEL_ACTION_DISCARD,0,0,0,0,0)==DUEL_OK);
check(test_game.players[enemy].grave_serial[0]==200&&identity_unique(&test_game));
        /* Non-top selections are sent in selection order. Unselected cards
         * remain and are shuffled; Prohibition does not filter this pool. */
        pair_fixture(actor);test_game.players[actor].deck_count=0;for(unsigned i=0;i<7;++i)hand_selection_deck(actor,i,i==0?1079:77,100+i);contact_test_hand(actor,0,1081,500);state_prohibition(enemy,1079,0);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);pass_chain();const unsigned selected[]={6,0,4,1,3};for(unsigned i=0;i<5;++i)check(choose(actor,selected[i])==DUEL_OK);
check(choose(enemy,1)==DUEL_OK);
check(test_game.players[actor].deck_count==2&&test_game.players[actor].hand_state[0].serial==100);const unsigned grave_ids[]={106,104,101,103};for(unsigned i=0;i<4;++i)check(test_game.players[actor].grave_serial[i]==grave_ids[i]);
check(identity_unique(&test_game));
    }
    uint16_t deck[46];const unsigned extra[]={1205,1081,1205,1081,77,1079};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<6;++i)deck[40+i]=simple_record(extra[i]);unsigned completed=0,activated=0;
    for(unsigned seed=0;seed<10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,46,deck,46,17400+seed,seed&1)==DUEL_OK);
check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));if(c.action==DUEL_ACTION_ACTIVATE_HAND||c.action==DUEL_ACTION_ACTIVATE_BACKROW){unsigned card=c.action==DUEL_ACTION_ACTIVATE_HAND?test_game.players[c.player].hand[c.source]:test_game.players[c.player].backrow[c.source].card;unsigned name=test_game.cards[card].name_group;activated+=name==1081||name==1205;}int r=duel_command(&test_game,&c);
check(r==DUEL_OK);if(r!=DUEL_OK)break;
check(owned_cards(&test_game,0)==46&&owned_cards(&test_game,1)==46);
check(identity_unique(&test_game));}check(test_game.winner>=0);completed+=test_game.winner>=0;}check(completed==10);native_hand_selection_test_result[2]=completed;native_hand_selection_test_result[3]=activated;
    native_hand_selection_test_result[0]=checks-initial;native_hand_selection_test_result[1]=matrix;native_hand_selection_test_result[4]=reference;
}
