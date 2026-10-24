#ifndef JOEY_DUEL_OPENING_H
#define JOEY_DUEL_OPENING_H
#include <stdint.h>
enum DuelOpeningPhase {DUEL_OPEN_CHOOSE,DUEL_OPEN_DRAW,DUEL_OPEN_PLAYER_ORDER,DUEL_OPEN_NPC_ORDER,DUEL_OPEN_READY};
enum DuelOpeningResult {DUEL_OPEN_RESULT_DRAW,DUEL_OPEN_RESULT_WIN,DUEL_OPEN_RESULT_LOSS};
typedef uint32_t (*DuelOpeningRandom)(void *);
typedef struct {uint32_t rounds;uint8_t phase,human,enemy,outcome,first,npc_first;} DuelOpening;
void duel_opening_begin(DuelOpening *);
int duel_opening_throw(DuelOpening *,unsigned hand,DuelOpeningRandom,void *);
int duel_opening_continue(DuelOpening *);
int duel_opening_order(DuelOpening *,unsigned first);
#endif
