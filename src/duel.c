/* Native game rules and typed state. No original PC addresses, CPU registers,
 * instruction records, Win32 services or binary translator are used here. */
#include "duel.h"

static int fake_trap_ready(const Duel *,const DuelCommand *);
static void fake_trap_resolve(Duel *,const DuelChainLink *);
static int fake_trap_shields(const Duel *,unsigned,unsigned,unsigned);
static unsigned fake_trap_mask(const Duel *,unsigned);
static int bait_validate(const Duel *,const DuelCommand *);
static void bait_resolve(Duel *,const DuelChainLink *);
static unsigned bait_options(Duel *,DuelChoiceOption *,unsigned);
static int bait_choose(Duel *,unsigned);
static int bait_advance(Duel *);
static void begin_response(Duel *d,unsigned player);
static void forced_control_battle_end(Duel *);
static void destiny_clear_departures(Duel *,unsigned,unsigned);

static int activate(Duel *,const DuelCommand *,unsigned);
static int activate_monster(Duel *,const DuelCommand *);
static int activate_continuous_ability(Duel *,const DuelCommand *);
static void call_dark_cleanup(Duel *);
static int special_summon_allowed_for(const Duel *,unsigned);
static void special_summon_success(Duel *,unsigned,unsigned);
static void damage_state_clear(Duel *);
static void queue_trigger(Duel *,unsigned,unsigned,unsigned,uint32_t);
static unsigned discard_hand_effect(Duel *,unsigned,unsigned,unsigned,unsigned);
static unsigned send_hand_effect(Duel *,unsigned,unsigned,unsigned,unsigned);
static unsigned discard_hand_unpaid(Duel *,unsigned,unsigned,unsigned,unsigned);
static void passive_grave_collect(Duel *,unsigned);
static void reactive_discard_collect(Duel *,unsigned);
static void reactive_draw_batch(Duel *,unsigned,unsigned);
static void reactive_promote(Duel *);
static void reactive_field_move(Duel *,unsigned,unsigned,const DuelSlot *,unsigned,unsigned);
static int reactive_hand_name(unsigned);
static void reactive_hand_resolve(Duel *,const DuelChainLink *);
static void passive_discard_collect(Duel *,unsigned,unsigned);
static int passive_flush(Duel *);
static void borrow_penalty(Duel *,unsigned,uint32_t);
static void return_monster(Duel *,const DuelTarget *);
static void yado_begin(Duel *,const DuelChainLink *);
static void arrival_choice_trigger(Duel *,unsigned,unsigned,unsigned);
static void arrival_target_resolve(Duel *,const DuelChainLink *,const DuelTarget *,unsigned);
static int sleep_attack_locked(const Duel *,unsigned,unsigned);
static void monster_negation_refresh(Duel *);
static void castle_flip_resolve(Duel *,const DuelChainLink *);
static void castle_standby(Duel *,unsigned);
static void pumpking_observer(Duel *,unsigned);
static unsigned ground_free_count(const Duel *);
static void ground_reconcile(Duel *);
static void finish_turn(Duel *);
static void crush_draw(Duel *,unsigned,unsigned);
static void hats_battle_end(Duel *);
static void borrow_end(Duel *);
static void parasite_draw(Duel *,unsigned,unsigned);
static void parasite_resolve(Duel *,const DuelChainLink *);
static void cyber_begin(Duel *,const DuelChainLink *);
static void drill_begin(Duel *,unsigned,unsigned);
static int weather_gate(const Duel *,unsigned);
static void weather_resolve(Duel *,const DuelChainLink *);
static void temporary_zone_reconcile(Duel *);
static void temporary_standby(Duel *,unsigned);
static void temporary_end(Duel *);
static int jigen_ready(const Duel *,unsigned,unsigned,uint32_t);
static int jigen_standby_step(Duel *);
static void jigen_resolve(Duel *,const DuelChainLink *);
static void kotodama_observer(Duel *);
static int same_name(const Duel *,unsigned,unsigned);
static int castle_bonus_since(const Duel *,unsigned,unsigned,int);
static int goddess_unordered_bonus(const Duel *,unsigned,unsigned);
static void advance_resolution(Duel *d);
static void finish_choice(Duel *d);
static void equip_grave_trigger(Duel *,unsigned,unsigned,uint32_t);
static void periodic_queue(Duel *,unsigned,unsigned);
static void time_wizard_offer(Duel *,unsigned,unsigned);
static void dark_sage_search_begin(Duel *,unsigned,unsigned);
static void leave_field(Duel *d,unsigned controller,unsigned slot,unsigned backrow,unsigned cause);
static unsigned monsters(const DuelPlayer *);
static int battle_destroyed(const Duel *d,unsigned player,unsigned slot);
static int has_backrow_target(const Duel *,unsigned,unsigned,unsigned);
static unsigned live_backrow_name(const Duel *,unsigned,unsigned);
static unsigned live_monster_name(const Duel *,unsigned,unsigned);
static int equip_effect_negated(const Duel *,unsigned);
static void upkeep_queue(Duel *,unsigned);
static void continuous_standby_queue(Duel *,unsigned);
static void pile_target_resolve(Duel *,const DuelChainLink *,unsigned);
static int control_add(Duel *,unsigned,unsigned,unsigned,unsigned,uint32_t,unsigned);
static void control_reconcile(Duel *);
static void control_exchange(Duel *,const DuelChainLink *,const DuelTarget *);
static void control_replace_name(Duel *,uint32_t,unsigned);
static int big_shield_intercept(Duel *,const DuelChainLink *);
static void big_shield_battle_end(Duel *);
static void gila_summon_trigger(Duel *,unsigned,unsigned);
static int magnet_target_rule(const Duel *,unsigned,unsigned,unsigned);
static unsigned race_excluding(const Duel *,unsigned,unsigned,uint32_t);
static int trait_attack_locked(const Duel *,unsigned,unsigned);
static void equipment_order_attach(Duel *,const DuelSlot *,const DuelSlot *);
static uint32_t source_field_order(const DuelSlot *);
static const DuelSlot *stat_link_source(const Duel *,const DuelStatLink *);
static int stat_source_after(const Duel *,uint32_t,uint32_t,unsigned,int);
static int binding_stat_bonus_since(const Duel *,unsigned,unsigned,int);
static void fairy_battle_phase_end(Duel *);
static void fairy_attack_begin(Duel *);
static unsigned link_player(const DuelChainLink *,unsigned);
static int retarget_validate(const Duel *,const DuelCommand *,unsigned);
static int retarget_ai(const Duel *,unsigned,unsigned);
static int stat_snapshot_validate(const Duel *,const DuelCommand *,unsigned);
static void stat_snapshot_resolve(Duel *,const DuelChainLink *);
static void stat_limiter_end(Duel *);
static int stat_snapshot_ai(const Duel *,unsigned,unsigned);
int duel_card_prohibited(const Duel *,unsigned,uint32_t);
static int activation_blocked_for_slot(const Duel *,unsigned,uint32_t);
static void prohibition_snapshot(Duel *,const DuelSlot *);
static void revival_job_queue(Duel *,unsigned);
static unsigned revival_job_space(const Duel *);
static void necro_schedule(Duel *,unsigned,unsigned,uint32_t);
static void monster_equipment_new_deployment(Duel *,uint32_t);
static void monster_equipment_battle_end(Duel *);
static void sword_capture_battle(Duel *);
static void battle_countdown_reset(Duel *,uint32_t);
static void battle_countdown_end(Duel *);
static void dimensional_resolve(Duel *,const DuelChainLink *);
void duel_battle_revival_capture(Duel *);
static void battle_revival_commit(Duel *);
static void battle_revival_resolve(Duel *,const DuelChainLink *);
void duel_punch_finish(Duel *);
static void queen_capture_battle(Duel *);
static void queen_end_queue(Duel *);
static int thousand_eyes_locked(const Duel *,unsigned,unsigned);
static int absorption_battle_protect(Duel *,unsigned,unsigned);
static unsigned absorption_reflection(const Duel *,const DuelWindow *,unsigned);
static void history_schedule(Duel *,unsigned,unsigned);
static int summon_procedure_illusion_gate(const Duel *,unsigned,unsigned);
static void pile_summon_begin(Duel *,unsigned,unsigned,unsigned);
static void history_end_queue(Duel *);
static void history_supply_begin(Duel *,unsigned,unsigned);
static unsigned history_return_name(unsigned);
static void history_will_resolve(Duel *,const DuelChainLink *);
static unsigned history_target_options(const Duel *,unsigned,unsigned,DuelChoiceOption *,unsigned);

static void clear(void *p,unsigned n){uint8_t *b=p;while(n--)*b++=0;}
static uint32_t random_next(Duel *d){uint32_t x=d->random_state;x^=x<<13;x^=x>>17;x^=x<<5;return d->random_state=x;}
static void shuffle(Duel *d,DuelPlayer *p){for(unsigned n=p->deck_count;n>1;--n){unsigned j=random_next(d)%n;uint16_t card=p->deck[n-1];p->deck[n-1]=p->deck[j];p->deck[j]=card;DuelCardState state=p->deck_state[n-1];p->deck_state[n-1]=p->deck_state[j];p->deck_state[j]=state;}}
static void event(Duel *d,unsigned type,unsigned player,unsigned slot,unsigned card,int amount){if(d->phase==DUEL_MAIN1&&type!=DUEL_EVENT_NONE&&type!=DUEL_EVENT_PHASE&&type!=DUEL_EVENT_DRAW&&type!=DUEL_EVENT_WIN)d->main1_action_used=1;if(type==DUEL_EVENT_SUMMON||type==DUEL_EVENT_REVIVE||type==DUEL_EVENT_FUSION)d->summon_activity[player]=1;d->last_event=(DuelLogEvent){type,player,slot,0,card,amount};++d->revision;++d->event_sequence;}
static void battle_phase_cleanup(Duel *d){hats_battle_end(d);fairy_battle_phase_end(d);forced_control_battle_end(d);monster_equipment_battle_end(d);DuelPlayer *p=&d->players[d->active];for(unsigned i=0;i<5;++i){DuelSlot *s=&p->monsters[i];if(s->card&&s->attacked&&d->cards[s->card].effect==DUEL_E_GOBLIN){s->position=DUEL_FACEUP_DEFENSE;s->position_lock_until=d->turn+2;}}}
static int card_valid(const Duel *d,unsigned card){return card>0&&card<DUEL_CARD_COUNT&&d->cards[card].family<=DUEL_TRAP;}
static const DuelSuppressionRule *suppression_rule(const Duel *d,unsigned card){for(unsigned i=0;i<d->suppression_count;++i)if(d->suppression[i].card==card)return &d->suppression[i];return 0;}
int duel_suppression_setup(Duel *d,const DuelSuppressionRule *rules,unsigned count){
    if(!d||(!rules&&count)||count>DUEL_CARD_COUNT||d->turn!=1||d->chain_count||d->choice.kind)return DUEL_BAD_ARGUMENT;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card||d->players[side].backrow[i].card)return DUEL_NOT_ALLOWED;
    /* This rule system currently covers the original's global trap locks.
       Other families and general monster negation need their own timing. */
    for(unsigned i=0;i<count;++i){const DuelSuppressionRule *r=&rules[i];if(!card_valid(d,r->card)||d->cards[r->card].effect!=DUEL_E_TRAP_LOCK||r->negates!=(1u<<DUEL_TRAP)||r->exempt_same_name>1||!((d->cards[r->card].family==DUEL_MONSTER&&r->blocks==r->negates&&!r->exempt_same_name)||(d->cards[r->card].family==DUEL_TRAP&&!r->blocks&&r->exempt_same_name)))return DUEL_BAD_ARGUMENT;
        for(unsigned j=0;j<i;++j)if(rules[j].card==r->card)return DUEL_BAD_ARGUMENT;
    }d->suppression=rules;d->suppression_count=count;return DUEL_OK;
}
static unsigned monster_suppression(const Duel *d,unsigned activation){
    if(!d->suppression_count)return 0;unsigned mask=0;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];if(!s->card||duel_card_prohibited(d,s->card,s->serial)||s->position==DUEL_FACEDOWN_DEFENSE||d->cards[s->card].effect!=DUEL_E_TRAP_LOCK||battle_destroyed(d,side,i)||(d->window.kind==DUEL_WINDOW_SUMMON_NEGATION&&d->window.player==side&&d->window.source==i&&d->window.serial==s->serial))continue;const DuelSuppressionRule *r=suppression_rule(d,s->card);if(r)mask|=activation?r->blocks:r->negates;}return mask;
}
static int activation_blocked_for_slot(const Duel *d,unsigned card,uint32_t serial){return d&&card_valid(d,card)&&((monster_suppression(d,1)&(1u<<d->cards[card].family))!=0||duel_card_prohibited(d,card,serial)||(d->cold_wave_until&&d->cards[card].family!=DUEL_MONSTER));}
int duel_card_activation_blocked(const Duel *d,unsigned card){return activation_blocked_for_slot(d,card,0);}
static unsigned card_effect_class(const DuelCardDef *c){unsigned type=c->original_property>>17&7;return c->family==DUEL_SPELL?(type==2?1:type==4?2:0):c->family==DUEL_TRAP&&type==4?4:0;}
static int type_effect_negated(const Duel *d,unsigned card){return d->type_suppression_turn==d->turn&&(d->type_suppression_mask&card_effect_class(&d->cards[card]));}
int duel_field_effect_negated(const Duel *d,unsigned card){
    if(!d||!card_valid(d,card))return 0;
    if(type_effect_negated(d,card))return 1;
    if(d->cards[card].family==DUEL_SPELL)return live_backrow_name(d,1154,3)!=0;
    if(!d->suppression_count||d->cards[card].family!=DUEL_TRAP)return 0;
    if(monster_suppression(d,0)&(1u<<DUEL_TRAP))return 1;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].backrow[i];if(!s->card||!s->effect_active||s->position==DUEL_FACEDOWN_DEFENSE||d->cards[s->card].effect!=DUEL_E_TRAP_LOCK||type_effect_negated(d,s->card))continue;const DuelSuppressionRule *r=suppression_rule(d,s->card);if(r&&(r->negates&(1u<<DUEL_TRAP))&&!(r->exempt_same_name&&d->cards[s->card].name_group==d->cards[card].name_group))return 1;}return 0;
}
static unsigned live_backrow_name(const Duel *d,unsigned name,unsigned sides){unsigned n=0;for(unsigned side=0;side<2;++side)if(sides&(1u<<side))for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].backrow[i];if(s->card&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==name&&!duel_field_effect_negated(d,s->card)&&((name==1154||name==1537||name==1530)||!duel_card_prohibited(d,s->card,s->serial)))++n;}return n;}
static int equip_effect_negated(const Duel *d,unsigned card){return duel_field_effect_negated(d,card)||(d->cards[card].family==DUEL_MONSTER&&live_backrow_name(d,1154,3))||live_backrow_name(d,1537,3)||(d->type_suppression_turn==d->turn&&(d->type_suppression_mask&8));}
/* Source family matters for effects on a monster; costs and player-level
 * effects deliberately do not use this filter. */
static int effect_affects_monster(const Duel *d,unsigned card,unsigned side,unsigned index){return d->cards[card].family!=DUEL_SPELL||!duel_spell_immune(d,side,index);}
static unsigned effect_monster_mask(const Duel *d,unsigned card,unsigned mask){if(d->cards[card].family==DUEL_SPELL)for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i)if(duel_spell_immune(d,p,i))mask&=~(1u<<(p*5+i));return mask;}
static DuelSlot *spell_zone(DuelPlayer *p,unsigned slot){return slot==DUEL_FIELD_SLOT?&p->field:&p->backrow[slot];}
static const DuelFieldRule *field_rule(const Duel *d,unsigned card){for(unsigned i=0;i<d->field_count;++i)if(d->fields[i].card==card)return &d->fields[i];return 0;}
int duel_field_setup(Duel *d,const DuelFieldRule *rules,unsigned count){
    if(!d||(!rules&&count)||count>DUEL_CARD_COUNT||d->turn!=1||d->chain_count||d->choice.kind)return DUEL_BAD_ARGUMENT;
    for(unsigned side=0;side<2;++side){if(d->players[side].field.card)return DUEL_NOT_ALLOWED;for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card||d->players[side].backrow[i].card)return DUEL_NOT_ALLOWED;}
    for(unsigned i=0;i<count;++i){const DuelFieldRule *r=&rules[i];if(!card_valid(d,r->card)||d->cards[r->card].family!=DUEL_SPELL||d->cards[r->card].effect!=DUEL_E_FIELD||r->kind>3||(!r->kind&&(r->positive||r->negative||r->attack||r->defense))||(r->positive&r->negative)||(r->kind==DUEL_FIELD_RACE&&((r->positive|r->negative)&~0x1ffffeu))||(r->kind==DUEL_FIELD_ATTRIBUTE&&((r->positive|r->negative)&~0x7eu))||(r->kind==DUEL_FIELD_DEFENSE&&(r->positive||r->negative||r->attack)))return DUEL_BAD_ARGUMENT;
        for(unsigned j=0;j<i;++j)if(rules[j].card==r->card)return DUEL_BAD_ARGUMENT;}
    d->fields=rules;d->field_count=count;return DUEL_OK;
}
static int field_bonus_since(const Duel *d,unsigned side,unsigned slot,unsigned defense,int after){
    const DuelSlot *monster=&d->players[side].monsters[slot];if(!monster->card||monster->position==DUEL_FACEDOWN_DEFENSE||duel_spell_immune(d,side,slot))return 0;const DuelCardDef *def=&d->cards[monster->card];int value=0;
    for(unsigned p=0;p<2;++p){const DuelSlot *s=&d->players[p].field;if(after>=0&&!stat_source_after(d,s->serial,monster->serial,4,after))continue;if(!s->card||!s->effect_active||s->position==DUEL_FACEDOWN_DEFENSE||duel_field_effect_negated(d,s->card))continue;const DuelFieldRule *r=field_rule(d,s->card);if(!r)continue;unsigned selector=r->kind==DUEL_FIELD_RACE?duel_race(d,side,slot):duel_attribute(d,side,slot);int sign=r->kind==DUEL_FIELD_DEFENSE?monster->position==DUEL_FACEUP_DEFENSE:selector<32?(r->positive&(1u<<selector)?1:r->negative&(1u<<selector)?-1:0):0;value+=(defense?r->defense:r->attack)*sign;}return value;
}
static int field_bonus(const Duel *d,unsigned side,unsigned slot,unsigned defense){return field_bonus_since(d,side,slot,defense,-1);}
#include "duel_spell_equipment.inc"
static const DuelEquipRule *equip_rule(const Duel *d,unsigned card){for(unsigned i=0;i<d->equip_count;++i)if(d->equips[i].card==card)return &d->equips[i];return 0;}
static int equip_eligible(const Duel *d,unsigned card,unsigned side,unsigned slot){
    if(!d||side>1||slot>=5||!card_valid(d,card))return 0;const DuelEquipRule *r=equip_rule(d,card);const DuelSlot *s=&d->players[side].monsters[slot];
    if(!r||!s->card||s->position==DUEL_FACEDOWN_DEFENSE)return 0;const DuelCardDef *c=&d->cards[s->card];if(c->family!=DUEL_MONSTER)return 0;
    if(r->kind==DUEL_EQUIP_RACE||r->kind==DUEL_EQUIP_EXCLUDE_RACE){unsigned race=duel_race(d,side,slot);return race<32&&((r->mask&(1u<<race))!=0)==(r->kind==DUEL_EQUIP_RACE);}
    if(r->kind==DUEL_EQUIP_ATTRIBUTE){unsigned attribute=duel_attribute(d,side,slot);return attribute<32&&(r->mask&(1u<<attribute))!=0;}
    return r->kind==DUEL_EQUIP_ANY||(r->kind==DUEL_EQUIP_NAMES&&(c->name_group==r->names[0]||c->name_group==r->names[1]));
}
int duel_equip_can_target(const Duel *d,unsigned card,unsigned side,unsigned slot){return d&&card_valid(d,card)&&d->cards[card].name_group==1417?spell_equipment_target(d,side,slot):equip_eligible(d,card,side,slot)&&duel_can_target_monster(d,side,slot);}
int duel_equip_setup(Duel *d,const DuelEquipRule *rules,unsigned count){
    if(!d||(!rules&&count)||count>DUEL_CARD_COUNT||d->turn!=1||d->chain_count||d->choice.kind)return DUEL_BAD_ARGUMENT;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card||d->players[side].backrow[i].card)return DUEL_NOT_ALLOWED;
    for(unsigned i=0;i<count;++i){const DuelEquipRule *r=&rules[i];if(!card_valid(d,r->card)||(d->cards[r->card].family==DUEL_MONSTER&&d->cards[r->card].name_group!=71&&d->cards[r->card].name_group!=1242&&d->cards[r->card].name_group!=1514&&d->cards[r->card].name_group!=735&&d->cards[r->card].name_group!=1170)||(d->cards[r->card].effect!=DUEL_E_EQUIP&&d->cards[r->card].name_group!=1258&&!(d->cards[r->card].name_group==1160&&d->cards[r->card].effect==DUEL_E_CALL_HAUNTED||(d->cards[r->card].name_group==1514||d->cards[r->card].name_group==735||d->cards[r->card].name_group==1170)&&d->cards[r->card].effect==DUEL_E_MONSTER)))return DUEL_BAD_ARGUMENT;
        if(r->kind==DUEL_EQUIP_RACE||r->kind==DUEL_EQUIP_EXCLUDE_RACE){if(!r->mask||(r->mask&~0x1ffffeu))return DUEL_BAD_ARGUMENT;}
        else if(r->kind==DUEL_EQUIP_ATTRIBUTE){if(!r->mask||(r->mask&~0x7eu))return DUEL_BAD_ARGUMENT;}
        else if(r->kind!=DUEL_EQUIP_NAMES&&r->kind!=DUEL_EQUIP_ANY)return DUEL_BAD_ARGUMENT;
        for(unsigned j=0;j<i;++j)if(rules[j].card==r->card)return DUEL_BAD_ARGUMENT;
    }d->equips=rules;d->equip_count=count;return DUEL_OK;
}
const DuelLifeRule *duel_life_rule(const Duel *d,unsigned card){if(!d)return 0;for(unsigned i=0;i<d->life_rule_count;++i)if(d->life_rules[i].card==card)return &d->life_rules[i];return 0;}
int duel_life_setup(Duel *d,const DuelLifeRule *rules,unsigned count){
    if(!d||(!rules&&count)||count>DUEL_CARD_COUNT||d->turn!=1||d->chain_count||d->choice.kind)return DUEL_BAD_ARGUMENT;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card||d->players[side].backrow[i].card)return DUEL_NOT_ALLOWED;
    for(unsigned i=0;i<count;++i){const DuelLifeRule *r=&rules[i];if(!card_valid(d,r->card)||d->cards[r->card].family!=DUEL_SPELL||d->cards[r->card].effect!=DUEL_E_LIFE||!r->amount||r->amount>32767||r->operation>DUEL_LIFE_DAMAGE||r->target>DUEL_LIFE_BOTH||(r->operation==DUEL_LIFE_DAMAGE&&r->target==DUEL_LIFE_BOTH))return DUEL_BAD_ARGUMENT;
        for(unsigned j=0;j<i;++j)if(rules[j].card==r->card)return DUEL_BAD_ARGUMENT;
    }d->life_rules=rules;d->life_rule_count=count;return DUEL_OK;
}
static int recall_family(const Duel *d,unsigned card){for(unsigned i=0;i<d->recall_count;++i)if(d->recalls[i].card==card)return d->recalls[i].family;return -1;}
static int recall_available(const Duel *d,unsigned side,unsigned card){const DuelPlayer *p=&d->players[side];int family=recall_family(d,card);if(family<0||p->hand_count>=DUEL_HAND_MAX)return 0;for(unsigned i=0;i<p->grave_count;++i)if(d->cards[p->grave[i]].family==family)return 1;return 0;}
int duel_recall_setup(Duel *d,const DuelRecallRule *rules,unsigned count){
    if(!d||(!rules&&count)||count>DUEL_CARD_COUNT||d->turn!=1||d->chain_count||d->choice.kind)return DUEL_BAD_ARGUMENT;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card||d->players[side].backrow[i].card)return DUEL_NOT_ALLOWED;
    for(unsigned i=0;i<count;++i){if(!card_valid(d,rules[i].card)||d->cards[rules[i].card].family!=DUEL_MONSTER||d->cards[rules[i].card].effect!=DUEL_E_GRAVE_RECALL||(rules[i].family!=DUEL_SPELL&&rules[i].family!=DUEL_TRAP))return DUEL_BAD_ARGUMENT;
        for(unsigned j=0;j<i;++j)if(rules[i].card==rules[j].card)return DUEL_BAD_ARGUMENT;}
    d->recalls=rules;d->recall_count=count;return DUEL_OK;
}
/* Attachments follow a monster instance when control or its zone changes. */
static int equip_host(const Duel *d,unsigned card,uint32_t serial,unsigned *side,unsigned *slot){
    if(!serial||!card_valid(d,card))return 0;if(d->cards[card].name_group==1417)return spell_equipment_host(d,serial,side,slot);for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i)if(d->players[p].monsters[i].card&&d->players[p].monsters[i].serial==serial&&d->players[p].monsters[i].position!=DUEL_FACEDOWN_DEFENSE){if(side)*side=p;if(slot)*slot=i;return 1;}return 0;
}
static void attach_equip(Duel *d,const DuelChainLink *link){if(d->cards[link->card].name_group==1417){DuelSlot *source=&d->players[link->command.player].backrow[link->source_slot];if(source->card==link->card&&source->serial==link->source_serial&&source->position!=DUEL_FACEDOWN_DEFENSE&&source->serial!=link->target_serial&&spell_equipment_host(d,link->target_serial,0,0)){source->effect_active=1;source->link_serial=link->target_serial;}return;}unsigned side=0,index=0;DuelSlot *s=&d->players[link->command.player].backrow[link->source_slot];if(s->card!=link->card||s->serial!=link->source_serial||s->position==DUEL_FACEDOWN_DEFENSE||!equip_host(d,link->card,link->target_serial,&side,&index)||!duel_can_target_monster(d,side,index))return;if((d->cards[link->card].name_group==1068||d->cards[link->card].name_group==1514)&&side==link->command.player)return;s->effect_active=1;s->link_serial=link->target_serial;if(d->cards[link->card].name_group==1068||d->cards[link->card].name_group==1514)control_add(d,side,index,link->command.player,link->card,s->serial,0);if(d->cards[link->card].name_group==1258){s->control_used=1-side;control_add(d,side,index,s->control_used,link->card,s->serial,0);}if(d->cards[link->card].name_group==962)s->control_used=link->command.position+1;if(d->cards[link->card].name_group==1448)s->control_used=link->command.position;equipment_order_attach(d,s,&d->players[side].monsters[index]);d->summon_notice=(DuelWindow){DUEL_WINDOW_EQUIP,link->command.player,link->source_slot,index,s->serial,link->target_serial,link->card};}
#include "duel_dynamic_equip.inc"
static int equip_bonus_since(const Duel *d,unsigned side,unsigned slot,unsigned defense,int after){
    const DuelSlot *host=&d->players[side].monsters[slot];if(!host->serial||host->position==DUEL_FACEDOWN_DEFENSE)return 0;int value=0;
    for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[p].backrow[i];if(after>=0&&!stat_source_after(d,s->serial,host->serial,2,after))continue;if(s->card&&s->effect_active&&s->link_serial==host->serial&&s->equip_role){if(s->equip_role==DUEL_EQUIP_ROLE_SWORD&&!defense&&!equip_effect_negated(d,s->card))value+=200;continue;}if(s->card&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE&&s->link_serial==host->serial&&(d->cards[s->card].name_group==1550?race_excluding(d,side,slot,source_field_order(s))==15:equip_eligible(d,s->card,side,slot))&&!equip_effect_negated(d,s->card)&&effect_affects_monster(d,s->card,side,slot)){const DuelEquipRule *r=equip_rule(d,s->card);value+=defense?r->defense:r->attack;unsigned op=dynamic_equip_op(d,s->card);if(op==1||op==2)value+=dynamic_equip_count(d,p,op)*(op==1?800:500);if(!defense&&(op==4||op==5))value-=s->control_used*(op==4?200:300);if(op==6&&s->control_used==(defense?2:1))value+=700;}}return value;
}
static int equip_bonus(const Duel *d,unsigned side,unsigned slot,unsigned defense){return equip_bonus_since(d,side,slot,defense,-1);}
int duel_aura_setup(Duel *d,const DuelAuraRule *rules,unsigned count){
    if(!d||(!rules&&count)||count>DUEL_CARD_COUNT||d->turn!=1||d->chain_count||d->choice.kind)return DUEL_BAD_ARGUMENT;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card||d->players[side].backrow[i].card)return DUEL_NOT_ALLOWED;
    for(unsigned i=0;i<count;++i){const DuelAuraRule *r=&rules[i];if(!card_valid(d,r->card)||(r->positive&r->negative)||((r->positive|r->negative)&~0x7eu)||!(r->positive|r->negative)||r->increase<0||r->decrease<0||(!r->increase&&r->positive)||(!r->decrease&&r->negative))return DUEL_BAD_ARGUMENT;
        const DuelCardDef *c=&d->cards[r->card];if(!((c->family==DUEL_MONSTER&&c->effect==DUEL_E_AURA)||(c->family==DUEL_SPELL&&c->effect==DUEL_E_EQUIP&&equip_rule(d,r->card))))return DUEL_BAD_ARGUMENT;
        for(unsigned j=0;j<i;++j)if(rules[j].card==r->card)return DUEL_BAD_ARGUMENT;
    }d->auras=rules;d->aura_count=count;return DUEL_OK;
}
#include "duel_status_equip.inc"
static const DuelAuraRule *aura_rule(const Duel *d,unsigned card){for(unsigned i=0;i<d->aura_count;++i)if(d->auras[i].card==card)return &d->auras[i];return 0;}
static int aura_bonus(const Duel *d,unsigned side,unsigned slot){
    if(!d->aura_count)return 0;const DuelSlot *host=&d->players[side].monsters[slot];if(!host->card||host->position==DUEL_FACEDOWN_DEFENSE)return 0;unsigned attribute=duel_attribute(d,side,slot);if(attribute<1||attribute>6)return 0;int value=0;
    for(unsigned p=0;p<2;++p)for(unsigned i=0;i<5;++i)for(unsigned backrow=0;backrow<2;++backrow){const DuelSlot *s=backrow?&d->players[p].backrow[i]:&d->players[p].monsters[i];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE||d->cards[s->card].effect!=(backrow?DUEL_E_EQUIP:DUEL_E_AURA))continue;const DuelAuraRule *r=aura_rule(d,s->card);if(!r)continue;
        if(backrow){if(!s->effect_active||equip_effect_negated(d,s->card)||!equip_host(d,s->card,s->link_serial,0,0)||!effect_affects_monster(d,s->card,side,slot))continue;}
        else if(battle_destroyed(d,p,i))continue;
        value+=(r->positive&(1u<<attribute)?r->increase:0)-(r->negative&(1u<<attribute)?r->decrease:0);
    }return value;
}
#include "duel_stat_links.inc"
#include "duel_monster.inc"
#include "duel_continuous.inc"
#include "duel_traits.inc"
static unsigned monsters(const DuelPlayer *p){unsigned n=0;for(unsigned i=0;i<5;++i)n+=p->monsters[i].card!=0;return n;}
static int empty_zone(const DuelSlot *slots){for(unsigned i=0;i<5;++i)if(!slots[i].card&&!slots[i].zone_blocked)return i;return -1;}
static void win(Duel *d,unsigned player){if(d->winner<0){d->winner=player;event(d,DUEL_EVENT_WIN,player,0,0,0);}}
int duel_exodia_complete(const Duel *d,unsigned side){if(!d||side>1)return 0;unsigned mask=0;const DuelPlayer *p=&d->players[side];for(unsigned i=0;i<p->hand_count;++i){unsigned name=d->cards[p->hand[i]].name_group;if(name>=16&&name<=20)mask|=1u<<(name-16);}return mask==31;}
/* The original checks each completed resolution: deck-out, Exodia, Board,
 * then LP. A callback may update both LP values before this boundary. */
static void check_victory_boundary(Duel *d){if(d->winner>=0||d->choice.kind)return;unsigned one=d->deck_failure_mask&1,two=d->deck_failure_mask&2;if(one||two){win(d,one&&two?2:one?1:0);return;}one=duel_exodia_complete(d,0);two=duel_exodia_complete(d,1);if(one||two){win(d,one&&two?2:two?1:0);return;}one=duel_destiny_complete(d,0);two=duel_destiny_complete(d,1);if(one||two){win(d,one&&two?2:two?1:0);return;}one=d->players[0].life<=0;two=d->players[1].life<=0;if(one||two)win(d,one&&two?2:one?1:0);}
static void check_special_victory(Duel *d){if(d->chain_count||d->trigger_count||d->resolving||d->response_player<2)return;check_victory_boundary(d);}
static void damage(Duel *d,unsigned player,unsigned amount){if(amount>(unsigned)d->players[player].life)amount=d->players[player].life;d->players[player].life-=amount;if(amount)d->reactive_pending.damage|=1u<<player;event(d,DUEL_EVENT_DAMAGE,player,0,0,amount);}
/* Original5787f0 dispatches every positive recovery request, including at
 * the 16-bit LP ceiling; FirePrincess observes that request, not the gain. */
