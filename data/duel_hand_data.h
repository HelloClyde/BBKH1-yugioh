typedef struct {uint16_t name;uint8_t op;uint16_t cost;} HandRule;
static const HandRule hand_rules[]={
{1071,1,1000},
{1070,2,1000},
{1077,3,0},
{1025,4,0},
{1067,5,0},
{533,6,1000},
{462,7,2},
};
#define DUEL_HAND_RULE_COUNT 7u
