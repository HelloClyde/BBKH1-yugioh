#include "original_soul_exchange_cases.h"
volatile uint32_t native_soul_exchange_test_result[5];
static void soul_fixture(unsigned side,unsigned index,unsigned card,unsigned position){expansion_fixture(side);test_game.phase=DUEL_MAIN1;test_game.players[1-side].monsters[index]=slot(card,1-side,position);test_game.players[1-side].monsters[index].serial=200;test_game.next_serial=200;test_game.players[side].hand[0]=simple_record(1220);test_game.players[side].hand_count=1;}
static void soul_cast(unsigned side,unsigned index){check(command(DUEL_ACTION_ACTIVATE_HAND,0,1-side,index,0,0)==DUEL_OK);
        pass_chain();}
static void soul_exchange_tests(void){unsigned initial=checks,matrix=0,reference_only=0;DuelCommand c;DuelChoiceOption options[DUEL_OPTION_MAX];
    for(unsigned n=0;n<sizeof(original_soul_phase_cases)/sizeof(original_soul_phase_cases[0]);++n){const unsigned *v=original_soul_phase_cases[n];soul_fixture(v[0],4,24,0);test_game.phase=v[1];uint32_t before=duel_state_hash(&test_game);int result=command(DUEL_ACTION_ACTIVATE_HAND,0,1-v[0],4,0,0);
        check((result==DUEL_OK)==v[2]);if(!v[2])check(duel_state_hash(&test_game)==before);++matrix;}
    for(unsigned n=0;n<sizeof(original_soul_target_cases)/sizeof(original_soul_target_cases[0]);++n){const unsigned *v=original_soul_target_cases[n];/* An independently supplied immunity bit on a face-down normal monster
           has no native field representation. The eight abstract false-legality rows on an otherwise unprotected face-up/down normal target are also reference-only. */
        if((v[6]&&v[4]==2)||(!v[5]&&v[0]!=v[1]&&v[2]<5&&v[3]&&!v[6])){++reference_only;continue;}soul_fixture(v[0],4,0,0);DuelSlot *s=v[2]<5?&test_game.players[v[1]].monsters[v[2]]:&test_game.players[v[1]].backrow[0];*s=slot(v[3]?(v[6]?simple_record(1326):24):0,v[1],v[4]);s->serial=201;if(v[6]){test_game.players[0].field=slot(simple_record(333),0,0);test_game.players[0].field.effect_active=1;}if(!v[5]&&v[2]<5){test_game.battle_destroy_count=1;test_game.battle_destroy[0]=(DuelTarget){201,v[1],v[2]};}int result=command(DUEL_ACTION_ACTIVATE_HAND,0,v[1],v[2],0,0);
#ifdef _WIN32
        if((result==DUEL_OK)!=v[7])fprintf(stderr,"soul target side%u owner%u i%u present%u pos%u legal%u immune%u result%d want%u\n",v[0],v[1],v[2],v[3],v[4],v[5],v[6],result,v[7]);
#endif
        check((result==DUEL_OK)==v[7]);++matrix;}
    for(unsigned n=0;n<sizeof(original_soul_callback_cases)/sizeof(original_soul_callback_cases[0]);++n){const unsigned *v=original_soul_callback_cases[n];soul_fixture(v[0],v[1],v[2]?(v[6]?simple_record(1201):24):0,v[4]);test_game.players[v[0]].hand_count=0;test_game.players[1-v[0]].monsters[v[1]].serial=v[3]?200:201;test_game.chain_count=1;test_game.response_player=1-v[0];DuelChainLink *link=&test_game.chain[0];link->card=simple_record(1220);link->command.player=v[0];link->command.target_player=1-v[0];link->command.target=v[1];link->target_serial=200;link->source_slot=DUEL_NO_SLOT;link->effect_negated=v[5];link->speed=1;
        pass_chain();
        check((test_game.battle_skip_turn[v[0]]==test_game.turn)==v[7]);
        check((test_game.tribute_grant_count!=0)==v[8]);++matrix;}
    for(unsigned n=0;n<sizeof(original_soul_grant_cases)/sizeof(original_soul_grant_cases[0]);++n){const unsigned *v=original_soul_grant_cases[n];expansion_fixture(v[0]);test_game.players[v[1]].monsters[v[2]]=slot(24,v[1],v[5]);test_game.players[v[1]].monsters[v[2]].serial=200;if(v[3]){test_game.tribute_grant_count=1;test_game.tribute_grants[0]=(DuelTributeGrant){200,v[3]==13?0:3,v[4],v[3]};}if(v[6])expansion_place(1-v[0],0,1418);
        check(duel_can_tribute(&test_game,v[0],v[1],v[2])==(int)v[7]);++matrix;}
    const unsigned patterns[6][2][2]={{{3,0},{0,0}},{{3,1},{0,0}},{{3,0},{3,1}},{{3,1},{3,0}},{{1,1},{3,0}},{{13,1},{3,0}}};
    for(unsigned n=0;n<sizeof(original_soul_grant_order_cases)/sizeof(original_soul_grant_order_cases[0]);++n){const unsigned *v=original_soul_grant_order_cases[n];expansion_fixture(v[0]);test_game.players[v[1]].monsters[0]=slot(24,v[1],0);test_game.players[v[1]].monsters[0].serial=200;test_game.tribute_grant_count=v[2]<2?1:2;for(unsigned i=0;i<test_game.tribute_grant_count;++i)test_game.tribute_grants[i]=(DuelTributeGrant){200,patterns[v[2]][i][0]==13?0:3,patterns[v[2]][i][1],patterns[v[2]][i][0]};
        check(duel_can_tribute(&test_game,v[0],v[1],0)==(int)v[3]);++matrix;}
    for(unsigned side=0;side<2;++side){unsigned enemy=1-side;
        soul_fixture(side,4,24,2);test_game.players[side].monsters[0]=slot(24,side,0);test_game.players[side].hand[1]=816;test_game.players[side].hand_count=2;soul_cast(side,4);
        check(duel_can_tribute(&test_game,side,enemy,4)&&test_game.battle_skip_turn[side]==3);
        check(command(DUEL_ACTION_SUMMON,0,side,0,0,1u|(1u<<9))==DUEL_OK);
        pass_chain();
        check(test_game.players[side].monsters[0].card==816&&test_game.players[enemy].grave_count==1&&test_game.players[side].grave_count==2);uint32_t before=duel_state_hash(&test_game);
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_BATTLE,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
        check(duel_ai_decide(&test_game,side,&c)&&c.action==DUEL_ACTION_PHASE&&c.target==DUEL_MAIN2);
        /* Grants follow the field identity, not an index or a same-name replacement. */
        soul_fixture(side,4,24,0);soul_cast(side,4);test_game.players[enemy].monsters[3]=test_game.players[enemy].monsters[4];test_game.players[enemy].monsters[4]=empty_slot();
        check(duel_can_tribute(&test_game,side,enemy,3)&&!duel_can_tribute(&test_game,side,enemy,4));test_game.players[enemy].monsters[3].serial=201;
        check(!duel_can_tribute(&test_game,side,enemy,3));test_game.players[enemy].monsters[3].serial=200;test_game.turn=4;
        check(!duel_can_tribute(&test_game,side,enemy,3));
        soul_fixture(side,4,24,0);expansion_place(enemy,0,1154);soul_cast(side,4);
        check(test_game.battle_skip_turn[side]==3&&!test_game.tribute_grant_count);
        soul_fixture(side,4,simple_record(1201),2);soul_cast(side,4);
        check(!test_game.battle_skip_turn[side]&&!test_game.tribute_grant_count&&test_game.players[enemy].monsters[4].position==DUEL_FACEUP_DEFENSE);
        /* A full own field requires at least one own tribute. Opposing costs
           cannot turn an occupied own destination into an empty zone. */
        soul_fixture(side,4,24,0);for(unsigned i=0;i<5;++i)test_game.players[side].monsters[i]=slot(24,side,0);test_game.players[side].hand[1]=816;test_game.players[side].hand_count=2;soul_cast(side,4);before=duel_state_hash(&test_game);
        check(command(DUEL_ACTION_SUMMON,0,side,0,0,(1u<<9)|(1u<<1))==DUEL_ZONE_FULL&&duel_state_hash(&test_game)==before);
        check(command(DUEL_ACTION_SUMMON,0,side,0,0,(1u<<9)|1u)==DUEL_OK);
        pass_chain();
        /* Generic activated monster costs use the same association. */
        soul_fixture(side,4,24,0);unsigned cannon=0;for(unsigned i=0;i<DUEL_IGNITION_RULE_COUNT;++i)if(ignition_rules[i].op==1&&ignition_rules[i].tributes==1&&ignition_rules[i].amount==500)cannon=simple_record(ignition_rules[i].name);
        check(cannon!=0);test_game.players[side].monsters[0]=slot(cannon,side,0);soul_cast(side,4);
        check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,1u<<9)==DUEL_OK);
        pass_chain();
        check(test_game.players[enemy].grave_count==1&&test_game.players[enemy].life==7500&&test_game.players[side].monsters[0].card==cannon);
        /* A constructed same-turn maintenance task verifies the shared cost
           consumer independently of the normal phase scheduler. */
        soul_fixture(side,4,24,2);test_game.players[side].monsters[0]=slot(simple_record(1431),side,0);test_game.players[side].monsters[0].serial=100;soul_cast(side,4);test_game.choice=(DuelTrigger){DUEL_CHOICE_TRIBUTE_UPKEEP,side,simple_record(1431),100};
        check(tribute_upkeep_has(side,side,9));
        check(choose(side,9)==DUEL_OK&&test_game.players[enemy].grave_count==1&&test_game.players[side].monsters[0].card);
        /* Ritual materials are actor-relative field tokens after the hand
           domain. Only an own field material supplies destination space. */
        const OriginalRitualCase *r=&original_ritual_cases[0];soul_fixture(side,4,816,2);test_game.players[side].hand[1]=r->spell;test_game.players[side].hand[2]=r->monster;test_game.players[side].hand_count=3;soul_cast(side,4);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_RITUAL_MONSTER&&choose(side,0)==DUEL_OK);
        check(test_game.choice.kind==DUEL_CHOICE_RITUAL_MATERIAL&&choose(side,DUEL_HAND_MAX+9)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_RITUAL_POSITION&&choose(side,0)==DUEL_OK);
        pass_chain();
        check(test_game.players[enemy].grave_count==1&&test_game.players[side].monsters[0].card==r->monster);
        /* Toon procedure consumes two opposing grants and opens its ordinary
           inherent special-summon response after all costs are paid. */
        soul_fixture(side,4,24,2);test_game.players[enemy].monsters[3]=slot(24,enemy,0);test_game.players[enemy].monsters[3].serial=201;expansion_place(side,0,954);soul_cast(side,4);test_game.players[side].hand[0]=simple_record(1220);test_game.players[side].hand_count=1;soul_cast(side,3);test_game.players[side].hand[0]=simple_record(726);test_game.players[side].hand_count=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_SPECIAL_HAND_COST&&choose(side,9)==DUEL_OK&&choose(side,8)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_SPECIAL_HAND_POSITION&&choose(side,0)==DUEL_OK);
        pass_chain();
        check(test_game.players[enemy].grave_count==2&&test_game.players[side].monsters[0].card==simple_record(726));
    }
    for(unsigned side=0;side<2;++side){unsigned enemy=1-side;
        const OriginalRitualCase *r=&original_ritual_cases[0];soul_fixture(side,4,816,2);for(unsigned i=0;i<5;++i)test_game.players[side].monsters[i]=slot(simple_record(57),side,0);test_game.players[side].hand[1]=r->spell;test_game.players[side].hand[2]=r->monster;test_game.players[side].hand_count=3;soul_cast(side,4);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&choose(side,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_RITUAL_MATERIAL);unsigned count=duel_choice_options(&test_game,options,DUEL_OPTION_MAX),enemy_option=0;for(unsigned i=0;i<count;++i)enemy_option|=options[i].index==DUEL_HAND_MAX+9;check(!enemy_option);check(choose(side,DUEL_HAND_MAX)==DUEL_OK&&choose(side,DUEL_HAND_MAX+9)==DUEL_OK&&choose(side,0)==DUEL_OK);pass_chain();check(test_game.players[side].monsters[0].card==r->monster&&test_game.players[enemy].grave_count==1);
        soul_fixture(side,4,24,2);test_game.players[enemy].monsters[3]=slot(24,enemy,0);test_game.players[enemy].monsters[3].serial=201;for(unsigned i=0;i<5;++i)test_game.players[side].monsters[i]=slot(24,side,0);expansion_place(side,0,954);soul_cast(side,4);test_game.players[side].hand[0]=simple_record(1220);test_game.players[side].hand_count=1;soul_cast(side,3);test_game.players[side].hand[0]=simple_record(726);test_game.players[side].hand_count=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&choose(side,9)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_SPECIAL_HAND_COST);uint32_t before=duel_state_hash(&test_game);check(choose(side,8)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);check(choose(side,0)==DUEL_OK&&choose(side,0)==DUEL_OK);pass_chain();check(test_game.players[side].monsters[0].card==simple_record(726)&&test_game.players[enemy].grave_count==1);
        /* A constructed same-turn summon window tests Horn of Heaven's
           generic tribute payment, including excluding the pending summon. */
        soul_fixture(side,4,24,2);soul_cast(side,4);unsigned horn=0;for(unsigned card=1;card<DUEL_CARD_COUNT;++card)if(duel_counter_tribute_cost(&duel_card_definitions[card]))horn=card;check(horn!=0);test_game.players[side].backrow[0]=slot(horn,side,2);test_game.players[side].backrow[0].set_turn=1;test_game.players[enemy].monsters[1]=slot(24,enemy,0);test_game.players[enemy].monsters[1].serial=300;test_game.active=enemy;test_game.window=(DuelWindow){DUEL_WINDOW_SUMMON_NEGATION,enemy,1,0,300,0,24,0};test_game.response_player=side;test_game.response_passes=0;c=(DuelCommand){side,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,1u<<9};check(duel_command(&test_game,&c)==DUEL_OK);pass_chain();check(test_game.players[enemy].grave_count==2&&!test_game.players[enemy].monsters[1].card&&!test_game.players[enemy].monsters[4].card);
    }
    unsigned completed=0,activations=0;uint16_t deck[48];const unsigned extra[8]={1220,1220,1220,0,1530,1079,1431,539};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<8;++i)deck[40+i]=simple_record(extra[i]);
    for(unsigned seed=1;seed<=10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,48,deck,48,seed+8100,seed&1)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));activations+=(c.action==DUEL_ACTION_ACTIVATE_HAND&&test_game.players[c.player].hand[c.source]==simple_record(1220))||(c.action==DUEL_ACTION_ACTIVATE_BACKROW&&test_game.players[c.player].backrow[c.source].card==simple_record(1220));int result=duel_command(&test_game,&c);
#ifdef _WIN32
        if(result!=DUEL_OK)fprintf(stderr,"soul game seed%u phase%u action%u source%u target%u trib%u result%d\n",seed,test_game.phase,c.action,c.source,c.target,c.tributes,result);
#endif
        check(result==DUEL_OK);if(result!=DUEL_OK)break;
        check(owned_cards(&test_game,0)==48&&owned_cards(&test_game,1)==48);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(completed==10);native_soul_exchange_test_result[0]=checks-initial;native_soul_exchange_test_result[1]=matrix;native_soul_exchange_test_result[2]=completed;native_soul_exchange_test_result[3]=activations;native_soul_exchange_test_result[4]=reference_only;
}
