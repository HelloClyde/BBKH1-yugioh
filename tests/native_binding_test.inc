#include "original_binding_cases.h"
volatile uint32_t native_binding_test_result[5];
static DuelCardDef binding_defs[DUEL_CARD_COUNT];
static void binding_fixture(unsigned side){expansion_fixture(side);test_game.cards=binding_defs;test_game.players[side].monsters[0]=slot(24,side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[0].serial=100;test_game.next_serial=100;}
static void binding_attach(unsigned name,unsigned owner,unsigned index,uint32_t serial){dynamic_equip_attach(owner,index,simple_record(name),serial);}
static void binding_begin(unsigned name,unsigned side){binding_fixture(1-side);test_game.active=side;test_game.players[side].backrow[0]=slot(simple_record(name),side,DUEL_FACEDOWN_DEFENSE);test_game.players[side].backrow[0].set_turn=1;metal_hold_response(side);check(command(DUEL_ACTION_ACTIVATE_BACKROW,0,1-side,0,0,0)==DUEL_OK&&test_game.chain_count==1);}
static void binding_tests(void){unsigned initial=checks,matrix=0,reference_only=0;DuelCommand c;for(unsigned i=0;i<DUEL_CARD_COUNT;++i)binding_defs[i]=duel_card_definitions[i];binding_defs[24].attack=1900;binding_defs[24].defense=900;
    for(unsigned n=0;n<sizeof(original_binding_target_cases)/sizeof(original_binding_target_cases[0]);++n){const int *v=original_binding_target_cases[n];if(!v[6]){++reference_only;continue;}binding_fixture(v[2]);test_game.players[v[2]].monsters[0]=empty_slot();if(v[4]){if(v[3]<5)test_game.players[v[2]].monsters[v[3]]=slot(24,v[2],v[5]);else test_game.players[v[2]].backrow[0]=slot(24,v[2],v[5]);}check(duel_binding_can_target(&test_game,v[1],simple_record(v[0]),v[2],v[3])==v[7]);++matrix;}
    for(unsigned n=0;n<sizeof(original_binding_resolve_cases)/sizeof(original_binding_resolve_cases[0]);++n){const int *v=original_binding_resolve_cases[n];unsigned side=v[1];binding_begin(v[0],side);DuelSlot *source=&test_game.players[side].backrow[0],*target=&test_game.players[1-side].monsters[0];if(v[2]==1)*source=empty_slot();if(v[2]==2){*source=slot(simple_record(v[0]==348?1244:348),side,DUEL_FACEUP_ATTACK);source->serial=++test_game.next_serial;}if(v[2]==3){source->position=DUEL_FACEDOWN_DEFENSE;source->serial=++test_game.next_serial;}if(!v[3])*target=empty_slot();else {if(!v[4])++target->serial;target->position=v[5];}test_game.chain[0].effect_negated=v[6];pass_chain();check(((source->card==simple_record(v[0])&&source->effect_active&&source->link_serial==target->serial&&target->card)!=0)==v[7]);++matrix;}
    for(unsigned n=0;n<sizeof(original_binding_live_cases)/sizeof(original_binding_live_cases[0]);++n){const int *v=original_binding_live_cases[n];if(v[5]){++reference_only;continue;}binding_fixture(v[1]);binding_attach(v[0],v[2],0,100);test_game.players[v[2]].backrow[0].position=v[3];if(v[4])expansion_place(1-v[2],1,1033);check(duel_binding_locked(&test_game,v[1],0)==v[6]);test_game.players[v[1]].monsters[0].summoned_turn=1;int result=command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_DEFENSE,0);check((result==DUEL_OK)==v[7]);++matrix;}
    for(unsigned n=0;n<sizeof(original_binding_numeric_cases)/sizeof(original_binding_numeric_cases[0]);++n){const int *v=original_binding_numeric_cases[n];if(v[4]){++reference_only;continue;}binding_fixture(v[0]);binding_attach(1244,v[1],0,100);test_game.players[v[1]].backrow[0].position=v[2];if(v[3])expansion_place(1-v[1],1,1033);if(v[5])test_game.reverse_turn=test_game.turn;check(duel_attack_value(&test_game,v[0],0)==v[6]&&duel_defense_value(&test_game,v[0],0)==v[7]);++matrix;}
    for(unsigned n=0;n<sizeof(original_binding_position_cases)/sizeof(original_binding_position_cases[0]);++n){const int *v=original_binding_position_cases[n];binding_fixture(v[1]);binding_attach(v[0],v[2],0,100);test_game.players[v[1]].monsters[0].position=v[3]?DUEL_FACEUP_DEFENSE:DUEL_FACEDOWN_DEFENSE;duel_change_monster_position(&test_game,v[1],0,v[3]?DUEL_FACEDOWN_DEFENSE:DUEL_FACEUP_DEFENSE);check(duel_binding_locked(&test_game,v[1],0)==v[4]);check((test_game.players[v[2]].backrow[0].card!=0)==v[5]);++matrix;}
    for(unsigned n=0;n<sizeof(original_binding_depart_cases)/sizeof(original_binding_depart_cases[0]);++n){const int *v=original_binding_depart_cases[n];binding_fixture(v[1]);binding_attach(v[0],v[2],0,100);if(v[3])expansion_place(1-v[2],1,1033);test_game.players[v[1]].hand_count=1;test_game.players[v[1]].hand[0]=v[4]?expansion_effect_card(DUEL_E_DARK_HOLE):simple_record(751);check(command(v[4]?DUEL_ACTION_ACTIVATE_HAND:DUEL_ACTION_SUMMON,0,0,0,0,v[4]?0:1)==DUEL_OK);pass_chain();check((test_game.players[v[2]].backrow[0].card==0)==v[5]);if(!v[5])check(!test_game.players[v[2]].backrow[0].link_serial);++matrix;}
    for(unsigned which=0;which<2;++which)for(unsigned side=0;side<2;++side){unsigned name=which?1244:348;
        binding_begin(name,side);expansion_place(1-side,2,1033);pass_chain();check(test_game.players[side].backrow[0].card==simple_record(name)&&test_game.players[side].backrow[0].effect_active&&!duel_binding_locked(&test_game,1-side,0));test_game.players[1-side].backrow[2]=empty_slot();check(duel_binding_locked(&test_game,1-side,0));
        /* Replacing the source by the same record still attaches, despite a
         * different source serial; the original resolver checks the record. */
        binding_begin(name,side);test_game.players[side].backrow[0].serial=++test_game.next_serial;pass_chain();check(duel_binding_locked(&test_game,1-side,0));
        /* Target destruction while face-down still counts as destruction. */
        binding_fixture(side);binding_attach(name,1-side,0,100);test_game.players[side].monsters[0].position=DUEL_FACEDOWN_DEFENSE;test_game.players[side].hand[0]=expansion_effect_card(DUEL_E_DARK_HOLE);test_game.players[side].hand_count=1;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);pass_chain();check(!test_game.players[1-side].backrow[0].card);
        /* Returning or banishing is a departure, not a destruction. */
        binding_fixture(side);binding_attach(name,1-side,0,100);test_game.active=1-side;test_game.players[1-side].monsters[1]=slot(simple_record(540),1-side,DUEL_FACEDOWN_DEFENSE);test_game.players[1-side].monsters[1].set_turn=1;check(command(DUEL_ACTION_POSITION,1,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_FLIP_TARGET);check(choose(1-side,side*5)==DUEL_OK);pass_chain();check(!test_game.players[side].monsters[0].card);check((test_game.players[1-side].backrow[0].card==0)==which);
        binding_fixture(side);binding_attach(name,1-side,0,100);test_game.players[side].monsters[0].position=DUEL_FACEDOWN_DEFENSE;test_game.active=1-side;test_game.players[1-side].hand[0]=simple_record(1157);test_game.players[1-side].hand_count=1;check(command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0)==DUEL_OK);pass_chain();check((test_game.players[1-side].backrow[0].card==0)==which);
        /* Source removal releases the same monster without destroying it. */
        binding_fixture(side);binding_attach(name,1-side,0,100);test_game.players[side].hand[0]=simple_record(1079);test_game.players[side].hand_count=1;check(command(DUEL_ACTION_ACTIVATE_HAND,0,1-side,0,0,0)==DUEL_OK);pass_chain();check(!duel_binding_locked(&test_game,side,0)&&test_game.players[side].monsters[0].card==24&&duel_attack_value(&test_game,side,0)==1900);
        /* The defender's real attack response prevents damage. */
        binding_fixture(1-side);test_game.phase=DUEL_BATTLE;test_game.players[side].backrow[0]=slot(simple_record(name),side,DUEL_FACEDOWN_DEFENSE);test_game.players[side].backrow[0].set_turn=1;check(command(DUEL_ACTION_ATTACK,0,side,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.response_player==side);check(duel_ai_decide(&test_game,side,&c)&&c.action==DUEL_ACTION_ACTIVATE_BACKROW&&c.target_player==1-side&&c.target==0);check(duel_command(&test_game,&c)==DUEL_OK);pass_chain();check(test_game.players[side].life==8000&&duel_binding_locked(&test_game,1-side,0));
    }
    /* The real reveal path calls attachment cleanup; the monster has not
     * departed, so its binding must remain. */
    for(unsigned which=0;which<2;++which){binding_fixture(0);binding_attach(which?1244:348,1,0,100);test_game.players[0].monsters[0].position=DUEL_FACEDOWN_DEFENSE;test_game.players[0].backrow[1]=slot(simple_record(1150),0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].backrow[1].set_turn=1;check(command(DUEL_ACTION_ACTIVATE_BACKROW,1,0,0,0,0)==DUEL_OK);pass_chain();check(test_game.players[0].monsters[0].position==DUEL_FACEUP_DEFENSE&&duel_binding_locked(&test_game,0,0)&&test_game.players[1].backrow[0].card);}
    /* Each live Shadow contributes independently; Reverse Trap reverses the
     * penalty and final Mirror Wall halving remains after that total. */
    binding_fixture(0);binding_attach(1244,1,0,100);binding_attach(1244,1,1,100);check(duel_attack_value(&test_game,0,0)==500);test_game.reverse_turn=test_game.turn;check(duel_attack_value(&test_game,0,0)==3300);test_game.reverse_turn=0;dynamic_equip_attach(1,2,simple_record(1099),100);test_game.stat_link_count=1;test_game.stat_links[0]=(DuelStatLink){test_game.players[1].backrow[2].serial,100,1};check(duel_attack_value(&test_game,0,0)==250);
    /* A counter negating activation still prevents the link and sends the
     * Trap away; ordinary field-effect negation only disables its association. */
    binding_begin(348,0);test_game.players[1].backrow[2]=slot(simple_record(1028),1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[2].set_turn=1;c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,2};check(duel_command(&test_game,&c)==DUEL_OK);pass_chain();check(!test_game.players[0].backrow[0].card&&!duel_binding_locked(&test_game,1,0));
    unsigned completed=0,activations=0;uint16_t deck[48];const unsigned extra[8]={348,348,1244,1244,335,1079,1033,540};for(unsigned i=0;i<40;++i)deck[i]=duel_original_deck[i];for(unsigned i=0;i<8;++i)deck[40+i]=simple_record(extra[i]);
    for(unsigned seed=1;seed<=10;++seed){
        check(duel_start(&test_game,duel_card_definitions,deck,48,deck,48,seed+7500,seed&1)==DUEL_OK);
        check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK);
        check(duel_suppression_setup(&test_game,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT)==DUEL_OK);
        unsigned actions=0;
        while(test_game.winner<0&&actions++<7000){
            check(duel_ai_decide(&test_game,decision_player(),&c));
            activations+=c.action==DUEL_ACTION_ACTIVATE_BACKROW&&c.source<5&&test_game.players[c.player].backrow[c.source].card&&test_game.cards[test_game.players[c.player].backrow[c.source].card].effect==DUEL_E_BIND;
            int result=duel_command(&test_game,&c);
#ifdef _WIN32
            if(result!=DUEL_OK)fprintf(stderr,"binding seed %u action %u player %u source %u target %u:%u result %d phase %u turn %u response %u\n",seed,c.action,c.player,c.source,c.target_player,c.target,result,test_game.phase,test_game.turn,test_game.response_player);if(result!=DUEL_OK)fprintf(stderr,"binding source card %u group %u effect %u position %u hand %u\n",test_game.players[c.player].backrow[c.source].card,test_game.cards[test_game.players[c.player].backrow[c.source].card].name_group,test_game.cards[test_game.players[c.player].backrow[c.source].card].effect,test_game.players[c.player].backrow[c.source].position,test_game.players[c.player].hand_count);
#endif
            check(result==DUEL_OK);if(result!=DUEL_OK)break;
            check(owned_cards(&test_game,0)==48&&owned_cards(&test_game,1)==48);
        }
#ifdef _WIN32
        if(test_game.winner<0)fprintf(stderr,"binding seed %u incomplete at %u actions phase %u turn %u\n",seed,actions,test_game.phase,test_game.turn);
#endif
        check(test_game.winner>=0);completed+=test_game.winner>=0;
    }
    check(completed==10);native_binding_test_result[0]=checks-initial;native_binding_test_result[1]=matrix;native_binding_test_result[2]=completed;native_binding_test_result[3]=activations;native_binding_test_result[4]=reference_only;
}
