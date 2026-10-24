/* A Deck equipment effect selects its monster before opening the response
 * chain. Source departure does not require its original monster to remain
 * in the grave. The first Deck Mask is searched again at resolution. */
static int deck_equipment_index(const Duel *d,unsigned side){for(unsigned i=0;i<d->players[side].deck_count;++i)if(d->cards[d->players[side].deck[i]].name_group==1258)return i;return -1;}
static unsigned deck_equipment_options(const Duel *d,DuelChoiceOption *options,unsigned capacity){
    unsigned side=d->choice.player,n=0;int at=deck_equipment_index(d,side);if(at<0)return 0;unsigned card=d->players[side].deck[at];
    for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i)if(duel_equip_can_target_for(d,side,card,p,i)){if(options&&n<capacity)options[n]=(DuelChoiceOption){d->players[p].monsters[i].card,p*5+i,DUEL_OPTION_CARD};++n;}
    if(!n){if(options&&capacity)options[0]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_CONFIRM};return 1;}
    return n;
}
static void deck_equipment_choose(Duel *d,unsigned index){unsigned side=d->choice.player;DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->card=d->choice.card;link->source_serial=d->choice.serial;link->effect_activation=1;link->speed=1;link->command.player=side;link->command.target_player=DUEL_NO_SLOT;link->command.target=DUEL_NO_SLOT;
    if(index!=DUEL_NO_SLOT){unsigned p=index/5,i=index%5;DuelSlot *s=&d->players[p].monsters[i];if(!s->serial)s->serial=++d->next_serial;link->command.target_player=p;link->command.target=i;link->target_serial=s->serial;link->target_card=s->card;}d->trigger_chain_pending=1;finish_choice(d);
}
static void deck_equipment_resolve(Duel *d,const DuelChainLink *link){unsigned actor=link->command.player,owner=link->command.target_player,index=link->command.target;DuelPlayer *p=&d->players[actor];int at=deck_equipment_index(d,actor);if(at<0)return;unsigned card=p->deck[at];int to=empty_zone(p->backrow);unsigned count=p->deck_count;
    if(to<0||owner>1||index>=5||d->players[owner].monsters[index].serial!=link->target_serial||!duel_equip_can_target_for(d,actor,card,owner,index)){DuelCardState sent=take_pile_card(p,PILE_DECK,at);sent.history&=3;grave_card_state(d,actor,sent);shuffle(d,p);event(d,DUEL_EVENT_PILE_DESTROY,actor,at,card,1);return;}
    DuelCardState equipped=take_pile_card(p,PILE_DECK,at);equipped.history&=3;DuelSlot *source=&p->backrow[to];clear(source,sizeof(*source));source->card=card;source->owner=actor;source->position=DUEL_FACEUP_ATTACK;slot_install_state(d,source,equipped);source_field_reveal(d,source);DuelChainLink attached=*link;attached.card=card;attached.source_slot=to;attached.source_serial=source->serial;attach_equip(d,&attached);shuffle(d,p);
}
