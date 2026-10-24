/* Observable rule checks plus complete normal-monster games. These are
 * explicit unit fixtures, not claimed as original full-card gameplay proof. */
#include "duel.h"
#include "duel_cards.h"
#ifdef _WIN32
#include <stdio.h>
#endif
volatile uint32_t native_duel_test_result[24];
volatile uint32_t native_duel_test_failure_lines[16];
static Duel test_game;
static DuelCardDef unsupported_fixture_cards[DUEL_CARD_COUNT];
static uint16_t test_deck[40];
static unsigned checks,failures;
static void check_at(int condition,unsigned line){++checks;if(!condition){++failures;if(failures<=16)native_duel_test_failure_lines[failures-1]=line;if(!native_duel_test_result[2]){native_duel_test_result[2]=checks;native_duel_test_result[12]=line;}}}
#ifdef _WIN32
static void check_host(int condition,unsigned line,const char *file){if(!condition)fprintf(stderr,"fixture %s:%u failed\n",file,line);check_at(condition,line);}
#define check(condition) check_host((condition),__LINE__,__FILE__)
#else
#define check(condition) check_at((condition),__LINE__)
#endif
static void reset(void){duel_start(&test_game,duel_card_definitions,test_deck,40,test_deck,40,123,0);duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,0,0,0,0);duel_equip_setup(&test_game,duel_equip_rules,DUEL_EQUIP_RULE_COUNT);duel_life_setup(&test_game,duel_life_rules,DUEL_LIFE_RULE_COUNT);duel_recall_setup(&test_game,duel_recall_rules,DUEL_RECALL_RULE_COUNT);duel_field_setup(&test_game,duel_field_rules,DUEL_FIELD_RULE_COUNT);duel_aura_setup(&test_game,duel_aura_rules,DUEL_AURA_RULE_COUNT);duel_suppression_setup(&test_game,duel_suppression_rules,DUEL_SUPPRESSION_RULE_COUNT);test_game.phase=DUEL_MAIN1;}
static DuelSlot empty_slot(void){DuelSlot s;volatile uint8_t *bytes=(volatile uint8_t *)&s;for(unsigned i=0;i<sizeof(s);++i)bytes[i]=0;return s;}
static DuelSlot slot(unsigned card,unsigned owner,unsigned position){DuelSlot s=empty_slot();s.card=card;s.owner=owner;s.position=position;s.return_controller=DUEL_NO_SLOT;s.properly_summoned=1;return s;}
static int command(unsigned action,unsigned source,unsigned other,unsigned target,unsigned position,unsigned tribute){DuelCommand c={test_game.active,action,source,other,target,position,tribute};return duel_command(&test_game,&c);}
static unsigned decision_player(void){return test_game.choice.kind?test_game.choice.player:test_game.response_player<2?test_game.response_player:test_game.active;}
static unsigned owned_cards(const Duel *d,unsigned player){unsigned n=d->players[player].grave_count+d->players[player].banished_count+d->players[player].fusion_count;for(unsigned side=0;side<2;++side){const DuelPlayer *p=&d->players[side];for(unsigned i=0;i<p->deck_count;++i)n+=(p->deck_state[i].card&&p->deck_state[i].card==p->deck[i]?p->deck_state[i].owner:side)==player;for(unsigned i=0;i<p->hand_count;++i)n+=(p->hand_state[i].card&&p->hand_state[i].card==p->hand[i]?p->hand_state[i].owner:side)==player;n+=p->field.card&&p->field.owner==player;for(unsigned i=0;i<5;++i){n+=p->monsters[i].card&&!duel_is_token(d,p->monsters[i].card)&&p->monsters[i].owner==player;n+=p->backrow[i].card&&!duel_is_token(d,p->backrow[i].card)&&p->backrow[i].owner==player;}}return n;}

