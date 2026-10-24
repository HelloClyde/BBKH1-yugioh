#include "duel_target_flip_data.h"
static const TargetFlipRule *target_flip_rule(const Duel *d,unsigned card){for(unsigned i=0;i<DUEL_TARGET_FLIP_RULE_COUNT;++i)if(target_flip_rules[i].name==d->cards[card].name_group)return &target_flip_rules[i];return 0;}
/* Face visibility and orientation are independent. Store the rare hidden
 * attack orientation in existing alignment padding; the64-byte slot and all
 * ordinary face-down legality queries keep their existing representation. */
unsigned duel_monster_position(const Duel *d,unsigned side,unsigned index){if(!d||side>1||index>=5)return DUEL_FACEUP_ATTACK;const DuelSlot *s=&d->players[side].monsters[index];return s->position==DUEL_FACEDOWN_DEFENSE&&s->facedown_attack?DUEL_FACEDOWN_ATTACK:s->position;}
void duel_change_monster_position(Duel *d,unsigned side,unsigned index,unsigned position){
    if(!d||side>1||index>=5||position>DUEL_FACEDOWN_ATTACK)return;DuelSlot *s=&d->players[side].monsters[index];unsigned old=s->position,old_hidden_attack=s->facedown_attack,hidden_attack=position==DUEL_FACEDOWN_ATTACK;
    if(hidden_attack)position=DUEL_FACEDOWN_DEFENSE;
    if(position==DUEL_FACEDOWN_DEFENSE&&old!=DUEL_FACEDOWN_DEFENSE&&live_backrow_name(d,1151,3)){position=old==DUEL_FACEUP_ATTACK?DUEL_FACEUP_DEFENSE:old;hidden_attack=0;}
    if(position==DUEL_FACEDOWN_DEFENSE){if(old!=position){s->temporary_bonus=0;s->temporary_attack=s->temporary_defense=0;s->lasting_attack=0;s->lasting_attack_until=0;s->original_attack_active=0;s->original_attack=0;s->original_until=0;s->swap_until=0;s->swap_parity=0;if(s->card&&d->cards[s->card].effect==DUEL_E_ZOMBYRA)s->attack_bonus=0;fairy_target_hidden(d,s->serial);binding_position_reset(d,s->serial);battle_countdown_reset(d,s->serial);s->destroy_end_turn=0;monster_equipment_new_deployment(d,s->serial);if(s->card){switch(d->cards[s->card].name_group){case 15:case 98:case 370:case 371:case 372:case 428:case 461:case 579:case 730:case 742:case 1112:case 1116:case 1254:case 1334:case 1344:case 1513:s->ignition_turn=0;break;}}}if(!s->card||d->cards[s->card].name_group!=761)s->control_used=0;if(s->card&&(d->cards[s->card].name_group==82||d->cards[s->card].name_group==98))s->hourglass_turn=0;s->battle_curse=0;}
    if(position==DUEL_FACEUP_ATTACK&&!continuous_position_allowed(d,side,index,position))position=DUEL_FACEUP_DEFENSE;
    s->position=position;s->facedown_attack=position==DUEL_FACEDOWN_DEFENSE&&hidden_attack;
    if(s->card&&old!=DUEL_FACEDOWN_DEFENSE&&position==DUEL_FACEDOWN_DEFENSE&&!hidden_attack&&d->resolving)d->summon_notice=(DuelWindow){DUEL_WINDOW_SET,side,index,0,s->serial,0,s->card};
    if(!s->card||old==position||position==DUEL_FACEDOWN_DEFENSE||d->cards[s->card].effect!=DUEL_E_TARGET_FLIP)return;const TargetFlipRule *r=target_flip_rule(d,s->card);if(!r||!(r->op>=6&&r->op<=8||r->op==12))return;unsigned was_defense=old==DUEL_FACEUP_DEFENSE||old==DUEL_FACEDOWN_DEFENSE&&!old_hidden_attack;if(r->op==7?(!was_defense||position!=DUEL_FACEUP_ATTACK):(was_defense||position!=DUEL_FACEUP_DEFENSE))return;if(!s->serial)s->serial=++d->next_serial;queue_trigger(d,r->op==8||r->op==12?DUEL_TRIGGER_POSITION_AUTO:DUEL_CHOICE_TARGET_FLIP,side,s->card,s->serial);
}

/* Tokens preserve the original distinction between monsters, backrow and
 * the field zone: side * 11 + [monster 0..4, backrow 5..9, field 10]. */
