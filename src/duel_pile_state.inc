/* Typed card-instance transport shared by every pile and field procedure.
 * A move changes only the history/provenance its semantic cause requires. */
enum {PILE_HAND,PILE_DECK,PILE_FUSION,PILE_GRAVE,PILE_BANISHED};
static DuelCardState slot_card_state(const DuelSlot *s){return (DuelCardState){s->serial,s->card,s->properly_summoned,s->card_history,s->owner};}
static DuelCardState pile_card_state(const DuelPlayer *p,unsigned pile,unsigned index){
    if(pile==PILE_GRAVE)return (DuelCardState){p->grave_serial[index],p->grave[index],p->grave_proper[index],p->grave_history[index],p->pile_side};
    if(pile==PILE_BANISHED)return (DuelCardState){p->banished_serial[index],p->banished[index],p->banished_proper[index],p->banished_history[index],p->pile_side};
    const DuelCardState *states=pile==PILE_HAND?p->hand_state:pile==PILE_DECK?p->deck_state:p->fusion_state;
    const uint16_t *cards=pile==PILE_HAND?p->hand:pile==PILE_DECK?p->deck:p->fusion;
    return states[index].card==cards[index]?states[index]:(DuelCardState){0,cards[index],0,0,p->pile_side};
}
static DuelCardState take_pile_card(DuelPlayer *p,unsigned pile,unsigned index){
    DuelCardState result=pile_card_state(p,pile,index);unsigned reserved=pile==PILE_BANISHED&&p->banished_flags[index]==0x40;uint16_t *cards;unsigned count;
    if(pile==PILE_GRAVE){cards=p->grave;count=p->grave_count;}
    else if(pile==PILE_BANISHED){cards=p->banished;count=p->banished_count;}
    else if(pile==PILE_HAND){cards=p->hand;count=p->hand_count;}
    else if(pile==PILE_DECK){cards=p->deck;count=p->deck_count;}
    else{cards=p->fusion;count=p->fusion_count;}
    remove_card(cards,&count,index);
    if(pile==PILE_GRAVE){for(unsigned i=index;i<count;++i){p->grave_serial[i]=p->grave_serial[i+1];p->grave_proper[i]=p->grave_proper[i+1];p->grave_history[i]=p->grave_history[i+1];}p->grave_serial[count]=0;p->grave_proper[count]=p->grave_history[count]=0;p->grave_count=count;}
    else if(pile==PILE_BANISHED){for(unsigned i=index;i<count;++i){p->banished_serial[i]=p->banished_serial[i+1];p->banished_proper[i]=p->banished_proper[i+1];p->banished_history[i]=p->banished_history[i+1];p->banished_flags[i]=p->banished_flags[i+1];p->banished_position[i]=p->banished_position[i+1];}p->banished_serial[count]=0;p->banished_proper[count]=p->banished_history[count]=p->banished_flags[count]=p->banished_position[count]=0;p->banished_count=count;if(reserved){p->temporary_reserved_mask=0;for(unsigned i=0;i<count;++i)if(p->banished_flags[i]==0x40){unsigned meta=p->banished_position[i];if((meta>>4)<5)p->temporary_reserved_mask|=1u<<((meta&1)*5+(meta>>4));}}}
    else{DuelCardState *states=pile==PILE_HAND?p->hand_state:pile==PILE_DECK?p->deck_state:p->fusion_state;for(unsigned i=index;i<count;++i)states[i]=states[i+1];clear(&states[count],sizeof(*states));if(pile==PILE_HAND)p->hand_count=count;else if(pile==PILE_DECK)p->deck_count=count;else p->fusion_count=count;}
    return result;
}
static DuelCardState card_state_identify(Duel *d,DuelCardState state){if(!state.serial)state.serial=++d->next_serial;return state;}
static DuelCardState card_state_reset(DuelCardState state){state.proper=state.history=0;return state;}
static void slot_install_state(Duel *d,DuelSlot *slot,DuelCardState state){state=card_state_identify(d,state);monster_equipment_new_deployment(d,state.serial);battle_countdown_reset(d,state.serial);if(d->punch_defender.serial==state.serial)clear(&d->punch_defender,sizeof(d->punch_defender));slot->card=state.card;slot->owner=state.owner;slot->serial=state.serial;slot->properly_summoned=state.proper;slot->card_history=state.history;}
static void pile_forget_prohibition(Duel *d,uint32_t serial){if(!(d->field_name_cache&16)||!serial)return;for(unsigned i=0;i<10;++i){DuelProhibition *p=&d->prohibitions[i];for(unsigned j=0;j<p->count;){if(p->exempt[j]==serial){for(unsigned n=j;n+1<p->count;++n)p->exempt[n]=p->exempt[n+1];p->exempt[--p->count]=0;}else ++j;}}}
static int put_pile_card(Duel *d,unsigned owner,unsigned pile,DuelCardState state,unsigned bottom){
    if(!state.card||duel_is_token(d,state.card))return 1;
    if((pile==PILE_HAND||pile==PILE_DECK)&&d->cards[state.card].family==DUEL_MONSTER&&d->cards[state.card].subtype==2)pile=PILE_FUSION;
    if(pile==PILE_FUSION||pile==PILE_BANISHED)owner=state.owner;DuelPlayer *p=&d->players[owner];
    unsigned count=pile==PILE_HAND?p->hand_count:pile==PILE_DECK?p->deck_count:pile==PILE_FUSION?p->fusion_count:p->banished_count;
    if(count>=(pile==PILE_HAND?DUEL_HAND_MAX:pile==PILE_DECK?DUEL_DECK_MAX:pile==PILE_FUSION?DUEL_FUSION_MAX:DUEL_PILE_MAX))return 0;
    state=card_state_identify(d,state);
    if(state.owner==owner)pile_forget_prohibition(d,state.serial);
    if(pile==PILE_BANISHED){p->banished[count]=state.card;p->banished_serial[count]=state.serial;p->banished_proper[count]=state.proper;p->banished_history[count]=state.history;p->banished_flags[count]=p->banished_position[count]=0;p->banished_count=count+1;return 1;}
    uint16_t *cards=pile==PILE_HAND?p->hand:pile==PILE_DECK?p->deck:p->fusion;DuelCardState *states=pile==PILE_HAND?p->hand_state:pile==PILE_DECK?p->deck_state:p->fusion_state;
    unsigned at=bottom?0:count;if(bottom)for(unsigned i=count;i>0;--i){cards[i]=cards[i-1];states[i]=states[i-1];}cards[at]=state.card;states[at]=state;
    if(pile==PILE_HAND)p->hand_count=count+1;else if(pile==PILE_DECK)p->deck_count=count+1;else p->fusion_count=count+1;return 1;
}
