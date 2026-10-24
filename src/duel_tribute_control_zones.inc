/* Original paid response, two-stage controller transfer and empty-zone nodes.
 * Zone selections are coordinates, not physical card targets. Ground nodes
 * live with the current same-name source deployment and ignore Prohibition. */
static int mary_ready(const Duel *d,unsigned side,unsigned index){
    if(side>1||index>=5||d->active==side||d->response_player!=side||!d->chain_count||d->chain_count>=DUEL_CHAIN_MAX)return 0;
    const DuelSlot *s=&d->players[side].monsters[index];const DuelChainLink *root=&d->chain[d->chain_count-1];
    return s->card&&d->cards[s->card].name_group==1525&&s->position!=DUEL_FACEDOWN_DEFENSE&&!s->monster_negated&&!activation_blocked_for_slot(d,s->card,s->serial)&&duel_can_tribute(d,side,side,index)&&root->speed<=2&&!root->effect_activation&&root->source_slot<=DUEL_FIELD_SLOT&&root->command.player!=side&&d->cards[root->card].family==DUEL_SPELL&&!activation_unnegatable(d,root->card);
}
static int mary_activate(Duel *d,const DuelCommand *c){
    if(!mary_ready(d,c->player,c->source))return DUEL_NOT_ALLOWED;
    DuelSlot old=d->players[c->player].monsters[c->source];unsigned root=d->chain_count-1;
    leave_field(d,c->player,c->source,0,LEAVE_TRIBUTE);
    DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command=*c;link->command.target=root;link->card=old.card;link->source_slot=c->source;link->source_serial=old.serial;link->speed=2;link->effect_activation=1;
    event(d,DUEL_EVENT_SPELL,c->player,c->source,old.card,0);begin_response(d,1-c->player);return DUEL_OK;
}
static unsigned ground_free_count(const Duel *d){unsigned n=0;for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i)n+=!d->players[p].monsters[i].card&&!d->players[p].monsters[i].zone_blocked;return n;}
static void ground_reconcile(Duel *d){
    unsigned mask=0;for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[p].backrow[i];if(s->card&&d->cards[s->card].name_group==1320&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE&&!duel_field_effect_negated(d,s->card))mask|=s->original_until;}
    for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[p].monsters[i];s->zone_blocked=(s->zone_blocked&2)|((mask>>(p*5+i))&1);}
}
static int coordinate_choice(unsigned kind){return kind==DUEL_CHOICE_MYSTIC_DESTROY||kind==DUEL_CHOICE_MYSTIC_GIVE||kind==DUEL_CHOICE_GROUND_FIRST||kind==DUEL_CHOICE_GROUND_SECOND;}
static int mystic_target(const Duel *d,unsigned actor,unsigned token,unsigned own){unsigned side=token/5,index=token%5;return token<10&&side==(own?actor:1-actor)&&d->players[side].monsters[index].card&&duel_can_target_monster(d,side,index);}
static int mystic_validate(const Duel *d,const DuelCommand *c){
    if(!c->cost){unsigned own=0,enemy=0;for(unsigned i=0;i<5;++i){own|=mystic_target(d,c->player,c->player*5+i,1);enemy|=mystic_target(d,c->player,(1-c->player)*5+i,0);}return own&&enemy?DUEL_OK:DUEL_NOT_ALLOWED;}
    return c->cost==1&&c->target_count==2&&mystic_target(d,c->player,c->targets[0],0)&&mystic_target(d,c->player,c->targets[1],1)?DUEL_OK:DUEL_BAD_TARGET;
}
static int ground_validate(const Duel *d,const DuelCommand *c){
    if(!c->cost)return ground_free_count(d)>=2?DUEL_OK:DUEL_NOT_ALLOWED;
    if(c->cost!=1||c->target_count!=2||c->targets[0]==c->targets[1])return DUEL_BAD_TARGET;
    for(unsigned i=0;i<2;++i){unsigned t=c->targets[i];if(t>=10||d->players[t/5].monsters[t%5].card||d->players[t/5].monsters[t%5].zone_blocked)return DUEL_BAD_TARGET;}return DUEL_OK;
}
static void coordinate_begin(Duel *d,const DuelCommand *c,unsigned card){d->equip_mode_command=*c;d->equip_mode_command.target_count=0;d->choice=(DuelTrigger){d->cards[card].name_group==1213?DUEL_CHOICE_MYSTIC_DESTROY:DUEL_CHOICE_GROUND_FIRST,c->player,card,0};}
static unsigned coordinate_options(const Duel *d,DuelChoiceOption *options,unsigned capacity){
    unsigned n=0,kind=d->choice.kind,ground=kind==DUEL_CHOICE_GROUND_FIRST||kind==DUEL_CHOICE_GROUND_SECOND;
    for(unsigned t=0;t<10;++t){const DuelSlot *s=&d->players[t/5].monsters[t%5];unsigned valid=ground?(!s->card&&!s->zone_blocked&&!(kind==DUEL_CHOICE_GROUND_SECOND&&t==d->equip_mode_command.targets[0])):mystic_target(d,d->choice.player,t,kind==DUEL_CHOICE_MYSTIC_GIVE);
        if(valid){if(options&&n<capacity)options[n]=(DuelChoiceOption){s->card,t,ground?DUEL_OPTION_CONFIRM:DUEL_OPTION_CARD};++n;}}
    if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return n+1;
}
static int coordinate_choose(Duel *d,unsigned token){
    if(token==DUEL_NO_SLOT){finish_choice(d);clear(&d->equip_mode_command,sizeof(d->equip_mode_command));return DUEL_OK;}
    DuelCommand *c=&d->equip_mode_command;c->targets[c->target_count++]=token;
    if(c->target_count==1){d->choice.kind=d->choice.kind==DUEL_CHOICE_MYSTIC_DESTROY?DUEL_CHOICE_MYSTIC_GIVE:DUEL_CHOICE_GROUND_SECOND;return DUEL_OK;}
    DuelCommand selected=*c;selected.cost=1;finish_choice(d);clear(c,sizeof(*c));return activate(d,&selected,selected.action==DUEL_ACTION_ACTIVATE_BACKROW);
}
static void mystic_resolve(Duel *d,const DuelChainLink *link){
    if(link->effect_negated||link->target_count!=2)return;
    for(unsigned i=0;i<2;++i){const DuelTarget *t=&link->targets[i];if(t->player>1||t->slot>=5||!d->players[t->player].monsters[t->slot].card||d->players[t->player].monsters[t->slot].serial!=t->serial)return;}
    const DuelTarget *enemy=&link->targets[0],*own=&link->targets[1];effect_destroy_monster(d,link->card,enemy->player,enemy->slot);continuous_cleanup(d);control_reconcile(d);
    DuelSlot *s=&d->players[own->player].monsters[own->slot];if(!s->card||!control_add(d,own->player,own->slot,enemy->player,link->card,0,0))return;
    if(!duel_spell_immune(d,own->player,own->slot)){const DuelSlot *old=&d->players[enemy->player].monsters[enemy->slot];int to=!old->card&&!old->zone_blocked?(int)enemy->slot:empty_zone(d->players[enemy->player].monsters);if(to>=0)change_controller(d,own->player,own->slot,enemy->player,to,own->player);}
}
static void ground_resolve(Duel *d,const DuelChainLink *link){
    unsigned side=link->command.player;if(link->source_slot>=5)return;DuelSlot *s=&d->players[side].backrow[link->source_slot];if(s->card!=link->card)return;
    if(s->position!=DUEL_FACEDOWN_DEFENSE)s->effect_active=1;
    if(link->effect_negated||duel_field_effect_negated(d,link->card)||link->command.target_count!=2)return;
    for(unsigned i=0;i<2;++i){unsigned t=link->command.targets[i];if(t<10&&!d->players[t/5].monsters[t%5].card)s->original_until|=1u<<t;}
}
