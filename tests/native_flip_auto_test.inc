#include "original_flip_auto_cases.h"
volatile uint32_t native_flip_auto_test_result[4];
static void flip_auto_pass(void){unsigned n=0;while(test_game.response_player<2&&n++<8){DuelCommand c={test_game.response_player,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK);}}
static void flip_auto_tests(void){unsigned first=checks,four=0,three=0;
    for(unsigned i=1;i<DUEL_CARD_COUNT;++i)if(duel_card_definitions[i].family==DUEL_MONSTER&&duel_card_definitions[i].subtype==0){if(duel_card_definitions[i].level==4)four=i;if(duel_card_definitions[i].level==3)three=i;}
    for(unsigned ci=0;ci<sizeof(original_flip_auto_cases)/sizeof(original_flip_auto_cases[0]);++ci){const unsigned *r=original_flip_auto_cases[ci];unsigned card=r[0],side=r[1],enemy=1-side,op=r[2],n=r[3],value=r[4];
        reset();test_game.turn=3;test_game.active=side;DuelPlayer *p=&test_game.players[side],*q=&test_game.players[enemy];p->hand_count=3;p->hand[0]=four;p->hand[1]=316;p->hand[2]=550;q->hand_count=1;q->hand[0]=316;
        p->monsters[0]=slot(card,side,DUEL_FACEDOWN_DEFENSE);p->monsters[0].serial=99;
        for(unsigned i=0;i<5;++i)q->monsters[i]=slot(i%3==1?three:four,enemy,i==4?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK);
        for(unsigned i=0;i<(n<5?n:5);++i){q->backrow[i]=slot(316,enemy,DUEL_FACEDOWN_DEFENSE);q->backrow[i].set_turn=3;}if(n==6)q->field=slot(duel_field_rules[0].card,enemy,DUEL_FACEUP_ATTACK);
        for(unsigned i=0;i<2;++i){DuelPlayer *a=&test_game.players[i];a->deck_count=7;for(unsigned j=0;j<7;++j)a->deck[j]=j%2?316:550;}
        unsigned owned0=owned_cards(&test_game,0),owned1=owned_cards(&test_game,1);check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);flip_auto_pass();check(!test_game.choice.kind&&!test_game.chain_count);
        if(op==1)check(p->hand_count==3+value&&p->deck_count==7-value&&p->hand[3]==550);
        if(op==2)check(q->hand_count==3&&q->deck_count==4&&q->grave_count==1&&q->grave[0]==316&&q->hand[0]==316&&q->hand[1]==550&&q->hand[2]==550);
        if(op==3)check(q->deck_count==7-value&&q->grave_count==value&&q->grave[0]==550&&q->grave[1]==316);
        if(op==4||op==7||op==8)check(q->life==8000-(int)value);
        if(op==5){for(unsigned i=0;i<5;++i)check((q->monsters[i].card==0)==((value&(1u<<(enemy*16+i)))!=0));check(q->grave_count==3&&q->monsters[4].position==DUEL_FACEDOWN_DEFENSE);}
        if(op==6)check(p->hand_count==5&&q->hand_count==5&&p->deck_count==2&&q->deck_count==2&&p->grave_count==3&&q->grave_count==1);
        if(op==7)check(p->deck_count==4&&p->banished_count==3&&p->banished[0]==550&&p->banished[1]==316&&p->hand_count==3);
        if(op==8)check(p->hand_count==1&&p->banished_count==2&&p->deck_count==7);
        check(owned_cards(&test_game,0)==owned0&&owned_cards(&test_game,1)==owned1);
    }
    for(unsigned side=0;side<2;++side){
        /* Mill fewer than five cards without a deck-out loss; draw from an
           empty deck loses. Reset-five checks both seats and resolution order. */
        reset();test_game.active=side;test_game.turn=3;test_game.players[side].monsters[0]=slot(simple_record(561),side,DUEL_FACEDOWN_DEFENSE);test_game.players[1-side].deck_count=2;check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.winner<0&&!test_game.players[1-side].deck_count&&test_game.players[1-side].grave_count==2);
        reset();test_game.active=side;test_game.turn=3;test_game.players[side].monsters[0]=slot(simple_record(539),side,DUEL_FACEDOWN_DEFENSE);test_game.players[side].deck_count=0;check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.winner==1-side);
        for(unsigned op=7;op<=8;++op){reset();test_game.active=side;test_game.turn=3;DuelPlayer *p=&test_game.players[side];p->monsters[0]=slot(simple_record(op==7?1435:1436),side,DUEL_FACEDOWN_DEFENSE);p->hand_count=op==8?1:3;p->deck_count=op==7?2:5;
            check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&!p->banished_count&&test_game.players[1-side].life==8000&&!test_game.chain_count);}
        /* A battle-destroyed flip source resolves before leaving the field. */
        reset();test_game.active=side;test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(550,side,DUEL_FACEUP_ATTACK);test_game.players[1-side].monsters[0]=slot(simple_record(561),1-side,DUEL_FACEDOWN_DEFENSE);unsigned deck=test_game.players[side].deck_count;
        check(command(DUEL_ACTION_ATTACK,0,1-side,0,0,0)==DUEL_OK&&test_game.players[side].deck_count==deck-5&&!test_game.players[1-side].monsters[0].card&&!test_game.battle_pending);
    }
    native_flip_auto_test_result[0]=checks-first;native_flip_auto_test_result[1]=sizeof(original_flip_auto_cases)/sizeof(original_flip_auto_cases[0]);
}