static DuelSlot *target_flip_slot(Duel *d,unsigned side,unsigned index){return index<5?&d->players[side].monsters[index]:spell_zone(&d->players[side],index-5);}
static unsigned target_flip_options(const Duel *d,DuelChoiceOption *options,unsigned capacity){const TargetFlipRule *r=target_flip_rule(d,d->choice.card);if(!r)return 0;if(r->op==11){unsigned alive=0;for(unsigned i=0;i<5;++i){const DuelSlot *source=&d->players[d->choice.player].monsters[i];alive|=source->card==d->choice.card&&source->serial==d->choice.serial&&source->position!=DUEL_FACEDOWN_DEFENSE;}if(!alive)return 0;}unsigned count=0;
    for(unsigned side=0;side<2;++side)for(unsigned index=0;index<11;++index){const DuelSlot *s=index<5?&d->players[side].monsters[index]:index==10?&d->players[side].field:&d->players[side].backrow[index-5];if(!s->card||(index<5&&!duel_can_target_monster(d,side,index)))continue;
        if(r->op==13){if(index>=5||s->position==DUEL_FACEDOWN_DEFENSE||duel_race(d,side,index)!=1)continue;}
        else if(r->op==14){if(side==d->choice.player||index>=5||s->position==DUEL_FACEDOWN_DEFENSE)continue;}
        else if(r->op==15||r->op==16){if(side==d->choice.player||index>=5)continue;}
        else if(r->op==11){if(side==d->choice.player||index>=5)continue;}
        else if(r->op==10){if(side==d->choice.player||index>=5||s->position==DUEL_FACEDOWN_DEFENSE||duel_race(d,side,index)!=7)continue;}
        else if(r->op==9){if(side==d->choice.player||index>=5||s->position!=DUEL_FACEDOWN_DEFENSE||battle_destroyed(d,side,index))continue;}
        else if(r->op==6||r->op==7){if(side==d->choice.player||index>=5||battle_destroyed(d,side,index))continue;}
        else if(r->op==1){if(index>=5||s->position==DUEL_FACEDOWN_DEFENSE||battle_destroyed(d,side,index))continue;}
        else {if(index<5)continue;if(r->op==3&&(index==10||d->cards[s->card].family==DUEL_MONSTER))continue;if(r->op==4&&s->position!=DUEL_FACEDOWN_DEFENSE)continue;
            if(s->position!=DUEL_FACEDOWN_DEFENSE&&((r->op==2&&d->cards[s->card].family==DUEL_TRAP)||(r->op==3&&d->cards[s->card].family!=DUEL_TRAP)))continue;
            if(r->op==5&&d->players[s->owner].hand_count>=DUEL_HAND_MAX)continue;}
        unsigned selected=0;for(unsigned j=0;j<d->choice.selected_count;++j)selected|=d->choice_targets[j].player==side&&d->choice_targets[j].slot==index;if(selected)continue;
        if(options&&count<capacity)options[count]=(DuelChoiceOption){side!=d->choice.player&&s->position==DUEL_FACEDOWN_DEFENSE?0:s->card,side*11+index,DUEL_OPTION_CARD};++count;
    }return count+d->choice.selected_count>=r->count?count:0;
}
static void target_flip_resolve(Duel *d,const DuelChainLink *link){const TargetFlipRule *r=target_flip_rule(d,link->card);if(!r)return;unsigned mask=0;
    if(r->op==12){yado_begin(d,link);return;}
    if(r->op==8){shuffle(d,&d->players[link->command.player]);return;}
    /* Snapshot identities were captured before either player could respond. */
    for(unsigned i=0;i<link->target_count;++i){const DuelTarget *t=&link->targets[i];DuelSlot *s=target_flip_slot(d,t->player,t->slot);if(!s->card||s->serial!=t->serial)continue;
        if(r->op>=13)arrival_target_resolve(d,link,t,r->op);
        else if(r->op==11)control_exchange(d,link,t);
        else if(r->op==10){if(t->player!=link->command.player&&s->position!=DUEL_FACEDOWN_DEFENSE&&duel_race(d,t->player,t->slot)==7)control_add(d,t->player,t->slot,link->command.player,link->card,0,d->turn+(d->active==link->command.player?2:1));}
        else if(r->op==9){if(s->position==DUEL_FACEDOWN_DEFENSE&&!battle_destroyed(d,t->player,t->slot)){if(duel_original_classification[s->card]&1)destroy(d,t->player,t->slot,0);else event(d,DUEL_EVENT_SPELL,t->player,t->slot,s->card,0);}}
        else if(r->op==6){if(!battle_destroyed(d,t->player,t->slot))destroy(d,t->player,t->slot,0);}
        else if(r->op==7)return_monster(d,t);
        else if(r->op==1){if(s->position!=DUEL_FACEDOWN_DEFENSE&&!battle_destroyed(d,t->player,t->slot))duel_change_monster_position(d,t->player,t->slot,s->position==DUEL_FACEUP_ATTACK?DUEL_FACEUP_DEFENSE:DUEL_FACEUP_ATTACK);}
        else if(r->op==5||(r->op==2&&d->cards[s->card].family!=DUEL_TRAP)||(r->op==3&&d->cards[s->card].family==DUEL_TRAP)||(r->op==4&&s->position==DUEL_FACEDOWN_DEFENSE))mask|=1u<<(t->player*6+t->slot-5);
        else event(d,DUEL_EVENT_SPELL,t->player,t->slot,s->card,0); /* Revealed wrong-family card remains set. */
    }if(mask)move_backrows_mask(d,mask,r->op==5,0);
}
