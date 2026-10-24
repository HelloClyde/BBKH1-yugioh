int duel_blast_can_target(const Duel *d,unsigned side,unsigned source,unsigned owner,unsigned index){
    if(!d||side>1||source>=5||owner>1||index>=5||(owner==side&&index==source))return 0;
    const DuelSlot *s=&d->players[owner].monsters[index];return s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&duel_can_target_monster(d,owner,index)&&duel_attack_value(d,owner,index)<=1000;
}
unsigned duel_blast_can_start(const Duel *d,unsigned side,unsigned source){
    if(!d||side>1||source>=5||d->active!=side||d->phase!=DUEL_STANDBY||d->chain_count||d->choice.kind||d->response_player<2||d->window.kind)return 0;
    const DuelSlot *s=&d->players[side].monsters[source];if(!s->card||d->cards[s->card].name_group!=416||s->position==DUEL_FACEDOWN_DEFENSE||!duel_can_tribute(d,side,side,source))return 0;
    unsigned n=0;for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i)n+=duel_blast_can_target(d,side,source,owner,i);return n>=2;
}
static int curse_standby_available(const Duel *d,unsigned side,unsigned index,unsigned from_set){
    if(!d||side>1||d->active!=side||d->phase!=DUEL_STANDBY||d->chain_count||d->choice.kind||d->response_player<2||d->window.kind||!(monsters(&d->players[0])+monsters(&d->players[1])))return 0;const DuelPlayer *p=&d->players[side];if(index>=(from_set?5:p->hand_count))return 0;const DuelSlot *s=from_set?&p->backrow[index]:0;unsigned card=from_set?s->card:p->hand[index];return card&&d->cards[card].name_group==1064&&duel_card_supported(&d->cards[card])&&(!from_set||s->position==DUEL_FACEDOWN_DEFENSE)&&!activation_blocked_for_slot(d,card,from_set?s->serial:0)&&fragrance_allows(d,side,index,from_set,card)&&(from_set||(empty_zone(p->backrow)>=0&&can_pay_action(d,side,hand_play_cost(d))));
}
unsigned duel_standby_has_action(const Duel *d,unsigned side){for(unsigned i=0;i<5;++i)if(duel_blast_can_start(d,side,i)||duel_patrol_ready(d,side,i)||curse_standby_available(d,side,i,1))return 1;if(d&&side<2)for(unsigned i=0;i<d->players[side].hand_count;++i)if(curse_standby_available(d,side,i,0))return 1;return 0;}
static int blast_activate(Duel *d,const DuelCommand *c){
    if(!duel_blast_can_start(d,c->player,c->source))return DUEL_NOT_ALLOWED;if(c->tributes||c->target_count!=2||c->targets[0]==c->targets[1])return DUEL_BAD_ARGUMENT;
    for(unsigned n=0;n<2;++n)if(!duel_blast_can_target(d,c->player,c->source,c->targets[n]/5,c->targets[n]%5))return DUEL_BAD_TARGET;
    DuelTarget targets[2];for(unsigned n=0;n<2;++n){unsigned token=c->targets[n],owner=token/5,index=token%5;if(!duel_blast_can_target(d,c->player,c->source,owner,index))return DUEL_BAD_TARGET;DuelSlot *s=&d->players[owner].monsters[index];if(!s->serial)s->serial=++d->next_serial;targets[n]=(DuelTarget){s->serial,owner,index};}
    DuelSlot *source=&d->players[c->player].monsters[c->source];unsigned card=source->card;uint32_t serial=source->serial;leave_field(d,c->player,c->source,0,LEAVE_TRIBUTE);
    DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command=*c;link->card=card;link->source_serial=serial;link->source_slot=c->source;link->speed=1;link->effect_activation=1;link->target_count=2;for(unsigned n=0;n<2;++n)link->targets[n]=targets[n];event(d,DUEL_EVENT_SPELL,c->player,c->source,card,0);begin_response(d,1-c->player);return DUEL_OK;
}
static void blast_resolve(Duel *d,const DuelChainLink *link){unsigned mask=0;for(unsigned n=0;n<link->target_count;++n){const DuelTarget *t=&link->targets[n];const DuelSlot *s=&d->players[t->player].monsters[t->slot];if(s->card&&s->serial==t->serial&&duel_blast_can_target(d,link->command.player,link->source_slot,t->player,t->slot))mask|=1u<<(t->player*5+t->slot);}destroy_monsters_mask(d,mask);}
static int blast_ai_command(const Duel *d,unsigned side,unsigned index,DuelCommand *c){
    if(!duel_blast_can_start(d,side,index))return 0;unsigned mask=0,enemy_count=0;for(unsigned n=0;n<2;++n){int best=-1,score=-2147483647;for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i){unsigned token=owner*5+i;if(mask&(1u<<token)||!duel_blast_can_target(d,side,index,owner,i))continue;int value=duel_attack_value(d,owner,i);value=owner==side?-10000-value:10000+value;if(value>score){best=token;score=value;}}if(best<0)return 0;mask|=1u<<best;c->targets[n]=best;enemy_count+=best/5!=side;}
    if(!enemy_count)return 0;c->action=DUEL_ACTION_ACTIVATE_MONSTER;c->source=index;c->target_count=2;return 1;
}