static void recover(Duel *d,unsigned player,unsigned amount,unsigned card){if(!amount)return;if(amount>65535)amount=65535;unsigned room=65535u-(unsigned)d->players[player].life;d->players[player].life+=amount>room?room:amount;event(d,DUEL_EVENT_RECOVER,player,0,card,amount);periodic_queue(d,3,player);}
static uint16_t remove_card(uint16_t *cards,unsigned *count,unsigned index){uint16_t card=cards[index];for(unsigned i=index;i+1<*count;++i)cards[i]=cards[i+1];cards[--*count]=0;return card;}
#include "duel_pile_state.inc"
static uint16_t remove_hand(DuelPlayer *p,unsigned index){return take_pile_card(p,PILE_HAND,index).card;}
#include "duel_recruit.inc"
#include "duel_swarm.inc"
static unsigned live_monster_name(const Duel *d,unsigned name,unsigned sides){unsigned count=0;for(unsigned side=0;side<2;++side)if(sides&(1u<<side))for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==name&&!duel_card_prohibited(d,s->card,s->serial)&&!battle_destroyed(d,side,i)&&!(d->window.kind==DUEL_WINDOW_SUMMON_NEGATION&&d->window.player==side&&d->window.source==i&&d->window.serial==s->serial))++count;}return count;}
int duel_summon_permission(const Duel *d,unsigned side,unsigned kind){if(!d||side>1||kind>DUEL_PERMISSION_SPECIAL)return 0;if(kind==DUEL_PERMISSION_SET&&(!(d->field_name_cache&64)||!live_backrow_name(d,1151,3)))return 1;if((d->summon_ban_turn[side]&&d->summon_ban_turn[side]==d->turn&&(d->summon_ban_mask[side]&(kind==DUEL_PERMISSION_SPECIAL?2:1)))||((d->field_name_cache&32)&&live_backrow_name(d,1426,1u<<side))||live_monster_name(d,1526,3))return 0;return kind!=DUEL_PERMISSION_SPECIAL||!live_monster_name(d,1510,3);}
static int special_summon_allowed_for(const Duel *d,unsigned side){return duel_summon_permission(d,side,DUEL_PERMISSION_SPECIAL);}
static void special_summon_success(Duel *d,unsigned side,unsigned index){DuelSlot *s=&d->players[side].monsters[index];s->special_summon_marker=1;arrival_choice_trigger(d,side,index,1);summon_restrictions(d,side,index);gila_summon_trigger(d,side,index);if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==1526){unsigned before=d->trigger_count;queue_trigger(d,DUEL_TRIGGER_ARRIVAL_AUTO,side,s->card,s->serial);if(d->trigger_count>before)d->triggers[before].selected_count=index;}}
static int grave_redirect_active(const Duel *d){return live_monster_name(d,1107,3)!=0;}
static int grave_banish_allowed(const Duel *d,unsigned actor){return !live_monster_name(d,1511,1u<<(1-actor));}
static void grave_card_state(Duel *d,unsigned owner,DuelCardState state){owner=state.owner;if(duel_is_token(d,state.card))return;state=card_state_identify(d,state);if(grave_redirect_active(d)){if(put_pile_card(d,owner,PILE_BANISHED,state,0))event(d,DUEL_EVENT_BANISH,owner,0,state.card,0);return;}DuelPlayer *p=&d->players[owner];if(p->grave_count<DUEL_PILE_MAX){unsigned at=p->grave_count++;p->grave_serial[at]=state.serial;p->grave_proper[at]=state.proper;p->grave_history[at]=state.history;p->grave[at]=state.card;passive_grave_collect(d,owner);if(d->cards[state.card].name_group==1242)queue_trigger(d,DUEL_CHOICE_EQUIP_RECYCLE,owner,state.card,state.serial);if(d->cards[state.card].name_group==478)queue_trigger(d,DUEL_TRIGGER_GRAVE_AUTO,owner,state.card,state.serial);}}
static unsigned grave_effect_history(const Duel *d,unsigned controller){return DUEL_HISTORY_ARRIVED|((d->resolving&&d->effect_actor_plus1&&d->effect_actor_plus1!=controller+1)?DUEL_HISTORY_OPPONENT:0);}
static void grave_material_state(Duel *d,unsigned owner,DuelCardState state,unsigned history){state.history=history;grave_card_state(d,owner,state);}
static void grave_field_state(Duel *d,unsigned owner,DuelCardState state){unsigned before=d->players[owner].grave_count;grave_card_state(d,owner,state);if(d->players[owner].grave_count>before)equip_grave_trigger(d,owner,state.card,d->players[owner].grave_serial[before]);}

static void grave_deck_state(Duel *d,unsigned side,DuelCardState state,unsigned actor,unsigned bulk){unsigned physical=state.owner,before=d->players[physical].grave_count;state.history=(bulk?state.history:state.history&3)|(actor!=side&&(!bulk||!grave_redirect_active(d))?DUEL_HISTORY_OPPONENT:0);grave_card_state(d,physical,state);if(d->players[physical].grave_count>before&&actor!=side&&d->cards[state.card].name_group==198){uint32_t serial=d->players[physical].grave_serial[before];if(!duel_card_prohibited(d,state.card,serial))queue_trigger(d,DUEL_TRIGGER_GRAVE_AUTO,side,state.card,serial);}}
unsigned duel_mill_deck(Duel *d,unsigned side,unsigned count,unsigned actor){if(!d||side>1||actor>1)return 0;DuelPlayer *p=&d->players[side];unsigned moved=0;while(moved<count&&p->deck_count){DuelCardState state=take_pile_card(p,PILE_DECK,p->deck_count-1);grave_deck_state(d,side,state,actor,1);++moved;}return moved;}
unsigned duel_return_grave_to_deck(Duel *d,unsigned side){if(!d||side>1)return 0;DuelPlayer *p=&d->players[side];unsigned moved=0;for(unsigned i=p->grave_count;i>0;--i){DuelCardState state=pile_card_state(p,PILE_GRAVE,i-1);unsigned fusion=d->cards[state.card].family==DUEL_MONSTER&&d->cards[state.card].subtype==2;if(fusion?p->fusion_count>=DUEL_FUSION_MAX:p->deck_count>=DUEL_DECK_MAX)continue;take_pile_card(p,PILE_GRAVE,i-1);put_pile_card(d,side,PILE_DECK,state,0);++moved;}shuffle(d,p);if(moved)event(d,DUEL_EVENT_RETURN,side,0,0,moved);return moved;}
int duel_target_present(const Duel *d,unsigned side,unsigned index,uint32_t serial){return d&&side<2&&index<5&&d->players[side].monsters[index].card&&d->players[side].monsters[index].serial==serial;}

static int grave_index(const DuelPlayer *p,uint32_t serial){for(unsigned i=0;i<p->grave_count;++i)if(p->grave_serial[i]==serial)return i;return -1;}
static int backup_target(const Duel *d,unsigned card){const DuelCardDef *c=&d->cards[card];return c->family==DUEL_MONSTER&&(c->subtype==0||(c->subtype==2&&c->effect==DUEL_E_NONE))&&c->attack<=1500;}
unsigned duel_backup_options(const Duel *d,unsigned player,DuelChoiceOption *options,unsigned capacity){
    if(!d||player>1)return 0;const DuelPlayer *p=&d->players[player];unsigned count=0,total=0;
    for(unsigned i=0;i<p->grave_count;++i)total+=d->cards[p->grave[i]].family==DUEL_MONSTER;
    if(total<5)return 0;
    for(unsigned i=0;i<p->grave_count;++i)if(backup_target(d,p->grave[i])&&(d->cards[p->grave[i]].subtype==2?p->fusion_count<DUEL_FUSION_MAX:p->hand_count<DUEL_HAND_MAX)){if(options&&count<capacity)options[count]=(DuelChoiceOption){p->grave[i],i,DUEL_OPTION_CARD};++count;}return count;
}
enum {LEAVE_DESTROY,LEAVE_BATTLE,LEAVE_TRIBUTE,LEAVE_FUSION,LEAVE_REPLACE,LEAVE_NEGATE,LEAVE_RETURN,LEAVE_BANISH,LEAVE_RITUAL,LEAVE_SEND};
#include "duel_binding.inc"
static unsigned source_departure_live(const Duel *d,unsigned card){return !(d->cards[card].name_group==1160?equip_effect_negated(d,card):duel_field_effect_negated(d,card));}
static unsigned live_linked_traps(const Duel *d){unsigned mask=0;for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].backrow[i];if(s->card&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].effect==DUEL_E_CALL_HAUNTED&&!duel_field_effect_negated(d,s->card))mask|=1u<<(side*5+i);}return mask;}
static void detach_monster_state(Duel *d,uint32_t serial,unsigned destroyed,unsigned live_traps){
    if(!serial)return;binding_detach(d,serial,destroyed);stat_link_clear_target(d,serial);
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].backrow[i];if(s->card&&s->effect_active&&s->link_serial==serial){unsigned effect=d->cards[s->card].effect;if(effect==DUEL_E_CALL_HAUNTED||s->equip_role||equip_rule(d,s->card)){s->link_serial=0;if(s->equip_role||equip_rule(d,s->card)||(destroyed&&(live_traps&(1u<<(side*5+i)))))leave_field(d,side,i,1,LEAVE_DESTROY);}}}
}
static void detach_monster(Duel *d,uint32_t serial,unsigned destroyed){detach_monster_state(d,serial,destroyed,live_linked_traps(d));}
static void linked_source_left(Duel *d,const DuelSlot *source,unsigned was_live,unsigned cause,unsigned controller){if(source->card&&d->cards[source->card].name_group==1160&&cause!=LEAVE_DESTROY)return;
    if(!was_live||!source->card||!source->effect_active||source->position==DUEL_FACEDOWN_DEFENSE||d->cards[source->card].effect!=DUEL_E_CALL_HAUNTED||!source->link_serial)return;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].monsters[i];if(s->card&&s->serial==source->link_serial&&s->position!=DUEL_FACEDOWN_DEFENSE){unsigned saved=d->effect_actor_plus1;d->effect_actor_plus1=controller+1;leave_field(d,side,i,0,LEAVE_DESTROY);d->effect_actor_plus1=saved;return;}}
}
static void queue_trigger(Duel *d,unsigned kind,unsigned player,unsigned card,uint32_t serial){
    /* These are original activations, using the physical source copy. Paid
     * revival jobs, upkeep costs and timer indices have different semantics. */
    unsigned activation=kind==DUEL_CHOICE_SEARCH_ATTACK||kind==DUEL_CHOICE_SEARCH_DEFENSE||kind==DUEL_CHOICE_NIMBLE_RECOVER||kind==DUEL_TRIGGER_RECRUIT||kind==DUEL_TRIGGER_SWARM||kind==DUEL_TRIGGER_GRAVE_AUTO||kind==DUEL_TRIGGER_FLIP_AUTO||kind==DUEL_TRIGGER_MORPH2||kind==DUEL_CHOICE_PENGUIN||kind==DUEL_CHOICE_GRAVE_RECALL||kind==DUEL_CHOICE_FLIP_TARGET||kind==DUEL_CHOICE_TARGET_FLIP||kind==DUEL_CHOICE_EQUIP_RECYCLE||kind==DUEL_TRIGGER_EQUIP_GRAVE;
    if(activation&&activation_blocked_for_slot(d,card,serial))return;
    if(d->trigger_count<DUEL_TRIGGER_MAX)d->triggers[d->trigger_count++]=(DuelTrigger){kind,player,card,serial};
}
static void toon_world_destroyed(Duel *d,unsigned card){if(d->cards[card].name_group!=954)return;for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&toon_card(&d->cards[s->card])){if(!s->serial)s->serial=++d->next_serial;queue_trigger(d,DUEL_TRIGGER_BATTLE_AUTO,side,s->card,s->serial);}}}
static void record_field_leave(Duel *d,unsigned controller,unsigned slot,unsigned backrow,unsigned cause,const DuelSlot *old,unsigned was_live,unsigned live_traps){uint16_t card=old->card;unsigned owner=old->owner,before=d->players[owner].grave_count,source_blocked=activation_blocked_for_slot(d,card,old->serial);DuelCardState state=slot_card_state(old);state.history=(state.history&3)|grave_effect_history(d,controller);if(cause==LEAVE_TRIBUTE)state.history=old->card_history&3;if(cause==LEAVE_FUSION)state.history=DUEL_HISTORY_FUSION;if(cause==LEAVE_RITUAL)state.history=DUEL_HISTORY_RITUAL;if(cause==LEAVE_NEGATE){state.proper=0;grave_card_state(d,owner,state);}else grave_field_state(d,owner,state);event(d,cause==LEAVE_REPLACE?DUEL_EVENT_FIELD_REPLACE:DUEL_EVENT_DESTROY,controller,slot,card,0);
    /* Leaving the field queues mandatory effects; resolution waits until the
     * complete command (battle, tribute summon, or spell) has finished. */
    unsigned effect=d->cards[card].effect,entered_grave=d->players[owner].grave_count>before;if(entered_grave&&cause!=LEAVE_NEGATE)reactive_field_move(d,controller,backrow,old,14,d->effect_actor_plus1?d->effect_actor_plus1-1:controller);
    if(entered_grave&&!duel_is_token(d,card)&&owner==controller&&cause!=LEAVE_NEGATE&&(!backrow||cause==LEAVE_DESTROY))d->field_to_grave[controller]=1;
    if(entered_grave){uint8_t *h=&d->players[owner].grave_history[before];*h&=~DUEL_HISTORY_OPPONENT;if(cause==LEAVE_BATTLE)*h|=DUEL_HISTORY_BATTLE;if(cause==LEAVE_TRIBUTE)*h=old->card_history&3;if(cause==LEAVE_FUSION)*h=DUEL_HISTORY_FUSION;if(cause==LEAVE_RITUAL)*h=DUEL_HISTORY_RITUAL;if((cause==LEAVE_DESTROY||cause==LEAVE_NEGATE||cause==LEAVE_SEND)&&d->effect_actor_plus1&&d->effect_actor_plus1!=controller+1)*h|=DUEL_HISTORY_OPPONENT;}
    if(!backrow&&cause!=LEAVE_NEGATE&&!source_blocked&&((entered_grave&&d->cards[card].name_group==1116&&old->control_used)||(d->cards[card].name_group==1241&&(cause==LEAVE_DESTROY||cause==LEAVE_BATTLE))))queue_trigger(d,DUEL_CHOICE_GRAVE_PAIR,d->cards[card].name_group==1116?owner:controller,card,old->serial);
    if(entered_grave&&!backrow&&!source_blocked&&owner==controller&&d->cards[card].name_group==1514&&(cause==LEAVE_BATTLE||cause==LEAVE_DESTROY&&d->effect_actor_plus1&&d->effect_actor_plus1!=controller+1))necro_schedule(d,owner,card,old->serial);
    if(entered_grave&&!backrow&&cause==LEAVE_BATTLE&&!source_blocked&&d->cards[card].name_group==1405)queue_trigger(d,DUEL_CHOICE_REVIVAL_JAM,owner,card,old->serial);
    if(entered_grave&&!backrow&&cause!=LEAVE_NEGATE&&d->cards[card].name_group==1257)queue_trigger(d,DUEL_CHOICE_DECK_EQUIP,owner,card,old->serial);
    if(entered_grave&&!backrow&&cause!=LEAVE_NEGATE&&d->cards[card].name_group==461&&old->control_used)queue_trigger(d,DUEL_TRIGGER_GRAVE_AUTO,controller,card,old->serial);
    if(entered_grave&&cause==LEAVE_BATTLE&&d->cards[card].name_group==1044)queue_trigger(d,DUEL_TRIGGER_BATTLE_AUTO,controller,card,old->serial);
    if(entered_grave&&cause!=LEAVE_NEGATE&&(effect==DUEL_E_SANGAN||effect==DUEL_E_WITCH))queue_trigger(d,effect==DUEL_E_SANGAN?DUEL_CHOICE_SEARCH_ATTACK:DUEL_CHOICE_SEARCH_DEFENSE,owner,card,old->serial);
    if(entered_grave&&!backrow&&effect==DUEL_E_NIMBLE&&cause==LEAVE_BATTLE)queue_trigger(d,DUEL_CHOICE_NIMBLE_RECOVER,owner,card,old->serial);
    if(entered_grave&&!backrow&&effect==DUEL_E_RECRUIT&&cause==LEAVE_BATTLE)queue_trigger(d,DUEL_TRIGGER_RECRUIT,owner,card,old->serial);
    if(entered_grave&&!backrow&&effect==DUEL_E_SWARM&&swarm_rule(d,card)->trigger==1&&cause==LEAVE_BATTLE)queue_trigger(d,DUEL_TRIGGER_SWARM,owner,card,old->serial);
    if(!backrow)binding_detach(d,old->serial,cause==LEAVE_DESTROY||cause==LEAVE_BATTLE);
    if(backrow){if(cause==LEAVE_DESTROY)toon_world_destroyed(d,card);linked_source_left(d,old,was_live,cause,controller);}else detach_monster_state(d,old->serial,old->position!=DUEL_FACEDOWN_DEFENSE&&(cause==LEAVE_DESTROY||cause==LEAVE_BATTLE),live_traps);
}
static void leave_field(Duel *d,unsigned controller,unsigned slot,unsigned backrow,unsigned cause){if(cause==LEAVE_DESTROY&&fake_trap_shields(d,controller,slot,backrow))return;DuelSlot *s=backrow?spell_zone(&d->players[controller],slot):&d->players[controller].monsters[slot];if(!s->card)return;DuelSlot old=*s;unsigned destiny=backrow?duel_destiny_departure_mask(d,controller,slot,old.card):0;unsigned live_traps=backrow?0:live_linked_traps(d),was_live=source_departure_live(d,old.card);clear(s,sizeof(*s));record_field_leave(d,controller,slot,backrow,cause,&old,was_live,live_traps);destiny_clear_departures(d,controller,destiny);}
static void destroy(Duel *d,unsigned controller,unsigned slot,unsigned backrow){leave_field(d,controller,slot,backrow,LEAVE_DESTROY);}
static void destroy_monsters_mask(Duel *d,unsigned mask){
    struct {DuelSlot source;uint8_t side,index;} moved[10];unsigned count=0,live_traps=live_linked_traps(d);
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].monsters[i];if(!(mask&(1u<<(side*5+i)))||!s->card)continue;moved[count].source=*s;moved[count].side=side;moved[count].index=i;++count;}
    for(unsigned i=0;i<count;++i)clear(&d->players[moved[i].side].monsters[moved[i].index],sizeof(DuelSlot));
    for(unsigned i=0;i<count;++i)record_field_leave(d,moved[i].side,moved[i].index,0,LEAVE_DESTROY,&moved[i].source,1,live_traps);
}
static void destroy_monsters(Duel *d,unsigned sides){destroy_monsters_mask(d,((sides&1)?31u:0)|((sides&2)?31u<<5:0));}
static void effect_destroy_monster(Duel *d,unsigned card,unsigned side,unsigned index){if(effect_affects_monster(d,card,side,index))destroy(d,side,index,0);}
/* Moving several spell/trap cards is simultaneous. Freeze continuous source
 * liveness before removing any negator, then apply the association effects. */
static void move_backrows_mask(Duel *d,unsigned mask,unsigned returning,uint32_t except_serial){if(!returning)mask=fake_trap_mask(d,mask);
    struct {DuelSlot source;uint8_t side,index,was_live;} moved[12];unsigned count=0,destiny[2]={0,0},room[2]={DUEL_HAND_MAX-d->players[0].hand_count,DUEL_HAND_MAX-d->players[1].hand_count};
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<=DUEL_FIELD_SLOT;++i){DuelSlot *s=spell_zone(&d->players[side],i);if(!(mask&(1u<<(side*6+i)))||!s->card||(except_serial&&s->serial==except_serial)||(returning&&!duel_is_token(d,s->card)&&!room[s->owner]))continue;destiny[side]|=duel_destiny_departure_mask(d,side,i,s->card);moved[count].source=*s;moved[count].side=side;moved[count].index=i;moved[count].was_live=source_departure_live(d,s->card);if(returning&&!duel_is_token(d,s->card))--room[s->owner];++count;}
    for(unsigned i=0;i<count;++i)clear(spell_zone(&d->players[moved[i].side],moved[i].index),sizeof(DuelSlot));
    for(unsigned i=0;i<count;++i){DuelSlot *old=&moved[i].source;if(returning){DuelPlayer *owner=&d->players[old->owner];if(!duel_is_token(d,old->card))put_pile_card(d,old->owner,PILE_HAND,card_state_reset(slot_card_state(old)),0);}else{unsigned before=d->players[old->owner].grave_count;DuelCardState state=slot_card_state(old);state.history=(state.history&3)|grave_effect_history(d,moved[i].side);grave_field_state(d,old->owner,state);if(d->players[old->owner].grave_count>before)reactive_field_move(d,moved[i].side,1,old,14,d->effect_actor_plus1?d->effect_actor_plus1-1:moved[i].side);if(d->players[old->owner].grave_count>before&&old->owner==moved[i].side)d->field_to_grave[old->owner]=1;if(d->players[old->owner].grave_count>before)d->players[old->owner].grave_history[before]=(d->players[old->owner].grave_history[before]&~DUEL_HISTORY_OPPONENT)|((d->effect_actor_plus1&&d->effect_actor_plus1!=moved[i].side+1)?DUEL_HISTORY_OPPONENT:0);event(d,DUEL_EVENT_DESTROY,moved[i].side,moved[i].index,old->card,0);}}
    for(unsigned i=0;i<count;++i){if(!returning)toon_world_destroyed(d,moved[i].source.card);linked_source_left(d,&moved[i].source,moved[i].was_live,returning?LEAVE_RETURN:LEAVE_DESTROY,moved[i].side);}
    for(unsigned side=0;side<2;++side)destiny_clear_departures(d,side,destiny[side]);
}
static void move_backrows(Duel *d,unsigned sides,unsigned returning,uint32_t except_serial){move_backrows_mask(d,((sides&1)?63u:0)|((sides&2)?63u<<6:0),returning,except_serial);}
static void destroy_field_mask(Duel *d,unsigned monster_mask,unsigned backrow_mask){backrow_mask=fake_trap_mask(d,backrow_mask);struct {DuelSlot source;uint8_t side,index,backrow,was_live;} moved[22];unsigned count=0,destiny[2]={0,0},live_traps=live_linked_traps(d);
    for(unsigned side=0;side<2;++side)for(unsigned index=0;index<11;++index){DuelSlot *s=index<5?&d->players[side].monsters[index]:spell_zone(&d->players[side],index-5);if(!s->card||!(index<5?(monster_mask&(1u<<(side*5+index))):(backrow_mask&(1u<<(side*6+index-5)))))continue;if(index>=5)destiny[side]|=duel_destiny_departure_mask(d,side,index-5,s->card);moved[count].source=*s;moved[count].side=side;moved[count].index=index<5?index:index-5;moved[count].backrow=index>=5;moved[count].was_live=source_departure_live(d,s->card);++count;}
    for(unsigned i=0;i<count;++i){DuelSlot *s=moved[i].backrow?spell_zone(&d->players[moved[i].side],moved[i].index):&d->players[moved[i].side].monsters[moved[i].index];clear(s,sizeof(*s));}
    for(unsigned i=0;i<count;++i)record_field_leave(d,moved[i].side,moved[i].index,moved[i].backrow,LEAVE_DESTROY,&moved[i].source,moved[i].was_live,live_traps);
    for(unsigned side=0;side<2;++side)destiny_clear_departures(d,side,destiny[side]);
}
static void destroy_all_field(Duel *d){destroy_field_mask(d,1023,4095);}
#include "duel_original_classification_data.h"
#include "duel_target_flip.inc"
static int targeted_flip(unsigned effect){return effect==DUEL_E_PENGUIN||effect==DUEL_E_HANE_HANE||effect==DUEL_E_MAN_EATER_BUG;}
static int flip_card(const Duel *d,unsigned card){unsigned effect=d->cards[card].effect;const SwarmRule *r=effect==DUEL_E_SWARM?swarm_rule(d,card):0;return targeted_flip(effect)||effect==DUEL_E_MORPH2||effect==DUEL_E_GRAVE_RECALL||effect==DUEL_E_FLIP||(effect==DUEL_E_TARGET_FLIP&&(target_flip_rule(d,card)->op<=5||target_flip_rule(d,card)->op==9||target_flip_rule(d,card)->op==10||target_flip_rule(d,card)->op==11||target_flip_rule(d,card)->op==16))||(r&&r->trigger==0);}
static int flip_target_choice(const Duel *d){return d->choice.kind==DUEL_CHOICE_PENGUIN||d->choice.kind==DUEL_CHOICE_FLIP_TARGET;}
static void flip_trigger(Duel *d,unsigned controller,unsigned slot){DuelSlot *s=&d->players[controller].monsters[slot];unsigned effect=d->cards[s->card].effect;if(d->cards[s->card].name_group==640&&d->phase==DUEL_BATTLE)return;
    if(live_backrow_name(d,1530,3))return;
    if(d->cards[s->card].name_group==731&&controller==d->active&&d->phase==DUEL_STANDBY&&!jigen_ready(d,controller,slot,s->serial))return;
    if(d->cards[s->card].name_group==582&&!weather_gate(d,controller))return;
    if(d->cards[s->card].name_group==1512){if(!summon_procedure_illusion_gate(d,controller,slot))return;unsigned before=d->trigger_count;queue_trigger(d,DUEL_TRIGGER_FLIP_AUTO,controller,s->card,s->serial);if(d->trigger_count>before)d->triggers[before].selected_count=slot;return;}
    if(d->cards[s->card].name_group==1524&&history_target_options(d,controller,1524,0,0)<2)return;
    if(d->cards[s->card].name_group==1116){if(activation_blocked_for_slot(d,s->card,s->serial))return;s->control_used=1;}
    if(d->cards[s->card].name_group==461||d->cards[s->card].name_group==1254)s->control_used=1;
    if(flip_card(d,s->card)){unsigned before=d->trigger_count;queue_trigger(d,effect==DUEL_E_TARGET_FLIP?DUEL_CHOICE_TARGET_FLIP:effect==DUEL_E_SWARM?DUEL_TRIGGER_SWARM:effect==DUEL_E_FLIP?DUEL_TRIGGER_FLIP_AUTO:effect==DUEL_E_MORPH2?DUEL_TRIGGER_MORPH2:effect==DUEL_E_PENGUIN?DUEL_CHOICE_PENGUIN:effect==DUEL_E_GRAVE_RECALL?DUEL_CHOICE_GRAVE_RECALL:DUEL_CHOICE_FLIP_TARGET,controller,s->card,s->serial);if(d->trigger_count>before&&(d->cards[s->card].name_group==82||d->cards[s->card].name_group==731||d->cards[s->card].name_group==762))d->triggers[before].selected_count=slot;}}
static int battle_destroyed(const Duel *d,unsigned player,unsigned slot){for(unsigned i=0;i<d->battle_destroy_count;++i){const DuelTarget *t=&d->battle_destroy[i];if(t->player==player&&t->slot==slot&&t->serial==d->players[player].monsters[slot].serial)return 1;}return 0;}
static int flip_battle_bonus(const Duel *d,const DuelSlot *s){return (s->position!=DUEL_FACEDOWN_DEFENSE?s->battle_curse:0)+(d->cards[s->card].name_group==1254&&s->control_used&&!live_backrow_name(d,1530,3)?500:0);}
static int mark_battle_destroy(Duel *d,unsigned player,unsigned slot,unsigned opposing_attack,unsigned opposing){if((opposing&&d->battle_protect_turn[player]==d->turn)||(duel_battle_protection(d,player,slot,opposing_attack)&1)||absorption_battle_protect(d,player,slot))return 0;d->battle_destroy[d->battle_destroy_count++]=(DuelTarget){d->players[player].monsters[slot].serial,player,slot};return 1;}
static void finish_battle(Duel *d){duel_battle_revival_capture(d);queen_capture_battle(d);sword_capture_battle(d);
    for(unsigned i=0;i<d->battle_destroy_count;++i){const DuelTarget *t=&d->battle_destroy[i];DuelSlot *s=&d->players[t->player].monsters[t->slot];if(s->card&&s->serial==t->serial)leave_field(d,t->player,t->slot,0,LEAVE_BATTLE);}
    for(unsigned n=0;n<d->battle_equipment_destroy_count;++n){const DuelTarget *t=&d->battle_equipment_destroy[n];DuelSlot *s=&d->players[t->player].backrow[t->slot];if(s->card&&s->serial==t->serial){unsigned actor=d->effect_actor_plus1;d->effect_actor_plus1=2-t->player;destroy(d,t->player,t->slot,1);d->effect_actor_plus1=actor;}}
    /* These effect deaths occur after the ordinary battle deaths, preserving
       grave triggers and the original target instance even if its Equip left. */
    duel_punch_finish(d);
    for(unsigned n=0;n<d->battle_effect_destroy_count;++n)for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];if(s->card&&s->serial==d->battle_effect_destroy[n].serial)destroy(d,side,i,0);}
    battle_revival_commit(d);big_shield_battle_end(d);d->battle_destroy_count=d->battle_pending=0;damage_state_clear(d);
}
static void return_monster(Duel *d,const DuelTarget *target){
    DuelSlot *s=&d->players[target->player].monsters[target->slot];if(!s->card||s->serial!=target->serial||battle_destroyed(d,target->player,target->slot))return;
    if(duel_is_token(d,s->card)){unsigned card=s->card;uint32_t serial=s->serial;clear(s,sizeof(*s));detach_monster(d,serial,0);event(d,DUEL_EVENT_RETURN,target->player,target->slot,card,0);return;}DuelPlayer *owner=&d->players[s->owner];unsigned fusion=d->cards[s->card].subtype==2;if(fusion?owner->fusion_count>=DUEL_FUSION_MAX:owner->hand_count>=DUEL_HAND_MAX)return;uint16_t card=s->card;uint32_t serial=s->serial;DuelCardState returned=card_state_reset(slot_card_state(s));put_pile_card(d,s->owner,PILE_HAND,returned,0);if(!fusion)reactive_field_move(d,target->player,0,s,11,d->effect_actor_plus1?d->effect_actor_plus1-1:target->player);clear(s,sizeof(*s));detach_monster(d,serial,0);event(d,DUEL_EVENT_RETURN,target->player,target->slot,card,0);
}
static int trigger_source(const Duel *d,const DuelTrigger *trigger){
    const DuelPlayer *p=&d->players[trigger->player];for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card==trigger->card&&s->serial==trigger->serial&&s->position!=DUEL_FACEDOWN_DEFENSE&&s->effect_active)return i;}return -1;
}
static void revival_triggers(Duel *d,unsigned grave_owner){
    const DuelPlayer *p=&d->players[grave_owner];for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].effect==DUEL_E_SAFE_RETURN)queue_trigger(d,DUEL_CHOICE_OPTIONAL_DRAW,grave_owner,s->card,s->serial);}
}
static unsigned draw_one(Duel *d,unsigned player){DuelPlayer *p=&d->players[player];if(!p->deck_count){d->deck_failure_mask|=1u<<player;return 0;}if(p->hand_count>=DUEL_HAND_MAX){d->deck_failure_mask|=1u<<player;return 0;}DuelCardState state=take_pile_card(p,PILE_DECK,p->deck_count-1);unsigned card=state.card;put_pile_card(d,player,PILE_HAND,state,0);event(d,DUEL_EVENT_DRAW,player,0,card,0);crush_draw(d,player,p->hand_count-1);if(d->cards[card].name_group==762&&p->hand_count)parasite_draw(d,player,p->hand_count-1);return card;}
unsigned duel_draw_cards(Duel *d,unsigned player,unsigned count,unsigned filter){if(!d||player>1)return 0;unsigned drawn=0,suppressed=0;for(unsigned i=0;i<count&&d->winner<0&&!(d->deck_failure_mask&(1u<<player));++i){unsigned card=draw_one(d,player);drawn+=card!=0;if(card&&filter&&d->cards[card].family==DUEL_SPELL){discard_hand_unpaid(d,player,d->players[player].hand_count-1,1-player,1);suppressed=1;}}if(drawn&&d->winner<0&&!(d->deck_failure_mask&(1u<<player))){reactive_draw_batch(d,player,suppressed);periodic_queue(d,2,player);}return drawn;}
static void draw_cards(Duel *d,unsigned player,unsigned count){duel_draw_cards(d,player,count,0);}
#include "duel_mushroom.inc"
#include "duel_tribute.inc"
#include "duel_magnet.inc"
#include "duel_guard.inc"
#include "duel_tokens.inc"
#include "duel_equipment_end.inc"
#include "duel_attack_equipment.inc"
#include "duel_periodic.inc"
#include "duel_upkeep.inc"
#include "duel_recycle_equip.inc"
#include "duel_monster_equipment.inc"
#include "duel_hand.inc"
#include "duel_counter.inc"
#include "duel_peek.inc"
#include "duel_standby_peek.inc"
#include "duel_flip.inc"
#include "duel_battle_events.inc"
#include "duel_destiny.inc"
#include "duel_banish.inc"
#include "duel_temporary_removal.inc"
#include "duel_spirit_makiu.inc"
#include "duel_destruction_choices.inc"
#include "duel_doomed_dark.inc"
#include "duel_hand_selection.inc"
#include "duel_recovery_yado.inc"
#include "duel_turn_status.inc"
#include "duel_arrival_bind.inc"
#include "duel_zombie_castle.inc"
#include "duel_flip_observers.inc"
#include "duel_battle_departure.inc"
#include "duel_battle_revival.inc"
#include "duel_battle_damage_followups.inc"
#include "duel_fairy_box.inc"
/* Controller-change observers belong to the common movement, rather than
 * a separate activation of each individual monster. */
