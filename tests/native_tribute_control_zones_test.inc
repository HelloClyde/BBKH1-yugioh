#include "original_tribute_control_zones_cases.h"
volatile uint32_t native_tribute_control_zones_test_result[6];
static void zones_tick(void){check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK);
pass_chain();}
static void zones_umi(unsigned side){test_game.players[side].field=slot(simple_record(333),side,DUEL_FACEUP_ATTACK);test_game.players[side].field.serial=350;test_game.players[side].field.effect_active=1;}
static void zones_root(unsigned name,unsigned actor,unsigned responding){pair_chain(name,actor);DuelChainLink *l=&test_game.chain[0];l->effect_activation=0;l->source_slot=0;l->speed=name==1030?3:name==1079?2:1;test_game.players[actor].backrow[0]=slot(simple_record(name),actor,DUEL_FACEUP_ATTACK);test_game.players[actor].backrow[0].serial=l->source_serial;test_game.response_player=responding;}
static void tribute_control_zones_tests(void){unsigned initial=checks,matrix=0,reference=0;DuelCommand c;
    for(unsigned n=0;n<sizeof(original_mary_gate_cases)/sizeof(original_mary_gate_cases[0]);++n){const int *v=original_mary_gate_cases[n];unsigned actor=v[0];pair_fixture(v[1]);redirect_place(actor,1,1525,v[3],200);if(v[4]){expansion_place(actor,3,1418);test_game.players[actor].backrow[3].serial=303;}zones_root(v[5],v[2],actor);test_game.chain[0].source_slot=v[6]-5;test_game.chain[0].effect_activation=v[7]!=0;c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_MONSTER,1};int r=duel_command(&test_game,&c);
check((r==DUEL_OK)==(v[8]&&v[3]>=2));if(r==DUEL_OK)check(!test_game.players[actor].monsters[1].card&&test_game.players[actor].grave_count==1);++matrix;}
    for(unsigned n=0;n<sizeof(original_mary_speed_cases)/sizeof(original_mary_speed_cases[0]);++n){const int *v=original_mary_speed_cases[n];unsigned actor=v[0];pair_fixture(1-actor);redirect_place(actor,1,1525,2,200);zones_root(v[1],1-actor,actor);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_MONSTER,1};
check((duel_command(&test_game,&c)==DUEL_OK)==v[2]);++matrix;}
    for(unsigned n=0;n<sizeof(original_mary_body_cases)/sizeof(original_mary_body_cases[0]);++n){const int *v=original_mary_body_cases[n];if(v[3]!=0x80){++reference;continue;}pair_fixture(v[0]);zones_root(1079,v[1],1-v[0]);DuelChainLink *l=&test_game.chain[1];*l=(DuelChainLink){0};l->card=simple_record(1525);l->command.player=v[0];l->command.target=0;l->source_slot=1;l->source_serial=200;l->speed=2;l->effect_activation=1;l->effect_negated=v[2]!=0;test_game.chain_count=2;
pass_chain();
check((test_game.chain[0].negated!=0)==v[4]);++matrix;}
    reference+=sizeof(original_mary_cost_cases)/sizeof(original_mary_cost_cases[0]);
    for(unsigned n=0;n<sizeof(original_mystic_gate_cases)/sizeof(original_mystic_gate_cases[0]);++n){const int *v=original_mystic_gate_cases[n];pair_fixture(v[0]);if(v[1])redirect_place(v[0],0,77,2,200);if(v[2])redirect_place(1-v[0],0,57,2,201);contact_test_hand(v[0],0,1213,500);int r=command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0);
check((r==DUEL_OK)==(v[3]!=0));if(r==DUEL_OK)check(test_game.choice.kind==DUEL_CHOICE_MYSTIC_DESTROY&&test_game.players[v[0]].hand_count==1&&!test_game.chain_count);++matrix;}
    for(unsigned n=0;n<sizeof(original_mystic_target_cases)/sizeof(original_mystic_target_cases[0]);++n){const int *v=original_mystic_target_cases[n];unsigned actor=v[0],side=v[1];pair_fixture(actor);redirect_place(side,0,v[2],v[3],200);redirect_place(1-side,0,57,2,201);if(v[4])zones_umi(actor);contact_test_hand(actor,0,1213,500);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_HAND,0};c.cost=1;c.target_count=2;c.targets[0]=(1-actor)*5;c.targets[1]=actor*5;
