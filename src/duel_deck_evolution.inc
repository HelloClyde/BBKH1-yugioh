#include "duel_deck_evolution_data.h"
static const DeckEvolutionRule *deck_evolution_rule(const DuelCardDef *c){for(unsigned i=0;i<DUEL_DECK_EVOLUTION_COUNT;++i)if(deck_evolution_rules[i].source==c->name_group)return &deck_evolution_rules[i];return 0;}
static int deck_evolution_provenance(const DuelCardDef *c){for(unsigned i=0;i<DUEL_DECK_EVOLUTION_COUNT;++i)if(deck_evolution_rules[i].result==c->name_group)return 1;return 0;}
unsigned duel_monster_deck_special(const Duel *d,unsigned side,unsigned index){
    if(!d||side>1||index>=5)return 0;
    const DuelPlayer *p=&d->players[side];const DuelSlot *s=&p->monsters[index];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE)return 0;
    const DeckEvolutionRule *r=deck_evolution_rule(&d->cards[s->card]);if(!r)return 0;
    if(d->active!=side||!main_phase(d)||d->chain_count||d->choice.kind||d->response_player<2||d->window.kind||!special_summon_allowed_for(d,side)||continuous_count(d,5))return 0;
    unsigned attached=0;for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i){const DuelSlot *e=&d->players[owner].backrow[i];if(e->card&&e->effect_active&&e->position!=DUEL_FACEDOWN_DEFENSE&&e->link_serial==s->serial&&d->cards[e->card].name_group==r->equip&&equip_host(d,e->card,e->link_serial,0,0))attached=1;}
    if(!attached)return 0;for(unsigned i=0;i<p->deck_count;++i)if(d->cards[p->deck[i]].name_group==r->result&&duel_card_supported(&d->cards[p->deck[i]]))return p->deck[i];return 0;
}
static int activate_deck_evolution(Duel *d,const DuelCommand *c){unsigned result=duel_monster_deck_special(d,c->player,c->source);if(!result)return DUEL_NOT_ALLOWED;DuelSlot *s=&d->players[c->player].monsters[c->source];unsigned card=s->card;uint32_t serial=s->serial;if(c->tributes)return DUEL_BAD_ARGUMENT;leave_field(d,c->player,c->source,0,LEAVE_TRIBUTE);
    DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command=*c;link->card=card;link->source_serial=serial;link->source_slot=c->source;link->target_card=result;link->speed=1;link->effect_activation=1;event(d,DUEL_EVENT_SPELL,c->player,c->source,card,0);begin_response(d,1-c->player);return DUEL_OK;
}
/* Magical Labyrinth is the ability source. Both it and its linked host are
 * costs; a Soul Exchange host need not belong to the equipment controller. */
unsigned duel_equipment_deck_special(const Duel *d,unsigned side,unsigned index){
    if(!d||side>1||index>=5)return 0;const DuelPlayer *p=&d->players[side];const DuelSlot *s=&p->backrow[index];
    if(!s->card||d->cards[s->card].name_group!=651||!s->effect_active||s->position==DUEL_FACEDOWN_DEFENSE||equip_effect_negated(d,s->card)||duel_card_activation_blocked(d,s->card))return 0;
    if(d->active!=side||!main_phase(d)||d->chain_count||d->choice.kind||d->response_player<2||d->window.kind||!special_summon_allowed_for(d,side))return 0;
    unsigned owner,host;if(!equip_host(d,s->card,s->link_serial,&owner,&host)||!duel_can_tribute(d,side,owner,host)||(empty_zone(p->monsters)<0&&owner!=side))return 0;
    for(unsigned i=0;i<p->deck_count;++i)if(d->cards[p->deck[i]].name_group==368&&duel_card_supported(&d->cards[p->deck[i]]))return p->deck[i];return 0;
}
static int activate_equipment_deck_evolution(Duel *d,const DuelCommand *c){
    unsigned result=duel_equipment_deck_special(d,c->player,c->source);if(!result)return DUEL_NOT_ALLOWED;if(c->tributes)return DUEL_BAD_ARGUMENT;
    DuelSlot *s=&d->players[c->player].backrow[c->source];unsigned card=s->card,owner,host;uint32_t serial=s->serial;if(!equip_host(d,card,s->link_serial,&owner,&host))return DUEL_BAD_TARGET;
    leave_field(d,c->player,c->source,1,LEAVE_TRIBUTE);leave_field(d,owner,host,0,LEAVE_TRIBUTE);
    DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command=*c;link->card=card;link->source_serial=serial;link->source_slot=c->source;link->target_card=result;link->speed=1;link->effect_activation=1;event(d,DUEL_EVENT_SPELL,c->player,c->source,card,0);begin_response(d,1-c->player);return DUEL_OK;
}
