/* Parameterized immediate effects share the native chain/instance lifecycle. */
#include "duel_simple_data.h"
static const SimpleRule *simple_rule_def(const DuelCardDef *c){if(c->effect!=DUEL_E_SIMPLE)return 0;for(unsigned i=0;i<DUEL_SIMPLE_RULE_COUNT;++i)if(simple_rules[i].name==c->name_group)return &simple_rules[i];return 0;}
unsigned duel_simple_target_kind(const DuelCardDef *c){if(c->name_group==1417||c->name_group==1545)return 2;const SimpleRule *r=simple_rule_def(c);return r&&(r->op==87||r->op==88||r->op==25||r->op==33||r->op==45)?2:r&&(r->op==91||r->op==7||r->op==17||r->op==23||r->op==26||r->op==27||r->op==31||r->op==34||r->op==46||r->op==49||r->op==51||r->op==56||r->op==58||r->op==62||r->op==63||r->op==69||r->op==77||r->op==81||r->op==83)?1:0;}
static unsigned grave_monsters(const Duel *d,unsigned side){unsigned count=0;for(unsigned i=0;i<d->players[side].grave_count;++i)count+=d->cards[d->players[side].grave[i]].family==DUEL_MONSTER;return count;}
static int summon_trap_threshold(const Duel *d,const SimpleRule *r,unsigned side,unsigned index){int value=r->other==2?duel_defense_value(d,side,index):duel_attack_value(d,side,index);return r->other?value<=r->amount:value>=r->amount;}
/* Non-targeting selection is made on resolution, using current ATK.
 * Equal highest values let the controller choose among all eligible copies. */
