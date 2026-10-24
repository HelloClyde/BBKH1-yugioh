/* The source counter is advanced on its host controller's End, even while
 * negated. Original old-age1 is the only destruction attempt. Temporarily
 * detaching the resolving source keeps the target's departure from sending
 * that same source to the grave before it can return to its owner's hand. */
static void equipment_end(Duel *d,unsigned side){
    for(unsigned order=0;order<2;++order)for(unsigned i=0;i<5;++i){unsigned owner=side^order;DuelSlot *s=&d->players[owner].backrow[i];
        if(!s->card||d->cards[s->card].name_group!=1548||!s->effect_active||s->position==DUEL_FACEDOWN_DEFENSE)continue;
        unsigned host=0,index=0;if(!equip_host(d,s->card,s->link_serial,&host,&index)||host!=side)continue;
        unsigned age=s->control_used;if(age<15)++s->control_used;
        if(age!=1||equip_effect_negated(d,s->card)||!effect_affects_monster(d,s->card,host,index))continue;
        DuelSlot saved=*s;clear(s,sizeof(*s));destroy(d,host,index,0);*s=saved;
        move_backrows_mask(d,1u<<(owner*6+i),1,0);
    }
}
