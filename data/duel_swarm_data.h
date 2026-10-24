typedef struct {uint16_t name;uint8_t trigger;int16_t life;uint8_t limit,position;} SwarmRule;
static const SwarmRule swarm_rules[]={
{1114,1,-500,2,0},
{1115,1,1000,2,2},
{1307,0,0,1,2},
};
#define DUEL_SWARM_RULE_COUNT 3u
