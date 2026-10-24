/* Explicit CPU-model fixtures; no firmware or physical RAM is modified. */
#include "original_recall_cases.h"
volatile uint32_t native_recall_test_result[6];
static void recall_fixture(unsigned source,unsigned target){
    reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(source,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].grave_count=1;test_game.players[0].grave[0]=target;test_game.players[0].grave_serial[0]=++test_game.next_serial;
}
static void hold_recall_response(void){test_game.players[0].backrow[0]=slot(851,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;}
static void grave_recall_tests(void){
    unsigned initial=checks,hand,grave_count;uint32_t before,random;DuelChoiceOption options[DUEL_OPTION_MAX];DuelCommand c;
    /* Compare actual native legal choices to the original selector cases,
       including no target, wrong family, and a matching opposing grave. */
    for(unsigned n=0;n<RECALL_REFERENCE_COUNT;++n){const RecallReference *r=&recall_reference_cases[n];reset();test_game.turn=3;test_game.active=r->side;DuelPlayer *p=&test_game.players[r->side];p->monsters[0]=slot(r->source,r->side,DUEL_FACEDOWN_DEFENSE);p->grave_count=r->count;
        for(unsigned i=0;i<r->count;++i){p->grave[i]=r->grave[i];p->grave_serial[i]=++test_game.next_serial;}
        test_game.players[1-r->side].grave_count=1;test_game.players[1-r->side].grave[0]=r->family==DUEL_SPELL?300:894;
        hand=p->hand_count;random=test_game.random_state;check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);
        if(!r->expected){check(!test_game.choice.kind&&p->hand_count==hand&&p->grave_count==r->count);continue;}
        check(test_game.choice.kind==DUEL_CHOICE_GRAVE_RECALL&&test_game.choice.player==r->side&&duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==r->expected);
        unsigned at=0;for(unsigned i=0;i<r->count;++i)if(duel_card_definitions[r->grave[i]].family==r->family){check(options[at].index==i&&options[at].card==r->grave[i]&&options[at].kind==DUEL_OPTION_CARD);++at;}
        before=duel_state_hash(&test_game);check(choose(r->side,DUEL_NO_SLOT)==DUEL_BAD_TARGET&&choose(1-r->side,options[0].index)==DUEL_EFFECT_PENDING&&duel_state_hash(&test_game)==before);
        unsigned target=options[0].card;check(choose(r->side,options[0].index)==DUEL_OK&&!test_game.choice.kind&&!test_game.chain_count&&p->hand_count==hand+1&&p->hand[hand]==target&&p->grave_count==r->count-1&&test_game.random_state==random);
    }
    for(unsigned which=0;which<2;++which){unsigned source=which?1016:471,target=which?894:300,other=which?851:359;
        /* A battle-destroyed source recovers first, then enters the grave. */
        recall_fixture(source,target);test_game.active=1;test_game.phase=DUEL_BATTLE;test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);hand=test_game.players[0].hand_count;
        check(command(DUEL_ACTION_ATTACK,0,0,0,0,0)==DUEL_OK&&test_game.battle_pending&&test_game.choice.kind==DUEL_CHOICE_GRAVE_RECALL&&test_game.players[0].monsters[0].card==source);
        check(choose(0,0)==DUEL_OK&&!test_game.battle_pending&&!test_game.players[0].monsters[0].card&&test_game.players[0].hand_count==hand+1&&test_game.players[0].hand[hand]==target&&test_game.players[0].grave_count==1&&test_game.players[0].grave[0]==source);
        /* Control, not original ownership, determines the recovering grave. */
        recall_fixture(source,target);test_game.players[0].monsters[0].owner=1;hand=test_game.players[0].hand_count;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.choice.player==0&&choose(0,0)==DUEL_OK&&test_game.players[0].hand_count==hand+1);
        /* The source may leave during the response chain without erasing the
           already activated recovery. */
        recall_fixture(source,target);test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;hand=test_game.players[0].hand_count;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,0)==DUEL_OK&&test_game.response_player==1);
        c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].monsters[0].card&&test_game.players[0].grave[0]==source&&test_game.players[0].hand[hand]==target);
        /* A prior removal shifts the selected instance; resolution follows
           its serial rather than the old grave index. */
        recall_fixture(source,other);test_game.players[0].grave[1]=target;test_game.players[0].grave_serial[1]=++test_game.next_serial;test_game.players[0].grave_count=2;hold_recall_response();hand=test_game.players[0].hand_count;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,1)==DUEL_OK&&test_game.response_player==1);
        test_game.players[0].hand[test_game.players[0].hand_count++]=other;test_game.players[0].grave[0]=target;test_game.players[0].grave_serial[0]=test_game.players[0].grave_serial[1];test_game.players[0].grave[1]=0;test_game.players[0].grave_serial[1]=0;test_game.players[0].grave_count=1;
        c=(DuelCommand){1,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==hand+2&&test_game.players[0].hand[hand+1]==target&&!test_game.players[0].grave_count);
        /* Removing and re-entering the same card is a different target. */
        recall_fixture(source,target);hold_recall_response();hand=test_game.players[0].hand_count;check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,0)==DUEL_OK&&test_game.chain_count==1);
        test_game.players[0].grave_serial[0]=++test_game.next_serial;c=(DuelCommand){1,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==hand&&test_game.players[0].grave_count==1);
        /* Negation and capacity changes cannot create or discard a target. */
        recall_fixture(source,target);hold_recall_response();hand=test_game.players[0].hand_count;check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,0)==DUEL_OK&&test_game.chain_count==1);test_game.chain[0].negated=1;c=(DuelCommand){1,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==hand&&test_game.players[0].grave_count==1);
        recall_fixture(source,target);test_game.players[0].hand_count=DUEL_HAND_MAX;for(unsigned i=0;i<DUEL_HAND_MAX;++i)test_game.players[0].hand[i]=24;check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&!test_game.choice.kind&&test_game.players[0].grave_count==1);
        /* Largest representable grave stresses choice-array capacity. This
           deliberately oversized fixture is not claimed as a legal deck. */
        recall_fixture(source,target);test_game.players[0].grave_count=DUEL_PILE_MAX;for(unsigned i=0;i<DUEL_PILE_MAX;++i){test_game.players[0].grave[i]=target;test_game.players[0].grave_serial[i]=++test_game.next_serial;}
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);struct {unsigned guard1;DuelChoiceOption o[2];unsigned guard2;} bounded={0x11223344,{{0}},0xaabbccdd};
        check(duel_choice_options(&test_game,bounded.o,2)==DUEL_PILE_MAX&&bounded.guard1==0x11223344&&bounded.guard2==0xaabbccdd);hand=test_game.players[0].hand_count;
        check(choose(0,DUEL_PILE_MAX-1)==DUEL_OK&&test_game.players[0].hand_count==hand+1&&test_game.players[0].grave_count==DUEL_PILE_MAX-1);
    }
    /* Reject malformed rule data atomically before exposing a bad chooser. */
    reset();DuelRecallRule invalid[2]={{471,DUEL_SPELL},{471,DUEL_TRAP}};before=duel_state_hash(&test_game);check(duel_recall_setup(&test_game,invalid,2)==DUEL_BAD_ARGUMENT&&duel_state_hash(&test_game)==before);
    native_recall_test_result[0]=checks-initial;
    uint16_t deck[44];for(unsigned i=0;i<42;++i)deck[i]=duel_original_deck[i];deck[42]=471;deck[43]=1016;
    for(unsigned seed=1;seed<=30;++seed){
        check(duel_start(&test_game,duel_card_definitions,deck,44,deck,44,seed+900,seed&1)==DUEL_OK);
        check(duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,duel_original_fusion_deck,1,duel_original_fusion_deck,1)==DUEL_OK);check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK&&duel_life_setup(&test_game,duel_life_rules,DUEL_LIFE_RULE_COUNT)==DUEL_OK&&duel_recall_setup(&test_game,duel_recall_rules,DUEL_RECALL_RULE_COUNT)==DUEL_OK);
        unsigned actions=0;while(test_game.winner<0&&actions++<3000){DuelCommand action;check(duel_ai_decide(&test_game,decision_player(),&action));
            if(action.action==DUEL_ACTION_CHOOSE&&test_game.choice.kind==DUEL_CHOICE_GRAVE_RECALL)++native_recall_test_result[test_game.choice.card==471?2:3];
            if(action.action==DUEL_ACTION_SET_MONSTER&&test_game.cards[test_game.players[action.player].hand[action.source]].effect==DUEL_E_GRAVE_RECALL)++native_recall_test_result[4];
            check(duel_command(&test_game,&action)==DUEL_OK);check(owned_cards(&test_game,0)==45&&owned_cards(&test_game,1)==45);
        }check(test_game.winner>=0);if(test_game.winner>=0)++native_recall_test_result[1];
    }
    check(native_recall_test_result[1]==30&&native_recall_test_result[2]&&native_recall_test_result[3]);native_recall_test_result[5]=checks-initial;
}
