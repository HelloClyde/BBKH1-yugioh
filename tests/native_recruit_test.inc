#include "original_recruit_cases.h"
#include "duel_recruit_data.h"
volatile uint32_t native_recruit_test_result[5];
static void recruit_tests(void){unsigned first=checks,skipped=0;reset();
    for(unsigned i=0;i<sizeof(original_recruit_cases)/sizeof(original_recruit_cases[0]);++i){const unsigned *v=original_recruit_cases[i];unsigned source=v[0],side=v[1],card=v[2],expected=v[3];if(!duel_card_supported(&duel_card_definitions[card])){skipped+=expected;expected=0;}
        test_game.choice=(DuelTrigger){DUEL_CHOICE_RECRUIT,side,source,0};test_game.players[side].deck_count=1;test_game.players[side].deck[0]=card;DuelChoiceOption options[2];unsigned n=duel_choice_options(&test_game,options,2);
#ifdef _WIN32
        if(n!=(expected?2u:0u))fprintf(stderr,"recruit predicate source %u side %u card %u expected %u got %u\n",source,side,card,expected,n);
#endif
        check(n==(expected?2u:0u));if(expected)check(options[0].card==card&&options[0].index==0&&options[1].kind==DUEL_OPTION_SKIP);}
    for(unsigned r=0;r<DUEL_RECRUIT_RULE_COUNT;++r)for(unsigned side=0;side<2;++side){unsigned source=simple_record(recruit_rules[r].name);reset();test_game.turn=3;test_game.active=1-side;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[side].deck_count=1;test_game.players[side].deck[0]=source;
        test_game.players[side].monsters[0]=slot(source,side,DUEL_FACEDOWN_DEFENSE);test_game.players[1-side].monsters[0]=slot(550,1-side,DUEL_FACEUP_ATTACK);test_game.players[1-side].backrow[0]=slot(911,1-side,DUEL_FACEDOWN_DEFENSE); /* overwritten with actual Torrential below */
        unsigned torrential=0;for(unsigned i=1;i<DUEL_CARD_COUNT;++i)if(duel_card_definitions[i].effect==DUEL_E_TORRENTIAL)torrential=i;test_game.players[1-side].backrow[0]=slot(torrential,1-side,DUEL_FACEDOWN_DEFENSE);unsigned before0=owned_cards(&test_game,0),before1=owned_cards(&test_game,1);
        check(command(DUEL_ACTION_ATTACK,0,side,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_RECRUIT&&test_game.choice.player==side&&!test_game.players[side].monsters[0].card&&test_game.players[side].grave[0]==source);
        check(choose(side,0)==DUEL_OK&&!test_game.choice.kind&&!test_game.chain_count&&test_game.players[side].monsters[0].card==source&&test_game.players[side].monsters[0].position==DUEL_FACEUP_ATTACK&&!test_game.players[side].normal_summon_used&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[1-side].backrow[0].card==torrential);
        check(owned_cards(&test_game,0)==before0&&owned_cards(&test_game,1)==before1);
        /* Effect destruction does not recruit. The choice is optional and
           an empty/full board cannot overflow a monster zone. */
        simple_fixture(802,side);test_game.players[side].monsters[0]=slot(source,side,DUEL_FACEUP_ATTACK);test_game.players[side].deck_count=1;test_game.players[side].deck[0]=source;check(simple_activate(802,0,0)==DUEL_OK&&!test_game.choice.kind&&!test_game.players[side].monsters[0].card&&test_game.players[side].deck_count==1);
        reset();test_game.turn=3;test_game.active=side;test_game.choice=(DuelTrigger){DUEL_CHOICE_RECRUIT,side,source,0};test_game.players[side].deck_count=2;test_game.players[side].deck[0]=test_game.players[side].deck[1]=source;check(choose(side,DUEL_NO_SLOT)==DUEL_OK&&!test_game.choice.kind&&test_game.players[side].deck_count==2&&!test_game.players[side].monsters[0].card);
        test_game.choice=(DuelTrigger){DUEL_CHOICE_RECRUIT,side,source,0};for(unsigned i=0;i<5;++i)test_game.players[side].monsters[i]=slot(550,side,DUEL_FACEUP_ATTACK);check(!duel_choice_options(&test_game,0,0));
    }
    /* A stolen recruiter activates for its graveyard owner, not the player
       who controlled it when battle destroyed it. */
    for(unsigned owner=0;owner<2;++owner){unsigned source=simple_record(1108);reset();test_game.turn=3;test_game.active=owner;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[owner].deck_count=1;test_game.players[owner].deck[0]=source;test_game.players[owner].monsters[0]=slot(550,owner,DUEL_FACEUP_ATTACK);test_game.players[1-owner].monsters[0]=slot(source,owner,DUEL_FACEUP_ATTACK);
        check(command(DUEL_ACTION_ATTACK,0,1-owner,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_RECRUIT&&test_game.choice.player==owner&&test_game.players[owner].grave[0]==source);check(choose(owner,0)==DUEL_OK&&test_game.players[owner].monsters[1].card==source&&!test_game.players[1-owner].monsters[0].card);}
    uint16_t deck[54];for(unsigned i=0;i<42;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<12;++i)deck[42+i]=simple_record(recruit_rules[i%DUEL_RECRUIT_RULE_COUNT].name);unsigned choices=0,completed=0;
    for(unsigned seed=1;seed<=20;++seed){check(duel_start(&test_game,duel_card_definitions,deck,54,deck,54,seed+2900,seed&1)==DUEL_OK);check(duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,duel_original_fusion_deck,1,duel_original_fusion_deck,1)==DUEL_OK);check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK&&duel_life_setup(&test_game,duel_life_rules,DUEL_LIFE_RULE_COUNT)==DUEL_OK&&duel_recall_setup(&test_game,duel_recall_rules,DUEL_RECALL_RULE_COUNT)==DUEL_OK&&duel_field_setup(&test_game,duel_field_rules,DUEL_FIELD_RULE_COUNT)==DUEL_OK&&duel_aura_setup(&test_game,duel_aura_rules,DUEL_AURA_RULE_COUNT)==DUEL_OK&&duel_suppression_setup(&test_game,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT)==DUEL_OK);unsigned actions=0;
        while(test_game.winner<0&&actions++<4000){DuelCommand c;check(duel_ai_decide(&test_game,decision_player(),&c));choices+=c.action==DUEL_ACTION_CHOOSE&&test_game.choice.kind==DUEL_CHOICE_RECRUIT;int result=duel_command(&test_game,&c);
#ifdef _WIN32
            if(result!=DUEL_OK)fprintf(stderr,"recruit seed %u turn %u action %u result %u\n",seed,test_game.turn,c.action,result);
#endif
            check(result==DUEL_OK);if(result!=DUEL_OK)break;check(owned_cards(&test_game,0)==55&&owned_cards(&test_game,1)==55);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(choices>0&&completed==20);native_recruit_test_result[0]=checks-first;native_recruit_test_result[1]=sizeof(original_recruit_cases)/sizeof(original_recruit_cases[0]);native_recruit_test_result[2]=skipped;native_recruit_test_result[3]=choices;native_recruit_test_result[4]=completed;
}
