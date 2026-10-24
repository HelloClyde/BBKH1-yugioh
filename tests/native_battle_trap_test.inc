#include "original_battle_trap_cases.h"
volatile uint32_t native_battle_trap_test_result[4];
static void battle_trap_tests(void){unsigned first=checks;
    for(unsigned ci=0;ci<sizeof(original_battle_trap_cases)/sizeof(original_battle_trap_cases[0]);++ci){const unsigned *v=original_battle_trap_cases[ci];unsigned card=v[0],side=v[1],enemy=1-side,name=duel_card_definitions[card].name_group;reset();test_game.turn=3;test_game.active=enemy;test_game.phase=DUEL_BATTLE;test_game.random_state=98;test_game.players[0].hand_count=test_game.players[1].hand_count=0;
        test_game.players[side].backrow[0]=slot(card,side,DUEL_FACEDOWN_DEFENSE);for(unsigned i=0;i<5;++i)test_game.players[enemy].monsters[i]=slot(550,enemy,v[i+2]==1?DUEL_FACEDOWN_DEFENSE:v[i+2]==2?DUEL_FACEUP_ATTACK:DUEL_FACEUP_DEFENSE);
        check(command(DUEL_ACTION_ATTACK,0,side,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.response_player==side);DuelCommand c={side,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.chain_count&&test_game.response_player==DUEL_NO_SLOT);
        for(unsigned i=0;i<5;++i){DuelSlot *s=&test_game.players[enemy].monsters[i];unsigned raw=v[i+2];
            if(name==1056)check((s->card==0)==(raw==2));
            else if(name==1177)check(s->position==(raw==1?DUEL_FACEDOWN_DEFENSE:raw==2?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK));
            else {check(duel_attack_value(&test_game,enemy,i)==1900-(raw==1?0:300));check(duel_defense_value(&test_game,enemy,i)==900-(raw==1?0:300));}}
        check(test_game.players[side].life==(name==1204?6400:8000));check(test_game.players[side].grave_count==1&&test_game.players[side].grave[0]==card);
    }
    for(unsigned side=0;side<2;++side){unsigned enemy=1-side;
        for(unsigned own=0;own<=5;++own)for(unsigned other=0;other<=5;++other){
            unsigned dessert=simple_record(1032),gift=simple_record(1092);simple_fixture(other?dessert:gift,side);for(unsigned i=0;i<own;++i)test_game.players[side].monsters[i]=slot(550,side,i%3);for(unsigned i=0;i<other;++i)test_game.players[enemy].monsters[i]=slot(550,enemy,i%3);
            check(simple_activate(other?dessert:gift,0,0)==DUEL_OK);check(test_game.players[enemy].life==8000-(int)(other*500));if(!other)check(test_game.players[side].life==8000+(int)(own*300));}
        for(unsigned kind=0;kind<3;++kind){unsigned card=simple_record(kind==0?960:kind==1?1055:1098);reset();test_game.turn=3;test_game.active=enemy;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[enemy].monsters[0]=slot(550,enemy,DUEL_FACEUP_ATTACK);test_game.players[side].backrow[0]=slot(card,side,DUEL_FACEDOWN_DEFENSE);
            check(command(DUEL_ACTION_ATTACK,0,side,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.response_player==side);DuelCommand c={side,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK);
            check(test_game.players[side].life==8000);if(kind==0){check(test_game.phase==DUEL_MAIN2);uint32_t before=duel_state_hash(&test_game);check(command(DUEL_ACTION_PHASE,0,0,DUEL_BATTLE,0,0)==DUEL_WRONG_PHASE&&before==duel_state_hash(&test_game));}else if(kind==1)check(test_game.battle_protect_turn[side]==3);
            /* The same response trap set this turn is unavailable, including
               spell-speed-three Negate Attack. */
            reset();test_game.turn=3;test_game.active=enemy;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;test_game.players[enemy].monsters[0]=slot(550,enemy,DUEL_FACEUP_ATTACK);test_game.players[side].backrow[0]=slot(card,side,DUEL_FACEDOWN_DEFENSE);test_game.players[side].backrow[0].set_turn=3;
            check(command(DUEL_ACTION_ATTACK,0,side,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[side].life==6100&&!test_game.chain_count);
        }
        /* Waboku stops battle destruction in both roles, including ties,
           but effect destruction and damage remain effective. */
        for(unsigned attacking=0;attacking<2;++attacking)for(unsigned position=0;position<3;++position){unsigned waboku=simple_record(1055);simple_fixture(waboku,side);check(simple_activate(waboku,0,0)==DUEL_OK);test_game.phase=DUEL_BATTLE;test_game.active=attacking?side:enemy;test_game.players[side].monsters[0]=slot(24,side,position);test_game.players[enemy].monsters[0]=slot(550,enemy,DUEL_FACEUP_ATTACK);
            if(attacking)test_game.players[side].monsters[0].position=DUEL_FACEUP_ATTACK;check(command(DUEL_ACTION_ATTACK,0,1-test_game.active,0,0,0)==DUEL_OK&&test_game.players[side].monsters[0].card==24&&test_game.players[side].life==8000);
            test_game.active=enemy;test_game.phase=DUEL_MAIN1;test_game.players[enemy].hand_count=1;test_game.players[enemy].hand[0]=simple_record(346);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[side].life==7000);
            test_game.players[enemy].hand_count=1;test_game.players[enemy].hand[0]=802;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&!test_game.players[side].monsters[0].card);}
        /* Waboku is temporary; the next turn no longer uses the flag. */
        simple_fixture(simple_record(1055),side);check(simple_activate(simple_record(1055),0,0)==DUEL_OK);test_game.turn=4;test_game.active=enemy;test_game.phase=DUEL_BATTLE;test_game.players[enemy].monsters[0]=slot(550,enemy,DUEL_FACEUP_ATTACK);check(command(DUEL_ACTION_ATTACK,0,side,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.players[side].life==6100);
        unsigned zombyra=0;for(unsigned i=1;i<DUEL_CARD_COUNT;++i)if(duel_card_definitions[i].effect==DUEL_E_ZOMBYRA)zombyra=i;
        simple_fixture(simple_record(1055),side);check(simple_activate(simple_record(1055),0,0)==DUEL_OK);test_game.active=enemy;test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(24,side,DUEL_FACEUP_ATTACK);test_game.players[enemy].monsters[0]=slot(zombyra,enemy,DUEL_FACEUP_ATTACK);
        check(command(DUEL_ACTION_ATTACK,0,side,0,0,0)==DUEL_OK&&test_game.players[side].monsters[0].card==24&&!test_game.players[enemy].monsters[0].attack_bonus);test_game.turn=4;test_game.players[enemy].monsters[0].attacked=0;
        check(command(DUEL_ACTION_ATTACK,0,side,0,0,0)==DUEL_OK&&!test_game.players[side].monsters[0].card&&test_game.players[enemy].monsters[0].attack_bonus==-200);
        /* Eternal Rest destroys every equipped monster simultaneously and
           moves its attachment; an unconnected equip is not a monster. */
        unsigned eternal=simple_record(655);simple_fixture(eternal,side);for(unsigned owner=0;owner<2;++owner){test_game.players[owner].monsters[0]=slot(550,owner,DUEL_FACEUP_ATTACK);test_game.players[owner].monsters[0].serial=30+owner;test_game.players[owner].backrow[1]=slot(duel_equip_rules[0].card,owner,DUEL_FACEUP_ATTACK);test_game.players[owner].backrow[1].effect_active=1;test_game.players[owner].backrow[1].link_serial=30+owner;test_game.players[owner].monsters[1]=slot(550,owner,DUEL_FACEUP_ATTACK);}
        check(simple_activate(eternal,0,0)==DUEL_OK);for(unsigned owner=0;owner<2;++owner)check(!test_game.players[owner].monsters[0].card&&!test_game.players[owner].backrow[1].card&&test_game.players[owner].monsters[1].card==550);
    }
    native_battle_trap_test_result[0]=checks-first;native_battle_trap_test_result[1]=sizeof(original_battle_trap_cases)/sizeof(original_battle_trap_cases[0]);
}