check((duel_command(&test_game,&c)==DUEL_OK)==v[5]);++matrix;}
    for(unsigned n=0;n<sizeof(original_mystic_start_cases)/sizeof(original_mystic_start_cases[0]);++n){const int *v=original_mystic_start_cases[n];unsigned actor=v[0];pair_fixture(actor);unsigned sides[]={1-actor,actor},indices[]={0,1},uids[]={200,201};for(unsigned i=0;i<2;++i)if(v[1]>>i&1)redirect_place(sides[i],indices[i],77,2,(v[2]>>i&1)?220+i:uids[i]);spirit_fake_spell(1213,actor);DuelChainLink *l=&test_game.chain[0];l->effect_negated=v[3]!=0;l->target_count=2;for(unsigned i=0;i<2;++i)l->targets[i]=(DuelTarget){uids[i],sides[i],indices[i]};
pass_chain();
check((test_game.players[1-actor].monsters[0].serial==201)==v[4]);++matrix;}
    reference+=sizeof(original_mystic_finish_cases)/sizeof(original_mystic_finish_cases[0]);reference+=sizeof(original_mystic_control_cases)/sizeof(original_mystic_control_cases[0]);reference+=sizeof(original_mystic_reconcile_cases)/sizeof(original_mystic_reconcile_cases[0]);
    for(unsigned n=0;n<sizeof(original_ground_gate_cases)/sizeof(original_ground_gate_cases[0]);++n){const int *v=original_ground_gate_cases[n];pair_fixture(0);for(unsigned t=0;t<10;++t)if(v[0]>>t&1)redirect_place(t/5,t%5,77,2,100+t);contact_test_hand(0,0,1320,500);
check((command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK)==v[1]);++matrix;}
    for(unsigned n=0;n<sizeof(original_ground_body_cases)/sizeof(original_ground_body_cases[0]);++n){const int *v=original_ground_body_cases[n];if(v[2]<2&&!v[3]){++reference;continue;}unsigned actor=v[0];pair_fixture(actor);for(unsigned t=0;t<2;++t)if(v[4]>>t&1)redirect_place(t,t,77,2,200+t);spirit_fake_spell(1320,actor);DuelChainLink *l=&test_game.chain[0];l->effect_negated=v[5]!=0;l->command.target_count=2;l->command.targets[0]=0;l->command.targets[1]=6;DuelSlot *s=&test_game.players[actor].backrow[4];s->card=simple_record(v[1]);s->serial+=v[3];s->position=v[2]<2?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK;
pass_chain();unsigned marked=(s->original_until&1)!=0;marked+=(s->original_until&64)!=0;
check(marked==(unsigned)v[6]);++matrix;}
    for(unsigned n=0;n<sizeof(original_ground_zone_cases)/sizeof(original_ground_zone_cases[0]);++n){const int *v=original_ground_zone_cases[n];unsigned actor=v[0],target=v[1];pair_fixture(actor);expansion_place(actor,1,1320);DuelSlot *s=&test_game.players[actor].backrow[1];s->serial=301;s->position=v[4]<2?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_ATTACK;s->effect_active=v[5]!=2;s->original_until=1u<<(target*5+v[3]);if(v[5]==32){expansion_place(1-actor,4,1154);test_game.players[1-actor].backrow[4].serial=304;}if(v[6])state_prohibition(1-actor,1320,0);zones_tick();
check((!test_game.players[target].monsters[v[2]].zone_blocked)==v[7]);++matrix;}
    for(unsigned actor=0;actor<2;++actor){unsigned enemy=1-actor;
        /* A paid Mary remains paid when its Monster effect is negated. */
        pair_fixture(enemy);redirect_place(actor,1,1525,2,200);zones_root(1008,enemy,actor);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_MONSTER,1};
check(duel_command(&test_game,&c)==DUEL_OK);test_game.chain[1].effect_negated=1;
pass_chain();
check(!test_game.players[actor].monsters[1].card&&test_game.players[actor].grave_count==1&&test_game.players[enemy].grave_count==1&&identity_unique(&test_game));
        /* Magic Box explicitly prefers the destroyed Monster's former slot,
         * and it gives hidden Monsters without changing physical ownership. */
        pair_fixture(actor);redirect_place(actor,2,77,1,200);redirect_place(enemy,4,21,2,201);contact_test_hand(actor,0,1213,500);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
