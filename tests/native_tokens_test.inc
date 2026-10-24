#include "original_tokens_cases.h"
volatile uint32_t native_tokens_test_result[6];
static unsigned token_count(unsigned side,unsigned name){unsigned n=0;for(unsigned i=0;i<5;++i){const DuelSlot *s=&test_game.players[side].monsters[i];n+=s->card&&duel_card_definitions[s->card].name_group==name;}return n;}
static void tokens_fixture(unsigned side,unsigned space){expansion_fixture(side);for(unsigned i=0;i<5-space;++i){test_game.players[side].monsters[i]=slot(simple_record(77),side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[i].serial=100+i;}}
static void tokens_tests(void){unsigned initial=checks,matrix=0,reference=0;
    for(unsigned n=0;n<sizeof(original_multiply_cost_cases)/sizeof(original_multiply_cost_cases[0]);++n){const int *v=original_multiply_cost_cases[n];expansion_fixture(v[0]);test_game.players[v[1]].monsters[0]=slot(v[2]==1901?1105:simple_record(v[2]),v[1],v[3]==1?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK);test_game.players[v[1]].monsters[0].serial=100;if(v[4]){test_game.tribute_grants[0]=(DuelTributeGrant){100,test_game.turn,v[0],3};test_game.tribute_grant_count=1;}if(v[5])expansion_place(v[0],0,1418);test_game.players[v[0]].hand[0]=simple_record(1232);test_game.players[v[0]].hand_count=1;uint32_t before=duel_state_hash(&test_game);int r=command(DUEL_ACTION_ACTIVATE_HAND,0,v[1],0,0,0);
        check((r==DUEL_OK)==v[6]);if(v[6]){pass_chain();
        check(token_count(v[0],1921)==5&&test_game.players[v[1]].monsters[0].card!=(v[2]==1901?1105:simple_record(v[2])));for(unsigned i=0;i<5;++i)check(test_game.players[v[0]].monsters[i].position==DUEL_FACEUP_DEFENSE);}else check(duel_state_hash(&test_game)==before);++matrix;}
    for(unsigned n=0;n<sizeof(original_scapegoat_gate_cases)/sizeof(original_scapegoat_gate_cases[0]);++n){const int *v=original_scapegoat_gate_cases[n];tokens_fixture(v[0],v[1]);test_game.summon_activity[v[0]]=v[2];test_game.summon_ban_turn[v[0]]=test_game.turn;test_game.summon_ban_mask[v[0]]=2*v[3];test_game.players[v[0]].hand[0]=simple_record(1245);test_game.players[v[0]].hand_count=1;
        check((command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK)==v[4]);if(v[4]){pass_chain();
        check(token_count(v[0],1922)==4&&test_game.summon_ban_mask[v[0]]==3);}++matrix;}
    for(unsigned n=0;n<sizeof(original_scapegoat_callback_cases)/sizeof(original_scapegoat_callback_cases[0]);++n){const int *v=original_scapegoat_callback_cases[n];if(v[5]!=128||v[1]<4||v[2]||v[3]){++reference;continue;}tokens_fixture(v[0],v[1]);if(v[4])expansion_place(1-v[0],4,1154);test_game.players[v[0]].hand[0]=simple_record(1245);test_game.players[v[0]].hand_count=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);pass_chain();
        check(token_count(v[0],1922)==(v[4]?0:4)&&test_game.summon_ban_turn[v[0]]==test_game.turn&&test_game.summon_ban_mask[v[0]]==3);++matrix;}
    for(unsigned n=0;n<sizeof(original_jam_callback_cases)/sizeof(original_jam_callback_cases[0]);++n){const int *v=original_jam_callback_cases[n];tokens_fixture(v[0],v[4]);expansion_place(v[1],0,1426);DuelSlot *s=&test_game.players[v[1]].backrow[0];s->position=v[2]==1?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK;if(v[3]==2)s->effect_active=0;if(v[3]==32){test_game.type_suppression_turn=test_game.turn;test_game.type_suppression_mask=2;}test_game.summon_ban_turn[v[0]]=test_game.turn;test_game.summon_ban_mask[v[0]]=2*v[5];test_game.phase=DUEL_DRAW;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_STANDBY,0,0)==DUEL_OK);pass_chain();
        check(token_count(v[0],1923)==v[6]);for(unsigned i=0;i<5;++i)if(duel_is_token(&test_game,test_game.players[v[0]].monsters[i].card))check(test_game.players[v[0]].monsters[i].position==DUEL_FACEUP_ATTACK);++matrix;}
    for(unsigned n=0;n<sizeof(original_token_placement_cases)/sizeof(original_token_placement_cases[0]);++n){const int *v=original_token_placement_cases[n];expansion_fixture(v[0]);for(unsigned i=1;i<=5-v[1];++i)test_game.players[v[0]].monsters[i]=slot(simple_record(77),v[0],DUEL_FACEUP_ATTACK);test_game.players[v[0]].monsters[0]=slot(simple_record(57),v[0],DUEL_FACEUP_ATTACK);test_game.players[v[0]].hand[0]=simple_record(1232);test_game.players[v[0]].hand_count=1;unsigned owned=owned_cards(&test_game,v[0]);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,v[0],0,0,0)==DUEL_OK);pass_chain();
        check(token_count(v[0],1921)==v[1]&&owned_cards(&test_game,v[0])==owned);for(unsigned i=0;i<5;++i)if(duel_is_token(&test_game,test_game.players[v[0]].monsters[i].card))check(test_game.players[v[0]].monsters[i].position==DUEL_FACEUP_DEFENSE);++matrix;}
    for(unsigned side=0;side<2;++side){DuelCommand c;
        /* A Normal Set consumes the normal allowance, while Scapegoat's
         * original any-summon marker remains clear for that face-down Set. */
        expansion_fixture(side);test_game.players[side].hand[0]=simple_record(77);test_game.players[side].hand[1]=simple_record(1245);test_game.players[side].hand_count=2;
        check(command(DUEL_ACTION_SET_MONSTER,0,0,0,0,0)==DUEL_OK&&!test_game.summon_activity[side]&&test_game.players[side].normal_summon_used);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);pass_chain();
        check(token_count(side,1922)==4);
        expansion_fixture(side);test_game.players[side].hand[0]=simple_record(77);test_game.players[side].hand[1]=simple_record(1245);test_game.players[side].hand_count=2;
        check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK);pass_chain();
        check(test_game.summon_activity[side]&&command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_NOT_ALLOWED);
        /* Summoning during the response invalidates the initial resolution
         * gate; no tokens and no player restriction are applied. */
        tokens_fixture(side,5);test_game.players[side].hand[0]=simple_record(1245);test_game.players[side].hand_count=1;magnet_hold(side);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.chain_count);test_game.summon_activity[side]=1;pass_chain();
        check(!token_count(side,1922)&&!test_game.summon_ban_turn[side]);
        /* Activation negation skips the callback entirely. */
        tokens_fixture(side,5);test_game.players[side].hand[0]=simple_record(1245);test_game.players[side].hand_count=1;test_game.players[1-side].backrow[0]=slot(893,1-side,DUEL_FACEDOWN_DEFENSE);test_game.players[1-side].backrow[0].set_turn=1;test_game.players[1-side].hand[0]=simple_record(77);test_game.players[1-side].hand_count=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);c=(DuelCommand){1-side,DUEL_ACTION_ACTIVATE_BACKROW,0};
        check(duel_command(&test_game,&c)==DUEL_OK);pass_chain();
        check(!token_count(side,1922)&&!test_game.summon_ban_turn[side]);
        /* The token arrival has its own ordinary Summon response window. */
        tokens_fixture(side,5);test_game.players[side].hand[0]=simple_record(1245);test_game.players[side].hand_count=1;test_game.players[1-side].backrow[0]=slot(272,1-side,DUEL_FACEDOWN_DEFENSE);test_game.players[1-side].backrow[0].set_turn=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.window.kind==DUEL_WINDOW_SUMMON&&token_count(side,1922)==4);c=(DuelCommand){1-side,DUEL_ACTION_ACTIVATE_BACKROW,0};
        check(duel_command(&test_game,&c)==DUEL_OK);pass_chain();
        check(!token_count(side,1922)&&test_game.players[side].grave_count==1);for(unsigned i=0;i<test_game.players[side].grave_count;++i)check(!duel_is_token(&test_game,test_game.players[side].grave[i]));
        /* Upkeep resolves before Jam's continuous token-production step. */
        tokens_fixture(side,5);expansion_place(side,0,1426);expansion_place(side,4,1154);test_game.phase=DUEL_DRAW;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_STANDBY,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_UPKEEP);
        check(choose(side,DUEL_NO_SLOT)==DUEL_OK);pass_chain();
        check(token_count(side,1923)==1&&!duel_summon_permission(&test_game,side,DUEL_PERMISSION_SPECIAL)&&duel_summon_permission(&test_game,side,DUEL_PERMISSION_SET));
        /* A constructed face-down token banishes by disappearing rather
         * than creating a new banished-pile card. */
        expansion_fixture(side);test_game.players[1-side].monsters[0]=slot(simple_record(1922),1-side,DUEL_FACEDOWN_DEFENSE);test_game.players[side].hand[0]=simple_record(1157);test_game.players[side].hand_count=1;
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,1-side,0,0,0)==DUEL_OK);pass_chain();
        check(!test_game.players[1-side].monsters[0].card&&!test_game.players[1-side].banished_count&&!test_game.players[1-side].grave_count);
    }
    uint16_t deck[52];const unsigned extra[12]={1232,1245,1426,57,57,335,1079,1154,1151,1056,1008,1510};unsigned completed=0,arrivals=0,plays=0;for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<12;++i)deck[40+i]=simple_record(extra[i]);
    for(unsigned seed=1;seed<=10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,52,deck,52,seed+7900,seed&1)==DUEL_OK);check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK&&duel_field_setup(&test_game,duel_field_rules,DUEL_FIELD_RULE_COUNT)==DUEL_OK&&duel_suppression_setup(&test_game,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT)==DUEL_OK);unsigned actions=0;DuelCommand c;while(test_game.winner<0&&actions++<9000){check(duel_ai_decide(&test_game,decision_player(),&c));unsigned before=0;for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i)before+=duel_is_token(&test_game,test_game.players[p].monsters[i].card);plays+=c.action==DUEL_ACTION_ACTIVATE_HAND||c.action==DUEL_ACTION_ACTIVATE_BACKROW;int r=duel_command(&test_game,&c);check(r==DUEL_OK);if(r!=DUEL_OK)break;unsigned after=0;for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i)after+=duel_is_token(&test_game,test_game.players[p].monsters[i].card);if(after>before)arrivals+=after-before;check(owned_cards(&test_game,0)==52&&owned_cards(&test_game,1)==52);}check(test_game.winner>=0);completed+=test_game.winner>=0;}
    check(completed==10&&arrivals>0);native_tokens_test_result[2]=completed;native_tokens_test_result[3]=arrivals;native_tokens_test_result[5]=plays;
    native_tokens_test_result[0]=checks-initial;native_tokens_test_result[1]=matrix;native_tokens_test_result[4]=reference;
}
