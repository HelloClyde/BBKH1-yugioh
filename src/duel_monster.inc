static int toon_card(const DuelCardDef *c){return c->name_group==726||c->name_group==727||c->name_group==728||c->name_group==766;}
static int toon_world_present(const Duel *d,unsigned side){const DuelPlayer *p=&d->players[side];for(unsigned i=0;i<6;++i){const DuelSlot *s=i==5?&p->field:&p->backrow[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==954)return 1;}return 0;}
static int toon_visible(const Duel *d,unsigned side){for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&toon_card(&d->cards[s->card]))return 1;}return 0;}
int duel_can_attack_target(const Duel *d,unsigned side,unsigned source,unsigned target){if(!d||side>1||source>=5||target>=5)return 0;const DuelSlot *a=&d->players[side].monsters[source],*b=&d->players[1-side].monsters[target];if(!a->card||!b->card)return 0;if(d->cards[b->card].name_group==1326&&duel_spell_immune(d,1-side,target))return 0;int forced=magnet_target_rule(d,side,source,target);if(forced>=0)return forced;return !toon_card(&d->cards[a->card])||!toon_visible(d,1-side)||(b->position!=DUEL_FACEDOWN_DEFENSE&&toon_card(&d->cards[b->card]));}
/* Passive monster arithmetic uses the same native field/grave instances. */
#include "duel_monster_data.h"
enum {MONSTER_DIRECT=1,MONSTER_DOUBLE=2,MONSTER_PIERCE=4,MONSTER_PROTECT_DRAGON=8,MONSTER_DEFENSE_ATTACK=256};
static unsigned monster_abilities_for(const DuelCardDef *c){if(c->effect!=DUEL_E_MONSTER)return 0;for(unsigned i=0;i<DUEL_MONSTER_ABILITY_COUNT;++i)if(monster_abilities[i].name==c->name_group)return monster_abilities[i].flags;return 0;}
int duel_can_target_monster(const Duel *d,unsigned side,unsigned index){if(!d||side>1||index>=5)return 0;const DuelSlot *target=&d->players[side].monsters[index];if(!target->card)return 0;if(target->position==DUEL_FACEDOWN_DEFENSE||duel_race(d,side,index)!=1)return 1;for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[owner].monsters[i];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE||battle_destroyed(d,owner,i)||!(monster_abilities_for(&d->cards[s->card])&MONSTER_PROTECT_DRAGON))continue;if(d->window.kind==DUEL_WINDOW_SUMMON_NEGATION&&d->window.player==owner&&d->window.source==i&&d->window.serial==s->serial)continue;return 0;}return 1;}
unsigned duel_attack_limit(const Duel *d,unsigned side,unsigned index){if(!d||side>1||index>=5||!d->players[side].monsters[index].card)return 0;return monster_abilities_for(&d->cards[d->players[side].monsters[index].card])&MONSTER_DOUBLE?2:1;}
static int monster_stat_bonus(const Duel *d,unsigned side,unsigned index,unsigned defense){
    const DuelSlot *host=&d->players[side].monsters[index];const DuelCardDef *c=&d->cards[host->card];if(!host->card||host->position==DUEL_FACEDOWN_DEFENSE||c->effect!=DUEL_E_MONSTER)return 0;
    if(c->name_group==98)return duel_card_prohibited(d,host->card,host->serial)?0:100*((host->hourglass_turn<4?host->hourglass_turn:4)+(host->control_used!=0));
    if(!defense&&d->phase==DUEL_BATTLE&&((c->name_group==1516&&side==d->active)||(c->name_group==1518&&side!=d->active)))return duel_card_prohibited(d,host->card,host->serial)?0:300;
    const MonsterStatRule *r=0;for(unsigned i=0;i<DUEL_MONSTER_STAT_COUNT;++i)if(monster_stat_rules[i].name==c->name_group){r=&monster_stat_rules[i];break;}if(!r||duel_card_prohibited(d,host->card,host->serial))return 0;
    unsigned count=0;const DuelPlayer *p=&d->players[side];
    if(defense&&r->op!=3&&r->op!=6)return 0;
    switch(r->op){
    case 1:for(unsigned i=0;i<p->grave_count;++i)count+=d->cards[p->grave[i]].family==DUEL_MONSTER;break;
    case 2:case 6:case 7:
        for(unsigned owner=0;owner<2;++owner)if(r->op!=7||owner==side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[owner].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&(r->op==2?duel_race(d,owner,i)==r->selector:d->cards[s->card].name_group==r->selector))++count;}break;
    case 8:count=monsters(&d->players[1-side])!=0;break;
    case 9:count=monsters(&d->players[1-side]);break;
    case 3:count=p->hand_count;break;
    case 4:for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[owner].backrow[i];if(host->serial&&s->card&&s->link_serial==host->serial&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].family!=DUEL_TRAP)++count;}break;
    case 5:for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<d->players[owner].grave_count;++i){unsigned name=d->cards[d->players[owner].grave[i]].name_group;if(name==34||name==721)++count;}break;
    }
    return (int)count*r->amount;
}

static int battle_aura_bonus(const Duel *d,unsigned side,unsigned index){if(d->phase!=DUEL_BATTLE||side!=d->active||!d->players[side].monsters[index].card||d->players[side].monsters[index].position==DUEL_FACEDOWN_DEFENSE)return 0;int value=0;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[1-side].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==1515&&!battle_destroyed(d,1-side,i))value-=300;}return value;}

static int combat_attack_position(const Duel *d,const DuelSlot *s){return s->position==DUEL_FACEUP_ATTACK||(s->position==DUEL_FACEUP_DEFENSE&&(monster_abilities_for(&d->cards[s->card])&MONSTER_DEFENSE_ATTACK));}

unsigned duel_battle_protection(const Duel *d,unsigned side,unsigned index,unsigned opposing_attack){if(!d||side>1||index>=5)return 0;const DuelSlot *s=&d->players[side].monsters[index];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE)return 0;unsigned name=d->cards[s->card].name_group;return name==1251&&opposing_attack>=1900?1:name==1231&&d->active==side&&d->phase==DUEL_BATTLE?3:0;}
