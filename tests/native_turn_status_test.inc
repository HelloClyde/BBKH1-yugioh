#include "original_turn_status_cases.h"
volatile uint32_t native_turn_status_test_result[6];
static unsigned turn_status_grave_has(unsigned side,unsigned serial){const DuelPlayer *p=&test_game.players[side];for(unsigned i=0;i<p->grave_count;++i)if(p->grave_serial[i]==serial)return 1;return 0;}
static void turn_status_tests(void){unsigned initial=checks,matrix=0,reference=0;DuelCommand c;
    for(unsigned n=0;n<sizeof(original_gamble_gate_cases)/sizeof(original_gamble_gate_cases[0]);++n){
        const int *v=original_gamble_gate_cases[n];unsigned actor=v[0];pair_fixture(actor);
        for(unsigned i=0;i<(unsigned)v[1];++i)contact_test_hand(1-actor,i,77,200+i);
        for(unsigned i=0;i<(unsigned)v[2];++i)contact_test_hand(actor,i,57,100+i);
        test_game.players[actor].deck_count=v[3];test_game.players[actor].backrow[0]=slot(simple_record(1303),actor,DUEL_FACEDOWN_DEFENSE);test_game.players[actor].backrow[0].serial=300;test_game.players[actor].backrow[0].set_turn=1;
        if(v[4])contact_test_hand(actor,0,1303,300);
        check((command(v[4]?DUEL_ACTION_ACTIVATE_HAND:DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0)==DUEL_OK)==v[5]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_gamble_coin_cases)/sizeof(original_gamble_coin_cases[0]);++n){
        const int *v=original_gamble_coin_cases[n];unsigned actor=v[0];pair_fixture(actor);
        for(unsigned i=0;i<(unsigned)v[1];++i)contact_test_hand(actor,i,57,100+i);
        spirit_fake_spell(1303,actor);test_game.chain[0].effect_negated=v[4]!=0;test_game.random_state=v[3]?1:2;pass_chain();
        check((test_game.choice.kind==DUEL_CHOICE_GAMBLE_COIN)==!v[4]);
        if(!v[4])check(choose(actor,v[2])==DUEL_OK);
        check(test_game.players[actor].hand_count==(unsigned)(v[1]+v[5])&&test_game.skip_next_turn[actor]==v[6]);++matrix;
    }
    reference+=sizeof(original_gamble_skip_start_cases)/sizeof(original_gamble_skip_start_cases[0]);reference+=sizeof(original_gamble_skip_draw_cases)/sizeof(original_gamble_skip_draw_cases[0]);
    for(unsigned n=0;n<sizeof(original_weather_body_cases)/sizeof(original_weather_body_cases[0]);++n){
        const int *v=original_weather_body_cases[n];unsigned actor=v[0],enemy=1-actor;pair_fixture(v[1]);test_game.phase=v[2]/2;
        expansion_place(enemy,0,347);test_game.players[enemy].backrow[0].position=v[3]<2?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK;
        pair_chain(582,actor);test_game.chain[0].effect_negated=v[4]!=0;pass_chain();
        check((!test_game.players[enemy].backrow[0].card)==(v[6]!=0));
        check(test_game.weather_until_end[actor]==(v[6]?3+(v[7]>>4):0));++matrix;
    }
    for(unsigned n=0;n<sizeof(original_weather_second_battle_cases)/sizeof(original_weather_second_battle_cases[0]);++n){
        const int *v=original_weather_second_battle_cases[n];pair_fixture(v[0]);test_game.phase=DUEL_BATTLE;test_game.weather_until_end[v[0]]=v[1]?test_game.turn:0;
        redirect_place(v[0],0,77,2,200);test_game.players[v[0]].monsters[0].attacked=1;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK);
        check((test_game.phase==DUEL_MAIN2)==v[2]&&test_game.players[v[0]].monsters[0].attacked==(unsigned)v[2]&&!test_game.weather_until_end[v[0]]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_turn_counter_cases)/sizeof(original_turn_counter_cases[0]);++n){
        const int *v=original_turn_counter_cases[n];if(v[1]==1303){++reference;continue;}pair_fixture(v[0]);
        if(v[1]==660)test_game.crush_until_end[v[0]]=test_game.turn+v[2];else test_game.weather_until_end[v[0]]=test_game.turn+v[2];
        for(unsigned i=0;i<(unsigned)v[3];++i){check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK||test_game.phase==DUEL_MAIN1);check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);pass_chain();}
        unsigned until=v[1]==660?test_game.crush_until_end[v[0]]:test_game.weather_until_end[v[0]];
        check((until!=0)==(v[4]>=0));if(until)check((int)until-test_game.turn==v[4]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_crush_cost_cases)/sizeof(original_crush_cost_cases[0]);++n){
        const int *v=original_crush_cost_cases[n];unsigned actor=v[0];pair_fixture(actor);redirect_place(v[1],0,v[2],v[3],200);
        if(v[4]){expansion_place(actor,3,1418);test_game.players[actor].backrow[3].serial=303;}
        test_game.players[actor].backrow[0]=slot(simple_record(660),actor,DUEL_FACEDOWN_DEFENSE);test_game.players[actor].backrow[0].serial=300;test_game.players[actor].backrow[0].set_turn=1;
        c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_BACKROW,0};c.cost=1;c.tributes=v[1]*5+1;
        check((duel_command(&test_game,&c)==DUEL_OK)==v[5]);if(v[5])check(!test_game.players[v[1]].monsters[0].card&&turn_status_grave_has(v[1],200));++matrix;
    }
    for(unsigned n=0;n<sizeof(original_crush_field_cases)/sizeof(original_crush_field_cases[0]);++n){
        const int *v=original_crush_field_cases[n];unsigned actor=v[0],enemy=1-actor;pair_fixture(actor);redirect_place(enemy,0,v[1],v[2],200);
        test_game.players[enemy].monsters[0].original_attack_active=1;test_game.players[enemy].monsters[0].original_attack=v[3];
        spirit_fake_spell(660,actor);test_game.chain[0].effect_negated=v[4]!=0;pass_chain();
        check((!test_game.players[enemy].monsters[0].card)==v[5]);
        if(!v[5])check(test_game.players[enemy].monsters[0].position==(v[2]<2?DUEL_FACEDOWN_DEFENSE:v[2]==3?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK)&&!test_game.choice.kind);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_crush_timer_cases)/sizeof(original_crush_timer_cases[0]);++n){
        const int *v=original_crush_timer_cases[n];pair_fixture(v[1]);spirit_fake_spell(660,v[0]);test_game.chain[0].effect_negated=v[2]!=0;pass_chain();
        check(test_game.crush_until_end[1-v[0]]==(v[3]>=0?3+v[3]:0));++matrix;
    }
    for(unsigned n=0;n<sizeof(original_crush_hand_cases)/sizeof(original_crush_hand_cases[0]);++n){
        const int *v=original_crush_hand_cases[n];unsigned actor=v[0],enemy=1-actor;pair_fixture(actor);contact_test_hand(enemy,0,v[1],200);test_game.players[enemy].hand_state[0].owner=v[2];
        spirit_fake_spell(660,actor);test_game.chain[0].effect_negated=v[3]!=0;pass_chain();
        check(test_game.players[enemy].hand_count==1u-v[4]&&turn_status_grave_has(v[2],200)==(unsigned)v[4]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_crush_draw_cases)/sizeof(original_crush_draw_cases[0]);++n){
        const int *v=original_crush_draw_cases[n];unsigned actor=v[0];pair_fixture(actor);test_game.players[actor].deck_count=1;test_game.players[actor].deck[0]=simple_record(v[1]);test_game.players[actor].deck_state[0]=(DuelCardState){700,simple_record(v[1]),0,0,v[3]};test_game.crush_until_end[actor]=v[2]?100:0;
        spirit_fake_spell(1065,actor);pass_chain();
        check(test_game.players[actor].hand_count==1u-v[4]&&turn_status_grave_has(v[3],700)==(unsigned)v[4]);++matrix;
    }
    for(unsigned actor=0;actor<2;++actor){unsigned enemy=1-actor;
        /* The cost is paid before Seven Tools negates the Trap activation. */
        pair_fixture(actor);redirect_place(actor,0,57,1,200);contact_test_hand(enemy,0,77,201);
        test_game.players[actor].backrow[0]=slot(simple_record(660),actor,DUEL_FACEDOWN_DEFENSE);test_game.players[actor].backrow[0].serial=300;test_game.players[actor].backrow[0].set_turn=1;
        test_game.players[enemy].backrow[0]=slot(simple_record(1030),enemy,DUEL_FACEDOWN_DEFENSE);test_game.players[enemy].backrow[0].serial=301;test_game.players[enemy].backrow[0].set_turn=1;
        check(command(DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_CRUSH_TRIBUTE);
        check(choose(actor,actor*5)==DUEL_OK&&!test_game.players[actor].monsters[0].card&&test_game.chain_count==1);
        c=(DuelCommand){enemy,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK);pass_chain();
        check(turn_status_grave_has(actor,200)&&test_game.players[enemy].hand_count==1&&!test_game.crush_until_end[enemy]&&identity_unique(&test_game));
        /* Weather's second Battle restores attack opportunities, then ends
         * normally. It does not create a second Main Phase 1. */
        pair_fixture(actor);redirect_place(actor,0,77,2,200);expansion_place(enemy,0,347);pair_chain(582,actor);pass_chain();
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_BATTLE,0,0)==DUEL_OK);check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_OK);pass_chain();
        check(test_game.players[enemy].life==6300&&test_game.players[actor].monsters[0].attacked==1);
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK&&test_game.phase==DUEL_BATTLE&&!test_game.players[actor].monsters[0].attacked);
        check(command(DUEL_ACTION_ATTACK,0,enemy,DUEL_NO_SLOT,0,0)==DUEL_OK);pass_chain();check(test_game.players[enemy].life==4600);
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK&&test_game.phase==DUEL_MAIN2);
        /* Gamble loses during the other player's turn. Its next turn has
         * no draw, Standby or action window, but End effects still execute. */
        pair_fixture(enemy);test_game.skip_next_turn[actor]=1;unsigned before=test_game.players[actor].deck_count;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);pass_chain();
        check(test_game.active==enemy&&test_game.turn==5&&!test_game.skip_next_turn[actor]&&test_game.players[actor].deck_count==before&&test_game.phase==DUEL_DRAW);
        /* A real Flip producer only queues Weather with a visible opposing
         * Swords, and destruction has no live-source dependency afterwards. */
        pair_fixture(actor);redirect_place(actor,0,582,1,200);expansion_place(enemy,0,347);
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);pass_chain();
        check(!test_game.players[enemy].backrow[0].card&&test_game.weather_until_end[actor]==test_game.turn);
    }
    uint16_t deck[48];const unsigned extra[]={660,582,1303,660,582,1303,57,347};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<8;++i)deck[40+i]=simple_record(extra[i]);unsigned completed=0;
    for(unsigned seed=0;seed<10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,48,deck,48,17900+seed,seed&1)==DUEL_OK);check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));int r=duel_command(&test_game,&c);check(r==DUEL_OK);if(r!=DUEL_OK)break;check(owned_cards(&test_game,0)==48&&owned_cards(&test_game,1)==48);check(identity_unique(&test_game));}check(test_game.winner>=0);completed+=test_game.winner>=0;}check(completed==10);native_turn_status_test_result[2]=completed;
    native_turn_status_test_result[0]=checks-initial;native_turn_status_test_result[1]=matrix;native_turn_status_test_result[4]=reference;
}
