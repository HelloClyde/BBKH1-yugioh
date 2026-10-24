#include "original_victory_priority_cases.h"
volatile uint32_t native_victory_priority_test_result[6];
static void victory_test_exodia(unsigned side,unsigned count){test_game.players[side].hand_count=0;for(unsigned i=0;i<count;++i)contact_test_hand(side,i,16+i,500+side*20+i);}
static unsigned victory_test_draw_name(void){for(unsigned i=1;i<DUEL_CARD_COUNT;++i)if(duel_card_definitions[i].effect==DUEL_E_DRAW2)return duel_card_definitions[i].name_group;return 0;}
static void victory_priority_tests(void){unsigned initial=checks,matrix=0,flows=0;for(unsigned n=0;n<sizeof(original_victory_priority_cases)/sizeof(original_victory_priority_cases[0]);++n){const int *v=original_victory_priority_cases[n];pair_fixture(v[0]);test_game.deck_failure_mask=v[1];for(unsigned side=0;side<2;++side){test_game.players[side].life=(v[4]>>side&1)?0:8000;if(v[2]>>side&1)victory_test_exodia(side,5);if(v[3]>>side&1)for(unsigned i=0;i<5;++i){expansion_place(side,i,i?1540+i:1528);test_game.players[side].backrow[i].serial=700+side*10+i;}}check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK);check(test_game.winner==v[5]);++matrix;}
 for(unsigned actor=0;actor<2;++actor){unsigned enemy=1-actor,draw=victory_test_draw_name();check(draw!=0);
 /* A completed top link wins before the lower Dark Hole can resolve. */
 pair_fixture(actor);victory_test_exodia(actor,4);redirect_place(enemy,0,77,2,100);reactive_test_deck(actor,77,2);test_game.players[actor].deck[1]=simple_record(20);test_game.players[actor].deck_state[1].card=simple_record(20);spirit_fake_spell(335,actor);DuelChainLink lower=test_game.chain[0];test_game.players[actor].backrow[3]=test_game.players[actor].backrow[4];test_game.players[actor].backrow[3].serial=lower.source_serial=305;lower.source_slot=3;spirit_fake_spell(draw,actor);test_game.chain[1]=test_game.chain[0];test_game.chain[0]=lower;test_game.chain_count=2;pass_chain();
check(test_game.winner==(int)actor&&test_game.players[enemy].monsters[0].card==simple_record(77));++flows;
 /* Failure on the second required draw beats the newly completed Exodia. */
 pair_fixture(actor);victory_test_exodia(actor,4);reactive_test_deck(actor,20,1);spirit_fake_spell(draw,actor);pass_chain();
check(test_game.winner==(int)enemy&&duel_exodia_complete(&test_game,actor));++flows;
 /* Within one callback the head arrives, then Crush sends a large drawn
  * monster to the GY and Skull Invitation reduces LP to zero: Exodia wins. */
 pair_fixture(actor);victory_test_exodia(actor,4);reactive_test_deck(actor,77,2);test_game.players[actor].deck[1]=simple_record(20);test_game.players[actor].deck_state[1].card=simple_record(20);test_game.players[actor].life=300;test_game.crush_until_end[actor]=test_game.turn+1;passive_test_source(enemy,1306,2,0,0);spirit_fake_spell(draw,actor);pass_chain();
check(test_game.players[actor].life==0&&duel_exodia_complete(&test_game,actor)&&test_game.winner==(int)actor);++flows;
 /* One Skull Invitation callback applies its two byte-counter channels
  * before victory is checked, so bilateral lethal damage is a draw. */
 pair_fixture(actor);test_game.players[0].life=test_game.players[1].life=300;redirect_place(0,0,77,2,100);redirect_place(1,0,57,2,101);passive_test_source(enemy,1306,2,0,0);spirit_fake_spell(335,actor);pass_chain();
check(!test_game.players[0].life&&!test_game.players[1].life&&test_game.winner==2);++flows;
 }
 for(unsigned actor=0;actor<2;++actor){unsigned enemy=1-actor,draw=victory_test_draw_name();
 /* Ring damage and its destruction observer finish before the LP boundary. */
 pair_fixture(actor);test_game.players[actor].life=300;test_game.players[enemy].life=600;redirect_place(enemy,0,57,2,100);passive_test_source(enemy,1306,2,0,0);spirit_fake_spell(939,actor);test_game.chain[0].command.target_player=enemy;test_game.chain[0].command.target=0;test_game.chain[0].target_serial=100;pass_chain();check(test_game.players[0].life==0&&test_game.players[1].life==0&&test_game.winner==2);++flows;
 /* A resolved Spell itself enters the GY before checking and moving to the
  * lower link; Skull's resulting fatal damage cannot wait for that link. */
 pair_fixture(actor);test_game.players[actor].life=300;redirect_place(enemy,0,77,2,100);reactive_test_deck(actor,57,2);passive_test_source(enemy,1306,2,0,0);spirit_fake_spell(335,actor);DuelChainLink lower=test_game.chain[0];test_game.players[actor].backrow[3]=test_game.players[actor].backrow[4];test_game.players[actor].backrow[3].serial=lower.source_serial=305;lower.source_slot=3;spirit_fake_spell(draw,actor);test_game.chain[1]=test_game.chain[0];test_game.chain[0]=lower;test_game.chain_count=2;pass_chain();check(test_game.players[actor].life==0&&test_game.winner==(int)enemy&&test_game.players[enemy].monsters[0].card==simple_record(77));++flows;
 }
 native_victory_priority_test_result[0]=checks-initial;native_victory_priority_test_result[1]=matrix;native_victory_priority_test_result[2]=flows;}
