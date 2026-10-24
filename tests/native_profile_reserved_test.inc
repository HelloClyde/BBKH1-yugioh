/* Persistent regression: PC reserved records never become playable decks. */
static void profile_reserved_metadata_tests(void){static Duel duel,before;JoeyProfile p,saved;unsigned reserved=0;uint16_t deck[40];for(unsigned j=0;j<40;++j)deck[j]=24;
for(unsigned card=1;card<DUEL_CARD_COUNT;++card){if(duel_card_definitions[card].race<23)continue;++reserved;
    /* Reject before clearing the caller's live duel, for either player's deck. */
    deck[39]=card;for(unsigned side=0;side<2;++side){for(unsigned i=0;i<sizeof(duel);++i)((uint8_t *)&duel)[i]=0xa5;before=duel;CHECK(duel_start(&duel,duel_card_definitions,side?duel_original_deck:deck,side?DUEL_ORIGINAL_DECK_COUNT:40,side?deck:duel_original_deck,side?40:DUEL_ORIGINAL_DECK_COUNT,1,0)==DUEL_BAD_ARGUMENT);CHECK(equal(&duel,&before,sizeof(duel)));}
    for(unsigned pile=0;pile<3;++pile){
        joey_profile_init(&p,duel_original_owned,duel_original_deck,DUEL_ORIGINAL_DECK_COUNT,duel_original_fusion_deck,DUEL_ORIGINAL_FUSION_DECK_COUNT);p.owned[card]=255;saved=p;
        CHECK(joey_profile_change_pile(&p,duel_card_definitions,duel_original_limits,card,1,pile)==JOEY_PROFILE_CARD);CHECK(equal(&p,&saved,sizeof(p)));
        /* A forged nonzero ownership count cannot make a reserved record legal. */
        if(pile==JOEY_MAIN)p.main[p.main_count++]=card;else if(pile==JOEY_FUSION)p.fusion[p.fusion_count++]=card;else p.side[p.side_count++]=card;
        CHECK(joey_profile_validate(&p,duel_card_definitions,duel_original_limits,1)==JOEY_PROFILE_CARD);
        /* Removing a bad entry is allowed so a caller can repair a deck. */
        CHECK(joey_profile_change_pile(&p,duel_card_definitions,duel_original_limits,card,0,pile)==JOEY_PROFILE_OK);CHECK(joey_profile_validate(&p,duel_card_definitions,duel_original_limits,1)==JOEY_PROFILE_OK);
    }
    joey_profile_init(&p,duel_original_owned,duel_original_deck,DUEL_ORIGINAL_DECK_COUNT,duel_original_fusion_deck,DUEL_ORIGINAL_FUSION_DECK_COUNT);p.owned[card]=255;saved=p;CHECK(joey_profile_change(&p,duel_card_definitions,duel_original_limits,card,1)==JOEY_PROFILE_CARD);CHECK(equal(&p,&saved,sizeof(p)));
}
CHECK(reserved==4);}
