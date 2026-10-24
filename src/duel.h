#ifndef JOEY_NATIVE_DUEL_H
#define JOEY_NATIVE_DUEL_H
#include <stdint.h>

#define DUEL_CARD_COUNT 1115
#define DUEL_DECK_MAX 80
#define DUEL_HAND_MAX DUEL_DECK_MAX
#define DUEL_PILE_MAX 192
#define DUEL_ZONES 5
#define DUEL_FIELD_SLOT 5
#define DUEL_NO_SLOT 255
#define DUEL_CHAIN_MAX 32
#define DUEL_TRIGGER_MAX 64
#define DUEL_TARGET_MAX 5
#define DUEL_FUSION_MAX 15
#define DUEL_STAT_LINK_MAX 80
#define DUEL_CONTROL_LAYER_MAX 64
#define DUEL_OPTION_MAX (DUEL_PILE_MAX+1)

enum DuelFamily {DUEL_MONSTER,DUEL_SPELL,DUEL_TRAP};
enum DuelPhase {DUEL_DRAW,DUEL_STANDBY,DUEL_MAIN1,DUEL_BATTLE,DUEL_MAIN2,DUEL_END};
enum DuelPosition {DUEL_FACEUP_ATTACK,DUEL_FACEUP_DEFENSE,DUEL_FACEDOWN_DEFENSE,DUEL_FACEDOWN_ATTACK};
enum DuelEffect {DUEL_E_NONE,DUEL_E_DRAW2,DUEL_E_REBORN,DUEL_E_DARK_HOLE,DUEL_E_RAIGEKI,
    DUEL_E_FEATHER_DUSTER,DUEL_E_MST,DUEL_E_FISSURE,DUEL_E_TRUNADE,DUEL_E_CHANGE_HEART,
    DUEL_E_DE_SPELL,DUEL_E_REMOVE_TRAP,DUEL_E_GOBLIN,DUEL_E_ZOMBYRA,
    DUEL_E_BUSTER,DUEL_E_GRACEFUL_DICE,DUEL_E_TRIBUTE_DOOMED,DUEL_E_SANGAN,DUEL_E_WITCH,
    DUEL_E_TORRENTIAL,DUEL_E_MAGIC_JAMMER,DUEL_E_MAGIC_CYLINDER,
    DUEL_E_NIMBLE,DUEL_E_SAFE_RETURN,DUEL_E_PENGUIN,DUEL_E_BACKUP,DUEL_E_CHAIN_DESTRUCTION,DUEL_E_CALL_HAUNTED,DUEL_E_MORPH2,DUEL_E_POLYMERIZATION,DUEL_E_ALLIGATOR_DRAGON,DUEL_E_FUSION_SUBSTITUTE,DUEL_E_EQUIP,DUEL_E_LIFE,DUEL_E_HANE_HANE,DUEL_E_MAN_EATER_BUG,DUEL_E_GRAVE_RECALL,DUEL_E_FIELD,DUEL_E_AURA,DUEL_E_TRAP_LOCK,DUEL_E_SIMPLE,DUEL_E_MONSTER,DUEL_E_RITUAL,DUEL_E_FLIP,DUEL_E_CONTINUOUS,DUEL_E_IGNITION,DUEL_E_RECRUIT,DUEL_E_SWARM,DUEL_E_TARGET_FLIP,DUEL_E_SEARCH,DUEL_E_HAND,DUEL_E_PERIODIC,DUEL_E_COUNTER,DUEL_E_VICTORY,DUEL_E_BIND,DUEL_E_UNSUPPORTED=255};
enum DuelAction {DUEL_ACTION_PHASE,DUEL_ACTION_SUMMON,DUEL_ACTION_SET_MONSTER,
    DUEL_ACTION_POSITION,DUEL_ACTION_ATTACK,DUEL_ACTION_SET_BACKROW,
    DUEL_ACTION_ACTIVATE_HAND,DUEL_ACTION_DISCARD,DUEL_ACTION_CONCEDE,DUEL_ACTION_CHOOSE,
    DUEL_ACTION_ACTIVATE_BACKROW,DUEL_ACTION_PASS,DUEL_ACTION_ACTIVATE_MONSTER,DUEL_ACTION_FUSION_GATE};
enum DuelResult {DUEL_OK,DUEL_BAD_ARGUMENT,DUEL_WRONG_TURN,DUEL_WRONG_PHASE,
    DUEL_ZONE_FULL,DUEL_SUMMON_USED,DUEL_NEED_TRIBUTES,DUEL_BAD_TARGET,
    DUEL_NOT_ALLOWED,DUEL_UNSUPPORTED,DUEL_FINISHED,DUEL_HAND_LIMIT,DUEL_EFFECT_PENDING};
