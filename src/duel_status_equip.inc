#include "duel_status_equip_data.h"
static unsigned status_equip_flags(const Duel *d,unsigned card){unsigned name=d->cards[card].name_group;return name<DUEL_STATUS_EQUIP_NAME_LIMIT?status_equip_by_name[name]:0;}
/* Return source-controller bits, so controller-relative effects remain
 * distinct even when several copies attach to the same monster. */
static unsigned equipment_status_summary(const Duel *d,unsigned side,unsigned index,unsigned flag){const DuelSlot *host=&d->players[side].monsters[index];if(!host->card||!host->serial||host->position==DUEL_FACEDOWN_DEFENSE)return 0;unsigned result=0;for(unsigned owner=0;owner<2;++owner)for(unsigned i=0;i<5;++i){const DuelSlot *s=&d->players[owner].backrow[i];if(s->card&&s->effect_active&&s->position!=DUEL_FACEDOWN_DEFENSE&&s->link_serial==host->serial&&(status_equip_flags(d,s->card)&flag)&&equip_eligible(d,s->card,side,index)&&!equip_effect_negated(d,s->card)&&effect_affects_monster(d,s->card,side,index))result=(result+4)|(1u<<owner);}return result;}
static unsigned equipment_status_mask(const Duel *d,unsigned side,unsigned index,unsigned flag){return equipment_status_summary(d,side,index,flag)&3;}
static unsigned equipment_status_count(const Duel *d,unsigned side,unsigned index,unsigned flag){return equipment_status_summary(d,side,index,flag)>>2;}
