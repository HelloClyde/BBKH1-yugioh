#include "original_temporary_removal_cases.h"
volatile uint32_t native_temporary_removal_test_result[6];
static void temporary_seed(unsigned owner,unsigned name,unsigned serial,unsigned flags,unsigned meta){DuelPlayer *p=&test_game.players[owner];unsigned i=p->banished_count++;p->banished[i]=simple_record(name);p->banished_serial[i]=serial;p->banished_proper[i]=0;p->banished_history[i]=0;p->banished_flags[i]=flags;p->banished_position[i]=meta;if(flags==0x40&&(meta>>4)<5)p->temporary_reserved_mask|=1u<<((meta&1)*5+(meta>>4));}
static unsigned temporary_meta(unsigned side,unsigned pos,unsigned zone){return side|(pos>>1)<<1|(pos&1)<<2|zone<<4;}
static void temporary_job(unsigned actor,unsigned owner,unsigned kind,unsigned serial,unsigned due){(void)owner;test_game.revival_jobs[0]=(DuelRevivalJob){serial,0,simple_record(kind==2?1319:964),due,actor,1,0,kind};}
static void temporary_enter_standby(void){test_game.phase=DUEL_DRAW;check(command(DUEL_ACTION_PHASE,0,0,DUEL_STANDBY,0,0)==DUEL_OK);pass_chain();}
static void temporary_removal_tests(void){unsigned initial=checks,matrix=0,reference=0;DuelCommand c;DuelChoiceOption opts[DUEL_OPTION_MAX];
    for(unsigned n=0;n<sizeof(original_dimension_target_cases)/sizeof(original_dimension_target_cases[0]);++n){
        const int *v=original_dimension_target_cases[n];pair_fixture(v[0]);redirect_place(v[1],1,v[2],v[3],200);if(v[4])zones_umi(v[0]);contact_test_hand(v[0],0,1319,500);
        check((command(DUEL_ACTION_ACTIVATE_HAND,0,v[1],1,0,0)==DUEL_OK)==v[5]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_dimension_body_cases)/sizeof(original_dimension_body_cases[0]);++n){
        const int *v=original_dimension_body_cases[n];unsigned actor=v[0];pair_fixture(actor);redirect_place(actor,1,v[1],v[2],v[5]?201:200);test_game.players[actor].monsters[1].owner=v[3];if(v[4])zones_umi(actor);
        spirit_fake_spell(1319,actor);test_game.chain[0].command.target_player=actor;test_game.chain[0].command.target=1;test_game.chain[0].target_serial=200;test_game.chain[0].effect_negated=v[6]!=0;pass_chain();
        check((!test_game.players[actor].monsters[1].card)==v[7]&&test_game.players[v[3]].banished_count==(unsigned)v[7]);
        if(v[7])check(test_game.players[v[3]].banished_flags[0]==0x40&&test_game.players[v[3]].banished_position[0]==temporary_meta(actor,v[2],1)&&(test_game.players[actor].monsters[1].zone_blocked&2));++matrix;
    }
    for(unsigned n=0;n<sizeof(original_dimension_return_cases)/sizeof(original_dimension_return_cases[0]);++n){
        const int *v=original_dimension_return_cases[n];unsigned actor=v[0];pair_fixture(actor);temporary_seed(v[1],77,200,v[5],temporary_meta(actor,v[2],v[3]));temporary_job(actor,v[1],2,200,test_game.turn+v[4]);temporary_enter_standby();
        const DuelSlot *s=&test_game.players[actor].monsters[v[3]];check((s->card!=0)==v[6]&&test_game.players[v[1]].banished_count==1u-v[6]);
        if(v[6])check(s->serial==200&&s->owner==v[1]&&s->position==(v[2]<2?DUEL_FACEDOWN_DEFENSE:v[2]==3?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK)&&s->facedown_attack==(v[2]==0)&&s->summoned_turn==test_game.turn&&!s->special_summon_marker);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_dimension_zone_cases)/sizeof(original_dimension_zone_cases[0]);++n){
        const int *v=original_dimension_zone_cases[n];pair_fixture(v[1]);temporary_seed(v[0],77,200,v[5],v[1]|v[2]<<4);zones_tick();check((!(test_game.players[v[3]].monsters[v[4]].zone_blocked&2))==v[6]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_lightforce_gate_cases)/sizeof(original_lightforce_gate_cases[0]);++n){
        const int *v=original_lightforce_gate_cases[n];pair_fixture(v[0]);for(unsigned i=0;i<(unsigned)v[1];++i)contact_test_hand(1-v[0],i,77,200+i);test_game.players[v[0]].backrow[0]=slot(simple_record(964),v[0],DUEL_FACEDOWN_DEFENSE);test_game.players[v[0]].backrow[0].serial=300;test_game.players[v[0]].backrow[0].set_turn=1;
        check((command(DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0)==DUEL_OK)==v[2]);++matrix;
    }
    static const unsigned seeds[]={1,49,48};
    for(unsigned n=0;n<sizeof(original_lightforce_banish_cases)/sizeof(original_lightforce_banish_cases[0]);++n){
        const int *v=original_lightforce_banish_cases[n];unsigned actor=v[0],victim=1-actor;pair_fixture(v[1]);for(unsigned i=0;i<3;++i){contact_test_hand(victim,i,77,200+i);test_game.players[victim].hand_state[i].owner=v[2];}spirit_fake_spell(964,actor);test_game.chain[0].effect_negated=v[4]!=0;test_game.random_state=seeds[v[3]];pass_chain();
        check(test_game.players[victim].hand_count==(v[4]?3:2)&&test_game.players[v[2]].banished_count==!v[4]);
        if(!v[4])check(test_game.players[v[2]].banished_serial[0]==200u+v[3]&&test_game.players[v[2]].banished_flags[0]==0x80&&test_game.revival_jobs[0].actor==victim&&test_game.revival_jobs[0].due_turn==3+v[5]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_lightforce_return_cases)/sizeof(original_lightforce_return_cases[0]);++n){
        const int *v=original_lightforce_return_cases[n];pair_fixture(v[0]);if(v[4])pair_grave(v[1],77,200,0,0);else temporary_seed(v[1],77,200,v[3],0);temporary_job(v[0],v[1],3,200,test_game.turn+v[2]);temporary_enter_standby();
        check(test_game.players[v[0]].hand_count==(unsigned)v[5]);if(v[5])check(test_game.players[v[0]].hand_state[0].serial==200&&test_game.players[v[0]].hand_state[0].owner==v[1]);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_lightforce_transport_cases)/sizeof(original_lightforce_transport_cases[0]);++n){
        const int *v=original_lightforce_transport_cases[n];if(v[4]!=0x81){++reference;continue;}unsigned actor=v[0];pair_fixture(1-actor);contact_test_hand(actor,0,77,200);test_game.players[actor].hand_state[0].owner=v[1];test_game.players[actor].hand_state[0].proper=v[2];test_game.players[actor].hand_state[0].history=v[3];spirit_fake_spell(964,1-actor);pass_chain();
        check(test_game.players[v[1]].banished_flags[0]==v[5]&&test_game.players[v[1]].banished_serial[0]==200&&test_game.players[v[1]].banished_proper[0]==v[2]&&test_game.players[v[1]].banished_history[0]==DUEL_HISTORY_OPPONENT);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_dimension_install_cases)/sizeof(original_dimension_install_cases[0]);++n){
        const int *v=original_dimension_install_cases[n];pair_fixture(v[0]);temporary_seed(v[1],77,200,0x40,temporary_meta(v[0],v[2],v[3]));test_game.players[v[1]].banished_proper[0]=v[4];test_game.players[v[1]].banished_history[0]=v[5];temporary_job(v[0],v[1],2,200,test_game.turn);temporary_enter_standby();
        const DuelSlot *s=&test_game.players[v[0]].monsters[v[3]];check(s->serial==200&&s->owner==v[1]&&s->properly_summoned==v[4]&&s->card_history==v[5]&&!test_game.players[v[1]].banished_count);++matrix;
    }
    for(unsigned n=0;n<sizeof(original_temporary_miracle_candidate_cases)/sizeof(original_temporary_miracle_candidate_cases[0]);++n){
        const int *v=original_temporary_miracle_candidate_cases[n];pair_fixture(v[0]);temporary_seed(v[0],77,200,v[1],0);test_game.chain[0]=(DuelChainLink){0};test_game.chain[0].command.player=v[0];test_game.chain[0].card=simple_record(1549);test_game.pile_task=(DuelPileTask){40,v[0],0};test_game.choice=(DuelTrigger){DUEL_CHOICE_PILE_CARD,v[0],simple_record(1549),0};check(duel_choice_options(&test_game,opts,DUEL_OPTION_MAX)==(unsigned)v[2]);++matrix;
    }
    for(unsigned actor=0;actor<2;++actor){unsigned enemy=1-actor;
        /* A skipped due Standby expires its notice but not the physical zone
         * reservation. Ordinary returns still obey the same card identity. */
        pair_fixture(actor);redirect_place(actor,2,57,1,200);contact_test_hand(actor,0,1319,500);check(command(DUEL_ACTION_ACTIVATE_HAND,0,actor,2,0,0)==DUEL_OK);pass_chain();test_game.skip_next_turn[actor]=1;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);pass_chain();
        check(test_game.turn==6&&test_game.active==enemy&&test_game.players[actor].banished_count==1&&(test_game.players[actor].monsters[2].zone_blocked&2)&&!test_game.revival_jobs[0].used);
        /* Reserved-zone returns ignore a concurrent Ground block and Special
         * Summon restrictions. They restore the old controller and position. */
        pair_fixture(actor);temporary_seed(enemy,77,200,0x40,temporary_meta(actor,0,2));temporary_job(actor,enemy,2,200,test_game.turn);expansion_place(enemy,1,1320);test_game.players[enemy].backrow[1].serial=301;test_game.players[enemy].backrow[1].original_until=1u<<(actor*5+2);redirect_place(enemy,0,1510,2,201);temporary_enter_standby();
        check(test_game.players[actor].monsters[2].serial==200&&test_game.players[actor].monsters[2].owner==enemy&&test_game.players[actor].monsters[2].facedown_attack&&test_game.players[actor].monsters[2].zone_blocked==1&&!test_game.summon_activity[actor]&&!test_game.summon_notice.kind&&identity_unique(&test_game));
        /* Real Miracle Dig removes three visible entries, shifts the hidden
         * Sword metadata with its physical card and releases the old zone. */
        pair_fixture(actor);for(unsigned i=0;i<6;++i)temporary_seed(actor,77,200+i,i==0?0x40:i==1?0x80:0,temporary_meta(actor,2,2));temporary_job(actor,actor,2,200,test_game.turn+2);contact_test_hand(actor,0,1549,500);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);check(choose(actor,0)==DUEL_OK&&choose(actor,2)==DUEL_OK&&choose(actor,3)==DUEL_OK);pass_chain();
        check(test_game.players[actor].banished_count==3&&test_game.players[actor].banished_serial[0]==201&&test_game.players[actor].banished_flags[0]==0x80&&test_game.players[actor].banished_serial[1]==204&&!test_game.players[actor].banished_flags[1]&&!(test_game.players[actor].monsters[2].zone_blocked&2)&&identity_unique(&test_game));
        /* A token disappears and never reserves a slot or receives a timer. */
        pair_fixture(actor);redirect_place(actor,0,57,2,200);contact_test_hand(actor,0,1232,500);contact_test_hand(actor,1,1319,501);check(command(DUEL_ACTION_ACTIVATE_HAND,0,actor,0,0,0)==DUEL_OK);pass_chain();check(token_count(actor,1921)==5);check(command(DUEL_ACTION_ACTIVATE_HAND,0,actor,0,0,0)==DUEL_OK);pass_chain();check(token_count(actor,1921)==4&&!test_game.players[actor].banished_count&&!test_game.players[actor].monsters[0].zone_blocked&&!test_game.revival_jobs[0].used);
    }
    uint16_t deck[46];const unsigned extra[]={1319,964,1319,964,57,77};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<6;++i)deck[40+i]=simple_record(extra[i]);unsigned completed=0;
    for(unsigned seed=0;seed<10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,46,deck,46,18000+seed,seed&1)==DUEL_OK);check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));int r=duel_command(&test_game,&c);check(r==DUEL_OK);if(r!=DUEL_OK)break;check(owned_cards(&test_game,0)==46&&owned_cards(&test_game,1)==46);check(identity_unique(&test_game));}check(test_game.winner>=0);completed+=test_game.winner>=0;}check(completed==10);native_temporary_removal_test_result[2]=completed;
    native_temporary_removal_test_result[0]=checks-initial;native_temporary_removal_test_result[1]=matrix;native_temporary_removal_test_result[4]=reference;
}