enum DuelEvent {DUEL_EVENT_NONE,DUEL_EVENT_DRAW,DUEL_EVENT_SUMMON,DUEL_EVENT_SET,
    DUEL_EVENT_DESTROY,DUEL_EVENT_DAMAGE,DUEL_EVENT_REVIVE,DUEL_EVENT_PHASE,
    DUEL_EVENT_WIN,DUEL_EVENT_SPELL,DUEL_EVENT_RECOVER,DUEL_EVENT_RETURN,DUEL_EVENT_PILE_DESTROY,DUEL_EVENT_REMODEL,DUEL_EVENT_FUSION,DUEL_EVENT_FIELD_REPLACE,DUEL_EVENT_BANISH,DUEL_EVENT_POSITION};

typedef struct {
    uint32_t original_property;
    uint16_t attack,defense;
    uint8_t level,family,subtype,race,attribute,effect;
    uint16_t name_group;
} DuelCardDef;
typedef struct {uint16_t result,materials[3];uint8_t count;} DuelFusionRecipe;
enum DuelEquipTarget {DUEL_EQUIP_RACE=1,DUEL_EQUIP_ATTRIBUTE,DUEL_EQUIP_NAMES,DUEL_EQUIP_ANY,DUEL_EQUIP_EXCLUDE_RACE};
typedef struct {uint16_t card;int16_t attack,defense;uint8_t kind;uint32_t mask;uint16_t names[2];} DuelEquipRule;
enum DuelLifeOperation {DUEL_LIFE_RECOVER,DUEL_LIFE_DAMAGE};
enum DuelLifeTarget {DUEL_LIFE_SELF,DUEL_LIFE_OPPONENT,DUEL_LIFE_EITHER,DUEL_LIFE_BOTH};
typedef struct {uint16_t card,amount;uint8_t operation,target;} DuelLifeRule;
typedef struct {uint16_t card;uint8_t family;} DuelRecallRule;
enum DuelFieldSelector {DUEL_FIELD_RACE=1,DUEL_FIELD_ATTRIBUTE,DUEL_FIELD_DEFENSE};
typedef struct {uint16_t card;uint8_t kind;uint32_t positive,negative;int16_t attack,defense;} DuelFieldRule;
/* Face-up monster or live attached Equip, affecting both players by attribute. */
typedef struct {uint16_t card;uint8_t positive,negative;int16_t increase,decrease;} DuelAuraRule;
typedef struct {uint16_t card;uint8_t negates,blocks,exempt_same_name;} DuelSuppressionRule;
typedef struct {uint16_t card;uint8_t owner,position,attacked,position_changed;uint16_t summoned_turn,set_turn;int16_t attack_bonus,defense_bonus;uint8_t return_controller,properly_summoned;uint16_t position_lock_until;int16_t temporary_bonus;uint32_t serial;uint8_t effect_active,facedown_attack,card_history;uint8_t equip_role:3,reborn_origin:1,monster_negated:1,zone_blocked:2,flip_armed:1;uint32_t link_serial;union {int32_t temporary_attack;uint32_t field_order;};int32_t temporary_defense;uint16_t ignition_turn,original_attack,original_until,destroy_end_turn;uint8_t original_attack_active,normal_summon_marker;uint16_t no_attack_until,swap_until;uint8_t swap_parity,special_summon_marker;int16_t lasting_attack;uint16_t lasting_attack_until;uint8_t control_used,hourglass_turn;int16_t battle_curse;} DuelSlot;
/* Identity belongs to a physical card, while attachments/counters belong to
 * its current field deployment. Stored card IDs validate imported fixtures. */
