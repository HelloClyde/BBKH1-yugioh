/* The original attack producer queues every effective Box in field order,
 * then records event18. They share one chain with attack responses and resolve
 * in reverse order; a response can therefore remove more than one source. */
static void fairy_attack_begin(Duel *d){
    const DuelWindow *w=&d->window;unsigned actor=1-w->player;
    for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[actor].backrow[i];
        if(!s->card||d->cards[s->card].name_group!=1424||!continuous_live(d,actor,i)||d->chain_count>=DUEL_CHAIN_MAX)continue;
        DuelChainLink *l=&d->chain[d->chain_count++];clear(l,sizeof(*l));l->card=s->card;l->source_serial=s->serial;l->source_slot=i;l->command.player=actor;l->command.target_player=w->player;l->command.target=w->source;l->speed=l->effect_activation=1;l->target_count=1;l->targets[0]=(DuelTarget){0,w->player,w->source};
    }
    begin_response(d,d->chain_count?w->player:actor);
}
static void fairy_box_resolve(Duel *d,const DuelChainLink *l){
    unsigned actor=l->command.player;if(l->effect_negated||l->source_slot>=5)return;DuelSlot *s=&d->players[actor].backrow[l->source_slot];
    /* The original callback rechecks the card at the coordinate, not the old
     * physical identity or position. Replacement Box cards are intentional. */
    if(s->card!=l->card)return;if(!l->effect_activation)s->effect_active=1;
    if(!l->target_count||l->targets[0].player==actor)return;
    d->choice=(DuelTrigger){DUEL_CHOICE_FAIRY_BOX_COIN,actor,l->card,d->chain_count-1};
}
static void fairy_box_choose(Duel *d,unsigned guess){
    unsigned at=d->choice.serial;finish_choice(d);if(at>=d->chain_count)return;DuelChainLink *l=&d->chain[at];unsigned roll=random_next(d)&1,success=roll==guess;
    d->random_result=(DuelRandomResult){d->random_result.sequence+1,l->card,l->command.player,1,roll,guess,success};event(d,DUEL_EVENT_SPELL,l->command.player,l->source_slot,l->card,success);
    const DuelTarget *t=&l->targets[0];if(success&&t->player<2&&t->slot<5)duel_fairy_reset_attack(d,l->command.player,l->source_slot,t->player,t->slot);
}
