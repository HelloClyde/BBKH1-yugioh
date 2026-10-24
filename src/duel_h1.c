/* Direct H1 frontend for the semantic C rules engine. */
#include <stddef.h>
#include "h1_sdk.h"
#include "duel_paths.h"
#include "duel.h"
#include "duel_profile.h"
#include "duel_opening.h"
#include "duel_cards.h"
#include "duel_font_requirements.h"
#include "duel_audio.h"

static const uint16_t *native_trait_label(unsigned card,unsigned value){
    static const uint16_t *const races[]={u"",u"龙族",u"不死族",u"恶魔族",u"炎族",u"海龙族",u"岩石族",u"机械族",u"鱼族",u"恐龙族",u"昆虫族",u"兽族",u"兽战士族",u"植物族",u"水族",u"战士族",u"鸟兽族",u"天使族",u"魔法使族",u"雷族",u"爬虫族"};
    static const uint16_t *const attributes[]={u"",u"光属性",u"暗属性",u"水属性",u"炎属性",u"地属性",u"风属性"};
    return duel_card_definitions[card].name_group==1448||duel_card_definitions[card].name_group==1097?(value<7?attributes[value]:u""):(value<21?races[value]:u"");
}
static uint16_t screen[480*272];
#include "duel_font.inc"
#include "duel_assets.inc"
#include "duel_scene.inc"

Duel joey_duel_state;
volatile uint32_t joey_native_diagnostic[24];
volatile uint32_t joey_pen_diagnostic[8];
volatile int32_t joey_preview_diagnostic[4];
volatile uint32_t joey_suppression_diagnostic[4];
JoeyProfile joey_profile_state,joey_edit_state;
volatile uint32_t joey_profile_diagnostic[12];
volatile uint32_t joey_result_diagnostic[8];
volatile uint32_t joey_opening_diagnostic[12];
const uint32_t joey_profile_layout[]={sizeof(JoeyProfile),offsetof(JoeyProfile,sequence),offsetof(JoeyProfile,main),offsetof(JoeyProfile,fusion),offsetof(JoeyProfile,main_count),offsetof(JoeyProfile,fusion_count),offsetof(JoeyProfile,owned),JOEY_PROFILE_SAVE_BYTES,offsetof(JoeyProfile,wins),offsetof(JoeyProfile,losses),offsetof(JoeyProfile,reward_card),offsetof(JoeyProfile,reward_gauge),offsetof(JoeyProfile,reward_flags),offsetof(JoeyProfile,side),offsetof(JoeyProfile,side_count)};
const uint32_t joey_duel_layout[]={sizeof(Duel),sizeof(DuelPlayer),offsetof(Duel,players),offsetof(Duel,turn),offsetof(Duel,active),offsetof(Duel,phase),offsetof(Duel,winner),
    offsetof(DuelPlayer,life),offsetof(DuelPlayer,deck),offsetof(DuelPlayer,hand),offsetof(DuelPlayer,grave),offsetof(DuelPlayer,deck_count),offsetof(DuelPlayer,hand_count),offsetof(DuelPlayer,grave_count),offsetof(DuelPlayer,monsters),offsetof(DuelPlayer,backrow),sizeof(DuelSlot),offsetof(Duel,choice),offsetof(Duel,trigger_count),offsetof(Duel,turn_end_pending),
    offsetof(Duel,response_player),offsetof(Duel,chain_count),offsetof(DuelPlayer,normal_summon_used),offsetof(DuelSlot,effect_active),sizeof(DuelChoiceOption),offsetof(Duel,revision),
    offsetof(Duel,battle_pending),offsetof(Duel,battle_destroy_count),offsetof(Duel,battle_destroy),sizeof(DuelTarget),offsetof(Duel,choice_targets),offsetof(DuelPlayer,grave_serial),offsetof(Duel,window),offsetof(Duel,last_event),offsetof(DuelSlot,link_serial),offsetof(Duel,deferred_attack),sizeof(DuelWindow),offsetof(Duel,remodel),sizeof(DuelRemodel),offsetof(DuelRemodel,cards),offsetof(DuelRemodel,deployed),
    offsetof(DuelPlayer,fusion),offsetof(DuelPlayer,fusion_count),offsetof(DuelPlayer,grave_proper),offsetof(Duel,resolving),offsetof(Duel,fusion_task),offsetof(DuelRemodel,returned_fusion),DUEL_OPTION_MAX,offsetof(Duel,first),offsetof(DuelPlayer,field),offsetof(DuelSlot,temporary_attack),offsetof(DuelSlot,temporary_defense),offsetof(DuelSlot,ignition_turn),offsetof(DuelSlot,original_attack),offsetof(DuelSlot,original_until),offsetof(DuelSlot,destroy_end_turn),offsetof(DuelSlot,original_attack_active),offsetof(Duel,random_result),sizeof(DuelRandomResult),offsetof(DuelPlayer,skip_next_draw),offsetof(DuelSlot,no_attack_until),offsetof(DuelSlot,normal_summon_marker),offsetof(DuelSlot,swap_until),offsetof(DuelSlot,swap_parity),offsetof(Duel,reverse_turn),offsetof(DuelPlayer,skip_next_standby),offsetof(Duel,special_hand_task),offsetof(Duel,pile_task),offsetof(DuelPlayer,banished_serial),offsetof(DuelPlayer,banished_proper)};
static unsigned scene,dirty=1,exit_requested,details_open,detail_scroll,about_open;
static uint8_t keys[45];
static unsigned native_keyboard,held_mask,key_tick_valid;
static unsigned pen_down,pen_valid,pen_secondary;
static int pen_x,pen_y;
static uint32_t key_tick;
static unsigned pending,source_hand,cost_hand,tribute_mask,required_tributes,attacker,grave_page,grave_choice,grave_position;
static unsigned activation_source,activation_from_backrow,response_count;
static unsigned life_choice;
static unsigned monster_source,monster_choice,tribute_monster_effect,coin_guess;
static DuelActivationOption response_options[DUEL_OPTION_MAX];
static unsigned picker_count,picker_owner[384],picker_index[384];
static uint8_t picker_selected[384];
static unsigned backup_count;
static DuelChoiceOption effect_options[DUEL_OPTION_MAX];
static unsigned hand_scroll,gallery_card=1;
static uint32_t next_ai,notice_until;
static uint32_t seen_remodel;
static const uint16_t *notice;
static const char *const profile_roots[][2]={
    {JOEY_RESOURCE_ROOT_A "JNAT0.SAV",JOEY_RESOURCE_ROOT_A "JNAT1.SAV"},
    {JOEY_RESOURCE_ROOT_B "JNAT0.SAV",JOEY_RESOURCE_ROOT_B "JNAT1.SAV"}};
static const char *const legacy_profile_paths[]={"A:\\JOEY\\JNAT0.SAV","A:\\JOEY\\JNAT1.SAV"};
static const char *const *profile_paths=profile_roots[0];
static uint8_t profile_bytes[JOEY_PROFILE_SAVE_BYTES];
static int profile_slot=-1;
static unsigned profile_valid_mask,profile_save_result;
static unsigned editor_tab,editor_count,editor_index,editor_changed,editor_leaving,editor_exit_choice,editor_destination;
static uint16_t editor_cards[DUEL_CARD_COUNT];
static uint16_t editor_lists[4][DUEL_CARD_COUNT];
static unsigned editor_counts[4],editor_cursor[4];
volatile uint32_t joey_editor_diagnostic[8];
static JoeyProfile result_candidate;
static unsigned result_winner,result_action,result_error,result_card,result_new,result_choice,result_claimed;
static uint32_t reward_random_state;
static DuelOpening opening_state;
static uint32_t opening_random_state;
static unsigned opening_hand_focus,opening_order_focus,opening_starts,opening_cancels;
static void result_open(void);
static void result_resume(void);
enum {PENDING_NONE,PENDING_TRIBUTE,PENDING_ATTACK,PENDING_SPELL_TARGET,PENDING_GRAVE,PENDING_POSITION,PENDING_DISCARD_COST,PENDING_EFFECT_CHOICE,PENDING_RESPONSE,PENDING_BACKUP,PENDING_REMODEL,PENDING_ATTACK_SELECT,PENDING_LIFE_TARGET,PENDING_MONSTER_MENU,PENDING_HAND_EFFECT,PENDING_MONSTER_TARGET,PENDING_COIN_GUESS,PENDING_PUBLIC_HAND};