static void control_change_effects(Duel *d,unsigned old,unsigned now,DuelSlot *s){if(old==now||!s->card||s->position==DUEL_FACEDOWN_DEFENSE||s->control_used)return;unsigned name=d->cards[s->card].name_group;if(name==483||name==546){s->control_used=1;if(name==483)damage(d,now,2000);else recover(d,old,3000,s->card);}}
static void change_controller(Duel *d,unsigned old,unsigned index,unsigned now,unsigned destination,unsigned returning){DuelSlot moved=d->players[old].monsters[index];clear(&d->players[old].monsters[index],sizeof(DuelSlot));DuelSlot *s=&d->players[now].monsters[destination];*s=moved;s->return_controller=returning;control_change_effects(d,old,now,s);}
#include "duel_control_layers.inc"
static int main_phase(const Duel *d){return d->phase==DUEL_MAIN1||d->phase==DUEL_MAIN2;}
int duel_card_supported(const DuelCardDef *card){return card->effect!=DUEL_E_UNSUPPORTED;}
#include "duel_equipment_transfer.inc"
#include "duel_kunai.inc"
#include "duel_deck_evolution.inc"
#include "duel_special_hand.inc"
#include "duel_hand_set.inc"
#include "duel_search.inc"
#include "duel_deck_equipment.inc"
static int revivable(const Duel *d,unsigned card){return d->cards[card].family==DUEL_MONSTER&&d->cards[card].race<21&&d->cards[card].subtype<2&&!special_hand_provenance(&d->cards[card])&&duel_card_supported(&d->cards[card]);}
int duel_can_revive_for(const Duel *d,unsigned actor,unsigned side,unsigned index){if(!d||actor>1||side>1||index>=d->players[side].grave_count)return 0;const DuelPlayer *p=&d->players[side];const DuelCardDef *c=&d->cards[p->grave[index]];if(!p->grave[index]||c->family!=DUEL_MONSTER||c->race>=21)return 0;if(toon_card(c)&&!toon_world_present(d,actor))return 0;return revivable(d,p->grave[index])||(c->family==DUEL_MONSTER&&(c->subtype>=2||special_hand_provenance(c))&&duel_card_supported(c)&&p->grave_proper[index]);}
int duel_can_revive(const Duel *d,unsigned side,unsigned index){return duel_can_revive_for(d,side,side,index);}
#include "duel_excavation.inc"
#include "duel_major_riot.inc"
#include "duel_borrow_hats.inc"
#include "duel_passive_journal.inc"
#include "duel_reactive_field.inc"
#include "duel_reactive_hand.inc"
static void resolve_morph2(Duel *d,unsigned source){
    DuelSlot returning[10];unsigned count=0;uint32_t sequence=d->remodel.sequence+1;clear(&d->remodel,sizeof(d->remodel));d->remodel.sequence=sequence;d->remodel.source_card=source;
    /* Clear the complete returning set before removing any associations. A
     * battle-doomed monster is still on the field, but cannot return. */
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].monsters[i];if(s->card&&!battle_destroyed(d,side,i)){returning[count++]=*s;clear(s,sizeof(*s));}}
    for(unsigned i=0;i<count;++i){DuelSlot *s=&returning[i];DuelPlayer *p=&d->players[s->owner];if(duel_is_token(d,s->card)){detach_monster(d,s->serial,0);continue;}DuelCardState returned=card_state_reset(slot_card_state(s));put_pile_card(d,s->owner,PILE_DECK,returned,0);if(d->cards[s->card].subtype==2)++d->remodel.returned_fusion[s->owner];else ++d->remodel.returned[s->owner];detach_monster(d,s->serial,0);}
    for(unsigned side=0;side<2;++side){DuelPlayer *p=&d->players[side];unsigned remaining=d->remodel.returned[side];if(remaining)shuffle(d,p);
        while(remaining&&p->deck_count){DuelCardState revealed=take_pile_card(p,PILE_DECK,p->deck_count-1);unsigned card=revealed.card;unsigned at=d->remodel.revealed[side]++;d->remodel.cards[side][at]=card;const DuelCardDef *def=&d->cards[card];
            if(def->family==DUEL_MONSTER)--remaining;
            int slot=empty_zone(p->monsters);
            if(def->level<=4&&revivable(d,card)&&slot>=0&&special_summon_allowed_for(d,side)){DuelSlot *s=&p->monsters[slot];s->card=card;s->owner=side;s->position=DUEL_FACEDOWN_DEFENSE;s->summoned_turn=s->set_turn=d->turn;s->properly_summoned=1;s->return_controller=DUEL_NO_SLOT;slot_install_state(d,s,revealed);s->properly_summoned=1;d->remodel.deployed[side][at]=slot+1;++d->remodel.summoned[side];special_summon_success(d,side,slot);
                /* Face-down Special Summons still have a summon response
                 * outside the damage step. They never trigger flip effects. */
                if(!d->battle_pending)d->summon_notice=(DuelWindow){DUEL_WINDOW_SUMMON,side,slot,0,s->serial,0,card,0,2};
            }else grave_deck_state(d,side,revealed,d->effect_actor_plus1?d->effect_actor_plus1-1:side,1);
        }
    }
    event(d,DUEL_EVENT_REMODEL,0,0,source,count);
}
static unsigned formation(const Duel *d,unsigned side,unsigned source){unsigned total=0,legal=0;for(unsigned i=0;i<5;++i){total+=d->players[1-side].monsters[i].card!=0;legal+=duel_can_attack_target(d,side,source,i);}return total|(legal<<8);}
static const DuelFusionRecipe *fusion_recipe(const Duel *d,unsigned card){for(unsigned i=0;i<d->fusion_recipe_count;++i){const DuelFusionRecipe *r=&d->fusion_recipes[i];if(r->result==card||(d->cards[r->result].name_group&&d->cards[r->result].name_group==d->cards[card].name_group))return r;}return 0;}
static unsigned material_card(const Duel *d,unsigned side,unsigned token){const DuelPlayer *p=&d->players[side];return token<DUEL_HAND_MAX?(token<p->hand_count?p->hand[token]:0):(token<DUEL_HAND_MAX+5?p->monsters[token-DUEL_HAND_MAX].card:0);}
static int fusion_assignment(const Duel *d,unsigned side,const DuelFusionRecipe *r,const uint8_t *chosen,unsigned n,unsigned depth,uint8_t *used,unsigned substitutes,unsigned field_count,unsigned excluded_hand){
    if(depth==r->count)return empty_zone(d->players[side].monsters)>=0||field_count;
    for(unsigned token=0;token<DUEL_HAND_MAX+5;++token){if(depth<n&&token!=chosen[depth])continue;if(token==excluded_hand)continue;unsigned duplicate=0;for(unsigned i=0;i<depth;++i)duplicate|=used[i]==token;if(duplicate)continue;
        unsigned card=material_card(d,side,token);if(!card)continue;const DuelCardDef *c=&d->cards[card];if(c->family!=DUEL_MONSTER||!duel_card_supported(c))continue;uint32_t serial=token<DUEL_HAND_MAX?d->players[side].hand_state[token].serial:d->players[side].monsters[token-DUEL_HAND_MAX].serial;if(duel_card_prohibited(d,card,serial))continue;unsigned sub=c->effect==DUEL_E_FUSION_SUBSTITUTE;
        if(substitutes+sub>1||(!sub&&c->name_group!=r->materials[depth]))continue;
        used[depth]=token;if(fusion_assignment(d,side,r,chosen,n,depth+1,used,substitutes+sub,field_count+(token>=DUEL_HAND_MAX),excluded_hand))return 1;
    }return 0;
}
static int fusion_possible(const Duel *d,unsigned side,unsigned card,const uint8_t *chosen,unsigned n,unsigned excluded_hand){const DuelFusionRecipe *r=fusion_recipe(d,card);uint8_t used[3];return special_summon_allowed_for(d,side)&&r&&r->count>=2&&r->count<=3&&duel_card_supported(&d->cards[card])&&fusion_assignment(d,side,r,chosen,n,0,used,0,0,excluded_hand);}
static unsigned fusion_options(const Duel *d,unsigned side,DuelChoiceOption *options,unsigned capacity,unsigned excluded_hand){unsigned count=0;const DuelPlayer *p=&d->players[side];for(unsigned i=0;i<p->fusion_count;++i)if(fusion_possible(d,side,p->fusion[i],0,0,excluded_hand)){if(options&&count<capacity)options[count]=(DuelChoiceOption){p->fusion[i],i,DUEL_OPTION_CARD};++count;}return count;}
static void finish_fusion(Duel *d,unsigned side,unsigned position){if(!special_summon_allowed_for(d,side))return;
    DuelPlayer *p=&d->players[side];DuelFusionTask task=d->fusion_task;unsigned card=p->fusion[task.index];
    for(unsigned i=p->hand_count;i>0;--i)for(unsigned j=0;j<task.count;++j)if(task.selected[j]==i-1){DuelCardState material=take_pile_card(p,PILE_HAND,i-1);if(task.banish){material.history=DUEL_HISTORY_RITUAL;put_pile_card(d,material.owner,PILE_BANISHED,material,0);event(d,DUEL_EVENT_BANISH,side,i-1,material.card,0);}else grave_material_state(d,side,material,DUEL_HISTORY_FUSION);}
    for(unsigned i=0;i<task.count;++i)if(task.selected[i]>=DUEL_HAND_MAX){if(task.banish){DuelSlot *material=&p->monsters[task.selected[i]-DUEL_HAND_MAX];unsigned owner=material->owner;uint32_t serial=material->serial;if(duel_banish_field(d,side,task.selected[i]-DUEL_HAND_MAX,0)){int index=banished_index(&d->players[owner],serial);if(index>=0)d->players[owner].banished_history[index]=DUEL_HISTORY_RITUAL;}}else leave_field(d,side,task.selected[i]-DUEL_HAND_MAX,0,LEAVE_FUSION);}
    int slot=empty_zone(p->monsters);if(slot<0)return;DuelCardState fusion_state=take_pile_card(p,PILE_FUSION,task.index);DuelSlot *s=&p->monsters[slot];clear(s,sizeof(*s));s->card=card;s->owner=side;s->position=position;s->summoned_turn=d->turn;s->properly_summoned=1;s->return_controller=DUEL_NO_SLOT;slot_install_state(d,s,fusion_state);s->properly_summoned=1;special_summon_success(d,side,slot);
    d->summon_notice=(DuelWindow){DUEL_WINDOW_SUMMON,side,slot,0,s->serial,0,card,0,2};event(d,DUEL_EVENT_FUSION,side,slot,card,task.count);
}
int duel_fusion_setup(Duel *d,const DuelFusionRecipe *recipes,unsigned count,const uint16_t *one,unsigned n1,const uint16_t *two,unsigned n2){
    if(!d||(!recipes&&count)||n1>DUEL_FUSION_MAX||n2>DUEL_FUSION_MAX||(!one&&n1)||(!two&&n2)||d->turn!=1||d->chain_count||d->choice.kind||monsters(&d->players[0])||monsters(&d->players[1]))return DUEL_BAD_ARGUMENT;
    for(unsigned side=0;side<2;++side){const uint16_t *cards=side?two:one;unsigned n=side?n2:n1;for(unsigned i=0;i<n;++i)if(!card_valid(d,cards[i])||d->cards[cards[i]].family!=DUEL_MONSTER||d->cards[cards[i]].subtype!=2||!duel_card_supported(&d->cards[cards[i]]))return DUEL_UNSUPPORTED;}
    d->fusion_recipes=recipes;d->fusion_recipe_count=count;for(unsigned side=0;side<2;++side){DuelPlayer *p=&d->players[side];clear(p->fusion,sizeof(p->fusion));clear(p->fusion_state,sizeof(p->fusion_state));p->fusion_count=side?n2:n1;for(unsigned i=0;i<p->fusion_count;++i){p->fusion[i]=side?two[i]:one[i];p->fusion_state[i]=(DuelCardState){++d->next_serial,p->fusion[i],0,0,side};}}return DUEL_OK;
}
#include "duel_ritual.inc"
static int attack_direct_base(const Duel *d,unsigned side,unsigned slot){
    if(!d||side>1||slot>=5)return 0;const DuelSlot *a=&d->players[side].monsters[slot];if(!a->card)return 0;unsigned effect=d->cards[a->card].effect;const DuelPlayer *enemy=&d->players[1-side];if(toon_card(&d->cards[a->card]))return !toon_visible(d,1-side);if(monster_abilities_for(&d->cards[a->card])&MONSTER_DIRECT)return 1;unsigned blocking=0;for(unsigned i=0;i<5;++i){const DuelSlot *s=&enemy->monsters[i];if(s->card&&!(d->cards[s->card].name_group==1326&&s->position!=DUEL_FACEDOWN_DEFENSE&&continuous_has_umi(d)))++blocking;}if(!blocking)return effect!=DUEL_E_ZOMBYRA;if(effect!=DUEL_E_ALLIGATOR_DRAGON)return 0;
    unsigned visible=0;for(unsigned i=0;i<5;++i){const DuelSlot *s=&enemy->monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE){unsigned attribute=duel_attribute(d,1-side,i);++visible;if(attribute!=3&&attribute!=4&&attribute!=5)return 0;}}return visible!=0;
}
int duel_can_attack_direct(const Duel *d,unsigned side,unsigned slot){return d&&side<2&&slot<5&&!magnet_forces_target(d,side,slot)&&attack_direct_base(d,side,slot);}
int duel_attack_value(const Duel *d,unsigned player,unsigned index){if(player>1||index>=5)return 0;const DuelSlot *s=&d->players[player].monsters[index];if(!s->card)return 0;if(s->position==DUEL_FACEDOWN_DEFENSE)return d->cards[s->card].attack;int original=original_stat(d,player,index,0);int value=castle_bonus_since(d,player,index,-1)+binding_stat_bonus(d,player,index)+flip_battle_bonus(d,s)+s->attack_bonus+s->temporary_bonus+s->temporary_attack+(s->lasting_attack_until>=d->turn?s->lasting_attack:0)+equip_bonus(d,player,index,0)+field_bonus(d,player,index,0)+aura_bonus(d,player,index)+continuous_stat_bonus(d,player,index);
    value+=monster_stat_bonus(d,player,index,0)+battle_aura_bonus(d,player,index);if(d->cards[s->card].effect==DUEL_E_BUSTER){const DuelPlayer *enemy=&d->players[1-player];
        for(unsigned i=0;i<5;++i)if(enemy->monsters[i].card&&enemy->monsters[i].position!=DUEL_FACEDOWN_DEFENSE&&duel_race(d,1-player,i)==1)value+=500;
        for(unsigned i=0;i<enemy->grave_count;++i)if(d->cards[enemy->grave[i]].family==DUEL_MONSTER&&d->cards[enemy->grave[i]].race==1)value+=500;
    }return final_attack_modifiers(d,player,index,original+(d->reverse_turn==d->turn?-value:value));}
static int goddess_unordered_bonus(const Duel *d,unsigned side,unsigned index){const DuelSlot *s=&d->players[side].monsters[index];int value=flip_battle_bonus(d,s)+s->attack_bonus+s->temporary_bonus+s->temporary_attack+(s->lasting_attack_until>=d->turn?s->lasting_attack:0)+aura_bonus(d,side,index)+continuous_stat_bonus(d,side,index)+monster_stat_bonus(d,side,index,0)+battle_aura_bonus(d,side,index);if(d->cards[s->card].effect==DUEL_E_BUSTER){const DuelPlayer *enemy=&d->players[1-side];for(unsigned i=0;i<5;++i)if(enemy->monsters[i].card&&enemy->monsters[i].position!=DUEL_FACEDOWN_DEFENSE&&duel_race(d,1-side,i)==1)value+=500;for(unsigned i=0;i<enemy->grave_count;++i)if(d->cards[enemy->grave[i]].family==DUEL_MONSTER&&d->cards[enemy->grave[i]].race==1)value+=500;}return value;}
int duel_defense_value(const Duel *d,unsigned player,unsigned index){if(player>1||index>=5)return 0;const DuelSlot *s=&d->players[player].monsters[index];if(!s->card)return 0;if(s->position==DUEL_FACEDOWN_DEFENSE)return d->cards[s->card].defense;int original=original_stat(d,player,index,1);int reset=fairy_last_reset(d,player,index);if(reset>=0&&s->swap_until==d->turn&&s->swap_parity)original=equipment_original_stat_since(d,player,index,0,reset);int value=castle_bonus_since(d,player,index,-1)+flip_battle_bonus(d,s)+s->defense_bonus+s->temporary_bonus+s->temporary_defense+equip_bonus(d,player,index,1)+field_bonus(d,player,index,1)+monster_stat_bonus(d,player,index,1)+continuous_stat_bonus(d,player,index);value=original+(d->reverse_turn==d->turn?-value:value);return value>0?value:0;}

/* Damage-calculation-only changes occur after ordinary field stats. The
 * original halves Kinetic Soldier's shared bonus once per Mirror Wall even
 * for DEF; preserve that behavior, including its signed integer rounding. */
static int combat_mirror_delta(const Duel *d,const DuelSlot *s,int bonus){if(!bonus)return 0;for(unsigned n=0;n<d->stat_link_count;++n){const DuelStatLink *l=&d->stat_links[n];const DuelSlot *source;if(l->kind==1&&l->target==s->serial&&(source=stat_link_source(d,l))&&source->effect_active&&source->position!=DUEL_FACEDOWN_DEFENSE&&!duel_field_effect_negated(d,source->card))bonus=(bonus*5+5)/10;}return bonus;}
static int combat_bonus(const Duel *d,unsigned side,unsigned index,unsigned other_side,unsigned other,unsigned attacking){const DuelSlot *s=&d->players[side].monsters[index];unsigned name=d->cards[s->card].name_group;const DuelCardDef *opponent=&d->cards[d->players[other_side].monsters[other].card];int bonus=name==1253&&duel_race(d,other_side,other)==15?2000:name==477&&attacking&&duel_attribute(d,other_side,other)==6?1000:0;if(d->reverse_turn==d->turn)bonus=-bonus;return combat_mirror_delta(d,s,bonus);}
int duel_combat_values_for(const Duel *d,unsigned side,unsigned attacker,unsigned target_side,unsigned target,int values[4]){if(!d||side>1||target_side>1||attacker>=5||target>=5||!values||!d->players[side].monsters[attacker].card||!d->players[target_side].monsters[target].card)return 0;values[0]=duel_attack_value(d,side,attacker);values[1]=duel_defense_value(d,side,attacker);values[2]=duel_attack_value(d,target_side,target);values[3]=duel_defense_value(d,target_side,target);for(unsigned i=0;i<2;++i){unsigned owner=i?target_side:side,opposing=i?side:target_side,index=i?target:attacker,other=i?attacker:target;int bonus=combat_bonus(d,owner,index,opposing,other,!i);values[i*2]+=bonus;if(d->cards[d->players[owner].monsters[index].card].name_group==1253)values[i*2+1]+=bonus;}unsigned meta_count=equipment_status_count(d,side,attacker,16);if(meta_count){int meta=(values[2]*5+5)/10*(int)meta_count;values[0]+=combat_mirror_delta(d,&d->players[side].monsters[attacker],meta);}for(unsigned i=0;i<4;++i)if(values[i]<0)values[i]=0;if(d->cards[d->players[target_side].monsters[target].card].name_group==287&&duel_attribute(d,side,attacker)==1)values[3]=(values[3]+1)/2;if(d->damage_zero_serial&&d->players[side].monsters[attacker].serial==d->damage_zero_serial)values[0]=0;return 1;}
int duel_combat_values(const Duel *d,unsigned side,unsigned attacker,unsigned target,int values[4]){return duel_combat_values_for(d,side,attacker,1-side,target,values);}
#include "duel_damage.inc"
#include "duel_tribute_control_zones.inc"
#include "duel_attack_tribute.inc"
static void begin_turn(Duel *d){
    d->battle_attacker[0]=d->battle_attacker[1]=0;
    DuelPlayer *p=&d->players[d->active];p->normal_summon_used=0;d->summon_activity[0]=d->summon_activity[1]=0;d->main1_action_used=0;if(!d->skip_next_turn[d->active]&&!p->skip_next_draw&&d->cold_wave_until&&d->active==d->cold_wave_actor&&d->turn>=d->cold_wave_until)d->cold_wave_until=0;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){d->players[side].monsters[i].normal_summon_marker=0;d->players[side].monsters[i].attacked=0;d->players[side].monsters[i].position_changed=0;if(d->players[side].monsters[i].card&&d->cards[d->players[side].monsters[i].card].name_group==761)d->players[side].monsters[i].control_used=0;}
    if(d->skip_next_turn[d->active]){d->skip_next_turn[d->active]=0;p->skip_next_draw=0;p->skip_next_standby=0;d->phase=DUEL_END;event(d,DUEL_EVENT_PHASE,d->active,0,0,DUEL_END);finish_turn(d);return;}
    if(p->skip_next_draw){p->skip_next_draw=0;if(p->skip_next_standby){p->skip_next_standby=0;d->phase=DUEL_MAIN1;}else {d->phase=DUEL_STANDBY;periodic_queue(d,1,d->active);}}else {d->phase=DUEL_DRAW;draw_cards(d,d->active,1);}
}
int duel_start(Duel *d,const DuelCardDef *defs,const uint16_t *one,unsigned n1,const uint16_t *two,unsigned n2,uint32_t seed,unsigned first){
    if(!d||!defs||!one||!two||n1<40||n1>DUEL_DECK_MAX||n2<40||n2>DUEL_DECK_MAX||first>1)return DUEL_BAD_ARGUMENT;
    for(unsigned p=0;p<2;++p){const uint16_t *deck=p?two:one;unsigned n=p?n2:n1;for(unsigned i=0;i<n;++i){if(!deck[i]||deck[i]>=DUEL_CARD_COUNT||defs[deck[i]].race>=23||(defs[deck[i]].name_group>=1920&&defs[deck[i]].name_group<=1923))return DUEL_BAD_ARGUMENT;if(defs[deck[i]].family==DUEL_MONSTER&&defs[deck[i]].subtype==2)return DUEL_BAD_ARGUMENT;if(defs[deck[i]].family==DUEL_MONSTER&&!duel_card_supported(&defs[deck[i]]))return DUEL_UNSUPPORTED;}}
    clear(d,sizeof(*d));d->cards=defs;d->random_state=seed?seed:0x9e3779b9u;d->winner=-1;d->first=d->active=first;d->turn=1;d->response_player=DUEL_NO_SLOT;
    for(unsigned side=0;side<2;++side){DuelPlayer *p=&d->players[side];const uint16_t *deck=side?two:one;p->deck_count=side?n2:n1;p->life=8000;p->pile_side=side;
        for(unsigned i=0;i<p->deck_count;++i){p->deck[i]=deck[i];p->deck_state[i]=(DuelCardState){++d->next_serial,deck[i],0,0,side};}
        shuffle(d,p);
        draw_cards(d,side,5);
    }
    /* The original's historical rules give the first player a draw too. */
    begin_turn(d);check_special_victory(d);return DUEL_OK;
}

/* Original normal/Flip success hooks do not run for an embedded Special
 * Summon. The newly arriving Puppeteer does not count its own arrival. */
static void normal_summon_events(Duel *d,const DuelWindow *w){if(w->formation>=2)return;arrival_choice_trigger(d,w->player,w->source,0);DuelSlot *arriving=&d->players[w->player].monsters[w->source];if(arriving->card&&d->cards[arriving->card].name_group==615){arriving->control_used=1;arriving->hourglass_turn=(d->turn-1)&127;}if(arriving->card&&(monster_abilities_for(&d->cards[arriving->card])&MONSTER_DEFENSE_ATTACK))arriving->position=DUEL_FACEUP_DEFENSE;for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];if(side==w->player&&i==w->source)continue;if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&(monster_abilities_for(&d->cards[s->card])&64))recover(d,side,500,s->card);}if(w->formation==0){DuelSlot *s=&d->players[w->player].monsters[w->source];if(s->card&&s->serial==w->serial&&(monster_abilities_for(&d->cards[s->card])&128))destroy(d,w->player,w->source,0);}}
unsigned duel_normal_tribute_cost(const Duel *d,unsigned side,unsigned index,unsigned set){if(!d||side>1||index>=d->players[side].hand_count)return 0;const DuelPlayer *p=&d->players[side];const DuelCardDef *c=&d->cards[p->hand[index]];if(!set&&c->name_group==1250&&p->hand_count==1&&empty_zone(p->monsters)>=0)return 0;return c->level>=7?2:c->level>=5?1:0;}
static void finish_summon_window(Duel *d,const DuelWindow *attempt){if(attempt->negated||attempt->player>1||attempt->source>=5)return;DuelSlot *s=&d->players[attempt->player].monsters[attempt->source];if(!s->card||s->serial!=attempt->serial)return;summon_restrictions(d,attempt->player,attempt->source);if(attempt->formation==2){s->properly_summoned=1;special_summon_success(d,attempt->player,attempt->source);}d->window=*attempt;d->window.kind=DUEL_WINDOW_SUMMON;if(attempt->formation<2)s->normal_summon_marker=1;normal_summon_events(d,attempt);if(!s->card||s->serial!=attempt->serial){clear(&d->window,sizeof(d->window));return;}unsigned flipped=attempt->formation==1;if(flipped)flip_trigger(d,attempt->player,attempt->source);unsigned search=attempt->formation<2?summon_search_trigger(d,attempt->player,attempt->source):0;if(!search&&!(flipped&&flip_card(d,s->card)))begin_response(d,1-attempt->player);}
static int normal_summon_check(const Duel *d,const DuelCommand *c,unsigned extra){
    if(!duel_summon_permission(d,c->player,c->action==DUEL_ACTION_SET_MONSTER?DUEL_PERMISSION_SET:DUEL_PERMISSION_NORMAL))return DUEL_NOT_ALLOWED;
    const DuelPlayer *p=&d->players[c->player];if(!extra&&!main_phase(d))return DUEL_WRONG_PHASE;if(c->source>=p->hand_count||c->target>=5)return DUEL_BAD_ARGUMENT;
    unsigned card=p->hand[c->source];const DuelCardDef *def=&d->cards[card];if(def->family!=DUEL_MONSTER||def->race>=21)return DUEL_NOT_ALLOWED;
    if(!duel_card_supported(def))return DUEL_UNSUPPORTED;
    if(duel_card_prohibited(d,card,0)||(c->action==DUEL_ACTION_SET_MONSTER&&live_backrow_name(d,1151,3)&&live_monster_name(d,1526,3)))return DUEL_NOT_ALLOWED;
    if(special_hand_provenance(def))return DUEL_NOT_ALLOWED;
    if(def->subtype>=2)return DUEL_NOT_ALLOWED; /* Ritual/fusion need their own legal procedure. */
    if(!extra&&p->normal_summon_used)return DUEL_SUMMON_USED;
    unsigned need=duel_normal_tribute_cost(d,c->player,c->source,c->action==DUEL_ACTION_SET_MONSTER),count=0;
    if(need&&continuous_count(d,5))return DUEL_NOT_ALLOWED;
    if(!can_pay_action(d,c->player,hand_play_cost(d)))return DUEL_NOT_ALLOWED;
    if(c->tributes&~1023u)return DUEL_BAD_ARGUMENT;
    for(unsigned token=0;token<10;++token)if(c->tributes&(1u<<token)){if(!duel_can_normal_tribute(d,c->player,tribute_owner(c->player,token),token%5))return DUEL_BAD_TARGET;++count;}
    if(count!=need)return DUEL_NEED_TRIBUTES;
    if(p->monsters[c->target].zone_blocked||(p->monsters[c->target].card&&!(c->tributes&(1u<<c->target))))return DUEL_ZONE_FULL;
    return DUEL_OK;
}
static int summon_context(Duel *d,const DuelCommand *c,unsigned extra){int valid=normal_summon_check(d,c,extra);if(valid!=DUEL_OK)return valid;DuelPlayer *p=&d->players[c->player];unsigned card=p->hand[c->source];
    pay_action(d,c->player,hand_play_cost(d));
    tribute_pay_mask(d,c->player,c->tributes);
    DuelCardState played=take_pile_card(p,PILE_HAND,c->source);DuelSlot *slot=&p->monsters[c->target];clear(slot,sizeof(*slot));slot->card=card;slot->owner=c->player;slot->summoned_turn=d->turn;slot->set_turn=d->turn;
    unsigned revealed_set=c->action==DUEL_ACTION_SET_MONSTER&&live_backrow_name(d,1151,3);slot->position=c->action==DUEL_ACTION_SET_MONSTER?(revealed_set?DUEL_FACEUP_DEFENSE:DUEL_FACEDOWN_DEFENSE):DUEL_FACEUP_ATTACK;slot->properly_summoned=1;slot->return_controller=DUEL_NO_SLOT;slot_install_state(d,slot,played);if(!extra)p->normal_summon_used=1;
    event(d,c->action==DUEL_ACTION_SET_MONSTER&&!revealed_set?DUEL_EVENT_SET:DUEL_EVENT_SUMMON,c->player,c->target,card,0);
    if(extra){DuelWindow notice={c->action==DUEL_ACTION_SET_MONSTER&&!revealed_set?DUEL_WINDOW_SET:DUEL_WINDOW_SUMMON,c->player,c->target,0,slot->serial,0,card,0,0};if(notice.kind==DUEL_WINDOW_SUMMON){slot->normal_summon_marker=1;summon_restrictions(d,c->player,c->target);normal_summon_events(d,&notice);if(slot->card&&slot->serial==notice.serial)summon_search_trigger(d,c->player,c->target);else notice.kind=0;}d->summon_notice=notice;return DUEL_OK;}
    if(c->action==DUEL_ACTION_SET_MONSTER&&!revealed_set){d->window=(DuelWindow){DUEL_WINDOW_SET,c->player,c->target,0,slot->serial,0,card};begin_response(d,1-c->player);}
    if(c->action==DUEL_ACTION_SUMMON||revealed_set){d->window=(DuelWindow){DUEL_WINDOW_SUMMON_NEGATION,c->player,c->target,0,slot->serial,0,card,0};begin_response(d,1-c->player);}return DUEL_OK;
}
static int summon(Duel *d,const DuelCommand *c){return summon_context(d,c,0);}
static int battle(Duel *d,const DuelCommand *c){
    if(d->phase!=DUEL_BATTLE||d->turn==1||d->battle_skip_turn[c->player]==d->turn)return DUEL_WRONG_PHASE;if(c->source>=5||c->target_player!=1-c->player)return DUEL_BAD_TARGET;
    DuelSlot *a=&d->players[c->player].monsters[c->source];if(!a->card||duel_card_prohibited(d,a->card,a->serial)||!combat_attack_position(d,a)||a->attacked>=duel_attack_limit(d,c->player,c->source)||!continuous_attack_allowed(d,c->player,c->source))return DUEL_NOT_ALLOWED;
    if(!servant_ready(d,c->player,c->source)&&!(d->attack_tribute.stage==3&&c->cost==1))return DUEL_NOT_ALLOWED;
    unsigned tribute=duel_attack_tribute_required(&d->cards[a->card]),paid=tribute&&d->attack_tribute.stage==3&&d->attack_tribute.serial==a->serial&&d->attack_tribute.command.player==c->player&&c->cost==1;
    if(tribute&&!paid){if(!can_pay_action(d,c->player,attack_play_cost(d,c->player,c->source)))return DUEL_NOT_ALLOWED;return attack_tribute_begin(d,c);}
    DuelPlayer *enemy=&d->players[1-c->player];
    if(c->target==DUEL_NO_SLOT){if(!duel_can_attack_direct(d,c->player,c->source))return DUEL_NOT_ALLOWED;}
    else if(!duel_can_attack_target(d,c->player,c->source,c->target))return DUEL_BAD_TARGET;
    unsigned cost=attack_play_cost(d,c->player,c->source);if(!can_pay_action(d,c->player,cost))return DUEL_NOT_ALLOWED;if(d->cards[a->card].name_group==366&&c->cost!=1){d->coin_attack=*c;d->choice=(DuelTrigger){DUEL_CHOICE_ATTACK_COIN,c->player,a->card,a->serial};return DUEL_OK;}if(d->cards[a->card].name_group==366&&c->position>1)return DUEL_BAD_ARGUMENT;if(d->cards[a->card].name_group==366){unsigned roll=random_next(d)&1;d->random_result=(DuelRandomResult){d->random_result.sequence+1,a->card,c->player,1,roll,c->position,roll==c->position};if(roll!=c->position)pay_action(d,c->player,d->players[c->player].life/2);}if(paid)clear(&d->attack_tribute,sizeof(d->attack_tribute));servant_pay(d,c->player,c->source);if(cost>=(unsigned)d->players[c->player].life){pay_action(d,c->player,d->players[c->player].life);check_victory_boundary(d);return DUEL_OK;}pay_action(d,c->player,cost);
    if(!a->serial)a->serial=++d->next_serial;
    unsigned target_serial=0;if(c->target<5){DuelSlot *b=&enemy->monsters[c->target];if(!b->serial)b->serial=++d->next_serial;target_serial=b->serial;}
    ++a->attacked;mirror_attach(d,c->player,c->source);if(!d->battle_attacker[c->player])d->battle_attacker[c->player]=a->serial;d->window=(DuelWindow){DUEL_WINDOW_ATTACK,c->player,c->source,c->target,a->serial,target_serial,a->card,0,formation(d,c->player,c->source)};++d->revision;fairy_attack_begin(d);return DUEL_OK;
}
static void battle_damage(Duel *d,unsigned side,unsigned amount){if(d->battle_protect_turn[side]!=d->turn&&!continuous_battle_protected(d,side))damage(d,side,amount);}
static void battle_damage_by_monster(Duel *d,unsigned side,unsigned index,unsigned victim,unsigned amount){int before=d->players[victim].life;battle_damage(d,victim,amount);if(victim==1-side&&d->players[victim].life<before)battle_damage_events(d,side,index,before-d->players[victim].life);}
static void battle_damage_channels(Duel *d,unsigned mask,unsigned amount){for(unsigned side=0;side<2;++side)if((mask&(1u<<side))&&d->battle_protect_turn[side]!=d->turn&&!continuous_battle_protected(d,side)){unsigned actual=amount>(unsigned)d->players[side].life?d->players[side].life:amount;d->players[side].life-=actual;if(actual)d->reactive_pending.damage|=1u<<side;if(actual)event(d,DUEL_EVENT_DAMAGE,side,0,0,actual);}check_victory_boundary(d);}
static void battle_after_damage(Duel *d,const DuelWindow *w){if(w->target>=5)return;DuelSlot *a=&d->players[w->player].monsters[w->source],*b=&d->players[duel_battle_target_player(w)].monsters[w->target];if(!a->card||!b->card||a->serial!=w->serial||b->serial!=w->target_serial)return;
    duel_battle_departure_events(d,w);
    if(d->cards[a->card].name_group==1231&&d->active==w->player&&d->phase==DUEL_BATTLE){b->temporary_attack-=500;fairy_stat_delta(d,b,-500);}
    if(d->cards[a->card].name_group==409&&duel_attribute(d,duel_battle_target_player(w),w->target)==2&&!battle_destroyed(d,duel_battle_target_player(w),w->target))queue_trigger(d,DUEL_TRIGGER_BATTLE_AUTO,w->player,a->card,b->serial);
    if(d->cards[b->card].name_group==609&&duel_race(d,w->player,w->source)!=2&&!battle_destroyed(d,w->player,w->source)){if(a->no_attack_until<d->turn+2)a->no_attack_until=d->turn+2;}
    for(unsigned host=0;host<2;++host){unsigned side=host?duel_battle_target_player(w):w->player,opposing=host?w->player:duel_battle_target_player(w),index=host?w->target:w->source,other_index=host?w->source:w->target;DuelSlot *other=&d->players[opposing].monsters[other_index];if(duel_race(d,opposing,other_index)==1&&equipment_status_mask(d,side,index,8)&&!battle_destroyed(d,opposing,other_index)&&d->battle_effect_destroy_count<2)d->battle_effect_destroy[d->battle_effect_destroy_count++]=(DuelTarget){other->serial,opposing,other_index};}
    for(unsigned dead=0;dead<2;++dead){unsigned owner=dead?duel_battle_target_player(w):w->player,index=dead?w->target:w->source;DuelSlot *fallen=&d->players[owner].monsters[index],*killer=dead?a:b;unsigned name=d->cards[fallen->card].name_group;if(battle_destroyed(d,owner,index)&&(name==1254||name==1255||name==1256)){killer->battle_curse-=500;fairy_stat_delta(d,killer,-500);}}
    if(d->winner<0&&d->cards[b->card].name_group==1049)queue_trigger(d,DUEL_TRIGGER_BATTLE_AUTO,duel_battle_target_player(w),b->card,a->serial);
    ++d->revision;
}
static void apply_battle_damage(Duel *d,const DuelWindow *w){unsigned actual[2]={0,0},reflected[2]={absorption_reflection(d,w,0),absorption_reflection(d,w,1)};for(unsigned side=0;side<2;++side){unsigned amount=d->damage_amount[side];if((w->target==DUEL_NO_SLOT||duel_battle_target_player(w)!=w->player)&&d->battle_protect_turn[side]==d->turn||continuous_battle_protected(d,side))amount=0;if(amount>(unsigned)d->players[side].life)amount=d->players[side].life;actual[side]=amount;d->players[side].life-=amount;if(amount)d->reactive_pending.damage|=1u<<side;if(amount)event(d,DUEL_EVENT_DAMAGE,side,0,0,amount);}for(unsigned side=0;side<2;++side)if(reflected[side]){unsigned victim=1-side,amount=reflected[side];if(amount>(unsigned)d->players[victim].life)amount=d->players[victim].life;d->players[victim].life-=amount;if(amount)d->reactive_pending.damage|=1u<<victim;event(d,DUEL_EVENT_DAMAGE,victim,0,0,amount);}check_victory_boundary(d);for(unsigned side=0;side<2;++side)if(actual[side]&&d->damage_dealer_side!=side&&(d->damage_event_mask&(1u<<side)))battle_damage_events(d,d->damage_dealer_side,d->damage_dealer_slot,actual[side]);battle_after_damage(d,w);if(w->target==DUEL_NO_SLOT&&actual[1-w->player]&&d->winner<0){d->damage_done_notice=*w;d->damage_done_notice.kind=DUEL_WINDOW_DAMAGE_DONE;d->damage_done_notice.formation=actual[1-w->player]&65535;}if(w->target==DUEL_NO_SLOT)damage_state_clear(d);}
static void begin_battle_damage(Duel *d,const DuelWindow *w,unsigned one,unsigned two,unsigned dealer,unsigned index,unsigned events){d->damage_amount[0]=one;d->damage_amount[1]=two;d->damage_dealer_side=dealer;d->damage_dealer_slot=index;d->damage_event_mask=events;for(unsigned side=0;side<2;++side)if((w->target==DUEL_NO_SLOT||duel_battle_target_player(w)!=w->player)&&d->battle_protect_turn[side]==d->turn||continuous_battle_protected(d,side))d->damage_amount[side]=0;unsigned victim=1-w->player;if(!(w->events&(1u<<29))&&d->damage_amount[victim]&&d->active==w->player&&damage_kuriboh_index(d,victim)>=0){d->damage_stage=3;d->window=*w;d->window.kind=DUEL_WINDOW_DAMAGE_LIFE;begin_response(d,victim);return;}apply_battle_damage(d,w);}