typedef struct {uint32_t serial;uint16_t card;uint8_t proper,history,owner;} DuelCardState;
typedef struct {
    int32_t life;
    uint16_t deck[DUEL_DECK_MAX],hand[DUEL_HAND_MAX],grave[DUEL_PILE_MAX],banished[DUEL_PILE_MAX];
    uint8_t deck_count,hand_count;uint16_t grave_count,banished_count;
    DuelSlot monsters[DUEL_ZONES],backrow[DUEL_ZONES];
    uint8_t normal_summon_used;
    uint32_t grave_serial[DUEL_PILE_MAX];
    uint16_t fusion[DUEL_FUSION_MAX];uint8_t fusion_count,grave_proper[DUEL_PILE_MAX];
    DuelSlot field;
    uint8_t skip_next_draw,skip_next_standby;
    uint32_t banished_serial[DUEL_PILE_MAX];
    uint8_t banished_proper[DUEL_PILE_MAX];
    uint8_t grave_history[DUEL_PILE_MAX];
    DuelCardState deck_state[DUEL_DECK_MAX],hand_state[DUEL_HAND_MAX],fusion_state[DUEL_FUSION_MAX];
    uint8_t banished_history[DUEL_PILE_MAX];
    uint8_t banished_flags[DUEL_PILE_MAX],banished_position[DUEL_PILE_MAX];
    uint8_t pile_side;uint16_t temporary_reserved_mask;
} DuelPlayer;
enum DuelGraveHistory {DUEL_HISTORY_RITUAL=1,DUEL_HISTORY_FUSION=2,DUEL_HISTORY_BATTLE=4,DUEL_HISTORY_OPPONENT=8,DUEL_HISTORY_ARRIVED=16};
enum DuelEquipmentRole {DUEL_EQUIP_ROLE_NONE,DUEL_EQUIP_ROLE_SWORD,DUEL_EQUIP_ROLE_ABSORB};
typedef struct {uint32_t serial;uint8_t remaining;} DuelBattleCountdown;
typedef struct {uint32_t source[320],victim[320];uint8_t actor[320];uint16_t count;} DuelSwordQueue;
typedef struct {uint16_t card,turn;uint8_t actor,offered,used;uint32_t offered_event;} DuelEndTimer;
typedef struct {uint32_t serial,order;uint16_t card,due_turn;uint8_t actor,used,queued,kind;} DuelRevivalJob;
typedef struct {DuelCardState victim;uint16_t card;uint8_t actor,controller,position,used,slot;} DuelBattleRevival;
typedef struct {uint8_t type,player,slot,other;uint16_t card;int16_t amount;} DuelLogEvent;
enum DuelChoiceKind {DUEL_CHOICE_NONE,DUEL_CHOICE_SEARCH_ATTACK,DUEL_CHOICE_SEARCH_DEFENSE,
    DUEL_CHOICE_NIMBLE_RECOVER,DUEL_CHOICE_NIMBLE_SUMMON,DUEL_CHOICE_OPTIONAL_DRAW,DUEL_CHOICE_PENGUIN,DUEL_CHOICE_ATTACK_REPLAY,DUEL_TRIGGER_MORPH2,DUEL_CHOICE_FUSION_MONSTER,DUEL_CHOICE_FUSION_MATERIAL,DUEL_CHOICE_FUSION_POSITION,DUEL_CHOICE_FLIP_TARGET,DUEL_CHOICE_GRAVE_RECALL,DUEL_CHOICE_DISCARD_HAND,DUEL_CHOICE_RITUAL_MONSTER,DUEL_CHOICE_RITUAL_MATERIAL,DUEL_CHOICE_RITUAL_POSITION,DUEL_TRIGGER_FLIP_AUTO,DUEL_TRIGGER_RECRUIT,DUEL_CHOICE_RECRUIT,DUEL_TRIGGER_SWARM,DUEL_CHOICE_TARGET_FLIP,DUEL_CHOICE_DECK_SEARCH,DUEL_CHOICE_SEARCH_OFFER,DUEL_CHOICE_HAND,DUEL_TRIGGER_DISCARD,DUEL_CHOICE_EQUIP_RECYCLE,DUEL_TRIGGER_EQUIP_GRAVE,DUEL_CHOICE_PERIODIC,DUEL_TRIGGER_PERIODIC,DUEL_CHOICE_MAGIC_DRAIN,DUEL_TRIGGER_POSITION_AUTO,DUEL_CHOICE_UPKEEP,DUEL_TRIGGER_CONTINUOUS,DUEL_TRIGGER_BATTLE_AUTO,DUEL_CHOICE_IGNITION_FUSION,DUEL_CHOICE_IGNITION_POSITION,DUEL_CHOICE_SIMPLE_DESTROY,DUEL_CHOICE_SPECIAL_HAND_COST,DUEL_CHOICE_SPECIAL_HAND_POSITION,DUEL_CHOICE_SPIRIT_POSITION,DUEL_CHOICE_PILE_PLAYER,DUEL_CHOICE_PILE_CARD,DUEL_TRIGGER_GRAVE_AUTO,DUEL_CHOICE_LAIR_TARGET,DUEL_TRIGGER_ARRIVAL_AUTO,DUEL_CHOICE_PEEK_VIEW,DUEL_CHOICE_PEEK_HAND,DUEL_CHOICE_PEEK_ORDER,DUEL_CHOICE_FLIP_OFFER,DUEL_CHOICE_PILE_SUMMON,DUEL_CHOICE_PILE_SUMMON_POSITION,DUEL_CHOICE_ATTACK_COIN,DUEL_CHOICE_DARK_SAGE_OFFER,DUEL_CHOICE_DARK_SAGE_TRIBUTE,DUEL_CHOICE_DARK_SAGE_SPELL,DUEL_CHOICE_EQUIP_STAT,DUEL_CHOICE_OPTIONAL_SET,DUEL_CHOICE_MUSHROOM,DUEL_CHOICE_TRIBUTE_UPKEEP,DUEL_TRIGGER_GILA,DUEL_CHOICE_ACTIVATION_PARAMETER,DUEL_TRIGGER_TOKEN_STANDBY,DUEL_CHOICE_DECK_EQUIP,DUEL_CHOICE_HISTORY_OFFER,DUEL_CHOICE_HISTORY_TARGET,DUEL_CHOICE_GRAVE_PAIR,DUEL_CHOICE_GRAVE_PAIR_POSITION,DUEL_CHOICE_REVIVAL_JAM,DUEL_TRIGGER_REVIVAL_JOB,DUEL_CHOICE_EQUIP_TRANSFER_SOURCE,DUEL_CHOICE_EQUIP_TRANSFER_TARGET,DUEL_CHOICE_KUNAI_MODE,DUEL_CHOICE_KUNAI_TARGET,DUEL_CHOICE_SUMMON_PROCEDURE_COST,DUEL_CHOICE_ATTACK_TRIBUTE_COST,DUEL_CHOICE_ATTACK_TRIBUTE_TARGET,DUEL_TRIGGER_QUEEN_END,DUEL_CHOICE_DEFUSION,DUEL_CHOICE_OFFERING_PAY,DUEL_CHOICE_OFFERING_HAND,DUEL_CHOICE_OFFERING_FORM,DUEL_CHOICE_OFFERING_TRIBUTE,DUEL_CHOICE_OFFERING_ZONE,DUEL_CHOICE_BATTLE_REVIVE,DUEL_CHOICE_BATTLE_REVIVE_POSITION,DUEL_CHOICE_BATTLE_REDIRECT,DUEL_CHOICE_FAIRY_BOX_COIN,DUEL_CHOICE_RETARGET_MODE,DUEL_CHOICE_RETARGET_MONSTER,DUEL_CHOICE_STAT_SOURCE,DUEL_CHOICE_STAT_DESTINATION,DUEL_CHOICE_LAIR_OFFER,DUEL_CHOICE_DESTRUCTION_SELECT,DUEL_CHOICE_SHARE_TRIBUTE,DUEL_CHOICE_EARTH_FIRST,DUEL_CHOICE_EARTH_SECOND,DUEL_CHOICE_EARTH_OPPONENT,DUEL_CHOICE_PATROL_TARGET,DUEL_CHOICE_INSPECTION_OFFER,DUEL_CHOICE_EXCHANGE_OTHER,DUEL_CHOICE_EXCHANGE_OWN,DUEL_CHOICE_PAINFUL_DECK,DUEL_CHOICE_PAINFUL_KEEP,DUEL_CHOICE_YADO_HAND,DUEL_CHOICE_MYSTIC_DESTROY,DUEL_CHOICE_MYSTIC_GIVE,DUEL_CHOICE_GROUND_FIRST,DUEL_CHOICE_GROUND_SECOND,DUEL_CHOICE_GAMBLE_COIN,DUEL_CHOICE_CRUSH_TRIBUTE,DUEL_TRIGGER_PARASITE_DRAW,DUEL_CHOICE_CHOSEN_MONSTER,DUEL_CHOICE_CHOSEN_NONMONSTER,DUEL_CHOICE_CHOSEN_BLIND,DUEL_CHOICE_CHOSEN_POSITION,DUEL_CHOICE_CYBER_POSITION,DUEL_CHOICE_DRILL_OFFER,DUEL_CHOICE_BORROW_TARGET,DUEL_CHOICE_HATS_TARGET,DUEL_CHOICE_HATS_DECK,DUEL_CHOICE_BAIT_TARGET,DUEL_CHOICE_BAIT_BACKUP,DUEL_CHOICE_MAJOR_RIOT};