check(choose(actor,enemy*5+4)==DUEL_OK&&choose(actor,actor*5+2)==DUEL_OK);
pass_chain();
check(test_game.players[enemy].monsters[4].serial==200&&test_game.players[enemy].monsters[4].position==DUEL_FACEDOWN_DEFENSE&&test_game.players[enemy].monsters[4].owner==actor&&test_game.players[enemy].grave_count==1&&identity_unique(&test_game));
        /* The destruction target may resist Spells; the other Monster then
         * uses a different free destination. No free destination destroys it
         * as caused by its opposing prospective controller. */
        for(unsigned full=0;full<2;++full){pair_fixture(actor);zones_umi(actor);redirect_place(actor,1,77,2,200);redirect_place(enemy,4,1326,2,201);if(full)for(unsigned i=0;i<4;++i)redirect_place(enemy,i,57,2,210+i);contact_test_hand(actor,0,1213,500);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_HAND,0};c.cost=1;c.target_count=2;c.targets[0]=enemy*5+4;c.targets[1]=actor*5+1;
check(duel_command(&test_game,&c)==DUEL_OK);
pass_chain();
check(test_game.players[enemy].monsters[4].serial==201&&!test_game.players[actor].monsters[1].card);if(full)check(test_game.players[actor].grave_count==2&&(test_game.players[actor].grave_history[0]&(DUEL_HISTORY_ARRIVED|DUEL_HISTORY_OPPONENT))==(DUEL_HISTORY_ARRIVED|DUEL_HISTORY_OPPONENT));else check(test_game.players[enemy].monsters[0].serial==200);
check(identity_unique(&test_game));}
        /* A Spell-immune given Monster retains its permanent layer; removing
         * Umi later moves it without any second activation. */
        pair_fixture(actor);zones_umi(actor);redirect_place(actor,1,1326,2,200);redirect_place(enemy,4,77,2,201);contact_test_hand(actor,0,1213,500);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_HAND,0};c.cost=1;c.target_count=2;c.targets[0]=enemy*5+4;c.targets[1]=actor*5+1;
check(duel_command(&test_game,&c)==DUEL_OK);
pass_chain();
check(test_game.players[actor].monsters[1].serial==200&&test_game.control_layer_count==1);test_game.players[actor].field=empty_slot();zones_tick();
check(!test_game.players[actor].monsters[1].card&&test_game.players[enemy].monsters[0].serial==200);
        /* Selection may be cancelled before paying or moving the Spell. */
        pair_fixture(actor);contact_test_hand(actor,0,1320,500);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
check(choose(actor,enemy*5)==DUEL_OK&&choose(actor,DUEL_NO_SLOT)==DUEL_OK);
check(!test_game.chain_count&&test_game.players[actor].hand_state[0].serial==500);
        /* Ground forbids both explicit normal placement and automatic token
         * placement; overlapping sources and IO reenable the same nodes. */
        pair_fixture(actor);contact_test_hand(actor,0,1320,500);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
check(choose(actor,actor*5)==DUEL_OK&&choose(actor,actor*5+1)==DUEL_OK);
pass_chain();contact_test_hand(actor,0,57,100);
check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_ZONE_FULL);
check(command(DUEL_ACTION_SUMMON,0,0,1,0,0)==DUEL_ZONE_FULL);
check(command(DUEL_ACTION_SUMMON,0,0,2,0,0)==DUEL_OK);
pass_chain();
check(test_game.players[actor].monsters[2].serial==100);contact_test_hand(actor,0,1320,501);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_HAND,0};c.cost=1;c.target_count=2;c.targets[0]=actor*5+3;c.targets[1]=actor*5+4;
check(duel_command(&test_game,&c)==DUEL_OK);
pass_chain();contact_test_hand(actor,0,1232,502);
check(command(DUEL_ACTION_ACTIVATE_HAND,0,actor,2,0,0)==DUEL_OK);
pass_chain();
check(!test_game.players[actor].monsters[0].card&&!test_game.players[actor].monsters[1].card&&token_count(actor,1921)==1&&!test_game.players[actor].monsters[3].card&&!test_game.players[actor].monsters[4].card);
    }
    for(unsigned actor=0;actor<2;++actor){unsigned enemy=1-actor;
        /* A new hidden Ground at the old source coordinate receives the
         * old packet's nodes. Its later activation appends more nodes. */
        pair_fixture(actor);contact_test_hand(actor,0,1320,500);contact_test_hand(actor,1,1320,501);contact_test_hand(actor,2,1079,502);test_game.players[enemy].backrow[0]=slot(simple_record(1079),enemy,DUEL_FACEDOWN_DEFENSE);test_game.players[enemy].backrow[0].serial=302;test_game.players[enemy].backrow[0].set_turn=1;test_game.players[actor].backrow[1]=slot(simple_record(1094),actor,DUEL_FACEDOWN_DEFENSE);test_game.players[actor].backrow[1].serial=301;test_game.players[actor].backrow[1].set_turn=1;c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_HAND,0};c.cost=1;c.target_count=2;c.targets[0]=enemy*5;c.targets[1]=enemy*5+1;
