typedef struct {uint16_t name;uint8_t op,tributes;uint16_t amount;} IgnitionRule;
static const IgnitionRule ignition_rules[]={
{1430,15,0,0},
{1433,15,0,0},
{1439,15,0,0},
{1444,15,0,0},
{416,14,1,1000},
{511,1,1,500},
{1432,1,2,1200},
{88,2,1,0},
{261,3,1,700},
{1442,4,2,1000},
{742,5,0,0},
{428,6,0,0},
{1112,7,0,0},
{401,8,0,1000},
{505,9,0,3000},
{419,10,0,5000},
{1513,11,0,300},
{1510,12,0,0},
{15,13,0,0},
{730,16,0,0},
{1334,16,0,0},
{845,17,0,0},
{579,18,0,0},
};
#define DUEL_IGNITION_RULE_COUNT 23u
static const uint16_t monster_eye_grave_records[]={1003,1034};
#define DUEL_MONSTER_EYE_TARGET_COUNT 2u
