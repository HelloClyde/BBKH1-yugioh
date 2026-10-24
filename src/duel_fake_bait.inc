/* Original trap substitution and Bait's forced use share the ordinary
 * activation validator, costs, physical identities and chain lifecycle. */
static int fake_mass_name(unsigned name){return name==1061||name==1062||name==671||name==1067;}
static int fake_single_name(unsigned name){return name==83||name==223||name==1004||name==1079||name==1134||name==1135||name==1545;}
static const DuelSlot *fake_target_slot(const Duel *d,const DuelChainLink *l,unsigned ordinal,unsigned *side,unsigned *index){
    unsigned name=d->cards[l->card].name_group;if(name==585||name==83||name==223){if(ordinal>=l->target_count)return 0;const DuelTarget *t=&l->targets[ordinal];*side=t->player;*index=t->slot;return t->player<2&&t->slot<11?target_flip_slot((Duel *)d,t->player,t->slot):0;}
    if(ordinal)return 0;*side=l->command.target_player;*index=l->command.target;return *side<2&&*index<=DUEL_FIELD_SLOT?spell_zone((DuelPlayer *)&d->players[*side],*index):0;
}
static int fake_trap_ready(const Duel *d,const DuelCommand *c){
    if(c->action!=DUEL_ACTION_ACTIVATE_BACKROW||!d->chain_count)return 0;const DuelChainLink *root=&d->chain[d->chain_count-1];unsigned name=d->cards[root->card].name_group;
    if((root->command.player^(root->player_redirect&1))==c->player)return 0;
    if(fake_mass_name(name)){for(unsigned i=0;i<6;++i){const DuelSlot *s=spell_zone((DuelPlayer *)&d->players[c->player],i);if(s->card&&i!=c->source&&d->cards[s->card].family==DUEL_TRAP)return 1;}return 0;}
    if(name!=585&&!fake_single_name(name))return 0;
    for(unsigned n=0;n<(name==585?2u:1u);++n){unsigned side,index;const DuelSlot *s=fake_target_slot(d,root,n,&side,&index);if(s&&s->card&&s!=spell_zone((DuelPlayer *)&d->players[c->player],c->source)&&d->cards[s->card].family==DUEL_TRAP)return 1;}return 0;
}
static void fake_trap_resolve(Duel *d,const DuelChainLink *l){
    unsigned actor=l->command.player,index=l->source_slot,at=l->command.target;if(index>DUEL_FIELD_SLOT||at>=d->chain_count-1)return;const DuelSlot *source=spell_zone(&d->players[actor],index);
    if(source->card!=l->card)return;DuelChainLink *root=&d->chain[at];if(root->command.player==actor)return;unsigned name=d->cards[root->card].name_group;
    if(fake_mass_name(name)||name==585){root->trap_protected=1;return;}
    if(!fake_single_name(name))return;unsigned side,target;const DuelSlot *s=fake_target_slot(d,root,0,&side,&target);
    /*586b30 ignores583470's validity result and follows the current coordinate. */
    if(!s||!s->card||s==source)return;if(d->cards[s->card].family==DUEL_TRAP)destroy(d,actor,index,1);root->trap_protected=1;
}
static int fake_trap_shields(const Duel *d,unsigned side,unsigned index,unsigned backrow){
    if(!backrow||!d->resolving||!d->chain_count)return 0;const DuelChainLink *l=&d->chain[d->chain_count-1];if(!l->trap_protected||side!=(1u-(l->command.player^(l->player_redirect&1))))return 0;const DuelSlot *s=spell_zone((DuelPlayer *)&d->players[side],index);if(!s->card||d->cards[s->card].family!=DUEL_TRAP)return 0;
    /* An activation-negated Fake Trap is the sole original exception. */
    if(d->cards[s->card].name_group==689)for(unsigned i=0;i<d->chain_count;++i)if(d->chain[i].source_serial==s->serial&&d->chain[i].negated)return 0;return 1;
}
static unsigned fake_trap_mask(const Duel *d,unsigned mask){for(unsigned side=0;side<2;++side)for(unsigned i=0;i<6;++i)if((mask&(1u<<(side*6+i)))&&fake_trap_shields(d,side,i,1))mask&=~(1u<<(side*6+i));return mask;}
static int bait_validate(const Duel *d,const DuelCommand *c){if(c->target_player>1||c->target>=5||(c->action==DUEL_ACTION_ACTIVATE_BACKROW&&c->target_player==c->player&&c->target==c->source))return DUEL_BAD_TARGET;const DuelSlot *s=&d->players[c->target_player].backrow[c->target];return s->card&&d->cards[s->card].family!=DUEL_MONSTER&&s->position==DUEL_FACEDOWN_DEFENSE?DUEL_OK:DUEL_BAD_TARGET;}
static int bait_probe(Duel *d,const DuelCommand *c){unsigned chain=d->chain_count,response=d->response_player,mode=d->bait_mode;d->chain_count=0;d->response_player=c->player;d->bait_mode=1;int result=activate(d,c,1);d->bait_mode=mode;d->response_player=response;d->chain_count=chain;return result==DUEL_OK;}
static unsigned bait_candidates(Duel *d,unsigned side,unsigned index,DuelChoiceOption *options,unsigned capacity,unsigned wanted,DuelCommand *selected){
    const DuelSlot *s=&d->players[side].backrow[index];if(!s->card||s->position!=DUEL_FACEDOWN_DEFENSE)return 0;const DuelCardDef *def=&d->cards[s->card];DuelCommand c={side,DUEL_ACTION_ACTIVATE_BACKROW,index};unsigned n=0,kind=def->name_group==650?0:def->effect==DUEL_E_BACKUP?4:def->effect==DUEL_E_CALL_HAUNTED?3:def->effect==DUEL_E_BIND||def->effect==DUEL_E_EQUIP?1:duel_simple_target_kind(def);
    if(kind==4){DuelChoiceOption list[DUEL_PILE_MAX];unsigned total=duel_backup_options(d,side,list,DUEL_PILE_MAX);for(unsigned i=0;i<total;++i){c.target_count=1;c.targets[0]=list[i].index;if(!bait_probe(d,&c))continue;if(options&&n<capacity)options[n]=(DuelChoiceOption){list[i].card,n,DUEL_OPTION_CARD};if(selected&&n==wanted)*selected=c;++n;}return n;}
    for(unsigned owner=0;owner<(kind?2u:1u);++owner)for(unsigned target=0;target<(kind==3?d->players[owner].grave_count:kind==2?6u:kind==1?5u:1u);++target){c.target_player=owner;c.target=target;c.position=DUEL_FACEUP_ATTACK;if(!bait_probe(d,&c))continue;unsigned card=kind==3?d->players[owner].grave[target]:kind==2?spell_zone(&d->players[owner],target)->card:kind==1?d->players[owner].monsters[target].card:s->card;if(options&&n<capacity)options[n]=(DuelChoiceOption){card,n,kind?DUEL_OPTION_CARD:DUEL_OPTION_CONFIRM};if(selected&&n==wanted)*selected=c;++n;}
    return n;
}
unsigned duel_forced_trap_options(Duel *d,unsigned side,unsigned index,DuelChoiceOption *options,unsigned capacity){return !d||side>1||index>=5?0:bait_candidates(d,side,index,options,capacity,0,0);}
int duel_trap_substitution_protects(const Duel *d,unsigned side,unsigned index,unsigned backrow){return d&&side<2&&index<=DUEL_FIELD_SLOT?fake_trap_shields(d,side,index,backrow):0;}
static void bait_return_source(Duel *d,const DuelChainLink *l){unsigned side=l->command.player,index=l->source_slot;if(index>DUEL_FIELD_SLOT)return;DuelSlot *s=spell_zone(&d->players[side],index);if(!s->card)return;unsigned owner=s->owner,card=s->card;if(d->players[owner].deck_count>=DUEL_DECK_MAX)return;DuelSlot old=*s;unsigned destiny=duel_destiny_departure_mask(d,side,index,card),was_live=source_departure_live(d,card);clear(s,sizeof(*s));put_pile_card(d,owner,PILE_DECK,card_state_reset(slot_card_state(&old)),0);linked_source_left(d,&old,was_live,LEAVE_RETURN,side);destiny_clear_departures(d,side,destiny);shuffle(d,&d->players[owner]);event(d,DUEL_EVENT_RETURN,side,index,card,0);}
static void bait_resolve(Duel *d,const DuelChainLink *l){unsigned side=l->command.target_player,index=l->command.target;if(side>1||index>=5)return;DuelSlot *s=&d->players[side].backrow[index];if(!s->card||s->serial!=l->target_serial)return;
    if(s->position==DUEL_FACEDOWN_DEFENSE){if(d->cards[s->card].family==DUEL_TRAP&&!l->trap_protected){if(bait_candidates(d,side,index,0,0,0,0)){if(d->bait_pending<DUEL_CHAIN_MAX)d->bait_queue[d->bait_pending++]=(DuelTarget){s->serial,side,index};}else destroy(d,side,index,1);}/* Spell and protected Trap are displayed, then hidden again. */}
    bait_return_source(d,l);
}
static unsigned bait_options(Duel *d,DuelChoiceOption *options,unsigned capacity){if(d->choice.kind!=DUEL_CHOICE_BAIT_BACKUP)return bait_candidates(d,d->bait_source.player,d->bait_source.slot,options,capacity,0,0);DuelChoiceOption list[DUEL_PILE_MAX];unsigned total=duel_backup_options(d,d->choice.player,list,DUEL_PILE_MAX),n=0;for(unsigned i=0;i<total;++i){unsigned used=0;for(unsigned j=0;j<d->bait_command.target_count;++j)used|=d->bait_command.targets[j]==list[i].index;if(used)continue;DuelCommand c=d->bait_command;c.targets[c.target_count++]=list[i].index;if(!bait_probe(d,&c))continue;if(options&&n<capacity)options[n]=list[i];++n;}if(options&&n<capacity)options[n]=(DuelChoiceOption){d->choice.card,DUEL_NO_SLOT,DUEL_OPTION_CONFIRM};return n+1;}
static int bait_launch(Duel *d,const DuelCommand *c){d->bait_mode=2;d->response_player=c->player;int result=activate(d,c,1);if(result!=DUEL_OK){d->bait_mode=0;d->response_player=DUEL_NO_SLOT;}return result;}
static int bait_choose(Duel *d,unsigned token){if(d->choice.kind==DUEL_CHOICE_BAIT_BACKUP){if(token!=DUEL_NO_SLOT)d->bait_command.targets[d->bait_command.target_count++]=token;if(token!=DUEL_NO_SLOT&&d->bait_command.target_count<3)return DUEL_OK;DuelCommand c=d->bait_command;finish_choice(d);return bait_launch(d,&c);}DuelCommand c;unsigned count=bait_candidates(d,d->bait_source.player,d->bait_source.slot,0,0,token,&c);if(token>=count)return DUEL_BAD_TARGET;if(d->cards[d->choice.card].effect==DUEL_E_BACKUP){d->bait_command=c;d->choice.kind=DUEL_CHOICE_BAIT_BACKUP;return DUEL_OK;}finish_choice(d);return bait_launch(d,&c);}
static int bait_advance(Duel *d){if(d->winner>=0||d->chain_count||d->resolving||d->choice.kind||d->response_player<2)return 0;while(d->bait_pending){d->bait_source=d->bait_queue[0];for(unsigned i=1;i<d->bait_pending;++i)d->bait_queue[i-1]=d->bait_queue[i];--d->bait_pending;unsigned side=d->bait_source.player,index=d->bait_source.slot;if(side>1||index>=5||d->players[side].backrow[index].serial!=d->bait_source.serial)continue;unsigned count=bait_candidates(d,side,index,0,0,0,0);if(!count)continue;d->choice=(DuelTrigger){DUEL_CHOICE_BAIT_TARGET,side,d->players[side].backrow[index].card,d->bait_source.serial};if(count==1)bait_choose(d,0);return 1;}return 0;}
