typedef struct {uint16_t name;uint8_t trigger,filter;uint16_t selector;uint8_t maximum;} SearchRule;
static const SearchRule search_rules[]={
{1109,1,1,3,1},
{1122,1,2,3,1},
{1240,1,3,746,1},
{1040,0,3,1003,1},
{424,2,3,424,2},
};
#define DUEL_SEARCH_RULE_COUNT 5u