static int choose(unsigned player,unsigned index){DuelCommand c={player,DUEL_ACTION_CHOOSE,index,0,0,0,0,0};return duel_command(&test_game,&c);}
static void fusion_fixture(void){reset();duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,duel_original_fusion_deck,1,duel_original_fusion_deck,1);test_game.turn=3;test_game.players[0].hand_count=3;test_game.players[0].hand[0]=1026;test_game.players[0].hand[1]=834;test_game.players[0].hand[2]=1099;}
static void fusion_finish(unsigned material1,unsigned material2,unsigned position){check(choose(0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_FUSION_MATERIAL);check(choose(0,material1)==DUEL_OK);check(choose(0,material2)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_FUSION_POSITION);check(choose(0,position)==DUEL_OK);}
static void fusion_tests(void){
    DuelCommand c;DuelChoiceOption options[DUEL_OPTION_MAX];unsigned before_owned[2];uint32_t before;
    fusion_fixture();before_owned[0]=owned_cards(&test_game,0);before_owned[1]=owned_cards(&test_game,1);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_FUSION_MONSTER&&test_game.resolving&&test_game.chain_count==1);
    check(test_game.players[0].hand_count==2&&test_game.players[0].backrow[0].card==1026&&!test_game.players[0].grave_count&&test_game.players[0].fusion_count==1);
    check(duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==1&&options[0].card==651);before=duel_state_hash(&test_game);check(choose(1,0)==DUEL_EFFECT_PENDING&&duel_state_hash(&test_game)==before);
    check(choose(0,0)==DUEL_OK&&duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==1&&options[0].card==834);check(choose(0,0)==DUEL_OK&&options[0].index==0);before=duel_state_hash(&test_game);check(choose(0,0)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    check(choose(0,1)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_FUSION_POSITION&&!test_game.players[0].grave_count);check(choose(0,DUEL_FACEUP_ATTACK)==DUEL_OK&&test_game.players[0].monsters[0].card==651&&test_game.players[0].monsters[0].properly_summoned&&!test_game.players[0].fusion_count&&!test_game.players[0].hand_count);
    check(test_game.players[0].grave_count==3&&test_game.players[0].grave[0]==1099&&test_game.players[0].grave[1]==834&&test_game.players[0].grave[2]==1026&&!test_game.players[0].normal_summon_used&&!test_game.resolving&&!test_game.chain_count);
    check(owned_cards(&test_game,0)==before_owned[0]&&owned_cards(&test_game,1)==before_owned[1]);
    /* Own face-down field material is legal, including a temporarily stolen card. */
    fusion_fixture();test_game.players[0].hand_count=2;test_game.players[0].hand[1]=1099;test_game.players[0].monsters[2]=slot(834,1,DUEL_FACEDOWN_DEFENSE);test_game.players[0].monsters[2].serial=700;test_game.next_serial=700;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(DUEL_HAND_MAX+2,0,DUEL_FACEUP_DEFENSE);check(test_game.players[0].monsters[0].card==651&&test_game.players[0].monsters[0].position==DUEL_FACEUP_DEFENSE&&test_game.players[1].grave[0]==834&&!test_game.players[0].monsters[2].card);
    fusion_fixture();for(unsigned i=0;i<5;++i)test_game.players[0].monsters[i]=slot(24,0,DUEL_FACEUP_ATTACK);before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
    test_game.players[0].monsters[2]=slot(834,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].hand_count=2;test_game.players[0].hand[1]=1099;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(DUEL_HAND_MAX+2,0,0);check(test_game.players[0].monsters[2].card==651);
    /* MST does not negate Polymerization; Jammer does and consumes no material. */
    fusion_fixture();test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_FUSION_MONSTER&&test_game.players[0].grave[0]==1026);fusion_finish(0,1,0);check(test_game.players[0].monsters[0].card==651);
    fusion_fixture();test_game.players[1].backrow[0]=slot(893,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.choice.kind&&test_game.players[0].hand_count==2&&test_game.players[0].fusion_count==1&&!test_game.players[0].monsters[0].card);
    /* A linked field material destroyed in a prior chain link is not reserved. */
    fusion_fixture();test_game.players[0].hand_count=2;test_game.players[0].hand[1]=1099;test_game.players[0].monsters[0]=slot(834,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[0].serial=99;test_game.players[0].backrow[1]=slot(1061,0,DUEL_FACEUP_ATTACK);test_game.players[0].backrow[1].effect_active=1;test_game.players[0].backrow[1].link_serial=99;test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,1};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.choice.kind&&!test_game.players[0].monsters[0].card&&test_game.players[0].hand[0]==1099&&test_game.players[0].fusion_count==1);
    fusion_fixture();test_game.players[0].hand_count=2;test_game.players[0].hand[1]=1099;test_game.players[0].monsters[0]=slot(834,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[0].serial=99;test_game.players[0].backrow[1]=slot(1061,0,DUEL_FACEUP_ATTACK);test_game.players[0].backrow[1].effect_active=1;test_game.players[0].backrow[1].link_serial=99;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(DUEL_HAND_MAX,0,0);check(test_game.players[0].backrow[1].card==1061&&!test_game.players[0].backrow[1].link_serial);
    /* Exactly one of the four original substitutes may replace a named card. */
    const unsigned substitutes[]={61,425,679,705};for(unsigned i=0;i<4;++i){fusion_fixture();test_game.players[0].hand[1]=substitutes[i];check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(0,1,0);check(test_game.players[0].monsters[0].card==651);}
    fusion_fixture();test_game.players[0].hand[1]=61;test_game.players[0].hand[2]=425;before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
    /* Repeated names still require three separate material instances, and
     * alternate Blue-Eyes artwork has the valid name-group value zero. */
    reset();const uint16_t ultimate[]={814};check(duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,ultimate,1,0,0)==DUEL_OK);test_game.turn=3;test_game.players[0].hand_count=4;test_game.players[0].hand[0]=1027;test_game.players[0].hand[1]=816;test_game.players[0].hand[2]=817;test_game.players[0].hand[3]=816;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&choose(0,0)==DUEL_OK&&choose(0,0)==DUEL_OK&&choose(0,1)==DUEL_OK&&choose(0,2)==DUEL_OK&&choose(0,0)==DUEL_OK&&test_game.players[0].monsters[0].card==814);
    /* A destroyed proper Fusion remains revivable; one dumped directly
     * from the Fusion Deck does not become proper by entering the grave. */
    fusion_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(0,1,0);test_game.players[0].hand_count=1;test_game.players[0].hand[0]=802;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[0].grave[3]==651&&test_game.players[0].grave_proper[3]&&duel_can_revive(&test_game,0,3));test_game.players[0].hand_count=1;test_game.players[0].hand[0]=359;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,3,0,0)==DUEL_OK&&test_game.players[0].monsters[0].card==651);
    fusion_fixture();test_game.players[0].grave[0]=651;test_game.players[0].grave_count=1;test_game.players[0].hand[0]=359;before=duel_state_hash(&test_game);check(!duel_can_revive(&test_game,0,0)&&command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    /* Conditional direct attacks inspect only visible opposing attributes. */
    fusion_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(0,1,0);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);check(duel_can_attack_direct(&test_game,0,0));test_game.players[1].monsters[1]=slot(834,1,DUEL_FACEDOWN_DEFENSE);check(duel_can_attack_direct(&test_game,0,0));test_game.players[1].monsters[1].position=DUEL_FACEUP_DEFENSE;check(!duel_can_attack_direct(&test_game,0,0));test_game.players[1].monsters[0].position=DUEL_FACEDOWN_DEFENSE;test_game.players[1].monsters[1].position=DUEL_FACEDOWN_DEFENSE;check(!duel_can_attack_direct(&test_game,0,0));test_game.players[1].monsters[0].position=DUEL_FACEUP_ATTACK;test_game.phase=DUEL_BATTLE;check(command(DUEL_ACTION_ATTACK,0,1,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.players[1].life==6300&&test_game.players[1].monsters[0].card==550);
    /* A Fusion return from Penguin or Jar goes to the Fusion Deck. */
    fusion_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(0,1,0);test_game.players[0].monsters[1]=slot(839,0,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_POSITION,1,0,0,0,0)==DUEL_OK&&choose(0,0)==DUEL_OK&&choose(0,DUEL_NO_SLOT)==DUEL_OK&&test_game.players[0].fusion_count==1&&test_game.players[0].fusion[0]==651&&!test_game.players[0].monsters[0].card&&!test_game.players[0].hand_count);
    fusion_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(0,1,0);test_game.players[0].monsters[1]=slot(146,0,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_POSITION,1,0,0,0,0)==DUEL_OK&&test_game.remodel.returned[0]==1&&test_game.remodel.returned_fusion[0]==1&&test_game.players[0].fusion_count==1);
    /* Chain Destruction destroys other copies in the Fusion Deck without
     * making those never-summoned cards eligible for Monster Reborn. */
    fusion_fixture();test_game.players[0].fusion[1]=test_game.players[0].fusion[2]=651;test_game.players[0].fusion_count=3;test_game.players[1].backrow[0]=slot(549,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(0,1,0);check(test_game.response_player==1);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].monsters[0].card==651&&!test_game.players[0].fusion_count&&test_game.players[0].grave_count==5&&!test_game.players[0].grave_proper[3]&&!test_game.players[0].grave_proper[4]);
    /* Backup Soldier returns a non-effect Fusion to its original pile,
     * even when the hand is full. Effect Fusions are not legal targets. */
    fusion_fixture();test_game.players[0].grave_count=5;test_game.players[0].grave[0]=794;test_game.players[0].grave[1]=test_game.players[0].grave[2]=test_game.players[0].grave[3]=test_game.players[0].grave[4]=651;test_game.players[0].hand_count=DUEL_HAND_MAX;for(unsigned i=0;i<DUEL_HAND_MAX;++i)test_game.players[0].hand[i]=24;test_game.players[0].backrow[0]=slot(851,0,DUEL_FACEDOWN_DEFENSE);check(duel_backup_options(&test_game,0,options,DUEL_OPTION_MAX)==1&&options[0].card==794);c=(DuelCommand){0,DUEL_ACTION_ACTIVATE_BACKROW,0};c.target_count=1;c.targets[0]=0;check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==DUEL_HAND_MAX&&test_game.players[0].fusion_count==2&&test_game.players[0].fusion[1]==794);
    /* A proper Fusion can also be revived by Call in attack position. */
    fusion_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(0,1,0);test_game.players[0].hand_count=1;test_game.players[0].hand[0]=802;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);test_game.players[0].backrow[0]=slot(1061,0,DUEL_FACEDOWN_DEFENSE);c=(DuelCommand){0,DUEL_ACTION_ACTIVATE_BACKROW,0,0,3,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].monsters[0].card==651&&test_game.players[0].backrow[0].link_serial==test_game.players[0].monsters[0].serial);
    /* Fusion cards cannot be put into the Main Deck; setup rejects an
     * unimplemented Fusion effect before changing either player's deck. */
    uint16_t saved=test_deck[0];test_deck[0]=651;before=duel_state_hash(&test_game);check(duel_start(&test_game,duel_card_definitions,test_deck,40,test_deck,40,1,0)==DUEL_BAD_ARGUMENT&&duel_state_hash(&test_game)==before);test_deck[0]=saved;
    /* Losing conditional direct-attack permission also requires replay,
     * even when the opposing monster instances did not change. */
    fusion_fixture();check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);fusion_finish(0,1,0);test_game.phase=DUEL_BATTLE;test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[1].backrow[0]=slot(894,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ATTACK,0,1,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.response_player==1);test_game.players[1].monsters[0].position=DUEL_FACEDOWN_DEFENSE;c=(DuelCommand){1,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_ATTACK_REPLAY&&duel_choice_options(&test_game,options,DUEL_OPTION_MAX)==2&&options[0].card==0&&choose(0,DUEL_NO_SLOT)==DUEL_OK&&test_game.players[1].life==8000);
    /* Capacity contract: a complete 15-entry Fusion list fits command
     * validation, while callers requesting four entries keep their canary. */
    fusion_fixture();for(unsigned i=0;i<DUEL_FUSION_MAX;++i)test_game.players[0].fusion[i]=651;test_game.players[0].fusion_count=DUEL_FUSION_MAX;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);options[4]=(DuelChoiceOption){62000,250,250};check(duel_choice_options(&test_game,options,4)==DUEL_FUSION_MAX&&options[4].card==62000&&options[4].index==250);check(choose(0,DUEL_FUSION_MAX-1)==DUEL_OK&&choose(0,0)==DUEL_OK&&choose(0,1)==DUEL_OK&&choose(0,0)==DUEL_OK&&test_game.players[0].fusion_count==DUEL_FUSION_MAX-1&&test_game.players[0].monsters[0].card==651);
}
static void morph_fixture(void){reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(146,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].monsters[0].serial=100;test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0].serial=200;test_game.next_serial=200;}
static void morph_tests(void){
    DuelCommand c;DuelActivationOption responses[69];unsigned spells=0,high=0;
    for(unsigned seed=1;seed<=80;++seed){
        morph_fixture();test_game.random_state=seed;DuelPlayer *p=&test_game.players[0],*e=&test_game.players[1];p->deck_count=6;p->deck[0]=291;p->deck[1]=153;p->deck[2]=359;p->deck[3]=834;p->deck[4]=525;p->deck[5]=291;e->deck_count=4;e->deck[0]=291;e->deck[1]=291;e->deck[2]=153;e->deck[3]=525;
        unsigned owned[2]={owned_cards(&test_game,0),owned_cards(&test_game,1)},hands[2]={p->hand_count,e->hand_count},graves[2]={p->grave_count,e->grave_count};
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.remodel.sequence==1&&!test_game.choice.kind&&!test_game.chain_count);
        check(test_game.remodel.returned[0]==1&&test_game.remodel.returned[1]==1&&p->hand_count==hands[0]&&e->hand_count==hands[1]);
        for(unsigned side=0;side<2;++side){unsigned monsters=0,summons=0,buried=0;DuelPlayer *p=&test_game.players[side];DuelRemodel *r=&test_game.remodel;
            for(unsigned i=0;i<r->revealed[side];++i){unsigned card=r->cards[side][i],placed=r->deployed[side][i];const DuelCardDef *def=&test_game.cards[card];monsters+=def->family==DUEL_MONSTER;spells+=def->family!=DUEL_MONSTER;high+=def->family==DUEL_MONSTER&&def->level>4;
                check(monsters<=r->returned[side]);if(placed){DuelSlot *s=&p->monsters[placed-1];++summons;check(def->family==DUEL_MONSTER&&def->level<=4&&s->card==card&&s->position==DUEL_FACEDOWN_DEFENSE&&s->owner==side&&s->summoned_turn==3&&s->serial&&!s->attack_bonus&&!s->link_serial);if(card==146||card==24)check(s->serial==(card==146?100:200));}else ++buried;
            }
            check(monsters==r->returned[side]&&summons==r->summoned[side]&&p->grave_count==graves[side]+buried&&!p->normal_summon_used);check(owned_cards(&test_game,side)==owned[side]);
        }
    }
    check(spells&&high);
    /* Owner, not current controller, determines each returned-deck count. */
    morph_fixture();test_game.players[0].monsters[0].owner=1;test_game.players[0].monsters[1]=slot(248,1,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0].owner=0;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.remodel.returned[0]==1&&test_game.remodel.returned[1]==2&&!test_game.choice.kind);
    /* Six owned monsters can return from both fields. First legal reveals
     * occupy the five zones; an additional reveal goes to its owner's grave. */
    morph_fixture();test_game.players[0].deck_count=test_game.players[1].deck_count=0;for(unsigned i=1;i<5;++i)test_game.players[0].monsters[i]=slot(834,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0].owner=0;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.remodel.returned[0]==6&&test_game.remodel.revealed[0]==6&&test_game.remodel.summoned[0]==5&&test_game.players[0].grave_count==1&&!test_game.remodel.returned[1]&&!test_game.players[0].deck_count);
    /* Returning five field cards to a 55-card deck fits the full legal deck. */
    morph_fixture();test_game.players[0].deck_count=55;for(unsigned i=0;i<55;++i)test_game.players[0].deck[i]=291;for(unsigned i=1;i<5;++i)test_game.players[0].monsters[i]=slot(834,0,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.remodel.returned[0]==5&&test_game.remodel.revealed[0]<=60&&test_game.players[0].deck_count<=60&&test_game.players[1].life==8000);
    /* The battle-doomed Jar is excluded, then sent to the grave afterwards. */
    morph_fixture();test_game.active=1;test_game.phase=DUEL_BATTLE;check(command(DUEL_ACTION_ATTACK,0,0,0,0,0)==DUEL_OK&&test_game.remodel.sequence==1&&!test_game.remodel.returned[0]&&test_game.remodel.returned[1]==1&&test_game.players[0].grave[0]==146&&!test_game.battle_pending&&!test_game.choice.kind&&test_game.players[0].life==8000);
    morph_fixture();test_game.active=1;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0].defense_bonus=2000;check(command(DUEL_ACTION_ATTACK,0,0,0,0,0)==DUEL_OK&&test_game.remodel.returned[0]==1&&test_game.remodel.returned[1]==1&&test_game.players[1].life==7000);
    /* Destruction of a face-down Jar by a spell does not activate it. */
    morph_fixture();test_game.players[0].hand[0]=802;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&!test_game.remodel.sequence&&test_game.players[0].grave[0]==146);
    /* The mandatory flip effect still resolves after its source is destroyed. */
    morph_fixture();test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.response_player==1&&test_game.chain_count==1);uint32_t random=test_game.random_state;c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.remodel.sequence==1&&!test_game.remodel.returned[0]&&!test_game.remodel.returned[1]&&!test_game.remodel.revealed[0]&&test_game.random_state==random);
    /* After resolving the flip summon, a fresh face-down Special Summon
     * permits Torrential, but is not a face-up Chain Destruction target. */
    morph_fixture();test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[1]=slot(549,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&duel_response_options(&test_game,1,responses,69)==2);
    c=(DuelCommand){1,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.remodel.sequence==1&&test_game.response_player==1&&duel_response_options(&test_game,1,responses,69)==1&&responses[0].card==272);c.action=DUEL_ACTION_ACTIVATE_BACKROW;c.source=0;check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.choice.kind);
    /* The battle damage step does not open that speed-two response. */
    morph_fixture();test_game.active=1;test_game.phase=DUEL_BATTLE;test_game.players[0].backrow[0]=slot(272,0,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ATTACK,0,0,0,0,0)==DUEL_OK&&test_game.remodel.sequence==1&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[0].backrow[0].position==DUEL_FACEDOWN_DEFENSE);
    /* Returning a linked monster leaves an orphaned face-up Call; neither
     * these deck summons nor returns trigger Safe Return or Sangan. */
    morph_fixture();test_game.players[0].monsters[1]=slot(248,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[1].serial=99;test_game.players[0].backrow[0]=slot(1061,0,DUEL_FACEUP_ATTACK);test_game.players[0].backrow[0].serial=98;test_game.players[0].backrow[0].effect_active=1;test_game.players[0].backrow[0].link_serial=99;test_game.players[0].backrow[1]=slot(461,0,DUEL_FACEUP_ATTACK);test_game.players[0].backrow[1].effect_active=1;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.players[0].backrow[0].card==1061&&!test_game.players[0].backrow[0].link_serial&&!test_game.choice.kind&&!test_game.players[0].grave_count);
    /* A legal game cannot reveal an unimplemented effect monster and
     * silently play it as a normal monster: reject that custom deck. */
    uint16_t original=test_deck[0];test_deck[0]=6;uint32_t before=duel_state_hash(&test_game);check(duel_start(&test_game,unsupported_fixture_cards,test_deck,40,test_deck,40,1,0)==DUEL_UNSUPPORTED&&duel_state_hash(&test_game)==before);test_deck[0]=original;
}
static void pass_chain(void){for(unsigned i=0;i<12&&test_game.response_player<2&&!test_game.choice.kind;++i){DuelCommand c={test_game.response_player,DUEL_ACTION_PASS};check(duel_command(&test_game,&c)==DUEL_OK);}}
static void call_fixture(void){reset();test_game.turn=3;test_game.players[0].grave[0]=550;test_game.players[0].grave_count=1;test_game.players[0].backrow[0]=slot(1061,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].backrow[0].set_turn=1;}
static int call_activate(unsigned side,unsigned source,unsigned target){DuelCommand c={side,DUEL_ACTION_ACTIVATE_BACKROW,source,side,target,DUEL_FACEUP_ATTACK};return duel_command(&test_game,&c);}
static void call_tests(void){
    DuelCommand c;DuelChoiceOption options[61];uint32_t before,serial;unsigned hand,deck;
    call_fixture();before=duel_state_hash(&test_game);c=(DuelCommand){0,DUEL_ACTION_ACTIVATE_BACKROW,0,1,0,0};check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    c.target_player=0;c.position=DUEL_FACEUP_DEFENSE;check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    test_game.players[0].backrow[0].set_turn=3;before=duel_state_hash(&test_game);check(call_activate(0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
    call_fixture();test_game.players[0].grave[0]=291;before=duel_state_hash(&test_game);check(call_activate(0,0,0)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    call_fixture();for(unsigned i=0;i<5;++i)test_game.players[0].monsters[i]=slot(550,0,DUEL_FACEUP_ATTACK);before=duel_state_hash(&test_game);check(call_activate(0,0,0)==DUEL_ZONE_FULL&&duel_state_hash(&test_game)==before);
    call_fixture();check(call_activate(0,0,0)==DUEL_OK);check(test_game.players[0].monsters[0].card==550&&test_game.players[0].monsters[0].position==DUEL_FACEUP_ATTACK&&!test_game.players[0].normal_summon_used&&!test_game.players[0].grave_count);
    serial=test_game.players[0].monsters[0].serial;check(serial&&test_game.players[0].backrow[0].effect_active&&test_game.players[0].backrow[0].link_serial==serial);
    before=duel_state_hash(&test_game);check(call_activate(0,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
    /* Source destruction after revival also destroys its linked monster. */
    test_game.players[0].hand[0]=316;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&!test_game.players[0].backrow[0].card&&!test_game.players[0].monsters[0].card&&test_game.players[0].grave_count==3);
    /* Destruction before resolution prevents revival. */
    call_fixture();test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);check(call_activate(0,0,0)==DUEL_OK&&test_game.response_player==1);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].monsters[0].card&&!test_game.players[0].backrow[0].card&&test_game.players[0].grave[0]==550);
    /* A destroyed monster destroys Call; tribute leaves the orphan trap. */
    call_fixture();check(call_activate(0,0,0)==DUEL_OK);test_game.players[0].hand[0]=802;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&!test_game.players[0].backrow[0].card&&!test_game.players[0].monsters[0].card);
    call_fixture();check(call_activate(0,0,0)==DUEL_OK);test_game.players[0].hand[0]=153;check(command(DUEL_ACTION_SUMMON,0,0,0,0,1)==DUEL_OK&&test_game.players[0].monsters[0].card==153&&test_game.players[0].backrow[0].card==1061&&!test_game.players[0].backrow[0].link_serial);
    /* Returning Call destroys the monster; returning the monster keeps Call. */
    call_fixture();check(call_activate(0,0,0)==DUEL_OK);test_game.players[0].hand[0]=729;hand=test_game.players[0].hand_count;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&!test_game.players[0].monsters[0].card&&!test_game.players[0].backrow[0].card&&test_game.players[0].hand_count==hand);
    call_fixture();check(call_activate(0,0,0)==DUEL_OK);test_game.players[0].monsters[1]=slot(839,0,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_POSITION,1,0,0,0,0)==DUEL_OK);check(choose(0,0)==DUEL_OK&&choose(0,DUEL_NO_SLOT)==DUEL_OK&&!test_game.players[0].monsters[0].card&&test_game.players[0].backrow[0].card==1061&&!test_game.players[0].backrow[0].link_serial);
    /* Source association follows the instance across a change of controller. */
    call_fixture();check(call_activate(0,0,0)==DUEL_OK);test_game.active=1;test_game.players[1].hand[0]=300;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[1].monsters[0].card==550);test_game.players[1].hand[0]=316;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&!test_game.players[1].monsters[0].card&&test_game.players[0].grave_count==2);
    /* A face-down monster has already lost the association. */
    call_fixture();check(call_activate(0,0,0)==DUEL_OK);test_game.players[0].monsters[0].position=DUEL_FACEDOWN_DEFENSE;test_game.players[0].hand[0]=316;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[0].monsters[0].card==550);
    /* Two chained Calls target the same grave instance: only one revives. */
    call_fixture();test_game.players[0].backrow[1]=slot(1061,0,DUEL_FACEDOWN_DEFENSE);check(call_activate(0,0,0)==DUEL_OK&&test_game.response_player==0);check(call_activate(0,1,0)==DUEL_OK&&!test_game.players[0].grave_count&&test_game.players[0].monsters[0].card==550&&!test_game.players[0].monsters[1].card&&test_game.players[0].backrow[0].effect_active&&!test_game.players[0].backrow[0].link_serial&&test_game.players[0].backrow[1].link_serial);
    /* A later chain link fills the last zone; the first Call stays orphaned. */
    call_fixture();test_game.players[0].grave[1]=834;test_game.players[0].grave_count=2;test_game.players[0].backrow[1]=slot(1061,0,DUEL_FACEDOWN_DEFENSE);for(unsigned i=1;i<5;++i)test_game.players[0].monsters[i]=slot(24,0,DUEL_FACEUP_ATTACK);
    check(call_activate(0,0,0)==DUEL_OK&&test_game.response_player==0);check(call_activate(0,1,1)==DUEL_OK&&test_game.players[0].monsters[0].card==834&&test_game.players[0].grave_count==1&&test_game.players[0].grave[0]==550&&test_game.players[0].backrow[0].effect_active&&!test_game.players[0].backrow[0].link_serial);
    /* A newly entered same-name grave card cannot replace the old target. */
    call_fixture();test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);check(call_activate(0,0,0)==DUEL_OK&&test_game.response_player==1);test_game.players[0].grave_serial[0]=++test_game.next_serial;pass_chain();check(!test_game.players[0].monsters[0].card&&test_game.players[0].grave[0]==550&&test_game.players[0].backrow[0].effect_active&&!test_game.players[0].backrow[0].link_serial);
    /* A replacement source in the same zone is also a different instance. */
    call_fixture();test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);check(call_activate(0,0,0)==DUEL_OK);test_game.players[0].backrow[0].serial=++test_game.next_serial;pass_chain();check(!test_game.players[0].monsters[0].card&&test_game.players[0].grave[0]==550&&!test_game.players[0].backrow[0].effect_active);
    /* Restoring a monster from face-down does not restore the old link. */
    call_fixture();check(call_activate(0,0,0)==DUEL_OK);test_game.turn=5;test_game.players[0].monsters[0].position=DUEL_FACEDOWN_DEFENSE;check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&!test_game.players[0].backrow[0].link_serial);test_game.players[0].hand[0]=316;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[0].monsters[0].card==550);
    /* Summon traps respond to the completed revival, before battle continues. */
    call_fixture();test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);check(call_activate(0,0,0)==DUEL_OK&&test_game.response_player==1&&test_game.window.kind==DUEL_WINDOW_SUMMON);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].monsters[0].card&&!test_game.players[0].backrow[0].card);
    /* CL2 revival misses optional 'when' timing after CL1 draws two cards. */
    call_fixture();test_game.players[0].backrow[1]=slot(461,0,DUEL_FACEUP_ATTACK);test_game.players[0].backrow[1].effect_active=1;test_game.players[0].backrow[1].serial=100;test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[0].hand[0]=291;deck=test_game.players[0].deck_count;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.response_player==0);check(call_activate(0,0,0)==DUEL_OK&&!test_game.choice.kind&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[0].deck_count==deck-2&&test_game.players[0].monsters[0].card==550);
    /* CL1 revival offers Safe Return, with Torrential legal in that window. */
    call_fixture();test_game.players[0].backrow[1]=slot(461,0,DUEL_FACEUP_ATTACK);test_game.players[0].backrow[1].effect_active=1;test_game.players[0].backrow[1].serial=100;test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);
    check(call_activate(0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_OPTIONAL_DRAW);check(choose(0,DUEL_NO_SLOT)==DUEL_OK&&test_game.response_player==1);pass_chain();
    /* Direct attack -> defender revival -> replay: no damage before choice. */
    call_fixture();test_game.active=1;test_game.phase=DUEL_BATTLE;test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);check(command(DUEL_ACTION_ATTACK,0,0,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.response_player==0);check(call_activate(0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_ATTACK_REPLAY&&test_game.choice.player==1&&test_game.players[0].life==8000);
    check(duel_choice_options(&test_game,options,61)==2&&options[0].index==0&&options[0].kind==DUEL_OPTION_CARD);before=duel_state_hash(&test_game);check(choose(1,5)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);check(choose(1,DUEL_NO_SLOT)==DUEL_OK&&test_game.players[1].monsters[0].attacked&&!test_game.deferred_attack.kind&&test_game.players[0].life==8000);
    call_fixture();test_game.active=1;test_game.phase=DUEL_BATTLE;test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0].attack_bonus=1000;check(command(DUEL_ACTION_ATTACK,0,0,DUEL_NO_SLOT,0,0)==DUEL_OK);check(call_activate(0,0,0)==DUEL_OK&&choose(1,0)==DUEL_OK&&!test_game.players[0].monsters[0].card&&!test_game.players[0].backrow[0].card&&test_game.players[0].life<8000&&!test_game.battle_pending);
    /* A defender removed at declaration permits a direct replay, not a
     * second declaration on which Magic Cylinder could be activated. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[1].backrow[0]=slot(894,1,DUEL_FACEDOWN_DEFENSE);test_game.players[0].hand[0]=316;
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.response_player==1);test_game.players[1].monsters[0]=empty_slot();pass_chain();check(test_game.choice.kind==DUEL_CHOICE_ATTACK_REPLAY&&duel_choice_options(&test_game,options,61)==2&&options[0].index==5);check(choose(0,5)==DUEL_OK&&test_game.players[1].life==8000-duel_attack_value(&test_game,0,0)&&test_game.players[1].backrow[0].card==894&&test_game.response_player==DUEL_NO_SLOT);
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(515,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[1].backrow[0]=slot(894,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK);test_game.players[1].monsters[0]=empty_slot();pass_chain();check(duel_choice_options(&test_game,options,61)==1&&options[0].kind==DUEL_OPTION_SKIP);check(choose(0,DUEL_NO_SLOT)==DUEL_OK&&test_game.players[1].life==8000);
}
static void chain_destruction_fixture(void){
    reset();test_game.turn=3;test_game.players[0].hand_count=3;test_game.players[0].hand[0]=550;test_game.players[0].hand[1]=550;test_game.players[0].hand[2]=24;
    test_game.players[0].deck_count=4;test_game.players[0].deck[0]=550;test_game.players[0].deck[1]=24;test_game.players[0].deck[2]=834;test_game.players[0].deck[3]=525;
    test_game.players[1].backrow[0]=slot(549,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
}
static void chain_destruction_tests(void){
    DuelCommand c;unsigned hand;uint32_t before,random;
    chain_destruction_fixture();check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1);before=duel_state_hash(&test_game);
    c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,1,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    random=test_game.random_state;c.target_player=0;check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].monsters[0].card==550&&test_game.players[0].hand_count==1&&test_game.players[0].hand[0]==24&&test_game.players[0].deck_count==3&&test_game.players[0].grave_count==2&&test_game.players[1].grave[0]==549&&test_game.random_state!=random);
    check(test_game.players[0].grave[0]==550&&test_game.players[0].grave[1]==550&&test_game.players[0].grave_serial[0]!=test_game.players[0].grave_serial[1]);
    /* 2000 is inclusive and refers to current ATK, not printed ATK. */
    for(unsigned extra=100;extra<=101;++extra){chain_destruction_fixture();test_game.players[0].monsters[0]=slot(550,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].monsters[0].attack_bonus=extra;
        check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);check(test_game.response_player==(extra==100?1:DUEL_NO_SLOT));}
    chain_destruction_fixture();test_game.players[0].hand[0]=307;check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[1].backrow[0].card==549);
    chain_destruction_fixture();check(command(DUEL_ACTION_SET_MONSTER,0,0,0,0,0)==DUEL_OK&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[1].backrow[0].card==549);
    chain_destruction_fixture();test_game.players[1].backrow[0].set_turn=3;check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK&&test_game.response_player==DUEL_NO_SLOT);
    /* A player may respond to their own summon, without destroying the field
       monster. A zero-copy activation still reaches the shuffle step. */
    chain_destruction_fixture();test_game.players[0].backrow[0]=test_game.players[1].backrow[0];test_game.players[0].backrow[0].owner=0;test_game.players[1].backrow[0]=empty_slot();
    check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK&&test_game.response_player==0);c=(DuelCommand){0,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].monsters[0].card==550&&test_game.players[0].grave_count==3);
    chain_destruction_fixture();test_game.players[0].hand_count=1;test_game.players[0].deck[0]=380;check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1);random=test_game.random_state;c.player=1;check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].grave_count&&test_game.random_state!=random);
    /* The original cached the name/player. Destruction of the target in a
       later chain link does not stop the hand/deck destruction. */
    chain_destruction_fixture();test_game.players[0].backrow[0]=slot(272,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].backrow[0].set_turn=1;
    check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK&&test_game.response_player==1);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.response_player==0);
    c=(DuelCommand){0,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].monsters[0].card&&test_game.players[0].hand_count==1&&test_game.players[0].deck_count==3&&test_game.players[0].grave_count==4);
    /* ATK can exceed 2000 before resolution. This does not recheck eligibility. */
    chain_destruction_fixture();test_game.players[0].monsters[0]=slot(550,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].monsters[0].attack_bonus=100;test_game.players[0].hand[0]=610;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.response_player==1);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.response_player==0);
    c=(DuelCommand){0,DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&duel_attack_value(&test_game,0,0)>2000&&test_game.players[0].hand_count==1&&test_game.players[0].hand[0]==24&&test_game.players[0].deck_count==3);
    /* MST destroying an activated trap does not negate its effect. */
    chain_destruction_fixture();test_game.players[0].hand[2]=316;check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.response_player==0);
    c=(DuelCommand){0,DUEL_ACTION_ACTIVATE_HAND,1,1,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].hand_count&&test_game.players[0].deck_count==3&&test_game.players[1].grave[0]==549&&test_game.players[0].monsters[0].card==550);
    /* Reborn can summon a card owned by the other player. The original uses
       the summoning controller's piles, not the monster owner's piles. */
    chain_destruction_fixture();test_game.players[0].hand[0]=359;test_game.players[1].grave[0]=550;test_game.players[1].grave_count=1;test_game.players[1].hand[0]=550;hand=test_game.players[1].hand_count;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,1,0,0,0)==DUEL_OK&&test_game.response_player==1&&test_game.players[0].monsters[0].owner==1);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==1&&test_game.players[0].deck_count==3&&test_game.players[1].hand_count==hand&&test_game.players[1].hand[0]==550&&test_game.players[0].monsters[0].card==550);
    /* The original name matcher includes alternate art and the Cyber Harpie
       alias; it is not equality of record numbers or English names. */
    chain_destruction_fixture();test_game.players[0].hand[0]=696;test_game.players[0].hand[1]=697;test_game.players[0].deck[0]=697;check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK);c=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==1&&test_game.players[0].grave[0]==697&&test_game.players[0].grave[1]==697&&test_game.players[0].monsters[0].card==696);
    chain_destruction_fixture();test_game.players[0].monsters[0]=slot(803,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].monsters[0].attack_bonus=-500;test_game.players[0].hand_count=1;test_game.players[0].hand[0]=805;test_game.players[0].deck[0]=805;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].hand_count&&test_game.players[0].grave_count==2&&test_game.players[0].monsters[0].card==803);
    /* Sending copies from hand/deck does not fire a field-to-grave search. */
    chain_destruction_fixture();test_game.players[0].hand[0]=test_game.players[0].hand[1]=test_game.players[0].deck[0]=248;check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK);check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.choice.kind&&test_game.players[0].grave_count==2&&test_game.players[0].monsters[0].card==248);
}
static void backup_fixture(void){
    reset();test_game.turn=3;test_game.players[0].backrow[0]=slot(851,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].backrow[0].set_turn=1;
    static const uint16_t cards[]={834,525,886,248,550};for(unsigned i=0;i<5;++i)test_game.players[0].grave[i]=cards[i];test_game.players[0].grave_count=5;
}
static DuelCommand backup_request(unsigned source,unsigned count,unsigned one,unsigned two,unsigned three){DuelCommand c={0,DUEL_ACTION_ACTIVATE_BACKROW,source,0,0,0,0,0};c.target_count=count;c.targets[0]=one;c.targets[1]=two;c.targets[2]=three;return c;}
static void backup_tests(void){
    DuelChoiceOption options[192];DuelCommand c,response;unsigned hand;uint32_t before;
    /* Five monsters are required, including effects, rather than five cards.
       Select one to three distinct low-ATK non-effect graveyard instances. */
    backup_fixture();check(duel_backup_options(&test_game,0,options,192)==3&&options[0].card==834&&options[1].card==525&&options[2].card==886);
    before=duel_state_hash(&test_game);c=backup_request(0,0,0,0,0);check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    c=backup_request(0,4,0,1,2);check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    c=backup_request(0,2,0,0,0);check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    c=backup_request(0,1,3,0,0);check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    c.targets[0]=4;check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);c.targets[0]=5;check(duel_command(&test_game,&c)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    test_game.players[0].grave[4]=291;before=duel_state_hash(&test_game);c.targets[0]=0;check(!duel_backup_options(&test_game,0,0,0)&&duel_command(&test_game,&c)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
    backup_fixture();test_game.players[0].backrow[0].set_turn=3;c=backup_request(0,1,0,0,0);before=duel_state_hash(&test_game);check(duel_command(&test_game,&c)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
    backup_fixture();hand=test_game.players[0].hand_count;c=backup_request(0,1,0,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==hand+1&&test_game.players[0].hand[hand]==834&&test_game.players[0].grave_count==5&&!test_game.players[0].backrow[0].card);
    backup_fixture();hand=test_game.players[0].hand_count;c=backup_request(0,2,2,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==hand+2&&test_game.players[0].hand[hand]==886&&test_game.players[0].hand[hand+1]==834&&test_game.players[0].grave[0]==525&&test_game.players[0].grave_count==4);
    backup_fixture();hand=test_game.players[0].hand_count;c=backup_request(0,3,0,1,2);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==hand+3&&test_game.players[0].grave_count==3&&test_game.players[0].grave[0]==248&&test_game.players[0].grave[1]==550);
    /* Resolving an earlier link does not recheck the five-monster condition. */
    backup_fixture();test_game.players[0].grave[3]=380;test_game.players[0].grave[4]=248;test_game.players[0].grave[5]=550;test_game.players[0].grave_count=6;test_game.players[0].backrow[1]=slot(851,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].backrow[1].set_turn=1;hand=test_game.players[0].hand_count;
    c=backup_request(0,1,3,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.response_player==0&&test_game.chain_count==1);c=backup_request(1,3,0,1,2);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==hand+4&&test_game.players[0].hand[hand+3]==380&&test_game.players[0].grave_count==4);
    /* A later recovery may remove some targets. Distinct same-name copies
       remain distinct when the grave array compacts. */
    backup_fixture();test_game.players[0].grave[1]=834;test_game.players[0].backrow[1]=slot(851,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].backrow[1].set_turn=1;hand=test_game.players[0].hand_count;
    c=backup_request(0,3,0,1,2);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.response_player==0);c=backup_request(1,1,0,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].hand_count==hand+3&&test_game.players[0].hand[hand]==834&&test_game.players[0].hand[hand+1]==834&&test_game.players[0].hand[hand+2]==886);
    /* Destruction of the activated trap does not negate its recovery. */
    backup_fixture();test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;hand=test_game.players[0].hand_count;c=backup_request(0,1,0,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.response_player==1);
    response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&!test_game.players[0].backrow[0].card&&test_game.players[0].hand_count==hand+1&&test_game.players[0].hand[hand]==834);
    backup_fixture();test_game.players[1].backrow[0]=slot(893,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;c=backup_request(0,1,0,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[1].backrow[0].card==893);
    /* A graveyard card that left and came back is a new instance. */
    backup_fixture();test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;hand=test_game.players[0].hand_count;c=backup_request(0,1,0,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.response_player==1);
    test_game.players[0].grave_serial[0]=++test_game.next_serial;response=(DuelCommand){1,DUEL_ACTION_PASS,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.players[0].hand_count==hand&&test_game.players[0].grave[0]==834);
    /* Reborn follows its actual target after an earlier recovery shifts it. */
    backup_fixture();test_game.players[0].grave[1]=550;test_game.players[0].hand[0]=359;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,1,0,0)==DUEL_OK&&test_game.response_player==0);c=backup_request(0,1,0,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[0].monsters[0].card==550&&test_game.players[0].hand[test_game.players[0].hand_count-1]==834);
    /* Recovering Reborn's target cannot redirect it to a same-name copy. */
    backup_fixture();test_game.players[0].grave[1]=834;test_game.players[0].hand[0]=359;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.response_player==0);c=backup_request(0,1,0,0,0);check(duel_command(&test_game,&c)==DUEL_OK&&!test_game.players[0].monsters[0].card&&test_game.players[0].grave[0]==834);
    /* Backup can respond to the attack declaration, not the post-calculation
       flip effect in the Damage Step. */
    backup_fixture();test_game.active=1;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(839,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_ATTACK,0,0,0,0,0)==DUEL_OK&&test_game.response_player==0);response=(DuelCommand){0,DUEL_ACTION_PASS,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_PENGUIN);
    check(choose(0,5)==DUEL_OK&&choose(0,DUEL_NO_SLOT)==DUEL_OK&&test_game.players[0].backrow[0].card==851&&test_game.response_player==DUEL_NO_SLOT&&!test_game.battle_pending);
}
static void penguin_tests(void){
    DuelChoiceOption options[61];DuelCommand response;unsigned own_hand,enemy_hand,count;uint32_t before;
    /* Select all targets before resolving. Own monsters and hidden opposing
       monsters are legal; the latter keep their identity hidden in the UI. */
    reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(839,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].monsters[1]=slot(248,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[1]=slot(834,1,DUEL_FACEDOWN_DEFENSE);
    own_hand=test_game.players[0].hand_count;enemy_hand=test_game.players[1].hand_count;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_PENGUIN&&test_game.choice.player==0&&test_game.response_player==DUEL_NO_SLOT);
    count=duel_choice_options(&test_game,options,61);check(count==5&&options[0].card==839&&options[3].card==0&&options[3].index==6&&options[4].kind==DUEL_OPTION_SKIP);
    before=duel_state_hash(&test_game);check(choose(1,0)==DUEL_EFFECT_PENDING&&choose(0,10)==DUEL_BAD_TARGET&&duel_state_hash(&test_game)==before);
    check(choose(0,0)==DUEL_OK&&test_game.choice.selected_count==1&&test_game.players[0].monsters[0].card==839&&test_game.players[0].hand_count==own_hand);
    check(choose(0,0)==DUEL_BAD_TARGET&&test_game.choice.selected_count==1);
    check(choose(0,5)==DUEL_OK&&!test_game.choice.kind&&!test_game.chain_count&&!test_game.players[0].monsters[0].card&&!test_game.players[1].monsters[0].card);
    check(test_game.players[0].hand_count==own_hand+1&&test_game.players[1].hand_count==enemy_hand+1&&!test_game.players[0].grave_count&&!test_game.players[1].grave_count&&!test_game.players[0].normal_summon_used);
    /* Returning a search monster never triggers its sent-to-graveyard effect. */
    reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(839,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].monsters[1]=slot(248,0,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);check(choose(0,1)==DUEL_OK&&test_game.players[0].monsters[1].card==248);check(choose(0,DUEL_NO_SLOT)==DUEL_OK&&!test_game.choice.kind&&!test_game.players[0].monsters[1].card&&!test_game.players[0].grave_count&&test_game.players[0].monsters[0].card==839);
    /* Choosing zero is allowed. Control determines the chooser, ownership the
       receiving hand, including the flip monster returning itself. */
    reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(839,1,DUEL_FACEDOWN_DEFENSE);test_game.players[0].monsters[0].return_controller=1;own_hand=test_game.players[0].hand_count;enemy_hand=test_game.players[1].hand_count;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.choice.player==0);check(choose(0,0)==DUEL_OK&&choose(0,DUEL_NO_SLOT)==DUEL_OK&&test_game.players[0].hand_count==own_hand&&test_game.players[1].hand_count==enemy_hand+1);
    reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(839,0,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);check(choose(0,DUEL_NO_SLOT)==DUEL_OK&&test_game.players[0].monsters[0].card==839&&!test_game.choice.kind);
    /* A battle-destroyed flip monster stays on the field during its effect,
       cannot target itself, then goes to the grave after the bounce resolves. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(839,1,DUEL_FACEDOWN_DEFENSE);own_hand=test_game.players[0].hand_count;
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_PENGUIN&&test_game.choice.player==1&&test_game.battle_pending&&test_game.players[1].monsters[0].card==839&&!test_game.players[1].grave_count);
    check(duel_choice_options(&test_game,options,61)==2&&options[0].index==0&&choose(1,5)==DUEL_BAD_TARGET);
    check(choose(1,0)==DUEL_OK&&choose(1,DUEL_NO_SLOT)==DUEL_OK&&!test_game.battle_pending&&!test_game.players[0].monsters[0].card&&test_game.players[0].hand_count==own_hand+1&&test_game.players[1].grave[0]==839&&!test_game.players[1].monsters[0].card);
    /* A surviving Penguin can return itself. LP damage happens before choices. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[0].attack_bonus=-1500;test_game.players[1].monsters[0]=slot(839,1,DUEL_FACEDOWN_DEFENSE);
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.players[0].life==7700&&test_game.choice.kind==DUEL_CHOICE_PENGUIN);check(choose(1,5)==DUEL_OK&&choose(1,0)==DUEL_OK&&!test_game.players[0].monsters[0].card&&!test_game.players[1].monsters[0].card&&!test_game.players[1].grave_count);
    /* Effect destruction of a set monster does not flip it. */
    reset();test_game.players[0].hand[0]=802;test_game.players[1].monsters[0]=slot(839,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&!test_game.choice.kind&&test_game.players[1].grave[0]==839);
    /* Torrential waits until targets are selected, then resolves first. */
    reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(839,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;own_hand=test_game.players[0].hand_count;enemy_hand=test_game.players[1].hand_count;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_PENGUIN&&test_game.response_player==DUEL_NO_SLOT);check(choose(0,0)==DUEL_OK&&choose(0,5)==DUEL_OK&&test_game.response_player==1&&test_game.chain_count==1);
    response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&!test_game.chain_count&&test_game.players[0].grave[0]==839&&test_game.players[1].grave[0]==550&&test_game.players[0].hand_count==own_hand&&test_game.players[1].hand_count==enemy_hand);
    /* A changed instance at a selected location is never returned instead. */
    reset();test_game.turn=3;test_game.players[0].monsters[0]=slot(839,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0].serial=1000;test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;test_game.players[0].backrow[0]=slot(461,0,DUEL_FACEUP_ATTACK);test_game.next_serial=1001;
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);check(choose(0,0)==DUEL_OK&&choose(0,5)==DUEL_OK&&test_game.response_player==1);
    test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0].serial=1002;response=(DuelCommand){1,DUEL_ACTION_PASS,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.players[1].monsters[0].card==24&&!test_game.players[0].monsters[0].card);
    /* MST/Dice can respond before battle damage, not after calculation to the
       flip effect; a summon trap is not legal in this damage-step chain. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].hand[0]=610;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(839,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;test_game.players[1].backrow[1]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[1].set_turn=1;
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.response_player==1);response=(DuelCommand){1,DUEL_ACTION_PASS,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.response_player==0);response.player=0;check(duel_command(&test_game,&response)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_PENGUIN);
    check(choose(1,0)==DUEL_OK&&choose(1,DUEL_NO_SLOT)==DUEL_OK&&test_game.response_player==DUEL_NO_SLOT&&!test_game.battle_pending&&test_game.players[1].backrow[0].card==316&&test_game.players[1].backrow[1].card==272&&test_game.players[0].hand[0]==610);
}
static void continuous_and_battle_tests(void){
    unsigned deck,hand,random;DuelChoiceOption options[61];DuelCommand response;
    /* Healing is mandatory even without another copy; declining never consumes cards. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(453,1,DUEL_FACEDOWN_DEFENSE);random=test_game.random_state;
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK);check(test_game.players[1].life==9000&&!test_game.choice.kind&&test_game.players[1].grave[0]==453&&test_game.random_state==random);
    /* A legal three-copy deck can choose zero, one, or both replacements. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(453,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].deck_count=3;test_game.players[1].deck[0]=453;test_game.players[1].deck[1]=453;test_game.players[1].deck[2]=24;
    test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK);check(test_game.choice.kind==DUEL_CHOICE_NIMBLE_SUMMON&&test_game.choice.player==1&&test_game.players[1].life==9000);
    check(duel_choice_options(&test_game,options,61)==3&&options[2].kind==DUEL_OPTION_SKIP&&options[2].index==DUEL_NO_SLOT);
    check(choose(1,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_NIMBLE_SUMMON&&test_game.players[1].deck_count==2&&test_game.players[1].monsters[0].card==453);
    check(choose(1,0)==DUEL_OK&&!test_game.choice.kind&&test_game.players[1].deck_count==1&&test_game.players[1].monsters[1].card==453);
    check(test_game.players[1].monsters[0].position==DUEL_FACEDOWN_DEFENSE&&test_game.players[1].monsters[1].position==DUEL_FACEDOWN_DEFENSE&&test_game.players[1].monsters[0].serial!=test_game.players[1].monsters[1].serial&&!test_game.players[1].normal_summon_used&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[1].backrow[0].card==272);
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(453,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].deck_count=2;test_game.players[1].deck[0]=453;test_game.players[1].deck[1]=24;
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK);random=test_game.random_state;check(choose(1,DUEL_NO_SLOT)==DUEL_OK&&!test_game.choice.kind&&test_game.players[1].deck_count==2&&!test_game.players[1].monsters[0].card&&test_game.players[1].life==9000&&test_game.random_state==random);
    /* Tributing or spell destruction does not satisfy the battle trigger. */
    reset();test_game.players[0].hand[0]=153;test_game.players[0].monsters[0]=slot(453,0,DUEL_FACEUP_ATTACK);check(command(DUEL_ACTION_SUMMON,0,0,0,0,1)==DUEL_OK&&test_game.players[0].life==8000&&!test_game.choice.kind);
    reset();test_game.players[0].hand[0]=802;test_game.players[1].monsters[0]=slot(453,1,DUEL_FACEUP_ATTACK);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[1].life==8000&&!test_game.choice.kind);
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(453,1,DUEL_FACEUP_ATTACK);test_game.players[1].life=500;check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.winner==0&&test_game.players[1].life==0);
    /* The original owner heals, but a full owner field leaves no summon space. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;for(unsigned i=0;i<5;++i)test_game.players[0].monsters[i]=slot(550,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(453,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].deck_count=1;test_game.players[0].deck[0]=453;
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.players[0].life==9000&&test_game.players[1].life==8000&&!test_game.choice.kind&&test_game.players[0].deck_count==1);
    /* Safe Return remains in the backrow, and its draw is optional and repeatable. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=461;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.players[0].backrow[0].card==461&&test_game.players[0].backrow[0].effect_active&&!test_game.players[0].grave_count);
    test_game.players[0].hand[0]=359;test_game.players[0].grave[0]=550;test_game.players[0].grave_count=1;deck=test_game.players[0].deck_count;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_OPTIONAL_DRAW);
    hand=test_game.players[0].hand_count;check(choose(0,DUEL_NO_SLOT)==DUEL_OK&&!test_game.choice.kind&&test_game.players[0].deck_count==deck&&test_game.players[0].hand_count==hand&&test_game.players[0].backrow[0].effect_active);
    test_game.players[0].hand[0]=359;test_game.players[0].grave[1]=550;test_game.players[0].grave_count=2;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,1,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_OPTIONAL_DRAW);hand=test_game.players[0].hand_count;
    check(choose(0,0)==DUEL_OK&&!test_game.choice.kind&&test_game.players[0].deck_count==deck-1&&test_game.players[0].hand_count==hand+1&&test_game.players[0].backrow[0].card==461);
    /* Each live copy can trigger; decisions are collected into one trigger chain. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=test_game.players[0].hand[1]=461;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
    test_game.players[0].hand[0]=359;test_game.players[0].grave[0]=550;test_game.players[0].grave_count=1;deck=test_game.players[0].deck_count;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
    check(choose(0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_OPTIONAL_DRAW&&test_game.chain_count==1&&test_game.response_player==DUEL_NO_SLOT);
    check(choose(0,0)==DUEL_OK&&!test_game.choice.kind&&!test_game.chain_count&&test_game.players[0].deck_count==deck-2&&test_game.players[0].backrow[0].effect_active&&test_game.players[0].backrow[1].effect_active);
    /* Reviving from the opponent's grave triggers that grave owner's Safe Return. */
    reset();test_game.players[0].hand[0]=461;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);
    test_game.players[1].hand[0]=359;test_game.players[0].grave[0]=550;test_game.players[0].grave_count=1;deck=test_game.players[0].deck_count;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.choice.player==0&&test_game.choice.kind==DUEL_CHOICE_OPTIONAL_DRAW);
    check(choose(0,0)==DUEL_OK&&test_game.players[0].deck_count==deck-1&&test_game.players[1].monsters[0].card==550&&test_game.players[1].monsters[0].owner==0&&test_game.active==1);
    /* Destroying a continuous card before its activation resolves leaves no live effect. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=461;test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&!test_game.players[0].backrow[0].card&&test_game.players[0].grave[0]==461);
    /* A draw effect is not a new Spell Card activation: Jammer cannot negate it. MST can remove its source. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=461;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);
    test_game.players[0].hand[0]=359;test_game.players[0].grave[0]=550;test_game.players[0].grave_count=1;test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);response=(DuelCommand){1,DUEL_ACTION_PASS,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_OPTIONAL_DRAW);
    test_game.players[1].backrow[1]=slot(893,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[1].set_turn=1;deck=test_game.players[0].deck_count;check(choose(0,0)==DUEL_OK&&test_game.response_player==1);
    DuelActivationOption replies[69];check(duel_response_options(&test_game,1,replies,69)==1&&replies[0].card==316);
    response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.response_player==1);
    response=(DuelCommand){1,DUEL_ACTION_PASS,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.players[0].deck_count==deck&&!test_game.players[0].backrow[0].card&&test_game.players[1].backrow[1].card==893&&test_game.players[0].monsters[0].card==550);
    /* A summon-response trap can chain to the optional draw activation. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=461;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);test_game.players[0].hand[0]=359;test_game.players[0].grave[0]=550;test_game.players[0].grave_count=1;test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;deck=test_game.players[0].deck_count;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK&&test_game.choice.kind==DUEL_CHOICE_OPTIONAL_DRAW);check(choose(0,0)==DUEL_OK&&test_game.response_player==1&&test_game.chain_count==1);
    response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&!test_game.players[0].monsters[0].card&&test_game.players[0].deck_count==deck-1&&test_game.players[0].backrow[0].effect_active);
    /* A trigger during end-phase cleanup can add a card: enforce the limit again before changing turns. */
    reset();test_game.players[0].monsters[0]=slot(248,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[0].return_controller=1;for(unsigned i=0;i<5;++i)test_game.players[1].monsters[i]=slot(550,1,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK&&test_game.active==0&&test_game.choice.kind==DUEL_CHOICE_SEARCH_ATTACK);
    unsigned count=duel_choice_options(&test_game,options,61);check(count>0);check(choose(0,options[0].index)==DUEL_OK&&test_game.players[0].hand_count==7&&test_game.discard_required&&test_game.active==0);
    check(command(DUEL_ACTION_DISCARD,6,0,0,0,0)==DUEL_OK&&test_game.players[0].hand_count==6&&test_game.active==1&&!test_game.discard_required);
}

