int duel_binding_can_target(const Duel *d,unsigned actor,unsigned card,unsigned owner,unsigned index){
    if(!d||actor>1||owner!=1-actor||index>=5||!card_valid(d,card)||d->cards[card].effect!=DUEL_E_BIND)return 0;
    const DuelSlot *s=&d->players[owner].monsters[index];return s->card&&duel_can_target_monster(d,owner,index)&&(d->cards[card].name_group==348||s->position!=DUEL_FACEDOWN_DEFENSE);
}
static int binding_live(const Duel *d,const DuelSlot *s,uint32_t serial){return s->card&&s->effect_active&&s->link_serial==serial&&d->cards[s->card].effect==DUEL_E_BIND&&!duel_field_effect_negated(d,s->card);}
int duel_binding_locked(const Duel *d,unsigned owner,unsigned index){
    if(!d||owner>1||index>=5)return 0;const DuelSlot *host=&d->players[owner].monsters[index];if(!host->card||!host->serial)return 0;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(binding_live(d,&d->players[side].backrow[i],host->serial))return 1;return 0;
}
static int binding_stat_bonus_since(const Duel *d,unsigned owner,unsigned index,int after){
    const DuelSlot *host=&d->players[owner].monsters[index];if(!host->card||!host->serial)return 0;int result=0;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].backrow[i];if((after<0||stat_source_after(d,s->serial,host->serial,2,after))&&binding_live(d,s,host->serial)&&d->cards[s->card].name_group==1244)result-=700;}return result;
}
static int binding_stat_bonus(const Duel *d,unsigned owner,unsigned index){return binding_stat_bonus_since(d,owner,index,-1);}
static void binding_resolve(Duel *d,const DuelChainLink *link){
    if(link->source_slot>=5||link->command.target_player>1||link->command.target>=5)return;
    DuelSlot *source=&d->players[link->command.player].backrow[link->source_slot],*host=&d->players[link->command.target_player].monsters[link->command.target];
    /* Original585200 checks the source record in its physical zone, not its
     * instance tag, and deliberately ignores flag4/current target position. */
    if(source->card!=link->card||!host->card||host->serial!=link->target_serial)return;source->effect_active=1;source->link_serial=host->serial;if(d->cards[source->card].name_group==1244)equipment_order_attach(d,source,host);
}
static void binding_detach(Duel *d,uint32_t serial,unsigned destroyed){
    if(!serial)return;for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card&&d->players[side].monsters[i].serial==serial)return;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].backrow[i];if(!s->card||d->cards[s->card].effect!=DUEL_E_BIND||s->link_serial!=serial)continue;
        unsigned remove=!duel_field_effect_negated(d,s->card)&&(d->cards[s->card].name_group==1244||destroyed);s->link_serial=0;if(remove)leave_field(d,side,i,1,LEAVE_DESTROY);
    }
}
static void binding_position_reset(Duel *d,uint32_t serial){for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].backrow[i];if(s->card&&s->link_serial==serial&&d->cards[s->card].name_group==1244&&d->cards[s->card].effect==DUEL_E_BIND)s->link_serial=0;}}
static int binding_ai_target(const Duel *d,unsigned side,unsigned card,DuelCommand *c){
    int best=-1,score=-1;for(unsigned i=0;i<5;++i)if(duel_binding_can_target(d,side,card,1-side,i)&&!duel_binding_locked(d,1-side,i)){const DuelSlot *s=&d->players[1-side].monsters[i];int value=s->position==DUEL_FACEDOWN_DEFENSE?1500:duel_attack_value(d,1-side,i);if(value>score){score=value;best=i;}}
    if(best<0)return 0;c->target_player=1-side;c->target=best;return 1;
}
