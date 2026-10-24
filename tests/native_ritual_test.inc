#include "original_ritual_cases.h"
volatile uint32_t native_ritual_test_result[4];
static void ritual_fixture(unsigned side,unsigned spell,unsigned monster){reset();test_game.turn=3;test_game.active=side;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[side].hand[0]=spell;test_game.players[side].hand[1]=monster;test_game.players[side].hand_count=2;}
static void ritual_tests(void){unsigned first=checks,valid=0;
    for(unsigned i=0;i<sizeof(original_ritual_cases)/sizeof(original_ritual_cases[0]);++i){const OriginalRitualCase *r=&original_ritual_cases[i];ritual_fixture(r->side,r->spell,r->monster);DuelPlayer *p=&test_game.players[r->side];
        p->hand_count=r->hand_count;for(unsigned j=0;j<r->hand_count;++j)p->hand[j]=r->hand[j];for(unsigned j=0;j<r->field_count;++j)p->monsters[j]=slot(r->field[j],r->side,r->position);p->hand[p->hand_count++]=r->spell;
        unsigned owned0=owned_cards(&test_game,0),owned1=owned_cards(&test_game,1);uint32_t before=duel_state_hash(&test_game);int result=command(DUEL_ACTION_ACTIVATE_HAND,p->hand_count-1,0,0,0,0);
        if(!r->allowed){check(result==DUEL_NOT_ALLOWED&&before==duel_state_hash(&test_game));continue;}
        ++valid;check(result==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_RITUAL_MONSTER&&test_game.chain_count==1);DuelChoiceOption options[DUEL_OPTION_MAX];unsigned count=duel_choice_options(&test_game,options,DUEL_OPTION_MAX);check(count>0);if(!count)continue;
        check(options[0].card==r->monster&&choose(r->side,options[0].index)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_RITUAL_MATERIAL);
        /* A phase command cannot escape mandatory payment. Duplicate material
           tokens are rejected without moving cards or changing state. */
        before=duel_state_hash(&test_game);check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_EFFECT_PENDING&&before==duel_state_hash(&test_game));unsigned steps=0;
        while(test_game.choice.kind==DUEL_CHOICE_RITUAL_MATERIAL&&steps++<DUEL_HAND_MAX+5){count=duel_choice_options(&test_game,options,DUEL_OPTION_MAX);check(count>0);if(!count)break;unsigned token=options[0].index;check(choose(r->side,token)==DUEL_OK);
            if(test_game.choice.kind==DUEL_CHOICE_RITUAL_MATERIAL){before=duel_state_hash(&test_game);check(choose(r->side,token)==DUEL_BAD_TARGET&&before==duel_state_hash(&test_game));}}
        check(test_game.choice.kind==DUEL_CHOICE_RITUAL_POSITION&&steps<DUEL_HAND_MAX+5);unsigned position=i&1;check(choose(r->side,position)==DUEL_OK);
        int found=-1;for(unsigned j=0;j<5;++j)if(p->monsters[j].card==r->monster&&p->monsters[j].properly_summoned)found=j;
        check(found>=0&&p->monsters[found].position==position&&!p->normal_summon_used&&!test_game.choice.kind&&!test_game.chain_count&&!test_game.resolving);
        check(owned_cards(&test_game,0)==owned0&&owned_cards(&test_game,1)==owned1);unsigned paid=0;for(unsigned j=0;j<p->grave_count;++j)if(test_game.cards[p->grave[j]].family==DUEL_MONSTER)paid+=test_game.cards[p->grave[j]].level;check(paid>=r->level);
    }
    unsigned spell=simple_record(669),monster=simple_record(363),low=simple_record(322),four=simple_record(5); /* actual IDs looked up below */
    for(unsigned i=1;i<DUEL_CARD_COUNT;++i)if(duel_card_definitions[i].family==DUEL_MONSTER&&duel_card_definitions[i].subtype==0&&duel_card_definitions[i].name_group<1920){if(duel_card_definitions[i].level==1)low=i;if(duel_card_definitions[i].level==4)four=i;}
    check(spell&&monster&&low&&four);
    for(unsigned side=0;side<2;++side){
        /* More than three tributes, including face-down field monsters. */
        ritual_fixture(side,spell,monster);DuelPlayer *p=&test_game.players[side];for(unsigned i=0;i<5;++i)p->monsters[i]=slot(low,side,DUEL_FACEDOWN_DEFENSE);for(unsigned i=0;i<3;++i)p->hand[p->hand_count++]=low;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&choose(side,0)==DUEL_OK);for(unsigned i=0;i<5;++i)check(choose(side,DUEL_HAND_MAX+i)==DUEL_OK);for(unsigned i=1;i<=3;++i)check(choose(side,i)==DUEL_OK);check(test_game.choice.kind==DUEL_CHOICE_RITUAL_POSITION&&test_game.ritual_task.count==8);check(choose(side,DUEL_FACEUP_DEFENSE)==DUEL_OK&&p->monsters[0].card==monster&&p->grave_count==9&&!p->hand_count);
        /* A full board requires at least one field tribute; spending hand
           monsters first cannot strand resolution with all five zones full. */
        ritual_fixture(side,spell,monster);p=&test_game.players[side];for(unsigned i=0;i<5;++i)p->monsters[i]=slot(low,side,DUEL_FACEUP_ATTACK);p->hand[p->hand_count++]=four;p->hand[p->hand_count++]=four;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&choose(side,0)==DUEL_OK);check(choose(side,1)==DUEL_OK);uint32_t before=duel_state_hash(&test_game);check(choose(side,2)==DUEL_BAD_TARGET&&before==duel_state_hash(&test_game));check(choose(side,DUEL_HAND_MAX)==DUEL_OK&&choose(side,2)==DUEL_OK&&choose(side,0)==DUEL_OK&&p->monsters[0].card==monster);
        /* Negation pays no materials and leaves the ritual monster in hand. */
        ritual_fixture(side,spell,monster);p=&test_game.players[side];p->hand[p->hand_count++]=monster;test_game.players[1-side].backrow[0]=slot(893,1-side,DUEL_FACEDOWN_DEFENSE);test_game.players[1-side].hand_count=1;test_game.players[1-side].hand[0]=four;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1-side);DuelCommand c={1-side,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.choice.kind&&p->hand_count==2&&p->grave_count==1&&!p->monsters[0].card);
        /* Proper ritual summons may be revived; a ritual sent directly from
           hand to the grave must still be rejected by Monster Reborn. */
        ritual_fixture(side,spell,monster);p=&test_game.players[side];p->grave_count=1;p->grave[0]=monster;p->grave_proper[0]=0;check(!duel_can_revive(&test_game,side,0));p->grave_proper[0]=1;check(duel_can_revive(&test_game,side,0));
        /* Native AI can finish all material/position decisions and activate
           a ritual previously set in its own back row. */
        ritual_fixture(side,spell,monster);p=&test_game.players[side];p->hand[p->hand_count++]=monster;p->backrow[0]=slot(spell,side,DUEL_FACEDOWN_DEFENSE);p->hand[0]=four;DuelCommand ai;check(duel_ai_decide(&test_game,side,&ai)&&ai.action==DUEL_ACTION_ACTIVATE_BACKROW&&duel_command(&test_game,&ai)==DUEL_OK);unsigned n=0;
        while(test_game.choice.kind&&n++<8){check(duel_ai_decide(&test_game,side,&ai)&&duel_command(&test_game,&ai)==DUEL_OK);}check(n<8&&!test_game.choice.kind&&p->monsters[0].card==monster);
    }
    native_ritual_test_result[0]=checks-first;native_ritual_test_result[1]=sizeof(original_ritual_cases)/sizeof(original_ritual_cases[0]);native_ritual_test_result[2]=valid;
}

