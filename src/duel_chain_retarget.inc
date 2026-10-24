#include "duel_retarget_data.h"
unsigned duel_retarget_class(unsigned card){return card<DUEL_CARD_COUNT?duel_retarget_classes[card]:0;}
static unsigned link_player(const DuelChainLink *l,unsigned target){return target^(l->player_redirect&1);}
static const DuelChainLink *retarget_previous(const Duel *d){return d->chain_count?&d->chain[d->chain_count-1]:0;}
static int retarget_selector(const Duel *d,const DuelChainLink *l,unsigned side,unsigned index){
    if(side>1||index>=5||!duel_can_target_monster(d,side,index))return 0;
    const DuelCardDef *def=&d->cards[l->card];const DuelSlot *m=&d->players[side].monsters[index];unsigned actor=l->command.player;
    if(side==l->command.target_player&&index==l->command.target)return 0;
    switch(def->effect){
    case DUEL_E_EQUIP:return duel_equip_can_target_for(d,actor,l->card,side,index);
    case DUEL_E_BIND:return duel_binding_can_target(d,actor,l->card,side,index);
    case DUEL_E_CHANGE_HEART:return side!=actor&&effect_affects_monster(d,l->card,side,index);
    case DUEL_E_TRIBUTE_DOOMED:return 1;
    case DUEL_E_CONTINUOUS:return def->name_group==1428&&side==actor&&d->cards[m->card].name_group==1405&&m->position!=DUEL_FACEDOWN_DEFENSE;
    case DUEL_E_SIMPLE:{const SimpleRule *r=simple_rule_def(def);if(!r)return 0;
        switch(r->op){
        case 7:return side!=actor&&(r->amount?m->position==DUEL_FACEUP_ATTACK:m->position!=DUEL_FACEUP_ATTACK);
        case 17:case 23:case 26:case 31:return m->position!=DUEL_FACEDOWN_DEFENSE;
        case 27:return side!=actor;
        case 34:return m->position==DUEL_FACEDOWN_DEFENSE;
        case 46:return side!=actor&&!tribute_spell_immune(d,side,index);
        case 49:return multiply_cost(d,actor,side,index);
        case 51:return m->position!=DUEL_FACEDOWN_DEFENSE&&!duel_is_token(d,m->card);
        case 58:return defusion_target(d,side,index);
        case 62:case 63:return side!=actor&&m->position!=DUEL_FACEDOWN_DEFENSE&&!(side==d->window.player&&index==d->window.source);
        case 69:return duel_makiu_target(d,actor,side,index);
        case 65:return 1;
        case 64:return side==actor&&!(side==l->targets[1].player&&index==l->targets[1].slot)&&l->targets[2].player<2&&l->targets[2].slot<5&&l->targets[1].slot<5&&duel_can_attack_target(d,l->targets[2].player,l->targets[2].slot,l->targets[1].slot);
        default:return 0;}}
    default:return 0;}
}
static int retarget_magic_ready(const Duel *d,unsigned name,unsigned actor){const DuelChainLink *l=retarget_previous(d);if(!l||!(duel_retarget_class(l->card)&1)||(d->cards[l->card].name_group==650&&l->command.cost==2)||l->command.player==actor||l->command.target_player>1||l->command.target>=5||!l->target_serial)return 0;unsigned family=d->cards[l->card].family;if(name==1073)return family==DUEL_SPELL&&(!l->effect_activation||l->source_slot<=DUEL_FIELD_SLOT);return name==1317&&family!=DUEL_MONSTER&&l->command.target_player==actor;}
static int retarget_battle_ready(const Duel *d,unsigned actor){const DuelWindow *w=&d->window;return w->kind==DUEL_WINDOW_ATTACK&&w->player!=actor&&duel_battle_target_player(w)==actor&&w->source<5&&w->target<5&&duel_can_attack_target(d,w->player,w->source,w->target);}
static int retarget_candidate(const Duel *d,unsigned name,unsigned actor,unsigned mode,unsigned side,unsigned index){if(mode==1)return retarget_magic_ready(d,name,actor)&&(name!=1317||side==actor)&&retarget_selector(d,retarget_previous(d),side,index);if(mode==2){const DuelWindow *w=&d->window;return name==1317&&retarget_battle_ready(d,actor)&&side==actor&&index<5&&index!=w->target&&duel_can_target_monster(d,side,index);}return 0;}
unsigned duel_retarget_modes(const Duel *d,unsigned name,unsigned actor){unsigned mask=0;for(unsigned mode=1;mode<=2;++mode)for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(retarget_candidate(d,name,actor,mode,side,i))mask|=1u<<(mode-1);return mask;}
static int refpanel_ready(const Duel *d,unsigned actor){const DuelChainLink *l=retarget_previous(d);if(!l||l->effect_activation||l->source_slot>DUEL_FIELD_SLOT||!(duel_retarget_class(l->card)&2)||d->cards[l->card].name_group==1547)return 0;unsigned name=d->cards[l->card].name_group;return name!=1014&&name!=1070&&name!=1071&&name!=1077&&name!=1025||d->players[name==1025?1-actor:actor].hand_count;}
static int retarget_validate(const Duel *d,const DuelCommand *c,unsigned card){unsigned name=d->cards[card].name_group;if(name==1247)return refpanel_ready(d,c->player)?DUEL_OK:DUEL_NOT_ALLOWED;if(!c->cost)return duel_retarget_modes(d,name,c->player)?DUEL_OK:DUEL_NOT_ALLOWED;return retarget_candidate(d,name,c->player,c->cost,c->target_player,c->target)?DUEL_OK:DUEL_BAD_TARGET;}
static int retarget_begin(Duel *d,const DuelCommand *c,unsigned card){unsigned mask=duel_retarget_modes(d,d->cards[card].name_group,c->player);d->equip_mode_command=*c;d->choice=(DuelTrigger){mask==3?DUEL_CHOICE_RETARGET_MODE:DUEL_CHOICE_RETARGET_MONSTER,c->player,card,0};d->choice.selected_count=mask==2?2:1;return DUEL_OK;}
static unsigned retarget_options(const Duel *d,DuelChoiceOption *options,unsigned capacity){unsigned n=0,actor=d->choice.player,name=d->cards[d->choice.card].name_group;if(d->choice.kind==DUEL_CHOICE_RETARGET_MODE){unsigned mask=duel_retarget_modes(d,name,actor);for(unsigned mode=1;mode<=2;++mode)if(mask>>(mode-1)&1){if(options&&n<capacity)options[n]=(DuelChoiceOption){d->choice.card,mode,DUEL_OPTION_CONFIRM};++n;}}else for(unsigned side=0;side<2;++side)for(unsigned i=0;i<5;++i)if(retarget_candidate(d,name,actor,d->choice.selected_count,side,i)){if(options&&n<capacity)options[n]=(DuelChoiceOption){d->players[side].monsters[i].card,side*5+i,DUEL_OPTION_CARD};++n;}if(options&&n<capacity)options[n]=(DuelChoiceOption){0,DUEL_NO_SLOT,DUEL_OPTION_SKIP};return n+1;}
static int retarget_choose(Duel *d,unsigned token){if(token==DUEL_NO_SLOT){finish_choice(d);return DUEL_OK;}if(d->choice.kind==DUEL_CHOICE_RETARGET_MODE){d->choice.kind=DUEL_CHOICE_RETARGET_MONSTER;d->choice.selected_count=token;return DUEL_OK;}DuelCommand c=d->equip_mode_command;c.cost=d->choice.selected_count;c.target_player=token/5;c.target=token%5;finish_choice(d);return activate(d,&c,c.action==DUEL_ACTION_ACTIVATE_BACKROW);}
void duel_retarget_resolve(Duel *d,const DuelChainLink *l){if(l->effect_negated)return;unsigned name=d->cards[l->card].name_group;if(name==1317&&l->command.cost==2){if(!duel_target_present(d,l->command.target_player,l->command.target,l->target_serial)||d->window.kind!=DUEL_WINDOW_ATTACK||d->window.negated)return;d->window.target=l->command.target;d->window.target_player_plus1=l->command.target_player+1;d->window.target_serial=l->target_serial;d->window.events|=BATTLE_REDIRECT_FORCED|BATTLE_REDIRECT_SEEN;return;}unsigned at=l->command.tributes;if(!at||at>=d->chain_count)return;DuelChainLink *old=&d->chain[at-1];if(name==1247){old->player_redirect^=1;return;}if(!duel_target_present(d,l->command.target_player,l->command.target,l->target_serial))return;old->command.target_player=l->command.target_player;old->command.target=l->command.target;old->target_serial=l->target_serial;old->target_card=d->players[l->command.target_player].monsters[l->command.target].card;if(old->target_count)old->targets[0]=(DuelTarget){l->target_serial,l->command.target_player,l->command.target};}
static int retarget_ai(const Duel *d,unsigned actor,unsigned card){if(d->cards[card].name_group!=1247)return duel_retarget_modes(d,d->cards[card].name_group,actor)!=0;if(!refpanel_ready(d,actor))return 0;const DuelChainLink *l=retarget_previous(d);unsigned name=d->cards[l->card].name_group;if(d->cards[l->card].effect==DUEL_E_LIFE){const DuelLifeRule *r=duel_life_rule(d,l->card);unsigned target=r->target==DUEL_LIFE_OPPONENT?1-l->command.player:r->target==DUEL_LIFE_EITHER?l->command.target_player:l->command.player;target=link_player(l,target);return r->operation==DUEL_LIFE_DAMAGE?target==actor:target!=actor;}if(name==1010||name==968)return link_player(l,l->command.player)!=actor;return link_player(l,1-l->command.player)==actor;}
