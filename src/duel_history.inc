/* Shared native pile history and owner-End jobs. History belongs to the pile
 * instance; an entry leaving and re-entering the grave gets fresh history. */
static unsigned history_return_name(unsigned name){return name==1430||name==1433||name==1439||name==1444;}
int duel_history_can_return(const Duel *d,unsigned actor,unsigned name,unsigned index){
    if(!d||actor>1||index>=d->players[actor].grave_count||(!history_return_name(name)&&name!=1524&&name!=1421))return 0;
    const DuelPlayer *p=&d->players[actor];unsigned card=p->grave[index],race=d->cards[card].original_property>>20&31;
    if(name==1524)return (p->grave_history[index]&3)==DUEL_HISTORY_FUSION&&(!toon_card(&d->cards[card])||toon_world_present(d,actor));
    return name==1439?race==22&&(p->grave_history[index]&DUEL_HISTORY_OPPONENT):card&&race<21&&(p->grave_history[index]&DUEL_HISTORY_BATTLE);
}
static unsigned history_target_options(const Duel *d,unsigned actor,unsigned name,DuelChoiceOption *options,unsigned capacity){
    const DuelPlayer *p=&d->players[actor];unsigned n=0;
    for(unsigned i=0;i<p->grave_count;++i)if(duel_history_can_return(d,actor,name,i)){
        unsigned fusion=d->cards[p->grave[i]].subtype==2;
        if(fusion?p->fusion_count>=DUEL_FUSION_MAX:(name==1444||name==1524||name==1421)?p->hand_count>=DUEL_HAND_MAX:p->deck_count>=DUEL_DECK_MAX)continue;
        if(options&&n<capacity)options[n]=(DuelChoiceOption){p->grave[i],i,DUEL_OPTION_CARD};++n;
    }return n;
}
static unsigned history_supply_capacity(const Duel *d,unsigned actor){const DuelPlayer *p=&d->players[actor];unsigned hand=0,fusion=0;for(unsigned i=0;i<p->grave_count;++i)if(duel_history_can_return(d,actor,1524,i)){if(d->cards[p->grave[i]].subtype==2)++fusion;else ++hand;}unsigned hand_room=DUEL_HAND_MAX-p->hand_count,fusion_room=DUEL_FUSION_MAX-p->fusion_count;return (hand<hand_room?hand:hand_room)+(fusion<fusion_room?fusion:fusion_room);}
static void history_supply_begin(Duel *d,unsigned actor,unsigned card){if(history_supply_capacity(d,actor)>=2)d->choice=(DuelTrigger){DUEL_CHOICE_HISTORY_TARGET,actor,card,0,2};}
static void history_schedule(Duel *d,unsigned actor,unsigned card){if(d->end_timer_count<64)d->end_timers[d->end_timer_count++]=(DuelEndTimer){card,d->turn,actor,0,0};}
int duel_last_will_candidate(const Duel *d,unsigned actor,unsigned card){if(!d||actor>1||card>=DUEL_CARD_COUNT)return 0;const DuelCardDef *c=&d->cards[card];return (c->original_property>>20&31)<21&&c->attack<=1500&&(!duel_normal_prohibited[card]||(toon_card(c)&&toon_world_present(d,actor)));}
static unsigned history_will_available(const Duel *d,unsigned actor){return d->field_to_grave[actor]&&pile_summon_candidates(d,actor,7,0,0);}
int duel_last_will_ready(const Duel *d,unsigned actor){if(!d||actor>1||actor!=d->active||d->response_player<2||d->chain_count||d->choice.kind||d->resolving||!history_will_available(d,actor))return 0;for(unsigned i=0;i<d->end_timer_count;++i)if(d->end_timers[i].actor==actor&&d->end_timers[i].turn==d->turn&&!d->end_timers[i].used&&d->cards[d->end_timers[i].card].name_group==1054)return 1;return 0;}
static void history_end_queue(Duel *d){for(unsigned i=0;i<d->end_timer_count;++i){DuelEndTimer *t=&d->end_timers[i];if(t->used||t->turn!=d->turn||t->actor!=d->active)continue;unsigned name=d->cards[t->card].name_group;
    if(name==1054){if(t->offered_event==d->event_sequence+1||!history_will_available(d,t->actor))continue;t->offered_event=d->event_sequence+1;}
    else{if(!d->turn_end_pending||t->offered||!history_target_options(d,t->actor,name,0,0))continue;t->offered=1;}
    queue_trigger(d,DUEL_CHOICE_HISTORY_OFFER,t->actor,t->card,i+1);return;
}}
static void history_will_resolve(Duel *d,const DuelChainLink *link){unsigned index=link->source_serial-1;if(index>=d->end_timer_count)return;DuelEndTimer *t=&d->end_timers[index];if(pile_summon_candidates(d,t->actor,7,0,0)){t->used=1;pile_summon_begin(d,t->actor,link->card,7);}else{t->used=0;t->offered_event=d->event_sequence+1;}}

