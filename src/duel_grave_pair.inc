/* A grave target is a complete physical-card snapshot, rather than a field
 * target or a mutable pile index. Both effects choose before the chain. */
int duel_grave_pair_candidate(const Duel *d,unsigned side,unsigned name,unsigned index,uint32_t source){
    if(!d||side>1||index>=d->players[side].grave_count)return 0;const DuelPlayer *p=&d->players[side];unsigned card=p->grave[index];const DuelCardDef *c=&d->cards[card];
    if(name==1241)return c->name_group==746||c->name_group==1240;
    return name==1116&&p->grave_serial[index]!=source&&duel_can_revive_for(d,side,side,index);
}
static int grave_pair_index(const DuelPlayer *p,const DuelCardState *state){for(unsigned i=0;i<p->grave_count;++i)if(p->grave[i]==state->card&&p->grave_serial[i]==state->serial&&p->grave_proper[i]==state->proper&&p->grave_history[i]==state->history&&p->pile_side==state->owner)return i;return -1;}
static unsigned grave_pair_candidates(const Duel *d,unsigned side,unsigned name,uint32_t source,DuelChoiceOption *options,unsigned capacity){if(!special_summon_allowed_for(d,side)||empty_zone(d->players[side].monsters)<0)return 0;unsigned n=0;for(unsigned i=0;i<d->players[side].grave_count;++i)if(duel_grave_pair_candidate(d,side,name,i,source)){if(options&&n<capacity)options[n]=(DuelChoiceOption){d->players[side].grave[i],i,DUEL_OPTION_CARD};++n;}return n;}
static void grave_pair_selection_next(Duel *d){DuelGravePairTask *t=&d->grave_pair_task;unsigned name=d->cards[t->card].name_group;
    while(t->round<(name==1116?2:1)){t->chooser=t->actor^t->round;if(grave_pair_candidates(d,t->chooser,name,t->source,0,0)){d->choice=(DuelTrigger){DUEL_CHOICE_GRAVE_PAIR,t->chooser,t->card,t->source};return;}++t->round;}
    if(name==1116||t->selected[0].card||t->selected[1].card){DuelChainLink *l=&d->chain[d->chain_count++];clear(l,sizeof(*l));l->card=t->card;l->command.player=t->actor;l->command.cost=1;l->source_serial=t->source;l->source_slot=DUEL_NO_SLOT;l->effect_activation=1;l->speed=1;l->grave_targets[0]=t->selected[0];l->grave_targets[1]=t->selected[1];d->trigger_chain_pending=1;}clear(t,sizeof(*t));finish_choice(d);
}
static void grave_pair_begin(Duel *d){DuelTrigger trigger=d->choice;DuelGravePairTask *t=&d->grave_pair_task;clear(t,sizeof(*t));t->card=trigger.card;t->actor=trigger.player;t->source=trigger.serial;grave_pair_selection_next(d);}
static void grave_pair_resolution_next(Duel *d){DuelGravePairTask *t=&d->grave_pair_task;unsigned name=d->cards[t->card].name_group;
    while(t->round<(name==1116?2:1)){t->chooser=name==1116?d->active^t->round:t->actor;unsigned side=t->chooser;DuelPlayer *p=&d->players[side];if((t->permission&(1u<<side))&&empty_zone(p->monsters)>=0&&t->selected[side].card&&grave_pair_index(p,&t->selected[side])>=0){d->choice=(DuelTrigger){DUEL_CHOICE_GRAVE_PAIR_POSITION,side,t->selected[side].card,0};return;}++t->round;}clear(t,sizeof(*t));finish_choice(d);
}
static void grave_pair_resolve(Duel *d,const DuelChainLink *l){if(l->effect_negated)return;DuelGravePairTask *t=&d->grave_pair_task;clear(t,sizeof(*t));t->card=l->card;t->actor=l->command.player;t->source=l->source_serial;t->selected[0]=l->grave_targets[0];t->selected[1]=l->grave_targets[1];for(unsigned side=0;side<2;++side)if(special_summon_allowed_for(d,side))t->permission|=1u<<side;grave_pair_resolution_next(d);}
static unsigned grave_pair_options(const Duel *d,DuelChoiceOption *options,unsigned capacity){const DuelGravePairTask *t=&d->grave_pair_task;unsigned side=t->chooser;
    if(d->choice.kind==DUEL_CHOICE_GRAVE_PAIR_POSITION){unsigned card=t->selected[side].card,defense=d->cards[t->card].name_group==1241||duel_normal_prohibited[card]?DUEL_FACEUP_DEFENSE:DUEL_FACEDOWN_DEFENSE;if(options&&capacity)options[0]=(DuelChoiceOption){card,DUEL_FACEUP_ATTACK,DUEL_OPTION_CONFIRM};if(options&&capacity>1)options[1]=(DuelChoiceOption){card,defense,DUEL_OPTION_CONFIRM};return 2;}
    unsigned n=grave_pair_candidates(d,side,d->cards[t->card].name_group,t->source,options,capacity);if(n){if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};++n;}return n;
}
static void grave_pair_choose(Duel *d,unsigned index){DuelGravePairTask *t=&d->grave_pair_task;unsigned side=t->chooser;DuelPlayer *p=&d->players[side];
    if(d->choice.kind==DUEL_CHOICE_GRAVE_PAIR){if(index!=DUEL_NO_SLOT)t->selected[side]=pile_card_state(p,PILE_GRAVE,index);++t->round;grave_pair_selection_next(d);return;}
    int from=grave_pair_index(p,&t->selected[side]),to=empty_zone(p->monsters);if(from>=0&&to>=0){DuelCardState state=take_pile_card(p,PILE_GRAVE,from);state.history&=3;DuelSlot *s=&p->monsters[to];clear(s,sizeof(*s));slot_install_state(d,s,state);s->position=index;s->set_turn=s->summoned_turn=d->turn;s->return_controller=DUEL_NO_SLOT;special_summon_success(d,side,to);d->summon_notice=(DuelWindow){DUEL_WINDOW_SUMMON,side,to,0,s->serial,0,s->card,0,2};revival_triggers(d,side);event(d,DUEL_EVENT_REVIVE,side,to,s->card,0);}++t->round;grave_pair_resolution_next(d);
}
