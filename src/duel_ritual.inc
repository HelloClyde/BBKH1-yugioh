/* Ritual procedures consume actual hand/field instances during resolution. */
#include "duel_ritual_data.h"
static const RitualRule *ritual_rule(unsigned spell){for(unsigned i=0;i<DUEL_RITUAL_RULE_COUNT;++i)if(ritual_rules[i].spell==spell)return &ritual_rules[i];return 0;}
static unsigned ritual_material_card(const Duel *d,unsigned side,unsigned token){const DuelPlayer *p=&d->players[side];if(token<DUEL_HAND_MAX)return token<p->hand_count?p->hand[token]:0;unsigned field=token-DUEL_HAND_MAX;return field<10&&duel_can_tribute(d,side,tribute_owner(side,field),field%5)?tribute_monster(d,side,field)->card:0;}
static int ritual_possible(const Duel *d,unsigned side,unsigned monster_index,const RitualRule *r){
    const DuelPlayer *p=&d->players[side];if(!special_summon_allowed_for(d,side)||!r||monster_index>=p->hand_count||continuous_count(d,5))return 0;unsigned card=p->hand[monster_index];if(d->cards[card].name_group!=r->monster||!duel_card_supported(&d->cards[card]))return 0;
    unsigned levels=0,field=0;for(unsigned token=0;token<DUEL_HAND_MAX+10;++token)if(token!=monster_index){unsigned c=ritual_material_card(d,side,token);if(c&&d->cards[c].family==DUEL_MONSTER&&d->cards[c].level){levels+=d->cards[c].level;field|=token>=DUEL_HAND_MAX&&token<DUEL_HAND_MAX+5;}}
    return levels>=r->level&&(empty_zone(p->monsters)>=0||field);
}
static unsigned ritual_monsters(const Duel *d,unsigned side,unsigned spell,DuelChoiceOption *out,unsigned capacity){const RitualRule *r=ritual_rule(spell);unsigned count=0;const DuelPlayer *p=&d->players[side];
    for(unsigned i=0;i<p->hand_count;++i)if(ritual_possible(d,side,i,r)){if(out&&count<capacity)out[count]=(DuelChoiceOption){p->hand[i],i,DUEL_OPTION_CARD};++count;}return count;}
static unsigned ritual_selected(const DuelRitualTask *t,unsigned token){for(unsigned i=0;i<t->count;++i)if(t->selected[i]==token)return 1;return 0;}
static unsigned ritual_levels(const Duel *d,unsigned side,unsigned *field){const DuelRitualTask *t=&d->ritual_task;unsigned levels=0;*field=0;
    for(unsigned i=0;i<t->count;++i){unsigned token=t->selected[i],card=ritual_material_card(d,side,token);levels+=d->cards[card].level;*field|=token>=DUEL_HAND_MAX&&token<DUEL_HAND_MAX+5;}return levels;}
static unsigned ritual_material_options(const Duel *d,unsigned side,DuelChoiceOption *out,unsigned capacity){const DuelRitualTask *t=&d->ritual_task;unsigned field,levels=ritual_levels(d,side,&field),count=0;
    for(unsigned token=0;token<DUEL_HAND_MAX+10;++token){if(token==t->monster_index||ritual_selected(t,token))continue;unsigned card=ritual_material_card(d,side,token);if(!card||d->cards[card].family!=DUEL_MONSTER||!d->cards[card].level)continue;
        if(levels+d->cards[card].level>=t->level&&empty_zone(d->players[side].monsters)<0&&!field&&(token<DUEL_HAND_MAX||token>=DUEL_HAND_MAX+5))continue;
        if(out&&count<capacity)out[count]=(DuelChoiceOption){card,token,DUEL_OPTION_CARD};++count;}return count;}
static void finish_ritual(Duel *d,unsigned side,unsigned position){if(!special_summon_allowed_for(d,side))return;DuelPlayer *p=&d->players[side];DuelRitualTask *t=&d->ritual_task;unsigned card=p->hand[t->monster_index];DuelCardState ritual_state=pile_card_state(p,PILE_HAND,t->monster_index);
    for(unsigned i=p->hand_count;i>0;--i){unsigned index=i-1;if(index==t->monster_index)remove_hand(p,index);else if(ritual_selected(t,index))grave_material_state(d,side,take_pile_card(p,PILE_HAND,index),DUEL_HISTORY_RITUAL);}
    for(unsigned token=0;token<10;++token)if(ritual_selected(t,DUEL_HAND_MAX+token))leave_field(d,tribute_owner(side,token),token%5,0,LEAVE_RITUAL);
    int index=empty_zone(p->monsters);if(index<0)return;DuelSlot *s=&p->monsters[index];clear(s,sizeof(*s));s->card=card;s->owner=side;s->position=position;s->summoned_turn=d->turn;s->properly_summoned=1;s->return_controller=DUEL_NO_SLOT;slot_install_state(d,s,ritual_state);s->properly_summoned=1;special_summon_success(d,side,index);
    d->summon_notice=(DuelWindow){DUEL_WINDOW_SUMMON,side,index,0,s->serial,0,card,0,2};event(d,DUEL_EVENT_REVIVE,side,index,card,t->count);clear(t,sizeof(*t));
}
