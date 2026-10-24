#include "duel_swarm_data.h"
static const SwarmRule *swarm_rule(const Duel *d,unsigned card){for(unsigned i=0;i<DUEL_SWARM_RULE_COUNT;++i)if(swarm_rules[i].name==d->cards[card].name_group)return &swarm_rules[i];return 0;}
static unsigned swarm_limit(const Duel *d,unsigned card){const SwarmRule *r=swarm_rule(d,card);return r?r->limit:2;}
static unsigned swarm_position(const Duel *d,unsigned card){const SwarmRule *r=swarm_rule(d,card);return r?r->position:DUEL_FACEDOWN_DEFENSE;}
static int swarm_available(const Duel *d,unsigned side,unsigned card){const DuelPlayer *p=&d->players[side];if(!special_summon_allowed_for(d,side)||empty_zone(p->monsters)<0)return 0;for(unsigned i=0;i<p->deck_count;++i)if(d->cards[p->deck[i]].name_group==d->cards[card].name_group)return 1;return 0;}
static void swarm_resolve(Duel *d,const DuelChainLink *link){unsigned side=link->command.player;const SwarmRule *r=swarm_rule(d,link->card);if(!r)return;if(r->life<0)damage(d,1-side,-r->life);else if(r->life>0)recover(d,side,r->life,link->card);check_victory_boundary(d);if(d->winner<0&&swarm_available(d,side,link->card))d->choice=(DuelTrigger){DUEL_CHOICE_NIMBLE_SUMMON,side,link->card,0};}
