/* Explicit rule fixtures; all game-memory setup below is confined to the CPU
 * model, never used by firmware or physical gameplay acceptance. */
#include "original_equip_cases.h"
static DuelCardDef equip_fixture_defs[DUEL_CARD_COUNT];
volatile unsigned native_equip_complete_games;
static unsigned equip_activations;
static void equip_fixture(void){
    reset();test_game.turn=3;test_game.players[0].hand_count=1;test_game.players[0].hand[0]=614;
    test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);
    test_game.players[0].monsters[0].serial=100;test_game.next_serial=100;
}
static void equip_tests(void){
    unsigned first_check=checks;DuelCommand c;uint32_t before;
    for(unsigned i=0;i<DUEL_CARD_COUNT;++i)equip_fixture_defs[i]=duel_card_definitions[i];
    for(unsigned n=0;n<sizeof(original_equip_cases)/sizeof(original_equip_cases[0]);++n){
        unsigned card=0;
        for(unsigned i=0;i<DUEL_EQUIP_RULE_COUNT;++i)if(duel_card_definitions[duel_equip_rules[i].card].name_group==original_equip_cases[n].internal)card=duel_equip_rules[i].card;
        /* The original equip oracle returns only the local stat component;
           Lightning Blade's global component is covered in aura fixtures. */
        check(card!=0);reset();check(duel_aura_setup(&test_game,0,0)==DUEL_OK);test_game.cards=equip_fixture_defs;
        DuelCardDef *host=&equip_fixture_defs[24];host->attack=host->defense=1000;
        host->race=original_equip_cases[n].race;host->attribute=original_equip_cases[n].attribute;
        host->name_group=original_equip_cases[n].name==1249?61:original_equip_cases[n].name;
        for(unsigned side=0;side<2;++side){
            test_game.players[side].monsters[0]=slot(24,side,DUEL_FACEDOWN_DEFENSE);
            test_game.players[side].monsters[0].serial=100+side;
            check(!duel_equip_can_target(&test_game,card,side,0));
            test_game.players[side].monsters[0].position=DUEL_FACEUP_ATTACK;
            check(duel_equip_can_target(&test_game,card,side,0)==original_equip_cases[n].eligible);
        }
        unsigned side=n&1;test_game.players[0].hand_count=1;test_game.players[0].hand[0]=card;test_game.next_serial=101;
        before=duel_state_hash(&test_game);int result=command(DUEL_ACTION_ACTIVATE_HAND,0,side,0,0,0);
        check(result==(original_equip_cases[n].eligible?DUEL_OK:DUEL_BAD_TARGET));
        if(!original_equip_cases[n].eligible)check(before==duel_state_hash(&test_game));
        check(duel_attack_value(&test_game,side,0)==1000+original_equip_cases[n].attack);
        check(duel_defense_value(&test_game,side,0)==1000+original_equip_cases[n].defense);
    }
    /* Same host, multiple equipment, then independent removal and return. */
    equip_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
    check(test_game.players[0].backrow[0].effect_active&&test_game.players[0].backrow[0].link_serial==100&&duel_attack_value(&test_game,0,0)==2000&&duel_defense_value(&test_game,0,0)==1450);
    test_game.players[0].hand_count=2;test_game.players[0].hand[0]=614;test_game.players[0].hand[1]=149;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
    check(duel_attack_value(&test_game,0,0)==2700&&duel_defense_value(&test_game,0,0)==1550);
    test_game.players[0].hand_count=1;test_game.players[0].hand[0]=316;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,1,0,0)==DUEL_OK&&duel_attack_value(&test_game,0,0)==2400&&test_game.players[0].monsters[0].card==24);
    test_game.players[0].hand_count=1;test_game.players[0].hand[0]=729;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&duel_attack_value(&test_game,0,0)==1700);
    check(test_game.players[0].hand_count==2&&test_game.players[0].hand[0]==614&&test_game.players[0].hand[1]==149);
    /* Set equipment can activate the same turn; no empty zone or wrong
     * monster never pays a card. An explicitly unsupported Equip is rejected. */
    equip_fixture();check(command(DUEL_ACTION_SET_BACKROW,0,0,3,0,0)==DUEL_OK);
    check(command(DUEL_ACTION_ACTIVATE_BACKROW,3,0,0,0,0)==DUEL_OK&&test_game.players[0].backrow[3].effect_active);
    equip_fixture();for(unsigned i=0;i<5;++i)test_game.players[0].backrow[i]=slot(316,0,DUEL_FACEDOWN_DEFENSE);
    before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_ZONE_FULL&&before==duel_state_hash(&test_game));
    equip_fixture();for(unsigned i=0;i<DUEL_CARD_COUNT;++i)equip_fixture_defs[i]=duel_card_definitions[i];equip_fixture_defs[1065].effect=DUEL_E_UNSUPPORTED;test_game.cards=equip_fixture_defs;test_game.players[0].hand[0]=1065;before=duel_state_hash(&test_game);
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_UNSUPPORTED&&before==duel_state_hash(&test_game));equip_fixture_defs[1065]=duel_card_definitions[1065];
    /* Source removal before resolution and a negated activation give no bonus. */
    for(unsigned jammer=0;jammer<2;++jammer){
        equip_fixture();test_game.players[1].backrow[0]=slot(jammer?893:316,1,DUEL_FACEDOWN_DEFENSE);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1);
        c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK);
        check(duel_attack_value(&test_game,0,0)==1700&&!test_game.players[0].backrow[0].card&&test_game.players[0].monsters[0].card==24);
    }
    /* Instance tracking: control/zone movement is followed, a replacement
     * in the old slot is never accidentally equipped. */
    for(unsigned changed=0;changed<3;++changed){
        equip_fixture();test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);
        check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
        if(changed==0){test_game.players[1].monsters[2]=test_game.players[0].monsters[0];test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[0].serial=500;}
        else if(changed==1)test_game.players[0].monsters[0].serial=500;
        else test_game.players[0].monsters[0].position=DUEL_FACEDOWN_DEFENSE;
        c=(DuelCommand){1,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK);
        check(duel_attack_value(&test_game,0,0)==1700);
        check(changed==0?(duel_attack_value(&test_game,1,2)==2000&&test_game.players[0].backrow[0].effect_active):!test_game.players[0].backrow[0].card);
    }
    /* Real control change and automatic turn-end return preserve association. */
    equip_fixture();test_game.players[1].monsters[0]=test_game.players[0].monsters[0];test_game.players[1].monsters[0].owner=1;test_game.players[0].monsters[0]=empty_slot();
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,1,0,0,0)==DUEL_OK);
    test_game.players[0].hand_count=1;test_game.players[0].hand[0]=300;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,1,0,0,0)==DUEL_OK&&duel_attack_value(&test_game,0,0)==2000);
    check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK&&duel_attack_value(&test_game,1,0)==2000&&test_game.players[0].backrow[0].card==614);
    /* Tribute, destruction, Penguin return and Fusion material all release
     * equipment into its owner's grave; Call's orphan semantics are separate. */
    for(unsigned mode=0;mode<3;++mode){
        equip_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
        test_game.players[0].hand_count=1;test_game.players[0].hand[0]=mode==0?153:802;
        if(mode==0)check(command(DUEL_ACTION_SUMMON,0,0,0,0,1)==DUEL_OK);
        else if(mode==1)check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
        else {test_game.players[1].monsters[0]=slot(839,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0].serial=200;test_game.phase=DUEL_BATTLE;
            check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_PENGUIN);
            check(choose(1,0)==DUEL_OK&&choose(1,DUEL_NO_SLOT)==DUEL_OK);}
        check(!test_game.players[0].backrow[0].card);unsigned found=0;for(unsigned i=0;i<test_game.players[0].grave_count;++i)found+=test_game.players[0].grave[i]==614;check(found==1);
    }
    fusion_fixture();test_game.players[0].monsters[0]=slot(834,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[0].serial=100;
    test_game.players[0].hand_count=3;test_game.players[0].hand[0]=652;test_game.players[0].hand[1]=1026;test_game.players[0].hand[2]=1099;test_game.next_serial=100;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&duel_attack_value(&test_game,0,0)==1500);
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(DUEL_HAND_MAX,0,0);
    check(test_game.players[0].monsters[0].card==651&&!test_game.players[0].backrow[0].card&&duel_attack_value(&test_game,0,0)==1700);
    /* A later face-down host invalidates the attachment; negative DEF clamps
     * after summing all equipment, rather than once per card. */
    equip_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);test_game.players[0].monsters[0].position=DUEL_FACEDOWN_DEFENSE;
    check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK&&!test_game.players[0].backrow[0].card);
    equip_fixture();test_game.players[0].hand_count=5;for(unsigned i=0;i<5;++i)test_game.players[0].hand[i]=149;
    for(unsigned i=0;i<5;++i)check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
    check(duel_defense_value(&test_game,0,0)==150&&duel_attack_value(&test_game,0,0)==3700);
    equip_fixture_defs[24]=duel_card_definitions[24];equip_fixture_defs[24].defense=100;test_game.cards=equip_fixture_defs;
    check(duel_defense_value(&test_game,0,0)==0);
    /* Missing/malformed metadata is rejected atomically. */
    reset();DuelEquipRule malformed=duel_equip_rules[0];malformed.kind=99;
    check(duel_equip_setup(&test_game,&malformed,1)==DUEL_BAD_ARGUMENT&&test_game.equip_count==DUEL_EQUIP_RULE_COUNT);
    check(duel_equip_setup(&test_game,0,0)==DUEL_OK);test_game.players[0].hand[0]=614;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);before=duel_state_hash(&test_game);
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_UNSUPPORTED&&before==duel_state_hash(&test_game));
    uint16_t deck[40];for(unsigned i=0;i<40;++i)deck[i]=i<20?24:i<28?614:i<32?149:i<36?316:i<38?802:695;
    for(unsigned seed=1;seed<=30;++seed){
        check(duel_start(&test_game,duel_card_definitions,deck,40,deck,40,seed,seed&1)==DUEL_OK);
        check(duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT)==DUEL_OK);
        for(unsigned n=0;test_game.winner<0&&n<3000;++n){
            check(duel_ai_decide(&test_game,decision_player(),&c));
            if(c.action==DUEL_ACTION_ACTIVATE_HAND&&duel_card_definitions[test_game.players[c.player].hand[c.source]].effect==DUEL_E_EQUIP){++equip_activations;check(c.player==c.target_player);}
            int result=duel_command(&test_game,&c);check(result==DUEL_OK);if(result!=DUEL_OK)break;
            check(owned_cards(&test_game,0)==40&&owned_cards(&test_game,1)==40);
        }check(test_game.winner>=0);native_equip_complete_games+=test_game.winner>=0;
    }check(equip_activations>30);native_duel_test_result[22]=checks-first_check;native_duel_test_result[23]=equip_activations;
}
