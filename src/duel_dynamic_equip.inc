#include "duel_dynamic_equip_data.h"
static unsigned dynamic_equip_op(const Duel *d,unsigned card){for(unsigned i=0;i<DUEL_DYNAMIC_EQUIP_COUNT;++i)if(dynamic_equip_rules[i].name==d->cards[card].name_group)return dynamic_equip_rules[i].op;return 0;}
static unsigned dynamic_equip_count(const Duel *d,unsigned side,unsigned op){unsigned count=0;const DuelPlayer *p=&d->players[side];if(op==2)count=p->field.card!=0;for(unsigned i=0;i<5;++i)count+=op==1?(p->monsters[i].card&&p->monsters[i].position!=DUEL_FACEDOWN_DEFENSE):p->backrow[i].card!=0;return count;}
/* Base-stat assignments follow attachment order. Ordinary additive Equip
 * components stay separate, so Cocoon does not erase another Equip's bonus.
 * The original Cocoon base assignment deliberately ignores Equip negation. */
static int goddess_base(const Duel *,unsigned,unsigned,unsigned);
static int equipment_original_stat_range(const Duel *d,unsigned side,unsigned index,unsigned defense,int after,unsigned before){
    const DuelSlot *host=&d->players[side].monsters[index];int original=defense?d->cards[host->card].defense:d->cards[host->card].attack;
    int result=after>=0?(d->stat_links[after].kind==14?goddess_base(d,side,index,after):d->stat_links[after].base_value):!defense&&host->original_attack_active?host->original_attack:original;
    if(after<0&&d->cards[host->card].name_group==615&&host->control_used&&host->position!=DUEL_FACEDOWN_DEFENSE&&(defense||!host->original_attack_active))result=host->control_used==2?original*2:(original+1)/2;
    if(!host->serial||host->position==DUEL_FACEDOWN_DEFENSE||(defense&&d->cards[host->card].name_group!=277&&d->cards[host->card].name_group!=730&&d->cards[host->card].name_group!=1334&&!(d->stat_snapshot_present&4)))return result;
    for(unsigned n=0;n<(before<d->stat_link_count?before:d->stat_link_count+10);++n){const DuelSlot *s=0;
        if(n<d->stat_link_count){const DuelStatLink *l=&d->stat_links[n];if((int)n<=after||l->target!=host->serial)continue;if((l->kind==14||l->kind==15)&&l->expires>=d->turn){if(!defense)result=l->kind==14?goddess_base(d,side,index,n):original*2;continue;}if(l->kind>=6&&l->kind<=8&&(!l->expires||l->expires>=d->turn)){if(l->kind==6&&!defense)result=l->base_value;else if(l->kind==7&&!defense)result+=l->base_value;else if(l->kind==8){unsigned record=l->base_value&4095;result=record<DUEL_CARD_COUNT?(defense?d->cards[record].defense:d->cards[record].attack):0;}continue;}if(l->kind!=2)continue;s=stat_link_source(d,l);}
        else {if(after>=0)continue;unsigned field=n-d->stat_link_count;s=&d->players[field/5].backrow[field%5];unsigned ordered=0;for(unsigned j=0;j<d->stat_link_count;++j)ordered|=d->stat_links[j].kind==2&&d->stat_links[j].source==s->serial&&d->stat_links[j].target==host->serial;if(ordered)continue;}
        if(!s||!s->card||!s->effect_active||(s->position==DUEL_FACEDOWN_DEFENSE&&s->equip_role!=DUEL_EQUIP_ROLE_ABSORB)||s->link_serial!=host->serial)continue;
        if(s->equip_role==DUEL_EQUIP_ROLE_ABSORB&&(d->cards[host->card].name_group==730||d->cards[host->card].name_group==1334)){result=s->position==DUEL_FACEDOWN_DEFENSE?0:defense?d->cards[s->card].defense:d->cards[s->card].attack;continue;}
        if(d->cards[host->card].name_group==277&&d->cards[s->card].name_group==71){result=defense?2000:0;continue;}
        if(defense||dynamic_equip_op(d,s->card)!=3||!equip_eligible(d,s->card,side,index)||equip_effect_negated(d,s->card)||!effect_affects_monster(d,s->card,side,index))continue;
        unsigned owner=s->owner;for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i)if(s==&d->players[p].backrow[i])owner=p;
        int life=d->players[owner].life,enemy=d->players[1-owner].life;if(life<enemy)result=original*2;else if(life>enemy)result=(original+1)/2;
    }return result;
}
static int equipment_original_stat_since(const Duel *d,unsigned side,unsigned index,unsigned defense,int after){return equipment_original_stat_range(d,side,index,defense,after,d->stat_link_count);}
static int equipment_original_stat(const Duel *d,unsigned side,unsigned index,unsigned defense){return equipment_original_stat_since(d,side,index,defense,-1);}
static int equipment_original_attack(const Duel *d,unsigned side,unsigned index){return equipment_original_stat(d,side,index,0);}
