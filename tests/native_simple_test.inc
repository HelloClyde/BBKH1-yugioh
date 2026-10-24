/* Compare real native commands with independent original-PC outcomes. */
#include "original_simple_cases.h"
volatile uint32_t native_simple_test_result[4];
static unsigned simple_record(unsigned group){for(unsigned i=1;i<DUEL_CARD_COUNT;++i)if(duel_card_definitions[i].name_group==group)return i;return 0;}
static void simple_fixture(unsigned card,unsigned side){reset();test_game.turn=3;test_game.active=side;test_game.players[0].hand_count=test_game.players[1].hand_count=0;
    if(duel_card_definitions[card].family==DUEL_TRAP){test_game.players[side].backrow[0]=slot(card,side,DUEL_FACEDOWN_DEFENSE);test_game.players[side].backrow[0].set_turn=1;}
    else {test_game.players[side].hand_count=1;test_game.players[side].hand[0]=card;}}
static int simple_activate(unsigned card,unsigned target_side,unsigned target_slot){return command(duel_card_definitions[card].family==DUEL_TRAP?DUEL_ACTION_ACTIVATE_BACKROW:DUEL_ACTION_ACTIVATE_HAND,0,target_side,target_slot,0,0);}
static void simple_negator(unsigned card,unsigned side){unsigned other=1-side;
    if(duel_card_definitions[card].family==DUEL_TRAP){unsigned decree=0;for(unsigned i=0;i<DUEL_SUPPRESSION_RULE_COUNT;++i)if(duel_card_definitions[duel_suppression_rules[i].card].family==DUEL_TRAP)decree=duel_suppression_rules[i].card;
        check(decree!=0);test_game.players[other].backrow[4]=slot(decree,other,DUEL_FACEUP_ATTACK);test_game.players[other].backrow[4].effect_active=1;}
    else {test_game.players[other].backrow[0]=slot(893,other,DUEL_FACEDOWN_DEFENSE);test_game.players[other].hand[0]=24;test_game.players[other].hand_count=1;}}
static void simple_finish_negation(unsigned card,unsigned side){if(duel_card_definitions[card].family==DUEL_TRAP)return;
    check(test_game.response_player==1-side);DuelCommand c={1-side,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK);}