typedef struct {uint8_t kind,player;uint16_t card;uint32_t serial;uint8_t selected_count;} DuelTrigger;
enum DuelOptionKind {DUEL_OPTION_CARD,DUEL_OPTION_CONFIRM,DUEL_OPTION_SKIP,DUEL_OPTION_PAGE};
typedef struct {uint16_t card;uint8_t index,kind;} DuelChoiceOption;
typedef struct {uint8_t player,action,source,target_player,target,position;uint16_t tributes;uint8_t cost,target_count,targets[DUEL_TARGET_MAX];} DuelCommand;
enum DuelWindowKind {DUEL_WINDOW_NONE,DUEL_WINDOW_SUMMON,DUEL_WINDOW_ATTACK,DUEL_WINDOW_SUMMON_NEGATION,DUEL_WINDOW_DAMAGE,DUEL_WINDOW_DAMAGE_LIFE,DUEL_WINDOW_EQUIP,DUEL_WINDOW_SET,DUEL_WINDOW_BATTLE_DEATH,DUEL_WINDOW_DAMAGE_DONE,DUEL_WINDOW_REACTIVE};
typedef struct {uint8_t kind,player,source,target;uint32_t serial,target_serial;uint16_t card;uint8_t negated;uint32_t formation,events;uint8_t target_player_plus1;} DuelWindow;
static inline unsigned duel_battle_target_player(const DuelWindow *w){return w->target_player_plus1?w->target_player_plus1-1:1-w->player;}
typedef struct {uint32_t serial;uint8_t player,slot;} DuelTarget;
typedef struct {
    DuelCommand command;
    uint16_t card,target_card;
    uint32_t source_serial,target_serial;
    uint8_t source_slot,speed,negated,effect_activation;
    DuelTarget targets[DUEL_TARGET_MAX];
    uint8_t target_count,resolution_started,effect_negated,player_redirect,trap_protected;
    DuelCardState grave_targets[2];
} DuelChainLink;
typedef struct {uint16_t card;uint8_t index,from_backrow;} DuelActivationOption;
/* Public excavation record: deployed is the destination slot plus one, or
 * zero for a card sent to the grave. It does not expose unrevealed deck cards. */
