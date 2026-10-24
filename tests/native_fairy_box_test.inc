#include "original_fairy_box_cases.h"
volatile uint32_t native_fairy_box_test_result[6];
static void fairy_fixture(unsigned side){departure_fixture(side,77,77);expansion_place(1-side,0,1424);test_game.players[1-side].backrow[0].serial=301;}
static unsigned fairy_guess(unsigned wanted){for(unsigned seed=1;;++seed){uint32_t x=seed;x^=x<<13;x^=x>>17;x^=x<<5;if((x&1)==wanted){test_game.random_state=seed;return seed;}}}
static void fairy_equip(unsigned side){dynamic_equip_attach(side,0,simple_record(303),100);test_game.stat_links[test_game.stat_link_count++]=(DuelStatLink){test_game.players[side].backrow[0].serial,100,2};}
static void fairy_box_tests(void){unsigned initial=checks,matrix=0,reference=0;DuelCommand c;
    for(unsigned n=0;n<sizeof(original_fairy_box_stat_cases)/sizeof(original_fairy_box_stat_cases[0]);++n){const int *v=original_fairy_box_stat_cases[n];unsigned side=v[0];
        /* A hidden source with a manually stale negation cache has no native
         * independent cache counterpart. Other hidden sources remain covered. */
        if(v[4]<2&&v[5]){++reference;continue;}
        fairy_fixture(side);DuelSlot *target=&test_game.players[side].monsters[0],*source=&test_game.players[1-side].backrow[0];
        target->position=v[2]<2?DUEL_FACEDOWN_DEFENSE:v[2]==3?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK;
        source->position=v[4]<2?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK;
        if(v[1]==1)fairy_equip(side);
        duel_fairy_reset_attack(&test_game,1-side,0,side,0);
        if(v[1]==2)fairy_equip(side);
        if(!v[3])source->card=0;
        if(v[5]){test_game.players[1-side].monsters[4]=slot(simple_record(751),1-side,0);test_game.players[1-side].monsters[4].serial=250;}
        if(v[6]){DuelSlot saved=test_game.players[side].backrow[0];state_prohibition(side,1424,0);test_game.players[side].backrow[4]=test_game.players[side].backrow[0];test_game.players[side].backrow[0]=saved;}
        test_game.reverse_turn=v[7]?test_game.turn:0;duel_refresh_field_cache(&test_game);
        #ifdef _WIN32
        if(duel_attack_value(&test_game,side,0)!=v[8])fprintf(stderr,"fairy stat row%u side%u order%d target%d present%d srcpos%d disabled%d prohi%d rev%d actual%d expect%d\n",n,side,v[1],v[2],v[3],v[4],v[5],v[6],v[7],duel_attack_value(&test_game,side,0),v[8]);
#endif
        check(duel_attack_value(&test_game,side,0)==v[8]);
        check(duel_defense_value(&test_game,side,0)==v[9]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_fairy_box_coin_cases)/sizeof(original_fairy_box_coin_cases[0]);++n){const int *v=original_fairy_box_coin_cases[n];unsigned actor=v[0],side=1-actor;
        fairy_fixture(side);test_game.window=(DuelWindow){DUEL_WINDOW_ATTACK,side,0,0,100,101,simple_record(77)};
        if(!v[1])test_game.players[actor].backrow[0].card=0;
        test_game.players[actor].backrow[0].position=v[2]<2?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK;
        pair_chain(1424,actor);test_game.chain[0].source_slot=0;test_game.chain[0].target_count=1;test_game.chain[0].targets[0]=(DuelTarget){0,side,0};test_game.chain[0].effect_negated=v[3]!=0;
        pass_chain();if(v[1]&&!v[3]){check(test_game.choice.kind==DUEL_CHOICE_FAIRY_BOX_COIN);fairy_guess(v[5]);check(choose(actor,v[4])==DUEL_OK);check(test_game.random_result.success==(v[4]==v[5]));}
        check((test_game.stat_link_count!=0)==v[6]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_fairy_box_ordered_cases)/sizeof(original_fairy_box_ordered_cases[0]);++n){const int *v=original_fairy_box_ordered_cases[n];unsigned side=v[0],op=v[1],after=v[2];fairy_fixture(side);
        DuelSlot *m=&test_game.players[side].monsters[0];int bonus=(int[]){1000,-700,300,-300,-500}[op];
        if(op<2){dynamic_equip_attach(side,0,simple_record(op?1244:303),100);if(!after)test_game.stat_links[test_game.stat_link_count++]=(DuelStatLink){test_game.players[side].backrow[0].serial,100,2};}
        else if(op<4)m->temporary_bonus=bonus;else m->battle_curse=bonus;
        duel_fairy_reset_attack(&test_game,1-side,0,side,0);
        if(op<2&&after)test_game.stat_links[test_game.stat_link_count++]=(DuelStatLink){test_game.players[side].backrow[0].serial,100,2};
        if(op>=2&&after)test_game.stat_links[0].attack_after=bonus;
        m->swap_until=test_game.turn;m->swap_parity=v[3];test_game.reverse_turn=v[4]?test_game.turn:0;
        check(duel_attack_value(&test_game,side,0)==v[5]);check(duel_defense_value(&test_game,side,0)==v[6]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_fairy_box_producer_cases)/sizeof(original_fairy_box_producer_cases[0]);++n){const int *v=original_fairy_box_producer_cases[n];unsigned side=v[0],actor=1-side;departure_fixture(side,77,77);magnet_hold(side);
        for(unsigned i=0;i<5;++i)if(v[1]>>i&1){expansion_place(actor,i,1424);test_game.players[actor].backrow[i].serial=301+i;test_game.players[actor].backrow[i].position=v[2]?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK;}
        if(v[3]){test_game.players[side].monsters[4]=slot(simple_record(751),side,0);test_game.players[side].monsters[4].serial=250;}
        check(command(DUEL_ACTION_ATTACK,0,actor,0,0,0)==DUEL_OK);check(test_game.chain_count==(unsigned)v[4]);unsigned j=0;
        for(unsigned i=0;i<5;++i)if((v[1]>>i&1)&&!v[2]&&!v[3]){check(test_game.chain[j].source_slot==i&&test_game.chain[j].target_count==1&&test_game.chain[j].targets[0].player==side&&test_game.chain[j].targets[0].slot==0);++j;}++matrix;
    }
    for(unsigned side=0;side<2;++side){unsigned actor=1-side;
        /* Initial Trap activation outside combat has no coin task. The next
         * actual attack queues both live Boxes in one chain with ordinary
         * attack-declaration response cards; the last Box resolves first. */
        pair_fixture(actor);test_game.players[actor].backrow[0]=slot(simple_record(1424),actor,DUEL_FACEDOWN_DEFENSE);test_game.players[actor].backrow[0].serial=300;
        check(command(DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0)==DUEL_OK);pass_chain();
        check(test_game.players[actor].backrow[0].effect_active&&!test_game.choice.kind&&!test_game.random_result.sequence);
        test_game.active=side;test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(simple_record(77),side,0);test_game.players[side].monsters[0].serial=100;test_game.players[actor].monsters[0]=slot(simple_record(77),actor,1);test_game.players[actor].monsters[0].serial=101;
        expansion_place(actor,1,1424);test_game.players[actor].backrow[1].serial=301;
        check(command(DUEL_ACTION_ATTACK,0,actor,0,0,0)==DUEL_OK);pass_chain();
        check(test_game.choice.kind==DUEL_CHOICE_FAIRY_BOX_COIN&&test_game.window.kind==DUEL_WINDOW_ATTACK);
        fairy_guess(0);check(choose(actor,0)==DUEL_OK);pass_chain();
        check(test_game.choice.kind==DUEL_CHOICE_FAIRY_BOX_COIN);fairy_guess(0);check(choose(actor,0)==DUEL_OK);contact_test_finish();
        check(test_game.random_result.sequence==2&&duel_attack_value(&test_game,side,0)==0&&test_game.players[side].monsters[0].attacked==1);
        check(test_game.players[side].life==6850&&test_game.stat_link_count==2);
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK);check(duel_attack_value(&test_game,side,0)==1700&&!test_game.stat_link_count);
        /* Both automatic effects are already in the chain when MST is
         * activated; removing one source suppresses only its later callback. */
        fairy_fixture(side);expansion_place(actor,1,1424);test_game.players[actor].backrow[1].serial=302;contact_test_hand(side,0,1079,500);
        check(command(DUEL_ACTION_ATTACK,0,actor,0,0,0)==DUEL_OK&&test_game.chain_count==2&&test_game.response_player==side);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,actor,0,0,0)==DUEL_OK);pass_chain();check(test_game.choice.kind==DUEL_CHOICE_FAIRY_BOX_COIN);fairy_guess(0);check(choose(actor,0)==DUEL_OK);contact_test_finish();check(test_game.random_result.sequence==1&&!test_game.players[actor].backrow[0].card&&test_game.players[actor].backrow[1].card);
        /* A newly activated Box in the declaration window uses the same coin
         * resolver, and source removal restores the ordinary attack value. */
        departure_fixture(side,77,77);test_game.players[actor].monsters[0].position=DUEL_FACEUP_DEFENSE;test_game.players[actor].backrow[0]=slot(simple_record(1424),actor,DUEL_FACEDOWN_DEFENSE);test_game.players[actor].backrow[0].serial=300;
        check(command(DUEL_ACTION_ATTACK,0,actor,0,0,0)==DUEL_OK);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK);pass_chain();
        check(test_game.choice.kind==DUEL_CHOICE_FAIRY_BOX_COIN&&duel_choice_options(&test_game,0,0)==2);fairy_guess(0);check(choose(actor,0)==DUEL_OK);contact_test_finish();check(duel_attack_value(&test_game,side,0)==0);
        test_game.players[actor].backrow[0].card=0;duel_refresh_field_cache(&test_game);check(duel_attack_value(&test_game,side,0)==1700);
        /* Target hiding clears the relation; hiding only its source retains
         * the original source-position quirk without changing printed DEF. */
        fairy_fixture(side);duel_fairy_reset_attack(&test_game,actor,0,side,0);test_game.players[actor].backrow[0].position=DUEL_FACEDOWN_DEFENSE;check(duel_attack_value(&test_game,side,0)==0);duel_change_monster_position(&test_game,side,0,DUEL_FACEDOWN_DEFENSE);check(duel_attack_value(&test_game,side,0)==1700);duel_change_monster_position(&test_game,side,0,DUEL_FACEUP_ATTACK);check(duel_attack_value(&test_game,side,0)==1700);
        /* Resolve actual Dice links after the reset instead of manually
         * assigning their post-reset accumulator. Earlier innate ATK is gone. */
        for(unsigned skull=0;skull<2;++skull){fairy_fixture(side);duel_fairy_reset_attack(&test_game,actor,0,side,0);unsigned caster=skull?actor:side;pair_chain(skull?1204:1203,caster);DuelChainLink *l=&test_game.chain[0];l->effect_activation=0;l->source_slot=1;test_game.players[caster].backrow[1]=slot(l->card,caster,0);test_game.players[caster].backrow[1].serial=300;l->source_serial=300;test_game.window=(DuelWindow){0};test_game.random_state=1;pass_chain();int attack=duel_attack_value(&test_game,side,0);check(skull?attack==0:attack>=100&&attack<=600);}
        /* Real Standby payment remains a maintenance task even under Jinzo. */
        fairy_fixture(side);test_game.active=actor;test_game.phase=DUEL_DRAW;test_game.players[side].monsters[4]=slot(simple_record(751),side,0);test_game.players[side].monsters[4].serial=250;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);check(test_game.choice.kind==DUEL_CHOICE_UPKEEP);check(choose(actor,0)==DUEL_OK&&test_game.players[actor].life==7500);
    }
    uint16_t deck[44];for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=40;i<44;++i)deck[i]=simple_record(1424);unsigned completed=0,coins=0;
    for(unsigned seed=0;seed<10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,44,deck,44,14100+seed,seed&1)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));coins+=test_game.choice.kind==DUEL_CHOICE_FAIRY_BOX_COIN;int r=duel_command(&test_game,&c);check(r==DUEL_OK);if(r!=DUEL_OK)break;check(owned_cards(&test_game,0)==44&&owned_cards(&test_game,1)==44);check(identity_unique(&test_game));}check(test_game.winner>=0);completed+=test_game.winner>=0;}check(completed==10);
    native_fairy_box_test_result[0]=checks-initial;native_fairy_box_test_result[1]=matrix;native_fairy_box_test_result[2]=completed;native_fairy_box_test_result[3]=coins;native_fairy_box_test_result[4]=reference;
}
