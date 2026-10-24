#include "original_transient_cases.h"
volatile uint32_t native_transient_test_result[4];
static void transient_tests(void){unsigned first=checks;
    for(unsigned ci=0;ci<sizeof(original_transient_cases)/sizeof(original_transient_cases[0]);++ci){const int *r=original_transient_cases[ci];unsigned card=r[0],count=r[1];if(duel_card_definitions[card].effect==DUEL_E_IGNITION)continue;
        for(unsigned side=0;side<2;++side)for(unsigned negate=0;negate<2;++negate){unsigned weak=duel_card_definitions[card].name_group==1415,phantom=duel_card_definitions[card].name_group==1534,target=weak?1-side:side;simple_fixture(card,side);
            test_game.players[target].monsters[0]=slot(550,target,DUEL_FACEUP_ATTACK);test_game.players[target].monsters[0].serial=99;DuelPlayer *p=&test_game.players[side];p->grave_count=count;for(unsigned i=0;i<count;++i)p->grave[i]=24;if(count<190){p->grave[p->grave_count++]=316;p->grave[p->grave_count++]=893;}
            if(phantom&&!count){uint32_t hash=duel_state_hash(&test_game);check(simple_activate(card,target,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==hash);continue;}
            if(negate)simple_negator(card,side);
            if(weak){test_game.active=1-side;test_game.phase=DUEL_BATTLE;check(command(DUEL_ACTION_ATTACK,0,side,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.response_player==side);DuelCommand c={side,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK);}
            else {check(simple_activate(card,target,0)==DUEL_OK);if(negate)simple_finish_negation(card,side);}
            int attack=1900+(negate?0:r[2]),defense=900+(negate?0:r[3]);if(attack<0)attack=0;if(defense<0)defense=0;check(duel_attack_value(&test_game,target,0)==attack&&duel_defense_value(&test_game,target,0)==defense);
            if(weak)check(p->life==8000-attack);
            test_game.phase=DUEL_MAIN2;test_game.players[test_game.active].hand_count=0;check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check(duel_attack_value(&test_game,target,0)==1900&&duel_defense_value(&test_game,target,0)==900);
        }
    }
    for(unsigned side=0;side<2;++side){unsigned rush=simple_record(1075),snake=simple_record(1082),reinforce=simple_record(1015);
        /* Invalid face-down targets are rejected before paying the card. */
        simple_fixture(rush,side);test_game.players[side].monsters[0]=slot(550,side,DUEL_FACEDOWN_DEFENSE);uint32_t hash=duel_state_hash(&test_game);check(simple_activate(rush,side,0)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==hash);
        /* Destruction of a normal/quick-play source leaves its effect in the
           chain; replacing the target instance makes the effect fizzle. */
        for(unsigned replace=0;replace<2;++replace){simple_fixture(rush,side);test_game.players[side].monsters[0]=slot(550,side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[0].serial=99;test_game.players[1-side].backrow[0]=slot(316,1-side,DUEL_FACEDOWN_DEFENSE);check(simple_activate(rush,side,0)==DUEL_OK&&test_game.response_player==1-side);
            if(replace){test_game.players[side].monsters[0]=slot(550,side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[0].serial=100;flip_auto_pass();check(duel_attack_value(&test_game,side,0)==1900);}else{DuelCommand c={1-side,DUEL_ACTION_ACTIVATE_BACKROW,0,side,0};check(duel_command(&test_game,&c)==DUEL_OK&&duel_attack_value(&test_game,side,0)==2600&&!test_game.players[side].backrow[0].card);}}
        /* The response picker and AI must find and carry a real monster
           target; target-requiring traps are not silently omitted. */
        reset();test_game.turn=3;test_game.active=1-side;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[side].monsters[0]=slot(24,side,DUEL_FACEUP_ATTACK);test_game.players[1-side].monsters[0]=slot(550,1-side,DUEL_FACEUP_ATTACK);test_game.players[side].backrow[0]=slot(reinforce,side,DUEL_FACEDOWN_DEFENSE);
        check(command(DUEL_ACTION_ATTACK,0,side,0,0,0)==DUEL_OK&&test_game.response_player==side);DuelCommand ai;check(duel_ai_decide(&test_game,side,&ai)&&ai.action==DUEL_ACTION_ACTIVATE_BACKROW&&ai.target_player==side&&ai.target==0&&duel_command(&test_game,&ai)==DUEL_OK&&test_game.players[side].monsters[0].card==24&&!test_game.players[1-side].monsters[0].card);
        /* Defenses clamp after adding both temporary changes. */
        simple_fixture(snake,side);test_game.players[side].monsters[0]=slot(550,side,DUEL_FACEUP_DEFENSE);check(simple_activate(snake,side,0)==DUEL_OK&&duel_defense_value(&test_game,side,0)==400);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=simple_record(1076);check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK&&duel_defense_value(&test_game,side,0)==1100);
    }
    uint16_t deck[56];static const unsigned names[7]={1015,1016,1076,1075,1082,1415,1534};for(unsigned i=0;i<42;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<14;++i)deck[42+i]=simple_record(names[i%7]);unsigned activations=0,completed=0;
    for(unsigned seed=1;seed<=20;++seed){check(duel_start(&test_game,duel_card_definitions,deck,56,deck,56,seed+2700,seed&1)==DUEL_OK);check(duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,duel_original_fusion_deck,1,duel_original_fusion_deck,1)==DUEL_OK);check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK&&duel_life_setup(&test_game,duel_life_rules,DUEL_LIFE_RULE_COUNT)==DUEL_OK&&duel_recall_setup(&test_game,duel_recall_rules,DUEL_RECALL_RULE_COUNT)==DUEL_OK&&duel_field_setup(&test_game,duel_field_rules,DUEL_FIELD_RULE_COUNT)==DUEL_OK&&duel_aura_setup(&test_game,duel_aura_rules,DUEL_AURA_RULE_COUNT)==DUEL_OK&&duel_suppression_setup(&test_game,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT)==DUEL_OK);unsigned actions=0;
        while(test_game.winner<0&&actions++<4000){DuelCommand c;check(duel_ai_decide(&test_game,decision_player(),&c));unsigned card=c.action==DUEL_ACTION_ACTIVATE_HAND?test_game.players[c.player].hand[c.source]:c.action==DUEL_ACTION_ACTIVATE_BACKROW?test_game.players[c.player].backrow[c.source].card:0;activations+=card&&duel_card_definitions[card].effect==DUEL_E_SIMPLE;int result=duel_command(&test_game,&c);
#ifdef _WIN32
            if(result!=DUEL_OK)fprintf(stderr,"transient seed %u turn %u action %u card %u result %u\n",seed,test_game.turn,c.action,card,result);
#endif
            check(result==DUEL_OK);if(result!=DUEL_OK)break;check(owned_cards(&test_game,0)==57&&owned_cards(&test_game,1)==57);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(activations>0&&completed==20);native_transient_test_result[0]=checks-first;native_transient_test_result[1]=sizeof(original_transient_cases)/sizeof(original_transient_cases[0]);native_transient_test_result[2]=activations;native_transient_test_result[3]=completed;
}