typedef struct {uint32_t sequence;uint16_t source_card;uint8_t returned[2],revealed[2],summoned[2];uint16_t cards[2][DUEL_DECK_MAX];uint8_t deployed[2][DUEL_DECK_MAX];uint8_t returned_fusion[2];} DuelRemodel;
typedef struct {uint8_t index,selected[3],count,banish;} DuelFusionTask;
typedef struct {uint16_t spell;uint8_t monster_index,selected[DUEL_HAND_MAX+5],count,level;} DuelRitualTask;
typedef struct {uint8_t owner,actor,remaining,optional,op,cost,filter,next_owner_plus1;} DuelHandTask;
typedef struct {DuelCardState pending;uint16_t source;uint8_t actor,round,left,permission[2],active,report_side,report_index;} DuelExcavationTask;
typedef struct {uint32_t sequence;uint16_t card;uint8_t player,count,mask,guess,success;} DuelRandomResult;
typedef struct {uint32_t source,target;uint8_t kind;int32_t attack_after,base_value;uint16_t expires;uint8_t destroy_at_end;} DuelStatLink;
typedef struct {uint32_t target,source;uint16_t card,until;uint8_t actor;} DuelControlLayer;
typedef struct {uint32_t target;uint8_t base;} DuelControlState;
typedef struct {uint32_t source,exempt[22];uint8_t count,suppressed;} DuelProhibition;
#define DUEL_TRIBUTE_GRANT_MAX 64
typedef struct {uint32_t target;uint16_t until;uint8_t actor,kind;} DuelTributeGrant;
typedef struct {uint16_t card;uint8_t index,remaining,used_pieces;} DuelSpecialHandTask;
typedef struct {uint8_t op,owner,root;} DuelPileTask;
typedef struct {uint8_t draw,discard,gust,snow,michi,damage,riot;} DuelReactiveNotice;
typedef struct {uint16_t source;uint8_t actor,round,left[2],permission[2],active,deployed[2];} DuelRiotTask;
typedef struct {uint16_t card;uint32_t source;DuelCardState selected[2];uint8_t actor,chooser,round,permission;} DuelGravePairTask;

