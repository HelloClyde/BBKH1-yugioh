typedef struct {uint16_t name;uint8_t count,attribute;} SpecialHandRule;
static const SpecialHandRule special_hand_rules[]={
{1520,252,5},
{1350,0,2},
{1515,2,1},
{1516,1,4},
{1518,1,5},
{1517,1,3},
{1519,1,6},
{726,254,4},
{727,254,3},
{728,254,2},
{766,254,1},
{373,253,2},
{55,251,5},
{56,251,5},
{66,251,5},
{1257,250,2},
{1514,3,2},
{845,255,5},
};
#define DUEL_SPECIAL_HAND_COUNT 18u
typedef struct {uint16_t source,materials[3];} ContactRecipe;
static const ContactRecipe contact_recipes[]={
{845,{737,756,800}},
};
#define DUEL_CONTACT_RECIPE_COUNT 1u