check(duel_command(&test_game,&c)==DUEL_OK);c=(DuelCommand){enemy,DUEL_ACTION_ACTIVATE_BACKROW,0,actor,0};
check(duel_command(&test_game,&c)==DUEL_OK);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_BACKROW,1,enemy,0};
check(duel_command(&test_game,&c)==DUEL_OK);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_HAND,1,actor,0};
check(duel_command(&test_game,&c)==DUEL_OK);
pass_chain();
check(test_game.choice.kind==DUEL_CHOICE_OPTIONAL_SET&&choose(actor,0)==DUEL_OK);
pass_chain();DuelSlot *ground=&test_game.players[actor].backrow[0];
check(ground->serial==501&&ground->position==DUEL_FACEDOWN_DEFENSE&&ground->original_until==((1u<<(enemy*5))|(1u<<(enemy*5+1)))&&!test_game.players[enemy].monsters[0].zone_blocked);c=(DuelCommand){actor,DUEL_ACTION_ACTIVATE_BACKROW,0};c.cost=1;c.target_count=2;c.targets[0]=enemy*5+2;c.targets[1]=enemy*5+3;
check(duel_command(&test_game,&c)==DUEL_OK);
pass_chain();for(unsigned i=0;i<4;++i)check(test_game.players[enemy].monsters[i].zone_blocked);
        /* Spell suppression suspends existing nodes without erasing them;
         * Prohibition does not suspend them. Destroying the source erases
         * them and releases the last zone for ordinary revival. */
        expansion_place(enemy,4,1154);test_game.players[enemy].backrow[4].serial=304;zones_tick();for(unsigned i=0;i<4;++i)check(!test_game.players[enemy].monsters[i].zone_blocked);test_game.players[enemy].backrow[4]=empty_slot();test_game.phase=DUEL_MAIN1;DuelSlot saved=*ground;test_game.players[actor].backrow[0]=empty_slot();test_game.players[actor].backrow[2]=saved;test_game.players[actor].backrow[2].original_until=31u<<(enemy*5);state_prohibition(actor,1320,0);zones_tick();for(unsigned i=0;i<5;++i)check(test_game.players[enemy].monsters[i].zone_blocked);
        test_game.phase=DUEL_MAIN1;test_game.active=enemy;pair_grave(enemy,77,600,0,0);contact_test_hand(enemy,0,1008,601);c=(DuelCommand){enemy,DUEL_ACTION_ACTIVATE_HAND,0,enemy,test_game.players[enemy].grave_count-1};
check(duel_command(&test_game,&c)==DUEL_ZONE_FULL);test_game.players[actor].backrow[2]=empty_slot();zones_tick();
check(duel_command(&test_game,&c)==DUEL_OK);
pass_chain();
check(test_game.players[enemy].monsters[0].serial==600);
    }
    uint16_t deck[48];const unsigned extra[]={1525,1213,1320,1525,1213,1320,1079,77};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<8;++i)deck[40+i]=simple_record(extra[i]);unsigned completed=0;
    for(unsigned seed=0;seed<10;++seed){check(duel_start(&test_game,duel_card_definitions,deck,48,deck,48,17800+seed,seed&1)==DUEL_OK);
check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK);unsigned actions=0;while(test_game.winner<0&&actions++<7000){check(duel_ai_decide(&test_game,decision_player(),&c));int r=duel_command(&test_game,&c);
check(r==DUEL_OK);if(r!=DUEL_OK)break;
check(owned_cards(&test_game,0)==48&&owned_cards(&test_game,1)==48);
check(identity_unique(&test_game));}check(test_game.winner>=0);completed+=test_game.winner>=0;}check(completed==10);native_tribute_control_zones_test_result[2]=completed;
    native_tribute_control_zones_test_result[0]=checks-initial;native_tribute_control_zones_test_result[1]=matrix;native_tribute_control_zones_test_result[4]=reference;
}