typedef struct {uint16_t spell,selected;uint8_t actor,chooser,kind,remaining,selected_index;uint16_t parameter;uint32_t instances[3];} DuelPileSummonTask;
typedef struct {DuelCommand command;uint32_t serial;uint8_t stage;} DuelAttackTributeTask;
typedef struct {uint16_t cards[22];uint32_t selected_mask;uint8_t actor,owner,kind,count,selected_count,selected[5];} DuelPeekTask;
typedef struct {
    DuelPlayer players[2];
    const DuelCardDef *cards;
    uint32_t random_state,revision;
    uint16_t turn;
    uint8_t active,first,phase,discard_required;
    int8_t winner;
    DuelLogEvent last_event;
    DuelTrigger triggers[DUEL_TRIGGER_MAX],choice;
    uint8_t trigger_count,turn_end_pending;
    uint32_t next_serial;
    DuelWindow window,summon_notice;
    DuelChainLink chain[DUEL_CHAIN_MAX];
    uint8_t chain_count,response_player,response_passes;
    uint8_t trigger_chain_pending;
    DuelTarget choice_targets[DUEL_TARGET_MAX],battle_destroy[2];
    uint8_t battle_destroy_count,battle_pending;
    DuelWindow deferred_attack,redirect_battle;
    DuelCommand redirect_command;
    uint16_t destiny_end_turn;uint8_t destiny_end_actor,destiny_end_cursor,destiny_end_pending;
    DuelRemodel remodel;
    const DuelFusionRecipe *fusion_recipes;uint16_t fusion_recipe_count;uint8_t resolving;
    DuelFusionTask fusion_task;
    DuelBattleCountdown battle_countdowns[10];
    DuelTarget dimensional_battle[2],punch_attacker,punch_defender;
    uint8_t punch_actor;
    DuelWindow damage_done_notice;
    DuelBattleRevival battle_revive_jobs[2],battle_revive_task,battle_last_death;
    uint8_t battle_death_count,battle_revival_notice,battle_death_position[2],battle_revival_relevant;
    DuelCommand offering_summon;
    const DuelEquipRule *equips;uint16_t equip_count;
    const DuelLifeRule *life_rules;uint16_t life_rule_count;
    const DuelRecallRule *recalls;uint16_t recall_count;
    const DuelFieldRule *fields;uint16_t field_count;
    const DuelAuraRule *auras;uint16_t aura_count;
    const DuelSuppressionRule *suppression;uint16_t suppression_count;
    DuelRitualTask ritual_task;
    uint16_t battle_protect_turn[2],spirit_elimination_turn[2],inspection_declined_turn[2];
    uint32_t battle_attacker[2];
    DuelHandTask hand_task;
    DuelExcavationTask excavation_task;
    DuelRiotTask riot_task;
    DuelReactiveNotice reactive_pending,reactive_current;
    DuelTarget bait_source,bait_queue[DUEL_CHAIN_MAX];DuelCommand bait_command;uint8_t bait_pending,bait_mode;
    uint8_t phase_request;
    uint16_t end_prepared_turn;
    DuelRandomResult random_result;
    uint16_t reverse_turn;
    DuelStatLink stat_links[DUEL_STAT_LINK_MAX];
    uint8_t stat_link_count,stat_snapshot_present;
    uint8_t deck_failure_mask;
    DuelSpecialHandTask special_hand_task;
    DuelPileTask pile_task;
    DuelPeekTask peek_task;
    DuelPileSummonTask pile_summon_task;
    DuelAttackTributeTask attack_tribute;
    DuelGravePairTask grave_pair_task;
    DuelRevivalJob revival_jobs[64];
    DuelSwordQueue sword_jobs;
    uint32_t damage_zero_serial;
    uint8_t damage_stage,damage_revealed;
    uint32_t damage_amount[2];
    uint8_t damage_dealer_side,damage_dealer_slot,damage_event_mask;
    DuelCommand coin_attack;
    DuelTarget battle_effect_destroy[2];
    uint8_t battle_effect_destroy_count;
    DuelTarget battle_equipment_destroy[10];
    uint8_t battle_equipment_destroy_count;
    DuelCommand equip_mode_command;
    uint16_t manual_position_lock_until[2];
    uint16_t type_suppression_turn;
    uint8_t type_suppression_mask;
    DuelControlLayer control_layers[DUEL_CONTROL_LAYER_MAX];
    DuelControlState control_states[10];
    uint8_t control_layer_count;
    DuelTributeGrant tribute_grants[DUEL_TRIBUTE_GRANT_MAX];
    uint8_t tribute_grant_count;
    uint16_t battle_skip_turn[2];
    uint16_t crush_until_end[2],weather_until_end[2];uint8_t skip_next_turn[2];
    uint8_t jigen_standby_actor_plus1,jigen_standby_next,standby_finish_actor_plus1;
    DuelTarget guard_battle;
    uint32_t next_field_order;
    DuelProhibition prohibitions[10];
    uint16_t field_name_cache,prohibition_disabled_mask;
    uint8_t passive_presence;
    uint16_t summon_ban_turn[2];uint8_t summon_activity[2],summon_ban_mask[2];
    uint16_t cold_wave_until;uint8_t cold_wave_actor,main1_action_used;
    DuelEndTimer end_timers[64];uint8_t end_timer_count,effect_actor_plus1;
    uint8_t field_to_grave[2];uint32_t event_sequence;
} Duel;
int duel_history_can_return(const Duel *,unsigned,unsigned,unsigned);
int duel_last_will_candidate(const Duel *,unsigned,unsigned);
int duel_last_will_ready(const Duel *,unsigned);
unsigned duel_return_grave_to_deck(Duel *,unsigned);
unsigned duel_mill_deck(Duel *,unsigned,unsigned,unsigned);
/* Typed pile primitives collect notices; duel_command advances priority. */
unsigned duel_draw_cards(Duel *,unsigned player,unsigned count,unsigned discard_spells);
unsigned duel_discard_hand(Duel *,unsigned owner,unsigned index,unsigned actor,unsigned by_effect,unsigned paid);
int duel_grave_pair_candidate(const Duel *,unsigned,unsigned,unsigned,uint32_t);
int duel_target_present(const Duel *,unsigned,unsigned,uint32_t);
int duel_spell_immune(const Duel *,unsigned,unsigned);
unsigned duel_race(const Duel *,unsigned,unsigned);
unsigned duel_attribute(const Duel *,unsigned,unsigned);

