#include "duel_continuous_data.h"
static const ContinuousRule *continuous_rule(const Duel *d,unsigned card){for(unsigned i=0;i<DUEL_CONTINUOUS_RULE_COUNT;++i)if(continuous_rules[i].name==d->cards[card].name_group)return &continuous_rules[i];return 0;}
static int continuous_live(const Duel *d,unsigned side,unsigned index){const DuelSlot *s=&d->players[side].backrow[index];return s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&s->effect_active&&d->cards[s->card].effect==DUEL_E_CONTINUOUS&&!duel_field_effect_negated(d,s->card)&&!duel_card_prohibited(d,s->card,s->serial);}
static unsigned continuous_count(const Duel *d,unsigned op){unsigned n=0;for(unsigned s=0;s<2;++s)for(unsigned i=0;i<5;++i)if(continuous_live(d,s,i)){const ContinuousRule *r=continuous_rule(d,d->players[s].backrow[i].card);n+=r&&r->op==op;}return n;}
int duel_hand_revealed(const Duel *d,unsigned owner){return owner<2&&(live_monster_name(d,1121,3)||live_backrow_name(d,1093,1u<<(1-owner))||(d->active==owner&&live_backrow_name(d,1152,3)));}
unsigned duel_public_hand_options(const Duel *d,unsigned owner,DuelChoiceOption *options,unsigned capacity){if(!duel_hand_revealed(d,owner))return 0;const DuelPlayer *p=&d->players[owner];for(unsigned i=0;options&&i<p->hand_count&&i<capacity;++i)options[i]=(DuelChoiceOption){p->hand[i],i,DUEL_OPTION_CARD};return p->hand_count;}
static int continuous_has_umi(const Duel *d){for(unsigned side=0;side<2;++side){const DuelSlot *s=&d->players[side].field;if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==333)return 1;}return 0;}
int duel_spell_immune(const Duel *d,unsigned side,unsigned index){if(!d||side>1||index>=5)return 0;const DuelSlot *s=&d->players[side].monsters[index];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE)return 0;unsigned name=d->cards[s->card].name_group;return (name==1326||name==1329)&&!duel_card_prohibited(d,s->card,s->serial)&&continuous_has_umi(d);}
static int continuous_validate(const Duel *d,unsigned card){const ContinuousRule *r=continuous_rule(d,card);return !r?DUEL_UNSUPPORTED:r->op==39&&ground_free_count(d)<2?DUEL_NOT_ALLOWED:r->op==35?DUEL_NOT_ALLOWED:r->op==17&&!continuous_has_umi(d)?DUEL_NOT_ALLOWED:DUEL_OK;}
static int fragrance_allows(const Duel *d,unsigned side,unsigned index,unsigned from_set,unsigned card){if(d->cards[card].family!=DUEL_SPELL||!live_backrow_name(d,1180,3))return 1;if(!from_set)return 0;const DuelSlot *s=index==DUEL_FIELD_SLOT?&d->players[side].field:&d->players[side].backrow[index];return !s->position_lock_until||s->position_lock_until<=d->turn;}
static int continuous_stat_bonus(const Duel *d,unsigned side,unsigned index){const DuelSlot *host=&d->players[side].monsters[index];if(!host->card||host->position==DUEL_FACEDOWN_DEFENSE)return 0;int value=0;unsigned copies=0;
    for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[p].monsters[i];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE)continue;if(!(p==side&&i==index)&&d->cards[s->card].name_group==d->cards[host->card].name_group)++copies;if(p==side&&s->position==DUEL_FACEUP_DEFENSE&&duel_race(d,side,index)==13&&(monster_abilities_for(&d->cards[s->card])&16)&&!battle_destroyed(d,p,i))value+=500;}
    if(copies)value+=500*continuous_count(d,9);return value;}
static int continuous_battle_protected(const Duel *d,unsigned side){return continuous_has_umi(d)&&live_backrow_name(d,1423,1u<<side);}
static int continuous_attack_allowed(const Duel *d,unsigned side,unsigned index){if(duel_card_prohibited(d,d->players[side].monsters[index].card,d->players[side].monsters[index].serial)||duel_binding_locked(d,side,index)||sleep_attack_locked(d,side,index)||trait_attack_locked(d,side,index))return 0;const DuelSlot *m=&d->players[side].monsters[index];
    if(thousand_eyes_locked(d,side,index))return 0;
    if(toon_card(&d->cards[m->card])&&m->summoned_turn==d->turn)return 0;
    if(m->no_attack_until&&d->turn<=m->no_attack_until)return 0;
    if(equipment_status_mask(d,side,index,64)||(duel_race(d,side,index)!=7&&equipment_status_mask(d,side,index,1)))return 0;
    if(equipment_status_mask(d,side,index,4))return 0;
    unsigned immune=duel_spell_immune(d,side,index);if(!immune&&continuous_count(d,14)&&duel_attack_value(d,side,index)>=1500)return 0;
    for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i)if(continuous_live(d,owner,i)){const ContinuousRule *r=continuous_rule(d,d->players[owner].backrow[i].card);if(!r)continue;
        if(!immune&&r->op==1&&side!=owner)return 0;if(!immune&&r->op==2)return 0;if(r->op==3&&d->battle_attacker[side]&&d->battle_attacker[side]!=m->serial)return 0;
        if(r->op==4&&d->cards[m->card].level>=r->amount)return 0;if(!immune&&r->op==6&&owner!=side&&duel_race(d,side,index)==r->amount)return 0;}return 1;}