static int key_held(unsigned scan){
    const uint32_t *function=h1_runtime_entry(h1_runtime_table(H1_RUNTIME_GUI_TABLE_SLOT),0x9d8u);
    if(!function||function[0]!=0x27bdffe8u||function[1]!=0xafbf0010u||function[2]!=0x0c00208du||function[3])return 0;
    return ((int (*)(unsigned))function)(scan)!=0;
}
static int font_assets_valid(void){
    h1_file *f=JOEY_OPEN_RESOURCE("FONT.BIN",0);if(!f)return 0;uint32_t header[4],books[25][5];
    int valid=h1_fread(header,4,4,f)==4&&header[0]==0x59454f4au&&header[1]==0x31544e46u&&header[2]==1&&header[3]==25&&h1_fread(books,20,25,f)==25;
    for(unsigned i=0;valid&&i<25;++i)if(books[i][0]!=i+8||books[i][4]!=DUEL_FONT_GLYPHS)valid=0;
    if(valid&&h1_fseek(f,0,H1_SEEK_END)!=DUEL_FONT_ASSET_BYTES)valid=0;h1_fclose(f);return valid;
}
static int shift_down(void){return keys[H1_KEY_SHIFT]!=0;}
static int pen(int *x,int *y){
    typedef void (*coordinate_fn)(uint16_t *,uint16_t *);
    const uint32_t *function=(const uint32_t *)h1_runtime_entry(h1_runtime_table(H1_RUNTIME_GUI_TABLE_SLOT),0x6c0u);
    if(!function||function[0]!=0x27bdffa8u||function[1]!=0xafbf0050u)return 0;
    uint16_t px=65535,py=65535;((coordinate_fn)function)(&px,&py);*x=px;*y=py;return px<480&&py<272;
}
static void message(const uint16_t *text){notice=text;notice_until=h1_timer_read_ms()+1800;dirty=1;}
static void result_message(int result){
    const uint16_t *messages[]={u"",u"操作无效",u"现在是对方回合",u"当前阶段不能操作",u"场地已满",u"本回合已经通常召唤",u"请选择祭品",u"请选择有效对象",u"当前不能执行",u"这张卡的效果还未接入",u"决斗已经结束",u"请将手牌弃至六张",u"请先完成卡片效果选择"};
    if(result>0&&result<13)message(messages[result]);
}
static int apply(DuelCommand *command){
    int32_t life0=joey_duel_state.players[0].life,life1=joey_duel_state.players[1].life;unsigned turn=joey_duel_state.turn;
    uint32_t random_sequence=joey_duel_state.random_result.sequence,started=h1_timer_read_ms();int result=duel_command(&joey_duel_state,command);
    joey_native_diagnostic[4]++;joey_native_diagnostic[5]=result;joey_native_diagnostic[18]=h1_timer_read_ms()-started;
    if(result==DUEL_OK){
        unsigned sound=JOEY_AUDIO_NONE;
        if(command->action==DUEL_ACTION_ATTACK)sound=JOEY_SOUND_ATTACK;
        else if(command->action==DUEL_ACTION_SUMMON)sound=command->tributes?JOEY_SOUND_IKENIE:JOEY_SOUND_CARD_OPEN;
        else if(command->action==DUEL_ACTION_SET_MONSTER||command->action==DUEL_ACTION_SET_BACKROW||command->action==DUEL_ACTION_DISCARD)sound=JOEY_SOUND_CARD_MOVE;
        else if(command->action==DUEL_ACTION_POSITION)sound=JOEY_SOUND_CARD_OPEN;
        else if(command->action==DUEL_ACTION_ACTIVATE_HAND||command->action==DUEL_ACTION_ACTIVATE_BACKROW||command->action==DUEL_ACTION_ACTIVATE_MONSTER)sound=joey_duel_state.chain_count>1?JOEY_SOUND_CHAIN:JOEY_SOUND_MAGIC_ACTIVE;
        else if(command->action==DUEL_ACTION_PHASE){if(joey_duel_state.turn!=turn)sound=JOEY_SOUND_TURN_CHANGE;else if(command->target==DUEL_BATTLE)sound=JOEY_SOUND_BATTLE_TURN;}
        if(joey_duel_state.last_event.type==DUEL_EVENT_REVIVE)sound=JOEY_SOUND_REBORN;
        else if(joey_duel_state.last_event.type==DUEL_EVENT_FUSION)sound=JOEY_SOUND_FUSION2;
        if(sound!=JOEY_AUDIO_NONE)joey_audio_effect(sound);
        if(joey_duel_state.players[0].life<life0||joey_duel_state.players[1].life<life1)joey_audio_effect(JOEY_SOUND_DAMAGE);
        else if(joey_duel_state.players[0].life>life0||joey_duel_state.players[1].life>life1)joey_audio_effect(JOEY_SOUND_LIFE_UP);
        pending=0;native_focus=-1;notice=0;dirty=1;next_ai=h1_timer_read_ms()+230;
        if(joey_duel_state.random_result.sequence!=random_sequence){DuelRandomResult *r=&joey_duel_state.random_result;if(duel_card_definitions[r->card].name_group==15){message(r->success?u"时间魔术师：对方怪兽破坏":u"时间魔术师：我方怪兽破坏");}else if(duel_card_definitions[r->card].name_group==1424){message(r->success?u"猜中硬币，攻击力变为零":u"未猜中，攻击力不变");}else if(duel_card_definitions[r->card].name_group==366){message(r->success?u"猜中硬币，继续攻击":u"未猜中，支付一半生命值");}else if(duel_card_definitions[r->card].name_group==1536){const uint16_t *results[]={u"骰子：1",u"骰子：2",u"骰子：3",u"骰子：4",u"骰子：5",u"骰子：6"};message(results[r->mask>=1&&r->mask<=6?r->mask-1:0]);}else if(r->count==3){unsigned heads=0;for(unsigned i=0;i<3;++i)heads+=(r->mask>>i)&1;const uint16_t *results[]={u"投币：零个正面，破坏失败",u"投币：一个正面，破坏失败",u"投币：两个正面，破坏成功",u"投币：三个正面，破坏成功"};message(results[heads]);}else message(r->success?u"猜中硬币，原攻击力加倍":u"未猜中，原攻击力减半");}
        if(joey_duel_state.winner>=0)result_open();}
    else {joey_audio_effect(JOEY_SOUND_UNABLE);result_message(result);
        if(result==DUEL_NOT_ALLOWED&&command->action==DUEL_ACTION_PHASE&&command->target==DUEL_BATTLE&&joey_duel_state.turn==1)message(u"先攻的第一回合不能攻击");}
    return result;
}
static void command_phase(unsigned phase){DuelCommand command={0,DUEL_ACTION_PHASE,0,0,phase,0,0};apply(&command);}
static int empty(const DuelSlot *slots){for(unsigned i=0;i<5;++i)if(!slots[i].card&&!slots[i].zone_blocked)return i;return -1;}
#include "duel_ai_scene.inc"
static int profile_read(unsigned slot,JoeyProfile *out,unsigned *exists){
    h1_file *f=h1_fopen(profile_paths[slot],"rb");if(!f)return 0;if(exists)*exists=1;
    int length=h1_fseek(f,0,H1_SEEK_END);int valid=(length==JOEY_PROFILE_SAVE_BYTES||length==JOEY_PROFILE_V3_BYTES||length==JOEY_PROFILE_V2_BYTES)&&h1_fseek(f,0,H1_SEEK_SET)==0&&h1_fread(profile_bytes,1,length,f)==(unsigned)length;
    h1_fclose(f);valid=valid&&joey_profile_decode(out,duel_card_definitions,duel_original_limits,profile_bytes,length)==JOEY_PROFILE_OK;
    if(valid&&length==JOEY_PROFILE_V2_BYTES)out->reward_gauge=DUEL_ORIGINAL_REWARD_GAUGE;return valid;
}
static void profile_load(void){
    profile_paths=profile_roots[native_resource_drive];
    joey_profile_init(&joey_profile_state,duel_original_owned,duel_original_deck,DUEL_ORIGINAL_DECK_COUNT,duel_original_fusion_deck,DUEL_ORIGINAL_FUSION_DECK_COUNT);
    joey_profile_state.reward_gauge=DUEL_ORIGINAL_REWARD_GAUGE;
    JoeyProfile a,b;unsigned exists=0;int va=profile_read(0,&a,&exists),vb=profile_read(1,&b,&exists);
    /* Read old A:\JOEY saves only when the selected new folder has no saves.
     * The next ordinary save goes to the new folder, preserving old files. */
    if(!exists){const char *const *selected=profile_paths;profile_paths=legacy_profile_paths;va=profile_read(0,&a,&exists);vb=profile_read(1,&b,&exists);profile_paths=selected;}
    profile_valid_mask=va|(vb<<1);profile_slot=joey_profile_newest(&a,va,&b,vb);
    if(profile_slot>=0)joey_profile_state=profile_slot?b:a;else if(exists)message(u"存档校验失败，已载入初始牌组");
}
static void profile_message(int result){
    const uint16_t *labels[]={u"",u"主牌组四十至八十张，副卡组和融合各十五张",u"卡片类型或位置不符",u"这张卡的效果还未接入",u"超过持有数量或未加入牌组",u"超过原版同名卡数量限制",u"存档格式无效",u"请先领取上局奖励"};
    if(result<8)message(labels[result]);
}
static int profile_commit(const JoeyProfile *proposed){
    JoeyProfile candidate=*proposed,verify;candidate.sequence=joey_profile_state.sequence+1;
    int result=joey_profile_encode(&candidate,duel_card_definitions,duel_original_limits,profile_bytes,JOEY_PROFILE_SAVE_BYTES);if(result){profile_message(result);return 0;}
    uint8_t expected_crc[4];for(unsigned i=0;i<4;++i)expected_crc[i]=profile_bytes[20+i];
    unsigned slot=profile_slot==0?1:0;h1_file *f=h1_fopen(profile_paths[slot],"wb");
    if(!f){profile_save_result=2;message(u"无法写入存档，请检查存储空间");return 0;}
    profile_valid_mask&=~(1u<<slot);unsigned written=h1_fwrite(profile_bytes,1,JOEY_PROFILE_SAVE_BYTES,f);int closed=h1_fclose(f);
    int valid=written==JOEY_PROFILE_SAVE_BYTES&&closed==0&&profile_read(slot,&verify,0)&&verify.sequence==candidate.sequence;
    for(unsigned i=0;i<4;++i)if(profile_bytes[20+i]!=expected_crc[i])valid=0;
    if(!valid){profile_save_result=2;message(u"存档写入失败，上一份记录仍保留");return 0;}
    joey_profile_state=verify;joey_edit_state=verify;profile_slot=slot;profile_valid_mask|=1u<<slot;profile_save_result=1;editor_changed=0;dirty=1;return 1;
}
static int profile_save(void){int ok=profile_commit(&joey_edit_state);if(ok)message(u"牌组已保存");return ok;}
static unsigned editor_copies(unsigned pile,unsigned card){
    const uint16_t *cards=pile==JOEY_SIDE?joey_edit_state.side:pile==JOEY_FUSION?joey_edit_state.fusion:joey_edit_state.main;
    unsigned count=pile==JOEY_SIDE?joey_edit_state.side_count:pile==JOEY_FUSION?joey_edit_state.fusion_count:joey_edit_state.main_count,n=0;
    for(unsigned i=0;i<count;++i)n+=cards[i]==card;return n;
}
static unsigned editor_page_size(unsigned pane){return pane==0?14:pane==3?4:7;}
static void editor_rebuild(unsigned selected){
    for(unsigned pane=0;pane<4;++pane)editor_counts[pane]=0;
    for(unsigned card=1;card<DUEL_CARD_COUNT;++card){
        for(unsigned pane=0;pane<3;++pane)if(editor_copies(pane,card))editor_lists[pane][editor_counts[pane]++]=card;
        if(joey_edit_state.owned[card]&&(scene==4||duel_card_supported(&duel_card_definitions[card])))editor_lists[3][editor_counts[3]++]=card;
    }
    editor_count=editor_counts[editor_tab];for(unsigned i=0;i<editor_count;++i)editor_cards[i]=editor_lists[editor_tab][i];
    if(editor_index>=editor_count)editor_index=editor_count?editor_count-1:0;
    for(unsigned i=0;i<editor_count;++i)if(editor_cards[i]==selected){editor_index=i;break;}
    editor_cursor[editor_tab]=editor_index;native_selected=editor_count?editor_cards[editor_index]:0;dirty=1;
}
static void editor_select(unsigned pane,unsigned index){
    editor_cursor[editor_tab]=editor_index;editor_tab=pane;editor_index=index;
    if(pane<3)editor_destination=pane;editor_rebuild(0);
}
static void editor_open(unsigned gallery){
    scene=gallery?4:3;joey_edit_state=joey_profile_state;editor_tab=gallery?3:0;editor_index=editor_changed=editor_leaving=editor_destination=0;for(unsigned i=0;i<4;++i)editor_cursor[i]=0;details_open=pending=0;editor_rebuild(0);
}
static void editor_change(int add){
    if(scene==4||!editor_count)return;unsigned card=editor_cards[editor_index],pile=editor_tab<3?editor_tab:editor_destination;
    if(editor_tab==3&&pile!=JOEY_SIDE)pile=duel_card_definitions[card].family==DUEL_MONSTER&&duel_card_definitions[card].subtype==2?JOEY_FUSION:JOEY_MAIN;
    int result=joey_profile_change_pile(&joey_edit_state,duel_card_definitions,duel_original_limits,card,add,pile);
    if(result){profile_message(result);return;}editor_changed=1;editor_rebuild(card);
}
static void editor_page(int direction){if(!editor_count)return;int index=(int)editor_index+direction*editor_page_size(editor_tab);if(index<0)index=0;if(index>=(int)editor_count)index=editor_count-1;editor_index=index;editor_cursor[editor_tab]=index;native_selected=editor_cards[index];dirty=1;}
static void editor_leave(unsigned exit_app){
    details_open=0;
    if(scene==3&&editor_changed){editor_leaving=exit_app?2:1;editor_exit_choice=0;dirty=1;return;}
    if(exit_app)exit_requested=1;else {scene=0;details_open=pending=0;dirty=1;}
}
static void editor_exit(void){
    unsigned exit_app=editor_leaving==2;
    if(editor_exit_choice==2){editor_leaving=0;dirty=1;return;}
    if(editor_exit_choice==0&&!profile_save())return;
    editor_changed=editor_leaving=0;if(exit_app)exit_requested=1;else {scene=0;dirty=1;}
}
static void begin_duel(unsigned first){
    int result=duel_start(&joey_duel_state,duel_card_definitions,joey_profile_state.main,joey_profile_state.main_count,duel_original_deck,DUEL_ORIGINAL_DECK_COUNT,
        h1_raw_tick_80hz()^h1_timer_read_ms()^joey_native_diagnostic[2],first);
    if(!result)result=duel_fusion_setup(&joey_duel_state,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,joey_profile_state.fusion,joey_profile_state.fusion_count,duel_original_fusion_deck,DUEL_ORIGINAL_FUSION_DECK_COUNT);
    if(!result)result=duel_equip_setup(&joey_duel_state,duel_equip_rules,DUEL_EQUIP_RULE_COUNT);
    if(!result)result=duel_life_setup(&joey_duel_state,duel_life_rules,DUEL_LIFE_RULE_COUNT);
    if(!result)result=duel_recall_setup(&joey_duel_state,duel_recall_rules,DUEL_RECALL_RULE_COUNT);
    if(!result)result=duel_field_setup(&joey_duel_state,duel_field_rules,DUEL_FIELD_RULE_COUNT);
    if(!result)result=duel_aura_setup(&joey_duel_state,duel_aura_rules,DUEL_AURA_RULE_COUNT);
    if(!result)result=duel_suppression_setup(&joey_duel_state,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT);
    if(result){result_message(result);return;}scene=1;native_focus=0;native_selected=joey_duel_state.players[0].hand[0];
    ++opening_starts;pending=0;hand_scroll=0;seen_remodel=0;next_ai=h1_timer_read_ms()+230;notice=0;dirty=1;if(joey_duel_state.winner>=0)result_open();
}
static uint32_t opening_random(void *context){uint32_t x=opening_random_state;if(!x)x=0x9e3779b9u;x^=x<<13;x^=x>>17;x^=x<<5;opening_random_state=x;return x;}
static void start_game(void){
    if(joey_profile_state.reward_flags&JOEY_REWARD_PENDING){result_resume();return;}
    duel_opening_begin(&opening_state);opening_random_state=h1_raw_tick_80hz()^h1_timer_read_ms()^(joey_native_diagnostic[2]<<7);
    opening_hand_focus=opening_order_focus=0;scene=7;details_open=pending=0;native_selected=0;notice=0;dirty=1;
}
static void opening_confirm(void){
    int result=opening_state.phase==DUEL_OPEN_CHOOSE?duel_opening_throw(&opening_state,opening_hand_focus,opening_random,0):opening_state.phase==DUEL_OPEN_PLAYER_ORDER?duel_opening_order(&opening_state,opening_order_focus):duel_opening_continue(&opening_state);
    if(result){result_message(DUEL_BAD_ARGUMENT);return;}opening_hand_focus=opening_order_focus=0;dirty=1;
    if(opening_state.phase==DUEL_OPEN_READY)begin_duel(opening_state.first);
}
static void close_game(void){scene=0;details_open=0;pending=0;native_menu_focus=0;dirty=1;}
static uint32_t reward_random(void *context){
    uint32_t x=reward_random_state;if(!x)x=0x13579bdfu;x^=x<<13;x^=x>>17;x^=x<<5;reward_random_state=x;return x;
}
static void result_commit(void){
    if(!result_action)return;
    if(!profile_commit(&result_candidate)){result_error=1;dirty=1;return;}
    if(result_action==3)result_claimed=1;
    else if(joey_profile_state.reward_flags&JOEY_REWARD_PENDING){result_card=joey_profile_state.reward_card;result_new=!!(joey_profile_state.reward_flags&JOEY_REWARD_NEW);}
    else result_claimed=1;
    native_selected=result_card;result_action=result_error=0;result_choice=0;notice=0;dirty=1;
}
static void result_resume(void){
    scene=5;result_winner=0;result_card=joey_profile_state.reward_card;result_new=!!(joey_profile_state.reward_flags&JOEY_REWARD_NEW);native_selected=result_card;result_action=result_error=result_choice=result_claimed=0;details_open=pending=0;notice=0;dirty=1;
}
static void result_open(void){
    scene=5;result_winner=joey_duel_state.winner;result_card=result_new=result_choice=result_claimed=result_error=0;details_open=pending=0;
    if(result_winner==2){result_action=0;result_claimed=1;dirty=1;return;}
    result_candidate=joey_profile_state;reward_random_state=h1_raw_tick_80hz()^h1_timer_read_ms()^joey_duel_state.random_state;
    int result=joey_profile_finish(&result_candidate,result_winner==0,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0);
    if(result){result_action=0;result_error=1;profile_message(result);return;}
    result_action=1;result_commit();
}
static void result_activate(unsigned choice){
    if(result_action){result_commit();return;}
    if(!(joey_profile_state.reward_flags&JOEY_REWARD_PENDING)){close_game();return;}
    result_candidate=joey_profile_state;int result;
    if(choice&&(joey_profile_state.reward_flags&JOEY_REWARD_REROLL)){
        if(!reward_random_state)reward_random_state=h1_raw_tick_80hz()^h1_timer_read_ms();
        result=joey_profile_reroll(&result_candidate,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0);result_action=2;
    }else {result=joey_profile_claim(&result_candidate);result_action=3;}
    if(result){result_action=0;profile_message(result);return;}result_commit();
}
static void records_open(void){scene=6;details_open=pending=0;notice=0;dirty=1;}
static void select_card(unsigned card,int focus){native_selected=card<1115?card:0;native_focus=focus;dirty=1;}
static unsigned hand_visible(void){unsigned n=joey_duel_state.players[0].hand_count;return n>10?10:n;}
static int hand_x(unsigned index,unsigned count){return 180+(count>1?index*205/(count-1):0);}
static void grave_choices(unsigned own_only){
    picker_count=0;grave_page=0;grave_choice=0;
    for(unsigned owner=0;owner<(own_only?1u:2u);++owner){DuelPlayer *p=&joey_duel_state.players[owner];for(unsigned i=0;i<p->grave_count;++i){const DuelCardDef *c=&duel_card_definitions[p->grave[i]];
        if(duel_can_revive_for(&joey_duel_state,0,owner,i)){picker_owner[picker_count]=owner;picker_index[picker_count++]=i;}}}
    if(picker_count)pending=PENDING_GRAVE;else message(u"墓地中没有可复活的怪兽");dirty=1;
}
static unsigned picker_card(unsigned index){return pending==PENDING_REMODEL?joey_duel_state.remodel.cards[picker_owner[index]][picker_index[index]]:pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND?effect_options[index].card:joey_duel_state.players[picker_owner[index]].grave[picker_index[index]];}
static void public_hand_choices(void){const DuelPlayer *p=&joey_duel_state.players[1];if(!duel_hand_revealed(&joey_duel_state,1))return;picker_count=duel_public_hand_options(&joey_duel_state,1,effect_options,DUEL_OPTION_MAX);grave_page=grave_choice=0;effect_options[picker_count++]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};pending=PENDING_PUBLIC_HAND;if(p->hand_count)native_selected=p->hand[0];dirty=1;}
static void attack_choices(unsigned source){
    if(duel_attack_tribute_required(&duel_card_definitions[joey_duel_state.players[0].monsters[source].card])){DuelCommand command={0,DUEL_ACTION_ATTACK,source,1,DUEL_NO_SLOT};apply(&command);return;}
    attacker=source;picker_count=grave_choice=grave_page=0;
    for(unsigned i=0;i<5;++i){DuelSlot *s=&joey_duel_state.players[1].monsters[i];if(s->card&&duel_can_attack_target(&joey_duel_state,0,source,i))effect_options[picker_count++]=(DuelChoiceOption){s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,i,DUEL_OPTION_CARD};}
    effect_options[picker_count++]=(DuelChoiceOption){joey_duel_state.players[0].monsters[source].card,5,DUEL_OPTION_CONFIRM};effect_options[picker_count++]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};pending=PENDING_ATTACK_SELECT;dirty=1;
}
static void remodel_report(void){
    DuelRemodel *r=&joey_duel_state.remodel;if(pending||r->sequence==seen_remodel)return;
    seen_remodel=r->sequence;picker_count=grave_choice=grave_page=0;
    for(unsigned side=0;side<2;++side)for(unsigned i=0;i<r->revealed[side];++i){picker_owner[picker_count]=side;picker_index[picker_count++]=i;}
    native_selected=r->source_card;pending=PENDING_REMODEL;dirty=1;
}
static void backup_choices(void){
    DuelChoiceOption options[DUEL_PILE_MAX];picker_count=duel_backup_options(&joey_duel_state,0,options,DUEL_PILE_MAX);grave_choice=grave_page=backup_count=0;
    if(!picker_count){pending=0;message(u"墓地需有五只怪兽及可回收对象");return;}
    for(unsigned i=0;i<picker_count;++i){picker_owner[i]=0;picker_index[i]=options[i].index;picker_selected[i]=0;}
    pending=PENDING_BACKUP;dirty=1;
}
static void effect_choices(void){
    if(!joey_duel_state.choice.kind||joey_duel_state.choice.player!=0||pending==PENDING_EFFECT_CHOICE||pending==PENDING_REMODEL)return;
    picker_count=duel_choice_options(&joey_duel_state,effect_options,DUEL_OPTION_MAX);
    for(unsigned i=0;i<picker_count;++i){picker_owner[i]=0;picker_index[i]=effect_options[i].index;}
    pending=PENDING_EFFECT_CHOICE;grave_choice=grave_page=0;native_selected=joey_duel_state.choice.card;dirty=1;
}
static void cancel_picker(void){
    if(pending==PENDING_REMODEL)next_ai=h1_timer_read_ms()+230;
    if(pending==PENDING_EFFECT_CHOICE){for(unsigned i=0;i<picker_count;++i)if(effect_options[i].kind==DUEL_OPTION_SKIP){DuelCommand command={0,DUEL_ACTION_CHOOSE,effect_options[i].index,0,0,0,0,0};apply(&command);return;}message(u"效果已发动，请完成选择");return;}
    pending=0;dirty=1;
}
static unsigned activation_action(void){return activation_from_backrow==2?DUEL_ACTION_ACTIVATE_MONSTER:activation_from_backrow?DUEL_ACTION_ACTIVATE_BACKROW:DUEL_ACTION_ACTIVATE_HAND;}
static unsigned activation_card(void){DuelPlayer *p=&joey_duel_state.players[0];return activation_from_backrow==2?p->monsters[activation_source].card:activation_from_backrow?(activation_source==DUEL_FIELD_SLOT?p->field.card:p->backrow[activation_source].card):p->hand[activation_source];}
static void pass_response(void){DuelCommand command={0,DUEL_ACTION_PASS,0,0,0,0,0,0};apply(&command);}
static void response_choices(void){
    if(joey_duel_state.response_player!=0||pending)return;
    response_count=duel_response_options(&joey_duel_state,0,response_options,DUEL_OPTION_MAX);if(!response_count)return;
    pending=PENDING_RESPONSE;grave_choice=grave_page=0;native_selected=response_options[0].card;dirty=1;
}
static void begin_activation(unsigned index,unsigned from_backrow){
    activation_source=index;activation_from_backrow=from_backrow;cost_hand=0;
    const DuelCardDef *card=&duel_card_definitions[activation_card()];
    if(!duel_card_supported(card)){result_message(DUEL_UNSUPPORTED);return;}
    if(duel_card_activation_blocked(&joey_duel_state,activation_card())){result_message(DUEL_NOT_ALLOWED);return;}
    if(card->family==DUEL_MONSTER&&!from_backrow&&joey_duel_state.window.kind==DUEL_WINDOW_DAMAGE_LIFE){DuelCommand command={0,DUEL_ACTION_ACTIVATE_HAND,index};apply(&command);return;}
    if(from_backrow==1&&card->name_group==651&&joey_duel_state.players[0].backrow[index].position!=DUEL_FACEDOWN_DEFENSE){DuelCommand command={0,DUEL_ACTION_ACTIVATE_BACKROW,index};apply(&command);return;}
    if(from_backrow==2){DuelCommand command={0,DUEL_ACTION_ACTIVATE_MONSTER,index};apply(&command);return;}
    if(duel_counter_tribute_cost(card)){required_tributes=1;tribute_mask=0;tribute_monster_effect=2;pending=PENDING_TRIBUTE;message(u"请点选效果的祭品，再按确认");return;}
    if(card->effect==DUEL_E_REBORN||card->effect==DUEL_E_CALL_HAUNTED){grave_choices(card->effect==DUEL_E_CALL_HAUNTED);return;}
    if(card->effect==DUEL_E_LIFE){const DuelLifeRule *r=duel_life_rule(&joey_duel_state,activation_card());if(r&&r->target==DUEL_LIFE_EITHER){pending=PENDING_LIFE_TARGET;life_choice=0;dirty=1;return;}}
    if(card->name_group==650){DuelCommand command={0,activation_action(),index};apply(&command);return;}
    if(card->effect==DUEL_E_BACKUP){backup_choices();return;}
    if(card->effect==DUEL_E_CHAIN_DESTRUCTION){DuelWindow *w=&joey_duel_state.window;DuelCommand command={0,activation_action(),index,w->player,w->source,0,0,0};apply(&command);return;}
    if(card->effect==DUEL_E_TRIBUTE_DOOMED||card->effect==DUEL_E_MAGIC_JAMMER){
        if(joey_duel_state.players[0].hand_count<(from_backrow?1u:2u)){result_message(DUEL_NOT_ALLOWED);return;}
        pending=PENDING_DISCARD_COST;message(u"请选择一张手牌作为代价");return;
    }
    if(card->effect==DUEL_E_MST||card->effect==DUEL_E_DE_SPELL||card->effect==DUEL_E_REMOVE_TRAP||card->effect==DUEL_E_CHANGE_HEART||card->effect==DUEL_E_EQUIP||card->effect==DUEL_E_BIND||duel_simple_target_kind(card)){pending=PENDING_SPELL_TARGET;message(card->effect==DUEL_E_EQUIP?u"请选择装备对象":u"请点选卡片效果的对象");return;}
    DuelCommand command={0,activation_action(),index,0,0,0,0,0};apply(&command);
}
static void summon_hand(unsigned index,unsigned secondary){DuelPlayer *p=&joey_duel_state.players[0];const DuelCardDef *card=&duel_card_definitions[p->hand[index]];required_tributes=duel_normal_tribute_cost(&joey_duel_state,0,index,secondary);tribute_mask=0;tribute_monster_effect=0;
    if(required_tributes){pending=PENDING_TRIBUTE;grave_position=secondary;message(u"请点选场上的祭品，再按确认");return;}int target=empty(p->monsters);if(target<0){result_message(DUEL_ZONE_FULL);return;}DuelCommand command={0,secondary?DUEL_ACTION_SET_MONSTER:DUEL_ACTION_SUMMON,index,0,target,0,0};apply(&command);
}
static void begin_hand(unsigned index,unsigned secondary){
    DuelPlayer *p=&joey_duel_state.players[0];if(index>=p->hand_count)return;
    if(pending==PENDING_DISCARD_COST){if(!activation_from_backrow&&index==activation_source){result_message(DUEL_BAD_TARGET);return;}cost_hand=index;
        if(duel_card_definitions[activation_card()].effect==DUEL_E_MAGIC_JAMMER){DuelCommand command={0,activation_action(),activation_source,0,0,0,0,cost_hand};apply(&command);}
        else {pending=PENDING_SPELL_TARGET;message(u"请选择要破坏的怪兽");}return;}
    source_hand=index;cost_hand=0;tribute_monster_effect=0;
    if(joey_duel_state.discard_required){DuelCommand command={0,DUEL_ACTION_DISCARD,index,0,0,0,0};apply(&command);return;}
    const DuelCardDef *card=&duel_card_definitions[p->hand[index]];
    if(!duel_card_supported(card)){result_message(DUEL_UNSUPPORTED);return;}
    if(card->family==DUEL_MONSTER){
        if(!secondary&&duel_hand_monster_effect(card)){monster_choice=0;pending=PENDING_HAND_EFFECT;dirty=1;return;}summon_hand(index,secondary);return;
    }
    if(secondary||card->family==DUEL_TRAP){int target=card->effect==DUEL_E_FIELD?DUEL_FIELD_SLOT:empty(p->backrow);if(target<0){result_message(DUEL_ZONE_FULL);return;}DuelCommand command={0,DUEL_ACTION_SET_BACKROW,index,0,target,0,0};apply(&command);return;}
    begin_activation(index,0);
}
static void field_action(unsigned player,unsigned index,unsigned backrow,unsigned secondary){
    DuelPlayer *p=&joey_duel_state.players[player];DuelSlot *s=backrow?(index==DUEL_FIELD_SLOT?&p->field:&p->backrow[index]):&p->monsters[index];
    if(pending==PENDING_SPELL_TARGET){unsigned effect=duel_card_definitions[activation_card()].effect,needs_backrow=effect==DUEL_E_MST||effect==DUEL_E_DE_SPELL||effect==DUEL_E_REMOVE_TRAP||duel_simple_target_kind(&duel_card_definitions[activation_card()])==2;
        if(backrow!=needs_backrow){result_message(DUEL_BAD_TARGET);return;}DuelCommand command={0,activation_action(),activation_source,player,index,0,0,cost_hand};apply(&command);return;}
    if(pending==PENDING_MONSTER_TARGET&&duel_monster_effect_target(&duel_card_definitions[joey_duel_state.players[0].monsters[monster_source].card])==2){if(!backrow&&duel_blast_can_target(&joey_duel_state,0,monster_source,player,index)){tribute_mask^=1u<<(player*5+index);dirty=1;unsigned count=0;DuelCommand command={0,DUEL_ACTION_ACTIVATE_MONSTER,monster_source};for(unsigned i=0;i<10;++i)if(tribute_mask&(1u<<i)){if(count<2)command.targets[count]=i;++count;}if(count==2){command.target_count=2;apply(&command);}}return;}
    if(pending==PENDING_MONSTER_TARGET){if(player==1&&!backrow&&s->card){DuelCommand command={0,DUEL_ACTION_ACTIVATE_MONSTER,monster_source,player,index};apply(&command);}return;}
    if(pending==PENDING_TRIBUTE){if(!backrow&&duel_can_normal_tribute(&joey_duel_state,0,player,index)){tribute_mask^=1u<<(player*5+index);dirty=1;}return;}
    if(pending==PENDING_ATTACK){if(player==1&&!backrow&&s->card){DuelCommand command={0,DUEL_ACTION_ATTACK,attacker,1,index,0,0};apply(&command);}return;}
    if(!pending&&!secondary&&backrow&&index==DUEL_FIELD_SLOT&&duel_fusion_gate_available(&joey_duel_state,0,player)){DuelCommand command={0,DUEL_ACTION_FUSION_GATE,player};apply(&command);return;}
    if(player!=0||!s->card)return;if(backrow){begin_activation(index,1);return;}
    if(joey_duel_state.phase==DUEL_BATTLE){
        if(duel_magnet_target_mode(&joey_duel_state,0,index)==1){for(unsigned i=0;i<5;++i)if(duel_can_attack_target(&joey_duel_state,0,index,i)){DuelCommand command={0,DUEL_ACTION_ATTACK,index,1,i};apply(&command);return;}}
        unsigned targets=0;for(unsigned i=0;i<5;++i)targets+=joey_duel_state.players[1].monsters[i].card!=0;
        if(!targets){DuelCommand command={0,DUEL_ACTION_ATTACK,index,1,DUEL_NO_SLOT,0,0};apply(&command);}
        else if(duel_can_attack_direct(&joey_duel_state,0,index))attack_choices(index);
        else {attacker=index;pending=PENDING_ATTACK;message(u"请选择攻击对象");}return;
    }
    if(!secondary&&s->position!=DUEL_FACEDOWN_DEFENSE&&duel_monster_has_ignition(&duel_card_definitions[s->card])){monster_source=index;monster_choice=0;pending=PENDING_MONSTER_MENU;dirty=1;return;}
    unsigned position=s->position==DUEL_FACEUP_ATTACK?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK;
    DuelCommand command={0,DUEL_ACTION_POSITION,index,0,0,position,0};apply(&command);
}
static void confirm_pending(void){
    if(pending==PENDING_PUBLIC_HAND){if(effect_options[grave_choice].kind==DUEL_OPTION_SKIP){cancel_picker();return;}native_selected=picker_card(grave_choice);details_open=1;detail_scroll=0;dirty=1;return;}
    if(pending==PENDING_HAND_EFFECT){if(monster_choice==2){cancel_picker();return;}if(monster_choice==1){pending=0;summon_hand(source_hand,0);return;}if(duel_card_definitions[joey_duel_state.players[0].hand[source_hand]].effect==DUEL_E_EQUIP){pending=0;begin_activation(source_hand,0);return;}DuelCommand command={0,DUEL_ACTION_ACTIVATE_HAND,source_hand};apply(&command);return;}
    if(pending==PENDING_MONSTER_MENU){if(monster_choice==2){cancel_picker();return;}if(monster_choice==1){pending=0;field_action(0,monster_source,0,1);return;}const DuelCardDef *def=&duel_card_definitions[joey_duel_state.players[0].monsters[monster_source].card];if(duel_monster_effect_target(def)){tribute_mask=0;pending=PENDING_MONSTER_TARGET;message(duel_monster_effect_target(def)==2?u"请点选两张怪兽作为对象":u"请选择对方怪兽作为对象");return;}if(duel_monster_effect_coin(def)){pending=PENDING_COIN_GUESS;coin_guess=0;dirty=1;return;}required_tributes=duel_monster_tribute_cost(def);if(!required_tributes){DuelCommand command={0,DUEL_ACTION_ACTIVATE_MONSTER,monster_source};apply(&command);return;}tribute_mask=0;tribute_monster_effect=1;pending=PENDING_TRIBUTE;message(u"请点选效果的祭品，再按确认");return;}
    if(pending==PENDING_COIN_GUESS){DuelCommand command={0,DUEL_ACTION_ACTIVATE_MONSTER,monster_source,0,0,coin_guess};apply(&command);return;}
    if(pending==PENDING_LIFE_TARGET){DuelCommand command={0,activation_action(),activation_source,life_choice,0,0,0,0};apply(&command);return;}
    if(pending==PENDING_ATTACK_SELECT){DuelChoiceOption option=effect_options[grave_choice];if(option.kind==DUEL_OPTION_SKIP){cancel_picker();return;}DuelCommand command={0,DUEL_ACTION_ATTACK,attacker,1,option.index==5?DUEL_NO_SLOT:option.index,0,0};apply(&command);return;}
    if(pending==PENDING_REMODEL){if(grave_choice==picker_count)cancel_picker();else{native_selected=picker_card(grave_choice);dirty=1;}return;}
    if(pending==PENDING_BACKUP){
        if(grave_choice==picker_count){if(!backup_count){message(u"请至少选择一只怪兽");return;}DuelCommand command={0,activation_action(),activation_source,0,0,0,0,0};
            for(unsigned i=0;i<picker_count;++i)if(picker_selected[i])command.targets[command.target_count++]=picker_index[i];apply(&command);return;}
        if(picker_selected[grave_choice]){picker_selected[grave_choice]=0;--backup_count;}
        else {unsigned hands=0;for(unsigned i=0;i<picker_count;++i)hands+=picker_selected[i]&&duel_card_definitions[picker_card(i)].subtype!=2;if(backup_count>=3||(duel_card_definitions[picker_card(grave_choice)].subtype!=2&&joey_duel_state.players[0].hand_count+hands>=DUEL_HAND_MAX)){message(u"最多选择三只怪兽");return;}picker_selected[grave_choice]=1;++backup_count;}
        dirty=1;return;
    }
    if(pending==PENDING_RESPONSE){if(grave_choice==response_count)pass_response();else begin_activation(response_options[grave_choice].index,response_options[grave_choice].from_backrow);return;}
    if(pending==PENDING_EFFECT_CHOICE){DuelCommand command={0,DUEL_ACTION_CHOOSE,picker_index[grave_choice],0,0,0,0,0};apply(&command);return;}
    if(pending==PENDING_TRIBUTE){unsigned count=0;for(unsigned i=0;i<10;++i)count+=(tribute_mask>>i)&1;
        if(tribute_monster_effect==2){if(count!=1){result_message(DUEL_NEED_TRIBUTES);return;}DuelCommand command={0,activation_action(),activation_source,0,0,0,tribute_mask};apply(&command);return;}
        if(tribute_monster_effect){if(count!=required_tributes){result_message(DUEL_NEED_TRIBUTES);return;}DuelCommand command={0,DUEL_ACTION_ACTIVATE_MONSTER,monster_source,0,0,0,tribute_mask};apply(&command);return;}
        if(count!=required_tributes){result_message(DUEL_NEED_TRIBUTES);return;}int target=empty(joey_duel_state.players[0].monsters);if(target<0)for(unsigned i=0;i<5;++i)if((tribute_mask&(1u<<i))&&!joey_duel_state.players[0].monsters[i].zone_blocked){target=i;break;}
        DuelCommand command={0,grave_position?DUEL_ACTION_SET_MONSTER:DUEL_ACTION_SUMMON,source_hand,0,target,0,tribute_mask};apply(&command);return;}
    if(pending==PENDING_GRAVE){if(duel_card_definitions[activation_card()].effect==DUEL_E_CALL_HAUNTED){DuelCommand command={0,activation_action(),activation_source,0,picker_index[grave_choice],DUEL_FACEUP_ATTACK,0};apply(&command);return;}pending=PENDING_POSITION;grave_position=0;dirty=1;return;}
    if(pending==PENDING_POSITION){DuelCommand command={0,activation_action(),activation_source,picker_owner[grave_choice],picker_index[grave_choice],grave_position,0};apply(&command);}
}
static void touch(int x,int y,unsigned secondary){
    joey_audio_effect(JOEY_SOUND_CURSOR);
    if(about_open){if(x>=184&&x<296&&y>=222&&y<254){about_open=0;dirty=1;}return;}
    if(details_open){if(y<26&&x>420){details_open=0;dirty=1;}return;}
    if(scene==0){if(x>=414&&x<472&&y>=243&&y<267){native_menu_focus=5;about_open=1;notice=0;dirty=1;return;}if(x>=148&&x<332&&y>=143&&y<253){native_menu_focus=(y-143)/22;if(native_menu_focus==0)start_game();else if(native_menu_focus==1||native_menu_focus==2)editor_open(native_menu_focus==2);else if(native_menu_focus==3)records_open();else if(native_menu_focus==4)exit_requested=1;}return;}
    if(scene==7){
        if(opening_state.phase==DUEL_OPEN_CHOOSE&&y>=137&&y<232){for(unsigned i=0;i<3;++i)if(x>=32+(int)i*161&&x<128+(int)i*161){opening_hand_focus=i;opening_confirm();break;}}
        else if(opening_state.phase==DUEL_OPEN_PLAYER_ORDER&&y>=103&&y<137){if(x>=82&&x<222){opening_order_focus=0;opening_confirm();}else if(x>=258&&x<398){opening_order_focus=1;opening_confirm();}}
        else if((opening_state.phase==DUEL_OPEN_DRAW||opening_state.phase==DUEL_OPEN_NPC_ORDER)&&x>=150&&x<331&&y>=103&&y<137)opening_confirm();return;
    }
    if(scene==5){if(y>=222&&y<257){result_choice=x>=298;result_activate(result_choice);}else if(result_card&&x>=50&&x<147&&y>=55&&y<195){details_open=1;detail_scroll=0;dirty=1;}return;}
    if(scene==6){if(y>=222)close_game();return;}
    if(scene==3||scene==4){
        if(editor_leaving){if(y>=145&&y<181&&x>=35&&x<455){editor_exit_choice=(x-35)/140;editor_exit();}return;}
        if(y>=241){if(x<110)editor_leave(0);else if(x<179)editor_change(1);else if(x<240)editor_change(0);else if(x<302&&scene==3)profile_save();else if(x<370)editor_leave(0);else editor_page(x<425?-1:1);return;}
        if(x<110){if(y<36&&x<55&&scene==3)profile_save();else if(native_selected){details_open=1;detail_scroll=0;dirty=1;}return;}
        if(x>=372){unsigned index=editor_cursor[3]/4*4+(y>=25?(y-25)/50:0);if(index>=editor_counts[3])index=editor_counts[3]?editor_counts[3]-1:0;editor_select(3,index);return;}
        if(scene==4)return;
        unsigned pane=y>=204?JOEY_FUSION:y>=158?JOEY_SIDE:JOEY_MAIN,index=editor_cursor[pane];
        if(pane==JOEY_MAIN&&y>=28&&y<133&&x>=123&&x<361)index=index/14*14+((y-28)/53)*7+(x-123)/34;
        else if(pane!=JOEY_MAIN&&x>=124&&x<264&&y>=(pane==JOEY_SIDE?175:215))index=index/7*7+(x-124)/20;
        editor_select(pane,index);return;
    }
    if(scene==2||joey_duel_state.winner>=0){close_game();return;}
    if(scene==1&&!pending&&ai_stage!=1&&y<32&&x>=185&&x<375&&duel_hand_revealed(&joey_duel_state,1)){public_hand_choices();return;}
    if(scene==1&&ai_stage==1){if(x<154){details_open=1;detail_scroll=0;dirty=1;}return;}
    if(pending==PENDING_MONSTER_MENU||pending==PENDING_HAND_EFFECT){if(x>=160&&x<430&&y>=80&&y<192&&((y-80)%40)<32){monster_choice=(y-80)/40;confirm_pending();}return;}
    if(pending==PENDING_COIN_GUESS){if(y>=147&&y<181&&((x>=145&&x<290)||(x>=310&&x<455))){coin_guess=x>=310;confirm_pending();}return;}
    if(pending==PENDING_LIFE_TARGET){if(y>=147&&y<181&&((x>=145&&x<290)||(x>=310&&x<455))){life_choice=x>=310;confirm_pending();}return;}
    if(pending==PENDING_REMODEL){
        if(y>=208&&y<242){if(x<170&&grave_page){--grave_page;grave_choice=grave_page*5;dirty=1;}else if(x>=400&&(grave_page+1)*5<picker_count){++grave_page;grave_choice=grave_page*5;dirty=1;}else if(x>=200&&x<380){grave_choice=picker_count;confirm_pending();}return;}
        if(y>=80&&y<200&&x>=104){unsigned i=grave_page*5+(x-104)/75;if(i<picker_count){grave_choice=i;native_selected=picker_card(i);dirty=1;}}return;
    }
    if(pending==PENDING_BACKUP){
        if(y>=176&&y<210){if(x>=130&&x<280)cancel_picker();else if(x>=300){grave_choice=picker_count;confirm_pending();}return;}
        if(y>=74&&y<165&&x>=104){unsigned i=grave_page*5+(x-104)/75;if(i<picker_count){if(grave_choice==i)confirm_pending();else{grave_choice=i;native_selected=picker_card(i);dirty=1;}}}return;
    }
    if(pending==PENDING_RESPONSE){
        if(y>=176&&y<210){if(x>=130&&x<280)pass_response();else if(x>=300)confirm_pending();return;}
        if(y>=74&&y<165&&x>=104){unsigned i=grave_page*5+(x-104)/75;if(i<response_count){if(grave_choice==i)confirm_pending();else{grave_choice=i;native_selected=response_options[i].card;dirty=1;}}}return;
    }
    if(pending==PENDING_PUBLIC_HAND&&y>=176&&y<210){if(x<180&&grave_page){--grave_page;grave_choice=grave_page*5;native_selected=picker_card(grave_choice);dirty=1;}else if(x>=390&&(grave_page+1)*5<picker_count){++grave_page;grave_choice=grave_page*5;native_selected=picker_card(grave_choice);dirty=1;}else if(x>=200&&x<380)cancel_picker();return;}
    if(pending==PENDING_GRAVE||pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND){if(y>=74&&y<165&&x>=104){unsigned i=grave_page*5+(x-104)/75;if(i<picker_count){if(grave_choice==i)confirm_pending();else{grave_choice=i;unsigned card=picker_card(i);native_selected=card?card:joey_duel_state.choice.card;dirty=1;}}}return;}
    if(pending==PENDING_POSITION){grave_position=x>=285;confirm_pending();return;}
    if(x<154){details_open=1;detail_scroll=0;dirty=1;return;}
    if(x>=154&&x<175&&y>=36&&y<240){command_phase((y-36)/34);return;}
    if(joey_duel_state.active!=0&&joey_duel_state.response_player!=0)return;
    if(x>=429&&x<472&&((y>=37&&y<95)||(y>=181&&y<239))){unsigned player=y<100?1:0;DuelSlot *s=&joey_duel_state.players[player].field;int focus=player?120:121;if(pending||native_focus==focus)field_action(player,DUEL_FIELD_SLOT,1,secondary);else select_card(player&&s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,focus);return;}
    if(y>=240&&x>=180&&x<421){unsigned shown=hand_visible();if(shown){unsigned index=hand_scroll+(x-180)*shown/241;if(index>=joey_duel_state.players[0].hand_count)return;
            if(native_focus==(int)index)begin_hand(index,secondary);else select_card(joey_duel_state.players[0].hand[index],index);}return;}
    if(x>=207&&x<386&&y>=37&&y<235){unsigned row=y<87?0:y<136?1:y<185?2:3,column=(x-207)*5/179,player=row<2?1:0,index=player?4-column:column,backrow=row==0||row==3;
        DuelSlot *s=backrow?&joey_duel_state.players[player].backrow[index]:&joey_duel_state.players[player].monsters[index];int focus=100+row*5+column;
        if(pending||native_focus==focus)field_action(player,index,backrow,secondary);
        else select_card(player&&s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,focus);
    }
}
static void activate(unsigned secondary){
    joey_audio_effect(JOEY_SOUND_DECIDE);
    if(about_open){about_open=0;dirty=1;return;}
    if(details_open){details_open=0;dirty=1;return;}if(scene==0){if(native_menu_focus==5)touch(442,254,secondary);else touch(240,151+native_menu_focus*22,secondary);return;}if(scene==7){opening_confirm();return;}
    if(scene==5){result_activate(result_choice);return;}if(scene==6){close_game();return;}
    if(scene==3||scene==4){if(editor_leaving)editor_exit();else if(scene==3)editor_change(secondary?editor_tab!=3:editor_tab==3);else {details_open=1;detail_scroll=0;dirty=1;}return;}
    if(scene==1&&ai_stage==1)return;
    if(joey_duel_state.winner>=0){close_game();return;}
    if(pending==PENDING_TRIBUTE){unsigned count=0;for(unsigned i=0;i<10;++i)count+=(tribute_mask>>i)&1;
        if(count!=required_tributes&&native_focus>=100&&native_focus<105){field_action(1,native_focus-100,0,secondary);return;}
        if(count!=required_tributes&&native_focus>=110&&native_focus<115){field_action(0,native_focus-110,0,secondary);return;}
        confirm_pending();return;
    }
    if(pending==PENDING_GRAVE||pending==PENDING_POSITION||pending==PENDING_EFFECT_CHOICE||pending==PENDING_RESPONSE||pending==PENDING_BACKUP||pending==PENDING_REMODEL||pending==PENDING_ATTACK_SELECT||pending==PENDING_LIFE_TARGET||pending==PENDING_MONSTER_MENU||pending==PENDING_HAND_EFFECT||pending==PENDING_COIN_GUESS||pending==PENDING_PUBLIC_HAND){confirm_pending();return;}
    if(native_focus>=200&&native_focus<206){command_phase(native_focus-200);return;}
    if(native_focus==120||native_focus==121){field_action(native_focus==120,DUEL_FIELD_SLOT,1,secondary);return;}
    if(native_focus>=100&&native_focus<120){unsigned cell=native_focus-100,row=cell/5,column=cell%5,player=row<2?1:0;field_action(player,player?4-column:column,row==0||row==3,secondary);return;}
    if(native_focus<0)native_focus=0;begin_hand(native_focus,secondary);
}
static void move(unsigned key){
    joey_audio_effect(JOEY_SOUND_CURSOR);
    if(about_open)return;
    if(details_open){if(key==H1_KEY_UP&&detail_scroll) --detail_scroll;if(key==H1_KEY_DOWN)++detail_scroll;dirty=1;return;}
    if(scene==0){native_menu_focus=(native_menu_focus+(key==H1_KEY_UP||key==H1_KEY_LEFT?5:1))%6;dirty=1;return;}
    if(scene==7){if(opening_state.phase==DUEL_OPEN_CHOOSE)opening_hand_focus=(opening_hand_focus+(key==H1_KEY_UP||key==H1_KEY_LEFT?2:1))%3;else if(opening_state.phase==DUEL_OPEN_PLAYER_ORDER)opening_order_focus=key==H1_KEY_RIGHT||key==H1_KEY_DOWN;dirty=1;return;}
    if(scene==5){if(!result_action&&(joey_profile_state.reward_flags&JOEY_REWARD_REROLL))result_choice^=1;dirty=1;return;}if(scene==6)return;
    if(scene==3||scene==4){
        if(editor_leaving){editor_exit_choice=(editor_exit_choice+(key==H1_KEY_LEFT||key==H1_KEY_UP?2:1))%3;dirty=1;return;}
        if(key==H1_KEY_LEFT||key==H1_KEY_RIGHT){if(scene==3){unsigned pane=(editor_tab+(key==H1_KEY_LEFT?3:1))%4;editor_select(pane,editor_cursor[pane]);}else editor_page(key==H1_KEY_LEFT?-1:1);}
        else if(editor_count){editor_index=(editor_index+(key==H1_KEY_UP?editor_count-1:1))%editor_count;editor_cursor[editor_tab]=editor_index;native_selected=editor_cards[editor_index];dirty=1;}return;
    }
    if(pending==PENDING_MONSTER_MENU||pending==PENDING_HAND_EFFECT){monster_choice=(monster_choice+(key==H1_KEY_UP||key==H1_KEY_LEFT?2:1))%3;dirty=1;return;}
    if(pending==PENDING_COIN_GUESS){coin_guess=key==H1_KEY_RIGHT||key==H1_KEY_DOWN;dirty=1;return;}
    if(pending==PENDING_LIFE_TARGET){life_choice=key==H1_KEY_RIGHT||key==H1_KEY_DOWN;dirty=1;return;}
    if(pending==PENDING_REMODEL){grave_choice=(grave_choice+(key==H1_KEY_LEFT||key==H1_KEY_UP?picker_count:1))%(picker_count+1);grave_page=(grave_choice==picker_count?(picker_count?picker_count-1:0):grave_choice)/5;if(grave_choice<picker_count)native_selected=picker_card(grave_choice);dirty=1;return;}
    if(pending==PENDING_RESPONSE){grave_choice=(grave_choice+(key==H1_KEY_LEFT||key==H1_KEY_UP?response_count:1))%(response_count+1);grave_page=(grave_choice==response_count?response_count-1:grave_choice)/5;if(grave_choice<response_count)native_selected=response_options[grave_choice].card;dirty=1;return;}
    if(pending==PENDING_BACKUP){grave_choice=(grave_choice+(key==H1_KEY_LEFT||key==H1_KEY_UP?picker_count:1))%(picker_count+1);grave_page=(grave_choice==picker_count?picker_count-1:grave_choice)/5;if(grave_choice<picker_count)native_selected=picker_card(grave_choice);dirty=1;return;}
    if(pending==PENDING_GRAVE||pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND){if(picker_count){grave_choice=(grave_choice+(key==H1_KEY_LEFT||key==H1_KEY_UP?picker_count-1:1))%picker_count;grave_page=grave_choice/5;unsigned card=picker_card(grave_choice);native_selected=card?card:joey_duel_state.choice.card;}dirty=1;return;}
    if(pending==PENDING_POSITION){grave_position=key==H1_KEY_RIGHT||key==H1_KEY_DOWN;dirty=1;return;}
    int focus=native_focus;unsigned n=joey_duel_state.players[0].hand_count;if(focus<0)focus=n?0:110;
    if(focus==120||focus==121){if(key==H1_KEY_UP)focus=120;else if(key==H1_KEY_DOWN)focus=121;else if(key==H1_KEY_LEFT)focus=focus==120?104:119;else if(key==H1_KEY_RIGHT)focus=202;}
    else if(focus>=200){int phase=focus-200;if(key==H1_KEY_UP){if(phase)phase--;else focus=120;}if(key==H1_KEY_DOWN){if(phase<5)phase++;else focus=121;}if(focus>=200)focus=key==H1_KEY_LEFT?114:200+phase;}
    else if(focus>=100){int row=(focus-100)/5,col=(focus-100)%5;if(key==H1_KEY_LEFT&&col)col--;if(key==H1_KEY_RIGHT){if(col<4)col++;else focus=202;}if(key==H1_KEY_UP&&row)row--;if(key==H1_KEY_DOWN){if(row<3)row++;else focus=n?(col*n/5):205;}if(focus>=100&&focus<200)focus=100+row*5+col;}
    else{if(key==H1_KEY_LEFT&&focus)focus--;if(key==H1_KEY_RIGHT&&focus+1<(int)n)focus++;if(key==H1_KEY_UP)focus=110+(n?focus*5/n:0);if(key==H1_KEY_DOWN)focus=205;}
    native_focus=focus;
    if(focus>=0&&focus<(int)n){native_selected=joey_duel_state.players[0].hand[focus];if((unsigned)focus<hand_scroll)hand_scroll=focus;if((unsigned)focus>=hand_scroll+10)hand_scroll=focus-9;}
    else if(focus>=100&&focus<120){unsigned row=(focus-100)/5,col=(focus-100)%5,player=row<2?1:0,index=player?4-col:col;DuelSlot *s=row==0||row==3?&joey_duel_state.players[player].backrow[index]:&joey_duel_state.players[player].monsters[index];native_selected=player&&s->position==DUEL_FACEDOWN_DEFENSE?0:s->card;}
    else if(focus==120||focus==121){unsigned player=focus==120;DuelSlot *s=&joey_duel_state.players[player].field;native_selected=player&&s->position==DUEL_FACEDOWN_DEFENSE?0:s->card;}
    dirty=1;
}
static void details_draw(void){
    native_panel(0,0,480,272);native_card(5,30,native_selected,0);uint16_t text[640];NativeCard *card=&native_cards[native_selected];
    unsigned count=native_string(card->name,card->name_length,text,640);native_wrapped(107,5,280,text,count,20,2,0x35230f);native_label(438,3,u"返回",16,0x513819);
    if(duel_card_definitions[native_selected].family==DUEL_MONSTER){native_label(6,174,u"攻击",16,0x513819);native_number(42,174,card->attack,16,0x211609);native_label(6,198,u"守备",16,0x513819);native_number(42,198,card->defense,16,0x211609);}
    count=native_string(card->description,card->description_length,text,640);unsigned line=0,column=0,skip=0;
    for(unsigned i=0;i<count;++i){unsigned advance=text[i]<128?8:16;if(text[i]=='\n'||column+advance>365){++line;column=0;if(line==detail_scroll)skip=i+(text[i]=='\n');if(text[i]=='\n')continue;}column+=advance;}
    if(detail_scroll>line)detail_scroll=line;native_wrapped(107,57,365,text+skip,count-skip,16,11,0x35230f);
}
static void result_draw(void){
    native_backdrop(2);native_panel(22,18,436,238);native_label(result_winner?208:210,25,result_winner==2?u"平局":result_winner?u"败北":joey_duel_state.turn?u"胜利":u"上局奖励",24,0x35230f);
    if(result_card){
        native_card(54,60,result_card,0);uint16_t text[160];NativeCard *c=&native_cards[result_card];unsigned n=native_string(c->name,c->name_length,text,160);native_wrapped(160,66,280,text,n,18,2,0x35230f);
        if(result_new)native_label(396,48,u"新卡",16,0x8f3010);
        native_label(160,121,result_claimed?u"已加入收藏":u"收下后加入收藏",18,0x35230f);native_label(160,148,u"持有",16,0x35230f);native_number(204,148,joey_profile_state.owned[result_card],16,0x35230f);native_label(242,148,u"张",16,0x35230f);
        native_label(160,175,u"换卡进度",16,0x35230f);native_number(244,175,joey_profile_state.reward_gauge,16,0x35230f);native_label(267,175,u"/20",16,0x35230f);
        native_label(160,198,joey_profile_state.reward_flags&JOEY_REWARD_REROLL?u"可免费换卡一次":u"点卡片查看说明",14,0x35230f);
    }else {native_label(result_winner==2?118:164,78,result_winner==2?u"双方同时满足决斗结束条件":u"下次再来挑战！",22,0x35230f);native_label(120,127,u"累计胜利",18,0x35230f);native_number(220,127,joey_profile_state.wins,18,0x35230f);native_label(120,162,u"累计败北",18,0x35230f);native_number(220,162,joey_profile_state.losses,18,0x35230f);}
    if(result_action){native_label(160,198,u"记录未保存，请重试",16,0x8f3010);native_button(170,222,190,u"重试保存");}
    else if(joey_profile_state.reward_flags&JOEY_REWARD_REROLL){native_button(145,222,140,u"收下卡片");native_button(298,222,140,u"换一张");native_outline(result_choice?296:143,220,144,36,0xffe0);}
    else native_button(173,222,190,joey_profile_state.reward_flags&JOEY_REWARD_PENDING?u"收下卡片":u"返回菜单");
}
static void records_draw(void){
    unsigned definitions=0,copies=0;for(unsigned i=1;i<DUEL_CARD_COUNT;++i){definitions+=joey_profile_state.owned[i]!=0;copies+=joey_profile_state.owned[i];}
    native_backdrop(2);native_panel(28,18,424,238);native_label(192,26,u"对局记录",24,0x35230f);
    const uint16_t *labels[]={u"累计胜利",u"累计败北",u"持有种类",u"累计张数",u"换卡进度"};unsigned values[]={joey_profile_state.wins,joey_profile_state.losses,definitions,copies,joey_profile_state.reward_gauge};
    for(unsigned i=0;i<5;++i){native_label(96,68+i*29,labels[i],18,0x35230f);native_number(254,68+i*29,values[i],18,0x35230f);}
    native_label(310,126,u"/1114",16,0x35230f);native_label(284,184,u"/20",16,0x35230f);native_button(176,222,128,u"返回");
}
static void editor_draw(void){
    scene_background(1);native_label(20,7,scene==4?u"卡片鉴赏":u"组建卡组",16,0x6d1f0e);native_label(20,24,u"保存",12,0xffffff);native_label(75,24,u"详情",12,0xffffff);
    scene_card(16,43,native_selected,0,77,112);uint16_t text[640];
    if(native_selected){NativeCard *c=&native_cards[native_selected];unsigned n=native_string(c->name,c->name_length,text,640);native_wrapped(5,162,99,text,n,12,2,0x35230f);
        if(duel_card_definitions[native_selected].family==DUEL_MONSTER){native_label(5,193,u"攻",12,0x35230f);native_number(18,193,c->attack,12,0x35230f);native_label(56,193,u"守",12,0x35230f);native_number(69,193,c->defense,12,0x35230f);}
        n=native_string(c->description,c->description_length,text,640);native_wrapped(5,208,96,text,n,12,2,0x35230f);}
    native_label(38,246,u"返回",18,0x6d1f0e);
    native_label(122,8,u"主牌组",14,0x35230f);native_number(169,9,joey_edit_state.main_count,12,0x35230f);
    const unsigned totals[]={joey_edit_state.main_count,joey_edit_state.fusion_count,joey_edit_state.side_count};
    for(unsigned pane=0;pane<3;++pane){unsigned size=editor_page_size(pane),page=editor_cursor[pane]/size,count=editor_counts[pane];
        if(pane){unsigned y=pane==JOEY_SIDE?160:204;native_number(124,y,totals[pane],10,0x35230f);native_label(138,y,u"/15",10,0x35230f);}
        if(editor_destination==pane&&scene==3)native_outline(115,pane==0?4:pane==JOEY_SIDE?159:204,253,pane==0?151:pane==JOEY_SIDE?41:35,0xffe0);
        if(pane==0){native_number(260,9,page+1,12,0x35230f);native_label(275,9,u"/",12,0x35230f);native_number(286,9,(count+13)/14,12,0x35230f);}
        for(unsigned i=0;i<size&&page*size+i<count;++i){unsigned index=page*size+i,card=editor_lists[pane][index];int x,y;
            if(pane==0){x=123+(i%7)*34;y=28+(i/7)*53;native_card(x,y,card,2);native_fill(x+21,y+33,11,12,0x211609);native_number(x+24,y+33,editor_copies(pane,card),10,0xffffff);}
            else {x=124+i*20;y=pane==JOEY_SIDE?175:215;scene_card(x,y,card,2,16,23);}
            if(editor_tab==pane&&index==editor_index)native_outline(x-1,y-1,pane==0?34:18,pane==0?48:25,0xffe0);
        }
    }
    native_label(393,5,u"持有卡",14,0x35230f);unsigned page=editor_cursor[3]/4;
    for(unsigned i=0;i<4&&page*4+i<editor_counts[3];++i){unsigned index=page*4+i,card=editor_lists[3][index],y=28+50*i;scene_card(377,y,card,2,21,30);NativeCard *c=&native_cards[card];unsigned n=native_string(c->name,c->name_length,text,640);native_wrapped(403,y,72,text,n,12,2,0xe3f1ff);
        if(duel_card_definitions[card].family==DUEL_MONSTER){native_label(403,y+27,u"攻",10,0xffe0b1);native_number(414,y+27,c->attack,10,0xffe0b1);native_label(439,y+27,u"守",10,0xffe0b1);native_number(450,y+27,c->defense,10,0xffe0b1);}
        native_number(376,y+32,joey_profile_count(&joey_edit_state,card),10,0xe3f1ff);native_label(383,y+32,u"/",10,0xe3f1ff);native_number(389,y+32,joey_edit_state.owned[card],10,0xe3f1ff);
        if(editor_tab==3&&index==editor_index)native_outline(373,y-2,104,46,0xffe0);}
    if(scene==3){native_label(132,247,u"加入",14,0x6d1f0e);native_label(193,247,u"移出",14,0x6d1f0e);native_label(252,247,editor_changed?u"保存＊":u"保存",14,0x6d1f0e);}
    native_label(319,247,u"返回",14,0x6d1f0e);native_label(379,248,u"上一页",12,0x6d1f0e);native_label(432,248,u"下一页",12,0x6d1f0e);
    if(editor_leaving){native_panel(25,79,430,110);native_label(87,94,u"牌组尚未保存，如何处理？",20,0x35230f);const uint16_t *labels[]={u"保存并离开",u"放弃修改",u"继续编辑"};for(unsigned i=0;i<3;++i){native_button(35+i*140,145,132,labels[i]);if(editor_exit_choice==i)native_outline(33+i*140,143,136,36,0xffe0);}}
}
static void board_draw(void){
    Duel *d=&joey_duel_state;native_backdrop(0);native_life(0,d->players[1].life);native_life(1,d->players[0].life);
    native_label(391,1,d->active?u"对方":u"我方",14,0x211609);native_label(390,0,d->active?u"对方":u"我方",14,d->active?0xffc1b1:0xb7e9ff);
    native_label(427,0,u"回合",14,0xd0d0d0);native_number(458,0,d->turn,14,0xffffff);
    joey_preview_diagnostic[0]=native_selected;joey_preview_diagnostic[1]=joey_preview_diagnostic[2]=-1;joey_preview_diagnostic[3]=native_focus;
    native_card(6,35,native_selected,0);NativeCard *selected=&native_cards[native_selected];uint16_t text[640];unsigned n=0;
    if(native_selected){n=native_string(selected->name,selected->name_length,text,640);native_wrapped(98,37,54,text,n,14,4,0x35230f);}
    if(duel_card_definitions[native_selected].family==DUEL_MONSTER&&native_selected){int attack=selected->attack,defense=selected->defense;
        if(native_focus>=105&&native_focus<115){unsigned row=(native_focus-100)/5,col=(native_focus-100)%5,player=row<2?1:0,index=player?4-col:col;attack=duel_attack_value(d,player,index);defense=duel_defense_value(d,player,index);}
        joey_preview_diagnostic[1]=attack;joey_preview_diagnostic[2]=defense;native_label(98,113,u"攻",12,0x513819);native_number(112,113,attack,12,0x211609);native_label(98,132,u"守",12,0x513819);native_number(112,132,defense,12,0x211609);}
    if(native_selected){n=native_string(selected->description,selected->description_length,text,640);native_wrapped(6,166,146,text,n,12,5,0x35230f);}
    unsigned negated=0,blocked=0,live_lock=0;
    if(native_selected&&duel_card_definitions[native_selected].family==DUEL_TRAP){blocked=duel_card_activation_blocked(d,native_selected);
        if(native_focus>=100&&native_focus<120){unsigned row=(native_focus-100)/5,col=(native_focus-100)%5,side=row<2?1:0,index=side?4-col:col;if(row==0||row==3){DuelSlot *s=&d->players[side].backrow[index];if(s->position!=DUEL_FACEDOWN_DEFENSE)negated=duel_field_effect_negated(d,native_selected);live_lock=s->effect_active&&duel_card_definitions[native_selected].effect==DUEL_E_TRAP_LOCK&&!negated;}}
        if(negated)native_label(98,113,u"效果无效",12,0x84482e);else if(live_lock)native_label(98,113,u"陷阱失效",12,0x513819);
        if(blocked)native_label(98,132,u"不可发动",12,0x84482e);
    }
    joey_suppression_diagnostic[0]=native_selected;joey_suppression_diagnostic[1]=negated;joey_suppression_diagnostic[2]=blocked;joey_suppression_diagnostic[3]=live_lock;
    for(unsigned player=0;player<2;++player){DuelSlot *s=&d->players[player].field;int y=player?40:184;if(s->card){native_label(432,y-15,u"场地",12,0xd0d0d0);native_card(432,y,s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,1);}if(native_focus==(player?120:121))native_outline(431,y-1,38,54,0xffe0);}
    native_outline(154,36+d->phase*34,19,34,0xd5a8);
    for(unsigned row=0;row<4;++row)for(unsigned col=0;col<5;++col){unsigned player=row<2?1:0,index=player?4-col:col,backrow=row==0||row==3;DuelSlot *s=backrow?&d->players[player].backrow[index]:&d->players[player].monsters[index];if(!s->card){if(!backrow&&s->zone_blocked){int bx=209+col*177/5,by=row==1?88:138;native_label(bx+8,by+12,u"X",18,0xc84928);}continue;}
        int x=209+col*177/5;static const unsigned ys[]={39,88,138,186};unsigned quarter=backrow?player?2:0:(s->position!=DUEL_FACEUP_ATTACK&&!(s->position==DUEL_FACEDOWN_DEFENSE&&s->facedown_attack))?1:player?2:0;
        native_field_card(x,ys[row],s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,quarter);
        if(native_focus==(int)(100+row*5+col)||((pending==PENDING_TRIBUTE||(pending==PENDING_MONSTER_TARGET&&duel_monster_effect_target(&duel_card_definitions[d->players[0].monsters[monster_source].card])==2))&&!backrow&&(tribute_mask&(1u<<(player*5+index)))))native_outline(x-(quarter&1?8:1),ys[row]+(quarter&1?6:-1),quarter&1?48:34,quarter&1?34:48,0xffe0);
    }
    unsigned shown=hand_visible(),count=d->players[0].hand_count;if(hand_scroll+shown>count)hand_scroll=count-shown;
    for(unsigned i=0;i<shown;++i){int x=hand_x(i,shown);native_card(x,241,d->players[0].hand[hand_scroll+i],1);if(native_focus==(int)(hand_scroll+i))native_outline(x-1,240,38,54,0xffe0);}
    if(native_focus>=200&&native_focus<206)native_outline(153,35+(native_focus-200)*34,21,36,0xffff);
    if(pending==PENDING_MONSTER_MENU||pending==PENDING_HAND_EFFECT){native_panel(104,45,376,166);native_label(222,49,u"怪兽操作",18,0x35230f);native_button(160,80,270,((pending==PENDING_MONSTER_MENU&&duel_monster_deck_special(&joey_duel_state,0,monster_source))||(pending==PENDING_HAND_EFFECT&&duel_hand_special(&duel_card_definitions[joey_duel_state.players[0].hand[source_hand]])))?u"特殊召唤":u"发动效果");native_button(160,120,270,pending==PENDING_HAND_EFFECT?u"通常召唤":u"改变表示");native_button(160,160,270,u"取消");native_outline(158,78+monster_choice*40,274,36,0xffe0);}
    if(pending==PENDING_BACKUP){native_panel(104,45,376,166);native_label(132,49,u"选一至三只怪兽加入手牌",18,0x35230f);native_number(443,50,backup_count,16,0x35230f);
        for(unsigned i=0;i<5&&grave_page*5+i<picker_count;++i){unsigned choice=grave_page*5+i,card=picker_card(choice);native_card(127+i*73,79,card,1);
            if(picker_selected[choice])native_outline(123+i*73,75,44,60,0x07e0);if(choice==grave_choice)native_outline(125+i*73,77,40,56,0xffe0);NativeCard *c=&native_cards[card];n=native_string(c->name,c->name_length,text,640);native_wrapped(110+i*73,139,70,text,n,14,2,0x35230f);}
        native_button(135,176,140,u"取消");native_button(300,176,140,u"完成选择");if(grave_choice==picker_count)native_outline(298,174,144,36,0xffe0);
    }
    if(pending==PENDING_RESPONSE){native_panel(104,45,376,166);native_label(113,50,u"连锁",14,0x35230f);native_number(145,50,d->chain_count,14,0x35230f);native_label(210,49,u"选择发动的卡",18,0x35230f);
        for(unsigned i=0;i<5&&grave_page*5+i<response_count;++i){unsigned choice=grave_page*5+i,card=response_options[choice].card;native_card(127+i*73,79,card,1);if(choice==grave_choice)native_outline(125+i*73,77,40,56,0xffe0);NativeCard *c=&native_cards[card];n=native_string(c->name,c->name_length,text,640);native_wrapped(110+i*73,139,70,text,n,14,2,0x35230f);}
        native_button(135,176,140,u"不发动");native_button(300,176,140,u"发动");native_outline(grave_choice==response_count?133:298,174,144,36,0xffe0);
    }
    if(pending==PENDING_GRAVE||pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND){native_panel(104,45,376,166);unsigned kind=d->choice.kind;
        const uint16_t *title=pending==PENDING_PUBLIC_HAND?u"对方公开手牌":pending==PENDING_ATTACK_SELECT?u"选择攻击对象":pending!=PENDING_EFFECT_CHOICE?u"选择对象":(kind==DUEL_CHOICE_GROUND_FIRST||kind==DUEL_CHOICE_GROUND_SECOND)?u"选择封锁的怪兽区域":kind==DUEL_CHOICE_MYSTIC_DESTROY?u"选择对方破坏目标":kind==DUEL_CHOICE_MYSTIC_GIVE?u"选择交给对方的怪兽":kind==DUEL_CHOICE_CRUSH_TRIBUTE?u"选择解放的暗属性怪兽":kind==DUEL_CHOICE_GAMBLE_COIN?u"猜硬币正反面":kind==DUEL_CHOICE_BORROW_TARGET?u"选择加入手牌的卡":kind==DUEL_CHOICE_HATS_TARGET?u"选择盖放的怪兽":kind==DUEL_CHOICE_HATS_DECK?u"选择魔法或陷阱":kind==DUEL_CHOICE_BAIT_TARGET?u"选择强制发动的对象":kind==DUEL_CHOICE_BAIT_BACKUP?u"选择回收的墓地卡":kind==DUEL_CHOICE_MAJOR_RIOT?u"选择里侧守备召唤的怪兽":kind==DUEL_CHOICE_CHOSEN_MONSTER?u"选择特殊召唤的怪兽":kind==DUEL_CHOICE_CHOSEN_NONMONSTER?u"选择魔法或陷阱":kind==DUEL_CHOICE_CHOSEN_BLIND?u"选择一张盖放卡":(kind==DUEL_CHOICE_CHOSEN_POSITION||kind==DUEL_CHOICE_CYBER_POSITION)?u"选择召唤表示":kind==DUEL_CHOICE_DRILL_OFFER?u"是否发动效果":kind==DUEL_CHOICE_YADO_HAND?u"选择放回卡组底的手牌":kind==DUEL_CHOICE_EXCHANGE_OTHER||kind==DUEL_CHOICE_EXCHANGE_OWN?u"选择交换的对方手牌":kind==DUEL_CHOICE_PAINFUL_DECK?u"选择五张卡展示给对方":kind==DUEL_CHOICE_PAINFUL_KEEP?u"选择对方加入手牌的卡":kind==DUEL_CHOICE_INSPECTION_OFFER?u"是否支付500查看手牌":kind==DUEL_CHOICE_PATROL_TARGET?u"选择查看的盖放卡":kind==DUEL_CHOICE_DESTRUCTION_SELECT?(duel_card_definitions[d->choice.card].name_group==1059?u"选择支付的祭品":d->equip_mode_command.target_count<2?u"选择己方破坏目标":u"选择对方破坏目标"):kind==DUEL_CHOICE_SHARE_TRIBUTE?u"选择解放的祭品":kind==DUEL_CHOICE_EARTH_FIRST?u"选择第一个属性":kind==DUEL_CHOICE_EARTH_SECOND?u"选择第二个属性":kind==DUEL_CHOICE_EARTH_OPPONENT?u"选择破坏的属性":kind==DUEL_CHOICE_LAIR_OFFER?u"是否除外发动效果":kind==DUEL_CHOICE_STAT_SOURCE?u"选择减半攻击力的怪兽":kind==DUEL_CHOICE_STAT_DESTINATION?u"选择增加攻击力的怪兽":kind==DUEL_CHOICE_RETARGET_MODE?u"选择转移效果":kind==DUEL_CHOICE_RETARGET_MONSTER?u"选择新的效果对象":kind==DUEL_CHOICE_BATTLE_REDIRECT?u"选择新的攻击目标":kind==DUEL_CHOICE_BATTLE_REVIVE?u"是否复活战斗破坏的怪兽":kind==DUEL_CHOICE_BATTLE_REVIVE_POSITION?u"选择复活表示":kind==DUEL_CHOICE_OFFERING_PAY?u"是否支付500发动":kind==DUEL_CHOICE_OFFERING_HAND?u"选择通常召唤的怪兽":kind==DUEL_CHOICE_OFFERING_FORM?u"选择召唤或盖放":kind==DUEL_CHOICE_OFFERING_TRIBUTE?u"请选择效果的祭品":kind==DUEL_CHOICE_OFFERING_ZONE?u"选择怪兽区域":kind==DUEL_CHOICE_DEFUSION?u"是否召唤融合素材":kind==DUEL_CHOICE_ATTACK_TRIBUTE_COST?u"请选择攻击的祭品":kind==DUEL_CHOICE_ATTACK_TRIBUTE_TARGET?u"选择攻击对象":kind==DUEL_CHOICE_SUMMON_PROCEDURE_COST?u"请选择效果的祭品":kind==DUEL_CHOICE_KUNAI_MODE?u"选择发动效果":kind==DUEL_CHOICE_KUNAI_TARGET?u"选择装备对象":kind==DUEL_CHOICE_EQUIP_TRANSFER_SOURCE?u"选择装备卡":kind==DUEL_CHOICE_EQUIP_TRANSFER_TARGET?u"选择装备对象":kind==DUEL_CHOICE_REVIVAL_JAM?u"支付1000下回合复活":kind==DUEL_CHOICE_GRAVE_PAIR?u"选择复活的怪兽":kind==DUEL_CHOICE_GRAVE_PAIR_POSITION?u"选择复活表示":kind==DUEL_CHOICE_HISTORY_OFFER?(duel_card_definitions[d->choice.card].name_group==1054?u"是否发动遗言":u"是否发动回合末回收"):kind==DUEL_CHOICE_HISTORY_TARGET?(duel_card_definitions[d->choice.card].name_group==1444||duel_card_definitions[d->choice.card].name_group==1524||duel_card_definitions[d->choice.card].name_group==1421?u"选择加入手牌的卡":duel_card_definitions[d->choice.card].name_group==1430?u"选择放到卡组上面的卡":u"选择放到卡组底的卡"):kind==DUEL_CHOICE_TRIBUTE_UPKEEP?u"选择费用怪兽":kind==DUEL_CHOICE_OPTIONAL_SET?u"选择盖放的卡":kind==DUEL_CHOICE_ACTIVATION_PARAMETER?(duel_card_definitions[d->choice.card].name_group==1162?u"宣言禁止的卡名":duel_card_definitions[d->choice.card].name_group==1448?u"选择属性":u"选择种族"):kind==DUEL_CHOICE_EQUIP_STAT?u"选择提升攻击力或守备力":kind==DUEL_CHOICE_DARK_SAGE_OFFER?u"是否召唤黑衣大贤者":kind==DUEL_CHOICE_DARK_SAGE_TRIBUTE?u"选择黑魔导作为祭品":kind==DUEL_CHOICE_DARK_SAGE_SPELL?u"选择加入手牌的魔法卡":(kind==DUEL_CHOICE_ATTACK_COIN||kind==DUEL_CHOICE_FAIRY_BOX_COIN||kind==DUEL_CHOICE_GAMBLE_COIN)?u"猜硬币正反面":kind==DUEL_CHOICE_PILE_SUMMON?u"选择特殊召唤的怪兽":kind==DUEL_CHOICE_PILE_SUMMON_POSITION?u"选择特殊召唤表示":kind==DUEL_CHOICE_FLIP_OFFER?u"是否支付2000发动":kind==DUEL_CHOICE_PEEK_VIEW?u"确认查看的卡片":kind==DUEL_CHOICE_PEEK_HAND?u"选择对方的一张手牌":kind==DUEL_CHOICE_PEEK_ORDER?u"先选择最先抽到的卡":kind==DUEL_CHOICE_LAIR_TARGET?u"选择等级相同的怪兽":kind==DUEL_CHOICE_PILE_PLAYER?u"选择卡片所在墓地":kind==DUEL_CHOICE_PILE_CARD?(d->pile_task.op==40?u"选择回到墓地的怪兽":u"选择除外的卡"):kind==DUEL_CHOICE_SPIRIT_POSITION?u"选择改变表示的怪兽":kind==DUEL_CHOICE_SPECIAL_HAND_COST?u"选择特殊召唤费用":kind==DUEL_CHOICE_SPECIAL_HAND_POSITION?u"选择特殊召唤表示":kind==DUEL_CHOICE_SIMPLE_DESTROY?u"选择破坏的怪兽":kind==DUEL_CHOICE_IGNITION_FUSION?u"选择融合卡组的怪兽":kind==DUEL_CHOICE_IGNITION_POSITION?u"选择召唤表示":(kind==DUEL_CHOICE_UPKEEP||kind==DUEL_CHOICE_MUSHROOM)?u"是否支付费用":kind==DUEL_CHOICE_MAGIC_DRAIN?u"选择弃掉的魔法卡":kind==DUEL_CHOICE_PERIODIC?u"是否回收墓地卡":kind==DUEL_CHOICE_EQUIP_RECYCLE?u"选择回收装备卡":kind==DUEL_CHOICE_HAND?(d->hand_task.cost?u"选择费用手牌":d->hand_task.op==2?u"选择返回卡组的卡":u"选择舍弃的卡"):kind==DUEL_CHOICE_SEARCH_OFFER?u"是否发动效果":kind==DUEL_CHOICE_TARGET_FLIP?u"选择效果对象":kind==DUEL_CHOICE_DISCARD_HAND?u"请选择弃牌":kind==DUEL_CHOICE_RITUAL_MONSTER?u"选择仪式怪兽":kind==DUEL_CHOICE_RITUAL_MATERIAL?u"选择仪式祭品":kind==DUEL_CHOICE_RITUAL_POSITION?u"选择召唤表示":kind==DUEL_CHOICE_FUSION_MONSTER?u"选择融合怪兽":kind==DUEL_CHOICE_FUSION_MATERIAL?u"选择融合素材":kind==DUEL_CHOICE_FUSION_POSITION?u"选择召唤表示":kind==DUEL_CHOICE_ATTACK_REPLAY?u"重新选择攻击对象":kind==DUEL_CHOICE_FLIP_TARGET&&duel_card_definitions[d->choice.card].effect==DUEL_E_MAN_EATER_BUG?u"选择破坏的怪兽":kind==DUEL_CHOICE_PENGUIN||kind==DUEL_CHOICE_FLIP_TARGET?u"选择返回手牌的怪兽":kind==DUEL_CHOICE_GRAVE_RECALL?u"选择回收的墓地卡":(kind==DUEL_CHOICE_NIMBLE_SUMMON||kind==DUEL_CHOICE_RECRUIT)?u"选择特殊召唤的怪兽":kind==DUEL_CHOICE_OPTIONAL_DRAW?u"是否抽一张牌":u"选择加入手牌的卡";
        native_label(190,49,title,18,0x35230f);
        for(unsigned i=0;i<5&&grave_page*5+i<picker_count;++i){unsigned choice=grave_page*5+i,card=picker_card(choice);
            if((pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND)&&effect_options[choice].kind==DUEL_OPTION_SKIP){native_button(110+i*73,91,70,pending==PENDING_PUBLIC_HAND?u"返回":(kind==DUEL_CHOICE_EQUIP_STAT||kind==DUEL_CHOICE_ACTIVATION_PARAMETER||kind==DUEL_CHOICE_DESTRUCTION_SELECT)?u"取消发动":kind==DUEL_CHOICE_PEEK_VIEW?u"确认":kind==DUEL_CHOICE_PEEK_ORDER?u"保持原顺序":kind==DUEL_CHOICE_PILE_PLAYER?u"完成":kind==DUEL_CHOICE_PILE_CARD?(d->pile_task.op==38?u"返回":u"完成"):(kind==DUEL_CHOICE_ATTACK_REPLAY||kind==DUEL_CHOICE_ATTACK_COIN||pending==PENDING_ATTACK_SELECT)?u"取消攻击":kind==DUEL_CHOICE_OPTIONAL_DRAW?u"不抽":(kind==DUEL_CHOICE_HISTORY_OFFER||kind==DUEL_CHOICE_SEARCH_OFFER||kind==DUEL_CHOICE_FLIP_OFFER||kind==DUEL_CHOICE_DARK_SAGE_OFFER)?u"不发动":(kind==DUEL_CHOICE_EQUIP_RECYCLE||kind==DUEL_CHOICE_PERIODIC)?u"不回收":(kind==DUEL_CHOICE_REVIVAL_JAM||kind==DUEL_CHOICE_UPKEEP||kind==DUEL_CHOICE_MUSHROOM||kind==DUEL_CHOICE_TRIBUTE_UPKEEP)?u"不支付":kind==DUEL_CHOICE_INSPECTION_OFFER?u"不发动":kind==DUEL_CHOICE_PATROL_TARGET?u"取消发动":kind==DUEL_CHOICE_LAIR_OFFER?u"只发动此卡":kind==DUEL_CHOICE_MAGIC_DRAIN?u"不弃牌":kind==DUEL_CHOICE_PILE_SUMMON&&joey_duel_state.pile_summon_task.kind==6?u"不发动":u"结束");if(choice==grave_choice)native_outline(108+i*73,89,74,36,0xffe0);continue;}
            if(pending==PENDING_EFFECT_CHOICE&&effect_options[choice].kind==DUEL_OPTION_PAGE){native_button(110+i*73,91,70,effect_options[choice].index==254?u"下一组":u"上一组");if(choice==grave_choice)native_outline(108+i*73,89,74,36,0xffe0);continue;}
            if(pending==PENDING_EFFECT_CHOICE&&kind==DUEL_CHOICE_PILE_PLAYER){native_button(110+i*73,91,70,effect_options[choice].index?u"对方墓地":u"我方墓地");if(choice==grave_choice)native_outline(108+i*73,89,74,36,0xffe0);continue;}
            if(pending==PENDING_EFFECT_CHOICE&&kind==DUEL_CHOICE_PATROL_TARGET){unsigned index=effect_options[choice].index;native_card(127+i*73,79,0,1);if(choice==grave_choice)native_outline(125+i*73,77,40,56,0xffe0);native_label(110+i*73,139,index<5?u"怪兽区":index<10?u"魔陷区":u"场地区",14,0x35230f);if(index<10)native_number(128+i*73,162,5-index%5,14,0x35230f);continue;}
            native_card(127+i*73,79,card,1);if(choice==grave_choice)native_outline(125+i*73,77,40,56,0xffe0);
            if((pending==PENDING_EFFECT_CHOICE||pending==PENDING_ATTACK_SELECT||pending==PENDING_PUBLIC_HAND)&&effect_options[choice].kind==DUEL_OPTION_CONFIRM)native_label(110+i*73,139,(kind==DUEL_CHOICE_GROUND_FIRST||kind==DUEL_CHOICE_GROUND_SECOND)?(effect_options[choice].index/5?u"对方区域":u"我方区域"):(kind==DUEL_CHOICE_CHOSEN_POSITION||kind==DUEL_CHOICE_CYBER_POSITION)?(effect_options[choice].index?u"盖放守备":u"攻击表示"):kind==DUEL_CHOICE_BAIT_TARGET?u"强制发动":kind==DUEL_CHOICE_BAIT_BACKUP?u"完成":kind==DUEL_CHOICE_DRILL_OFFER?u"发动效果":kind==DUEL_CHOICE_INSPECTION_OFFER?u"支付500":kind==DUEL_CHOICE_LAIR_OFFER?u"除外发动":kind==DUEL_CHOICE_RETARGET_MODE?(effect_options[choice].index==1?u"转移卡片目标":u"转移攻击目标"):kind==DUEL_CHOICE_OFFERING_PAY?u"支付500":kind==DUEL_CHOICE_OFFERING_FORM?(effect_options[choice].index?u"盖放守备":u"通常召唤"):kind==DUEL_CHOICE_OFFERING_ZONE?u"怪兽区域":kind==DUEL_CHOICE_KUNAI_MODE?(effect_options[choice].index==1?u"攻击者转守备":effect_options[choice].index==2?u"装备加500攻":u"两项都发动"):(kind==DUEL_CHOICE_EARTH_FIRST||kind==DUEL_CHOICE_EARTH_SECOND||kind==DUEL_CHOICE_EARTH_OPPONENT||kind==DUEL_CHOICE_ACTIVATION_PARAMETER)?native_trait_label(d->choice.card,effect_options[choice].index):kind==DUEL_CHOICE_EQUIP_STAT?(effect_options[choice].index?u"守备力+700":u"攻击力+700"):(kind==DUEL_CHOICE_ATTACK_COIN||kind==DUEL_CHOICE_FAIRY_BOX_COIN||kind==DUEL_CHOICE_GAMBLE_COIN)?(effect_options[choice].index?u"反面":u"正面"):kind==DUEL_CHOICE_PILE_PLAYER?(effect_options[choice].index?u"对方墓地":u"我方墓地"):(kind==DUEL_CHOICE_BATTLE_REVIVE_POSITION||kind==DUEL_CHOICE_FUSION_POSITION||kind==DUEL_CHOICE_RITUAL_POSITION||kind==DUEL_CHOICE_IGNITION_POSITION||kind==DUEL_CHOICE_SPECIAL_HAND_POSITION||kind==DUEL_CHOICE_PILE_SUMMON_POSITION||kind==DUEL_CHOICE_GRAVE_PAIR_POSITION)?(effect_options[choice].index==DUEL_FACEDOWN_DEFENSE?u"盖放守备":effect_options[choice].index?u"守备表示":u"攻击表示"):(kind==DUEL_CHOICE_ATTACK_REPLAY||kind==DUEL_CHOICE_ATTACK_TRIBUTE_TARGET||pending==PENDING_ATTACK_SELECT)?u"直接攻击":(kind==DUEL_CHOICE_HISTORY_OFFER||kind==DUEL_CHOICE_SEARCH_OFFER||kind==DUEL_CHOICE_FLIP_OFFER||kind==DUEL_CHOICE_DARK_SAGE_OFFER)?u"发动效果":(kind==DUEL_CHOICE_EQUIP_RECYCLE||kind==DUEL_CHOICE_MUSHROOM)?u"支付500":kind==DUEL_CHOICE_UPKEEP?(duel_card_definitions[d->choice.card].name_group==1424?u"支付500":duel_card_definitions[d->choice.card].name_group==1131?u"支付100":duel_card_definitions[d->choice.card].name_group==1154?u"支付700":duel_card_definitions[d->choice.card].name_group==1099?u"支付2000":u"支付1000"):kind==DUEL_CHOICE_REVIVAL_JAM?u"支付1000":(kind==DUEL_CHOICE_DEFUSION||kind==DUEL_CHOICE_BATTLE_REVIVE)?u"特殊召唤":kind==DUEL_CHOICE_PERIODIC?u"加入手牌":u"抽一张牌",14,0x35230f);
            else {NativeCard *c=&native_cards[card];n=native_string(c->name,c->name_length,text,640);native_wrapped(110+i*73,139,70,text,n,14,2,0x35230f);}
            if(pending==PENDING_EFFECT_CHOICE&&(kind==DUEL_CHOICE_TRIBUTE_UPKEEP||kind==DUEL_CHOICE_RITUAL_MATERIAL||(kind==DUEL_CHOICE_SPECIAL_HAND_COST&&(duel_hand_special_cost_is_field(&duel_card_definitions[d->choice.card])||duel_spirit_active(d,d->choice.player))))){unsigned token=effect_options[choice].index,hand=(kind==DUEL_CHOICE_RITUAL_MATERIAL||(kind==DUEL_CHOICE_SPECIAL_HAND_COST&&duel_hand_special_cost_is_field(&duel_card_definitions[d->choice.card])==2))&&token<DUEL_HAND_MAX;if((kind==DUEL_CHOICE_RITUAL_MATERIAL||(kind==DUEL_CHOICE_SPECIAL_HAND_COST&&duel_hand_special_cost_is_field(&duel_card_definitions[d->choice.card])==2))&&!hand)token-=DUEL_HAND_MAX;native_label(110+i*73,178,hand?u"手牌":token<5?u"我方":u"对方",14,0x35230f);if(!hand)native_number(145+i*73,178,token%5+1,14,0x35230f);}
            if(pending==PENDING_EFFECT_CHOICE&&kind==DUEL_CHOICE_TARGET_FLIP){unsigned token=effect_options[choice].index,index=token%11;native_label(110+i*73,173,token/11?u"对方":u"我方",14,0x35230f);native_label(110+i*73,188,index<5?u"怪兽":index<10?u"魔陷":u"场地",14,0x35230f);if(index<10)native_number(145+i*73,188,token/11?5-index%5:index%5+1,14,0x35230f);}
            if(pending==PENDING_EFFECT_CHOICE&&(kind==DUEL_CHOICE_PENGUIN||kind==DUEL_CHOICE_FLIP_TARGET)){unsigned index=effect_options[choice].index;native_label(110+i*73,180,index/5?u"对方":u"我方",14,0x35230f);native_number(140+i*73,180,index/5?5-index%5:index%5+1,14,0x35230f);}
            if((pending==PENDING_ATTACK_SELECT||pending==PENDING_EFFECT_CHOICE&&kind==DUEL_CHOICE_ATTACK_REPLAY)&&effect_options[choice].kind==DUEL_OPTION_CARD){native_label(110+i*73,180,u"对方",14,0x35230f);native_number(140+i*73,180,5-effect_options[choice].index,14,0x35230f);}
            if(pending==PENDING_EFFECT_CHOICE&&kind==DUEL_CHOICE_OFFERING_ZONE)native_number(135+i*73,180,effect_options[choice].index+1,14,0x35230f);
            if(pending==PENDING_EFFECT_CHOICE&&kind==DUEL_CHOICE_FUSION_MATERIAL){unsigned token=effect_options[choice].index;native_label(110+i*73,180,token<DUEL_HAND_MAX?u"手牌":u"我方",14,0x35230f);native_number(140+i*73,180,token<DUEL_HAND_MAX?token+1:token-DUEL_HAND_MAX+1,14,0x35230f);}}}
    if(pending==PENDING_PUBLIC_HAND){native_button(108,176,72,u"上一页");native_button(207,176,170,u"返回");native_button(400,176,72,u"下一页");}
    if(pending==PENDING_REMODEL){DuelRemodel *r=&d->remodel;native_panel(104,40,376,205);native_label(194,43,u"混沌壶翻牌结果",18,0x35230f);native_label(110,66,u"我方回组",14,0x35230f);native_number(170,66,r->returned[0],14,0x35230f);native_label(260,66,u"对方回组",14,0x35230f);native_number(320,66,r->returned[1],14,0x35230f);native_label(202,66,u"融",14,0x35230f);native_number(218,66,r->returned_fusion[0],14,0x35230f);native_label(352,66,u"融",14,0x35230f);native_number(368,66,r->returned_fusion[1],14,0x35230f);
        for(unsigned i=0;i<5&&grave_page*5+i<picker_count;++i){unsigned choice=grave_page*5+i,side=picker_owner[choice],index=picker_index[choice],card=picker_card(choice);native_card(127+i*73,86,card,1);if(choice==grave_choice)native_outline(125+i*73,84,40,56,0xffe0);NativeCard *c=&native_cards[card];n=native_string(c->name,c->name_length,text,640);native_wrapped(110+i*73,141,70,text,n,14,2,0x35230f);native_label(110+i*73,177,side?u"对方":u"我方",14,0x35230f);native_label(110+i*73,192,r->deployed[side][index]?u"盖放召唤":u"送入墓地",14,0x35230f);}
        native_button(108,209,72,u"上一页");native_button(207,209,170,u"继续对局");native_button(400,209,72,u"下一页");if(grave_choice==picker_count)native_outline(205,207,174,36,0xffe0);
    }
    if(pending==PENDING_POSITION){native_panel(104,45,376,166);native_label(195,50,u"选择表示形式",18,0x35230f);unsigned card=d->players[picker_owner[grave_choice]].grave[picker_index[grave_choice]];native_card(178,83,card,1);native_card_rotated(330,91,card,1);native_button(145,153,125,u"攻击表示");native_button(300,153,125,u"守备表示");native_outline(grave_position?298:143,151,129,36,0xffe0);}
    if(pending==PENDING_COIN_GUESS){native_panel(132,82,340,116);native_label(205,94,u"猜硬币正反面",18,0x35230f);native_button(145,147,145,u"正面");native_button(310,147,145,u"反面");native_outline(coin_guess?308:143,145,149,36,0xffe0);}
    if(pending==PENDING_LIFE_TARGET){native_panel(132,82,340,116);native_label(190,94,u"请选择回复对象",18,0x35230f);native_button(145,147,145,u"我方");native_button(310,147,145,u"对方");native_outline(life_choice?308:143,145,149,36,0xffe0);}
    ai_draw();
    if(d->discard_required){native_panel(122,95,284,54);native_label(138,110,u"手牌超限，请选择弃牌",18,0x35230f);}
    if(d->winner>=0){native_panel(164,93,222,65);native_label(245,105,d->winner==0?u"胜利":u"败北",24,0x35230f);}
}
static void opening_draw(void){
    static const uint16_t *hands[]={u"石头",u"剪刀",u"布"};DuelOpening *o=&opening_state;
    native_backdrop(2);native_label(135,15,u"猜拳决定先后攻",22,0xffe0b1);
    if(o->phase==DUEL_OPEN_CHOOSE){native_label(145,73,u"请选择手势",18,0xffffff);native_outline(31+opening_hand_focus*161,137,96,94,0xffe0);}
    else {native_label(70,50,u"我方",18,0xffffff);if(o->human<3)native_label(116,50,hands[o->human],18,0xffe0b1);native_label(252,50,u"城之内",18,0xffffff);if(o->enemy<3)native_label(316,50,hands[o->enemy],18,0xffe0b1);
        if(o->phase==DUEL_OPEN_DRAW){native_label(140,76,u"平局，请再猜一次",18,0xffffff);native_button(150,103,181,u"再猜一次");}
        else if(o->phase==DUEL_OPEN_PLAYER_ORDER){native_label(165,76,u"你获胜，请选择",18,0xffffff);native_button(82,103,140,u"先攻");native_button(258,103,140,u"后攻");native_outline(opening_order_focus?256:80,101,144,36,0xffe0);}
        else if(o->phase==DUEL_OPEN_NPC_ORDER){native_label(140,76,o->npc_first?u"城之内选择先攻":u"城之内选择后攻",18,0xffffff);native_button(150,103,181,u"开始决斗");}
    }native_label(60,239,hands[0],18,0xffffff);native_label(220,239,hands[1],18,0xffffff);native_label(383,239,hands[2],18,0xffffff);
}
static void redraw(void){
    uint32_t started=h1_timer_read_ms();if(details_open)details_draw();else if(scene==7)opening_draw();else if(scene==5)result_draw();else if(scene==6)records_draw();else if(scene==3||scene==4)editor_draw();else if(scene==0)scene_menu();else board_draw();
    if(about_open)scene_about();
    if(!about_open&&notice&&(int32_t)(notice_until-h1_timer_read_ms())>0){unsigned y=scene==3||scene==4?110:scene==5?184:214;native_panel(134,y,346,28);native_label(140,y+5,notice,16,0x35230f);}
    h1_blit_rgb565(0,0,480,272,screen);h1_present_full_screen();++joey_native_diagnostic[2];joey_native_diagnostic[17]=h1_timer_read_ms()-started;dirty=0;
}
static void back(void){
    joey_audio_effect(JOEY_SOUND_RETURN);
    if(about_open){about_open=0;dirty=1;return;}
    if(details_open){details_open=0;dirty=1;}else if(scene==7){++opening_cancels;close_game();}else if(scene==5)result_activate(0);else if(scene==3||scene==4){if(editor_leaving){editor_leaving=0;dirty=1;}else editor_leave(0);}else if(pending==PENDING_RESPONSE)pass_response();else if(pending)cancel_picker();else if(scene)close_game();else exit_requested=1;
}
static void request_exit(void){if(scene==3&&editor_changed)editor_leave(1);else exit_requested=1;}
static int can_show_details(void){return scene==1||scene==3||scene==4||(scene==5&&result_card);}
static void poll_keyboard(void){
    if(!native_keyboard)return;uint32_t tick=h1_raw_tick_80hz();if(key_tick_valid&&tick==key_tick)return;key_tick_valid=1;key_tick=tick;
    static const unsigned scans[]={42,1,57,28,103,108,105,106};unsigned mask=0;
    for(unsigned i=0;i<8;++i)mask|=key_held(scans[i])<<i;
    unsigned pressed=mask&~held_mask;held_mask=mask;keys[H1_KEY_SHIFT]=mask&1;
    joey_native_diagnostic[20]=mask;++joey_native_diagnostic[21];
    if((mask&3)==3){if(pressed&3)request_exit();return;}
    if(pressed&2)back();
    if((pressed&4)&&can_show_details()&&!editor_leaving){details_open=!details_open;detail_scroll=0;dirty=1;}
    if(pressed&8)activate(mask&1);
    static const unsigned navigation[]={H1_KEY_UP,H1_KEY_DOWN,H1_KEY_LEFT,H1_KEY_RIGHT};
    for(unsigned i=0;i<4;++i)if(pressed&(16u<<i))move(navigation[i]);
}
static void diagnostic(void){
    joey_ai_diagnostic[0]=ai_sequence;joey_ai_diagnostic[1]=ai_stage;joey_ai_diagnostic[2]=ai_command.action;joey_ai_diagnostic[3]=ai_card;joey_ai_diagnostic[4]=ai_elapsed;joey_ai_diagnostic[5]=ai_x0;joey_ai_diagnostic[6]=ai_y0;joey_ai_diagnostic[7]=ai_x1;joey_ai_diagnostic[8]=ai_y1;joey_ai_diagnostic[9]=ai_motion;joey_ai_diagnostic[10]=ai_command.source;joey_ai_diagnostic[11]=ai_command.target;
    Duel *d=&joey_duel_state;joey_native_diagnostic[0]=0x4a4e4154u;joey_native_diagnostic[1]=details_open?4:scene==0?2:scene==7?10:scene==3?6:scene==4?7:scene==6?8:scene==5||d->winner>=0?5:3;joey_native_diagnostic[3]=h1_timer_read_ms();
    joey_native_diagnostic[6]=d->players[0].life;joey_native_diagnostic[7]=d->players[1].life;joey_native_diagnostic[8]=d->turn;joey_native_diagnostic[9]=d->active;joey_native_diagnostic[10]=d->phase;joey_native_diagnostic[11]=d->players[0].hand_count;joey_native_diagnostic[12]=native_selected;joey_native_diagnostic[13]=native_focus;joey_native_diagnostic[14]=pending;joey_native_diagnostic[15]=d->revision;joey_native_diagnostic[16]=sizeof(Duel);joey_native_diagnostic[19]=exit_requested;
    joey_native_diagnostic[22]=d->response_player;joey_native_diagnostic[23]=d->chain_count;
    joey_profile_diagnostic[0]=joey_profile_state.sequence;joey_profile_diagnostic[1]=profile_slot;joey_profile_diagnostic[2]=profile_valid_mask;joey_profile_diagnostic[3]=profile_save_result;joey_profile_diagnostic[4]=scene;joey_profile_diagnostic[5]=editor_tab;joey_profile_diagnostic[6]=editor_count;joey_profile_diagnostic[7]=editor_index;joey_profile_diagnostic[8]=editor_changed;joey_profile_diagnostic[9]=editor_leaving;joey_profile_diagnostic[10]=joey_edit_state.main_count;joey_profile_diagnostic[11]=joey_edit_state.fusion_count;
    joey_editor_diagnostic[0]=editor_destination;joey_editor_diagnostic[1]=joey_edit_state.side_count;for(unsigned i=0;i<4;++i)joey_editor_diagnostic[2+i]=editor_cursor[i];joey_editor_diagnostic[6]=joey_profile_state.side_count;joey_editor_diagnostic[7]=4;
    joey_result_diagnostic[0]=result_winner;joey_result_diagnostic[1]=result_action;joey_result_diagnostic[2]=result_error;joey_result_diagnostic[3]=result_card;joey_result_diagnostic[4]=result_choice;joey_result_diagnostic[5]=result_claimed;joey_result_diagnostic[6]=reward_random_state;joey_result_diagnostic[7]=result_new;
    joey_opening_diagnostic[0]=opening_state.phase;joey_opening_diagnostic[1]=opening_hand_focus;joey_opening_diagnostic[2]=opening_order_focus;joey_opening_diagnostic[3]=opening_state.human;joey_opening_diagnostic[4]=opening_state.enemy;joey_opening_diagnostic[5]=opening_state.outcome;joey_opening_diagnostic[6]=opening_state.first;joey_opening_diagnostic[7]=opening_state.rounds;joey_opening_diagnostic[8]=opening_starts;joey_opening_diagnostic[9]=opening_cancels;joey_opening_diagnostic[10]=opening_state.phase==DUEL_OPEN_NPC_ORDER?opening_state.npc_first:255;joey_opening_diagnostic[11]=scene==7;
}
static void audio_scene(void){
    unsigned music=JOEY_SOUND_M_MENU;
    if(scene==7)music=JOEY_SOUND_M_JYANK;
    else if(scene==3||scene==4)music=JOEY_SOUND_M_KABAN;
    else if(scene==1){static const unsigned tracks[]={JOEY_SOUND_M_DUEL2,JOEY_SOUND_M_DUEL3,JOEY_SOUND_M_DUEL1};music=tracks[opening_starts%3];}
    else if(scene==5)music=result_winner==2?JOEY_SOUND_M_MENU:result_winner?JOEY_SOUND_M_LOSE:JOEY_SOUND_M_WIN;
    joey_audio_music(music);joey_audio_tick();
}
int h1_app_main(void){
    h1_timer_start();joey_native_diagnostic[0]=0x4a4e4154u;joey_native_diagnostic[1]=1;
    const uint32_t *query=h1_runtime_entry(h1_runtime_table(H1_RUNTIME_GUI_TABLE_SLOT),0x9d8u);
    native_keyboard=query&&query[0]==0x27bdffe8u&&query[1]==0xafbf0010u&&query[2]==0x0c00208du&&!query[3];
    if(!native_load()){h1_timer_stop();h1_message_box(0,"Missing H1UI/H1ART assets","Joey",0);return 1;}
    if(!font_assets_valid()){h1_timer_stop();h1_message_box(0,"Please update yugioh/FONT.BIN","Joey",0);return 1;}
    if(!scene_load()){h1_timer_stop();h1_message_box(0,"Please update yugioh/H1SCN.PAK","Joey",0);return 1;}
    profile_load();joey_audio_open();
    if(joey_profile_state.reward_flags&JOEY_REWARD_PENDING)result_resume();
    while(!exit_requested){
        audio_scene();
        /* Native held-state edges also cover keys consumed by firmware GUI. */
        poll_keyboard();
        unsigned events=0;for(;events<64&&!exit_requested;++events){int code=-1,value=-1;h1_event_fetch(&code,&value);if(code==-1&&value==-1)break;
            if(native_keyboard&&(code==H1_EVENT_KEY_DOWN||code==H1_EVENT_KEY_UP))continue;
            if(code==H1_EVENT_KEY_UP&&value>=1&&value<=44)keys[value]=0;
            if(code==H1_EVENT_KEY_DOWN&&value>=1&&value<=44){if(keys[value])continue;keys[value]=1;
                if(value==H1_KEY_BACK&&shift_down()){request_exit();break;}
                if(value==H1_KEY_BACK||value==H1_KEY_ESCAPE)back();
                else if(value==H1_KEY_SPACE&&can_show_details()&&!editor_leaving){details_open=!details_open;detail_scroll=0;dirty=1;}
                else if(value==H1_KEY_ENTER||value==H1_KEY_CONFIRM)activate(shift_down());
                else if(value==H1_KEY_LEFT||value==H1_KEY_RIGHT||value==H1_KEY_UP||value==H1_KEY_DOWN)move(value);
            }
            if(code==8){
                ++joey_pen_diagnostic[2];
                if(pen_down){int x,y;if(pen(&x,&y)){pen_x=x;pen_y=y;pen_valid=1;}
                    if(pen_valid){++joey_pen_diagnostic[4];joey_pen_diagnostic[5]=pen_x;joey_pen_diagnostic[6]=pen_y;joey_pen_diagnostic[7]=h1_raw_tick_80hz();touch(pen_x,pen_y,pen_secondary);}}
                pen_down=pen_valid=0;
            }
            if(code==12)++joey_pen_diagnostic[3];
            if(code==11){++joey_pen_diagnostic[0];if(pen_down){++joey_pen_diagnostic[1];continue;}pen_down=1;pen_valid=0;pen_secondary=shift_down();}
        }
        /* The first ADC sample can still lie between two nearby taps. Use the
           settled contact coordinates and dispatch exactly once on release. */
        if(pen_down){int x,y;if(pen(&x,&y)){pen_x=x;pen_y=y;pen_valid=1;}}
        uint32_t now=h1_timer_read_ms();
        ai_tick(now);
        if(scene==1&&!details_open&&!ai_stage&&pending!=PENDING_REMODEL&&pending!=PENDING_PUBLIC_HAND&&joey_duel_state.winner<0&&(int32_t)(now-next_ai)>=0){
            if(joey_duel_state.response_player<2){if(joey_duel_state.response_player==1){DuelCommand c;if(duel_ai_decide(&joey_duel_state,1,&c))ai_queue(&c);}}
            else if(joey_duel_state.choice.kind){if(joey_duel_state.choice.player==1){DuelCommand c;if(duel_ai_decide(&joey_duel_state,1,&c))ai_queue(&c);}}
            else if(joey_duel_state.active==1){DuelCommand c;if(duel_ai_decide(&joey_duel_state,1,&c))ai_queue(&c);}
            else if(joey_duel_state.phase<DUEL_MAIN1&&!duel_standby_has_action(&joey_duel_state,0))command_phase(joey_duel_state.phase+1);
            next_ai=now+230;
        }
        if(notice&&(int32_t)(now-notice_until)>=0){notice=0;dirty=1;}
        if(scene==1){remodel_report();effect_choices();response_choices();}audio_scene();diagnostic();if(dirty)redraw();
    }
    joey_audio_close();
    if(native_file)h1_fclose(native_file);if(native_art_file)h1_fclose(native_art_file);if(font_file)h1_fclose(font_file);if(scene_file)h1_fclose(scene_file);
    if(font_glyph_cache)h1_free(font_glyph_cache);for(unsigned i=1;i<3;++i)if(font_books[i])h1_free(font_books[i]);h1_timer_stop();joey_native_diagnostic[1]=9;return 0;
}
