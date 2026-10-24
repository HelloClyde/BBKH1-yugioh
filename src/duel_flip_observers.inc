/* Paid Standby effects are scanned lazily: a preceding Jigen may destroy
 * another source before that source ever gets to pay its own tribute. */
static int jigen_ready(const Duel *d,unsigned side,unsigned index,uint32_t serial){
    if(side!=d->active||d->phase!=DUEL_STANDBY||index>=5)return 0;const DuelSlot *s=&d->players[side].monsters[index];
    return s->card&&d->cards[s->card].name_group==731&&s->serial==serial&&s->position!=DUEL_FACEDOWN_DEFENSE&&s->flip_armed&&!s->monster_negated&&!activation_blocked_for_slot(d,s->card,s->serial)&&!live_backrow_name(d,1530,3)&&duel_can_tribute(d,side,side,index);
}
static int jigen_standby_step(Duel *d){
    if(!d->jigen_standby_actor_plus1)return 0;if(!(d->field_name_cache&16384)){d->jigen_standby_actor_plus1=0;return 0;}unsigned side=d->jigen_standby_actor_plus1-1;
    while(d->jigen_standby_next<5){unsigned i=d->jigen_standby_next++;const DuelSlot *s=&d->players[side].monsters[i];if(!jigen_ready(d,side,i,s->serial))continue;unsigned before=d->trigger_count;queue_trigger(d,DUEL_TRIGGER_FLIP_AUTO,side,s->card,s->serial);if(d->trigger_count>before){d->triggers[before].selected_count=i;return 1;}}
    d->jigen_standby_actor_plus1=0;return 0;
}
/* Original opcode94 writes one shared ready bit on the current old
 * coordinate. These typed fields are its consumers in the native model. */
static void source_ready_used(Duel *d,DuelSlot *s){s->flip_armed=1;s->ignition_turn=d->turn;unsigned name=d->cards[s->card].name_group;if(name==1116||name==461||name==1254)s->control_used=1;if(name==1336&&!s->attacked)s->attacked=1;}
static void jigen_resolve(Duel *d,const DuelChainLink *link){
    unsigned side=link->command.player;if(side==d->active&&d->phase==DUEL_STANDBY){unsigned total=0;for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card)total+=duel_attack_value(d,side,i);destroy_monsters_mask(d,31u<<(side*5));damage(d,1-side,(total+1)/2);return;}
    if(link->source_slot>=5)return;DuelSlot *s=&d->players[side].monsters[link->source_slot];if(!s->card)return;source_ready_used(d,s);
}
static int acid_validate(const Duel *d,const DuelCommand *c){return c->target_player<2&&c->target<5&&d->players[c->target_player].monsters[c->target].card&&d->players[c->target_player].monsters[c->target].position==DUEL_FACEDOWN_DEFENSE&&!d->players[c->target_player].monsters[c->target].facedown_attack?DUEL_OK:DUEL_BAD_TARGET;}
static void acid_resolve(Duel *d,const DuelChainLink *link){
    unsigned side=link->command.target_player,index=link->command.target;if(side>1||index>=5)return;DuelSlot *s=&d->players[side].monsters[index];if(!s->card||s->serial!=link->target_serial||s->position!=DUEL_FACEDOWN_DEFENSE||s->facedown_attack)return;
    unsigned name=d->cards[s->card].name_group;duel_change_monster_position(d,side,index,DUEL_FACEUP_DEFENSE);
    /* The original invokes these two callbacks directly once at stage80.
     * Immortal heals; Spear only captures permissions and arms its grave
     * callback. It does not reach its ordinary initial Flip's next stage. */
    if(name==461||name==1116){s->control_used=1;if(name==461)recover(d,side,3000,s->card);}else flip_trigger(d,side,index);
    continuous_cleanup(d);control_reconcile(d);s=&d->players[side].monsters[index];if(!s->card)return;
    if(duel_defense_value(d,side,index)>2000)duel_change_monster_position(d,side,index,s->position==DUEL_FACEUP_ATTACK?DUEL_FACEDOWN_ATTACK:DUEL_FACEDOWN_DEFENSE);else effect_destroy_monster(d,link->card,side,index);
}
static int kotodama_visible(const Duel *d,unsigned side,unsigned index){const DuelSlot *s=&d->players[side].monsters[index];return s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&!(d->window.kind==DUEL_WINDOW_SUMMON_NEGATION&&d->window.player==side&&d->window.source==index&&d->window.serial==s->serial);}
static int kotodama_marked(const Duel *d,uint32_t target){
    if(!(d->stat_snapshot_present&32))return 0;for(unsigned i=0;i<d->stat_link_count;++i){const DuelStatLink *l=&d->stat_links[i];const DuelSlot *source;if(l->kind==11&&l->target==target&&(source=stat_link_source(d,l))&&d->cards[source->card].name_group==1124&&!source->monster_negated&&!duel_card_prohibited(d,source->card,source->serial))return 1;}return 0;
}
static void kotodama_observer(Duel *d){
    if(!(d->field_name_cache&8192)||d->damage_stage)return;unsigned actor=DUEL_NO_SLOT,slot=0;for(unsigned order=0;order<2&&actor==DUEL_NO_SLOT;++order){unsigned side=d->active^order;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];if(kotodama_visible(d,side,i)&&d->cards[s->card].name_group==1124&&!duel_card_prohibited(d,s->card,s->serial)){actor=side;slot=i;break;}}}if(actor==DUEL_NO_SLOT)return;
    DuelSlot *source=&d->players[actor].monsters[slot];if(!source->serial)source->serial=++d->next_serial;stat_links_cleanup(d);unsigned destroyed=0;
    for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[p].monsters[i];if(!kotodama_visible(d,p,i)||kotodama_marked(d,s->serial))continue;unsigned duplicate=0;for(unsigned q=0;q<2;++q)for(unsigned j=0;j<5;++j)if((q!=p||j!=i)&&kotodama_visible(d,q,j)&&same_name(d,s->card,d->players[q].monsters[j].card))duplicate=1;
        if(duplicate)destroyed|=1u<<(p*5+i);else {if(!s->serial)s->serial=++d->next_serial;if(d->stat_link_count<DUEL_STAT_LINK_MAX){d->stat_links[d->stat_link_count++]=(DuelStatLink){source->serial,s->serial,11};d->stat_snapshot_present|=32;}}}
    if(destroyed){unsigned saved=d->effect_actor_plus1;d->effect_actor_plus1=actor+1;destroy_monsters_mask(d,destroyed);d->effect_actor_plus1=saved;}
}
