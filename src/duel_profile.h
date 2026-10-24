#ifndef JOEY_NATIVE_PROFILE_H
#define JOEY_NATIVE_PROFILE_H
#include "duel.h"

#define JOEY_PROFILE_SAVE_BYTES 1377u
#define JOEY_PROFILE_V3_BYTES 1345u
#define JOEY_PROFILE_V2_BYTES 1333u
#define JOEY_SIDE_MAX 15u
enum JoeyPile {JOEY_MAIN,JOEY_FUSION,JOEY_SIDE};
enum JoeyRewardFlags {JOEY_REWARD_PENDING=1,JOEY_REWARD_REROLL=2,JOEY_REWARD_NEW=4};
typedef struct {
    uint32_t sequence;
    uint16_t main[DUEL_DECK_MAX],fusion[DUEL_FUSION_MAX];
    uint16_t main_count,fusion_count;
    uint8_t owned[DUEL_CARD_COUNT];
    uint32_t wins,losses;
    uint16_t reward_card;
    uint8_t reward_gauge,reward_flags;
    uint16_t side[JOEY_SIDE_MAX],side_count;
} JoeyProfile;
enum JoeyProfileResult {JOEY_PROFILE_OK,JOEY_PROFILE_SIZE,JOEY_PROFILE_CARD,JOEY_PROFILE_UNSUPPORTED,JOEY_PROFILE_OWNED,JOEY_PROFILE_LIMIT,JOEY_PROFILE_FORMAT,JOEY_PROFILE_PENDING};
typedef uint32_t (*JoeyRewardRandom)(void *);

void joey_profile_init(JoeyProfile *,const uint8_t *,const uint16_t *,unsigned,const uint16_t *,unsigned);
int joey_profile_validate(const JoeyProfile *,const DuelCardDef *,const uint8_t *,unsigned allow_short);
int joey_profile_change(JoeyProfile *,const DuelCardDef *,const uint8_t *,unsigned card,int add);
int joey_profile_change_pile(JoeyProfile *,const DuelCardDef *,const uint8_t *,unsigned card,int add,unsigned pile);
unsigned joey_profile_count(const JoeyProfile *,unsigned card);
int joey_profile_encode(const JoeyProfile *,const DuelCardDef *,const uint8_t *,uint8_t *,unsigned);
int joey_profile_decode(JoeyProfile *,const DuelCardDef *,const uint8_t *,const uint8_t *,unsigned);
int joey_profile_newest(const JoeyProfile *,int valid_a,const JoeyProfile *,int valid_b);
int joey_profile_finish(JoeyProfile *,unsigned won,const uint16_t *pool,unsigned count,JoeyRewardRandom,void *);
int joey_profile_reroll(JoeyProfile *,const uint16_t *pool,unsigned count,JoeyRewardRandom,void *);
int joey_profile_claim(JoeyProfile *);
#endif