#include "duel_battle_redirect.inc"
#include "duel_forced_attack.inc"
static void resolve_battle(Duel *d,const DuelWindow *w){
    DuelSlot *a=&d->players[w->player].monsters[w->source];if(w->negated||!a->card||a->serial!=w->serial||!combat_attack_position(d,a)||(!(w->events&BATTLE_FORCED_ATTACKER)&&!continuous_attack_allowed(d,w->player,w->source))){damage_state_clear(d);return;}
    unsigned target_side=duel_battle_target_player(w),opposing=target_side!=w->player;DuelPlayer *enemy=&d->players[target_side];mirror_attach(d,w->player,w->source);
    if(w->target==DUEL_NO_SLOT){if((w->events&BATTLE_FORCED_ATTACKER)||attack_direct_base(d,w->player,w->source)){unsigned damage=duel_attack_value(d,w->player,w->source);begin_battle_damage(d,w,w->player?damage:0,w->player?0:damage,w->player,w->source,1u<<(target_side));}return;}
    DuelSlot *b=&enemy->monsters[w->target];if(!b->card&&(w->events&BATTLE_FORCED_ATTACKER)){unsigned power=duel_attack_value(d,w->player,w->source);d->battle_pending=1;d->dimensional_battle[0]=(DuelTarget){a->serial,w->player,w->source};d->dimensional_battle[1]=(DuelTarget){0,target_side,w->target};begin_battle_damage(d,w,w->player?power:0,w->player?0:power,w->player,w->source,1u<<target_side);if(d->cards[a->card].name_group==255&&!activation_blocked_for_slot(d,a->card,a->serial))queue_trigger(d,DUEL_TRIGGER_BATTLE_AUTO,w->player,a->card,a->serial);return;}if(!b->card||b->serial!=w->target_serial){damage_state_clear(d);return;}
    if(!(w->events&BATTLE_REDIRECT_FORCED)&&(duel_battle_redirect_begin(d,w)||attack_equipment_begin(d,w)))return;
    unsigned flipped=b->position==DUEL_FACEDOWN_DEFENSE;if(flipped){detach_monster(d,b->serial,0);b->position=b->facedown_attack?DUEL_FACEUP_ATTACK:DUEL_FACEUP_DEFENSE;b->facedown_attack=0;}
    if(!d->damage_stage&&!(w->events&BATTLE_REDIRECT_FORCED)&&(damage_component_matches(d,w,target_side,w->target)||punch_window_needed(d,w))){d->damage_stage=1;d->damage_revealed=flipped;d->window=*w;d->window.kind=DUEL_WINDOW_DAMAGE;begin_response(d,target_side);return;}
    big_shield_battle_begin(d,w);d->battle_pending=1;if(flipped||d->damage_revealed)flip_trigger(d,target_side,w->target);d->damage_revealed=0;
    int values[4];duel_combat_values_for(d,w->player,w->source,target_side,w->target,values);unsigned power=values[0],defense=b->position!=DUEL_FACEUP_ATTACK;int other=values[defense?3:2];unsigned amounts[2]={0,0},dealer=w->player,dealer_index=w->source,events=1u<<(target_side);
    if(power>(unsigned)other){int destroyed=mark_battle_destroy(d,target_side,w->target,power,opposing);if(destroyed&&d->cards[a->card].effect==DUEL_E_ZOMBYRA)a->attack_bonus-=200;if(!defense){if(!(duel_battle_protection(d,target_side,w->target,power)&2))amounts[target_side]=power-other;}else {unsigned sources=equipment_status_mask(d,w->player,w->source,2),victims=((sources&1)?2:0)|((sources&2)?1:0);if(monster_abilities_for(&d->cards[a->card])&MONSTER_PIERCE)victims|=1u<<(1-w->player);if(duel_battle_protection(d,w->player,w->source,values[2])&2)victims&=~(1u<<w->player);if(duel_battle_protection(d,target_side,w->target,power)&2)victims&=~(1u<<(target_side));for(unsigned side=0;side<2;++side)if(victims&(1u<<side))amounts[side]=power-other;}}
    else if(power<(unsigned)other){if(!defense)mark_battle_destroy(d,w->player,w->source,values[2],opposing);if(!(duel_battle_protection(d,w->player,w->source,values[2])&2))amounts[w->player]=other-power;dealer=target_side;dealer_index=w->target;events=1u<<w->player;}
    else if(!defense&&power){mark_battle_destroy(d,w->player,w->source,values[2],opposing);mark_battle_destroy(d,target_side,w->target,power,opposing);}
    begin_battle_damage(d,w,amounts[0],amounts[1],dealer,dealer_index,events);++d->revision;
}
static void continue_attack(Duel *d){
    DuelWindow w=d->deferred_attack;DuelSlot *a=&d->players[w.player].monsters[w.source];
    if(w.negated||!a->card||a->serial!=w.serial||!combat_attack_position(d,a)||(!(w.events&BATTLE_FORCED_ATTACKER)&&!continuous_attack_allowed(d,w.player,w.source))){clear(&d->deferred_attack,sizeof(d->deferred_attack));return;}
    if(!(w.events&BATTLE_REDIRECT_FORCED)&&(formation(d,w.player,w.source)!=w.formation||(w.target==DUEL_NO_SLOT&&!attack_direct_base(d,w.player,w.source))||(w.target<5&&(!d->players[duel_battle_target_player(&w)].monsters[w.target].card||d->players[duel_battle_target_player(&w)].monsters[w.target].serial!=w.target_serial))||magnet_replay_needed(d,&w))){d->choice=(DuelTrigger){DUEL_CHOICE_ATTACK_REPLAY,w.player,a->card,a->serial};return;}
    clear(&d->deferred_attack,sizeof(d->deferred_attack));resolve_battle(d,&w);
}
#include "duel_pile_targets.inc"
#include "duel_fusion_gate_return.inc"
#include "duel_pile_summon.inc"
#include "duel_summon_procedures.inc"
#include "duel_gila.inc"
#include "duel_time_wizard.inc"
#include "duel_optional_set.inc"
#include "duel_simple.inc"
#include "duel_chain_retarget.inc"
#include "duel_stat_snapshot.inc"
#include "duel_offering.inc"
#include "duel_continuous_abilities.inc"
static unsigned spell_speed(const DuelCardDef *def){const SimpleRule *simple=simple_rule_def(def);return simple?simple->speed:(def->effect==DUEL_E_MAGIC_JAMMER||def->effect==DUEL_E_COUNTER)?3:def->family==DUEL_TRAP||def->effect==DUEL_E_MST||def->effect==DUEL_E_GRACEFUL_DICE?2:1;}
static const DuelSlot *chain_destruction_target(const Duel *d){
    const DuelWindow *w=&d->window;if(w->kind!=DUEL_WINDOW_SUMMON||w->player>1||w->source>=5)return 0;const DuelSlot *s=&d->players[w->player].monsters[w->source];
    return s->card&&s->serial==w->serial&&s->position!=DUEL_FACEDOWN_DEFENSE&&duel_can_target_monster(d,w->player,w->source)&&duel_attack_value(d,w->player,w->source)<=2000?s:0;
}
static int same_name(const Duel *d,unsigned one,unsigned two){return one==two||d->cards[one].name_group==d->cards[two].name_group;}
static int has_backrow_target(const Duel *d,unsigned player,unsigned source,unsigned from_set){
    for(unsigned side=0;side<2;++side){if(d->players[side].field.card)return 1;for(unsigned i=0;i<5;++i)if(d->players[side].backrow[i].card&&!(from_set&&side==player&&i==source))return 1;}return 0;
}
static int response_card(const Duel *d,unsigned player,unsigned source,unsigned from_set){
    if(d->window.kind==DUEL_WINDOW_DAMAGE_LIFE)return 0;
    const DuelPlayer *p=&d->players[player];unsigned card;
    if(from_set){if(source>=DUEL_ZONES)return 0;const DuelSlot *s=&p->backrow[source];if(!s->card)return 0;if(s->position!=DUEL_FACEDOWN_DEFENSE){if(!continuous_ability_can_start(d,player,source))return 0;}else if(s->set_turn>=d->turn)return 0;card=s->card;}
    else {if(player!=d->active||source>=p->hand_count||empty_zone(p->backrow)<0||!can_pay_action(d,player,hand_play_cost(d)))return 0;card=p->hand[source];}
    const DuelCardDef *def=&d->cards[card];if(!duel_card_supported(def)||activation_blocked_for_slot(d,card,from_set?(source==DUEL_FIELD_SLOT?d->players[player].field.serial:d->players[player].backrow[source].serial):0)||(!from_set&&def->family!=DUEL_SPELL))return 0;
    if(!fragrance_allows(d,player,source,from_set,card))return 0;unsigned speed=spell_speed(def);if(d->window.kind==DUEL_WINDOW_DAMAGE&&speed<3&&def->name_group!=1535)return 0;if(d->window.kind==DUEL_WINDOW_DAMAGE_DONE&&speed<3&&def->name_group!=1179)return 0;if(speed<2||(d->chain_count&&speed<d->chain[d->chain_count-1].speed))return 0;
    if(d->window.kind==DUEL_WINDOW_SUMMON_NEGATION&&!d->chain_count){const CounterRule *r=counter_rule(d,card);if(!r||(r->op!=4&&r->op!=5))return 0;}
    if(d->battle_pending&&speed<3)return 0; /* After damage calculation. */
    switch(def->effect){
    case DUEL_E_SIMPLE:{DuelCommand c;clear(&c,sizeof(c));c.player=player;c.action=from_set?DUEL_ACTION_ACTIVATE_BACKROW:DUEL_ACTION_ACTIVATE_HAND;c.source=source;if(duel_simple_target_kind(def)){for(unsigned side=0;side<2;++side)for(unsigned index=0;index<(duel_simple_target_kind(def)==2?6:5);++index){c.target_player=side;c.target=index;if(simple_validate(d,&c,card)==DUEL_OK)return 1;}return 0;}return simple_rule_def(def)!=0&&simple_validate(d,&c,card)==DUEL_OK;}
    case DUEL_E_EQUIP:if(def->name_group==650)return kunai_modes(d,player,card,0,0)!=0;for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(duel_equip_can_target(d,card,side,i))return 1;return 0;
    case DUEL_E_COUNTER:return counter_validate(d,player,card)==DUEL_OK;
    case DUEL_E_PERIODIC:return periodic_validate(d,player,card)==DUEL_OK;
    case DUEL_E_CONTINUOUS:if(reactive_hand_name(def->name_group)&&!reactive_hand_gate(d,player,def->name_group))return 0;if(def->name_group==1428&&!duel_jam_redirect_ready(d,player))return 0;return from_set&&p->backrow[source].position!=DUEL_FACEDOWN_DEFENSE?continuous_ability_can_start(d,player,source):continuous_validate(d,card)==DUEL_OK;
    case DUEL_E_MST:return has_backrow_target(d,player,source,from_set);
    case DUEL_E_GRACEFUL_DICE:for(unsigned i=0;i<5;++i)if(p->monsters[i].card&&p->monsters[i].position!=DUEL_FACEDOWN_DEFENSE)return 1;return 0;
    case DUEL_E_MAGIC_JAMMER:return p->hand_count&&d->chain_count&&!d->chain[d->chain_count-1].effect_activation&&d->cards[d->chain[d->chain_count-1].card].family==DUEL_SPELL&&!activation_unnegatable(d,d->chain[d->chain_count-1].card);
    case DUEL_E_TORRENTIAL:return d->window.kind==DUEL_WINDOW_SUMMON&&(monsters(p)||monsters(&d->players[1-player]));
    case DUEL_E_MAGIC_CYLINDER:{const DuelWindow *w=&d->window;if(w->kind!=DUEL_WINDOW_ATTACK||w->player==player||w->negated)return 0;const DuelSlot *s=&d->players[w->player].monsters[w->source];return s->card&&s->serial==w->serial&&combat_attack_position(d,s);}
    case DUEL_E_BACKUP:return duel_backup_options(d,player,0,0)>0;
    case DUEL_E_CHAIN_DESTRUCTION:return chain_destruction_target(d)!=0;
    case DUEL_E_BIND:for(unsigned i=0;i<5;++i)if(duel_binding_can_target(d,player,card,1-player,i))return 1;return 0;
    case DUEL_E_CALL_HAUNTED:if(!special_summon_allowed_for(d,player)||empty_zone(p->monsters)<0)return 0;for(unsigned i=0;i<p->grave_count;++i)if(duel_can_revive(d,player,i))return 1;return 0;
    case DUEL_E_TRAP_LOCK:return def->family==DUEL_TRAP&&suppression_rule(d,card)!=0;
    default:return 0;
    }
}
unsigned duel_response_options(const Duel *d,unsigned player,DuelActivationOption *options,unsigned capacity){
    if(!d||player>1||d->response_player!=player||d->winner>=0||d->chain_count>=DUEL_CHAIN_MAX)return 0;unsigned count=0;
    for(unsigned source=0;source<5;++source)if(response_card(d,player,source,1)){if(options&&count<capacity)options[count]=(DuelActivationOption){d->players[player].backrow[source].card,source,1};++count;}
    if(player==d->active)for(unsigned source=0;source<d->players[player].hand_count;++source)if(response_card(d,player,source,0)){if(options&&count<capacity)options[count]=(DuelActivationOption){d->players[player].hand[source],source,0};++count;}
    int kuriboh=damage_kuriboh_index(d,player);if(kuriboh>=0&&damage_kuriboh_response(d,player,kuriboh)){if(options&&count<capacity)options[count]=(DuelActivationOption){d->players[player].hand[kuriboh],kuriboh,0};++count;}
    for(unsigned source=0;source<5;++source)if(mary_ready(d,player,source)||damage_component_response(d,player,source)){if(options&&count<capacity)options[count]=(DuelActivationOption){d->players[player].monsters[source].card,source,2};++count;}
    return count;
}
static void begin_response(Duel *d,unsigned player){reactive_promote(d);d->response_player=player;d->response_passes=0;}
static int activate(Duel *d,const DuelCommand *c,unsigned from_set){
    DuelPlayer *p=&d->players[c->player];if((from_set&&c->source>DUEL_FIELD_SLOT)||(!from_set&&c->source>=p->hand_count))return DUEL_BAD_ARGUMENT;
    unsigned card=from_set?spell_zone(p,c->source)->card:p->hand[c->source],enemy=1-c->player;if(!card)return DUEL_BAD_ARGUMENT;
    const DuelCardDef *def=&d->cards[card];unsigned procedure_lp=def->name_group==1212?(unsigned)p->life/2:0,procedure_level=0;if(!from_set&&d->window.kind==DUEL_WINDOW_DAMAGE_LIFE)return damage_kuriboh_activate(d,c);if(!from_set&&def->family==DUEL_MONSTER&&def->name_group!=71)return activate_hand_monster(d,c);if((def->family==DUEL_MONSTER&&def->name_group!=71)||(!from_set&&def->family==DUEL_TRAP))return DUEL_NOT_ALLOWED;
    if(!duel_card_supported(def))return DUEL_UNSUPPORTED;
    if(activation_blocked_for_slot(d,card,from_set?spell_zone(&d->players[c->player],c->source)->serial:0)||!fragrance_allows(d,c->player,c->source,from_set,card))return DUEL_NOT_ALLOWED;
    if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;
    if(from_set&&c->source<5&&spell_zone(p,c->source)->position!=DUEL_FACEDOWN_DEFENSE&&continuous_ability_kind(d,card))return activate_continuous_ability(d,c);
    if(from_set&&c->source<5&&spell_zone(p,c->source)->position!=DUEL_FACEDOWN_DEFENSE&&def->name_group==651)return activate_equipment_deck_evolution(d,c);
    unsigned speed=spell_speed(def),responding=d->response_player<2;
    if(responding){if(d->response_player!=c->player)return DUEL_EFFECT_PENDING;if(!response_card(d,c->player,c->source,from_set))return DUEL_NOT_ALLOWED;}
    else {if(c->player!=d->active&&!(from_set&&def->name_group==1017&&d->phase==DUEL_BATTLE))return DUEL_WRONG_TURN;if(def->family==DUEL_TRAP&&def->effect!=DUEL_E_BACKUP&&def->effect!=DUEL_E_CALL_HAUNTED&&def->effect!=DUEL_E_TRAP_LOCK&&def->effect!=DUEL_E_SIMPLE&&def->effect!=DUEL_E_CONTINUOUS&&def->effect!=DUEL_E_PERIODIC&&def->effect!=DUEL_E_EQUIP&&def->effect!=DUEL_E_BIND)return DUEL_NOT_ALLOWED;if(speed==1&&!main_phase(d)&&!(def->name_group==1064&&d->phase==DUEL_STANDBY))return DUEL_WRONG_PHASE;}
    int source_slot=from_set?(int)c->source:def->effect==DUEL_E_FIELD?DUEL_FIELD_SLOT:empty_zone(p->backrow);if(source_slot<0)return DUEL_ZONE_FULL;
    if((source_slot==DUEL_FIELD_SLOT)!=(def->effect==DUEL_E_FIELD))return DUEL_NOT_ALLOWED;
    if(from_set&&(spell_zone(p,c->source)->position!=DUEL_FACEDOWN_DEFENSE||(speed>=2&&spell_zone(p,c->source)->set_turn>=d->turn)))return DUEL_NOT_ALLOWED;
    if(def->name_group==650&&!c->cost){if(d->bait_mode==1)return kunai_modes(d,c->player,card,0,0)?DUEL_OK:DUEL_NOT_ALLOWED;if(!kunai_modes(d,c->player,card,0,0))return DUEL_NOT_ALLOWED;d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_KUNAI_MODE,c->player,card,0};return DUEL_OK;}
    if(def->name_group==1428&&d->bait_mode!=1){int r=redirect_jam_begin(d,c,0);if(r!=DUEL_OK||d->choice.kind)return r;}
    /* Validate every target before paying a card or changing a zone. */
    int slot=-1;
    switch(def->effect){
    case DUEL_E_COUNTER:{int result=counter_validate(d,c->player,card);if(result!=DUEL_OK)return result;result=counter_validate_tribute(d,c,card);if(result!=DUEL_OK)return result;break;}
    case DUEL_E_PERIODIC:{int result=periodic_validate(d,c->player,card);if(result!=DUEL_OK)return result;break;}
    case DUEL_E_HAND:{int result=hand_validate(d,c,card,from_set);if(result!=DUEL_OK)return result;break;}
    case DUEL_E_SEARCH:if(!search_rule(d,card)||search_rule(d,card)->trigger!=0)return DUEL_NOT_ALLOWED;break;
    case DUEL_E_SIMPLE:{int result=simple_validate(d,c,card);if(result!=DUEL_OK)return result;break;}
    case DUEL_E_CONTINUOUS:{if(reactive_hand_name(def->name_group)&&!reactive_hand_gate(d,c->player,def->name_group))return DUEL_NOT_ALLOWED;if(def->name_group==1320){int r=ground_validate(d,c);if(r!=DUEL_OK)return r;}if(def->name_group==1428&&!duel_jam_redirect_ready(d,c->player))return DUEL_NOT_ALLOWED;if(def->name_group==954&&p->life<=1000+(int)(from_set?0:hand_play_cost(d)))return DUEL_NOT_ALLOWED;int result=continuous_validate(d,card);if(result!=DUEL_OK)return result;break;}
    case DUEL_E_RITUAL:if(!ritual_monsters(d,c->player,card,0,0))return DUEL_NOT_ALLOWED;break;
    case DUEL_E_TRAP_LOCK:if(!suppression_rule(d,card))return DUEL_UNSUPPORTED;break;
    case DUEL_E_FIELD:if(!field_rule(d,card))return DUEL_UNSUPPORTED;break;
    case DUEL_E_LIFE:{const DuelLifeRule *r=duel_life_rule(d,card);if(!r)return DUEL_UNSUPPORTED;if(r->target==DUEL_LIFE_EITHER&&c->target_player>1)return DUEL_BAD_TARGET;break;}
    case DUEL_E_BIND:if(!duel_binding_can_target(d,c->player,card,c->target_player,c->target))return DUEL_BAD_TARGET;break;
    case DUEL_E_EQUIP:if(def->name_group==650){if(c->cost<1||c->cost>3)return DUEL_BAD_ARGUMENT;if(c->cost!=2&&!kunai_position_available(d,c->player))return DUEL_NOT_ALLOWED;if(c->cost==1)break;}if(def->name_group==1417&&from_set&&c->target_player==c->player&&c->target==c->source)return DUEL_BAD_TARGET;if(def->name_group==71&&(from_set||p->normal_summon_used||empty_zone(p->monsters)<0))return DUEL_NOT_ALLOWED;if(def->name_group==1068&&(empty_zone(p->monsters)<0||!monsters(&d->players[enemy])))return DUEL_NOT_ALLOWED;if(!equip_rule(d,card))return DUEL_UNSUPPORTED;if(!duel_equip_can_target_for(d,c->player,card,c->target_player,c->target))return DUEL_BAD_TARGET;break;
    case DUEL_E_CALL_HAUNTED:
        if(def->name_group==1160&&p->life<=800+(int)(from_set?0:hand_play_cost(d)))return DUEL_NOT_ALLOWED;
        if(c->target_player!=c->player||c->position!=DUEL_FACEUP_ATTACK)return DUEL_BAD_TARGET;
        /* Fall through to the shared grave-instance and free-zone checks. */
    case DUEL_E_REBORN:
        if(def->effect==DUEL_E_REBORN&&continuous_count(d,38))return DUEL_NOT_ALLOWED;
        if(!special_summon_allowed_for(d,c->player))return DUEL_NOT_ALLOWED;
        if(c->target_player>1||c->target>=d->players[c->target_player].grave_count||c->position>DUEL_FACEUP_DEFENSE)return DUEL_BAD_TARGET;
        if(!duel_can_revive_for(d,c->player,c->target_player,c->target))return DUEL_BAD_TARGET;
        slot=empty_zone(p->monsters);if(slot<0)return DUEL_ZONE_FULL;break;
    case DUEL_E_MST:case DUEL_E_DE_SPELL:case DUEL_E_REMOVE_TRAP:
        if(c->target_player>1||c->target>DUEL_FIELD_SLOT||!spell_zone(&d->players[c->target_player],c->target)->card)return DUEL_BAD_TARGET;
        if(from_set&&c->target_player==c->player&&c->target==c->source)return DUEL_BAD_TARGET;
        {DuelSlot *t=spell_zone(&d->players[c->target_player],c->target);unsigned family=d->cards[t->card].family;
            if(def->effect==DUEL_E_REMOVE_TRAP&&(family!=DUEL_TRAP||t->position==DUEL_FACEDOWN_DEFENSE))return DUEL_BAD_TARGET;if(def->effect==DUEL_E_DE_SPELL&&family==DUEL_TRAP&&t->position!=DUEL_FACEDOWN_DEFENSE)return DUEL_BAD_TARGET;}
        break;
    case DUEL_E_FISSURE:
        for(unsigned i=0;i<5;++i)if(d->players[enemy].monsters[i].card&&d->players[enemy].monsters[i].position!=DUEL_FACEDOWN_DEFENSE&&(slot<0||duel_attack_value(d,enemy,i)<duel_attack_value(d,enemy,slot)))slot=i;
        if(slot<0)return DUEL_BAD_TARGET;break;
    case DUEL_E_CHANGE_HEART:
        if(c->target_player!=enemy||c->target>=5||!duel_can_target_monster(d,enemy,c->target)||!effect_affects_monster(d,card,enemy,c->target))return DUEL_BAD_TARGET;
        slot=empty_zone(p->monsters);if(slot<0)return DUEL_ZONE_FULL;break;
    case DUEL_E_TRIBUTE_DOOMED:
        if(c->cost>=p->hand_count||(!from_set&&c->cost==c->source))return DUEL_BAD_TARGET;
        if(c->target_player>1||c->target>=5||!duel_can_target_monster(d,c->target_player,c->target)||!effect_affects_monster(d,card,c->target_player,c->target))return DUEL_BAD_TARGET;break;
    case DUEL_E_DRAW2:case DUEL_E_DARK_HOLE:case DUEL_E_RAIGEKI:case DUEL_E_FEATHER_DUSTER:case DUEL_E_TRUNADE:case DUEL_E_GRACEFUL_DICE:case DUEL_E_SAFE_RETURN:break;
    case DUEL_E_POLYMERIZATION:if(!fusion_options(d,c->player,0,0,from_set?DUEL_NO_SLOT:c->source))return DUEL_NOT_ALLOWED;break;
    case DUEL_E_MAGIC_JAMMER:if(c->cost>=p->hand_count)return DUEL_BAD_TARGET;break;
    case DUEL_E_BACKUP:
        if(!duel_backup_options(d,c->player,0,0))return DUEL_NOT_ALLOWED;
        if(!c->target_count||c->target_count>3)return DUEL_BAD_TARGET;
        {unsigned hand=0,fusion=0;for(unsigned i=0;i<c->target_count;++i){if(c->targets[i]>=p->grave_count||!backup_target(d,p->grave[c->targets[i]]))return DUEL_BAD_TARGET;if(d->cards[p->grave[c->targets[i]]].subtype==2)++fusion;else ++hand;for(unsigned j=0;j<i;++j)if(c->targets[i]==c->targets[j])return DUEL_BAD_TARGET;}if(p->hand_count+hand>DUEL_HAND_MAX||p->fusion_count+fusion>DUEL_FUSION_MAX)return DUEL_BAD_TARGET;}break;
    case DUEL_E_TORRENTIAL:case DUEL_E_MAGIC_CYLINDER:break;
    case DUEL_E_CHAIN_DESTRUCTION:
        if(!chain_destruction_target(d)||c->target_player!=d->window.player||c->target!=d->window.source)return DUEL_BAD_TARGET;break;
    default:return DUEL_UNSUPPORTED;
    }
    if(!from_set){unsigned cost=hand_play_cost(d)+(def->effect==DUEL_E_HAND?hand_rule(d,card)->cost:0)+procedure_lp;if(!can_pay_action(d,c->player,cost))return DUEL_NOT_ALLOWED;}
    if(from_set&&procedure_lp&&!can_pay_action(d,c->player,procedure_lp))return DUEL_NOT_ALLOWED;
    if(d->bait_mode==1)return DUEL_OK;
    if(def->name_group==660&&!c->cost){d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_CRUSH_TRIBUTE,c->player,card,0};return DUEL_OK;}
    if((def->name_group==1213||def->name_group==1320)&&!c->cost){coordinate_begin(d,c,card);return DUEL_OK;}
    if(def->name_group==1532&&from_set&&!c->position&&lair_initial_ready(d,c->player)){d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_LAIR_OFFER,c->player,card,0};return DUEL_OK;}
    if((def->name_group==1059||def->name_group==1005)&&!c->cost){destruction_begin(d,c,card);return DUEL_OK;}
    if(def->name_group==1181&&!c->cost){stat_snapshot_begin(d,c,card);return DUEL_OK;}
    if((def->name_group==1317||def->name_group==1073)&&!c->cost)return retarget_begin(d,c,card);
    if(def->name_group==1017&&!c->cost&&offering_extra_available(d,c->player,500)){d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_OFFERING_PAY,c->player,card,0};return DUEL_OK;}
    if(def->name_group==1017&&c->cost==1){if(!can_pay_action(d,c->player,500))return DUEL_NOT_ALLOWED;pay_action(d,c->player,500);}
    if(def->name_group==1318&&!c->cost){d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_SUMMON_PROCEDURE_COST,c->player,card,0};return DUEL_OK;}
    if(def->name_group==1074&&!c->cost){d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_EQUIP_TRANSFER_SOURCE,c->player,card,0};return DUEL_OK;}
    if(def->name_group==1091&&!c->cost){d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_BORROW_TARGET,c->player,card,0};return DUEL_OK;}
    if(def->name_group==962){if(!c->cost){d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_EQUIP_STAT,c->player,card,0};return DUEL_OK;}if(c->cost!=1||c->position>1)return DUEL_BAD_ARGUMENT;}
    unsigned parameter_count=trait_parameter_count(def->name_group);if(parameter_count){if(!c->cost){d->equip_mode_command=*c;d->choice=(DuelTrigger){DUEL_CHOICE_ACTIVATION_PARAMETER,c->player,card,0};return DUEL_OK;}if(c->cost!=1||(def->name_group==1162?(!c->tributes||c->tributes>=DUEL_CARD_COUNT):(!c->position||c->position>parameter_count))||(def->name_group==1448&&c->position==duel_attribute(d,c->target_player,c->target)))return DUEL_BAD_ARGUMENT;}
    if(!from_set)pay_action(d,c->player,hand_play_cost(d));if(procedure_lp)pay_action(d,c->player,procedure_lp);if(def->name_group==1318){unsigned token=c->tributes-1;procedure_level=d->cards[d->players[token/5].monsters[token%5].card].level+1;leave_field(d,token/5,token%5,0,LEAVE_TRIBUTE);}if(def->name_group==71)p->normal_summon_used=1;
    /* The discard is a paid cost, separate from the spell's resolution. */
    if(def->effect==DUEL_E_HAND&&hand_rule(d,card)->cost){p->life-=hand_rule(d,card)->cost;event(d,DUEL_EVENT_DAMAGE,c->player,0,card,hand_rule(d,card)->cost);}
    if(def->name_group==660){unsigned t=c->tributes-1;leave_field(d,t/5,t%5,0,LEAVE_TRIBUTE);}
    if(def->effect==DUEL_E_COUNTER)counter_pay(d,c,card);if(def->effect==DUEL_E_SIMPLE&&simple_rule_def(def)->op==49)leave_field(d,c->target_player,c->target,0,LEAVE_TRIBUTE);
    if(def->name_group==1059){unsigned token=c->tributes-1;leave_field(d,token/5,token%5,0,LEAVE_TRIBUTE);}
    if(def->name_group==1160)pay_action(d,c->player,800);
    if(def->name_group==954)pay_action(d,c->player,1000);
    if(def->effect==DUEL_E_SIMPLE&&simple_rule_def(def)->op==41&&simple_rule_def(def)->other)pay_action(d,c->player,simple_rule_def(def)->other);
    unsigned source_index=c->source;
    if(def->effect==DUEL_E_TRIBUTE_DOOMED||def->effect==DUEL_E_MAGIC_JAMMER){discard_hand_effect(d,c->player,c->cost,c->player,0);if(!from_set&&c->cost<c->source)--source_index;}
    if(!from_set&&source_slot==DUEL_FIELD_SLOT&&p->field.card)leave_field(d,c->player,DUEL_FIELD_SLOT,1,LEAVE_REPLACE);
    DuelSlot *source=spell_zone(p,source_slot);
    if(!from_set){DuelCardState played=take_pile_card(p,PILE_HAND,source_index);clear(source,sizeof(*source));slot_install_state(d,source,played);}
    else if(!source->serial)source->serial=++d->next_serial;
    source->position=DUEL_FACEUP_ATTACK;source_field_reveal(d,source);if(parameter_count){if(def->name_group==1162)source->link_serial=c->tributes;else source->control_used=c->position;}
    DuelChainLink *link=&d->chain[d->chain_count];clear(link,sizeof(*link));link->command=*c;link->card=card;link->source_slot=source_slot;link->source_serial=source->serial;link->speed=speed;
    if(def->effect==DUEL_E_SIMPLE&&(simple_rule_def(def)->op==21||simple_rule_def(def)->op==24||simple_rule_def(def)->op==53)){link->command.target_player=d->window.player;link->command.target=d->window.source;link->target_serial=d->window.serial;}
    if(def->effect==DUEL_E_MST||def->effect==DUEL_E_DE_SPELL||def->effect==DUEL_E_REMOVE_TRAP||def->effect==DUEL_E_CHANGE_HEART||def->effect==DUEL_E_TRIBUTE_DOOMED||(def->effect==DUEL_E_EQUIP&&!(def->name_group==650&&c->cost==1))||def->effect==DUEL_E_BIND||(duel_simple_target_kind(def)&&!(def->effect==DUEL_E_SIMPLE&&simple_rule_def(def)->op==49))){
        unsigned backrow=def->effect==DUEL_E_MST||def->effect==DUEL_E_DE_SPELL||def->effect==DUEL_E_REMOVE_TRAP||duel_simple_target_kind(def)==2;
        DuelSlot *target=backrow?spell_zone(&d->players[c->target_player],c->target):&d->players[c->target_player].monsters[c->target];if(!target->serial)target->serial=++d->next_serial;link->target_serial=target->serial;link->target_card=target->card;
    }else if(def->effect==DUEL_E_REBORN||def->effect==DUEL_E_CALL_HAUNTED){DuelPlayer *from=&d->players[c->target_player];if(!from->grave_serial[c->target])from->grave_serial[c->target]=++d->next_serial;link->target_card=from->grave[c->target];link->target_serial=from->grave_serial[c->target];}
    if(def->effect==DUEL_E_BACKUP){link->target_count=c->target_count;for(unsigned i=0;i<c->target_count;++i){unsigned index=c->targets[i];if(!p->grave_serial[index])p->grave_serial[index]=++d->next_serial;link->targets[i]=(DuelTarget){p->grave_serial[index],c->player,index};}}
    if(def->effect==DUEL_E_CHAIN_DESTRUCTION){const DuelSlot *s=chain_destruction_target(d);link->target_card=s->card;link->target_serial=s->serial;}
    if(def->name_group==689||def->effect==DUEL_E_MAGIC_JAMMER||def->effect==DUEL_E_COUNTER||(def->effect==DUEL_E_SIMPLE&&simple_rule_def(def)->op==35))link->command.target=d->chain_count?d->chain_count-1:DUEL_NO_SLOT;
    if(def->name_group==1005){link->target_count=3;for(unsigned i=0;i<3;++i){unsigned token=c->targets[i];DuelSlot *m=&d->players[token/5].monsters[token%5];if(!m->serial)m->serial=++d->next_serial;link->targets[i]=(DuelTarget){m->serial,token/5,token%5};}}
    if(def->name_group==1181||def->name_group==1213){link->target_count=2;for(unsigned i=0;i<2;++i){unsigned token=c->targets[i];DuelSlot *m=&d->players[token/5].monsters[token%5];if(!m->serial)m->serial=++d->next_serial;link->targets[i]=(DuelTarget){m->serial,token/5,token%5};}}
    if(def->name_group==1247||def->name_group==1317||def->name_group==1073){link->command.tributes=d->chain_count;if(def->name_group!=1247){link->target_serial=d->players[c->target_player].monsters[c->target].serial;link->target_card=d->players[c->target_player].monsters[c->target].card;link->target_count=1;link->targets[0]=(DuelTarget){link->target_serial,c->target_player,c->target};link->targets[1]=(DuelTarget){d->window.target_serial,duel_battle_target_player(&d->window),d->window.target};link->targets[2]=(DuelTarget){d->window.serial,d->window.player,d->window.source};}}
    if(def->name_group==1424&&d->window.kind==DUEL_WINDOW_ATTACK&&d->window.player!=c->player){link->target_count=1;link->targets[0]=(DuelTarget){0,d->window.player,d->window.source};}
    if(def->name_group==1529||def->name_group==990){link->targets[0]=(DuelTarget){d->window.serial,d->window.player,d->window.source};link->targets[1]=(DuelTarget){d->window.target_serial,duel_battle_target_player(&d->window),d->window.target};link->target_count=2;}
    if(def->name_group==1428){DuelSlot *t=&d->players[c->target_player].monsters[c->target];if(!t->serial)t->serial=++d->next_serial;link->target_serial=t->serial;}
    if(def->name_group==1091)link->grave_targets[0]=card_state_identify(d,pile_card_state(&d->players[1-c->player],PILE_GRAVE,c->tributes-1));
    if(def->name_group==1179)link->target_card=d->window.formation&65535;
    if(def->name_group==950){link->grave_targets[0]=d->battle_last_death.victim;link->command.target_player=d->battle_last_death.controller;link->command.position=d->battle_last_death.position;}
    if(def->name_group==1318)link->target_card=procedure_level;
    if(def->name_group==650&&c->cost!=2){link->targets[0]=(DuelTarget){d->window.serial,d->window.player,d->window.source};}
    if(def->name_group==1074){unsigned token=c->tributes-1;DuelSlot *equipment=spell_zone(&d->players[token/6],token%6),*host=&d->players[c->target_player].monsters[c->target];if(!equipment->serial)equipment->serial=++d->next_serial;if(!host->serial)host->serial=++d->next_serial;link->targets[0]=(DuelTarget){equipment->serial,token/6,token%6};link->target_count=1;link->target_serial=host->serial;link->target_card=host->card;}
    ++d->chain_count;if(d->bait_mode==2)d->bait_mode=0;event(d,DUEL_EVENT_SPELL,c->player,source_slot,card,0);if(def->name_group==1532&&c->position==1)pile_target_begin(d,d->chain_count-1,43);else if(def->effect==DUEL_E_SIMPLE&&simple_rule_def(def)->op>=38&&simple_rule_def(def)->op<=40)pile_target_begin(d,d->chain_count-1,simple_rule_def(def)->op);else if(!hand_begin_cost(d,c->player,card))begin_response(d,1-c->player);return DUEL_OK;
}
#include "duel_fake_bait.inc"
#include "duel_blast.inc"
#include "duel_ignition.inc"
#include "duel_history.inc"
#include "duel_grave_pair.inc"
#include "duel_revival_jobs.inc"
#include "duel_necrofear.inc"
static void resolve_link(Duel *d,const DuelChainLink *link){
    const DuelCommand *c=&link->command;unsigned card=link->card,enemy=1-c->player;const DuelCardDef *def=&d->cards[card];DuelPlayer *p=&d->players[c->player];int slot=-1;if(def->name_group==1248)d->battle_skip_turn[c->player]=d->turn;
    if(!link->effect_activation&&def->name_group==1320){ground_resolve(d,link);return;}
    if(link->effect_activation&&def->name_group==1525){if(!link->effect_negated)counter_negate(d,link->command.target);return;}
    if(link->effect_activation&&def->name_group==1421){doomed_end_resolve(d,link);return;}
    if(link->effect_activation&&((def->name_group==1116&&c->cost==1)||def->name_group==1241)){grave_pair_resolve(d,link);return;}
    if((def->name_group==1243||def->name_group==1522||def->name_group==1428)&&link->effect_activation){duel_redirect_battle(d,link);return;}
    if(link->effect_activation&&attack_equipment_name(def->name_group)){attack_equipment_resolve(d,link);return;}
    if(link->effect_activation&&def->name_group==1514){equip_grave_resolve(d,link);return;}
    if(def->name_group==1054){if(link->effect_activation)history_will_resolve(d,link);else history_schedule(d,c->player,card);return;}
    if(link->effect_activation&&def->name_group==1547){const DuelSlot *source=&d->players[c->source].field;if(source->card==card&&!link->effect_negated&&!duel_field_effect_negated(d,card)&&fusion_options(d,c->player,0,0,DUEL_NO_SLOT)){clear(&d->fusion_task,sizeof(d->fusion_task));d->fusion_task.banish=1;d->choice=(DuelTrigger){DUEL_CHOICE_FUSION_MONSTER,c->player,card,0};}return;}
    if(link->effect_activation&&deck_evolution_rule(def)){if(def->family==DUEL_SPELL&&equip_effect_negated(d,card))return;pile_summon_begin(d,c->player,card,5);return;}
    if(!link->effect_activation&&def->effect==DUEL_E_BIND){binding_resolve(d,link);return;}
    if(!link->effect_activation&&def->name_group==1026){if(link->source_slot<5&&p->backrow[link->source_slot].card==card){p->backrow[link->source_slot].effect_active=1;if(!duel_field_effect_negated(d,card))call_dark_resolve(d,link);}return;}
    if(!link->effect_activation&&(def->effect==DUEL_E_TRAP_LOCK||def->effect==DUEL_E_CALL_HAUNTED||def->effect==DUEL_E_SAFE_RETURN||def->effect==DUEL_E_CONTINUOUS||def->effect==DUEL_E_PERIODIC)){
        DuelSlot *s=&p->backrow[link->source_slot];if(s->card!=card||s->serial!=link->source_serial||s->position==DUEL_FACEDOWN_DEFENSE)return;if(def->name_group!=1160)s->effect_active=1;
        /* Register a resolved continuous trap even while its effect is
           suppressed. It stays face-up and may resume later; a suppressed
           Call does not retroactively revive its old target. */
        if(def->effect==DUEL_E_TRAP_LOCK)return;
    }
    /* Equip and Field activations establish their attachment/zone even under
       spell-effect suppression; their live effects are queried separately. */
    if(!link->effect_activation&&def->name_group==1017){DuelSlot *source=&p->backrow[link->source_slot];if(source->card==card&&source->serial==link->source_serial&&source->position!=DUEL_FACEDOWN_DEFENSE)source->effect_active=1;offering_resolve(d,link);return;}
    if(!link->effect_activation&&def->name_group==650){kunai_resolve(d,link);return;}
    if(!link->effect_activation&&def->effect==DUEL_E_EQUIP){if(def->family!=DUEL_TRAP||!duel_field_effect_negated(d,card))attach_equip(d,link);return;}
    if(!link->effect_activation&&def->effect==DUEL_E_FIELD){DuelSlot *s=&p->field;if(s->card==card&&s->serial==link->source_serial&&s->position!=DUEL_FACEDOWN_DEFENSE){DuelSlot *other=&d->players[enemy].field;if(other->card&&other->position!=DUEL_FACEDOWN_DEFENSE)destroy(d,enemy,DUEL_FIELD_SLOT,1);s->effect_active=1;stat_field_attach(d,s);}return;}
    if(!link->effect_activation&&def->name_group==1220){soul_exchange_resolve(d,link);return;}
    /* These original callbacks apply their player restriction even when
     * the Spell effect is suppressed. Activation negation never calls us. */
    if((def->name_group==1245||def->name_group==1212)&&(link->effect_negated||duel_field_effect_negated(d,card)))summon_turn_ban(d,c->player);
    if(def->name_group==1160&&equip_effect_negated(d,card))return;
    if(def->family!=DUEL_MONSTER&&(!link->effect_activation||def->effect==DUEL_E_SAFE_RETURN||def->effect==DUEL_E_PERIODIC||def->effect==DUEL_E_CONTINUOUS)&&!activation_unnegatable(d,card)&&duel_field_effect_negated(d,card)&&!(def->name_group==1424&&link->source_slot<5&&p->backrow[link->source_slot].position==DUEL_FACEDOWN_DEFENSE))return;
    if(!link->effect_activation&&def->family==DUEL_SPELL&&big_shield_intercept(d,link))return;
    if(link->effect_activation&&!(def->name_group==461&&c->cost==1)&&flip_card(d,card)&&live_backrow_name(d,1530,3))return;
    if(link->effect_activation){
        if(def->effect==DUEL_E_PENGUIN||def->effect==DUEL_E_HANE_HANE){for(unsigned i=0;i<link->target_count;++i)return_monster(d,&link->targets[i]);}
        else if(def->effect==DUEL_E_MAN_EATER_BUG){for(unsigned i=0;i<link->target_count;++i){const DuelTarget *t=&link->targets[i];const DuelSlot *s=&d->players[t->player].monsters[t->slot];if(s->card&&s->serial==t->serial&&!battle_destroyed(d,t->player,t->slot))destroy(d,t->player,t->slot,0);}}
        else if(def->effect==DUEL_E_GRAVE_RECALL&&link->target_count){int index=grave_index(p,link->targets[0].serial),family=recall_family(d,card);
            if(index>=0&&family>=0&&p->hand_count<DUEL_HAND_MAX&&d->cards[p->grave[index]].family==family){DuelCardState returned=card_state_reset(take_pile_card(p,PILE_GRAVE,index));unsigned target=returned.card;put_pile_card(d,c->player,PILE_HAND,returned,0);event(d,DUEL_EVENT_RETURN,c->player,index,target,0);}}
        else if(def->effect==DUEL_E_MORPH2)resolve_morph2(d,card);
        else if(def->effect==DUEL_E_FLIP)flip_auto_resolve(d,link);
        else if(def->effect==DUEL_E_MONSTER){if(def->name_group==1257)deck_equipment_resolve(d,link);else if(def->name_group==1520)pile_summon_begin(d,c->player,card,6);else if(damage_component_name(def->name_group))damage_component_resolve(d,link);else battle_event_resolve(d,link);}
        else if(def->effect==DUEL_E_IGNITION)ignition_resolve(d,link);
        else if(def->effect==DUEL_E_SWARM)swarm_resolve(d,link);
        else if(def->effect==DUEL_E_TARGET_FLIP)target_flip_resolve(d,link);
        else if(def->effect==DUEL_E_SEARCH)search_resolve(d,link);
        else if(def->effect==DUEL_E_HAND)hand_resolve(d,link);
        else if(def->effect==DUEL_E_EQUIP)equip_grave_resolve(d,link);
        else if(def->effect==DUEL_E_PERIODIC){if(def->name_group==552)mushroom_resolve(d,link);else periodic_resolve(d,link);}
        else if(def->effect==DUEL_E_CONTINUOUS){if(def->name_group==1424)fairy_box_resolve(d,link);else if(def->name_group==1528)duel_destiny_resolve(d,link);else if(continuous_ability_kind(d,card))continuous_ability_resolve(d,link);else continuous_event_resolve(d,link);}
        else if(def->effect==DUEL_E_RECRUIT&&recruit_options(d,c->player,card,0,0))d->choice=(DuelTrigger){DUEL_CHOICE_RECRUIT,c->player,card,0};
        else if(def->effect==DUEL_E_SAFE_RETURN){DuelSlot *s=&p->backrow[link->source_slot];if(s->card==card&&s->serial==link->source_serial&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE)draw_cards(d,c->player,1);}return;
    }
    if(def->effect==DUEL_E_CHAIN_DESTRUCTION){
        /* PC 2004 stores the selected controller/name at activation; its
           resolver never re-reads the summoned monster or its current ATK. */
        unsigned side=c->target_player,count=0;DuelPlayer *affected=&d->players[side];
        for(unsigned i=affected->hand_count;i>0;--i)if(same_name(d,affected->hand[i-1],link->target_card)){discard_hand_effect(d,side,i-1,c->player,1);++count;}
        for(unsigned i=affected->deck_count;i>0;--i)if(same_name(d,affected->deck[i-1],link->target_card)){grave_deck_state(d,side,take_pile_card(affected,PILE_DECK,i-1),c->player,0);++count;}
        for(unsigned i=affected->fusion_count;i>0;--i)if(same_name(d,affected->fusion[i-1],link->target_card)){DuelCardState sent=take_pile_card(affected,PILE_FUSION,i-1);sent.history=0;grave_card_state(d,side,sent);++count;}
        shuffle(d,affected);event(d,DUEL_EVENT_PILE_DESTROY,side,0,link->target_card,count);return;
    }
    int grave_target=-1;
    if(def->effect==DUEL_E_FIELD){DuelSlot *s=&p->field;if(s->card==card&&s->serial==link->source_serial&&s->position!=DUEL_FACEDOWN_DEFENSE){DuelSlot *other=&d->players[enemy].field;if(other->card&&other->position!=DUEL_FACEDOWN_DEFENSE)destroy(d,enemy,DUEL_FIELD_SLOT,1);s->effect_active=1;}return;}
    if(def->effect==DUEL_E_EQUIP){attach_equip(d,link);return;}
    if(def->effect==DUEL_E_CALL_HAUNTED){DuelSlot *source=&p->backrow[link->source_slot];if(source->card!=card||source->serial!=link->source_serial||source->position==DUEL_FACEDOWN_DEFENSE)return;if(def->name_group!=1160)source->effect_active=1;}
    if(link->target_serial&&def->effect!=DUEL_E_REBORN&&def->effect!=DUEL_E_CALL_HAUNTED&&!(def->effect==DUEL_E_SIMPLE&&simple_rule_def(def)->op==45)){unsigned backrow=def->effect==DUEL_E_MST||def->effect==DUEL_E_DE_SPELL||def->effect==DUEL_E_REMOVE_TRAP||duel_simple_target_kind(def)==2;DuelSlot *target=backrow?spell_zone(&d->players[c->target_player],c->target):&d->players[c->target_player].monsters[c->target];if(backrow?(!target->card||target->serial!=link->target_serial):!duel_target_present(d,c->target_player,c->target,link->target_serial))return;}
    if(def->effect==DUEL_E_REBORN||def->effect==DUEL_E_CALL_HAUNTED){grave_target=grave_index(&d->players[c->target_player],link->target_serial);if(grave_target<0||!duel_can_revive_for(d,c->player,c->target_player,grave_target))return;slot=empty_zone(p->monsters);if(slot<0)return;}
    if(def->effect==DUEL_E_CHANGE_HEART){slot=empty_zone(p->monsters);if(slot<0)return;}
    if(def->effect==DUEL_E_FISSURE){for(unsigned i=0;i<5;++i)if(d->players[enemy].monsters[i].card&&d->players[enemy].monsters[i].position!=DUEL_FACEDOWN_DEFENSE&&(slot<0||duel_attack_value(d,enemy,i)<duel_attack_value(d,enemy,slot)))slot=i;if(slot<0)return;}
    switch(def->effect){
    case DUEL_E_COUNTER:counter_resolve(d,link);break;
    case DUEL_E_PERIODIC:{DuelSlot *s=&p->backrow[link->source_slot];if(s->card==card&&s->serial==link->source_serial)s->effect_active=1;break;}
    case DUEL_E_HAND:hand_resolve(d,link);break;
    case DUEL_E_SEARCH:search_resolve(d,link);break;
    case DUEL_E_SIMPLE:simple_resolve(d,link);break;
    case DUEL_E_CONTINUOUS:{if(def->name_group==1532&&c->cost){continuous_ability_resolve(d,link);break;}if(def->name_group==1424){fairy_box_resolve(d,link);break;}if(def->name_group==1428){duel_redirect_battle(d,link);break;}DuelSlot *s=&p->backrow[link->source_slot];if(s->card!=card||s->serial!=link->source_serial)return;s->effect_active=1;if(def->name_group==1528)duel_destiny_resolve(d,link);if(def->name_group==1162)prohibition_snapshot(d,s);if(def->name_group==1145||def->name_group==1146)s->control_used=c->position;const ContinuousRule *r=continuous_rule(d,card);if(r->op==18&&d->window.kind==DUEL_WINDOW_ATTACK)mirror_attach(d,d->window.player,d->window.source);if(r->op==16){unsigned mask=(1u<<5)|(1u<<11);move_backrows_mask(d,mask,0,0);}if(r->op==1||r->op==2){s->position_lock_until=d->turn+r->amount*2-1;if(r->op==1)for(unsigned i=0;i<5;++i){DuelSlot *m=&d->players[enemy].monsters[i];if(m->card&&m->position==DUEL_FACEDOWN_DEFENSE){detach_monster(d,m->serial,0);m->position=m->facedown_attack?DUEL_FACEUP_ATTACK:DUEL_FACEUP_DEFENSE;m->facedown_attack=0;flip_trigger(d,enemy,i);}}}break;}
    case DUEL_E_RITUAL:if(ritual_monsters(d,c->player,card,0,0)){clear(&d->ritual_task,sizeof(d->ritual_task));d->ritual_task.spell=card;d->ritual_task.level=ritual_rule(card)->level;d->choice=(DuelTrigger){DUEL_CHOICE_RITUAL_MONSTER,c->player,card,0};}break;
    case DUEL_E_POLYMERIZATION:if(fusion_options(d,c->player,0,0,DUEL_NO_SLOT)){clear(&d->fusion_task,sizeof(d->fusion_task));d->choice=(DuelTrigger){DUEL_CHOICE_FUSION_MONSTER,c->player,card,0};}break;
    case DUEL_E_LIFE:{const DuelLifeRule *r=duel_life_rule(d,card);if(!r)break;unsigned target=r->target==DUEL_LIFE_OPPONENT?enemy:r->target==DUEL_LIFE_EITHER?c->target_player:c->player;
        target=link_player(link,target);if(r->operation==DUEL_LIFE_DAMAGE)damage(d,target,r->amount);else recover(d,target,r->amount,card);
        if(r->target==DUEL_LIFE_BOTH){if(r->operation==DUEL_LIFE_DAMAGE)damage(d,1-target,r->amount);else recover(d,1-target,r->amount,card);}break;}
    case DUEL_E_DRAW2:draw_cards(d,link_player(link,c->player),2);break;
    case DUEL_E_REBORN:case DUEL_E_CALL_HAUNTED:{if(!special_summon_allowed_for(d,c->player))break;DuelCardState revived=take_pile_card(&d->players[c->target_player],PILE_GRAVE,grave_target);uint16_t target=revived.card;DuelSlot *s=&p->monsters[slot];clear(s,sizeof(*s));s->card=target;s->owner=c->target_player;s->position=c->position;s->summoned_turn=d->turn;s->return_controller=DUEL_NO_SLOT;s->properly_summoned=1;revived.history&=3;slot_install_state(d,s,revived);s->properly_summoned=1;s->reborn_origin=def->effect==DUEL_E_REBORN;special_summon_success(d,c->player,slot);
        if(def->effect==DUEL_E_CALL_HAUNTED){DuelSlot *source=&p->backrow[link->source_slot];source->link_serial=s->serial;source->effect_active=1;if(def->name_group==1160)equipment_order_attach(d,source,s);}
        d->summon_notice=(DuelWindow){DUEL_WINDOW_SUMMON,c->player,slot,0,s->serial,0,target,0,2};if(def->name_group==1160)d->summon_notice.events|=1u<<DUEL_WINDOW_EQUIP;revival_triggers(d,c->target_player);event(d,DUEL_EVENT_REVIVE,c->player,slot,target,0);break;}
    case DUEL_E_BACKUP:
        for(unsigned i=0;i<link->target_count;++i){int index=grave_index(p,link->targets[i].serial);if(index>=0&&backup_target(d,p->grave[index])){unsigned fusion=d->cards[p->grave[index]].subtype==2;if(fusion?p->fusion_count>=DUEL_FUSION_MAX:p->hand_count>=DUEL_HAND_MAX)continue;DuelCardState returned=card_state_reset(take_pile_card(p,PILE_GRAVE,index));unsigned target=returned.card;put_pile_card(d,c->player,PILE_HAND,returned,0);event(d,DUEL_EVENT_RETURN,c->player,0,target,0);}}break;
    case DUEL_E_DARK_HOLE:destroy_monsters_mask(d,effect_monster_mask(d,card,1023));break;
    case DUEL_E_RAIGEKI:destroy_monsters_mask(d,effect_monster_mask(d,card,31u<<(enemy*5)));break;
    case DUEL_E_FEATHER_DUSTER:move_backrows(d,1u<<enemy,0,0);break;
    case DUEL_E_MST:case DUEL_E_REMOVE_TRAP:destroy(d,c->target_player,c->target,1);break;
    case DUEL_E_DE_SPELL:{DuelSlot *s=spell_zone(&d->players[c->target_player],c->target);if(d->cards[s->card].family!=DUEL_TRAP)destroy(d,c->target_player,c->target,1);break;}
    case DUEL_E_FISSURE:{DuelChoiceOption options[5];unsigned count=simple_destroy_options(d,c->player,card,options,5);if(count==1)effect_destroy_monster(d,card,enemy,options[0].index);else if(count>1)d->choice=(DuelTrigger){DUEL_CHOICE_SIMPLE_DESTROY,c->player,card,link->source_serial};break;}
    case DUEL_E_TRUNADE:move_backrows(d,3,1,link->source_serial);break;
    case DUEL_E_CHANGE_HEART:control_add(d,enemy,c->target,c->player,card,0,d->turn);break;
    case DUEL_E_GRACEFUL_DICE:{unsigned roll=random_next(d)%6+1;for(unsigned i=0;i<5;++i)if(p->monsters[i].card&&p->monsters[i].position!=DUEL_FACEDOWN_DEFENSE&&effect_affects_monster(d,card,c->player,i)){p->monsters[i].temporary_bonus+=roll*100;fairy_stat_delta(d,&p->monsters[i],roll*100);}event(d,DUEL_EVENT_SPELL,c->player,0,card,roll);break;}
    case DUEL_E_TRIBUTE_DOOMED:effect_destroy_monster(d,card,c->target_player,c->target);break;
    case DUEL_E_TORRENTIAL:destroy_monsters(d,3);break;
    case DUEL_E_MAGIC_JAMMER:
        if(c->target<d->chain_count){DuelChainLink *target=&d->chain[c->target];target->negated=1;DuelSlot *s=spell_zone(&d->players[target->command.player],target->source_slot);if(s->card&&s->serial==target->source_serial)leave_field(d,target->command.player,target->source_slot,1,LEAVE_NEGATE);}break;
    case DUEL_E_MAGIC_CYLINDER:{DuelWindow *w=&d->window;DuelSlot *a=&d->players[w->player].monsters[w->source];if(!w->negated&&a->card&&a->serial==w->serial&&combat_attack_position(d,a)){w->negated=1;damage(d,w->player,duel_attack_value(d,w->player,w->source));}break;}
    case DUEL_E_SAFE_RETURN:{DuelSlot *s=&p->backrow[link->source_slot];if(s->card==card&&s->serial==link->source_serial&&s->position!=DUEL_FACEDOWN_DEFENSE)s->effect_active=1;break;}
    }
}
static void advance_resolution(Duel *d){
    while(d->winner<0&&(d->response_player<2||d->resolving)){
        if(!d->resolving){while(d->response_passes<2){if(duel_response_options(d,d->response_player,0,0))return;++d->response_passes;d->response_player=1-d->response_player;}d->response_player=DUEL_NO_SLOT;d->resolving=1;}
        while(d->chain_count){DuelChainLink *link=&d->chain[d->chain_count-1];if(!link->resolution_started){link->resolution_started=1;if(activation_unnegatable(d,link->card))link->negated=link->effect_negated=0;if(!link->negated&&(!link->effect_negated||(link->effect_activation&&d->cards[link->card].name_group==552)||d->cards[link->card].name_group==582||d->cards[link->card].name_group==1320||d->cards[link->card].name_group==1220||d->cards[link->card].name_group==1245||d->cards[link->card].name_group==1212||d->cards[link->card].name_group==1054||d->cards[link->card].name_group==198||d->cards[link->card].name_group==1514||d->cards[link->card].name_group==255||d->cards[link->card].name_group==1340||d->cards[link->card].name_group==1417||d->cards[link->card].name_group==1248||d->cards[link->card].name_group==1246||d->cards[link->card].name_group==1513||(link->effect_activation&&d->cards[link->card].name_group==1421)||attack_equipment_name(d->cards[link->card].name_group)||history_return_name(d->cards[link->card].name_group)||d->cards[link->card].effect==DUEL_E_BIND)&&d->winner<0){
                /* Optional 'when' effects and summon responses require the
                 * revival to be the last processing of this chain. */
                clear(&d->summon_notice,sizeof(d->summon_notice));unsigned count=0;
                for(unsigned i=0;i<d->trigger_count;++i)if(d->triggers[i].kind!=DUEL_CHOICE_OPTIONAL_DRAW)d->triggers[count++]=d->triggers[i];d->trigger_count=count;
                d->effect_actor_plus1=link->command.player+1;resolve_link(d,link);continuous_cleanup(d);control_reconcile(d);}}
            if(d->choice.kind)return;
            if(!link->effect_activation&&!link->negated)borrow_penalty(d,link->command.player,link->source_serial);
            if(!link->effect_activation){DuelSlot *s=spell_zone(&d->players[link->command.player],link->source_slot);unsigned effect=d->cards[link->card].effect;
                if(s->card&&s->serial==link->source_serial&&!((effect==DUEL_E_BIND||effect==DUEL_E_SAFE_RETURN||effect==DUEL_E_CALL_HAUNTED||effect==DUEL_E_EQUIP||effect==DUEL_E_FIELD||effect==DUEL_E_TRAP_LOCK||effect==DUEL_E_CONTINUOUS||effect==DUEL_E_PERIODIC)&&s->effect_active&&!link->negated)){DuelCardState state=slot_card_state(s);unsigned owner=s->owner;state.history=(state.history&3)|grave_effect_history(d,link->command.player);unsigned destiny=duel_destiny_departure_mask(d,link->command.player,link->source_slot,s->card);clear(s,sizeof(*s));destiny_clear_departures(d,link->command.player,destiny);if(link->negated){state.proper=0;grave_card_state(d,owner,state);}else grave_field_state(d,owner,state);}}if(d->cards[link->card].name_group==1054&&link->effect_activation&&link->source_serial&&link->source_serial<=d->end_timer_count&&d->end_timers[link->source_serial-1].used==2)d->end_timers[link->source_serial-1].used=0;continuous_cleanup(d);control_reconcile(d);check_victory_boundary(d);--d->chain_count;if(d->winner>=0)return;}
        d->resolving=0;d->effect_actor_plus1=0;
        clear(&d->reactive_current,sizeof(d->reactive_current));DuelWindow window=d->window;clear(&d->window,sizeof(d->window));if(window.kind==DUEL_WINDOW_ATTACK&&d->winner<0)d->deferred_attack=window;
        if(window.kind==DUEL_WINDOW_SUMMON_NEGATION&&d->winner<0)finish_summon_window(d,&window);
        if(window.kind==DUEL_WINDOW_DAMAGE&&d->winner<0){d->damage_stage=2;resolve_battle(d,&window);}
        if(window.kind==DUEL_WINDOW_DAMAGE_LIFE&&d->winner<0)apply_battle_damage(d,&window);
        if(d->summon_notice.kind&&d->winner<0){d->window=d->summon_notice;clear(&d->summon_notice,sizeof(d->summon_notice));
            unsigned optional=0;for(unsigned i=0;i<d->trigger_count;++i)optional|=d->triggers[i].kind==DUEL_CHOICE_OPTIONAL_DRAW;
            if(optional||d->trigger_count)return;begin_response(d,1-d->window.player);}
    }
}
static void finish_turn(Duel *d){
    if(d->end_prepared_turn!=d->turn){d->end_prepared_turn=d->turn;borrow_end(d);for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[d->active].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].name_group==615&&s->control_used==1&&d->turn-1>s->hourglass_turn)s->control_used=2;}cocoon_end(d,d->active);equipment_end(d,d->active);battle_countdown_end(d);stat_limiter_end(d);periodic_queue(d,5,d->active);unsigned mask=0;for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(d->players[side].monsters[i].card&&(d->players[side].monsters[i].destroy_end_turn==d->turn||(continuous_count(d,15)&&d->players[side].monsters[i].position!=DUEL_FACEDOWN_DEFENSE&&d->players[side].monsters[i].normal_summon_marker&&d->players[side].monsters[i].summoned_turn==d->turn&&d->cards[d->players[side].monsters[i].card].level<=3)))mask|=1u<<(side*5+i);if(mask)destroy_monsters_mask(d,mask);queen_end_queue(d);
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].backrow[i];if(s->card&&d->cards[s->card].effect==DUEL_E_CONTINUOUS&&s->effect_active&&s->position_lock_until&&s->position_lock_until<=d->turn)destroy(d,side,i,1);}
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].monsters[i];s->temporary_bonus=0;s->temporary_attack=s->temporary_defense=0;if(s->lasting_attack_until==d->turn){s->lasting_attack=0;s->lasting_attack_until=0;}s->swap_until=0;s->swap_parity=0;if(s->original_until==d->turn){s->original_attack_active=0;s->original_attack=0;s->original_until=0;}if(s->position_lock_until==d->turn)s->position_lock_until=0;}
    control_end_turn(d);for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i)if(d->players[owner].monsters[i].card&&d->cards[d->players[owner].monsters[i].card].name_group==1201)d->players[owner].monsters[i].control_used=0;
    }d->reverse_turn=0;d->turn_end_pending=1;
}
static unsigned duel_choice_options_inner(const Duel *d,DuelChoiceOption *options,unsigned capacity){
    if(!d||!d->choice.kind)return 0;const DuelPlayer *p=&d->players[d->choice.player];unsigned count=0;
    if(d->choice.kind==DUEL_CHOICE_MAJOR_RIOT)return riot_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_BAIT_TARGET||d->choice.kind==DUEL_CHOICE_BAIT_BACKUP)return bait_options((Duel *)d,options,capacity);
    if(excavation_choice(d->choice.kind))return excavation_options(d,options,capacity);
    if(borrow_hats_choice(d->choice.kind))return borrow_hats_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_BATTLE_REDIRECT)return redirect_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_PATROL_TARGET){unsigned n=patrol_options(d,d->choice.player,options,capacity);if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return n+1;}
    if(d->choice.kind==DUEL_CHOICE_INSPECTION_OFFER)return inspection_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_GAMBLE_COIN){for(unsigned i=0;i<2;++i)if(options&&i<capacity)options[i]=(DuelChoiceOption){d->choice.card,i,DUEL_OPTION_CONFIRM};return 2;}
    if(d->choice.kind==DUEL_CHOICE_CRUSH_TRIBUTE){unsigned n=crush_options(d,d->choice.player,options,capacity);if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return n+1;}
    if(coordinate_choice(d->choice.kind))return coordinate_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_YADO_HAND)return yado_options(d,options,capacity);
    if(hand_selection_kind(d->choice.kind))return hand_selection_options(d,options,capacity);
    if(destruction_choice_kind(d->choice.kind))return destruction_choice_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_LAIR_OFFER)return lair_offer_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_STAT_SOURCE||d->choice.kind==DUEL_CHOICE_STAT_DESTINATION)return stat_snapshot_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_RETARGET_MODE||d->choice.kind==DUEL_CHOICE_RETARGET_MONSTER)return retarget_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_FAIRY_BOX_COIN){for(unsigned i=0;i<2;++i)if(options&&i<capacity)options[i]=(DuelChoiceOption){d->choice.card,i,DUEL_OPTION_CONFIRM};return 2;}
    if(d->choice.kind==DUEL_CHOICE_ATTACK_COIN){for(unsigned i=0;i<2;++i)if(options&&i<capacity)options[i]=(DuelChoiceOption){d->choice.card,i,DUEL_OPTION_CONFIRM};if(options&&capacity>2)options[2]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return 3;}
    if(d->choice.kind==DUEL_CHOICE_REVIVAL_JAM)return revival_jam_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_GRAVE_PAIR||d->choice.kind==DUEL_CHOICE_GRAVE_PAIR_POSITION)return grave_pair_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_HISTORY_OFFER||d->choice.kind==DUEL_CHOICE_HISTORY_TARGET)return history_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_DECK_EQUIP)return deck_equipment_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_ACTIVATION_PARAMETER)return trait_parameter_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_ATTACK_TRIBUTE_COST||d->choice.kind==DUEL_CHOICE_ATTACK_TRIBUTE_TARGET){unsigned n=d->choice.kind==DUEL_CHOICE_ATTACK_TRIBUTE_COST?attack_tribute_cost_options(d,d->choice.player,d->attack_tribute.command.source,options,capacity):attack_tribute_target_options(d,options,capacity);if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return n+1;}
    if(d->choice.kind==DUEL_CHOICE_SUMMON_PROCEDURE_COST)return summon_procedure_cost_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_KUNAI_MODE||d->choice.kind==DUEL_CHOICE_KUNAI_TARGET){unsigned n=d->choice.kind==DUEL_CHOICE_KUNAI_MODE?kunai_modes(d,d->choice.player,d->choice.card,options,capacity):kunai_targets(d,d->choice.player,d->choice.card,options,capacity);if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return n+1;}
    if(d->choice.kind==DUEL_CHOICE_EQUIP_TRANSFER_SOURCE||d->choice.kind==DUEL_CHOICE_EQUIP_TRANSFER_TARGET){unsigned token=d->equip_mode_command.tributes-1,n=d->choice.kind==DUEL_CHOICE_EQUIP_TRANSFER_SOURCE?equipment_transfer_sources(d,options,capacity):equipment_transfer_targets(d,token/6,token%6,options,capacity);if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return n+1;}
    if(d->choice.kind==DUEL_CHOICE_EQUIP_STAT){for(unsigned i=0;i<2;++i)if(options&&i<capacity)options[i]=(DuelChoiceOption){d->choice.card,i,DUEL_OPTION_CONFIRM};if(options&&capacity>2)options[2]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return 3;}
    if(d->choice.kind>=DUEL_CHOICE_DARK_SAGE_OFFER&&d->choice.kind<=DUEL_CHOICE_DARK_SAGE_SPELL)return dark_sage_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_PILE_SUMMON||d->choice.kind==DUEL_CHOICE_PILE_SUMMON_POSITION)return pile_summon_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_IGNITION_FUSION||d->choice.kind==DUEL_CHOICE_IGNITION_POSITION)return ignition_choice_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_PEEK_VIEW||d->choice.kind==DUEL_CHOICE_PEEK_HAND||d->choice.kind==DUEL_CHOICE_PEEK_ORDER)return peek_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_LAIR_TARGET)return lair_target_options(d,d->choice.selected_count,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_PILE_PLAYER||d->choice.kind==DUEL_CHOICE_PILE_CARD)return pile_target_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_SPIRIT_POSITION)return periodic_spirit_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_SPECIAL_HAND_COST||d->choice.kind==DUEL_CHOICE_SPECIAL_HAND_POSITION)return special_hand_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_OPTIONAL_SET)return optional_set_options(d,d->choice.player,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_SIMPLE_DESTROY)return simple_destroy_options(d,d->choice.player,d->choice.card,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_TRIBUTE_UPKEEP)return tribute_upkeep_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_MUSHROOM)return mushroom_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_UPKEEP)return upkeep_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_MAGIC_DRAIN)return counter_drain_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_PERIODIC)return periodic_offer_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_EQUIP_RECYCLE)return equip_recycle_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_FLIP_OFFER)return flip_offer_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_HAND)return hand_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_SEARCH_OFFER){if(options&&capacity)options[0]=(DuelChoiceOption){d->choice.card,0,DUEL_OPTION_CONFIRM};if(options&&capacity>1)options[1]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return 2;}
    if(d->choice.kind==DUEL_CHOICE_DECK_SEARCH){unsigned n=search_options(d,d->choice.player,d->choice.card,options,capacity);if(n&&d->choice.selected_count){if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};++n;}return n;}
    if(d->choice.kind==DUEL_CHOICE_TARGET_FLIP)return target_flip_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_RECRUIT)return recruit_options(d,d->choice.player,d->choice.card,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_RITUAL_MONSTER)return ritual_monsters(d,d->choice.player,d->ritual_task.spell,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_RITUAL_MATERIAL)return ritual_material_options(d,d->choice.player,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_RITUAL_POSITION){if(options&&capacity)options[0]=(DuelChoiceOption){d->choice.card,DUEL_FACEUP_ATTACK,DUEL_OPTION_CONFIRM};if(options&&capacity>1)options[1]=(DuelChoiceOption){d->choice.card,DUEL_FACEUP_DEFENSE,DUEL_OPTION_CONFIRM};return 2;}
    if(d->choice.kind==DUEL_CHOICE_DISCARD_HAND){for(unsigned i=0;i<p->hand_count;++i){if(options&&count<capacity)options[count]=(DuelChoiceOption){p->hand[i],i,DUEL_OPTION_CARD};++count;}return count;}
    if(d->choice.kind==DUEL_CHOICE_BATTLE_REVIVE||d->choice.kind==DUEL_CHOICE_BATTLE_REVIVE_POSITION)return battle_revival_options(d,options,capacity);
    if(offering_choice(d->choice.kind))return offering_options(d,options,capacity);
    if(d->choice.kind==DUEL_CHOICE_DEFUSION){if(options&&capacity)options[0]=(DuelChoiceOption){d->choice.card,0,DUEL_OPTION_CONFIRM};if(options&&capacity>1)options[1]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return 2;}
    if(d->choice.kind==DUEL_CHOICE_FUSION_MONSTER)return fusion_options(d,d->choice.player,options,capacity,DUEL_NO_SLOT);
    if(d->choice.kind==DUEL_CHOICE_FUSION_MATERIAL){DuelFusionTask task=d->fusion_task;unsigned card=p->fusion[task.index];
        for(unsigned token=0;token<DUEL_HAND_MAX+5;++token){task.selected[task.count]=token;if(fusion_possible(d,d->choice.player,card,task.selected,task.count+1,DUEL_NO_SLOT)){if(options&&count<capacity)options[count]=(DuelChoiceOption){material_card(d,d->choice.player,token),token,DUEL_OPTION_CARD};++count;}}return count;}
    if(d->choice.kind==DUEL_CHOICE_FUSION_POSITION){if(options&&capacity)options[0]=(DuelChoiceOption){d->choice.card,0,DUEL_OPTION_CONFIRM};if(options&&capacity>1)options[1]=(DuelChoiceOption){d->choice.card,1,DUEL_OPTION_CONFIRM};return 2;}
    if(d->choice.kind==DUEL_CHOICE_ATTACK_REPLAY){
        const DuelPlayer *enemy=&d->players[1-d->choice.player];
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&enemy->monsters[i];if(s->card&&duel_can_attack_target(d,d->choice.player,d->deferred_attack.source,i)){if(options&&count<capacity)options[count]=(DuelChoiceOption){s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,i,DUEL_OPTION_CARD};++count;}}
        if(duel_can_attack_direct(d,d->choice.player,d->deferred_attack.source)){if(options&&count<capacity)options[count]=(DuelChoiceOption){d->choice.card,5,DUEL_OPTION_CONFIRM};++count;}
        if(options&&count<capacity)options[count]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return count+1;
    }
    if(flip_target_choice(d)){
        unsigned returning=d->cards[d->choice.card].effect!=DUEL_E_MAN_EATER_BUG;
        for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];const DuelPlayer *owner=&d->players[s->owner];if(!s->card||!duel_can_target_monster(d,side,i)||battle_destroyed(d,side,i)||(returning&&(d->cards[s->card].subtype==2?owner->fusion_count>=DUEL_FUSION_MAX:owner->hand_count>=DUEL_HAND_MAX)))continue;unsigned selected=0;
            for(unsigned j=0;j<d->choice.selected_count;++j)selected|=d->choice_targets[j].player==side&&d->choice_targets[j].slot==i;
            if(!selected){if(options&&count<capacity)options[count]=(DuelChoiceOption){side!=d->choice.player&&s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,side*5+i,DUEL_OPTION_CARD};++count;}}
        if(d->choice.kind==DUEL_CHOICE_PENGUIN&&(count||d->choice.selected_count)){if(options&&count<capacity)options[count]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};++count;}return count;
    }
    if(d->choice.kind==DUEL_CHOICE_GRAVE_RECALL){
        int family=recall_family(d,d->choice.card);if(family<0||p->hand_count>=DUEL_HAND_MAX)return 0;
        for(unsigned i=0;i<p->grave_count;++i)if(d->cards[p->grave[i]].family==family){if(options&&count<capacity)options[count]=(DuelChoiceOption){p->grave[i],i,DUEL_OPTION_CARD};++count;}return count;
    }
    if(d->choice.kind==DUEL_CHOICE_OPTIONAL_DRAW){
        if(!p->deck_count||p->hand_count>=DUEL_HAND_MAX||trigger_source(d,&d->choice)<0)return 0;
        if(options&&capacity)options[0]=(DuelChoiceOption){d->choice.card,0,DUEL_OPTION_CONFIRM};
        if(options&&capacity>1)options[1]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return 2;
    }
    if(d->choice.kind==DUEL_CHOICE_NIMBLE_SUMMON){
        if(!special_summon_allowed_for(d,d->choice.player))return 0;
        if(empty_zone(p->monsters)<0||d->choice.selected_count>=swarm_limit(d,d->choice.card))return 0;
        for(unsigned i=0;i<p->deck_count;++i)if(d->cards[p->deck[i]].name_group==d->cards[d->choice.card].name_group){if(options&&count<capacity)options[count]=(DuelChoiceOption){p->deck[i],i,DUEL_OPTION_CARD};++count;}
        if(count){if(options&&count<capacity)options[count]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};++count;}return count;
    }
    if(p->hand_count>=DUEL_HAND_MAX)return 0;
    for(unsigned i=0;i<p->deck_count;++i){unsigned card=p->deck[i];const DuelCardDef *def=&d->cards[card];
        if(def->family==DUEL_MONSTER&&(d->choice.kind==DUEL_CHOICE_SEARCH_ATTACK?def->attack<=1500:def->defense<=1500)){
            if(count<capacity&&options)options[count]=(DuelChoiceOption){card,i};++count;
        }}return count;
}
unsigned duel_choice_options(const Duel *d,DuelChoiceOption *options,unsigned capacity){if(!d||d->bait_mode!=2||d->choice.kind==DUEL_CHOICE_OFFERING_PAY||d->choice.kind==DUEL_CHOICE_LAIR_OFFER)return duel_choice_options_inner(d,options,capacity);DuelChoiceOption all[DUEL_OPTION_MAX];unsigned total=duel_choice_options_inner(d,all,DUEL_OPTION_MAX),n=0;for(unsigned i=0;i<total&&i<DUEL_OPTION_MAX;++i)if(all[i].kind!=DUEL_OPTION_SKIP){if(options&&n<capacity)options[n]=all[i];++n;}return n;}

