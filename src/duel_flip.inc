#include "duel_flip_data.h"
static const FlipRule *flip_rule(const Duel *d,unsigned card){for(unsigned i=0;i<DUEL_FLIP_RULE_COUNT;++i)if(flip_rules[i].name==d->cards[card].name_group)return &flip_rules[i];return 0;}
static unsigned flip_offer_options(const Duel *d,DuelChoiceOption *options,unsigned capacity){unsigned n=0;if(d->players[d->choice.player].life>2000){if(options&&capacity)options[n]=(DuelChoiceOption){d->choice.card,0,DUEL_OPTION_CONFIRM};++n;}if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return n+1;}
static void flip_offer_choose(Duel *d,unsigned index){unsigned side=d->choice.player,card=d->choice.card;uint32_t serial=d->choice.serial;finish_choice(d);if(index==DUEL_NO_SLOT)return;d->players[side].life-=2000;DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command.player=side;link->card=card;link->source_serial=serial;link->speed=1;link->effect_activation=1;d->trigger_chain_pending=1;}
static int flip_auto_pay(Duel *d,unsigned side,unsigned card){const FlipRule *r=flip_rule(d,card);if(!r)return 0;DuelPlayer *p=&d->players[side];unsigned count=r->op==7?3:r->op==8?2:0;
    if(!count)return 1;if((r->op==7?p->deck_count:p->hand_count)<count||p->banished_count+count>DUEL_PILE_MAX)return 0;
    for(unsigned i=0;i<count;++i){DuelCardState removed=r->op==7?take_pile_card(p,PILE_DECK,p->deck_count-1):take_pile_card(p,PILE_HAND,random_next(d)%p->hand_count);removed.history=r->op==7?removed.history&3:0;put_pile_card(d,side,PILE_BANISHED,removed,0);}return 1;}
static void flip_auto_resolve(Duel *d,const DuelChainLink *link){unsigned side=link->command.player,enemy=1-side;const FlipRule *r=flip_rule(d,link->card);if(!r)return;
    if(r->op==22){parasite_resolve(d,link);}
    else if(r->op==21){cyber_begin(d,link);}
    else if(r->op==20){jigen_resolve(d,link);}
    else if(r->op==19){weather_resolve(d,link);}
    else if(r->op==18){castle_flip_resolve(d,link);}
    else if(r->op==17){pile_summon_begin(d,side,link->card,9);}
    else if(r->op==15){history_supply_begin(d,side,link->card);}
    else if(r->op==11){if(link->command.cost==1){DuelPlayer *p=&d->players[side];p->life=p->life>5000?p->life-5000:0;event(d,DUEL_EVENT_SPELL,side,0,link->card,-5000);}else recover(d,side,r->amount,link->card);}
    else if(r->op==12){} /* Availability flag supplies the original continuous bonus. */
    else if(r->op==13)damage(d,enemy,r->amount);
    else if(r->op==14){if(d->players[side].hand_count){hand_begin_choice(d,side,side,side,link->card,1,1,0,0);d->hand_task.next_owner_plus1=enemy+1;}else hand_begin_choice(d,enemy,enemy,side,link->card,1,1,0,0);}
    else if(r->op==1){draw_cards(d,side,r->amount);}
    else if(r->op==2){duel_draw_cards(d,enemy,r->amount,1);}
    else if(r->op==3){duel_mill_deck(d,enemy,r->amount,side);}
    else if(r->op==4){unsigned n=d->players[enemy].field.card!=0;for(unsigned i=0;i<5;++i)n+=d->players[enemy].backrow[i].card!=0;damage(d,enemy,n*r->amount);}
    else if(r->op==5){unsigned mask=0;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[enemy].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].level==r->amount)mask|=1u<<(enemy*5+i);}destroy_monsters_mask(d,mask);}
    else if(r->op==6){for(unsigned j=0;j<2;++j){unsigned owner=j?enemy:side;DuelPlayer *p=&d->players[owner];while(p->hand_count)discard_hand_effect(d,owner,0,side,1);}for(unsigned j=0;j<2&&d->winner<0;++j)draw_cards(d,j?enemy:side,r->amount);}
    else if(r->op==10)peek_begin(d,side,link->card,r->amount);
    else if(r->op==9){unsigned mask=0;for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[p].backrow[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==328)mask|=1u<<(p*6+i);}if(mask){move_backrows_mask(d,mask,0,0);for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[p].monsters[i];if(s->card&&s->position==DUEL_FACEUP_DEFENSE&&duel_race(d,p,i)==1&&!battle_destroyed(d,p,i))duel_change_monster_position(d,p,i,DUEL_FACEUP_ATTACK);}}}
    else if(r->op==7||r->op==8)damage(d,enemy,r->amount);
}
