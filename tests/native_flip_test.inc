/* Explicit MIPS fixtures and AI games; these are not firmware input traces. */
volatile uint32_t native_flip_test_result[6];
static void targeted_flip_tests(void){
    unsigned initial=checks,hand,enemy_hand,count;uint32_t before;DuelChoiceOption options[DUEL_OPTION_MAX];DuelCommand c;
    for(unsigned which=0;which<2;++which){unsigned card=which?753:723;
        /* Mandatory selection also applies when only the source is present. */
        reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(card,0,DUEL_FACEDOWN_DEFENSE);hand=test_game.players[0].hand_count;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_FLIP_TARGET);
        check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==1&&options[0].card==card&&options[0].index==0&&options[0].kind==DUEL_OPTION_CARD);
        before=duel_state_hash(&test_game);check(choose(0,DUEL_NO_SLOT)==DUEL_BAD_TARGET&&choose(1,0)==DUEL_EFFECT_PENDING&&duel_state_hash(&test_game)==before);
        check(choose(0,0)==DUEL_OK&&!test_game.choice.kind&&!test_game.players[0].monsters[0].card);
        check(which?test_game.players[0].grave_count==1&&test_game.players[0].grave[0]==card:test_game.players[0].hand_count==hand+1&&test_game.players[0].hand[hand]==card&&!test_game.players[0].grave_count);
        /* Either field and face-down targets are legal without disclosing the
           opposing card; a stolen target returns to its original owner. */
        reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(card,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(834,0,DUEL_FACEDOWN_DEFENSE);hand=test_game.players[0].hand_count;enemy_hand=test_game.players[1].hand_count;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);
        count=duel_choice_options(&test_game,options,DUEL_OPTION_MAX);check(count==2&&options[1].index==5&&options[1].card==0&&options[1].kind==DUEL_OPTION_CARD);
        check(choose(0,5)==DUEL_OK&&!test_game.players[1].monsters[0].card&&test_game.players[0].monsters[0].card==card&&test_game.players[1].hand_count==enemy_hand);
        check(which?test_game.players[0].grave[0]==834:test_game.players[0].hand_count==hand+1&&test_game.players[0].hand[hand]==834);
        /* A battle-doomed source still resolves, cannot choose itself, and is
           sent to the grave only after the effect has moved the attacker. */
        reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(card,1,DUEL_FACEDOWN_DEFENSE);hand=test_game.players[0].hand_count;
        check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_FLIP_TARGET&&test_game.choice.player==1&&test_game.battle_pending&&!test_game.players[1].grave_count);
        check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==1&&options[0].index==0);
        before=duel_state_hash(&test_game);check(choose(1,5)==DUEL_BAD_TARGET&&choose(1,DUEL_NO_SLOT)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
        check(choose(1,0)==DUEL_OK&&!test_game.battle_pending&&!test_game.players[0].monsters[0].card&&!test_game.players[1].monsters[0].card&&test_game.players[1].grave[0]==card);
        check(which?test_game.players[0].grave[0]==24:test_game.players[0].hand_count==hand+1&&test_game.players[0].hand[hand]==24);
        check(test_game.players[0].life==8000&&test_game.players[1].life==8000);
        /* The same-name replacement at a selected slot is not the instance
           selected before the response window. */
        reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(card,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[0].backrow[0]=slot(851,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,5)==DUEL_OK&&test_game.response_player==1&&test_game.chain_count==1);
        test_game.players[1].monsters[0].serial=++test_game.next_serial;c=(DuelCommand){1,DUEL_ACTION_PASS};
        check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[1].monsters[0].card==550&&!test_game.chain_count);
        /* Destroying the source in an earlier chain link doesn't erase its
           effect. With no surviving target, it resolves without a substitute. */
        reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(card,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,5)==DUEL_OK&&test_game.response_player==1&&test_game.chain_count==1);
        c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].monsters[0].card&&!test_game.players[1].monsters[0].card&&!test_game.chain_count);
        check(test_game.players[0].grave[0]==card&&test_game.players[1].grave[0]==550);
        /* An explicitly negated link cannot move its selected target. */
        reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(card,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[0].backrow[0]=slot(851,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,5)==DUEL_OK&&test_game.chain_count==1);
        test_game.chain[0].negated=1;c=(DuelCommand){1,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[1].monsters[0].card==550);
    }
    /* Bug destruction is an effect, so a face-down Penguin never flips and
       Nimble doesn't receive its battle-only recovery. Sangan still searches. */
    for(unsigned i=0;i<3;++i){unsigned target=i==0?839:i==1?453:248;reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(753,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(target,1,DUEL_FACEDOWN_DEFENSE);
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,5)==DUEL_OK&&test_game.players[1].grave[0]==target&&test_game.players[1].life==8000);
        check(i==2?test_game.choice.kind==DUEL_CHOICE_SEARCH_ATTACK&&test_game.choice.player==1:!test_game.choice.kind);
    }
    /* Hane returns a Fusion monster to its owner's Fusion Deck. */
    reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(723,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(651,1,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&choose(0,5)==DUEL_OK&&!test_game.players[1].monsters[0].card&&test_game.players[1].fusion_count==1&&test_game.players[1].fusion[0]==651);
    native_flip_test_result[0]=checks-initial;
    /* Thirty complete games actually use both newly legal cards. */
    uint16_t deck[44];for(unsigned i=0;i<42;++i)deck[i]=duel_original_deck[i];deck[42]=723;deck[43]=753;
    for(unsigned seed=1;seed<=30;++seed){
        check(duel_start(&test_game,duel_card_definitions,deck,44,deck,44,seed+600,seed&1)==DUEL_OK);
        check(duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,duel_original_fusion_deck,1,duel_original_fusion_deck,1)==DUEL_OK);
        unsigned actions=0;while(test_game.winner<0&&actions++<3000){DuelCommand command;check(duel_ai_decide(&test_game,decision_player(),&command));
            if(command.action==DUEL_ACTION_CHOOSE&&test_game.choice.kind==DUEL_CHOICE_FLIP_TARGET)++native_flip_test_result[test_game.choice.card==723?2:3];
            if(command.action==DUEL_ACTION_SET_MONSTER){unsigned card=test_game.players[command.player].hand[command.source];if(card==723||card==753)++native_flip_test_result[4];}
            check(duel_command(&test_game,&command)==DUEL_OK);check(owned_cards(&test_game,0)==45&&owned_cards(&test_game,1)==45);
        }check(test_game.winner>=0);if(test_game.winner>=0)++native_flip_test_result[1];
    }
    check(native_flip_test_result[1]==30&&native_flip_test_result[2]&&native_flip_test_result[3]&&native_flip_test_result[4]);native_flip_test_result[5]=checks-initial;
}