static void finish_choice(Duel *d){if((d->choice.kind==DUEL_CHOICE_NIMBLE_SUMMON&&d->choice.selected_count)||d->choice.kind==DUEL_CHOICE_RECRUIT||d->choice.kind==DUEL_CHOICE_DECK_SEARCH)shuffle(d,&d->players[d->choice.player]);clear(&d->choice,sizeof(d->choice));clear(d->choice_targets,sizeof(d->choice_targets));}
static void activate_flip_choice(Duel *d){
    if(d->choice.selected_count){DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command.player=d->choice.player;link->card=d->choice.card;link->source_serial=d->choice.serial;if(d->cards[link->card].name_group==640||d->cards[link->card].name_group==499||d->cards[link->card].name_group==1246||d->cards[link->card].name_group==1332||d->cards[link->card].name_group==729){link->source_slot=DUEL_NO_SLOT;for(unsigned i=0;i<5;++i)if(d->players[d->choice.player].monsters[i].serial==link->source_serial&&d->players[d->choice.player].monsters[i].card==link->card)link->source_slot=i;}link->speed=1;link->effect_activation=1;link->target_count=d->choice.selected_count;
        for(unsigned i=0;i<link->target_count;++i)link->targets[i]=d->choice_targets[i];d->trigger_chain_pending=1;}
    finish_choice(d);
}
static void advance_effects(Duel *d){if(bait_advance(d))return;
    for(;;){if(!d->chain_count&&!d->choice.kind){check_victory_boundary(d);if(d->winner>=0)return;}if(d->response_player<2)return;
    if(!d->choice.kind&&!d->chain_count&&d->battle_revival_notice){unsigned mandatory=0;for(unsigned i=0;i<d->trigger_count;++i)mandatory|=d->triggers[i].kind!=DUEL_CHOICE_BATTLE_REVIVE;if(!mandatory&&battle_revival_window(d))return;}
    while(d->winner<0&&!d->choice.kind&&d->trigger_count){
        unsigned index=0;for(unsigned i=0;i<d->trigger_count;++i)if(d->triggers[i].player==d->active){index=i;break;}
        if(d->battle_revival_notice&&d->triggers[index].kind==DUEL_CHOICE_BATTLE_REVIVE){for(unsigned i=0;i<d->trigger_count;++i)if(d->triggers[i].kind!=DUEL_CHOICE_BATTLE_REVIVE){index=i;break;}}
        d->choice=d->triggers[index];for(unsigned i=index;i+1<d->trigger_count;++i)d->triggers[i]=d->triggers[i+1];--d->trigger_count;
        if(d->choice.kind==DUEL_CHOICE_BATTLE_REVIVE&&d->battle_revival_notice){DuelTrigger saved=d->choice;finish_choice(d);queue_trigger(d,saved.kind,saved.player,saved.card,saved.serial);if(battle_revival_window(d))return;continue;}
        if(d->choice.kind==DUEL_CHOICE_BATTLE_REVIVE){unsigned at=d->choice.serial-1;if(at>=2||!duel_battle_revival_ready(d,&d->battle_revive_jobs[at])){if(at<2)d->battle_revive_jobs[at].used=0;finish_choice(d);continue;}}
        if(d->choice.kind==DUEL_CHOICE_GRAVE_PAIR){grave_pair_begin(d);if(!d->choice.kind)continue;}
        if(d->choice.kind==DUEL_TRIGGER_QUEEN_END){DuelTrigger t=d->choice;finish_choice(d);queen_end_resolve(d,&t);if(d->summon_notice.kind){d->window=d->summon_notice;clear(&d->summon_notice,sizeof(d->summon_notice));begin_response(d,1-d->window.player);return;}continue;}
        if(d->choice.kind==DUEL_TRIGGER_REVIVAL_JOB){DuelTrigger t=d->choice;finish_choice(d);revival_job_resolve(d,&t);if(d->summon_notice.kind){d->window=d->summon_notice;clear(&d->summon_notice,sizeof(d->summon_notice));begin_response(d,1-d->window.player);return;}continue;}
        if(d->choice.kind==DUEL_TRIGGER_TOKEN_STANDBY){DuelTrigger t=d->choice;finish_choice(d);token_standby_resolve(d,&t);if(d->summon_notice.kind){d->window=d->summon_notice;clear(&d->summon_notice,sizeof(d->summon_notice));begin_response(d,1-d->window.player);return;}continue;}
        if(d->choice.kind==DUEL_TRIGGER_PARASITE_DRAW){DuelTrigger t=d->choice;finish_choice(d);DuelChainLink *l=&d->chain[d->chain_count++];clear(l,sizeof(*l));l->command.player=t.player;l->command.cost=2;l->card=t.card;l->source_serial=t.serial;l->source_slot=DUEL_NO_SLOT;l->speed=1;l->effect_activation=1;d->trigger_chain_pending=1;continue;}
        if(d->choice.kind==DUEL_CHOICE_DECK_EQUIP){DuelChoiceOption option;if(deck_equipment_options(d,&option,1)==1&&option.index==DUEL_NO_SLOT){deck_equipment_choose(d,DUEL_NO_SLOT);continue;}}
        if(d->choice.kind==DUEL_CHOICE_TRIBUTE_UPKEEP&&duel_choice_options(d,0,0)==1){tribute_upkeep_choose(d,DUEL_NO_SLOT);continue;}
        if(d->choice.kind==DUEL_TRIGGER_FLIP_AUTO&&d->cards[d->choice.card].name_group==1512){d->choice.kind=DUEL_CHOICE_SUMMON_PROCEDURE_COST;if(summon_procedure_cost_options(d,0,0)==1){finish_choice(d);continue;}return;}
        if(d->choice.kind==DUEL_TRIGGER_FLIP_AUTO&&d->cards[d->choice.card].name_group==489){d->choice.kind=DUEL_CHOICE_FLIP_OFFER;if(!duel_choice_options(d,0,0))finish_choice(d);continue;}
        if(d->choice.kind==DUEL_TRIGGER_FLIP_AUTO&&d->cards[d->choice.card].name_group==731&&d->choice.player==d->active&&d->phase==DUEL_STANDBY){unsigned side=d->choice.player,index=d->choice.selected_count;if(!jigen_ready(d,side,index,d->choice.serial)){finish_choice(d);continue;}leave_field(d,side,index,0,LEAVE_TRIBUTE);}
        if(d->choice.kind==DUEL_TRIGGER_ARRIVAL_AUTO||d->choice.kind==DUEL_TRIGGER_GRAVE_AUTO||d->choice.kind==DUEL_TRIGGER_MORPH2||d->choice.kind==DUEL_TRIGGER_FLIP_AUTO||d->choice.kind==DUEL_TRIGGER_RECRUIT||d->choice.kind==DUEL_TRIGGER_SWARM||d->choice.kind==DUEL_TRIGGER_DISCARD||d->choice.kind==DUEL_TRIGGER_EQUIP_GRAVE||d->choice.kind==DUEL_TRIGGER_PERIODIC||d->choice.kind==DUEL_TRIGGER_POSITION_AUTO||d->choice.kind==DUEL_TRIGGER_CONTINUOUS||d->choice.kind==DUEL_TRIGGER_BATTLE_AUTO||d->choice.kind==DUEL_TRIGGER_GILA){if(d->choice.kind==DUEL_TRIGGER_FLIP_AUTO&&!flip_auto_pay(d,d->choice.player,d->choice.card)){finish_choice(d);continue;}DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command.player=d->choice.player;link->card=d->choice.card;link->source_serial=d->choice.serial;if(d->choice.kind==DUEL_TRIGGER_GRAVE_AUTO&&d->cards[link->card].name_group==461)link->command.cost=1;if(d->choice.kind==DUEL_TRIGGER_ARRIVAL_AUTO||d->cards[link->card].name_group==1528||(d->cards[link->card].name_group==82||d->cards[link->card].name_group==731||d->cards[link->card].name_group==762||reactive_hand_name(d->cards[link->card].name_group)))link->source_slot=d->choice.selected_count;link->speed=1;link->effect_activation=1;unsigned kycoo=d->choice.kind==DUEL_TRIGGER_BATTLE_AUTO&&battle_event_op(d,d->choice.card)==6;d->trigger_chain_pending=1;finish_choice(d);if(kycoo){if(!pile_target_candidates(d,link->command.player,41,1-link->command.player,0,0,0)){--d->chain_count;continue;}d->trigger_chain_pending=0;pile_target_begin(d,d->chain_count-1,41);}continue;}
        if(d->choice.kind==DUEL_CHOICE_NIMBLE_RECOVER){recover(d,d->choice.player,1000,d->choice.card);d->choice.kind=DUEL_CHOICE_NIMBLE_SUMMON;}
        if(!duel_choice_options(d,0,0))finish_choice(d);
    }
    if(d->choice.kind||d->winner>=0)return;
    if(!d->chain_count&&!d->trigger_count&&d->damage_done_notice.kind&&damage_done_window(d))return;
    if(d->trigger_chain_pending&&d->chain_count){d->trigger_chain_pending=0;begin_response(d,1-d->chain[d->chain_count-1].command.player);return;}
    if(d->window.kind==DUEL_WINDOW_SUMMON&&!d->chain_count){begin_response(d,1-d->window.player);return;}
    if(d->battle_pending&&!d->chain_count){finish_battle(d);continue;}
    if(d->deferred_attack.kind&&!d->chain_count){continue_attack(d);if(d->choice.kind)return;continue;}
    if(!d->chain_count){if(d->turn_end_pending&&destiny_end_advance(d))continue;history_end_queue(d);if(!d->trigger_count&&d->turn_end_pending)necro_end_queue(d);if(d->trigger_count)continue;}
    if(!d->chain_count&&!d->trigger_count&&reactive_open(d))return;
    if(!d->chain_count&&!d->trigger_count&&jigen_standby_step(d))continue;
    if(!d->chain_count&&!d->trigger_count&&d->standby_finish_actor_plus1){unsigned side=d->standby_finish_actor_plus1-1;d->standby_finish_actor_plus1=0;if(d->phase==DUEL_STANDBY){castle_standby(d,side);continuous_cleanup(d);control_reconcile(d);}continue;}
    if(!d->chain_count&&d->phase==DUEL_STANDBY&&inspection_offer(d))return;
    if(d->phase_request&&d->phase==DUEL_STANDBY&&patrol_any(d))d->phase_request=0;
    if(d->phase_request&&!d->chain_count){d->phase=d->phase_request;d->phase_request=0;event(d,DUEL_EVENT_PHASE,d->active,0,0,d->phase);}
    if(d->winner<0&&!d->choice.kind&&!d->trigger_count&&d->turn_end_pending){
        if(d->players[d->active].hand_count>hand_limit(d)){d->discard_required=1;return;}
        turn_status_end(d);history_turn_cleanup(d);d->turn_end_pending=0;d->active=1-d->active;++d->turn;d->discard_required=0;begin_turn(d);continue;}return;
    }
}
static int execute_command(Duel *d,const DuelCommand *c){
    if(!d||!c||c->player>1)return DUEL_BAD_ARGUMENT;if(d->winner>=0)return DUEL_FINISHED;
    if(c->action==DUEL_ACTION_CONCEDE){win(d,1-c->player);return DUEL_OK;}
    if(d->choice.kind){
        if(c->action!=DUEL_ACTION_CHOOSE||c->player!=d->choice.player)return DUEL_EFFECT_PENDING;
        DuelChoiceOption options[DUEL_OPTION_MAX];unsigned count=duel_choice_options(d,options,DUEL_OPTION_MAX);int legal=0;
        for(unsigned i=0;i<count;++i)if(options[i].index==c->source)legal=1;if(!legal)return DUEL_BAD_TARGET;
        if(d->choice.kind==DUEL_CHOICE_BAIT_TARGET||d->choice.kind==DUEL_CHOICE_BAIT_BACKUP){int r=bait_choose(d,c->source);++d->revision;return r;}
        if(borrow_hats_choice(d->choice.kind)){int result=borrow_hats_choose(d,c->source);++d->revision;return result;}
        if(d->choice.kind==DUEL_CHOICE_MAJOR_RIOT){riot_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(excavation_choice(d->choice.kind)){excavation_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_PATROL_TARGET){int r=patrol_choose(d,c->source);++d->revision;return r;}
        if(d->choice.kind==DUEL_CHOICE_INSPECTION_OFFER){int r=inspection_choose(d,c->source);++d->revision;return r;}
        if(d->choice.kind==DUEL_CHOICE_GAMBLE_COIN){gamble_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_CRUSH_TRIBUTE){int r=crush_choose(d,c->source);++d->revision;return r;}
        if(coordinate_choice(d->choice.kind)){int r=coordinate_choose(d,c->source);++d->revision;return r;}
        if(d->choice.kind==DUEL_CHOICE_YADO_HAND){yado_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(hand_selection_kind(d->choice.kind)){hand_selection_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(destruction_choice_kind(d->choice.kind)){int r=destruction_choose(d,c->source);++d->revision;return r;}
        if(d->choice.kind==DUEL_CHOICE_LAIR_OFFER){int r=lair_offer_choose(d,c->source);++d->revision;return r;}
        if(d->choice.kind==DUEL_CHOICE_STAT_SOURCE||d->choice.kind==DUEL_CHOICE_STAT_DESTINATION){int r=stat_snapshot_choose(d,c->source);++d->revision;return r;}
        if(d->choice.kind==DUEL_CHOICE_RETARGET_MODE||d->choice.kind==DUEL_CHOICE_RETARGET_MONSTER){int r=retarget_choose(d,c->source);++d->revision;return r;}
        if(d->choice.kind==DUEL_CHOICE_FAIRY_BOX_COIN){fairy_box_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_BATTLE_REDIRECT){int r=redirect_choose(d,c->source);++d->revision;return r;}
        if(d->choice.kind==DUEL_CHOICE_BATTLE_REVIVE||d->choice.kind==DUEL_CHOICE_BATTLE_REVIVE_POSITION){battle_revival_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(offering_choice(d->choice.kind)){return offering_choose(d,c->source);}
        if(d->choice.kind==DUEL_CHOICE_ATTACK_COIN){DuelCommand attack=d->coin_attack;finish_choice(d);clear(&d->coin_attack,sizeof(d->coin_attack));if(c->source==DUEL_NO_SLOT){++d->revision;return DUEL_OK;}attack.position=c->source;attack.cost=1;return battle(d,&attack);}
        if(d->choice.kind==DUEL_CHOICE_OPTIONAL_SET){optional_set_choose(d,c->source);return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_HISTORY_OFFER||d->choice.kind==DUEL_CHOICE_HISTORY_TARGET){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;history_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_DECK_EQUIP){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;deck_equipment_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_ATTACK_TRIBUTE_COST){attack_tribute_cost_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_ATTACK_TRIBUTE_TARGET){DuelCommand selected=d->attack_tribute.command;finish_choice(d);if(c->source==DUEL_NO_SLOT){clear(&d->attack_tribute,sizeof(d->attack_tribute));++d->revision;return DUEL_OK;}selected.cost=1;selected.target_player=1-selected.player;selected.target=c->source==5?DUEL_NO_SLOT:c->source;int r=battle(d,&selected);if(r!=DUEL_OK){clear(&d->attack_tribute,sizeof(d->attack_tribute));++d->revision;return DUEL_OK;}return r;}
        if(d->choice.kind==DUEL_CHOICE_SUMMON_PROCEDURE_COST){if(d->cards[d->choice.card].name_group==1512){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;summon_procedure_illusion_choose(d,c->source);++d->revision;return DUEL_OK;}DuelCommand selected=d->equip_mode_command;finish_choice(d);clear(&d->equip_mode_command,sizeof(d->equip_mode_command));if(c->source==DUEL_NO_SLOT){++d->revision;return DUEL_OK;}selected.cost=1;selected.tributes=c->source+1;return activate(d,&selected,selected.action==DUEL_ACTION_ACTIVATE_BACKROW);}
        if(d->choice.kind==DUEL_CHOICE_KUNAI_MODE||d->choice.kind==DUEL_CHOICE_KUNAI_TARGET){if(c->source==DUEL_NO_SLOT){finish_choice(d);clear(&d->equip_mode_command,sizeof(d->equip_mode_command));++d->revision;return DUEL_OK;}if(d->choice.kind==DUEL_CHOICE_KUNAI_MODE){d->equip_mode_command.cost=c->source;if(c->source!=1){d->choice.kind=DUEL_CHOICE_KUNAI_TARGET;++d->revision;return DUEL_OK;}}else{d->equip_mode_command.target_player=d->choice.player;d->equip_mode_command.target=c->source;}DuelCommand selected=d->equip_mode_command;finish_choice(d);clear(&d->equip_mode_command,sizeof(d->equip_mode_command));return activate(d,&selected,selected.action==DUEL_ACTION_ACTIVATE_BACKROW);}
        if(d->choice.kind==DUEL_CHOICE_EQUIP_TRANSFER_SOURCE||d->choice.kind==DUEL_CHOICE_EQUIP_TRANSFER_TARGET){if(c->source==DUEL_NO_SLOT){finish_choice(d);clear(&d->equip_mode_command,sizeof(d->equip_mode_command));++d->revision;return DUEL_OK;}if(d->choice.kind==DUEL_CHOICE_EQUIP_TRANSFER_SOURCE){d->equip_mode_command.tributes=c->source+1;d->choice.kind=DUEL_CHOICE_EQUIP_TRANSFER_TARGET;++d->revision;return DUEL_OK;}DuelCommand selected=d->equip_mode_command;selected.cost=2;selected.target_player=c->source/5;selected.target=c->source%5;finish_choice(d);clear(&d->equip_mode_command,sizeof(d->equip_mode_command));return activate(d,&selected,selected.action==DUEL_ACTION_ACTIVATE_BACKROW);}
        if(d->choice.kind==DUEL_CHOICE_EQUIP_STAT||d->choice.kind==DUEL_CHOICE_ACTIVATION_PARAMETER){DuelCommand selected=d->equip_mode_command;unsigned name=d->cards[d->choice.card].name_group,page=d->choice.selected_count;if(name==1162&&(c->source==253||c->source==254)){d->choice.selected_count=page+(c->source==254?1:-1);++d->revision;return DUEL_OK;}finish_choice(d);clear(&d->equip_mode_command,sizeof(d->equip_mode_command));if(c->source==DUEL_NO_SLOT){++d->revision;return DUEL_OK;}if(name==1162)selected.tributes=page*128+c->source;else selected.position=c->source;selected.cost=1;return activate(d,&selected,selected.action==DUEL_ACTION_ACTIVATE_BACKROW);}
        if(d->choice.kind>=DUEL_CHOICE_DARK_SAGE_OFFER&&d->choice.kind<=DUEL_CHOICE_DARK_SAGE_SPELL){dark_sage_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_REVIVAL_JAM){revival_jam_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_GRAVE_PAIR||d->choice.kind==DUEL_CHOICE_GRAVE_PAIR_POSITION){grave_pair_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_PILE_SUMMON||d->choice.kind==DUEL_CHOICE_PILE_SUMMON_POSITION){pile_summon_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_IGNITION_FUSION||d->choice.kind==DUEL_CHOICE_IGNITION_POSITION){ignition_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_PEEK_VIEW||d->choice.kind==DUEL_CHOICE_PEEK_HAND||d->choice.kind==DUEL_CHOICE_PEEK_ORDER){peek_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_FLIP_OFFER){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;flip_offer_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_LAIR_TARGET){lair_target_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_PILE_PLAYER||d->choice.kind==DUEL_CHOICE_PILE_CARD){pile_target_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_SPIRIT_POSITION){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;periodic_spirit_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_SPECIAL_HAND_COST||d->choice.kind==DUEL_CHOICE_SPECIAL_HAND_POSITION){special_hand_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_SIMPLE_DESTROY){simple_destroy_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_TRIBUTE_UPKEEP){tribute_upkeep_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_MUSHROOM){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;mushroom_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_UPKEEP){upkeep_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_MAGIC_DRAIN){counter_drain_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_PERIODIC){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;periodic_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_EQUIP_RECYCLE){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;equip_recycle_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_HAND){hand_choose(d,c->source);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_SEARCH_OFFER){if(c->source!=DUEL_NO_SLOT){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command.player=c->player;link->card=d->choice.card;link->source_serial=d->choice.serial;link->effect_activation=1;link->speed=1;d->trigger_chain_pending=1;}finish_choice(d);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_DECK_SEARCH){if(c->source==DUEL_NO_SLOT)finish_choice(d);else {DuelPlayer *p=&d->players[c->player];DuelCardState selected=card_state_reset(take_pile_card(p,PILE_DECK,c->source));unsigned card=selected.card;put_pile_card(d,c->player,PILE_HAND,selected,0);event(d,DUEL_EVENT_RETURN,c->player,0,card,0);++d->choice.selected_count;if(d->choice.selected_count>=search_rule(d,d->choice.card)->maximum||!search_options(d,c->player,d->choice.card,0,0))finish_choice(d);}++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_RECRUIT){if(c->source!=DUEL_NO_SLOT)finish_recruit(d,c->player,c->source);finish_choice(d);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_RITUAL_MONSTER){d->ritual_task.monster_index=c->source;d->choice.card=d->players[c->player].hand[c->source];d->choice.kind=DUEL_CHOICE_RITUAL_MATERIAL;++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_RITUAL_MATERIAL){DuelRitualTask *t=&d->ritual_task;if(t->count>=DUEL_HAND_MAX+5)return DUEL_NOT_ALLOWED;t->selected[t->count++]=c->source;unsigned field;if(ritual_levels(d,c->player,&field)>=t->level)d->choice.kind=DUEL_CHOICE_RITUAL_POSITION;++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_RITUAL_POSITION){finish_choice(d);finish_ritual(d,c->player,c->source);return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_DISCARD_HAND){DuelPlayer *p=&d->players[c->player];discard_hand_effect(d,c->player,c->source,c->player,0);if(d->choice.selected_count)--d->choice.selected_count;if(!d->choice.selected_count||!p->hand_count)finish_choice(d);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_DEFUSION){finish_choice(d);if(c->source==DUEL_NO_SLOT)clear(&d->pile_summon_task,sizeof(d->pile_summon_task));else pile_summon_next(d);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_FUSION_MONSTER){d->fusion_task.index=c->source;d->fusion_task.count=0;d->choice.kind=DUEL_CHOICE_FUSION_MATERIAL;d->choice.card=d->players[c->player].fusion[c->source];++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_FUSION_MATERIAL){DuelFusionTask *task=&d->fusion_task;task->selected[task->count++]=c->source;const DuelFusionRecipe *r=fusion_recipe(d,d->choice.card);if(task->count==r->count)d->choice.kind=DUEL_CHOICE_FUSION_POSITION;++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_FUSION_POSITION){finish_choice(d);finish_fusion(d,c->player,c->source);clear(&d->fusion_task,sizeof(d->fusion_task));return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_ATTACK_REPLAY){
            DuelWindow window=d->deferred_attack;clear(&d->deferred_attack,sizeof(d->deferred_attack));finish_choice(d);++d->revision;
            if(c->source!=DUEL_NO_SLOT){window.target=c->source==5?DUEL_NO_SLOT:c->source;window.target_serial=0;
                if(window.target<5){DuelSlot *s=&d->players[1-c->player].monsters[window.target];if(!s->serial)s->serial=++d->next_serial;window.target_serial=s->serial;}resolve_battle(d,&window);}return DUEL_OK;
        }
        if(d->choice.kind==DUEL_CHOICE_GRAVE_RECALL){
            if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;DuelPlayer *p=&d->players[c->player];if(!p->grave_serial[c->source])p->grave_serial[c->source]=++d->next_serial;
            d->choice_targets[0]=(DuelTarget){p->grave_serial[c->source],c->player,c->source};d->choice.selected_count=1;activate_flip_choice(d);++d->revision;return DUEL_OK;
        }
        if(d->choice.kind==DUEL_CHOICE_TARGET_FLIP){if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;unsigned side=c->source/11,index=c->source%11;DuelSlot *s=target_flip_slot(d,side,index);if(!s->serial)s->serial=++d->next_serial;d->choice_targets[d->choice.selected_count++]=(DuelTarget){s->serial,side,index};if(d->choice.selected_count==target_flip_rule(d,d->choice.card)->count)activate_flip_choice(d);++d->revision;return DUEL_OK;}
        if(flip_target_choice(d)){
            if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;
            if(c->source==DUEL_NO_SLOT){activate_flip_choice(d);++d->revision;return DUEL_OK;}
            unsigned side=c->source/5,index=c->source%5;DuelSlot *s=&d->players[side].monsters[index];if(!s->serial)s->serial=++d->next_serial;
            d->choice_targets[d->choice.selected_count++]=(DuelTarget){s->serial,side,index};++d->revision;
            if(d->choice.selected_count==(d->choice.kind==DUEL_CHOICE_PENGUIN?2:1))activate_flip_choice(d);return DUEL_OK;
        }
        if(c->source==DUEL_NO_SLOT){finish_choice(d);++d->revision;return DUEL_OK;}
        if(d->choice.kind==DUEL_CHOICE_OPTIONAL_DRAW){
            int slot=trigger_source(d,&d->choice);if(slot<0)return DUEL_BAD_TARGET;if(d->chain_count>=DUEL_CHAIN_MAX)return DUEL_NOT_ALLOWED;
            DuelChainLink *link=&d->chain[d->chain_count++];clear(link,sizeof(*link));link->command.player=c->player;link->card=d->choice.card;link->source_slot=slot;link->source_serial=d->choice.serial;link->speed=1;link->effect_activation=1;d->trigger_chain_pending=1;
            finish_choice(d);++d->revision;return DUEL_OK;
        }
        if(d->choice.kind==DUEL_CHOICE_NIMBLE_SUMMON){
            DuelPlayer *p=&d->players[c->player];int slot=empty_zone(p->monsters);if(slot<0)return DUEL_ZONE_FULL;
            DuelCardState recruited=take_pile_card(p,PILE_DECK,c->source);unsigned card=recruited.card;DuelSlot *s=&p->monsters[slot];clear(s,sizeof(*s));s->card=card;s->owner=c->player;s->position=swarm_position(d,d->choice.card);s->summoned_turn=s->set_turn=d->turn;s->properly_summoned=1;s->return_controller=DUEL_NO_SLOT;slot_install_state(d,s,recruited);s->properly_summoned=1;special_summon_success(d,c->player,slot);++d->choice.selected_count;
            event(d,DUEL_EVENT_REVIVE,c->player,slot,card,0);if(!duel_choice_options(d,0,0))finish_choice(d);return DUEL_OK;
        }
        DuelPlayer *p=&d->players[c->player];DuelCardState selected=card_state_reset(take_pile_card(p,PILE_DECK,c->source));unsigned card=selected.card;put_pile_card(d,c->player,PILE_HAND,selected,0);shuffle(d,p);
        clear(&d->choice,sizeof(d->choice));event(d,DUEL_EVENT_DRAW,c->player,0,card,0);return DUEL_OK;
    }
    if(d->response_player<2){
        if(c->player!=d->response_player)return DUEL_EFFECT_PENDING;
        if(c->action==DUEL_ACTION_PASS){++d->response_passes;d->response_player=1-d->response_player;++d->revision;return DUEL_OK;}
        if(c->action==DUEL_ACTION_ACTIVATE_HAND)return activate(d,c,0);
        if(c->action==DUEL_ACTION_ACTIVATE_BACKROW)return activate(d,c,1);
        if(c->action==DUEL_ACTION_ACTIVATE_MONSTER)return c->source<5&&d->cards[d->players[c->player].monsters[c->source].card].name_group==1525?mary_activate(d,c):damage_component_activate(d,c);
        return DUEL_EFFECT_PENDING;
    }
    if(c->player!=d->active){if(c->action==DUEL_ACTION_ACTIVATE_BACKROW&&duel_inspection_ready(d,c->player,c->source))return activate(d,c,1);if(c->action==DUEL_ACTION_ACTIVATE_BACKROW&&c->source<5&&d->players[c->player].backrow[c->source].card&&d->cards[d->players[c->player].backrow[c->source].card].name_group==1017&&d->phase==DUEL_BATTLE)return activate(d,c,1);return DUEL_WRONG_TURN;}
    DuelPlayer *p=&d->players[c->player];if(d->discard_required&&c->action!=DUEL_ACTION_DISCARD)return DUEL_HAND_LIMIT;
    switch(c->action){
    case DUEL_ACTION_FUSION_GATE:return fusion_gate_activate(d,c);
    case DUEL_ACTION_ACTIVATE_MONSTER:return activate_monster(d,c);
    case DUEL_ACTION_SUMMON:case DUEL_ACTION_SET_MONSTER:return summon(d,c);
    case DUEL_ACTION_ATTACK:return battle(d,c);
    case DUEL_ACTION_ACTIVATE_HAND:return activate(d,c,0);
    case DUEL_ACTION_ACTIVATE_BACKROW:return activate(d,c,1);
    case DUEL_ACTION_SET_BACKROW:
        if(!main_phase(d))return DUEL_WRONG_PHASE;return set_hand_card(d,c->player,c->source,c->target,0);
    case DUEL_ACTION_POSITION:
        if(!main_phase(d)||c->source>=5||c->position>DUEL_FACEUP_DEFENSE)return DUEL_WRONG_PHASE;
        {DuelSlot *s=&p->monsters[c->source];if(thousand_eyes_locked(d,c->player,c->source)||duel_card_prohibited(d,s->card,s->serial)||duel_binding_locked(d,c->player,c->source)||d->manual_position_lock_until[c->player]>=d->turn||!s->card||s->summoned_turn==d->turn||s->position_changed||s->attacked||s->position==c->position||(s->position_lock_until&&d->turn<=s->position_lock_until))return DUEL_NOT_ALLOWED;
            if(c->position==DUEL_FACEUP_ATTACK&&s->position!=DUEL_FACEDOWN_DEFENSE&&!continuous_position_allowed(d,c->player,c->source,c->position))return DUEL_NOT_ALLOWED;if(s->position==DUEL_FACEDOWN_DEFENSE&&!duel_summon_permission(d,c->player,DUEL_PERMISSION_NORMAL))return DUEL_NOT_ALLOWED;if(s->position==DUEL_FACEDOWN_DEFENSE&&c->position!=DUEL_FACEUP_ATTACK)return DUEL_NOT_ALLOWED;unsigned flipped=s->position==DUEL_FACEDOWN_DEFENSE;if(flipped)detach_monster(d,s->serial,0);duel_change_monster_position(d,c->player,c->source,c->position);s->position_changed=1;event(d,flipped?DUEL_EVENT_SUMMON:DUEL_EVENT_POSITION,c->player,c->source,s->card,0);
            if(flipped){if(goblin_fan_flip(d,c->player,c->source))return DUEL_OK;if(!s->serial)s->serial=++d->next_serial;s->summoned_turn=d->turn;d->window=(DuelWindow){DUEL_WINDOW_SUMMON_NEGATION,c->player,c->source,0,s->serial,0,s->card,0,1};begin_response(d,1-c->player);}return DUEL_OK;}
    case DUEL_ACTION_PHASE:
        if(c->target<=d->phase||c->target>DUEL_END)return DUEL_WRONG_PHASE;if(c->target==DUEL_BATTLE&&(d->turn==1||d->battle_skip_turn[c->player]==d->turn))return DUEL_NOT_ALLOWED;
        if(d->phase<DUEL_MAIN1&&c->target>DUEL_MAIN1)return DUEL_WRONG_PHASE;
        if(d->phase==DUEL_BATTLE){battle_phase_cleanup(d);if(weather_battle_repeat(d,c->player))return DUEL_OK;}
        if(d->phase<DUEL_STANDBY&&c->target>=DUEL_STANDBY){if(p->skip_next_standby){p->skip_next_standby=0;d->phase=DUEL_MAIN1;if(c->target==DUEL_STANDBY){event(d,DUEL_EVENT_PHASE,c->player,0,0,DUEL_MAIN1);return DUEL_OK;}}else {d->phase=DUEL_STANDBY;periodic_queue(d,1,d->active);if(d->trigger_count||d->standby_finish_actor_plus1||inspection_any(d)||patrol_any(d)){d->phase_request=c->target;event(d,DUEL_EVENT_PHASE,c->player,0,0,DUEL_STANDBY);return DUEL_OK;}}}
        d->phase=c->target;event(d,DUEL_EVENT_PHASE,c->player,0,0,0);
        if(d->phase==DUEL_BATTLE&&offering_extra_available(d,1-c->player,500)){for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[1-c->player].backrow[i];if(s->card&&d->cards[s->card].name_group==1017&&s->set_turn<d->turn&&!activation_blocked_for_slot(d,s->card,s->serial)){begin_response(d,1-c->player);break;}}}
        if(d->phase==DUEL_END)finish_turn(d);return DUEL_OK;
    case DUEL_ACTION_DISCARD:
        if(!d->discard_required||c->source>=p->hand_count)return DUEL_NOT_ALLOWED;discard_hand_effect(d,c->player,c->source,c->player,0);++d->revision;if(p->hand_count<=hand_limit(d))finish_turn(d);return DUEL_OK;
    default:return DUEL_BAD_ARGUMENT;
    }
}
int duel_command(Duel *d,const DuelCommand *c){
    if(d&&d->cards)duel_refresh_field_cache(d);
    int result=execute_command(d,c);if(result==DUEL_OK&&!d->resolving&&!d->choice.kind)check_victory_boundary(d);if(result==DUEL_OK)for(;;){advance_resolution(d);if(d->winner>=0){if(d->battle_pending)finish_battle(d);break;}if(d->response_player<2)break;
        if((!d->chain_count||d->trigger_chain_pending)&&!d->resolving)advance_effects(d);
        attack_tribute_resume(d);if(d->choice.kind||d->winner>=0||d->response_player==DUEL_NO_SLOT)break;
    }if(result==DUEL_OK)for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){DuelSlot *s=&d->players[side].backrow[i];if(s->card&&s->effect_active&&(s->equip_role||equip_rule(d,s->card))&&((s->position==DUEL_FACEDOWN_DEFENSE&&s->equip_role!=DUEL_EQUIP_ROLE_ABSORB)||!equip_host(d,s->card,s->link_serial,0,0)))destroy(d,side,i,1);}if(result==DUEL_OK){continuous_cleanup(d);control_reconcile(d);stat_links_cleanup(d);monster_equipment_prune(d);reactive_open(d);check_special_victory(d);}return result;
}
static int backup_command(const Duel *d,unsigned player,DuelCommand *out){
    DuelChoiceOption options[DUEL_PILE_MAX];unsigned count=duel_backup_options(d,player,options,DUEL_PILE_MAX);out->target_count=0;
    while(out->target_count<3){int best=-1;unsigned hands=0,fusions=0;for(unsigned i=0;i<out->target_count;++i)if(d->cards[d->players[player].grave[out->targets[i]]].subtype==2)++fusions;else ++hands;
        for(unsigned i=0;i<count;++i){unsigned used=0;for(unsigned j=0;j<out->target_count;++j)used|=out->targets[j]==options[i].index;unsigned full=d->cards[options[i].card].subtype==2?d->players[player].fusion_count+fusions>=DUEL_FUSION_MAX:d->players[player].hand_count+hands>=DUEL_HAND_MAX;if(!used&&!full&&(best<0||d->cards[options[i].card].attack>d->cards[options[best].card].attack))best=i;}
        if(best<0)break;out->targets[out->target_count++]=options[best].index;}return out->target_count>0;
}
static int call_command(const Duel *d,unsigned player,DuelCommand *out){
    const DuelPlayer *p=&d->players[player];if(!special_summon_allowed_for(d,player)||empty_zone(p->monsters)<0)return 0;int best=-1;
    for(unsigned i=0;i<p->grave_count;++i)if(duel_can_revive(d,player,i)&&(best<0||d->cards[p->grave[i]].attack>d->cards[p->grave[best]].attack))best=i;
    if(best<0)return 0;out->target_player=player;out->target=best;out->position=DUEL_FACEUP_ATTACK;return 1;
}
static int equip_command(const Duel *d,unsigned player,unsigned card,DuelCommand *out){
    if(d->cards[card].name_group==1417){for(unsigned i=0;i<=DUEL_FIELD_SLOT;++i)if(spell_equipment_target(d,1-player,i)){out->target_player=1-player;out->target=i;return 1;}return 0;}
    if(d->cards[card].family==DUEL_TRAP&&duel_field_effect_negated(d,card))return 0;if(d->cards[card].name_group==71&&(d->players[player].normal_summon_used||empty_zone(d->players[player].monsters)<0))return 0;unsigned target_side=player,op=dynamic_equip_op(d,card);if(d->cards[card].name_group==1068&&(empty_zone(d->players[player].monsters)<0||duel_field_effect_negated(d,card)))return 0;if((status_equip_flags(d,card)&101)||op==5)target_side=1-player;if(op==3&&d->players[player].life>d->players[1-player].life)target_side=1-player;int best=-1,benefit=-1;
    for(unsigned i=0;i<5;++i)if(duel_equip_can_target_for(d,player,card,target_side,i)&&!((status_equip_flags(d,card)&1)&&duel_race(d,target_side,i)==7)){int score=duel_attack_value(d,target_side,i);if(op==3){unsigned host=d->players[target_side].monsters[i].card;int base=d->cards[host].attack,current=equipment_original_attack(d,target_side,i),future=d->players[player].life<d->players[1-player].life?base*2:d->players[player].life>d->players[1-player].life?(base+1)/2:current;score=(target_side==player?1:-1)*(future-current);if(target_side==player&&d->cards[host].name_group==492)score+=500;if(score<=0)continue;}if(best<0||score>benefit){best=i;benefit=score;}}
    if(best<0){if(d->cards[card].name_group==650&&kunai_position_available(d,player)){out->cost=1;return 1;}return 0;}out->target_player=target_side;out->target=best;if(d->cards[card].name_group==650)out->cost=kunai_position_available(d,player)?3:2;return 1;
}
static int field_score(const Duel *d,unsigned player,unsigned card){
    const DuelFieldRule *r=field_rule(d,card);if(!r)return 0;int score=0;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[side].monsters[i];if(!s->card||s->position==DUEL_FACEDOWN_DEFENSE)continue;const DuelCardDef *c=&d->cards[s->card];unsigned selector=r->kind==DUEL_FIELD_RACE?duel_race(d,side,i):duel_attribute(d,side,i);int factor=r->kind==DUEL_FIELD_DEFENSE?s->position==DUEL_FACEUP_DEFENSE:selector<32?(r->positive&(1u<<selector)?1:r->negative&(1u<<selector)?-1:0):0;int value=(s->position==DUEL_FACEUP_DEFENSE?r->defense:r->attack)*factor;score+=side==player?value:-value;}return score;
}
static int field_improves(const Duel *d,unsigned player,unsigned card){if(d->cards[card].name_group==1547)return fusion_options(d,player,0,0,DUEL_NO_SLOT)!=0;
    int old=0;for(unsigned side=0;side<2;++side){const DuelSlot *s=&d->players[side].field;if(s->card&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE)old+=field_score(d,player,s->card);}return field_score(d,player,card)>old;
}
int duel_ai_decide(const Duel *d,unsigned player,DuelCommand *out){
    if(!d||!out||player>1||d->winner>=0)return 0;clear(out,sizeof(*out));out->player=player;
    if(d->choice.kind){if(d->choice.player!=player)return 0;DuelChoiceOption choices[DUEL_OPTION_MAX];unsigned n=duel_choice_options(d,choices,DUEL_OPTION_MAX);if(!n)return 0;unsigned best=0;
        if(d->choice.kind==DUEL_CHOICE_ATTACK_TRIBUTE_COST){int score=2147483647;for(unsigned i=0;i<n;++i){if(choices[i].kind!=DUEL_OPTION_CARD)continue;unsigned token=choices[i].index;int value=token/5==player?duel_attack_value(d,player,token%5):-100000-duel_attack_value(d,token/5,token%5);if(value<score){score=value;best=i;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_ATTACK_TRIBUTE_TARGET){int score=2147483647;for(unsigned i=0;i<n;++i){if(choices[i].kind==DUEL_OPTION_SKIP)continue;int value=choices[i].index==5?-100000:choices[i].card?duel_attack_value(d,1-player,choices[i].index):1500;if(value<score){score=value;best=i;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_BATTLE_REVIVE||d->choice.kind==DUEL_CHOICE_BATTLE_REVIVE_POSITION){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(offering_choice(d->choice.kind)){unsigned kind=d->choice.kind;if(kind==DUEL_CHOICE_OFFERING_HAND){for(unsigned i=1;i<n;++i)if(choices[i].kind==DUEL_OPTION_CARD&&d->cards[choices[i].card].attack>d->cards[choices[best].card].attack)best=i;}if(kind==DUEL_CHOICE_OFFERING_TRIBUTE){int score=2147483647;for(unsigned i=0;i<n;++i)if(choices[i].kind==DUEL_OPTION_CARD){unsigned token=choices[i].index;int value=token<5?duel_attack_value(d,player,token):-100000-duel_attack_value(d,1-player,token%5);if(value<score){score=value;best=i;}}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_SUMMON_PROCEDURE_COST){int score=2147483647;for(unsigned i=0;i<n;++i){if(choices[i].kind!=DUEL_OPTION_CARD)continue;unsigned token=choices[i].index;int value=token/5==player?duel_attack_value(d,player,token%5):-100000-duel_attack_value(d,token/5,token%5);if(value<score){score=value;best=i;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_KUNAI_MODE||d->choice.kind==DUEL_CHOICE_KUNAI_TARGET){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;for(unsigned i=0;i<n;++i)if(d->choice.kind==DUEL_CHOICE_KUNAI_MODE&&choices[i].index==3)out->source=3;return 1;}
        if(d->choice.kind==DUEL_CHOICE_EQUIP_TRANSFER_SOURCE||d->choice.kind==DUEL_CHOICE_EQUIP_TRANSFER_TARGET){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(coordinate_choice(d->choice.kind)){unsigned ground=d->choice.kind==DUEL_CHOICE_GROUND_FIRST||d->choice.kind==DUEL_CHOICE_GROUND_SECOND;int score=-2147483647;for(unsigned i=0;i<n;++i)if(choices[i].kind!=DUEL_OPTION_SKIP){unsigned t=choices[i].index;int value=ground?(t/5!=player?100:0):(t/5==player?-1:1)*duel_attack_value(d,t/5,t%5);if(value>score){score=value;best=i;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_ACTIVATION_PARAMETER){unsigned name=d->cards[d->choice.card].name_group;if(name==1162){const DuelPlayer *enemy=&d->players[1-player];unsigned target=enemy->hand_count?enemy->hand[d->random_state%enemy->hand_count]:0;for(unsigned k=1;!target&&k<DUEL_CARD_COUNT;++k)if(d->cards[k].name_group==1008)target=k;unsigned page=(target-1)/128;out->action=DUEL_ACTION_CHOOSE;out->source=page==d->choice.selected_count?target-page*128:page>d->choice.selected_count?254:253;return 1;}int score=-2147483647;for(unsigned i=0;i<n;++i){if(choices[i].kind==DUEL_OPTION_SKIP)continue;int value=0;if(name==1146)for(unsigned p=0;p<2;++p)for(unsigned j=0;j<5;++j){const DuelSlot *m=&d->players[p].monsters[j];if(m->card&&m->position!=DUEL_FACEDOWN_DEFENSE&&duel_race(d,p,j)==choices[i].index)value+=(p==player?-1:1)*duel_attack_value(d,p,j);}if(value>score){best=i;score=value;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_PEEK_VIEW){out->action=DUEL_ACTION_CHOOSE;out->source=DUEL_NO_SLOT;return 1;}
        if(d->choice.kind==DUEL_CHOICE_PEEK_HAND){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_LAIR_TARGET){int best=0,score=-2147483647;for(unsigned i=0;i<n;++i){unsigned token=choices[i].index,owner=token/5;int value=(owner==player?-10000:10000)+d->cards[choices[i].card].attack;if(value>score){score=value;best=i;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_PATROL_TARGET){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_INSPECTION_OFFER){out->action=DUEL_ACTION_CHOOSE;out->source=d->players[player].life>2000&&d->players[player].backrow[d->choice.selected_count].ignition_turn!=d->turn?0:DUEL_NO_SLOT;return 1;}
        if(d->choice.kind==DUEL_CHOICE_YADO_HAND){out->action=DUEL_ACTION_CHOOSE;out->source=DUEL_NO_SLOT;return 1;}
        if(hand_selection_kind(d->choice.kind)){out->action=DUEL_ACTION_CHOOSE;out->source=hand_selection_ai_choice(d);return 1;}
        if(destruction_choice_kind(d->choice.kind)){out->action=DUEL_ACTION_CHOOSE;out->source=destruction_ai_choice(d);return 1;}
        if(d->choice.kind==DUEL_CHOICE_LAIR_OFFER){out->action=DUEL_ACTION_CHOOSE;out->source=0;return 1;}
        if(d->choice.kind==DUEL_CHOICE_PILE_CARD&&d->pile_task.op==41&&d->chain[d->pile_task.root].command.position){out->action=DUEL_ACTION_CHOOSE;out->source=DUEL_NO_SLOT;return 1;}
        if(d->choice.kind==DUEL_CHOICE_PILE_CARD&&(d->pile_task.op==42||d->pile_task.op==43)){unsigned paid=d->chain[d->pile_task.root].command.cost,wanted=3;if(d->pile_task.op==42&&d->chain[d->pile_task.root].command.position){unsigned room=monsters(&d->players[player])+paid;if(room>0)--room;if(wanted>room)wanted=room;}if(d->pile_task.op==43){int power=-1;wanted=1;unsigned available=grave_cost_count(d,player)+paid;for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[1-player].monsters[i];if(s->card&&s->position!=DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].level&&d->cards[s->card].level<=available&&d->cards[s->card].attack>power){power=d->cards[s->card].attack;wanted=d->cards[s->card].level;}}}int best=-1;for(unsigned i=0;i<n;++i){if(choices[i].kind==DUEL_OPTION_SKIP&&paid>=wanted){out->action=DUEL_ACTION_CHOOSE;out->source=DUEL_NO_SLOT;return 1;}if(choices[i].kind==DUEL_OPTION_CARD&&!(d->pile_task.op==42&&d->chain[d->pile_task.root].command.position&&choices[i].index==d->chain[d->pile_task.root].source_slot)&&(best<0||d->cards[choices[i].card].attack<d->cards[choices[best].card].attack))best=i;}if(best>=0){out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}}
        if(d->choice.kind==DUEL_CHOICE_OPTIONAL_SET){out->action=DUEL_ACTION_CHOOSE;out->source=d->players[player].life<=(int)hand_play_cost(d)?DUEL_NO_SLOT:choices[0].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_PILE_PLAYER){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(borrow_hats_choice(d->choice.kind)){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_MAJOR_RIOT){unsigned best=0;for(unsigned i=1;i<n;++i)if(d->cards[choices[i].card].attack>d->cards[choices[best].card].attack)best=i;out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(excavation_choice(d->choice.kind)){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_SPECIAL_HAND_POSITION){out->action=DUEL_ACTION_CHOOSE;out->source=DUEL_FACEUP_ATTACK;return 1;}
        if(d->choice.kind==DUEL_CHOICE_SPECIAL_HAND_COST){if(duel_hand_special_cost_is_field(&d->cards[d->choice.card])==2&&empty_zone(d->players[player].monsters)<0){for(unsigned i=0;i<n;++i)if(choices[i].index>=DUEL_HAND_MAX&&choices[i].index<DUEL_HAND_MAX+5){out->action=DUEL_ACTION_CHOOSE;out->source=choices[i].index;return 1;}}for(unsigned i=1;i<n;++i)if(d->cards[choices[i].card].attack<d->cards[choices[best].card].attack)best=i;out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_IGNITION_POSITION){out->action=DUEL_ACTION_CHOOSE;out->source=DUEL_FACEUP_ATTACK;return 1;}
        if(d->choice.kind==DUEL_CHOICE_IGNITION_FUSION){for(unsigned i=1;i<n;++i)if(d->cards[choices[i].card].attack>d->cards[choices[best].card].attack)best=i;out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_TRIBUTE_UPKEEP){out->action=DUEL_ACTION_CHOOSE;int best=-1;for(unsigned i=0;i<n;++i)if(choices[i].kind==DUEL_OPTION_CARD&&(best<0||duel_attack_value(d,tribute_owner(player,choices[i].index),choices[i].index%5)<duel_attack_value(d,tribute_owner(player,choices[best].index),choices[best].index%5)))best=i;out->source=best<0?DUEL_NO_SLOT:choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_MUSHROOM){out->action=DUEL_ACTION_CHOOSE;out->source=0;return 1;}
        if(d->choice.kind==DUEL_CHOICE_UPKEEP){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_MAGIC_DRAIN){out->action=DUEL_ACTION_CHOOSE;out->source=choices[0].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_FLIP_OFFER){out->action=DUEL_ACTION_CHOOSE;out->source=d->players[player].life>2000&&(d->players[player].life>d->players[1-player].life+1000||d->players[1-player].life<=1000)?0:DUEL_NO_SLOT;return 1;}
        if(d->choice.kind==DUEL_CHOICE_HISTORY_OFFER||d->choice.kind==DUEL_CHOICE_HISTORY_TARGET){DuelChoiceOption options[DUEL_OPTION_MAX];unsigned n=history_options(d,options,DUEL_OPTION_MAX);if(!n)return 0;out->action=DUEL_ACTION_CHOOSE;out->source=options[0].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_PERIODIC){out->action=DUEL_ACTION_CHOOSE;out->source=0;return 1;}
        if(d->choice.kind==DUEL_CHOICE_DECK_EQUIP){int power=-2147483647;for(unsigned i=0;i<n;++i){int value=choices[i].kind==DUEL_OPTION_CARD?(choices[i].index/5==player?-100000:100000)+duel_attack_value(d,choices[i].index/5,choices[i].index%5):0;if(value>power){power=value;best=i;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_EQUIP_RECYCLE){const DuelPlayer *p=&d->players[player];unsigned kind=equip_grave_kind(d,d->choice.card),index=DUEL_NO_SLOT;if(kind==3&&p->life>2000)index=0;if(kind>=5){int power=-2147483647;for(unsigned j=0;j<n;++j)if(choices[j].kind==DUEL_OPTION_CARD&&choices[j].index/5!=player){int value=duel_attack_value(d,choices[j].index/5,choices[j].index%5);if(value>power){power=value;index=choices[j].index;}}}if(kind==4){int weakest=-1;for(unsigned i=0;i<5;++i)if(p->monsters[i].card&&(weakest<0||duel_attack_value(d,player,i)<duel_attack_value(d,player,weakest)))weakest=i;if(weakest>=0&&duel_attack_value(d,player,weakest)<1000)index=weakest;}out->action=DUEL_ACTION_CHOOSE;out->source=index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_HAND){unsigned largest=d->hand_task.owner!=player;for(unsigned i=1;i<n;++i)if(choices[i].kind==DUEL_OPTION_CARD&&(largest?d->cards[choices[i].card].attack>d->cards[choices[best].card].attack:d->cards[choices[i].card].attack<d->cards[choices[best].card].attack))best=i;out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_STAT_SOURCE||d->choice.kind==DUEL_CHOICE_STAT_DESTINATION){unsigned wanted=d->choice.kind==DUEL_CHOICE_STAT_SOURCE?1-player:player;int score=-2147483647;for(unsigned i=0;i<n;++i){if(choices[i].kind!=DUEL_OPTION_CARD)continue;unsigned token=choices[i].index;int value=duel_attack_value(d,token/5,token%5)+(token/5==wanted?100000:0);if(value>score){best=i;score=value;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_DISCARD_HAND){for(unsigned i=1;i<n;++i)if(d->cards[choices[i].card].attack<d->cards[choices[best].card].attack)best=i;out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_FUSION_POSITION||d->choice.kind==DUEL_CHOICE_RITUAL_POSITION){out->action=DUEL_ACTION_CHOOSE;out->source=DUEL_FACEUP_ATTACK;return 1;}
        if(d->choice.kind==DUEL_CHOICE_FUSION_MATERIAL){for(unsigned i=1;i<n;++i){unsigned a=d->cards[choices[i].card].attack+(choices[i].index<DUEL_HAND_MAX?200:0)+(d->cards[choices[i].card].effect==DUEL_E_FUSION_SUBSTITUTE?10000:0),b=d->cards[choices[best].card].attack+(choices[best].index<DUEL_HAND_MAX?200:0)+(d->cards[choices[best].card].effect==DUEL_E_FUSION_SUBSTITUTE?10000:0);if(a<b)best=i;}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(d->choice.kind==DUEL_CHOICE_OPTIONAL_DRAW){out->action=DUEL_ACTION_CHOOSE;out->source=d->players[player].deck_count>2?0:DUEL_NO_SLOT;return 1;}
        if(d->choice.kind==DUEL_CHOICE_ATTACK_REPLAY){int target=-1,value=0x7fffffff,power=duel_attack_value(d,player,d->deferred_attack.source);
            for(unsigned i=0;i<n;++i){if(choices[i].kind==DUEL_OPTION_CONFIRM){target=choices[i].index;break;}if(choices[i].kind!=DUEL_OPTION_CARD)continue;unsigned slot=choices[i].index;const DuelSlot *s=&d->players[1-player].monsters[slot];int strength=s->position==DUEL_FACEDOWN_DEFENSE?1500:s->position==DUEL_FACEUP_ATTACK?duel_attack_value(d,1-player,slot):duel_defense_value(d,1-player,slot);if(strength<=power&&strength<value){target=slot;value=strength;}}
            out->action=DUEL_ACTION_CHOOSE;out->source=target<0?DUEL_NO_SLOT:target;return 1;}
        if(d->choice.kind==DUEL_CHOICE_TARGET_FLIP){int best=-1,value=-2147483647;const TargetFlipRule *r=target_flip_rule(d,d->choice.card);for(unsigned i=0;i<n;++i){unsigned token=choices[i].index,side=token/11,index=token%11;int score=side==player?-10000:10000;if(r->op>=10)score+=duel_attack_value(d,side,index);if(r->op==1){const DuelSlot *s=&d->players[side].monsters[index];score=(side==player?1:-1)*(s->position==DUEL_FACEUP_ATTACK?-1:1)*duel_attack_value(d,side,index);}if(score>value){value=score;best=i;}}out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
        if(flip_target_choice(d)){int best=-1,value=-1;for(unsigned i=0;i<n;++i)if(choices[i].kind==DUEL_OPTION_CARD&&choices[i].index/5!=player){unsigned side=choices[i].index/5,index=choices[i].index%5;int power=d->players[side].monsters[index].position==DUEL_FACEDOWN_DEFENSE?1500:duel_attack_value(d,side,index);if(power>value){best=i;value=power;}}
            if(best<0&&d->choice.kind==DUEL_CHOICE_FLIP_TARGET){value=0x7fffffff;for(unsigned i=0;i<n;++i)if(choices[i].kind==DUEL_OPTION_CARD){int power=duel_attack_value(d,player,choices[i].index%5);if(power<value){best=i;value=power;}}}
            out->action=DUEL_ACTION_CHOOSE;out->source=best<0?DUEL_NO_SLOT:choices[best].index;return 1;}
        for(unsigned i=1;i<n;++i)if(d->cards[choices[i].card].attack>d->cards[choices[best].card].attack)best=i;
        out->action=DUEL_ACTION_CHOOSE;out->source=choices[best].index;return 1;}
    if(d->response_player<2){
        if(d->response_player!=player)return 0;DuelActivationOption choices[DUEL_OPTION_MAX];unsigned count=duel_response_options(d,player,choices,DUEL_OPTION_MAX);
        for(unsigned i=0;i<count;++i){const DuelActivationOption *option=&choices[i];unsigned effect=d->cards[option->card].effect;out->action=option->from_backrow?DUEL_ACTION_ACTIVATE_BACKROW:DUEL_ACTION_ACTIVATE_HAND;out->source=option->index;if(d->window.kind==DUEL_WINDOW_DAMAGE_LIFE&&d->cards[option->card].name_group==57)return 1;if(option->from_backrow==2){out->action=DUEL_ACTION_ACTIVATE_MONSTER;return 1;}
            if(duel_field_effect_negated(d,option->card))continue;
            if(effect==DUEL_E_COUNTER&&((d->chain_count&&d->chain[d->chain_count-1].command.player!=player)||(!d->chain_count&&d->window.kind==DUEL_WINDOW_SUMMON_NEGATION&&d->window.player!=player))&&counter_ai_cost(d,player,option->card,out))return 1;
            if(effect==DUEL_E_EQUIP){if(duel_field_effect_negated(d,option->card))continue;const DuelWindow *w=&d->window;unsigned target=w->player==player?w->source:w->target;if(w->kind==DUEL_WINDOW_ATTACK&&target<5&&duel_equip_can_target(d,option->card,player,target)){out->target_player=player;out->target=target;return 1;}if(equip_command(d,player,option->card,out))return 1;}
            if(effect==DUEL_E_PERIODIC&&periodic_validate(d,player,option->card)==DUEL_OK)return 1;
            if(effect==DUEL_E_SIMPLE&&simple_ai_command(d,player,out,option->card))return 1;
            if(effect==DUEL_E_CONTINUOUS){if(option->from_backrow&&d->players[player].backrow[option->index].position!=DUEL_FACEDOWN_DEFENSE&&continuous_ability_ai_wants(d,player,option->index))return 1;const ContinuousRule *r=continuous_rule(d,option->card);if(r->op==32&&d->players[player].backrow[option->index].position==DUEL_FACEDOWN_DEFENSE&&offering_ai_wants(d,player))return 1;if(r->op==36&&d->window.kind==DUEL_WINDOW_ATTACK&&d->window.player!=player)return 1;if(r->op==4&&d->window.kind==DUEL_WINDOW_ATTACK&&d->window.player!=player&&d->cards[d->window.card].level>=4)return 1;if(r->op==5&&d->chain_count&&d->chain[d->chain_count-1].command.player!=player&&d->cards[d->chain[d->chain_count-1].card].effect==DUEL_E_RITUAL)return 1;}
            if(effect==DUEL_E_TRAP_LOCK)return 1;
            if(effect==DUEL_E_MAGIC_CYLINDER)return 1;
            if(effect==DUEL_E_BACKUP&&backup_command(d,player,out))return 1;
            if(effect==DUEL_E_CALL_HAUNTED&&call_command(d,player,out))return 1;
            if(effect==DUEL_E_CHAIN_DESTRUCTION&&d->window.player!=player){out->target_player=d->window.player;out->target=d->window.source;return 1;}
            if(effect==DUEL_E_MAGIC_JAMMER&&d->chain_count&&d->chain[d->chain_count-1].command.player!=player){out->cost=0;for(unsigned j=1;j<d->players[player].hand_count;++j)if(d->cards[d->players[player].hand[j]].attack<d->cards[d->players[player].hand[out->cost]].attack)out->cost=j;return 1;}
            if(effect==DUEL_E_TORRENTIAL){unsigned own=monsters(&d->players[player]),enemy=monsters(&d->players[1-player]);if(enemy>own)return 1;}
            if(effect==DUEL_E_GRACEFUL_DICE&&d->window.kind==DUEL_WINDOW_ATTACK)return 1;
            if(effect==DUEL_E_BIND){if(d->window.kind==DUEL_WINDOW_ATTACK&&duel_binding_can_target(d,player,option->card,d->window.player,d->window.source)&&!duel_binding_locked(d,d->window.player,d->window.source)){out->target_player=d->window.player;out->target=d->window.source;return 1;}if(binding_ai_target(d,player,option->card,out))return 1;}
            if(effect==DUEL_E_MST){for(unsigned j=0;j<=DUEL_FIELD_SLOT;++j){const DuelSlot *s=j==DUEL_FIELD_SLOT?&d->players[1-player].field:&d->players[1-player].backrow[j];if(s->card&&(s->position==DUEL_FACEDOWN_DEFENSE||s->effect_active||j==DUEL_FIELD_SLOT)){out->target_player=1-player;out->target=j;return 1;}}}
        }
        clear(out,sizeof(*out));out->player=player;out->action=DUEL_ACTION_PASS;return 1;
    }
    if(d->active!=player)return 0;
    const DuelPlayer *p=&d->players[player],*enemy=&d->players[1-player];
    if(d->phase==DUEL_STANDBY){for(unsigned i=0;i<5;++i)if(duel_patrol_ready(d,player,i)){out->action=DUEL_ACTION_ACTIVATE_MONSTER;out->source=i;return 1;}for(unsigned i=0;i<5;++i)if(blast_ai_command(d,player,i,out))return 1;for(unsigned from=0;from<2;++from)for(unsigned i=0;i<(from?5:p->hand_count);++i)if(curse_standby_available(d,player,i,from)){out->action=from?DUEL_ACTION_ACTIVATE_BACKROW:DUEL_ACTION_ACTIVATE_HAND;out->source=i;return 1;}}
    if(d->discard_required){out->action=DUEL_ACTION_DISCARD;unsigned worst=0;for(unsigned i=1;i<p->hand_count;++i)if(d->cards[p->hand[i]].attack<d->cards[p->hand[worst]].attack)worst=i;out->source=worst;return 1;}
    if(d->phase<DUEL_MAIN1){out->action=DUEL_ACTION_PHASE;out->target=d->phase+1;return 1;}
    if(d->phase==DUEL_MAIN2)for(unsigned i=0;i<5;++i)if(p->monsters[i].card&&history_return_name(d->cards[p->monsters[i].card].name_group)&&ignition_ai_command(d,player,i,out))return 1;
    if(d->phase==DUEL_MAIN1){
        for(unsigned side=0;side<2;++side)if(duel_fusion_gate_available(d,player,side)){out->action=DUEL_ACTION_FUSION_GATE;out->source=side;return 1;}
        for(unsigned i=0;i<5;++i)if(continuous_ability_ai_wants(d,player,i)&&!duel_card_activation_blocked(d,p->backrow[i].card)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;return 1;}
        int slot=empty_zone(p->monsters),best=-1;unsigned mask=0,available=monsters(p),tribute_zone=0;for(unsigned i=0;i<5;++i)tribute_zone|=!p->monsters[i].zone_blocked&&duel_can_normal_tribute(d,player,player,i);
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&d->cards[s->card].effect==DUEL_E_BIND&&s->position==DUEL_FACEDOWN_DEFENSE&&s->set_turn<d->turn&&!activation_blocked_for_slot(d,s->card,s->serial)&&!duel_field_effect_negated(d,s->card)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;if(binding_ai_target(d,player,s->card,out))return 1;}}
        for(unsigned i=0;i<5;++i)if(duel_equipment_deck_special(d,player,i)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;return 1;}
        for(unsigned i=0;i<5;++i)if(ignition_ai_command(d,player,i,out))return 1;
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&d->cards[s->card].effect==DUEL_E_BACKUP&&s->position==DUEL_FACEDOWN_DEFENSE&&s->set_turn<d->turn&&!activation_blocked_for_slot(d,s->card,s->serial)&&!duel_field_effect_negated(d,s->card)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;if(backup_command(d,player,out))return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&d->cards[s->card].effect==DUEL_E_CALL_HAUNTED&&s->position==DUEL_FACEDOWN_DEFENSE&&s->set_turn<d->turn&&!activation_blocked_for_slot(d,s->card,s->serial)&&!duel_field_effect_negated(d,s->card)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;if((d->cards[s->card].name_group!=1160||p->life>800)&&call_command(d,player,out))return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&d->cards[s->card].effect==DUEL_E_TRAP_LOCK&&s->position==DUEL_FACEDOWN_DEFENSE&&s->set_turn<d->turn&&!activation_blocked_for_slot(d,s->card,s->serial)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;return 1;}}
        if(p->field.card&&!activation_blocked_for_slot(d,p->field.card,p->field.serial)&&fragrance_allows(d,player,DUEL_FIELD_SLOT,1,p->field.card)&&p->field.position==DUEL_FACEDOWN_DEFENSE&&field_improves(d,player,p->field.card)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=DUEL_FIELD_SLOT;return 1;}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&d->cards[s->card].effect==DUEL_E_EQUIP&&s->position==DUEL_FACEDOWN_DEFENSE&&(d->cards[s->card].family!=DUEL_TRAP||s->set_turn<d->turn)&&!activation_blocked_for_slot(d,s->card,s->serial)&&fragrance_allows(d,player,i,1,s->card)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;if(equip_command(d,player,s->card,out))return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&!activation_blocked_for_slot(d,s->card,s->serial)&&d->cards[s->card].effect==DUEL_E_LIFE&&s->position==DUEL_FACEDOWN_DEFENSE&&duel_life_rule(d,s->card)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;out->target_player=player;return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&!activation_blocked_for_slot(d,s->card,s->serial)&&fragrance_allows(d,player,i,1,s->card)&&s->position==DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].effect==DUEL_E_SIMPLE&&(spell_speed(&d->cards[s->card])==1||s->set_turn<d->turn)&&!activation_blocked_for_slot(d,s->card,s->serial)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;if(simple_ai_command(d,player,out,s->card))return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&!activation_blocked_for_slot(d,s->card,s->serial)&&fragrance_allows(d,player,i,1,s->card)&&s->position==DUEL_FACEDOWN_DEFENSE&&s->set_turn<d->turn&&d->cards[s->card].effect==DUEL_E_PERIODIC&&!activation_blocked_for_slot(d,s->card,s->serial)&&periodic_validate(d,player,s->card)==DUEL_OK){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&!activation_blocked_for_slot(d,s->card,s->serial)&&fragrance_allows(d,player,i,1,s->card)&&s->position==DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].effect==DUEL_E_HAND&&hand_ai_wants(d,player,s->card,1)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&!activation_blocked_for_slot(d,s->card,s->serial)&&fragrance_allows(d,player,i,1,s->card)&&s->position==DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].effect==DUEL_E_SEARCH&&search_options(d,player,s->card,0,0)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&!activation_blocked_for_slot(d,s->card,s->serial)&&fragrance_allows(d,player,i,1,s->card)&&s->position==DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].effect==DUEL_E_RITUAL&&ritual_monsters(d,player,s->card,0,0)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->backrow[i];if(s->card&&!activation_blocked_for_slot(d,s->card,s->serial)&&fragrance_allows(d,player,i,1,s->card)&&s->position==DUEL_FACEDOWN_DEFENSE&&d->cards[s->card].effect==DUEL_E_CONTINUOUS&&s->set_turn<d->turn&&!activation_blocked_for_slot(d,s->card,s->serial)&&continuous_ai_wants(d,player,s->card)){out->action=DUEL_ACTION_ACTIVATE_BACKROW;out->source=i;return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->monsters[i];if(s->card&&!thousand_eyes_locked(d,player,i)&&!duel_card_prohibited(d,s->card,s->serial)&&d->cards[s->card].effect==DUEL_E_TARGET_FLIP&&target_flip_rule(d,s->card)->op==6&&s->position==DUEL_FACEUP_ATTACK&&monsters(enemy)&&!s->attacked&&!s->position_changed&&s->summoned_turn!=d->turn&&d->manual_position_lock_until[player]<d->turn&&!duel_binding_locked(d,player,i)&&(!s->position_lock_until||d->turn>s->position_lock_until)){out->action=DUEL_ACTION_POSITION;out->source=i;out->position=DUEL_FACEUP_DEFENSE;return 1;}}
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->monsters[i];if(s->card&&!thousand_eyes_locked(d,player,i)&&!duel_card_prohibited(d,s->card,s->serial)&&(s->position==DUEL_FACEUP_DEFENSE||(s->position==DUEL_FACEDOWN_DEFENSE&&duel_summon_permission(d,player,DUEL_PERMISSION_NORMAL)&&(flip_card(d,s->card)||(monster_abilities_for(&d->cards[s->card])&128))&&(d->cards[s->card].effect==DUEL_E_GRAVE_RECALL?recall_available(d,player,s->card):d->cards[s->card].effect==DUEL_E_PENGUIN||d->cards[s->card].effect==DUEL_E_MORPH2||monsters(enemy))))&&!s->attacked&&!s->position_changed&&s->summoned_turn!=d->turn&&d->manual_position_lock_until[player]<d->turn&&!duel_binding_locked(d,player,i)&&(!s->position_lock_until||d->turn>s->position_lock_until)){
            out->action=DUEL_ACTION_POSITION;out->source=i;out->position=DUEL_FACEUP_ATTACK;return 1;}}
        for(unsigned i=0;i<p->hand_count;++i)if(special_hand_can_start(d,player,i)&&gila_ai_special_preferred(d,player,p->hand[i])){out->action=DUEL_ACTION_ACTIVATE_HAND;out->source=i;return 1;}
        for(unsigned i=0;can_pay_action(d,player,hand_play_cost(d))&&i<p->hand_count;++i){unsigned effect=d->cards[p->hand[i]].effect;if(duel_card_activation_blocked(d,p->hand[i])||(d->cards[p->hand[i]].family!=DUEL_SPELL&&d->cards[p->hand[i]].name_group!=71)||(effect!=DUEL_E_FIELD&&empty_zone(p->backrow)<0))continue;
            if(!fragrance_allows(d,player,i,0,p->hand[i])){out->action=DUEL_ACTION_SET_BACKROW;out->source=i;out->target=effect==DUEL_E_FIELD?DUEL_FIELD_SLOT:empty_zone(p->backrow);return 1;}
            out->action=DUEL_ACTION_ACTIVATE_HAND;out->source=i;
            if(effect==DUEL_E_SIMPLE){if(simple_ai_command(d,player,out,p->hand[i]))return 1;continue;}
            if(effect==DUEL_E_RITUAL){if(ritual_monsters(d,player,p->hand[i],0,0))return 1;continue;}
            if(effect==DUEL_E_CONTINUOUS){if(fragrance_allows(d,player,i,0,p->hand[i])&&empty_zone(p->backrow)>=0&&continuous_ai_wants(d,player,p->hand[i]))return 1;continue;}
            if(effect==DUEL_E_LIFE&&empty_zone(p->backrow)>=0&&duel_life_rule(d,p->hand[i])){out->target_player=player;return 1;}
            if(effect==DUEL_E_FIELD&&field_improves(d,player,p->hand[i]))return 1;
            if(effect==DUEL_E_EQUIP&&empty_zone(p->backrow)>=0&&equip_command(d,player,p->hand[i],out))return 1;
            if(effect==DUEL_E_HAND&&hand_ai_wants(d,player,p->hand[i],0))return 1;
            if(effect==DUEL_E_SEARCH&&search_options(d,player,p->hand[i],0,0))return 1;
            if(effect==DUEL_E_DRAW2)return 1;
            if(effect==DUEL_E_SAFE_RETURN&&empty_zone(p->backrow)>=0)return 1;
            if(effect==DUEL_E_POLYMERIZATION&&empty_zone(p->backrow)>=0&&fusion_options(d,player,0,0,i))return 1;
            if(effect==DUEL_E_CALL_HAUNTED&&d->cards[p->hand[i]].name_group==1160&&p->life>800+(int)hand_play_cost(d)&&call_command(d,player,out))return 1;
            if(effect==DUEL_E_REBORN&&slot>=0&&special_summon_allowed_for(d,player)){int best_card=-1;
                for(unsigned owner=0;owner<2;++owner)for(unsigned j=0;j<d->players[owner].grave_count;++j){unsigned card=d->players[owner].grave[j];const DuelCardDef *c=&d->cards[card];if(duel_can_revive_for(d,player,owner,j)&&(best_card<0||c->attack>d->cards[best_card].attack)){best_card=card;out->target_player=owner;out->target=j;}}
                if(best_card>=0)return 1;
            }
            if(effect==DUEL_E_RAIGEKI&&monsters(enemy))return 1;
            if(effect==DUEL_E_FEATHER_DUSTER){if(enemy->field.card)return 1;for(unsigned j=0;j<5;++j)if(enemy->backrow[j].card)return 1;}
            if(effect==DUEL_E_FISSURE){for(unsigned j=0;j<5;++j)if(enemy->monsters[j].card&&enemy->monsters[j].position!=DUEL_FACEDOWN_DEFENSE)return 1;}
            if(effect==DUEL_E_MST||effect==DUEL_E_DE_SPELL||effect==DUEL_E_REMOVE_TRAP){for(unsigned j=0;j<=DUEL_FIELD_SLOT;++j){const DuelSlot *t=j==DUEL_FIELD_SLOT?&enemy->field:&enemy->backrow[j];if(t->card&&(effect==DUEL_E_MST||t->position!=DUEL_FACEDOWN_DEFENSE)&&
                    (effect!=DUEL_E_REMOVE_TRAP||d->cards[t->card].family==DUEL_TRAP)&&(effect!=DUEL_E_DE_SPELL||d->cards[t->card].family!=DUEL_TRAP)){out->target_player=1-player;out->target=j;return 1;}}}
            if(effect==DUEL_E_CHANGE_HEART&&slot>=0&&d->turn>1){int strongest=-1;for(unsigned j=0;j<5;++j)if(enemy->monsters[j].card&&duel_can_target_monster(d,1-player,j)&&effect_affects_monster(d,p->hand[i],1-player,j)&&enemy->monsters[j].position!=DUEL_FACEDOWN_DEFENSE&&(strongest<0||duel_attack_value(d,1-player,j)>duel_attack_value(d,1-player,strongest)))strongest=j;
                if(strongest>=0&&!(d->cards[enemy->monsters[strongest].card].name_group==483&&!enemy->monsters[strongest].control_used&&p->life<=2000)){out->target_player=1-player;out->target=strongest;return 1;}}
            if(effect==DUEL_E_GRACEFUL_DICE&&available&&d->turn>1)return 1;
            if(effect==DUEL_E_TRIBUTE_DOOMED&&p->hand_count>1&&monsters(enemy)){unsigned discard=i?0:1;int target=-1;
                int highest=-1;for(unsigned j=0;j<5;++j)if(enemy->monsters[j].card&&duel_can_target_monster(d,1-player,j)&&effect_affects_monster(d,p->hand[i],1-player,j)){int value=enemy->monsters[j].position==DUEL_FACEDOWN_DEFENSE?1500:duel_attack_value(d,1-player,j);if(value>highest){highest=value;target=j;}}
                if(target>=0){out->target_player=1-player;out->target=target;out->cost=discard;return 1;}}
        }
        for(unsigned i=0;i<p->hand_count;++i)if(duel_hand_monster_effect(&d->cards[p->hand[i]])&&search_options(d,player,p->hand[i],0,0)){out->action=DUEL_ACTION_ACTIVATE_HAND;out->source=i;return 1;}
        /* Candidate spell targets must not leak into the next summon command. */
        clear(out,sizeof(*out));out->player=player;
        if(!p->normal_summon_used&&can_pay_action(d,player,hand_play_cost(d)))for(unsigned i=0;i<p->hand_count;++i){const DuelCardDef *c=&d->cards[p->hand[i]];unsigned need=duel_normal_tribute_cost(d,player,i,flip_card(d,p->hand[i]));
            if(!duel_card_prohibited(d,p->hand[i],0)&&duel_summon_permission(d,player,live_monster_name(d,1526,3)||c->name_group==1201||attack_equipment_name(c->name_group)||flip_card(d,p->hand[i])||(monster_abilities_for(c)&128)?DUEL_PERMISSION_SET:DUEL_PERMISSION_NORMAL)&&c->family==DUEL_MONSTER&&c->subtype<2&&!special_hand_provenance(c)&&duel_card_supported(c)&&(!need||!continuous_count(d,5))&&normal_tribute_available(d,player)>=need&&(slot>=0||(need&&tribute_zone))&&(best<0||c->attack>d->cards[p->hand[best]].attack))best=i;}
        if(best>=0){unsigned need=duel_normal_tribute_cost(d,player,best,flip_card(d,p->hand[best]));
            while(need--){int weakest=-1,score=2147483647;for(unsigned token=0;token<10;++token){unsigned owner=tribute_owner(player,token),i=token%5;if(mask&(1u<<token)||!duel_can_normal_tribute(d,player,owner,i)||(slot<0&&(owner!=player||p->monsters[i].zone_blocked)))continue;int value=(owner==player?100000:-100000)+(owner==player?duel_attack_value(d,owner,i):-duel_attack_value(d,owner,i));if(value<score){score=value;weakest=token;}}mask|=1u<<weakest;if(slot<0&&weakest<5)slot=weakest;}
            out->action=live_monster_name(d,1526,3)||d->cards[p->hand[best]].name_group==1201||attack_equipment_name(d->cards[p->hand[best]].name_group)||flip_card(d,p->hand[best])||(monster_abilities_for(&d->cards[p->hand[best]])&128)?DUEL_ACTION_SET_MONSTER:DUEL_ACTION_SUMMON;out->source=best;out->target=slot;out->tributes=mask;return 1;}
        int backrow=empty_zone(p->backrow);if(backrow>=0&&can_pay_action(d,player,hand_play_cost(d)))for(unsigned i=0;i<p->hand_count;++i)if(!d->cold_wave_until&&!duel_card_prohibited(d,p->hand[i],0)&&d->cards[p->hand[i]].family==DUEL_TRAP&&duel_card_supported(&d->cards[p->hand[i]])){out->action=DUEL_ACTION_SET_BACKROW;out->source=i;out->target=backrow;return 1;}
        out->action=DUEL_ACTION_PHASE;out->target=d->turn==1||d->battle_skip_turn[player]==d->turn?DUEL_MAIN2:DUEL_BATTLE;return 1;
    }
    if(d->phase==DUEL_BATTLE){
        for(unsigned i=0;i<5;++i){const DuelSlot *s=&p->monsters[i];if(!s->card||s->attacked>=duel_attack_limit(d,player,i)||!combat_attack_position(d,s)||!continuous_attack_allowed(d,player,i)||!servant_ready(d,player,i)||!can_pay_action(d,player,attack_play_cost(d,player,i))||(duel_attack_tribute_required(&d->cards[s->card])&&!attack_tribute_cost_options(d,player,i,0,0)))continue;
            if(duel_can_attack_direct(d,player,i)){out->action=DUEL_ACTION_ATTACK;out->source=i;out->target_player=1-player;out->target=DUEL_NO_SLOT;return 1;}
            int target=-1,weakest=32767;for(unsigned j=0;j<5;++j){const DuelSlot *t=&enemy->monsters[j];if(!t->card||!duel_can_attack_target(d,player,i,j))continue;
                /* No AI access to the identity or DEF of a face-down card. */
                int value=t->position==DUEL_FACEDOWN_DEFENSE?1500:t->position==DUEL_FACEUP_ATTACK?duel_attack_value(d,1-player,j):duel_defense_value(d,1-player,j);
                int power=duel_attack_value(d,player,i);if(t->position!=DUEL_FACEDOWN_DEFENSE){int values[4];duel_combat_values(d,player,i,j,values);power=values[0];value=values[t->position==DUEL_FACEUP_ATTACK?2:3];}if(power>=value&&value<weakest){weakest=value;target=j;}}
            if(target<0)continue;
            if(target>=0){out->action=DUEL_ACTION_ATTACK;out->source=i;out->target_player=1-player;out->target=target<0?DUEL_NO_SLOT:target;return 1;}
        }
        out->action=DUEL_ACTION_PHASE;out->target=DUEL_MAIN2;return 1;
    }
    out->action=DUEL_ACTION_PHASE;out->target=DUEL_END;return 1;
}
uint32_t duel_state_hash(const Duel *d){const uint8_t *bytes=(const uint8_t *)d->players;uint32_t value=2166136261u;for(unsigned i=0;i<sizeof(d->players);++i)value=(value^bytes[i])*16777619u;return value^d->turn^((uint32_t)d->active<<16)^((uint32_t)d->phase<<24);}
