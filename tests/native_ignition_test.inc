#include "original_ignition_cases.h"
#include "duel_ignition_data.h"
volatile uint32_t native_ignition_test_result[4];
static DuelCardDef ignition_test_cards[DUEL_CARD_COUNT];
static void ignition_tests(void){unsigned first=checks;for(unsigned i=0;i<DUEL_CARD_COUNT;++i)ignition_test_cards[i]=duel_card_definitions[i];
    for(unsigned ci=0;ci<sizeof(original_ignition_cases)/sizeof(original_ignition_cases[0]);++ci){const unsigned *v=original_ignition_cases[ci];unsigned card=v[0],side=v[1],index=v[2],attack=v[3],amount=v[4];const IgnitionRule *r=0;
        for(unsigned j=0;j<DUEL_IGNITION_RULE_COUNT;++j)if(ignition_rules[j].name==duel_card_definitions[card].name_group)r=&ignition_rules[j];check(r!=0);if(!r)continue;
        reset();test_game.turn=3;test_game.active=side;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.cards=ignition_test_cards;ignition_test_cards[550].attack=attack;
        DuelPlayer *p=&test_game.players[side];p->monsters[index]=slot(card,side,DUEL_FACEUP_ATTACK);p->monsters[index].serial=99;unsigned mask=0;
        for(unsigned j=0;j<r->tributes;++j){unsigned slot_index=(index+1+j)%5;p->monsters[slot_index]=slot(r->op==2?550:24,side,j?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_DEFENSE);mask|=1u<<slot_index;}
        unsigned before0=owned_cards(&test_game,0),before1=owned_cards(&test_game,1);int before_attack=duel_attack_value(&test_game,side,index),before_defense=duel_defense_value(&test_game,side,index);check(duel_monster_tribute_cost(&duel_card_definitions[card])==r->tributes);
        check(command(DUEL_ACTION_ACTIVATE_MONSTER,index,0,0,0,mask)==DUEL_OK&&!test_game.chain_count&&!test_game.choice.kind);check(p->grave_count==r->tributes&&!p->normal_summon_used);
        if(r->op==1||r->op==2)check(test_game.players[1-side].life==8000-(int)amount);if(r->op==4)check(p->life==8000+(int)amount);
        if(r->op==3){check(duel_attack_value(&test_game,side,index)==before_attack+(int)amount&&duel_defense_value(&test_game,side,index)==before_defense);check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK&&duel_attack_value(&test_game,side,index)==before_attack);}
        check(owned_cards(&test_game,0)==before0&&owned_cards(&test_game,1)==before1);
    }
    for(unsigned side=0;side<2;++side){unsigned cannon=simple_record(511),little=simple_record(261);
        for(unsigned kind=0;kind<5;++kind){reset();test_game.turn=3;test_game.active=side;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[side].monsters[0]=slot(kind==1?little:cannon,side,kind==2?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK);unsigned mask=kind==0?0:1;int expected=kind==0?DUEL_NEED_TRIBUTES:kind==1?DUEL_BAD_TARGET:kind==2?DUEL_NOT_ALLOWED:kind==3?DUEL_WRONG_PHASE:DUEL_NOT_ALLOWED;
            if(kind==3)test_game.phase=DUEL_BATTLE;if(kind==4)continuous_place(1-side,simple_record(1418),1);uint32_t before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,mask)==expected&&duel_state_hash(&test_game)==before);}
        /* Cannon Soldier can tribute itself. Magic Jammer has no response
           to a monster effect and is not discarded as a fake counter. */
        reset();test_game.turn=3;test_game.active=side;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[side].monsters[0]=slot(cannon,side,DUEL_FACEUP_ATTACK);test_game.players[1-side].backrow[0]=slot(893,1-side,DUEL_FACEDOWN_DEFENSE);test_game.players[1-side].hand_count=1;test_game.players[1-side].hand[0]=24;
        check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,1)==DUEL_OK&&!test_game.players[side].monsters[0].card&&test_game.players[1-side].life==7500&&test_game.players[1-side].backrow[0].card==893);
        /* A paid Sangan tribute triggers after the damage effect resolves. */
        reset();test_game.turn=3;test_game.active=side;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[side].monsters[0]=slot(cannon,side,DUEL_FACEUP_ATTACK);unsigned sangan=0;for(unsigned i=1;i<DUEL_CARD_COUNT;++i)if(duel_card_definitions[i].effect==DUEL_E_SANGAN)sangan=i;test_game.players[side].monsters[1]=slot(sangan,side,DUEL_FACEUP_DEFENSE);
        check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,2)==DUEL_OK&&test_game.players[1-side].life==7500&&test_game.choice.kind==DUEL_CHOICE_SEARCH_ATTACK);DuelCommand ai;check(duel_ai_decide(&test_game,side,&ai)&&duel_command(&test_game,&ai)==DUEL_OK&&!test_game.choice.kind);
        /* Repeated Little Swordsman payments only increase ATK; leave effects
           keep the instance-associated bonus until the turn ends. */
        reset();test_game.turn=3;test_game.active=side;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[side].monsters[0]=slot(little,side,DUEL_FACEUP_ATTACK);for(unsigned i=1;i<=2;++i)test_game.players[side].monsters[i]=slot(24,side,DUEL_FACEUP_ATTACK);int power=duel_attack_value(&test_game,side,0);
        check(command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,2)==DUEL_OK&&command(DUEL_ACTION_ACTIVATE_MONSTER,0,0,0,0,4)==DUEL_OK&&duel_attack_value(&test_game,side,0)==power+1400&&test_game.players[side].monsters[0].temporary_attack==1400);
    }
    uint16_t deck[52];for(unsigned i=0;i<42;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<10;++i)deck[42+i]=simple_record(ignition_rules[i%DUEL_IGNITION_RULE_COUNT].name);unsigned activations=0,completed=0;
    for(unsigned seed=1;seed<=20;++seed){check(duel_start(&test_game,duel_card_definitions,deck,52,deck,52,seed+2500,seed&1)==DUEL_OK);check(duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,duel_original_fusion_deck,1,duel_original_fusion_deck,1)==DUEL_OK);check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK&&duel_life_setup(&test_game,duel_life_rules,DUEL_LIFE_RULE_COUNT)==DUEL_OK&&duel_recall_setup(&test_game,duel_recall_rules,DUEL_RECALL_RULE_COUNT)==DUEL_OK&&duel_field_setup(&test_game,duel_field_rules,DUEL_FIELD_RULE_COUNT)==DUEL_OK&&duel_aura_setup(&test_game,duel_aura_rules,DUEL_AURA_RULE_COUNT)==DUEL_OK&&duel_suppression_setup(&test_game,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT)==DUEL_OK);unsigned actions=0;
        while(test_game.winner<0&&actions++<4000){DuelCommand c;check(duel_ai_decide(&test_game,decision_player(),&c));activations+=c.action==DUEL_ACTION_ACTIVATE_MONSTER;int result=duel_command(&test_game,&c);check(result==DUEL_OK);if(result!=DUEL_OK)break;check(owned_cards(&test_game,0)==53&&owned_cards(&test_game,1)==53);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(activations>0&&completed==20);native_ignition_test_result[0]=checks-first;native_ignition_test_result[1]=sizeof(original_ignition_cases)/sizeof(original_ignition_cases[0]);native_ignition_test_result[2]=activations;native_ignition_test_result[3]=completed;
}
