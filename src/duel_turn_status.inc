/* Virtual player effects have no live field source. Original kind3 counters
 * tick after every player's End; expiry is therefore an absolute turn. */
static void turn_status_end(Duel *d){borrow_end(d);temporary_end(d);for(unsigned p=0;p<2;++p){if(d->crush_until_end[p]&&d->crush_until_end[p]<=d->turn)d->crush_until_end[p]=0;if(d->weather_until_end[p]&&d->weather_until_end[p]<=d->turn)d->weather_until_end[p]=0;}}
static void crush_draw(Duel *d,unsigned side,unsigned hand){
    if(!d->crush_until_end[side]||hand>=d->players[side].hand_count)return;
    const DuelCardDef *c=&d->cards[d->players[side].hand[hand]];
    if(c->family==DUEL_MONSTER&&c->attack>=1500)send_hand_effect(d,side,hand,1-side,1);
}
static int weather_gate(const Duel *d,unsigned actor){for(unsigned i=0;i<6;++i){const DuelSlot *s=(i==DUEL_FIELD_SLOT?&d->players[1-actor].field:&d->players[1-actor].backrow[i]);if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==347)return 1;}return 0;}
static void weather_resolve(Duel *d,const DuelChainLink *link){
    unsigned actor=link->command.player,enemy=1-actor,mask=0;
    for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[enemy].backrow[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==347)mask|=1u<<(enemy*6+i);}
    if(!mask)return;move_backrows_mask(d,mask,0,0);
    unsigned count=actor==d->active&&d->phase==DUEL_MAIN2?2:actor^d->active,until=d->turn+count;
    if(until>d->weather_until_end[actor])d->weather_until_end[actor]=until;
}
static int weather_battle_repeat(Duel *d,unsigned actor){
    if(!d->weather_until_end[actor])return 0;d->weather_until_end[actor]=0;d->battle_attacker[actor]=0;
    for(unsigned i=0;i<5;++i)d->players[actor].monsters[i].attacked=0;
    event(d,DUEL_EVENT_PHASE,actor,0,0,DUEL_BATTLE);return 1;
}
static int gamble_validate(const Duel *d,const DuelCommand *c){const DuelPlayer *p=&d->players[c->player];return d->players[1-c->player].hand_count>=6&&p->hand_count<=2&&p->deck_count>=5-p->hand_count?DUEL_OK:DUEL_NOT_ALLOWED;}
static void gamble_choose(Duel *d,unsigned guess){
    unsigned side=d->choice.player,card=d->choice.card,roll=random_next(d)&1;finish_choice(d);
    d->random_result=(DuelRandomResult){d->random_result.sequence+1,card,side,1,roll,guess,roll==guess};
    if(roll==guess){if(d->players[side].hand_count<5)draw_cards(d,side,5-d->players[side].hand_count);}else d->skip_next_turn[side]=1;
}
static int crush_cost_target(const Duel *d,unsigned actor,unsigned token){
    if(token>=10)return 0;unsigned side=token/5,index=token%5;const DuelSlot *s=&d->players[side].monsters[index];
    return duel_can_tribute(d,actor,side,index)&&(side==actor||s->position!=DUEL_FACEDOWN_DEFENSE)&&duel_attribute(d,side,index)==2&&duel_attack_value(d,side,index)<=1000;
}
static unsigned crush_options(const Duel *d,unsigned actor,DuelChoiceOption *options,unsigned capacity){
    unsigned n=0;for(unsigned t=0;t<10;++t)if(crush_cost_target(d,actor,t)){if(options&&n<capacity)options[n]=(DuelChoiceOption){d->players[t/5].monsters[t%5].card,t,DUEL_OPTION_CARD};++n;}return n;
}
static int crush_validate(const Duel *d,const DuelCommand *c){if(!c->cost)return crush_options(d,c->player,0,0)?DUEL_OK:DUEL_NOT_ALLOWED;return c->cost==1&&c->tributes&&crush_cost_target(d,c->player,c->tributes-1)?DUEL_OK:DUEL_BAD_TARGET;}
static int crush_choose(Duel *d,unsigned token){
    DuelCommand selected=d->equip_mode_command;finish_choice(d);clear(&d->equip_mode_command,sizeof(d->equip_mode_command));if(token==DUEL_NO_SLOT)return DUEL_OK;selected.cost=1;selected.tributes=token+1;return activate(d,&selected,selected.action==DUEL_ACTION_ACTIVATE_BACKROW);
}
static void crush_resolve(Duel *d,const DuelChainLink *link){
    unsigned actor=link->command.player,enemy=1-actor;DuelPlayer *p=&d->players[enemy];
    for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->monsters[i];if(s->card&&(s->position==DUEL_FACEDOWN_DEFENSE?d->cards[s->card].attack:duel_attack_value(d,enemy,i))>=1500)effect_destroy_monster(d,link->card,enemy,i);}
    for(unsigned i=0;i<p->hand_count;){const DuelCardDef *c=&d->cards[p->hand[i]];if(c->family==DUEL_MONSTER&&c->attack>=1500)send_hand_effect(d,enemy,i,actor,1);else ++i;}
    unsigned until=d->turn+4+(enemy^d->active);if(until>d->crush_until_end[enemy])d->crush_until_end[enemy]=until;
}
