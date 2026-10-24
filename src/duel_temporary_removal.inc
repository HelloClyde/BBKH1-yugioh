/* Banish presentation/reservation metadata belongs to that pile entry.
 * Timed return notices are independent physical-identity references. */
static void temporary_zone_reconcile(Duel *d){
    unsigned mask=d->players[0].temporary_reserved_mask|d->players[1].temporary_reserved_mask;if(!mask&&!(d->field_name_cache&4096))return;
    for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[p].monsters[i];s->zone_blocked=(s->zone_blocked&1)|(((mask>>(p*5+i))&1)<<1);}
}
static void temporary_notice(Duel *d,unsigned kind,unsigned actor,unsigned card,uint32_t serial,unsigned count){unsigned at=revival_job_space(d);if(at<64)d->revival_jobs[at]=(DuelRevivalJob){serial,d->event_sequence,card,d->turn+count,actor,1,0,kind};}
static void temporary_end(Duel *d){for(unsigned i=0;i<64;++i){DuelRevivalJob *j=&d->revival_jobs[i];if(j->used&&(j->kind==2||j->kind==3)&&j->due_turn<=d->turn)clear(j,sizeof(*j));}}
static int temporary_find(const Duel *d,uint32_t serial,unsigned *owner){for(unsigned p=0;p<2;++p){int index=banished_index(&d->players[p],serial);if(index>=0){if(owner)*owner=p;return index;}}return -1;}
static void temporary_standby(Duel *d,unsigned side){
    for(unsigned n=0;n<64;++n){unsigned best=64;for(unsigned i=0;i<64;++i){const DuelRevivalJob *j=&d->revival_jobs[i];if(j->used&&!j->queued&&j->actor==side&&(j->kind==2||(j->kind==3&&j->due_turn<=d->turn))&&(best==64||j->order<d->revival_jobs[best].order))best=i;}if(best==64)break;
        DuelRevivalJob *j=&d->revival_jobs[best];j->queued=1;unsigned owner=0;int from=temporary_find(d,j->serial,&owner);if(from<0)continue;DuelPlayer *p=&d->players[owner];
        if(j->kind==3){if(d->players[side].hand_count>=DUEL_HAND_MAX)continue;DuelCardState state=card_state_reset(take_pile_card(p,PILE_BANISHED,from));put_pile_card(d,side,PILE_HAND,state,0);event(d,DUEL_EVENT_RETURN,side,0,state.card,0);clear(j,sizeof(*j));continue;}
        if(p->banished_flags[from]!=0x40)continue;unsigned meta=p->banished_position[from],actor=meta&1,to=meta>>4;if(to>=5||d->players[actor].monsters[to].card)continue;
        DuelCardState state=take_pile_card(p,PILE_BANISHED,from);state.history&=3;DuelSlot *s=&d->players[actor].monsters[to];clear(s,sizeof(*s));slot_install_state(d,s,state);s->position=meta&2?(meta&4?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK):DUEL_FACEDOWN_DEFENSE;s->facedown_attack=!(meta&2)&&!(meta&4);s->set_turn=s->summoned_turn=d->turn;s->return_controller=DUEL_NO_SLOT;
        event(d,DUEL_EVENT_RETURN,actor,to,state.card,0);clear(j,sizeof(*j));
    }
    for(unsigned i=0;i<64;++i)if(d->revival_jobs[i].used&&(d->revival_jobs[i].kind==2||d->revival_jobs[i].kind==3))d->revival_jobs[i].queued=0;
    temporary_zone_reconcile(d);duel_refresh_field_cache(d);
}
static int dimension_validate(const Duel *d,const DuelCommand *c,unsigned card){if(revival_job_space(d)>=64)return DUEL_NOT_ALLOWED;return c->target_player==c->player&&c->target<5&&d->players[c->player].monsters[c->target].card&&duel_can_target_monster(d,c->player,c->target)&&effect_affects_monster(d,card,c->player,c->target)?DUEL_OK:DUEL_BAD_TARGET;}
static void dimension_resolve(Duel *d,const DuelChainLink *link){
    unsigned side=link->command.player,index=link->command.target;if(index>=5||link->command.target_player!=side)return;DuelSlot *s=&d->players[side].monsters[index];if(!s->card||s->serial!=link->target_serial||!effect_affects_monster(d,link->card,side,index))return;
    DuelSlot old=*s;unsigned pos=s->position==DUEL_FACEDOWN_DEFENSE?(s->facedown_attack?0:1):s->position==DUEL_FACEUP_ATTACK?2:3;
    if(!duel_banish_field(d,side,index,0)||duel_is_token(d,old.card))return;int at=banished_index(&d->players[old.owner],old.serial);if(at<0)return;DuelPlayer *owner=&d->players[old.owner];owner->banished_flags[at]=0x40;owner->banished_position[at]=side|(pos>>1)<<1|(pos&1)<<2|index<<4;owner->temporary_reserved_mask|=1u<<(side*5+index);
    temporary_notice(d,2,side,link->card,old.serial,2);temporary_zone_reconcile(d);
}
static void lightforce_resolve(Duel *d,const DuelChainLink *link){
    unsigned side=1-link->command.player;DuelPlayer *p=&d->players[side];if(!p->hand_count)return;unsigned index=random_next(d)%p->hand_count;DuelCardState state=pile_card_state(p,PILE_HAND,index);DuelPlayer *owner=&d->players[state.owner];if(owner->banished_count>=DUEL_PILE_MAX)return;
    state=take_pile_card(p,PILE_HAND,index);state.history=DUEL_HISTORY_OPPONENT;put_pile_card(d,state.owner,PILE_BANISHED,state,0);int at=banished_index(owner,state.serial);if(at<0)return;owner->banished_flags[at]=0x80;owner->banished_position[at]=0;event(d,DUEL_EVENT_BANISH,side,index,state.card,0);
    temporary_notice(d,3,side,link->card,state.serial,6+(side^d->active));
}
