#include "duel_profile.h"
#include "duel_cards.h"
#include "original_reward_cases.h"
/* Synthetic v2 migration input, generated below from the public defaults.
 * No personal save data is required by this test suite. */
static uint8_t profile_v2_fixture[JOEY_PROFILE_V2_BYTES];

volatile uint32_t native_profile_test_result[4];
static JoeyProfile a,b,c;
static DuelCardDef profile_unsupported_cards[DUEL_CARD_COUNT];
static uint8_t wire[JOEY_PROFILE_SAVE_BYTES];
static void check(int condition,unsigned line){unsigned n=++native_profile_test_result[0];if(!condition){++native_profile_test_result[1];if(!native_profile_test_result[2]){native_profile_test_result[2]=n;native_profile_test_result[3]=line;}}}
#define CHECK(c) check((c),__LINE__)
static int equal(const void *x,const void *y,unsigned n){const uint8_t *a=x,*b=y;while(n--)if(*a++!=*b++)return 0;return 1;}
static unsigned random_calls;
static uint32_t reward_random(void *context){static const uint32_t values[]={0,1,349,9};return values[random_calls++%4];}
#include "native_profile_reserved_test.inc"
int native_profile_test_run(void){
    profile_reserved_metadata_tests();
    joey_profile_init(&a,duel_original_owned,duel_original_deck,DUEL_ORIGINAL_DECK_COUNT,duel_original_fusion_deck,DUEL_ORIGINAL_FUSION_DECK_COUNT);
    CHECK(joey_profile_validate(&a,duel_card_definitions,duel_original_limits,0)==0);CHECK(a.main_count==42&&a.fusion_count==1);CHECK(a.owned[300]==17&&a.owned[1026]==16);
    b=a;b.sequence=1;CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0);
    CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0);CHECK(equal(&b,&c,sizeof(b)));
    for(unsigned i=0;i<sizeof(wire);++i){c=a;CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,i)==JOEY_PROFILE_FORMAT);CHECK(equal(&a,&c,sizeof(a)));}
    for(unsigned i=0;i<sizeof(wire);i+=37){wire[i]^=1;c=a;CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==JOEY_PROFILE_FORMAT);CHECK(equal(&a,&c,sizeof(a)));wire[i]^=1;}
    CHECK(joey_profile_encode(&a,duel_card_definitions,duel_original_limits,wire,sizeof(wire)-1)==JOEY_PROFILE_FORMAT);
    b=a;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,834,1)==0);CHECK(joey_profile_count(&b,834)==3);c=b;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,834,1)==JOEY_PROFILE_LIMIT);CHECK(equal(&b,&c,sizeof(b)));
    b=a;b.owned[24]=2;c=b;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,24,1)==JOEY_PROFILE_OWNED);CHECK(equal(&b,&c,sizeof(b)));
    b=a;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,651,1)==0);CHECK(b.fusion_count==2&&b.main_count==42);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,651,0)==0);CHECK(b.fusion_count==1);
    b=a;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,24,0)==0);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,24,0)==0);CHECK(b.main_count==40);CHECK(joey_profile_validate(&b,duel_card_definitions,duel_original_limits,0)==0);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,823,0)==0);CHECK(b.main_count==39);CHECK(joey_profile_validate(&b,duel_card_definitions,duel_original_limits,1)==0);CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==JOEY_PROFILE_SIZE);
    b=a;c=b;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,0,1)==JOEY_PROFILE_CARD);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,1115,1)==JOEY_PROFILE_CARD);CHECK(equal(&b,&c,sizeof(b)));
    /* An explicit fixture keeps this rejection test meaningful once every
       original playable effect has been implemented. */
    unsigned unsupported=0;for(unsigned card=1;card<DUEL_CARD_COUNT;++card){profile_unsupported_cards[card]=duel_card_definitions[card];if(!unsupported&&duel_card_definitions[card].race<23&&duel_card_supported(&duel_card_definitions[card])&&!joey_profile_count(&a,card))unsupported=card;}
    CHECK(unsupported>0&&unsupported<DUEL_CARD_COUNT);profile_unsupported_cards[unsupported].effect=DUEL_E_UNSUPPORTED;b=a;b.owned[unsupported]=3;c=b;CHECK(joey_profile_change(&b,profile_unsupported_cards,duel_original_limits,unsupported,1)==JOEY_PROFILE_UNSUPPORTED);CHECK(equal(&b,&c,sizeof(b)));
    b=a;b.main[0]=651;CHECK(joey_profile_validate(&b,duel_card_definitions,duel_original_limits,0)==JOEY_PROFILE_CARD);b=a;b.fusion[0]=834;CHECK(joey_profile_validate(&b,duel_card_definitions,duel_original_limits,0)==JOEY_PROFILE_CARD);b=a;b.main[0]=65535;CHECK(joey_profile_validate(&b,duel_card_definitions,duel_original_limits,0)==JOEY_PROFILE_CARD);b=a;b.main_count=65535;CHECK(joey_profile_validate(&b,duel_card_definitions,duel_original_limits,0)==JOEY_PROFILE_SIZE);
    // Alternate artwork shares the same original name and aggregate quota.
    b=a;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,816,1)==0);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,817,1)==0);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,817,1)==0);c=b;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,816,1)==JOEY_PROFILE_LIMIT);CHECK(equal(&b,&c,sizeof(b)));
    b=a;c=b;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,359,1)==JOEY_PROFILE_LIMIT);CHECK(equal(&b,&c,sizeof(b)));
    // Native opening consumes the edited main and extra decks, not defaults.
    static Duel duel;b=a;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,24,0)==0);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,61,1)==0);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,651,1)==0);
    CHECK(duel_start(&duel,duel_card_definitions,b.main,b.main_count,duel_original_deck,DUEL_ORIGINAL_DECK_COUNT,12345,0)==0);CHECK(duel_fusion_setup(&duel,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,b.fusion,b.fusion_count,duel_original_fusion_deck,DUEL_ORIGINAL_FUSION_DECK_COUNT)==0);
    for(unsigned card=1;card<DUEL_CARD_COUNT;++card){unsigned found=0;DuelPlayer *p=&duel.players[0];for(unsigned j=0;j<p->deck_count;++j)found+=p->deck[j]==card;for(unsigned j=0;j<p->hand_count;++j)found+=p->hand[j]==card;for(unsigned j=0;j<p->fusion_count;++j)found+=p->fusion[j]==card;CHECK(found==joey_profile_count(&b,card));}
    // The PC original allows 80 main and 15 Fusion cards. Fill only through
    // normal validated edits, preserving ownership and aggregate name quotas.
    b=a;
    for(unsigned card=1;card<DUEL_CARD_COUNT&&b.main_count<80;++card)if(duel_card_definitions[card].subtype!=2||duel_card_definitions[card].family!=DUEL_MONSTER)for(unsigned n=0;n<3&&b.main_count<80;++n)if(joey_profile_change(&b,duel_card_definitions,duel_original_limits,card,1))break;
    for(unsigned card=1;card<DUEL_CARD_COUNT&&b.fusion_count<15;++card)if(duel_card_definitions[card].subtype==2&&duel_card_definitions[card].family==DUEL_MONSTER)for(unsigned n=0;n<3&&b.fusion_count<15;++n)if(joey_profile_change(&b,duel_card_definitions,duel_original_limits,card,1))break;
    CHECK(b.main_count==80&&b.fusion_count==15);c=b;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,24,1)==JOEY_PROFILE_SIZE);CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,651,1)==JOEY_PROFILE_SIZE);CHECK(equal(&b,&c,sizeof(b)));
    CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0);CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0&&equal(&b,&c,sizeof(b)));
    CHECK(duel_start(&duel,duel_card_definitions,b.main,80,duel_original_deck,42,12345,0)==0&&duel.players[0].deck_count==74);CHECK(duel_fusion_setup(&duel,duel_fusion_recipes,DUEL_FUSION_RECIPE_COUNT,b.fusion,15,duel_original_fusion_deck,1)==0&&duel.players[0].fusion_count==15);
    CHECK(duel_start(&duel,duel_card_definitions,b.main,81,duel_original_deck,42,12345,0)==DUEL_BAD_ARGUMENT);
    b=a;b.sequence=2;CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0);
    for(unsigned i=0;i<sizeof(profile_v2_fixture);++i)profile_v2_fixture[i]=wire[i];
    profile_v2_fixture[7]='2';profile_v2_fixture[8]=2;unsigned v2_size=sizeof(profile_v2_fixture);
    profile_v2_fixture[16]=v2_size;profile_v2_fixture[17]=v2_size>>8;profile_v2_fixture[18]=profile_v2_fixture[19]=0;
    for(unsigned i=20;i<24;++i)profile_v2_fixture[i]=0;
    uint32_t v2_crc=~0u;for(unsigned i=0;i<v2_size;++i){v2_crc^=profile_v2_fixture[i];for(unsigned j=0;j<8;++j)v2_crc=(v2_crc>>1)^((0u-(v2_crc&1u))&0xedb88320u);}v2_crc=~v2_crc;
    for(unsigned i=0;i<4;++i)profile_v2_fixture[20+i]=v2_crc>>(8*i);
    CHECK(joey_profile_decode(&b,duel_card_definitions,duel_original_limits,profile_v2_fixture,sizeof(profile_v2_fixture))==0&&b.sequence==2&&b.main_count==42&&b.fusion_count==1&&b.wins==0&&b.losses==0&&b.reward_flags==0);
    CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0&&wire[7]=='4');CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0&&equal(&b,&c,sizeof(b)));
    /* A side card is reserved against ownership and same-name quotas, saved
       independently, and excluded from the actual duel's draw pile. */
    b=a;CHECK(joey_profile_change_pile(&b,duel_card_definitions,duel_original_limits,61,1,JOEY_SIDE)==0);CHECK(b.main_count==42&&b.fusion_count==1&&b.side_count==1&&b.side[0]==61);
    CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0);CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0&&equal(&b,&c,sizeof(b)));
    CHECK(duel_start(&duel,duel_card_definitions,c.main,c.main_count,duel_original_deck,42,12345,0)==0);unsigned found=0;for(unsigned i=0;i<duel.players[0].deck_count;++i)found+=duel.players[0].deck[i]==61;for(unsigned i=0;i<duel.players[0].hand_count;++i)found+=duel.players[0].hand[i]==61;CHECK(found==0);
    CHECK(joey_profile_change_pile(&b,duel_card_definitions,duel_original_limits,61,0,JOEY_SIDE)==0&&b.side_count==0);
    b=a;CHECK(joey_profile_change_pile(&b,duel_card_definitions,duel_original_limits,834,1,JOEY_SIDE)==0);c=b;CHECK(joey_profile_change(&b,duel_card_definitions,duel_original_limits,834,1)==JOEY_PROFILE_LIMIT&&equal(&b,&c,sizeof(b)));
    b=a;b.owned[24]=2;c=b;CHECK(joey_profile_change_pile(&b,duel_card_definitions,duel_original_limits,24,1,JOEY_SIDE)==JOEY_PROFILE_OWNED&&equal(&b,&c,sizeof(b)));
    b=a;for(unsigned card=1;card<DUEL_CARD_COUNT&&b.side_count<15;++card)for(unsigned n=0;n<3&&b.side_count<15;++n)if(joey_profile_change_pile(&b,duel_card_definitions,duel_original_limits,card,1,JOEY_SIDE))break;
    CHECK(b.side_count==15);c=b;CHECK(joey_profile_change_pile(&b,duel_card_definitions,duel_original_limits,61,1,JOEY_SIDE)==JOEY_PROFILE_SIZE&&equal(&b,&c,sizeof(b)));
    CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0);CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0&&equal(&b,&c,sizeof(b)));
    /* Recreate the previous v3 envelope, preserving all old fields and CRC. */
    b=a;CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0);wire[7]='3';wire[8]=3;unsigned old_size=JOEY_PROFILE_V3_BYTES;wire[16]=old_size;wire[17]=old_size>>8;wire[18]=wire[19]=0;for(unsigned i=20;i<24;++i)wire[i]=0;
    uint32_t crc=~0u;for(unsigned i=0;i<old_size;++i){crc^=wire[i];for(unsigned j=0;j<8;++j)crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u);}crc=~crc;for(unsigned i=0;i<4;++i)wire[20+i]=crc>>(8*i);
    CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,old_size)==0&&equal(&b,&c,sizeof(b))&&c.side_count==0);
    for(unsigned f=0;f<sizeof(original_reward_cases)/sizeof(original_reward_cases[0]);++f){
        b=a;for(unsigned i=0;i<DUEL_CARD_COUNT;++i)b.owned[i]=0;for(unsigned i=0;i<original_reward_cases[f].owned;++i)b.owned[duel_reward_pool[i]]=1;
        b.reward_gauge=original_reward_cases[f].gauge;b.reward_card=original_reward_cases[f].replacement?duel_reward_pool[0]:0;b.reward_flags=original_reward_cases[f].replacement?JOEY_REWARD_PENDING|JOEY_REWARD_REROLL:0;random_calls=0;
        int result=original_reward_cases[f].replacement?joey_profile_reroll(&b,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0):joey_profile_finish(&b,1,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0);
        CHECK(result==0);CHECK(b.reward_card==original_reward_cases[f].card);CHECK(!!(b.reward_flags&JOEY_REWARD_NEW)==original_reward_cases[f].is_new);CHECK(b.reward_gauge==original_reward_cases[f].progress);CHECK(!!(b.reward_flags&JOEY_REWARD_REROLL)==original_reward_cases[f].reroll);CHECK(random_calls==original_reward_cases[f].calls);CHECK(b.wins==(original_reward_cases[f].replacement?0:1));
    }
    b=a;random_calls=0;CHECK(joey_profile_finish(&b,1,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0)==0&&b.wins==1);c=b;CHECK(joey_profile_finish(&b,1,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0)==JOEY_PROFILE_PENDING&&equal(&b,&c,sizeof(b)));
    CHECK(joey_profile_encode(&b,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0);CHECK(joey_profile_decode(&c,duel_card_definitions,duel_original_limits,wire,sizeof(wire))==0&&equal(&b,&c,sizeof(b)));unsigned prize=b.reward_card,before=b.owned[prize];CHECK(joey_profile_claim(&b)==0&&b.owned[prize]==before+1&&b.reward_flags==0&&b.reward_card==0&&b.wins==1);c=b;CHECK(joey_profile_claim(&b)==JOEY_PROFILE_PENDING&&equal(&b,&c,sizeof(b)));
    b=a;random_calls=0;CHECK(joey_profile_finish(&b,0,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0)==0&&b.losses==1&&b.wins==0&&b.reward_flags==0&&random_calls==0);b.losses=0xffffffffu;CHECK(joey_profile_finish(&b,0,0,0,0,0)==0&&b.losses==0xffffffffu);
    b=a;b.wins=0xffffffffu;b.reward_gauge=19;random_calls=0;CHECK(joey_profile_finish(&b,1,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0)==0&&b.wins==0xffffffffu&&(b.reward_flags&JOEY_REWARD_REROLL));random_calls=0;CHECK(joey_profile_reroll(&b,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0)==0&&!(b.reward_flags&JOEY_REWARD_REROLL)&&b.reward_gauge==0&&random_calls==3);c=b;CHECK(joey_profile_reroll(&b,duel_reward_pool,DUEL_REWARD_POOL_COUNT,reward_random,0)==JOEY_PROFILE_PENDING&&equal(&b,&c,sizeof(b)));
    b.owned[b.reward_card]=255;CHECK(joey_profile_claim(&b)==0&&b.owned[prize]==255);b=a;b.reward_gauge=20;CHECK(joey_profile_validate(&b,duel_card_definitions,duel_original_limits,0)==JOEY_PROFILE_FORMAT);b=a;b.reward_flags=JOEY_REWARD_REROLL;CHECK(joey_profile_validate(&b,duel_card_definitions,duel_original_limits,0)==JOEY_PROFILE_FORMAT);
    b=a;c=b;CHECK(joey_profile_finish(&b,1,0,0,reward_random,0)==JOEY_PROFILE_CARD&&equal(&b,&c,sizeof(b)));
    /* Prize transport is independent of effect availability. Mark a real
       prize unsupported in isolated metadata instead of assuming a missing card. */
    uint16_t single_prize=duel_reward_pool[0];CHECK(single_prize>0&&single_prize<DUEL_CARD_COUNT);profile_unsupported_cards[single_prize].effect=DUEL_E_UNSUPPORTED;CHECK(!duel_card_supported(&profile_unsupported_cards[single_prize]));b=a;random_calls=0;before=b.owned[single_prize];CHECK(joey_profile_finish(&b,1,&single_prize,1,reward_random,0)==0&&b.reward_card==single_prize);CHECK(joey_profile_claim(&b)==0&&b.owned[single_prize]==before+1);
    a.sequence=4;b.sequence=5;CHECK(joey_profile_newest(&a,1,&b,1)==1);CHECK(joey_profile_newest(&a,1,&b,0)==0);CHECK(joey_profile_newest(&a,0,&b,1)==1);CHECK(joey_profile_newest(&a,0,&b,0)==-1);b.sequence=4;CHECK(joey_profile_newest(&a,1,&b,1)==0);a.sequence=0xffffffffu;b.sequence=0;CHECK(joey_profile_newest(&a,1,&b,1)==1);CHECK(joey_profile_newest(&b,1,&a,1)==0);
    return native_profile_test_result[1]!=0;
}