int duel_start(Duel *,const DuelCardDef *,const uint16_t *,unsigned,const uint16_t *,unsigned,uint32_t seed,unsigned first);
int duel_fusion_setup(Duel *,const DuelFusionRecipe *,unsigned,const uint16_t *,unsigned,const uint16_t *,unsigned);
int duel_equip_setup(Duel *,const DuelEquipRule *,unsigned);
int duel_equip_can_target(const Duel *,unsigned card,unsigned player,unsigned slot);
unsigned duel_gila_revival_options(const Duel *,unsigned actor,DuelChoiceOption *,unsigned capacity);
int duel_big_shield_can_intercept(const Duel *,unsigned owner,unsigned index);
int duel_can_tribute(const Duel *,unsigned actor,unsigned owner,unsigned index);
int duel_can_normal_tribute(const Duel *,unsigned,unsigned,unsigned);
enum DuelSummonPermission {DUEL_PERMISSION_NORMAL,DUEL_PERMISSION_SET,DUEL_PERMISSION_SPECIAL};
int duel_summon_permission(const Duel *,unsigned,unsigned);
int duel_is_token(const Duel *,unsigned);
int duel_banish_field(Duel *,unsigned,unsigned,unsigned);
int duel_can_target_monster(const Duel *,unsigned player,unsigned slot);
unsigned duel_public_hand_options(const Duel *,unsigned owner,DuelChoiceOption *,unsigned capacity);
int duel_hand_revealed(const Duel *,unsigned owner);
int duel_exodia_complete(const Duel *,unsigned player);
int duel_life_setup(Duel *,const DuelLifeRule *,unsigned);
const DuelLifeRule *duel_life_rule(const Duel *,unsigned card);
int duel_recall_setup(Duel *,const DuelRecallRule *,unsigned);
int duel_field_setup(Duel *,const DuelFieldRule *,unsigned);
void duel_battle_departure_events(Duel *,const DuelWindow *);
unsigned duel_battle_countdown(const Duel *,unsigned,unsigned);
void duel_battle_revival_capture(Duel *);
int duel_battle_revival_ready(const Duel *,const DuelBattleRevival *);
int duel_time_machine_available(const Duel *,unsigned);
int duel_destruction_punch_ready(const Duel *,unsigned);
int duel_sebek_ready(const Duel *,unsigned);
void duel_punch_finish(Duel *);
void duel_punch_mark(Duel *,const DuelChainLink *);
void duel_battle_countdown_set(Duel *,unsigned,unsigned,unsigned);
unsigned duel_offering_forms(const Duel *,unsigned,unsigned);
int duel_fusion_gate_available(const Duel *,unsigned,unsigned);
unsigned duel_defusion_materials(const Duel *,unsigned,unsigned,uint32_t *);
int duel_aura_setup(Duel *,const DuelAuraRule *,unsigned);
int duel_suppression_setup(Duel *,const DuelSuppressionRule *,unsigned);
int duel_field_effect_negated(const Duel *,unsigned card);
int duel_card_activation_blocked(const Duel *,unsigned card);
int duel_card_prohibited(const Duel *,unsigned,uint32_t);
void duel_refresh_field_cache(Duel *);
unsigned duel_forced_trap_options(Duel *,unsigned,unsigned,DuelChoiceOption *,unsigned);
int duel_trap_substitution_protects(const Duel *,unsigned,unsigned,unsigned);
int duel_command(Duel *,const DuelCommand *);
int duel_attack_value(const Duel *,unsigned player,unsigned slot);
int duel_defense_value(const Duel *,unsigned player,unsigned slot);
unsigned duel_battle_protection(const Duel *,unsigned player,unsigned index,unsigned opposing_attack);
int duel_combat_values(const Duel *,unsigned player,unsigned attacker,unsigned target,int values[4]);
int duel_combat_values_for(const Duel *,unsigned,unsigned,unsigned,unsigned,int[4]);
int duel_card_supported(const DuelCardDef *);
int duel_ai_decide(const Duel *,unsigned player,DuelCommand *);
unsigned duel_choice_options(const Duel *,DuelChoiceOption *,unsigned capacity);
unsigned duel_response_options(const Duel *,unsigned player,DuelActivationOption *,unsigned capacity);
unsigned duel_backup_options(const Duel *,unsigned player,DuelChoiceOption *,unsigned capacity);
int duel_can_revive_for(const Duel *,unsigned actor,unsigned grave_owner,unsigned index);
int duel_can_revive(const Duel *,unsigned player,unsigned grave_index);
int duel_can_attack_target(const Duel *,unsigned player,unsigned source,unsigned target);
int duel_can_attack_direct(const Duel *,unsigned player,unsigned slot);
unsigned duel_simple_target_kind(const DuelCardDef *);
int duel_summon_procedure_candidate(const Duel *,unsigned player,unsigned kind,unsigned pile_index,unsigned parameter);
int duel_equipment_transfer_can_target(const Duel *,unsigned owner,unsigned source,unsigned target_side,unsigned target);
unsigned duel_attack_limit(const Duel *,unsigned player,unsigned slot);
unsigned duel_hand_special_cost_is_field(const DuelCardDef *);
unsigned duel_hand_special(const DuelCardDef *);
unsigned duel_hand_monster_effect(const DuelCardDef *);
unsigned duel_counter_tribute_cost(const DuelCardDef *);
unsigned duel_normal_tribute_cost(const Duel *,unsigned player,unsigned index,unsigned set);
unsigned duel_attack_tribute_required(const DuelCardDef *);
unsigned duel_monster_tribute_cost(const DuelCardDef *);
unsigned duel_monster_has_ignition(const DuelCardDef *);
int duel_blast_can_target(const Duel *,unsigned,unsigned,unsigned,unsigned);
unsigned duel_blast_can_start(const Duel *,unsigned,unsigned);
unsigned duel_standby_has_action(const Duel *,unsigned);
int duel_binding_can_target(const Duel *,unsigned,unsigned,unsigned,unsigned);
int duel_binding_locked(const Duel *,unsigned,unsigned);
int duel_equip_can_target_for(const Duel *,unsigned,unsigned,unsigned,unsigned);
unsigned duel_magnet_target_mode(const Duel *,unsigned,unsigned);
/* Forced effect position mutation, without a manual-turn/action check. */
unsigned duel_monster_position(const Duel *,unsigned,unsigned);
void duel_change_monster_position(Duel *,unsigned,unsigned,unsigned);
unsigned duel_monster_deck_special(const Duel *,unsigned,unsigned);
unsigned duel_equipment_deck_special(const Duel *,unsigned,unsigned);
unsigned duel_control_desired(const Duel *,uint32_t,unsigned,unsigned);
unsigned duel_monster_effect_target(const DuelCardDef *);
unsigned duel_monster_effect_coin(const DuelCardDef *);
uint32_t duel_state_hash(const Duel *);
int duel_battle_redirect_target(const Duel *,unsigned,unsigned,const DuelWindow *,unsigned,unsigned);
int duel_jam_redirect_ready(const Duel *,unsigned);