#include "native_equip_test.inc"
#include "native_life_test.inc"
#include "native_opening_test.inc"
#include "native_flip_test.inc"
#include "native_recall_test.inc"
#include "native_field_test.inc"
#include "native_aura_test.inc"
#include "native_suppression_test.inc"
#include "native_simple_test.inc"
#include "native_monster_test.inc"
#include "native_ritual_test.inc"
#include "native_flip_auto_test.inc"
#include "native_battle_trap_test.inc"
#include "native_continuous_test.inc"
#include "native_ignition_test.inc"
#include "native_transient_test.inc"
#include "native_recruit_test.inc"
#include "native_swarm_test.inc"
#include "native_target_flip_test.inc"
#include "native_search_test.inc"
#include "native_hand_test.inc"
#include "native_recycle_equip_test.inc"
#include "native_periodic_test.inc"
#include "native_dynamic_equip_test.inc"
#include "native_counter_test.inc"
#include "native_summon_negation_test.inc"
#include "native_zero_ignition_test.inc"
#include "native_summon_trap_test.inc"
#include "native_position_change_test.inc"
#include "native_target_protection_test.inc"
#include "native_exodia_test.inc"
#include "native_zebra_test.inc"
#include "native_target_spell_test.inc"
#include "native_status_equip_test.inc"
#include "native_bog_test.inc"
#include "native_continuous_expansion_test.inc"
#include "native_stat_layer_test.inc"
#include "native_battle_event_test.inc"
#include "native_immediate_expansion_test.inc"
#include "native_protection_counter_test.inc"
#include "native_lp_ignition_test.inc"
#include "native_action_cost_test.inc"
#include "native_summon_end_test.inc"
#include "native_resolution_selector_test.inc"
#include "native_special_hand_test.inc"
#include "native_spirit_position_test.inc"
#include "native_pile_target_test.inc"
#include "native_global_banish_test.inc"
#include "native_combat_stat_test.inc"
#include "native_grave_flip_test.inc"
#include "native_grave_cost_test.inc"
#include "native_continuous_ability_test.inc"
#include "native_summon_lock_test.inc"
#include "native_peek_test.inc"
#include "native_control_change_test.inc"
#include "native_public_hand_test.inc"
#include "native_flip_battle_test.inc"
#include "native_toon_test.inc"
#include "native_battle_passive_test.inc"
#include "native_cleanup_dice_test.inc"
#include "native_pile_summon_test.inc"
#include "native_damage_monster_test.inc"
#include "native_kuriboh_test.inc"
#include "native_guardian_coin_test.inc"
#include "native_equip_periodic_test.inc"
#include "native_time_wizard_test.inc"
#include "native_equip_choice_battle_test.inc"
#include "native_metalmorph_test.inc"
#include "native_metal_evolution_test.inc"
#include "native_mesmeric_test.inc"
#include "native_type_suppression_test.inc"
#include "native_dust_tornado_test.inc"
#include "native_control_layer_test.inc"
#include "native_bagworm_test.inc"
#include "native_snatch_steal_test.inc"
#include "native_invader_test.inc"
#include "native_mushroom_test.inc"
#include "native_tribute_upkeep_test.inc"
#include "native_soul_exchange_test.inc"
#include "native_big_shield_test.inc"
#include "native_gila_test.inc"
#include "native_labyrinth_test.inc"
#include "native_blast_juggler_test.inc"
#include "native_binding_test.inc"
#include "native_magnet_test.inc"
#include "native_spell_immunity_test.inc"
#include "native_traits_test.inc"
#include "native_global_state_test.inc"
#include "native_summon_procedure_test.inc"
#include "native_tokens_test.inc"
#include "native_cocoon_test.inc"
#include "native_equip_end_test.inc"
#include "native_grave_equipment_test.inc"
#include "native_revival_equipment_test.inc"
#include "native_mask_equipment_test.inc"
#include "native_four_positions_test.inc"
#include "native_history_test.inc"
#include "native_supply_test.inc"
#include "native_last_will_test.inc"
#include "native_pile_identity_test.inc"
#include "native_grave_pair_test.inc"
#include "native_revival_jobs_test.inc"
#include "native_necrofear_test.inc"
#include "native_mask_dispel_test.inc"
#include "native_attack_equipment_test.inc"
#include "native_monster_equipment_test.inc"
#include "native_absorption_test.inc"
#include "native_trigger_gate_test.inc"
#include "native_equipment_transfer_test.inc"
#include "native_kunai_test.inc"
#include "native_summon_extension_test.inc"
#include "native_attack_tribute_test.inc"
#include "native_contact_summon_test.inc"
#include "native_fusion_gate_return_test.inc"
#include "native_offering_test.inc"
#include "native_battle_departure_test.inc"
#include "native_battle_revival_test.inc"
#include "native_battle_damage_followups_test.inc"
#include "native_lp_mutation_test.inc"
#include "native_battle_redirect_test.inc"
#include "native_forced_attack_test.inc"
#include "native_destiny_board_test.inc"
#include "native_fairy_box_test.inc"
#include "native_chain_retarget_test.inc"
#include "native_stat_snapshot_test.inc"
#include "native_spirit_makiu_test.inc"
#include "native_destruction_choices_test.inc"
#include "native_standby_peek_test.inc"
#include "native_doomed_dark_test.inc"
#include "native_hand_selection_test.inc"
#include "native_recovery_yado_test.inc"
#include "native_arrival_bind_test.inc"
#include "native_zombie_castle_test.inc"
#include "native_tribute_control_zones_test.inc"
#include "native_turn_status_test.inc"
#include "native_temporary_removal_test.inc"
#include "native_flip_observers_test.inc"
#include "native_excavation_test.inc"
#include "native_borrow_hats_test.inc"
#include "native_passive_journal_test.inc"
#include "native_reactive_hand_test.inc"
#include "native_reactive_field_test.inc"
#include "native_major_riot_test.inc"
#include "native_ordered_multiplier_test.inc"
#include "native_victory_priority_test.inc"
#include "native_source_coordinate_test.inc"
#include "native_fake_bait_test.inc"
static void first_turn_rules(void){
    /* Both seats: the duel's first turn is blocked; the second player's
       first personal turn may attack with a monster summoned that turn. */
    for(unsigned first=0;first<2;++first){
        check(duel_start(&test_game,duel_card_definitions,test_deck,40,test_deck,40,123,first)==DUEL_OK);
        check(test_game.turn==1&&test_game.active==first&&test_game.players[first].hand_count==6);
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);
        test_game.players[first].hand[0]=24;
        check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK);
        uint32_t before=duel_state_hash(&test_game);
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_BATTLE,0,0)==DUEL_NOT_ALLOWED&&duel_state_hash(&test_game)==before);
        DuelCommand ai;test_game.players[first].hand_count=0;
        check(duel_ai_decide(&test_game,first,&ai)&&ai.action==DUEL_ACTION_PHASE&&ai.target==DUEL_MAIN2);
        /* Defensive attack guard even if a bad caller bypasses phase entry. */
        test_game.phase=DUEL_BATTLE;before=duel_state_hash(&test_game);
        check(command(DUEL_ACTION_ATTACK,0,1-first,DUEL_NO_SLOT,0,0)==DUEL_WRONG_PHASE&&duel_state_hash(&test_game)==before);
        test_game.phase=DUEL_MAIN1;test_game.players[first].monsters[0]=empty_slot();
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK&&test_game.turn==2&&test_game.active==1-first);
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);
        test_game.players[1-first].hand[0]=24;
        check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK&&test_game.players[1-first].monsters[0].summoned_turn==2);
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_BATTLE,0,0)==DUEL_OK);
        check(command(DUEL_ACTION_ATTACK,0,first,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.players[first].life==6300);
        /* EP waits for the six-card hand limit before passing the turn. */
        DuelPlayer *p=&test_game.players[1-first];while(p->hand_count<8)p->hand[p->hand_count++]=24;
        check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK&&test_game.discard_required&&test_game.turn==2);
        check(command(DUEL_ACTION_DISCARD,0,0,0,0,0)==DUEL_OK&&p->hand_count==7&&test_game.turn==2);
        check(command(DUEL_ACTION_DISCARD,0,0,0,0,0)==DUEL_OK&&p->hand_count==6&&test_game.turn==3&&test_game.active==first);
    }
}
int native_duel_test_run(void){
    for(unsigned i=0;i<DUEL_CARD_COUNT;++i)unsupported_fixture_cards[i]=duel_card_definitions[i];unsupported_fixture_cards[6].effect=DUEL_E_UNSUPPORTED;
    unsigned selected=0;for(unsigned i=1;i<DUEL_CARD_COUNT&&selected<20;++i){const DuelCardDef *c=&duel_card_definitions[i];if(c->family==DUEL_MONSTER&&c->subtype==0&&c->level<=4&&c->attack>=1000){test_deck[selected*2]=test_deck[selected*2+1]=i;++selected;}}
#ifdef DUEL_FAST_TEST
    check(selected==20);DUEL_FAST_TEST();native_duel_test_result[0]=checks;native_duel_test_result[1]=failures;native_duel_test_result[6]=sizeof(Duel);return failures;
#else
    check(selected==20);simple_tests();monster_tests();ritual_tests();flip_auto_tests();battle_trap_tests();continuous_tests();ignition_tests();transient_tests();recruit_tests();swarm_tests();target_flip_tests();search_tests();hand_tests();recycle_equip_tests();periodic_tests();dynamic_equip_tests();counter_tests();summon_negation_tests();zero_ignition_tests();summon_trap_tests();position_change_tests();target_protection_tests();exodia_tests();zebra_tests();target_spell_tests();status_equip_tests();bog_tests();continuous_expansion_tests();stat_layer_tests();battle_event_tests();immediate_expansion_tests();protection_counter_tests();lp_ignition_tests();action_cost_tests();summon_end_tests();resolution_selector_tests();special_hand_tests();spirit_position_tests();pile_target_tests();global_banish_tests();combat_stat_tests();grave_flip_tests();grave_cost_tests();continuous_ability_tests();summon_lock_tests();peek_tests();control_change_tests();public_hand_tests();flip_battle_tests();toon_tests();battle_passive_tests();cleanup_dice_tests();pile_summon_tests();damage_monster_tests();kuriboh_tests();guardian_coin_tests();equip_periodic_tests();time_wizard_tests();equip_choice_battle_tests();metalmorph_tests();metal_evolution_tests();mesmeric_tests();type_suppression_tests();dust_tornado_tests();control_layer_tests();bagworm_tests();snatch_steal_tests();invader_tests();mushroom_tests();tribute_upkeep_tests();soul_exchange_tests();big_shield_tests();gila_tests();labyrinth_tests();blast_juggler_tests();binding_tests();magnet_tests();spell_immunity_tests();traits_tests();global_state_tests();summon_procedure_tests();tokens_tests();cocoon_tests();equip_end_tests();grave_equipment_tests();revival_equipment_tests();mask_equipment_tests();four_positions_tests();history_tests();supply_tests();last_will_tests();pile_identity_tests();grave_pair_tests();revival_jobs_tests();necrofear_tests();mask_dispel_tests();attack_equipment_tests();monster_equipment_tests();absorption_tests();trigger_gate_tests();equipment_transfer_tests();kunai_tests();summon_extension_tests();attack_tribute_tests();contact_summon_tests();fusion_gate_return_tests();offering_tests();battle_departure_tests();battle_revival_tests();battle_damage_followups_tests();lp_mutation_tests();battle_redirect_tests();forced_attack_tests();destiny_board_tests();fairy_box_tests();chain_retarget_tests();stat_snapshot_tests();spirit_makiu_tests();destruction_choices_tests();standby_peek_tests();doomed_dark_tests();hand_selection_tests();recovery_yado_tests();arrival_bind_tests();zombie_castle_tests();tribute_control_zones_tests();turn_status_tests();temporary_removal_tests();flip_observers_tests();excavation_tests();borrow_hats_tests();passive_journal_tests();reactive_hand_tests();reactive_field_tests();major_riot_tests();ordered_multiplier_tests();victory_priority_tests();source_coordinate_tests();fake_bait_tests();first_turn_rules();reset();check(test_game.players[0].life==8000&&test_game.players[1].life==8000);check(test_game.players[0].hand_count==6&&test_game.players[1].hand_count==5);check(owned_cards(&test_game,0)==40&&owned_cards(&test_game,1)==40);
    uint32_t before=duel_state_hash(&test_game);check(command(DUEL_ACTION_PHASE,0,0,DUEL_BATTLE,0,0)==DUEL_NOT_ALLOWED);check(before==duel_state_hash(&test_game));
    test_game.players[0].hand[0]=24;check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK);check(test_game.players[0].monsters[0].card==24&&test_game.players[0].hand_count==5);
    test_game.players[0].hand[0]=550;check(command(DUEL_ACTION_SUMMON,0,0,1,0,0)==DUEL_SUMMON_USED);
    reset();test_game.players[0].hand[0]=153;before=duel_state_hash(&test_game);check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_NEED_TRIBUTES);check(before==duel_state_hash(&test_game));
    test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);check(command(DUEL_ACTION_SUMMON,0,0,0,0,1)==DUEL_OK);check(test_game.players[0].monsters[0].card==153&&test_game.players[0].grave_count==1);
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(550,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK);check(test_game.players[1].life==7800&&!test_game.players[1].monsters[0].card&&test_game.players[1].grave[0]==24);check(command(DUEL_ACTION_ATTACK,0,1,DUEL_NO_SLOT,0,0)==DUEL_NOT_ALLOWED);
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(380,1,DUEL_FACEDOWN_DEFENSE);
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK);check(test_game.players[0].life==7700&&test_game.players[0].monsters[0].card&&test_game.players[1].monsters[0].position==DUEL_FACEUP_DEFENSE);
    reset();test_game.players[0].hand[0]=291;unsigned hand=test_game.players[0].hand_count,deck=test_game.players[0].deck_count;check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);check(test_game.players[0].hand_count==hand+1&&test_game.players[0].deck_count==deck-2&&test_game.players[0].grave[0]==291);
    reset();test_game.players[0].hand[0]=359;test_game.players[1].grave[0]=550;test_game.players[1].grave_count=1;check(command(DUEL_ACTION_ACTIVATE_HAND,0,1,0,DUEL_FACEUP_DEFENSE,0)==DUEL_OK);check(test_game.players[0].monsters[0].card==550&&test_game.players[0].monsters[0].owner==1&&!test_game.players[0].normal_summon_used&&test_game.players[1].grave_count==0);
    reset();test_game.players[0].hand[0]=316;before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,1,0,0,0)==DUEL_BAD_TARGET);check(before==duel_state_hash(&test_game));
    reset();test_game.cards=unsupported_fixture_cards;test_game.players[0].hand[0]=6;before=duel_state_hash(&test_game);check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_UNSUPPORTED);check(before==duel_state_hash(&test_game));
    /* Goblin changes at battle-phase end, and remains locked through the next own turn. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].hand_count=test_game.players[1].hand_count=0;
    test_game.players[0].monsters[0]=slot(307,0,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_ATTACK,0,1,DUEL_NO_SLOT,0,0)==DUEL_OK);check(test_game.players[1].life==5700&&test_game.players[0].monsters[0].position==DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN2,0,0)==DUEL_OK);check(test_game.players[0].monsters[0].position==DUEL_FACEUP_DEFENSE&&test_game.players[0].monsters[0].position_lock_until==5);
    check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);
    check(test_game.turn==5&&test_game.active==0);check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);before=duel_state_hash(&test_game);
    check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_NOT_ALLOWED);check(before==duel_state_hash(&test_game));
    check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check(command(DUEL_ACTION_PHASE,0,0,DUEL_MAIN1,0,0)==DUEL_OK);
    check(test_game.turn==7&&command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK);
    /* Zombyra cannot attack directly, and loses 200 ATK per battle destruction. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(515,0,DUEL_FACEUP_ATTACK);before=duel_state_hash(&test_game);
    check(command(DUEL_ACTION_ATTACK,0,1,DUEL_NO_SLOT,0,0)==DUEL_NOT_ALLOWED);check(before==duel_state_hash(&test_game));test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK);check(test_game.players[1].life==7600&&duel_attack_value(&test_game,0,0)==1900);
    /* Buster counts the opponent's visible Dragon monsters and graveyard, without revealing sets. */
    reset();test_game.players[0].monsters[0]=slot(716,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(834,1,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[1]=slot(834,1,DUEL_FACEDOWN_DEFENSE);
    test_game.players[1].grave[0]=834;test_game.players[1].grave[1]=24;test_game.players[1].grave_count=2;check(duel_attack_value(&test_game,0,0)==3600);
    test_game.players[1].monsters[1].position=DUEL_FACEUP_DEFENSE;check(duel_attack_value(&test_game,0,0)==4100);
    /* Dice modifies only monsters face up at resolution; bonuses expire at the end of this turn. */
    reset();test_game.players[0].hand[0]=610;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[1]=slot(834,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);int roll=test_game.last_event.amount;check(roll>=1&&roll<=6);check(duel_attack_value(&test_game,0,0)==1700+roll*100&&duel_defense_value(&test_game,0,0)==1150+roll*100);
    check(duel_attack_value(&test_game,0,1)==1200&&duel_attack_value(&test_game,1,0)==1900);check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check(duel_attack_value(&test_game,0,0)==1700&&duel_defense_value(&test_game,0,0)==1150);
    /* Tribute to the Doomed pays a distinct selected discard, even below the spell's index. */
    reset();test_game.players[0].hand[1]=363;test_game.players[0].hand[0]=24;test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEDOWN_DEFENSE);
    DuelCommand paid={0,DUEL_ACTION_ACTIVATE_HAND,1,1,0,0,0,1};before=duel_state_hash(&test_game);check(duel_command(&test_game,&paid)==DUEL_BAD_TARGET);check(before==duel_state_hash(&test_game));
    paid.cost=0;check(duel_command(&test_game,&paid)==DUEL_OK);check(test_game.players[0].hand_count==4&&test_game.players[0].grave_count==2&&test_game.players[0].grave[0]==24&&test_game.players[0].grave[1]==363&&test_game.players[1].grave[0]==550&&!test_game.players[1].monsters[0].card);
    reset();test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0].attacked=1;test_game.players[0].monsters[0]=slot(24,0,DUEL_FACEUP_ATTACK);test_game.players[0].monsters[0].attacked=1;
    check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK);check(!test_game.players[0].monsters[0].attacked&&!test_game.players[1].monsters[0].attacked);
    /* Search effects wait until the tribute summon finishes; invalid choices consume nothing. */
    reset();test_game.players[0].monsters[0]=slot(248,0,DUEL_FACEUP_ATTACK);test_game.players[0].hand[0]=153;
    test_game.players[0].deck_count=3;test_game.players[0].deck[0]=24;test_game.players[0].deck[1]=834;test_game.players[0].deck[2]=550;
    check(command(DUEL_ACTION_SUMMON,0,0,0,0,1)==DUEL_OK);check(test_game.players[0].monsters[0].card==153&&test_game.choice.kind==DUEL_CHOICE_SEARCH_ATTACK&&test_game.choice.player==0);
    DuelChoiceOption options[60];check(duel_choice_options(&test_game,options,60)==1&&options[0].card==834&&options[0].index==1);before=duel_state_hash(&test_game);
    check(command(DUEL_ACTION_PHASE,0,0,DUEL_BATTLE,0,0)==DUEL_EFFECT_PENDING);check(command(DUEL_ACTION_CHOOSE,0,0,0,0,0)==DUEL_BAD_TARGET);check(before==duel_state_hash(&test_game));
    hand=test_game.players[0].hand_count;check(command(DUEL_ACTION_CHOOSE,1,0,0,0,0)==DUEL_OK);check(!test_game.choice.kind&&test_game.players[0].hand_count==hand+1&&test_game.players[0].hand[hand]==834&&test_game.players[0].deck_count==2);
    /* A graveyard trigger belongs to its owner, including during the opponent's battle phase. */
    reset();test_game.active=1;test_game.turn=2;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(261,0,DUEL_FACEDOWN_DEFENSE);test_game.players[1].monsters[0]=slot(550,1,DUEL_FACEUP_ATTACK);
    test_game.players[0].deck_count=1;test_game.players[0].deck[0]=550;
    check(command(DUEL_ACTION_ATTACK,0,0,0,0,0)==DUEL_OK);check(test_game.choice.kind==DUEL_CHOICE_SEARCH_DEFENSE&&test_game.choice.player==0&&test_game.active==1);
    DuelCommand response;check(!duel_ai_decide(&test_game,1,&response)&&duel_ai_decide(&test_game,0,&response));check(response.action==DUEL_ACTION_CHOOSE&&duel_command(&test_game,&response)==DUEL_OK);check(test_game.active==1&&!test_game.choice.kind&&test_game.players[0].deck_count==0);
    /* A mass destruction collects both searches, resolving the turn player's first. */
    reset();test_game.active=1;test_game.players[0].monsters[0]=slot(248,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(261,1,DUEL_FACEUP_ATTACK);test_game.players[1].hand[0]=802;
    for(unsigned side=0;side<2;++side){test_game.players[side].deck_count=1;test_game.players[side].deck[0]=834;}
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);check(!test_game.players[0].monsters[0].card&&!test_game.players[1].monsters[0].card&&test_game.choice.player==1&&test_game.trigger_count==1);
    check(command(DUEL_ACTION_CHOOSE,0,0,0,0,0)==DUEL_OK);check(test_game.choice.kind==DUEL_CHOICE_SEARCH_ATTACK&&test_game.choice.player==0);
    response=(DuelCommand){0,DUEL_ACTION_CHOOSE,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&!test_game.choice.kind&&!test_game.trigger_count);
    reset();test_game.players[0].hand_count=8;test_game.players[0].hand[6]=24;test_game.players[0].hand[7]=550;check(command(DUEL_ACTION_PHASE,0,0,DUEL_END,0,0)==DUEL_OK&&test_game.discard_required);check(command(DUEL_ACTION_DISCARD,0,0,0,0,0)==DUEL_OK&&test_game.active==0);check(command(DUEL_ACTION_DISCARD,0,0,0,0,0)==DUEL_OK&&test_game.active==1&&test_game.players[0].grave_count==2);
    /* Destroying an activated spell does not negate its effect. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=291;test_game.players[1].backrow[0]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;deck=test_game.players[0].deck_count;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);check(test_game.response_player==1&&test_game.chain_count==1&&test_game.players[0].deck_count==deck&&test_game.players[0].hand_count==5);
    response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK);
    check(test_game.response_player==DUEL_NO_SLOT&&!test_game.chain_count&&test_game.players[0].deck_count==deck-2&&test_game.players[0].hand_count==7&&test_game.players[0].grave[0]==291&&test_game.players[1].grave[0]==316);
    /* A counter trap pays its discard, negates the last spell, and blocks speed-two responses. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=291;test_game.players[0].hand[1]=316;test_game.players[1].backrow[0]=slot(893,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;deck=test_game.players[0].deck_count;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,255};before=duel_state_hash(&test_game);
    check(duel_command(&test_game,&response)==DUEL_BAD_TARGET&&before==duel_state_hash(&test_game));response.cost=0;check(duel_command(&test_game,&response)==DUEL_OK);
    check(test_game.players[0].deck_count==deck&&test_game.players[0].hand_count==5&&test_game.players[0].hand[0]==316&&test_game.players[1].hand_count==4&&test_game.players[1].grave_count==2&&test_game.players[0].grave[0]==291&&!test_game.chain_count&&test_game.response_player==DUEL_NO_SLOT);
    /* Cylinder stops battle damage and reflects the attacker's current ATK. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].monsters[0]=slot(550,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);test_game.players[1].backrow[0]=slot(894,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
    check(command(DUEL_ACTION_ATTACK,0,1,0,0,0)==DUEL_OK&&test_game.response_player==1&&test_game.players[1].life==8000);response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK);
    check(test_game.players[0].life==6100&&test_game.players[1].life==8000&&test_game.players[0].monsters[0].card==550&&test_game.players[0].monsters[0].attacked&&test_game.players[1].monsters[0].card==24&&test_game.players[1].grave[0]==894);
    /* LIFO resolution matters: Dice chained to Cylinder changes reflected damage. */
    reset();test_game.turn=3;test_game.phase=DUEL_BATTLE;test_game.players[0].hand[0]=610;test_game.players[0].monsters[0]=slot(550,0,DUEL_FACEUP_ATTACK);test_game.players[1].backrow[0]=slot(894,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
    check(command(DUEL_ACTION_ATTACK,0,1,DUEL_NO_SLOT,0,0)==DUEL_OK);response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.response_player==0&&test_game.chain_count==1);
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_OK);roll=test_game.players[0].monsters[0].temporary_bonus/100;
    check(roll>=1&&roll<=6&&test_game.players[0].life==8000-1900-roll*100&&test_game.players[1].life==8000&&!test_game.chain_count&&test_game.response_player==DUEL_NO_SLOT);
    /* Torrential still resolves after MST destroys it, then grave searches run. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=550;test_game.players[0].hand[1]=316;test_game.players[0].monsters[0]=slot(248,0,DUEL_FACEUP_ATTACK);test_game.players[1].monsters[0]=slot(24,1,DUEL_FACEUP_ATTACK);test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
    check(command(DUEL_ACTION_SUMMON,0,0,1,0,0)==DUEL_OK&&test_game.response_player==1);response=(DuelCommand){1,DUEL_ACTION_ACTIVATE_BACKROW,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.response_player==0&&test_game.chain_count==1);
    response=(DuelCommand){0,DUEL_ACTION_ACTIVATE_HAND,0,1,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK);check(!test_game.players[0].monsters[0].card&&!test_game.players[0].monsters[1].card&&!test_game.players[1].monsters[0].card&&test_game.choice.kind==DUEL_CHOICE_SEARCH_ATTACK&&test_game.choice.player==0);
    /* Set traps cannot respond that turn; a later flip summon opens its own window. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=272;test_game.players[0].hand[1]=550;check(command(DUEL_ACTION_SET_BACKROW,0,0,0,0,0)==DUEL_OK);check(command(DUEL_ACTION_SUMMON,0,0,0,0,0)==DUEL_OK&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[0].monsters[0].card==550);
    test_game.turn=5;test_game.players[0].monsters[0].position=DUEL_FACEDOWN_DEFENSE;check(command(DUEL_ACTION_POSITION,0,0,0,DUEL_FACEUP_ATTACK,0)==DUEL_OK&&test_game.response_player==0);
    /* No free zone and an explicitly invalid fixture effect are rejected
     * before payment, even when all real card effects are implemented. */
    reset();for(unsigned i=0;i<5;++i)test_game.players[0].backrow[i]=slot(316,0,DUEL_FACEDOWN_DEFENSE);test_game.players[0].hand[0]=291;before=duel_state_hash(&test_game);check(command(DUEL_ACTION_ACTIVATE_HAND,0,0,0,0,0)==DUEL_ZONE_FULL&&before==duel_state_hash(&test_game));
    reset();unsigned missing_trap=0;for(unsigned i=0;i<DUEL_CARD_COUNT;++i){snapshot_cards[i]=duel_card_definitions[i];if(!missing_trap&&i&&duel_card_definitions[i].family==DUEL_TRAP)missing_trap=i;}check(missing_trap!=0);snapshot_cards[missing_trap].effect=DUEL_E_UNSUPPORTED;test_game.cards=snapshot_cards;test_game.players[0].hand[0]=missing_trap;before=duel_state_hash(&test_game);check(command(DUEL_ACTION_SET_BACKROW,0,0,0,0,0)==DUEL_UNSUPPORTED&&before==duel_state_hash(&test_game));
    /* A set quick-play spell must wait, even when used during its owner's turn. */
    reset();test_game.players[0].hand[0]=316;test_game.players[1].backrow[0]=slot(851,1,DUEL_FACEDOWN_DEFENSE);check(command(DUEL_ACTION_SET_BACKROW,0,0,0,0,0)==DUEL_OK);before=duel_state_hash(&test_game);
    check(command(DUEL_ACTION_ACTIVATE_BACKROW,0,1,0,0,0)==DUEL_NOT_ALLOWED&&before==duel_state_hash(&test_game));test_game.turn=3;check(command(DUEL_ACTION_ACTIVATE_BACKROW,0,1,0,0,0)==DUEL_OK&&!test_game.players[1].backrow[0].card);
    /* A new card in the same zone is a different instance, never an accidental replacement target. */
    reset();test_game.turn=3;test_game.players[0].hand[0]=316;test_game.players[1].backrow[0]=slot(851,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].serial=1000;test_game.players[1].backrow[1]=slot(316,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[1].set_turn=1;
    check(command(DUEL_ACTION_ACTIVATE_HAND,0,1,0,0,0)==DUEL_OK&&test_game.response_player==1);test_game.players[1].backrow[0]=slot(893,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].serial=1001;
    response=(DuelCommand){1,DUEL_ACTION_PASS,0,0,0,0,0,0};check(duel_command(&test_game,&response)==DUEL_OK&&test_game.players[1].backrow[0].card==893);
    reset();test_game.turn=3;test_game.players[0].hand[0]=550;test_game.players[1].backrow[0]=slot(272,1,DUEL_FACEDOWN_DEFENSE);test_game.players[1].backrow[0].set_turn=1;
    check(command(DUEL_ACTION_SET_MONSTER,0,0,0,0,0)==DUEL_OK&&test_game.response_player==DUEL_NO_SLOT&&test_game.players[0].monsters[0].position==DUEL_FACEDOWN_DEFENSE);
    continuous_and_battle_tests();penguin_tests();backup_tests();chain_destruction_tests();call_tests();morph_tests();fusion_tests();equip_tests();life_tests();opening_tests();targeted_flip_tests();grave_recall_tests();field_tests();aura_tests();suppression_tests();
    unsigned games=0,actions=0,max_turn=0;
    for(unsigned seed=1;seed<=200;++seed){
        check(duel_start(&test_game,duel_card_definitions,test_deck,40,test_deck,40,seed,seed&1)==DUEL_OK);
        unsigned count=0;while(test_game.winner<0&&count<3000){DuelCommand c;int decided=duel_ai_decide(&test_game,decision_player(),&c);check(decided);if(!decided)break;int result=duel_command(&test_game,&c);check(result==DUEL_OK);if(result!=DUEL_OK)break;
            check(owned_cards(&test_game,0)==40&&owned_cards(&test_game,1)==40);++count;}
        check(test_game.winner>=0);if(test_game.winner>=0)++games;actions+=count;if(test_game.turn>max_turn)max_turn=test_game.turn;
    }
    unsigned mixed_games=0,mixed_actions=0,spells=0,mixed_max=0,traps=0,penguin_choices=0,penguin_sets=0,backup_activations=0,chain_destructions=0,calls=0,replays=0,remodels=0,polys=0,fusions=0;
    for(unsigned seed=1;seed<=50;++seed){
        check(duel_start(&test_game,duel_card_definitions,duel_original_deck,DUEL_ORIGINAL_DECK_COUNT,duel_original_deck,DUEL_ORIGINAL_DECK_COUNT,seed,seed&1)==DUEL_OK);
        check(duel_fusion_setup(&test_game,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,duel_original_fusion_deck,DUEL_ORIGINAL_FUSION_DECK_COUNT,duel_original_fusion_deck,DUEL_ORIGINAL_FUSION_DECK_COUNT)==DUEL_OK);
        unsigned count=0;while(test_game.winner<0&&count<3000){DuelCommand c;int decided=duel_ai_decide(&test_game,decision_player(),&c);check(decided);if(!decided)break;unsigned from_set=c.action==DUEL_ACTION_ACTIVATE_BACKROW,card=from_set?test_game.players[c.player].backrow[c.source].card:0;
            polys+=(c.action==DUEL_ACTION_ACTIVATE_HAND&&test_game.cards[test_game.players[c.player].hand[c.source]].effect==DUEL_E_POLYMERIZATION)||(from_set&&test_game.cards[card].effect==DUEL_E_POLYMERIZATION);fusions+=c.action==DUEL_ACTION_CHOOSE&&test_game.choice.kind==DUEL_CHOICE_FUSION_POSITION;
            replays+=c.action==DUEL_ACTION_CHOOSE&&test_game.choice.kind==DUEL_CHOICE_ATTACK_REPLAY;calls+=from_set&&card==1061;penguin_choices+=c.action==DUEL_ACTION_CHOOSE&&test_game.choice.kind==DUEL_CHOICE_PENGUIN;penguin_sets+=c.action==DUEL_ACTION_SET_MONSTER&&test_game.players[c.player].hand[c.source]==839;int result=duel_command(&test_game,&c);check(result==DUEL_OK);if(result!=DUEL_OK)break;
            spells+=c.action==DUEL_ACTION_ACTIVATE_HAND;traps+=from_set&&test_game.cards[card].family==DUEL_TRAP;backup_activations+=from_set&&card==851;chain_destructions+=from_set&&card==549;check(owned_cards(&test_game,0)==DUEL_ORIGINAL_DECK_COUNT+DUEL_ORIGINAL_FUSION_DECK_COUNT&&owned_cards(&test_game,1)==DUEL_ORIGINAL_DECK_COUNT+DUEL_ORIGINAL_FUSION_DECK_COUNT);++count;}
        remodels+=test_game.remodel.sequence;check(test_game.winner>=0);if(test_game.winner>=0)++mixed_games;mixed_actions+=count;if(test_game.turn>mixed_max)mixed_max=test_game.turn;
    }
    native_duel_test_result[0]=checks;native_duel_test_result[1]=failures;native_duel_test_result[3]=games;native_duel_test_result[4]=actions;native_duel_test_result[5]=max_turn;native_duel_test_result[6]=sizeof(Duel);
    native_duel_test_result[7]=mixed_games;native_duel_test_result[8]=mixed_actions;native_duel_test_result[9]=spells;native_duel_test_result[10]=mixed_max;native_duel_test_result[11]=traps;native_duel_test_result[13]=penguin_choices;native_duel_test_result[14]=penguin_sets;native_duel_test_result[15]=backup_activations;native_duel_test_result[16]=chain_destructions;native_duel_test_result[17]=calls;native_duel_test_result[18]=replays;native_duel_test_result[19]=remodels;native_duel_test_result[20]=polys;native_duel_test_result[21]=fusions;return failures;
#endif
}