static unsigned simple_highest_options(const Duel *d,unsigned side,DuelChoiceOption *options,unsigned capacity){unsigned enemy=1-side,count=0;int highest=-1;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[enemy].monsters[i];if(!s->card||s->position!=DUEL_FACEUP_ATTACK)continue;int value=duel_attack_value(d,enemy,i);if(value>highest){highest=value;count=0;}if(value==highest){if(options&&count<capacity)options[count]=(DuelChoiceOption){s->card,i,DUEL_OPTION_CARD};++count;}}return count;}
static unsigned simple_destroy_options(const Duel *d,unsigned side,unsigned card,DuelChoiceOption *options,unsigned capacity){if(d->cards[card].effect!=DUEL_E_FISSURE)return simple_highest_options(d,side,options,capacity);unsigned enemy=1-side,count=0;int lowest=0;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[enemy].monsters[i];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE)continue;int value=duel_attack_value(d,enemy,i);if(!count||value<lowest){lowest=value;count=0;}if(value==lowest){if(options&&count<capacity)options[count]=(DuelChoiceOption){s->card,i,DUEL_OPTION_CARD};++count;}}return count;}
static void simple_destroy_choose(Duel *d,unsigned index){unsigned side=1-d->choice.player,card=d->choice.card;finish_choice(d);effect_destroy_monster(d,card,side,index);}
static int simple_validate(const Duel *d,const DuelCommand *c,unsigned card){
    const SimpleRule *r=simple_rule_def(&d->cards[card]);if(!r)return DUEL_UNSUPPORTED;if(r->op==92)return fake_trap_ready(d,c)?DUEL_OK:DUEL_NOT_ALLOWED;if(r->op==93)return bait_validate(d,c);if(r->op==94)return c->action==DUEL_ACTION_ACTIVATE_BACKROW&&reactive_field_gate(d,c->player,1149)?DUEL_OK:DUEL_NOT_ALLOWED;if(r->op>=87&&r->op<=91)return reactive_field_validate(d,c,card);if(r->op==86)return hats_validate(d,c);if(r->op==85)return borrow_validate(d,c);if(r->op==84)return chosen_ready(d,c->player,c->action==DUEL_ACTION_ACTIVATE_HAND)?DUEL_OK:DUEL_NOT_ALLOWED;if(r->op==83)return acid_validate(d,c);if(r->op==82)return d->players[1-c->player].hand_count&&revival_job_space(d)<64?DUEL_OK:DUEL_NOT_ALLOWED;if(r->op==81)return dimension_validate(d,c,card);if(r->op==80)return crush_validate(d,c);if(r->op==79)return gamble_validate(d,c);if(r->op==78)return mystic_validate(d,c);if(r->op==77)return recovery_validate(d,c,card);if(r->op>=75)return hand_selection_validate(d,c,d->cards[card].name_group);if(r->op==74)return doomed_validate(d,c->player);if(r->op>=71)return destruction_validate(d,c,d->cards[card].name_group);if(r->op==70)return DUEL_OK;if(r->op==69)return makiu_validate(d,c);if(r->op>=67)return stat_snapshot_validate(d,c,card);if(r->op>=64)return retarget_validate(d,c,card);if(r->op==62||r->op==63){if(!duel_forced_attack_ready(d,c->player,d->cards[card].name_group))return DUEL_NOT_ALLOWED;return duel_battle_redirect_target(d,d->cards[card].name_group,c->player,&d->window,c->target_player,c->target)?DUEL_OK:DUEL_BAD_TARGET;}if(r->op==59)return duel_destruction_punch_ready(d,c->player)?DUEL_OK:DUEL_NOT_ALLOWED;if(r->op==60)return duel_sebek_ready(d,c->player)?DUEL_OK:DUEL_NOT_ALLOWED;if(r->op==61)return duel_time_machine_available(d,c->player)?DUEL_OK:DUEL_NOT_ALLOWED;if(r->op==58)return defusion_target(d,c->target_player,c->target)?DUEL_OK:DUEL_BAD_TARGET;if(r->op==57)return summon_procedure_insect_validate(d,c);if(r->op==55)return equipment_transfer_validate(d,c);if(r->op==56)return equipment_transfer_any(d)&&c->target_player<2&&c->target<5&&d->players[c->target_player].monsters[c->target].card&&d->players[c->target_player].monsters[c->target].position!=DUEL_FACEDOWN_DEFENSE&&duel_can_target_monster(d,c->target_player,c->target)?DUEL_OK:DUEL_BAD_TARGET;if(r->op==54)return d->end_timer_count<64?DUEL_OK:DUEL_NOT_ALLOWED;
    if(duel_simple_target_kind(&d->cards[card])==1&&c->target_player<2&&c->target<5&&!effect_affects_monster(d,card,c->target_player,c->target))return DUEL_BAD_TARGET;
    if(r->op==51){if(d->players[c->player].hand_count<2+(c->action==DUEL_ACTION_ACTIVATE_HAND))return DUEL_NOT_ALLOWED;if(c->target_player>1||c->target>=5)return DUEL_BAD_TARGET;const DuelSlot *s=&d->players[c->target_player].monsters[c->target];return s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&!duel_is_token(d,s->card)&&duel_can_target_monster(d,c->target_player,c->target)?DUEL_OK:DUEL_BAD_TARGET;}
    if(r->op==52)return d->phase==DUEL_STANDBY&&(monsters(&d->players[0])+monsters(&d->players[1]))?DUEL_OK:DUEL_NOT_ALLOWED;
    if(r->op==53){const DuelWindow *w=&d->window;if(w->kind!=DUEL_WINDOW_SET||w->player==c->player||w->source>=5)return DUEL_NOT_ALLOWED;const DuelSlot *s=&d->players[w->player].monsters[w->source];return s->card&&s->serial==w->serial&&s->position==DUEL_FACEDOWN_DEFENSE&&!s->facedown_attack&&duel_can_target_monster(d,w->player,w->source)?DUEL_OK:DUEL_NOT_ALLOWED;}
    if(r->op==49)return special_summon_allowed_for(d,c->player)&&multiply_cost(d,c->player,c->target_player,c->target)?DUEL_OK:DUEL_BAD_TARGET;
    if(r->op==50)return special_summon_allowed_for(d,c->player)&&token_space(d,c->player)>=4&&!d->summon_activity[c->player]?DUEL_OK:DUEL_NOT_ALLOWED;
    if(r->op==47)return d->phase==DUEL_MAIN1&&!d->main1_action_used?DUEL_OK:DUEL_NOT_ALLOWED;
    if(r->op==46){if(d->phase!=DUEL_MAIN1)return DUEL_WRONG_PHASE;if(c->target_player!=1-c->player||c->target>=5||!duel_can_target_monster(d,c->target_player,c->target)||tribute_spell_immune(d,c->target_player,c->target))return DUEL_BAD_TARGET;}
    if(r->op==44){if(r->amount==8)return (d->window.kind==DUEL_WINDOW_EQUIP||(d->window.events&(1u<<DUEL_WINDOW_EQUIP)))?DUEL_OK:DUEL_NOT_ALLOWED;return d->chain_count&&(card_effect_class(&d->cards[d->chain[d->chain_count-1].card])&r->amount)?DUEL_OK:DUEL_NOT_ALLOWED;}
    if(r->op==42)return pile_summon_validate(d,c->player,r->amount);
    if(r->op==41){if(r->amount==2&&!d->players[1-c->player].hand_count)return DUEL_NOT_ALLOWED;if(r->amount==3&&(d->players[c->player].life<=r->other+(c->action==DUEL_ACTION_ACTIVATE_HAND?hand_play_cost(d):0)||!peek_hidden_field(d,c->player,0,0)))return DUEL_NOT_ALLOWED;}
    if(r->op>=38&&r->op<=40)return pile_target_validate(d,c->player,r->op);
    if(r->op==36){const DuelWindow *w=&d->window;if(w->kind!=DUEL_WINDOW_ATTACK||w->player==c->player||w->negated||!simple_highest_options(d,c->player,0,0))return DUEL_NOT_ALLOWED;}
    if(r->op==35){if(!d->chain_count)return DUEL_NOT_ALLOWED;const DuelChainLink *root=&d->chain[d->chain_count-1];if(root->effect_activation||root->command.player==c->player||d->cards[root->card].family!=DUEL_SPELL||d->cards[root->card].effect!=r->amount)return DUEL_NOT_ALLOWED;}
    if(r->op==33){if(c->target_player>1||c->target>DUEL_FIELD_SLOT||(c->action==DUEL_ACTION_ACTIVATE_BACKROW&&c->target_player==c->player&&c->target==c->source))return DUEL_BAD_TARGET;const DuelSlot *s=c->target==DUEL_FIELD_SLOT?&d->players[c->target_player].field:&d->players[c->target_player].backrow[c->target];if(!s->card||s->position!=DUEL_FACEDOWN_DEFENSE)return DUEL_BAD_TARGET;}
    if(r->op==31||r->op==34){if(c->target_player>1||c->target>=5)return DUEL_BAD_TARGET;const DuelSlot *s=&d->players[c->target_player].monsters[c->target];if(!s->card||!duel_can_target_monster(d,c->target_player,c->target)||(r->op==34?s->position!=DUEL_FACEDOWN_DEFENSE:s->position==DUEL_FACEDOWN_DEFENSE))return DUEL_BAD_TARGET;}
    if(r->op==32){unsigned found=0;for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];found|=s->card&&(s->position==DUEL_FACEDOWN_DEFENSE||(duel_original_classification[s->card]&1));}if(!found)return DUEL_NOT_ALLOWED;}
    if(r->op==25||r->op==45){if(c->target_player!=1-c->player||c->target>DUEL_FIELD_SLOT)return DUEL_BAD_TARGET;const DuelSlot *s=c->target==DUEL_FIELD_SLOT?&d->players[c->target_player].field:&d->players[c->target_player].backrow[c->target];if(!s->card)return DUEL_BAD_TARGET;}
    if(r->op==26||r->op==27){if(r->op==27){unsigned found=0;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[c->player].monsters[i];found|=s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==34;}if(!found)return DUEL_NOT_ALLOWED;}if(c->target_player>1||c->target>=5||(r->op==27&&c->target_player==c->player)||!duel_can_target_monster(d,c->target_player,c->target)||(r->op==26&&d->players[c->target_player].monsters[c->target].position==DUEL_FACEDOWN_DEFENSE))return DUEL_BAD_TARGET;}
    if(r->op==24){const DuelWindow *w=&d->window;if(w->kind!=DUEL_WINDOW_SUMMON||w->formation>1||w->player!=1-c->player||w->source>=5)return DUEL_NOT_ALLOWED;const DuelSlot *s=&d->players[w->player].monsters[w->source];if(!s->card||s->serial!=w->serial||s->position==DUEL_FACEDOWN_DEFENSE||!duel_can_target_monster(d,w->player,w->source)||!summon_trap_threshold(d,r,w->player,w->source))return DUEL_NOT_ALLOWED;}
    if((r->op==3||r->op==4||r->op==10)&&d->players[c->player].deck_count<r->amount)return DUEL_NOT_ALLOWED;
    if(r->op==13||r->op==14||r->op==20||r->op==21){const DuelWindow *w=&d->window;if(w->kind!=DUEL_WINDOW_ATTACK||w->player==c->player||w->negated)return DUEL_NOT_ALLOWED;}
    if(r->op==21&&!duel_can_target_monster(d,d->window.player,d->window.source))return DUEL_NOT_ALLOWED;
    if(r->op==12||r->op==16){unsigned faceup=0;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[1-c->player].monsters[i];faceup|=s->card&&s->position!=DUEL_FACEDOWN_DEFENSE;}if(!faceup)return DUEL_NOT_ALLOWED;}
    if(r->op==23&&!grave_monsters(d,c->player))return DUEL_NOT_ALLOWED;
    if(r->op==17||r->op==23){if(c->target_player>1||c->target>=5)return DUEL_BAD_TARGET;const DuelSlot *s=&d->players[c->target_player].monsters[c->target];if(!s->card||!duel_can_target_monster(d,c->target_player,c->target)||s->position==DUEL_FACEDOWN_DEFENSE)return DUEL_BAD_TARGET;}
    if(r->op==7){if(c->target_player!=1-c->player||c->target>=5)return DUEL_BAD_TARGET;const DuelSlot *s=&d->players[c->target_player].monsters[c->target];
        if(!s->card||!duel_can_target_monster(d,c->target_player,c->target)||(r->amount?s->position!=DUEL_FACEUP_ATTACK:s->position==DUEL_FACEUP_ATTACK))return DUEL_BAD_TARGET;}
    return DUEL_OK;
}
static void simple_resolve(Duel *d,const DuelChainLink *link){
    const DuelCommand *c=&link->command;const SimpleRule *r=simple_rule_def(&d->cards[link->card]);if(!r)return;unsigned side=c->player,enemy=1-side;DuelPlayer *p=&d->players[side];
    switch(r->op){
    case 92:fake_trap_resolve(d,link);break;
    case 93:bait_resolve(d,link);break;
    case 94:riot_begin(d,link);break;
    case 87:case 88:case 89:case 90:case 91:reactive_field_resolve(d,link);break;
    case 86:hats_begin(d,link);break;
    case 85:borrow_resolve(d,link);break;
    case 84:chosen_begin(d,link);break;
    case 83:acid_resolve(d,link);break;
    case 82:lightforce_resolve(d,link);break;
    case 81:dimension_resolve(d,link);break;
    case 80:crush_resolve(d,link);break;
    case 79:d->choice=(DuelTrigger){DUEL_CHOICE_GAMBLE_COIN,side,link->card,0};break;
    case 78:mystic_resolve(d,link);break;
    case 77:recovery_resolve(d,link);break;
    case 75:case 76:hand_selection_resolve(d,link);break;
    case 74:history_schedule(d,side,link->card);break;
    case 71:case 72:case 73:destruction_resolve(d,link);break;
    case 70:d->spirit_elimination_turn[side]=d->turn;break;
    case 69:makiu_resolve(d,link);break;
    case 67:case 68:stat_snapshot_resolve(d,link);break;
    case 64:case 65:case 66:duel_retarget_resolve(d,link);break;
    case 62:case 63:duel_forced_battle(d,link);break;
    case 59:duel_punch_mark(d,link);break;
    case 60:recover(d,side,link->target_card,link->card);break;
    case 61:battle_revival_resolve(d,link);break;
    case 58:defusion_resolve(d,link);break;
    case 57:pile_summon_begin_parameter(d,side,link->card,10,link->target_card);break;
    case 55:case 56:equipment_transfer_resolve(d,link,r->op==56);break;
    case 49:while(summon_token(d,side,1921,DUEL_FACEUP_DEFENSE)){}break;
    case 50:if(token_space(d,side)<4||d->summon_activity[side]||!special_summon_allowed_for(d,side))return;for(unsigned i=0;i<4;++i)summon_token(d,side,1922,DUEL_FACEUP_DEFENSE);summon_turn_ban(d,side);break;
    case 47:d->cold_wave_until=d->turn+2;d->cold_wave_actor=side;break;
    case 51:{DuelSlot *s=&d->players[c->target_player].monsters[c->target];if(!s->card||s->serial!=link->target_serial||s->position==DUEL_FACEDOWN_DEFENSE||live_backrow_name(d,1151,3)||!effect_affects_monster(d,link->card,c->target_player,c->target))return;duel_change_monster_position(d,c->target_player,c->target,s->position==DUEL_FACEUP_ATTACK?DUEL_FACEDOWN_ATTACK:DUEL_FACEDOWN_DEFENSE);break;}
    case 52:for(unsigned order=0;order<2;++order){unsigned owner=side^order;for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[owner].monsters[i];if(!s->card||!effect_affects_monster(d,link->card,owner,i))continue;unsigned pos=duel_monster_position(d,owner,i),flipped=pos==DUEL_FACEDOWN_DEFENSE;duel_change_monster_position(d,owner,i,pos==DUEL_FACEDOWN_ATTACK?DUEL_FACEDOWN_DEFENSE:pos==DUEL_FACEUP_ATTACK?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK);s->position_changed=1;if(flipped&&s->position!=DUEL_FACEDOWN_DEFENSE)flip_trigger(d,owner,i);}}break;
    case 53:{DuelSlot *s=&d->players[c->target_player].monsters[c->target];if(s->card&&s->serial==link->target_serial&&s->position==DUEL_FACEDOWN_DEFENSE&&!s->facedown_attack)duel_change_monster_position(d,c->target_player,c->target,DUEL_FACEUP_ATTACK);break;}
    case 48:{DuelSlot *s=spell_zone(p,link->source_slot);if(s->card==link->card&&s->serial==link->source_serial&&p->deck_count<DUEL_DECK_MAX){put_pile_card(d,side,PILE_DECK,card_state_reset(slot_card_state(s)),0);clear(s,sizeof(*s));event(d,DUEL_EVENT_RETURN,side,0,link->card,0);}shuffle(d,p);break;}
    case 46:soul_exchange_resolve(d,link);break;
    case 42:pile_summon_begin(d,side,link->card,r->amount);break;
    case 41:if(!(d->cards[link->card].name_group==1014&&link->player_redirect))peek_begin(d,side,link->card,r->amount);break;
    case 45:{DuelSlot *s=spell_zone(&d->players[c->target_player],c->target);if(s->card&&s->serial==link->target_serial)destroy(d,c->target_player,c->target,1);optional_set_begin(d,side,link->card);break;}
    case 43:{unsigned target=link_player(link,enemy);d->manual_position_lock_until[target]=d->turn+(target!=d->active);}break;
    case 44:if(d->type_suppression_turn!=d->turn)d->type_suppression_mask=0;d->type_suppression_turn=d->turn;d->type_suppression_mask|=r->amount;break;
    case 38:case 39:case 40:pile_target_resolve(d,link,r->op);break;
    case 37:p->skip_next_standby=1;break;
    case 36:{DuelChoiceOption options[5];unsigned count=simple_highest_options(d,side,options,5);if(count==1)destroy(d,enemy,options[0].index,0);else if(count>1)d->choice=(DuelTrigger){DUEL_CHOICE_SIMPLE_DESTROY,side,link->card,link->source_serial};break;}
    case 35:{if(c->target>=d->chain_count-1)return;DuelChainLink *root=&d->chain[c->target];if(root->command.player==side||root->effect_activation||d->cards[root->card].effect!=r->amount)return;root->effect_negated=1;unsigned mm=r->other==0?31u<<(enemy*5):0,bm=r->other==1?63u<<(enemy*6):0;if(d->cards[link->card].name_group==1022){const DuelSlot *s=&p->backrow[link->source_slot];if(s->card==link->card&&s->serial==link->source_serial)bm|=1u<<(side*6+link->source_slot);}destroy_field_mask(d,mm,bm);break;}
    case 31:{DuelSlot *s=&d->players[c->target_player].monsters[c->target];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE||s->serial!=link->target_serial)return;unsigned amount=duel_attack_value(d,c->target_player,c->target);destroy(d,c->target_player,c->target,0);simultaneous_effect_damage(d,amount);break;}
    case 32:{unsigned count=0;for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].monsters[i];if(!s->card)continue;if(s->position==DUEL_FACEDOWN_DEFENSE){detach_monster(d,s->serial,0);s->position=s->facedown_attack?DUEL_FACEUP_ATTACK:DUEL_FACEUP_DEFENSE;s->facedown_attack=0;}count+=(duel_original_classification[s->card]&1)!=0;}if(count&&!duel_field_effect_negated(d,link->card))damage(d,enemy,count*r->amount);break;}
    case 33:case 34:{unsigned backrow=r->op==33;DuelSlot *s=backrow?spell_zone(&d->players[c->target_player],c->target):&d->players[c->target_player].monsters[c->target];if(!s->card||s->serial!=link->target_serial||s->position!=DUEL_FACEDOWN_DEFENSE)return;unsigned card=s->card,purge=backrow?d->cards[card].family==DUEL_TRAP:(duel_original_classification[card]&2)!=0;if(duel_banish_field(d,c->target_player,c->target,backrow)&&purge)banish_deck_name(d,card);break;}
    case 30:d->players[enemy].skip_next_draw=1;break;
    case 28:d->reverse_turn=d->turn;break;
    case 29:for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&effect_affects_monster(d,link->card,side,i)){s->swap_parity=s->swap_until==d->turn?!s->swap_parity:1;s->swap_until=d->turn;}}break;
    case 25:move_backrows_mask(d,1u<<(c->target_player*6+c->target),1,0);break;
    case 26:case 27:{DuelSlot *s=&d->players[c->target_player].monsters[c->target];if(!s->card||s->serial!=link->target_serial||(r->op==26&&s->position==DUEL_FACEDOWN_DEFENSE))return;effect_destroy_monster(d,link->card,c->target_player,c->target);if(r->op==26)p->skip_next_draw=1;break;}
    case 24:{DuelSlot *s=&d->players[c->target_player].monsters[c->target];if(s->card&&s->serial==link->target_serial&&s->position!=DUEL_FACEDOWN_DEFENSE&&summon_trap_threshold(d,r,c->target_player,c->target))destroy(d,c->target_player,c->target,0);break;}
    case 1:move_backrows(d,3,0,0);break;
    case 2:{unsigned mask=0;for(unsigned s=0;s<2;++s)for(unsigned i=0;i<5;++i){const DuelSlot *m=&d->players[s].monsters[i];if(m->card&&m->position!=DUEL_FACEDOWN_DEFENSE&&duel_race(d,s,i)==r->amount)mask|=1u<<(s*5+i);}destroy_monsters_mask(d,effect_monster_mask(d,link->card,mask));break;}
    case 3:case 4:case 10:
        draw_cards(d,link_player(link,side),r->amount);
        if(r->op==4&&d->winner<0)recover(d,enemy,r->other,link->card);
        if(r->op==10&&d->winner<0){unsigned owner=link_player(link,side);DuelPlayer *drawn=&d->players[owner];if(drawn->hand_count){d->choice=(DuelTrigger){DUEL_CHOICE_DISCARD_HAND,owner,link->card,0};d->choice.selected_count=drawn->hand_count<r->other?drawn->hand_count:r->other;}}break;
    case 5:damage(d,enemy,r->amount);if(d->winner<0)damage(d,side,r->other);break;
    case 6:{unsigned target=link_player(link,enemy);damage(d,target,r->amount*d->players[target].hand_count);}break;
    case 7:{DuelSlot *m=&d->players[c->target_player].monsters[c->target];
        if(!m->card||m->serial!=link->target_serial||!effect_affects_monster(d,link->card,c->target_player,c->target))return;
        if(r->amount){if(m->position==DUEL_FACEUP_ATTACK)duel_change_monster_position(d,c->target_player,c->target,DUEL_FACEUP_DEFENSE);}
        else if(m->position!=DUEL_FACEUP_ATTACK){unsigned flipped=m->position==DUEL_FACEDOWN_DEFENSE;if(flipped)detach_monster(d,m->serial,0);duel_change_monster_position(d,c->target_player,c->target,DUEL_FACEUP_ATTACK);if(flipped)flip_trigger(d,c->target_player,c->target);}break;}
    case 8:for(unsigned i=0;i<5;++i){DuelSlot *m=&d->players[enemy].monsters[i];if(m->card&&m->position==DUEL_FACEDOWN_DEFENSE){detach_monster(d,m->serial,0);m->position=m->facedown_attack?DUEL_FACEUP_ATTACK:DUEL_FACEUP_DEFENSE;m->facedown_attack=0;flip_trigger(d,enemy,i);}}break;
    case 9:{unsigned counts[2]={d->players[0].hand_count,d->players[1].hand_count};
        for(unsigned order=0;order<2;++order){unsigned owner=d->active^order;DuelPlayer *a=&d->players[owner];while(a->hand_count)discard_hand_effect(d,owner,a->hand_count-1,side,1);}
        for(unsigned order=0;order<2&&d->winner<0;++order){unsigned owner=d->active^order;draw_cards(d,owner,counts[owner]);}break;}
    case 11:damage(d,enemy,r->amount*monsters(&d->players[enemy]));break;
    case 12:for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[enemy].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE)duel_change_monster_position(d,enemy,i,s->position==DUEL_FACEUP_ATTACK?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK);}break;
    case 13:{unsigned mask=0;for(unsigned i=0;i<5;++i)if(d->players[enemy].monsters[i].card&&d->players[enemy].monsters[i].position==DUEL_FACEUP_ATTACK)mask|=1u<<(enemy*5+i);destroy_monsters_mask(d,mask);break;}
    case 14:d->window.negated=1;battle_phase_cleanup(d);d->phase=DUEL_MAIN2;event(d,DUEL_EVENT_PHASE,d->active,0,0,DUEL_MAIN2);break;
    case 15:d->battle_protect_turn[side]=d->turn;break;
    case 16:{unsigned roll=random_next(d)%6+1;for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[enemy].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE){s->temporary_bonus-=roll*r->amount;fairy_stat_delta(d,s,-(int)(roll*r->amount));}}event(d,DUEL_EVENT_SPELL,side,0,link->card,roll);break;}
    case 18:recover(d,side,r->amount*(monsters(p)+monsters(&d->players[enemy])),link->card);break;
    case 19:{unsigned mask=0;for(unsigned s=0;s<2;++s)for(unsigned i=0;i<5;++i){const DuelSlot *host=&d->players[s].monsters[i];if(!host->card||!host->serial)continue;for(unsigned owner=0;owner<2;++owner)for(unsigned j=0;j<5;++j){const DuelSlot *e=&d->players[owner].backrow[j];if(e->card&&e->effect_active&&d->cards[e->card].effect==DUEL_E_EQUIP&&e->link_serial==host->serial)mask|=1u<<(s*5+i);}}destroy_monsters_mask(d,effect_monster_mask(d,link->card,mask));break;}
    case 20:{DuelWindow *w=&d->window;if(w->player<2&&w->source<5){const DuelSlot *a=&d->players[w->player].monsters[w->source];if(a->card&&a->serial==w->serial&&a->position==DUEL_FACEUP_ATTACK)recover(d,side,duel_attack_value(d,w->player,w->source),link->card);}break;}
    case 17:case 21:case 23:{DuelSlot *s=&d->players[c->target_player].monsters[c->target];if(!s->card||s->serial!=link->target_serial||s->position==DUEL_FACEDOWN_DEFENSE||!effect_affects_monster(d,link->card,c->target_player,c->target))return;int amount=r->amount*(r->op==23?grave_monsters(d,side):1);s->temporary_attack+=amount;fairy_stat_delta(d,s,amount);s->temporary_defense+=r->other;break;}
    }
}
static int simple_ai_command(const Duel *d,unsigned side,DuelCommand *c,unsigned card){
    const SimpleRule *r=simple_rule_def(&d->cards[card]);if(r&&r->op==41){if(simple_validate(d,c,card)!=DUEL_OK)return 0;return r->amount!=1||d->players[1-side].deck_count>0;}if(!r)return 0;if(r->op==92)return fake_trap_ready(d,c);if(r->op==93){for(unsigned order=0;order<2;++order)for(unsigned i=0;i<5;++i){c->target_player=side^1^order;c->target=i;if(bait_validate(d,c)==DUEL_OK)return 1;}return 0;}if(r->op==94)return simple_validate(d,c,card)==DUEL_OK;if(r->op>=87&&r->op<=91)return reactive_field_ai(d,side,c,card);if(r->op==86)return hats_validate(d,c)==DUEL_OK;if(r->op==85){if(borrow_validate(d,c)!=DUEL_OK)return 0;for(unsigned i=0;i<d->players[1-side].grave_count;++i)if(borrow_candidate(d,side,i)){c->cost=1;c->tributes=i+1;return 1;}return 0;}if(r->op==84)return simple_validate(d,c,card)==DUEL_OK;if(r->op==83){for(unsigned i=0;i<5;++i){c->target_player=1-side;c->target=i;if(acid_validate(d,c)==DUEL_OK)return 1;}return 0;}if(r->op==82)return d->players[1-side].hand_count&&revival_job_space(d)<64;if(r->op==81){for(unsigned i=0;i<5;++i){c->target_player=side;c->target=i;if(dimension_validate(d,c,card)==DUEL_OK)return duel_attack_value(d,side,i)<500;}return 0;}if(r->op==80){for(unsigned t=0;t<10;++t)if(crush_cost_target(d,side,t)){c->cost=1;c->tributes=t+1;return 1;}return 0;}if(r->op==79)return gamble_validate(d,c)==DUEL_OK;if(r->op==78)return mystic_validate(d,c)==DUEL_OK;if(r->op==77){for(unsigned i=0;i<5;++i){c->target_player=side;c->target=i;if(recovery_validate(d,c,card)==DUEL_OK)return d->players[side].hand_count>=3;}return 0;}if(r->op>=75)return hand_selection_validate(d,c,d->cards[card].name_group)==DUEL_OK;if(r->op==74)return doomed_validate(d,side)==DUEL_OK&&d->players[side].hand_count>2;if(r->op>=71)return destruction_ai(d,side,c,card);if(r->op>=69)return makiu_ai(d,side,c,card);if(r->op>=67)return stat_snapshot_ai(d,side,card);if(r->op>=64)return retarget_ai(d,side,card);
    if(r->op==59||r->op==60)return simple_validate(d,c,card)==DUEL_OK;
    if(r->op==61)return duel_time_machine_available(d,side)&&d->battle_last_death.controller==side;
    if(r->op==58){for(unsigned order=0;order<2;++order){unsigned owner=side^order;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[owner].monsters[i];c->target_player=owner;c->target=i;if(!defusion_target(d,owner,i))continue;if(owner!=side)return 1;const DuelFusionRecipe *recipe=fusion_recipe(d,s->card);if(recipe&&s->owner==side&&s->properly_summoned&&token_space(d,side)+1>=recipe->count&&duel_defusion_materials(d,side,s->card,0)==recipe->count&&special_summon_allowed_for(d,side))return 1;}}return 0;}
    if(r->op==57)return summon_procedure_insect_validate(d,c)==DUEL_OK;
    if(r->op==55){DuelChoiceOption sources[12],targets[10];unsigned n=equipment_transfer_sources(d,sources,12);for(unsigned k=0;k<n;++k){unsigned token=sources[k].index,m=equipment_transfer_targets(d,token/6,token%6,targets,10);for(unsigned j=0;j<m;++j)if(targets[j].index/5==side){c->cost=2;c->tributes=token+1;c->target_player=side;c->target=targets[j].index%5;return 1;}}return 0;}
    if(r->op==56){for(unsigned i=0;i<5;++i){c->target_player=side;c->target=i;if(simple_validate(d,c,card)==DUEL_OK)return 1;}return 0;}
    if(r->op==42)return pile_summon_validate(d,side,r->amount)==DUEL_OK&&pile_summon_candidates(d,side,r->amount,0,0)>0;
    if(r->op==41){if(r->amount==2&&!d->players[1-c->player].hand_count)return DUEL_NOT_ALLOWED;if(r->amount==3&&(d->players[c->player].life<=r->other+(c->action==DUEL_ACTION_ACTIVATE_HAND?hand_play_cost(d):0)||!peek_hidden_field(d,c->player,0,0)))return DUEL_NOT_ALLOWED;}
    if(r->op>=38&&r->op<=40)return pile_target_validate(d,side,r->op)==DUEL_OK;
    if(r->op==36)return simple_validate(d,c,card)==DUEL_OK;
    if(r->op==37)return !d->players[side].skip_next_standby;
    if(r->op==35)return simple_validate(d,c,card)==DUEL_OK;
    if(r->op==31||r->op==33||r->op==34){int best=-1;for(unsigned i=0;i<(r->op==33?6:5);++i){c->target_player=1-side;c->target=i;if(simple_validate(d,c,card)==DUEL_OK&&(best<0||r->op!=31||duel_attack_value(d,1-side,i)>duel_attack_value(d,1-side,best)))best=i;}if(best<0)return 0;c->target_player=1-side;c->target=best;return r->op!=31||d->players[side].life>duel_attack_value(d,1-side,best);}
    if(r->op==32)return simple_validate(d,c,card)==DUEL_OK&&d->turn>1;
    if(r->op==30)return !d->players[1-side].skip_next_draw;
    if(r->op==28)return 0;
    if(r->op==29){int score=0;for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[p].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE)score+=(p==side?1:-1)*((int)d->cards[s->card].defense-d->cards[s->card].attack);}return score>0;}
    if(r->op==25||r->op==45){for(unsigned i=0;i<=DUEL_FIELD_SLOT;++i){c->target_player=1-side;c->target=i;if(simple_validate(d,c,card)==DUEL_OK)return 1;}return 0;}
    if(r->op==26||r->op==27){int best=-1;for(unsigned i=0;i<5;++i){c->target_player=1-side;c->target=i;if(simple_validate(d,c,card)==DUEL_OK&&(best<0||duel_attack_value(d,1-side,i)>duel_attack_value(d,1-side,best)))best=i;}if(best<0)return 0;c->target_player=1-side;c->target=best;return 1;}
    if(r->op==24)return simple_validate(d,c,card)==DUEL_OK;
    if(r->op==7){for(unsigned i=0;i<5;++i){c->target_player=1-side;c->target=i;if(simple_validate(d,c,card)==DUEL_OK)return 1;}return 0;}
    if(r->op==17||r->op==23){unsigned target=r->amount<0||r->other<0?1-side:side;int best=-1;for(unsigned i=0;i<5;++i){c->target_player=target;c->target=i;if(simple_validate(d,c,card)==DUEL_OK&&(best<0||duel_attack_value(d,target,i)>duel_attack_value(d,target,best)))best=i;}if(best<0)return 0;c->target_player=target;c->target=best;return d->turn>1;}
    if(r->op==54)return d->end_timer_count<64&&pile_summon_candidates(d,side,7,0,0)&&(d->field_to_grave[side]||monsters(&d->players[side]));
    if(r->op==51){if(d->players[side].hand_count<2+(c->action==DUEL_ACTION_ACTIVATE_HAND))return 0;for(unsigned i=0;i<5;++i){c->target_player=1-side;c->target=i;if(simple_validate(d,c,card)==DUEL_OK)return 1;}return 0;}
    if(r->op==52||r->op==53)return simple_validate(d,c,card)==DUEL_OK;
    if(r->op==50)return simple_validate(d,c,card)==DUEL_OK;
    if(r->op==49){for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i)if(multiply_cost(d,side,owner,i)){c->target_player=owner;c->target=i;return 1;}return 0;}
    if(r->op==47)return simple_validate(d,c,card)==DUEL_OK;
    if(r->op==46){if(d->phase!=DUEL_MAIN1||d->players[side].normal_summon_used)return 0;unsigned wanted=0;for(unsigned i=0;i<d->players[side].hand_count;++i){const DuelCardDef *m=&d->cards[d->players[side].hand[i]];wanted|=m->family==DUEL_MONSTER&&m->subtype<2&&!special_hand_provenance(m)&&duel_normal_tribute_cost(d,side,i,0)>0;}if(!wanted)return 0;int best=-1;for(unsigned i=0;i<5;++i){c->target_player=1-side;c->target=i;if(simple_validate(d,c,card)==DUEL_OK&&!duel_can_tribute(d,side,1-side,i)&&(best<0||duel_attack_value(d,1-side,i)>duel_attack_value(d,1-side,best)))best=i;}if(best<0)return 0;c->target_player=1-side;c->target=best;return 1;}
    if(r->op==44)return simple_validate(d,c,card)==DUEL_OK&&(r->amount==8?d->window.player!=side:d->chain[d->chain_count-1].command.player!=side);
    if(r->op==43)return d->manual_position_lock_until[1-side]<d->turn+1;
    if(r->op==1)return monsters(&d->players[side])<=monsters(&d->players[1-side])&&has_backrow_target(d,side,c->source,c->action==DUEL_ACTION_ACTIVATE_BACKROW);
    if(r->op==2){int benefit=0;for(unsigned s=0;s<2;++s)for(unsigned i=0;i<5;++i){const DuelSlot *m=&d->players[s].monsters[i];if(m->card&&m->position!=DUEL_FACEDOWN_DEFENSE&&duel_race(d,s,i)==r->amount)benefit+=s==side?-1:1;}return benefit>0;}
    if(r->op==8){for(unsigned i=0;i<5;++i)if(d->players[1-side].monsters[i].card&&d->players[1-side].monsters[i].position==DUEL_FACEDOWN_DEFENSE)return 1;return 0;}
    if(r->op==3||r->op==4||r->op==10)return d->players[side].deck_count>=r->amount;
    if(r->op==9)return d->players[side].hand_count<d->players[1-side].hand_count&&d->players[side].deck_count>=d->players[side].hand_count;
    if(r->op==5)return d->players[side].life>r->other||d->players[1-side].life<=r->amount;
    if(r->op>=11){if(simple_validate(d,c,card)!=DUEL_OK)return 0;if(r->op==13||r->op==14||r->op==20||r->op==21)return 1;if(r->op==15)return d->window.kind==DUEL_WINDOW_ATTACK&&d->window.player!=side&&d->battle_protect_turn[side]!=d->turn;return r->op==11||r->op==12||r->op==16||r->op==18;}
    return r->op==6&&d->players[1-side].hand_count>0;
}
