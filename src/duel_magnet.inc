/* Original attack rules distinguish effective associations from the
 * face-up-host count which first enables Ring's target restriction. */
static unsigned magnet_host(const Duel *d,unsigned side,unsigned index){
    const DuelSlot *host=&d->players[side].monsters[index];if(!host->card||!host->serial||tribute_spell_immune(d,side,index))return 0;
    for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[owner].backrow[i];if(s->card&&s->effect_active&&s->link_serial==host->serial&&d->cards[s->card].name_group==1058&&!equip_effect_negated(d,s->card))return 1;}return 0;
}
unsigned duel_magnet_target_mode(const Duel *d,unsigned side,unsigned source){
    if(!d||side>1||source>=5||!d->players[side].monsters[source].card||tribute_spell_immune(d,side,source))return 0;unsigned count=0;for(unsigned i=0;i<5;++i)count+=d->players[1-side].monsters[i].position!=DUEL_FACEDOWN_DEFENSE&&magnet_host(d,1-side,i);return count>1?2:count;
}
static unsigned magnet_forces_target(const Duel *d,unsigned side,unsigned source){return duel_magnet_target_mode(d,side,source)!=0;}
static int magnet_target_rule(const Duel *d,unsigned side,unsigned source,unsigned target){return magnet_forces_target(d,side,source)?(int)magnet_host(d,1-side,target):-1;}
static int magnet_replay_needed(const Duel *d,const DuelWindow *w){
    if(w->target>=5||magnet_host(d,duel_battle_target_player(w),w->target))return 0;
    /* This original replay continuation does not repeat the attacker's
     * immunity exemption, and includes facedown alternative hosts. */
    for(unsigned i=0;i<5;++i)if(i!=w->target&&magnet_host(d,duel_battle_target_player(w),i))return 1;return 0;
}
int duel_equip_can_target_for(const Duel *d,unsigned actor,unsigned card,unsigned side,unsigned index){if(d&&actor<2&&side<2&&index<5&&card_valid(d,card)&&d->cards[card].name_group==71){const DuelSlot *host=&d->players[side].monsters[index];for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[p].backrow[i];if(s->card&&d->cards[s->card].name_group==71&&s->link_serial==host->serial&&host->serial)return 0;}}return d&&actor<2&&card_valid(d,card)&&(d->cards[card].name_group!=1058||actor==side)&&(d->cards[card].name_group!=71||actor==side)&&(d->cards[card].name_group!=650||actor==side)&&(d->cards[card].name_group!=1514||actor!=side)&&duel_equip_can_target(d,card,side,index);}