static void history_turn_cleanup(Duel *d){
    for(unsigned owner=0;owner<2;++owner){DuelPlayer *p=&d->players[owner];for(unsigned i=0;i<p->grave_count;++i)p->grave_history[i]&=3;for(unsigned i=0;i<p->banished_count;++i)p->banished_history[i]&=3;for(unsigned i=0;i<p->hand_count;++i)p->hand_state[i].history&=3;for(unsigned i=0;i<p->deck_count;++i)p->deck_state[i].history&=3;for(unsigned i=0;i<p->fusion_count;++i)p->fusion_state[i].history&=3;}
    d->field_to_grave[0]=d->field_to_grave[1]=0;d->end_timer_count=0;clear(d->end_timers,sizeof(d->end_timers));
}
static unsigned history_options(const Duel *d,DuelChoiceOption *options,unsigned capacity){
    unsigned actor=d->choice.player,name=d->cards[d->choice.card].name_group;
    if(d->choice.kind==DUEL_CHOICE_HISTORY_TARGET)return history_target_options(d,actor,name,options,capacity);
    unsigned timer=d->choice.serial-1;if(timer>=d->end_timer_count||d->end_timers[timer].used||(name==1054?!history_will_available(d,actor):!history_target_options(d,actor,name,0,0)))return 0;
    if(options&&capacity)options[0]=(DuelChoiceOption){d->choice.card,0,DUEL_OPTION_CONFIRM};
    if(options&&capacity>1)options[1]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return 2;
}
static void history_choose(Duel *d,unsigned index){
    unsigned actor=d->choice.player,card=d->choice.card,name=d->cards[card].name_group;DuelPlayer *p=&d->players[actor];
    if(d->choice.kind==DUEL_CHOICE_HISTORY_OFFER){unsigned timer=d->choice.serial-1;if(timer>=d->end_timer_count)return;d->end_timers[timer].used=name==1054?(index==DUEL_NO_SLOT?0:2):1;
        if(index!=DUEL_NO_SLOT){DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->card=card;link->command.player=actor;link->command.cost=name==1054?2:1;link->source_serial=timer+1;link->source_slot=DUEL_NO_SLOT;link->speed=1;link->effect_activation=1;d->trigger_chain_pending=1;}
        finish_choice(d);return;
    }
    if(!duel_history_can_return(d,actor,name,index))return;DuelCardState returned=card_state_reset(take_pile_card(p,PILE_GRAVE,index));unsigned target=returned.card;
    put_pile_card(d,actor,name==1444||name==1524||name==1421?PILE_HAND:PILE_DECK,returned,name!=1444&&name!=1524&&name!=1421&&name!=1430);

    event(d,DUEL_EVENT_RETURN,actor,index,target,0);if(name==1524&&d->choice.selected_count>1){--d->choice.selected_count;if(history_target_options(d,actor,name,0,0))return;}finish_choice(d);
}
