/* Native LP fixtures: original reference covers only queued parameters;
 * arithmetic, payment, chain and AI behavior are separately checked here. */
#include "original_life_cases.h"
volatile uint32_t native_life_test_result[4];
static void life_fixture(unsigned card,unsigned caster){
    reset();test_game.turn=3;test_game.active=caster;test_game.players[caster].hand_count=1;test_game.players[caster].hand[0]=card;
}
static void life_tests(void){
    unsigned first=checks;DuelCommand c;uint32_t before;
    check(sizeof(DuelLifeRule)==6&&DUEL_LIFE_RULE_COUNT>=12);
    for(unsigned i=0;i<sizeof(original_life_cases)/sizeof(original_life_cases[0]);++i){
        unsigned caster=original_life_cases[i].caster,other=1-caster,card=original_life_cases[i].card;
        life_fixture(card,caster);
        if(original_life_cases[i].negated){test_game.players[other].backrow[0]=slot(893,other,DUEL_FACEDOWN_DEFENSE);test_game.players[other].hand_count=1;test_game.players[other].hand[0]=24;}
        unsigned owned0=owned_cards(&test_game,0),owned1=owned_cards(&test_game,1);
        c=(DuelCommand){caster,DUEL_ACTION_ACTIVATE_HAND,0,original_life_cases[i].target};check(duel_command(&test_game,&c)==DUEL_OK);
        if(original_life_cases[i].negated){check(test_game.response_player==other);c=(DuelCommand){other,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK);}
        int expected[2]={8000,8000};
        if(original_life_cases[i].count){expected[original_life_cases[i].side1]+=original_life_cases[i].op1?-(int)original_life_cases[i].amount1:(int)original_life_cases[i].amount1;
            if(original_life_cases[i].count==2)expected[original_life_cases[i].side2]+=original_life_cases[i].op2?-(int)original_life_cases[i].amount2:(int)original_life_cases[i].amount2;}
        check(test_game.players[0].life==expected[0]&&test_game.players[1].life==expected[1]);
        check(!test_game.players[caster].hand_count&&!test_game.players[caster].backrow[0].card&&test_game.players[caster].grave_count==1&&test_game.players[caster].grave[0]==card);
        check(owned_cards(&test_game,0)==owned0&&owned_cards(&test_game,1)==owned1);
    }
    /* No cost on rejected targets, phases, unavailable rules or full zones. */
    life_fixture(986,0);before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,2,0,0,0)==DUEL_BAD_TARGET&&before==duel_state_hash(&test_game));
    life_fixture(559,0);test_game.phase=DUEL_BATTLE;before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_WRONG_PHASE&&before==duel_state_hash(&test_game));
    life_fixture(559,0);for(unsigned i=0;i<5;++i){test_game.players[0].backrow[i]=slot(316,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].backrow[i].set_turn=3;}
    before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_ZONE_FULL&&before==duel_state_hash(&test_game));
    /* A set normal spell works on that turn even with all zones occupied. */
    test_game.players[0].backrow[0]=slot(559,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].backrow[0].set_turn=3;
    check(command(DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0)==DUEL_OK&&test_game.players[0].life==9000&&!test_game.players[0].backrow[0].card);
    /* MST destroys the source but does not negate the normal LP spell. */
    for(unsigned burn=0;burn<2;++burn){
        life_fixture(burn?557:559,0);test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1);
        c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK);
        check(test_game.players[0].life==(burn?8000:9000)&&test_game.players[1].life==(burn?7200:8000)&&test_game.players[0].grave_count==1);
    }
    /* Either side can be deliberately healed; no implicit target substitution. */
    life_fixture(986,0);check(command(DUEL_ACTION_ACTIVATE_HAND,0,1,0,0,0)==DUEL_OK&&test_game.players[0].life==8000&&test_game.players[1].life==8200);
    life_fixture(986,1);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[0].life==8200&&test_game.players[1].life==8000);
    /* Original573fb0 clamps recovery at the16-bit LP ceiling. */
    life_fixture(559,0);test_game.players[0].life=65535-500;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[0].life==65535);
    life_fixture(969,1);test_game.players[0].life=65535-10;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[0].life==65535&&test_game.players[1].life==9000);
    /* Damage clamps at zero and ends the duel; the spell still leaves once. */
    for(unsigned caster=0;caster<2;++caster){life_fixture(755,caster);test_game.players[1-caster].life=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.winner==caster&&test_game.players[1-caster].life==0&&test_game.players[caster].grave[0]==755);
        before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_FINISHED&&before==duel_state_hash(&test_game));}
    /* Metadata setup is atomic and unavailable after play begins. */
    reset();DuelLifeRule bad=duel_life_rules[0];bad.amount=0;
    check(duel_life_setup(&test_game,&bad,1)==DUEL_BAD_ARGUMENT&&test_game.life_rule_count==DUEL_LIFE_RULE_COUNT);
    bad=duel_life_rules[0];bad.operation=DUEL_LIFE_DAMAGE;bad.target=DUEL_LIFE_BOTH;
    check(duel_life_setup(&test_game,&bad,1)==DUEL_BAD_ARGUMENT&&test_game.life_rule_count==DUEL_LIFE_RULE_COUNT);
    DuelLifeRule duplicate[2]={duel_life_rules[0],duel_life_rules[0]};check(duel_life_setup(&test_game,duplicate,2)==DUEL_BAD_ARGUMENT);
    check(duel_life_setup(&test_game,0,0)==DUEL_OK);test_game.players[0].hand[0]=559;before=duel_state_hash(&test_game);
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_UNSUPPORTED&&before==duel_state_hash(&test_game));
    life_fixture(559,0);check(duel_life_setup(&test_game,duel_life_rules,DUEL_LIFE_RULE_COUNT)==DUEL_BAD_ARGUMENT);
    /* Repeated common effects play complete games, not just isolated calls. */
    uint16_t deck[40];for(unsigned i=0;i<40;++i)deck[i]=i<16?24:i<36?duel_life_rules[(i-16)%DUEL_LIFE_RULE_COUNT].card:316;
    unsigned games=0,activations=0,lethal=0;
    for(unsigned seed=1;seed<=30;++seed){
        check(duel_start(&test_game,duel_card_definitions,deck,40,deck,40,seed,seed&1)==DUEL_OK);
        check(duel_life_setup(&test_game,duel_life_rules,DUEL_LIFE_RULE_COUNT)==DUEL_OK);
        for(unsigned n=0;test_game.winner<0&&n<3000;++n){
            check(duel_ai_decide(&test_game,decision_player(),&c));unsigned life=c.action==DUEL_ACTION_ACTIVATE_HAND&&test_game.cards[test_game.players[c.player].hand[c.source]].effect==DUEL_E_LIFE;
            activations+=life;if(life&&duel_life_rule(&test_game,test_game.players[c.player].hand[c.source])->target==DUEL_LIFE_EITHER)check(c.target_player==c.player);
            int result=duel_command(&test_game,&c);check(result==DUEL_OK);if(result!=DUEL_OK)break;
            lethal+=life&&test_game.winner>=0;
            check(owned_cards(&test_game,0)==40&&owned_cards(&test_game,1)==40);
        }check(test_game.winner>=0);games+=test_game.winner>=0;
    }check(activations>30);native_life_test_result[0]=checks-first;native_life_test_result[1]=games;native_life_test_result[2]=activations;native_life_test_result[3]=lethal;
}
