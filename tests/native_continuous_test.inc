#include "original_continuous_cases.h"
#include "duel_continuous_data.h"
#ifdef _WIN32
#include <stdio.h>
#endif
volatile uint32_t native_continuous_test_result[4];
static DuelCardDef continuous_test_cards[DUEL_CARD_COUNT];
static void continuous_place(unsigned side,unsigned card,unsigned enabled){test_game.players[side].backrow[0]=slot(card,side,DUEL_FACEUP_ATTACK);test_game.players[side].backrow[0].effect_active=enabled;}
static void continuous_tests(void){unsigned first=checks;for(unsigned i=0;i<DUEL_CARD_COUNT;++i)continuous_test_cards[i]=duel_card_definitions[i];
    /* Match the original oracle's explicit immutable level/race inputs. Only
       this parameter-gate fixture substitutes metadata; all cards and actual
       commands below otherwise use the generated original definitions. */
    for(unsigned ci=0;ci<sizeof(original_continuous_attack_cases)/sizeof(original_continuous_attack_cases[0]);++ci){const unsigned *r=original_continuous_attack_cases[ci];unsigned card=r[0],side=r[1],owner=r[2],enabled=r[3],level=r[4],race=r[5],door=r[6],allowed=r[7];reset();test_game.turn=3;test_game.active=side;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.cards=continuous_test_cards;continuous_test_cards[550].level=level;continuous_test_cards[550].race=race;
        test_game.players[side].monsters[0]=slot(550,side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[0].serial=99;test_game.battle_attacker[side]=door==1?99:door==2?101:0;continuous_place(owner,card,enabled);
        uint32_t before=duel_state_hash(&test_game);int result=command(DUEL_ACTION_ATTACK,0,1-side,DUEL_NO_SLOT,0,0);check(result==(allowed?DUEL_OK:DUEL_NOT_ALLOWED));if(!allowed)check(before==duel_state_hash(&test_game));else check(test_game.players[1-side].life==6100);
    }
    for(unsigned ci=0;ci<sizeof(original_hand_limit_cases)/sizeof(original_hand_limit_cases[0]);++ci){const unsigned *r=original_hand_limit_cases[ci];unsigned side=r[0];reset();test_game.turn=3;test_game.active=side;continuous_place(1-side,simple_record(1427),r[1]);test_game.players[side].hand_count=r[2];for(unsigned i=0;i<r[2];++i)test_game.players[side].hand[i]=550;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check((test_game.discard_required!=0)==(r[3]!=0));if(!r[3])check(test_game.active==1-side&&test_game.turn==4);
    }
    for(unsigned side=0;side<2;++side){
        for(unsigned kind=0;kind<2;++kind){unsigned spell=simple_record(kind?347:1230),duration=kind?3:2;simple_fixture(spell,side);check(simple_activate(spell,0,0)==DUEL_OK&&test_game.players[side].backrow[0].effect_active&&test_game.players[side].backrow[0].position_lock_until==3+duration*2-1);
            for(unsigned turn=3;turn<=3+duration*2-1;++turn){check(test_game.turn==turn&&test_game.players[side].backrow[0].card==spell);test_game.players[test_game.active].hand_count=0;test_game.phase=DUEL_MAIN1;check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);}
            check(!test_game.players[side].backrow[0].card&&test_game.players[side].grave_count==1&&test_game.players[side].grave[0]==spell);}
        /* Swords reveals face-down monsters, resolves their flip effects and
           only prevents the opposite side from declaring attacks. */
        simple_fixture(simple_record(347),side);test_game.players[1-side].monsters[0]=slot(simple_record(539),1-side,DUEL_FACEDOWN_DEFENSE);unsigned count=test_game.players[1-side].deck_count;check(simple_activate(simple_record(347),0,0)==DUEL_OK&&test_game.players[1-side].monsters[0].position==DUEL_FACEUP_DEFENSE&&test_game.players[1-side].deck_count==count-1);
        /* Gravity Bind can cancel an attack already declared; destroying the
           continuous source frees the monster without resetting its level. */
        reset();test_game.turn=3;test_game.active=1-side;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[1-side].monsters[0]=slot(550,1-side,DUEL_FACEUP_ATTACK);test_game.players[side].backrow[0]=slot(simple_record(1323),side,DUEL_FACEDOWN_DEFENSE);
        check(command(DUEL_ACTION_ATTACK,0,side,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.response_player==side);DuelCommand c={side,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[side].life==8000&&test_game.players[side].backrow[0].effect_active);
        /* Dark Door permits both attacks of one Hayabusa but bars a second
           monster, even if the first attack was negated. */
        reset();test_game.turn=3;test_game.active=side;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;continuous_place(1-side,simple_record(1538),1);test_game.players[side].monsters[0]=slot(simple_record(1336),side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[1]=slot(550,side,DUEL_FACEUP_ATTACK);
        check(command(DUEL_ACTION_ATTACK,0,1-side,DUEL_NO_SLOT,0,0)==DUEL_OK);check(command(DUEL_ACTION_ATTACK,0,1-side,DUEL_NO_SLOT,0,0)==DUEL_OK);uint32_t before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ATTACK,1,1-side,DUEL_NO_SLOT,0,0)==DUEL_NOT_ALLOWED&&before==duel_state_hash(&test_game));
        /* Mask forbids Tribute Summons and ritual payments, including a Mask
           activated as the response to a legal ritual spell. */
        unsigned high=0;for(unsigned i=1;i<DUEL_CARD_COUNT;++i)if(duel_card_definitions[i].family==DUEL_MONSTER&&duel_card_definitions[i].subtype==0&&duel_card_definitions[i].level>=7){high=i;break;}
        reset();test_game.active=side;test_game.turn=3;continuous_place(1-side,simple_record(1418),1);test_game.players[side].hand_count=1;test_game.players[side].hand[0]=high;for(unsigned i=0;i<2;++i)test_game.players[side].monsters[i]=slot(550,side,DUEL_FACEUP_ATTACK);before=duel_state_hash(&test_game);check(command(DUEL_ACTION_SUMMON,0,0,0,0,3)==DUEL_NOT_ALLOWED&&before==duel_state_hash(&test_game));
        ritual_fixture(side,simple_record(669),simple_record(363));test_game.players[side].hand[test_game.players[side].hand_count++]=simple_record(363);continuous_place(1-side,simple_record(1418),1);before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_NOT_ALLOWED&&before==duel_state_hash(&test_game));
        ritual_fixture(side,simple_record(669),simple_record(363));test_game.players[side].hand[test_game.players[side].hand_count++]=simple_record(363);test_game.players[1-side].backrow[0]=slot(simple_record(1418),1-side,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1-side);c=(DuelCommand){1-side,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.choice.kind&&test_game.players[side].hand_count==2&&test_game.players[side].grave_count==1);
    }
    /* Mixed full games exercise the AI's activation and attack decisions
       under persistent restrictions, including stall and deck-out endings. */
    uint16_t deck[56];for(unsigned i=0;i<42;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<14;++i)deck[42+i]=simple_record(continuous_rules[i%DUEL_CONTINUOUS_RULE_COUNT].name);unsigned activations=0,completed=0;
    for(unsigned seed=1;seed<=20;++seed){check(duel_start(&test_game,duel_card_definitions,deck,56,deck,56,seed+2200,seed&1)==DUEL_OK);check(duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,duel_original_fusion_deck,1,duel_original_fusion_deck,1)==DUEL_OK);
        check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK&&duel_life_setup(&test_game,duel_life_rules,DUEL_LIFE_RULE_COUNT)==DUEL_OK&&duel_recall_setup(&test_game,duel_recall_rules,DUEL_RECALL_RULE_COUNT)==DUEL_OK&&duel_field_setup(&test_game,duel_field_rules,DUEL_FIELD_RULE_COUNT)==DUEL_OK&&duel_aura_setup(&test_game,duel_aura_rules,DUEL_AURA_RULE_COUNT)==DUEL_OK&&duel_suppression_setup(&test_game,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT)==DUEL_OK);
        unsigned actions=0;while(test_game.winner<0&&actions++<4000){DuelCommand c;check(duel_ai_decide(&test_game,decision_player(),&c));unsigned card=c.action==DUEL_ACTION_ACTIVATE_HAND?test_game.players[c.player].hand[c.source]:c.action==DUEL_ACTION_ACTIVATE_BACKROW?test_game.players[c.player].backrow[c.source].card:0;activations+=card&&duel_card_definitions[card].effect==DUEL_E_CONTINUOUS;int result=duel_command(&test_game,&c);
#ifdef _WIN32
            if(result!=DUEL_OK)fprintf(stderr,"continuous seed %u turn %u action %u card %u result %u\n",seed,test_game.turn,c.action,card,result);
#endif
            check(result==DUEL_OK);if(result!=DUEL_OK)break;
            check(owned_cards(&test_game,0)==57&&owned_cards(&test_game,1)==57);}
        check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(completed==20&&activations>0);
    native_continuous_test_result[0]=checks-first;native_continuous_test_result[1]=sizeof(original_continuous_attack_cases)/sizeof(original_continuous_attack_cases[0]);native_continuous_test_result[2]=sizeof(original_hand_limit_cases)/sizeof(original_hand_limit_cases[0]);native_continuous_test_result[3]=completed;
}