static void simple_tests(void){
    unsigned first=checks;static unsigned races[21];for(unsigned i=1;i<DUEL_CARD_COUNT;++i){const DuelCardDef *c=&duel_card_definitions[i];if(c->family==DUEL_MONSTER&&c->subtype==0&&c->effect==DUEL_E_NONE&&c->race<=20&&!races[c->race])races[c->race]=i;}
    for(unsigned i=0;i<sizeof(original_simple_races)/sizeof(original_simple_races[0]);++i){
        unsigned card=original_simple_races[i].card,side=original_simple_races[i].caster,target=original_simple_races[i].side,index=original_simple_races[i].slot,race=original_simple_races[i].race,raw=original_simple_races[i].position;
        simple_fixture(card,side);check(races[race]!=0);unsigned position=raw==1?DUEL_FACEDOWN_DEFENSE:raw==2?DUEL_FACEUP_ATTACK:DUEL_FACEUP_DEFENSE;
        test_game.players[target].monsters[index]=slot(races[race],target,position);unsigned before[2]={owned_cards(&test_game,0),owned_cards(&test_game,1)};
        check(simple_activate(card,0,0)==DUEL_OK);check((test_game.players[target].monsters[index].card==0)==(original_simple_races[i].result!=0));
        check(owned_cards(&test_game,0)==before[0]&&owned_cards(&test_game,1)==before[1]);
    }
    for(unsigned i=0;i<sizeof(original_simple_queues)/sizeof(original_simple_queues[0]);++i){
        unsigned card=original_simple_queues[i].card,side=original_simple_queues[i].caster,other=1-side;simple_fixture(card,side);
        while(test_game.players[other].hand_count<original_simple_queues[i].hand_count)test_game.players[other].hand[test_game.players[other].hand_count++]=24;
        if(original_simple_queues[i].negated)simple_negator(card,side);
        unsigned hands[2]={test_game.players[0].hand_count,test_game.players[1].hand_count};if(duel_card_definitions[card].family!=DUEL_TRAP)--hands[side];
        unsigned decks[2]={test_game.players[0].deck_count,test_game.players[1].deck_count};int life[2]={8000,8000};
        unsigned kinds[2]={original_simple_queues[i].kind1,original_simple_queues[i].kind2},sides[2]={original_simple_queues[i].side1,original_simple_queues[i].side2},values[2]={original_simple_queues[i].value1,original_simple_queues[i].value2};
        for(unsigned j=0;j<original_simple_queues[i].count;++j){unsigned target=sides[j],amount=values[j];if(kinds[j]==1){hands[target]+=amount;decks[target]-=amount;}else if(kinds[j]==2)life[target]+=amount;else if(kinds[j]==3)life[target]-=amount;}
        check(simple_activate(card,0,0)==DUEL_OK);if(original_simple_queues[i].negated){simple_finish_negation(card,side);if(duel_card_definitions[card].family!=DUEL_TRAP)--hands[other];}
        check(test_game.players[0].life==life[0]&&test_game.players[1].life==life[1]);check(test_game.players[0].hand_count==hands[0]&&test_game.players[1].hand_count==hands[1]);check(test_game.players[0].deck_count==decks[0]&&test_game.players[1].deck_count==decks[1]);
        check(test_game.players[side].grave_count==1&&test_game.players[side].grave[0]==card&&!test_game.choice.kind);
    }
    for(unsigned side=0;side<2;++side){
        unsigned heavy=simple_record(1061);simple_fixture(heavy,side);
        for(unsigned owner=0;owner<2;++owner){for(unsigned i=owner==side?1:0;i<5;++i){test_game.players[owner].backrow[i]=slot(316,owner,DUEL_FACEDOWN_DEFENSE);test_game.players[owner].backrow[i].set_turn=3;}test_game.players[owner].field=slot(duel_field_rules[0].card,owner,DUEL_FACEUP_ATTACK);test_game.players[owner].field.effect_active=1;}
        unsigned before[2]={owned_cards(&test_game,0),owned_cards(&test_game,1)};check(simple_activate(heavy,0,0)==DUEL_OK);
        for(unsigned owner=0;owner<2;++owner){check(!test_game.players[owner].field.card&&owned_cards(&test_game,owner)==before[owner]);for(unsigned i=0;i<5;++i)check(!test_game.players[owner].backrow[i].card);}
        unsigned charity=simple_record(968);simple_fixture(charity,side);unsigned count=test_game.players[side].deck_count;check(simple_activate(charity,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_DISCARD_HAND&&test_game.players[side].hand_count==3);
        DuelCommand pass={side,DUEL_ACTION_PHASE,0,0,DUEL_END};uint32_t hash=duel_state_hash(&test_game);check(duel_command(&test_game,&pass)==DUEL_EFFECT_PENDING&&duel_state_hash(&test_game)==hash);
        check(choose(side,0)==DUEL_OK&&test_game.choice.selected_count==1&&test_game.players[side].hand_count==2);check(choose(side,0)==DUEL_OK&&!test_game.choice.kind&&test_game.players[side].hand_count==1&&test_game.players[side].deck_count==count-3&&test_game.players[side].grave_count==3);
        simple_fixture(charity,side);test_game.players[side].deck_count=2;hash=duel_state_hash(&test_game);check(simple_activate(charity,0,0)==DUEL_NOT_ALLOWED&&hash==duel_state_hash(&test_game));
        for(unsigned mode=0;mode<2;++mode){unsigned card=simple_record(mode?319:1043);simple_fixture(card,side);test_game.players[1-side].monsters[2]=slot(24,1-side,mode?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK);
            check(simple_activate(card,1-side,2)==DUEL_OK&&test_game.players[1-side].monsters[2].position==(mode?DUEL_FACEUP_ATTACK:DUEL_FACEUP_DEFENSE));
            simple_fixture(card,side);hash=duel_state_hash(&test_game);check(simple_activate(card,1-side,2)==DUEL_BAD_TARGET&&hash==duel_state_hash(&test_game));}
        unsigned reveal=simple_record(349);simple_fixture(reveal,side);test_game.players[1-side].monsters[0]=slot(839,1-side,DUEL_FACEDOWN_DEFENSE);
        check(simple_activate(reveal,0,0)==DUEL_OK&&test_game.players[1-side].monsters[0].position==DUEL_FACEUP_DEFENSE&&test_game.choice.kind==DUEL_CHOICE_PENGUIN);
        unsigned reset_hand=simple_record(1221);simple_fixture(reset_hand,side);test_game.players[side].hand[test_game.players[side].hand_count++]=24;test_game.players[1-side].hand_count=3;for(unsigned i=0;i<3;++i)test_game.players[1-side].hand[i]=550;
        check(simple_activate(reset_hand,0,0)==DUEL_OK&&test_game.players[side].hand_count==1&&test_game.players[1-side].hand_count==3&&test_game.players[side].grave_count==2&&test_game.players[1-side].grave_count==3);
    }
    native_simple_test_result[0]=checks-first;native_simple_test_result[1]=sizeof(original_simple_races)/sizeof(original_simple_races[0]);native_simple_test_result[2]=sizeof(original_simple_queues)/sizeof(original_simple_queues[0]);
}
