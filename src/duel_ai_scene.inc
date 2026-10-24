/* Opponent presentation queues semantic commands; it never interprets x86. */
static DuelCommand ai_command;
static unsigned ai_stage,ai_elapsed,ai_card,ai_motion,ai_sequence;
static int ai_x0,ai_y0,ai_x1,ai_y1;
static uint32_t ai_clock,ai_frame;
static const uint16_t *ai_label;
volatile uint32_t joey_ai_diagnostic[12];
static void ai_queue(const DuelCommand *c){
    ai_command=*c;ai_stage=1;ai_elapsed=0;ai_clock=h1_timer_read_ms();ai_frame=0;ai_card=ai_motion=0;ai_x0=280;ai_y0=3;ai_x1=280;ai_y1=88;
    const DuelPlayer *p=&joey_duel_state.players[1];ai_label=u"对方操作";
    switch(c->action){
    case DUEL_ACTION_SUMMON:ai_label=u"对方召唤";ai_card=p->hand[c->source];ai_motion=1;ai_x1=209+(4-c->target)*177/5;break;
    case DUEL_ACTION_SET_MONSTER:ai_label=u"对方盖放怪兽";ai_motion=1;ai_x1=209+(4-c->target)*177/5;break;
    case DUEL_ACTION_SET_BACKROW:ai_label=u"对方盖放卡片";ai_motion=1;ai_y1=39;ai_x1=209+(4-c->target)*177/5;break;
    case DUEL_ACTION_ACTIVATE_HAND:ai_label=u"对方发动卡片";ai_card=p->hand[c->source];ai_motion=1;ai_x1=290;ai_y1=45;break;
    case DUEL_ACTION_ACTIVATE_BACKROW:ai_label=u"对方发动卡片";ai_card=c->source==DUEL_FIELD_SLOT?p->field.card:p->backrow[c->source].card;ai_x0=209+(4-(c->source<5?c->source:0))*177/5;ai_y0=39;ai_x1=290;ai_y1=45;ai_motion=1;break;
    case DUEL_ACTION_ACTIVATE_MONSTER:ai_label=u"对方发动怪兽效果";ai_card=p->monsters[c->source].card;ai_x0=ai_x1=209+(4-c->source)*177/5;ai_y0=ai_y1=88;ai_motion=3;break;
    case DUEL_ACTION_ATTACK:ai_label=c->target==DUEL_NO_SLOT?u"对方直接攻击":u"对方攻击";ai_card=p->monsters[c->source].card;ai_motion=2;ai_x0=225+(4-c->source)*177/5;ai_y0=111;ai_x1=c->target==DUEL_NO_SLOT?225:225+c->target*177/5;ai_y1=c->target==DUEL_NO_SLOT?248:160;break;
    case DUEL_ACTION_POSITION:ai_label=u"对方改变表示";ai_card=p->monsters[c->source].card;ai_x0=ai_x1=209+(4-c->source)*177/5;ai_y0=ai_y1=88;ai_motion=3;break;
    case DUEL_ACTION_DISCARD:ai_label=u"对方弃牌";ai_card=p->hand[c->source];ai_motion=1;ai_x1=184;ai_y1=43;break;
    case DUEL_ACTION_CHOOSE:ai_label=u"对方选择效果";break;
    case DUEL_ACTION_PASS:ai_label=u"对方不发动";break;
    case DUEL_ACTION_PHASE:ai_label=c->target==DUEL_BATTLE?u"对方战斗阶段":c->target==DUEL_END?u"对方回合结束":c->target==DUEL_MAIN1||c->target==DUEL_MAIN2?u"对方主要阶段":u"对方准备阶段";break;
    }
    ++ai_sequence;dirty=1;
}
static void ai_tick(uint32_t now){
    if(scene!=1){ai_stage=0;ai_clock=now;return;}
    uint32_t delta=now-ai_clock;ai_clock=now;
    if(!ai_stage||details_open||pending==PENDING_REMODEL||pending==PENDING_PUBLIC_HAND)return;
    ai_elapsed+=delta;
    if(ai_stage==1&&ai_elapsed>=480){int result=apply(&ai_command);ai_stage=scene==1&&result==DUEL_OK?2:0;ai_elapsed=0;dirty=1;}
    else if(ai_stage==2&&ai_elapsed>=650){ai_stage=0;next_ai=now+100;dirty=1;}
    if(ai_stage&&(int32_t)(now-ai_frame)>=80){dirty=1;ai_frame=now;}
}
static void ai_draw(void){
    unsigned hand=joey_duel_state.players[1].hand_count,shown=hand>8?8:hand;
    for(unsigned i=0;i<shown;++i)scene_card(185+i*21,4,duel_hand_revealed(&joey_duel_state,1)?joey_duel_state.players[1].hand[i]:0,2,16,23);
    native_number(355,8,hand,12,0xffffff);
    if(!ai_stage)return;
    native_fill(178,0,243,32,0x1082);native_label(183,2,ai_label,12,0xffdf91);
    if(ai_card){uint16_t text[100];NativeCard *c=&native_cards[ai_card];unsigned n=native_string(c->name,c->name_length,text,100);native_wrapped(183,17,232,text,n,12,1,0xffffff);}
    if(ai_stage==1&&ai_motion==1){unsigned t=ai_elapsed>480?480:ai_elapsed;int x=ai_x0+(ai_x1-ai_x0)*(int)t/480,y=ai_y0+(ai_y1-ai_y0)*(int)t/480;native_card(x,y,ai_card,2);native_outline(x-1,y-1,34,48,0xffe0);}
    else if(ai_motion==2){
        int dx=ai_x1-ai_x0,dy=ai_y1-ai_y0;unsigned t=ai_stage==1?ai_elapsed>480?480:ai_elapsed:480;
        if(ai_stage==1||ai_elapsed<180)scene_attack_sword(ai_x0+dx*(int)t/480,ai_y0+dy*(int)t/480,dx,dy,ai_elapsed/100);
        native_outline(ai_x0-17,ai_y0-24,34,48,0xffe0);if(ai_stage==2&&((ai_elapsed/100)&1)==0)native_outline(ai_x1-18,ai_y1-16,36,32,0xf800);
    }else if(ai_stage==2&&ai_motion)native_outline(ai_x1-1,ai_y1-1,34,48,0xffe0);
}
