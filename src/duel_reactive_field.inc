/* Notices describe completed movements and damage, independently of where
 * the physical card moves again before the next response window. */
static void reactive_field_move(Duel *d,unsigned controller,unsigned backrow,const DuelSlot *old,unsigned destination,unsigned causer){
    unsigned family=d->cards[old->card].family;
    if(destination==14){
        if(!backrow&&family==DUEL_MONSTER&&d->cards[old->card].race<21)d->reactive_pending.michi|=1u<<old->owner;
        if(backrow&&causer<2&&causer!=controller){if(family==DUEL_TRAP)d->reactive_pending.snow|=1u<<controller;else d->reactive_pending.gust|=1u<<controller;}
    }else if(destination==11&&!backrow&&family==DUEL_MONSTER&&d->cards[old->card].race<21&&causer<2&&causer!=controller)d->reactive_pending.riot|=1u<<controller;
}
static int reactive_field_name(unsigned name){return name==1134||name==1135||name==1140||name==1304||name==1301||name==1149;}
static int reactive_field_gate(const Duel *d,unsigned actor,unsigned name){
    const DuelReactiveNotice *a=&d->reactive_current,*b=&d->reactive_pending;unsigned mask=name==1134?(a->gust|b->gust):name==1135?(a->snow|b->snow):name==1301?(a->michi|b->michi):name==1149?(a->riot|b->riot):(a->damage|b->damage);return !!(mask&(1u<<actor));
}
static int reactive_field_validate(const Duel *d,const DuelCommand *c,unsigned card){
    unsigned name=d->cards[card].name_group;if(c->action!=DUEL_ACTION_ACTIVATE_BACKROW||!reactive_field_gate(d,c->player,name))return DUEL_NOT_ALLOWED;
    if(name==1140||name==1304)return DUEL_OK;
    if(c->target_player>1)return DUEL_BAD_TARGET;
    if(name==1301)return c->target<5&&d->players[c->target_player].monsters[c->target].card&&duel_can_target_monster(d,c->target_player,c->target)?DUEL_OK:DUEL_BAD_TARGET;
    if(c->target>DUEL_FIELD_SLOT||(c->player==c->target_player&&c->source==c->target))return DUEL_BAD_TARGET;
    return spell_zone((DuelPlayer *)&d->players[c->target_player],c->target)->card?DUEL_OK:DUEL_BAD_TARGET;
}
static void reactive_field_resolve(Duel *d,const DuelChainLink *l){
    unsigned name=d->cards[l->card].name_group,side=l->command.player;const DuelCommand *c=&l->command;
    if(name==1140||name==1304){unsigned copies=0;for(unsigned i=0;i<d->players[side].grave_count;++i)copies+=d->players[side].grave[i]==l->card;
        if(name==1140){recover(d,side,1000,l->card);for(unsigned i=0;i<copies;++i)recover(d,side,500,l->card);}else{damage(d,1-side,700);for(unsigned i=0;i<copies;++i)damage(d,1-side,300);}return;}
    unsigned backrow=name!=1301;DuelSlot *s=backrow?spell_zone(&d->players[c->target_player],c->target):&d->players[c->target_player].monsters[c->target];
    if(s->card&&s->serial==l->target_serial){if(backrow)destroy(d,c->target_player,c->target,1);else effect_destroy_monster(d,l->card,c->target_player,c->target);}
}
static int reactive_field_ai(const Duel *d,unsigned side,DuelCommand *c,unsigned card){
    unsigned name=d->cards[card].name_group;if(name==1140||name==1304)return reactive_field_validate(d,c,card)==DUEL_OK;
    for(unsigned order=0;order<2;++order){unsigned target=1-side^order;for(unsigned i=0;i<(name==1301?5:6);++i){c->target_player=target;c->target=i;if(reactive_field_validate(d,c,card)==DUEL_OK)return 1;}}return 0;
}