static unsigned hand_limit(const Duel *d){return continuous_count(d,7)?DUEL_HAND_MAX:6;}
static int continuous_ai_wants(const Duel *d,unsigned side,unsigned card){const ContinuousRule *r=continuous_rule(d,card);if(!r||continuous_count(d,r->op))return 0;
    unsigned own=0,enemy=0,high=0,insects=0;for(unsigned i=0;i<5;++i){own+=d->players[side].monsters[i].card!=0;const DuelSlot *s=&d->players[1-side].monsters[i];if(s->card){++enemy;if(s->position!=DUEL_FACEDOWN_DEFENSE){high+=d->cards[s->card].level>=4;insects+=duel_race(d,1-side,i)==10;}}}
    if(r->op==39)return ground_free_count(d)>=2;
    if(r->op==32||r->op==36||r->op==37)return 1;if(r->op==38){for(unsigned i=0;i<5;++i)if(d->players[1-side].monsters[i].card&&d->players[1-side].monsters[i].reborn_origin)return 1;return 0;}
    if(r->op==31)return own<2;
    if(r->op==26){if(d->players[side].life<=1000+500*(int)continuous_count(d,21))return 0;for(unsigned i=0;i<d->players[side].hand_count;++i)if(toon_card(&d->cards[d->players[side].hand[i]]))return 1;return 0;}
    if(r->op==23||r->op==24||r->op==25||r->op==27||r->op==28||r->op==29||r->op==30)return 1;if(r->op==22){for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[1-side].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&duel_race(d,1-side,i)==1)return 1;}return 0;}if(r->op==20||r->op==21)return d->players[side].life>d->players[1-side].life||enemy>own;if(r->op==19)return own>0;if(r->op==18)return enemy>0;if(r->op==8)return enemy>=own;
    if(r->op==9)return own>0;if(r->op==10||r->op==12||r->op==13||r->op==15||r->op==16)return 1;if(r->op==11)return d->players[side].hand_count<3;if(r->op==14)return enemy>own;if(r->op==17)return continuous_has_umi(d)&&enemy>0;
    if(r->op==1)return enemy!=0;if(r->op==2||r->op==3)return enemy>own;if(r->op==4)return high&&enemy>own;if(r->op==6)return insects!=0;if(r->op==7)return d->players[side].hand_count>5;
    if(r->op==5){for(unsigned i=0;i<d->players[side].hand_count;++i){const DuelCardDef *c=&d->cards[d->players[side].hand[i]];if(c->family==DUEL_MONSTER&&c->level>=5)return 0;}return 1;}return 0;}

static void summon_restrictions(Duel *d,unsigned side,unsigned index){DuelSlot *s=&d->players[side].monsters[index];if(s->card&&s->position==DUEL_FACEUP_ATTACK&&duel_race(d,side,index)==1&&continuous_count(d,22))s->position=DUEL_FACEUP_DEFENSE;if(continuous_count(d,8))d->players[side].monsters[index].no_attack_until=d->turn;}

/* Costs are paid once at declaration, before any response. They are never
 * retested during battle resolution, a replay, or a later spell link. */
static unsigned hand_play_cost(const Duel *d){return 500*continuous_count(d,21);}
static unsigned attack_play_cost(const Duel *d,unsigned side,unsigned index){return (toon_card(&d->cards[d->players[side].monsters[index].card])?500:0)+500*continuous_count(d,20)+((monster_abilities_for(&d->cards[d->players[side].monsters[index].card])&32)?1000:0);}
static int can_pay_action(const Duel *d,unsigned side,unsigned amount){return d->players[side].life>(int)amount;}
static void pay_action(Duel *d,unsigned side,unsigned amount){if(amount){d->players[side].life-=amount;event(d,DUEL_EVENT_SPELL,side,0,0,-(int)amount);}}

static int continuous_position_allowed(const Duel *d,unsigned side,unsigned index,unsigned position){const DuelSlot *s=&d->players[side].monsters[index];return position!=DUEL_FACEUP_ATTACK||!s->card||duel_race(d,side,index)!=1||!continuous_count(d,22);}