int duel_battle_redirect_begin(Duel *,const DuelWindow *);
void duel_redirect_battle(Duel *,const DuelChainLink *);
int duel_forced_attack_ready(const Duel *,unsigned,unsigned);
void duel_forced_battle(Duel *,const DuelChainLink *);
int duel_destiny_complete(const Duel *,unsigned);
unsigned duel_destiny_departure_mask(const Duel *,unsigned,unsigned,unsigned);
void duel_destiny_resolve(Duel *,const DuelChainLink *);
void duel_destiny_end_queue(Duel *,unsigned);
void duel_fairy_reset_attack(Duel *,unsigned,unsigned,unsigned,unsigned);
void duel_stat_snapshot(Duel *,unsigned,unsigned,unsigned,unsigned);
int duel_spirit_active(const Duel *,unsigned);
int duel_makiu_target(const Duel *,unsigned,unsigned,unsigned);
unsigned duel_retarget_class(unsigned);
unsigned duel_retarget_modes(const Duel *,unsigned,unsigned);
void duel_retarget_resolve(Duel *,const DuelChainLink *);

int duel_patrol_ready(const Duel *,unsigned actor,unsigned index);
int duel_inspection_ready(const Duel *,unsigned actor,unsigned index);
int duel_patrol_target(const Duel *,unsigned actor,unsigned side,unsigned index);
#endif
