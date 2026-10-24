#include "original_monster_stat_cases.h"
#include "duel_monster_data.h"
#include "original_monster_battle_cases.h"
volatile uint32_t native_monster_test_result[4];
static void monster_tests(void){unsigned first=checks;
    for(unsigned i=0;i<sizeof(original_monster_stat_cases)/sizeof(original_monster_stat_cases[0]);++i){
        unsigned card=original_monster_stat_cases[i][0],side=original_monster_stat_cases[i][1],own=original_monster_stat_cases[i][2],enemy=original_monster_stat_cases[i][3];const MonsterStatRule *r=0;
        for(unsigned j=0;j<DUEL_MONSTER_STAT_COUNT;++j)if(monster_stat_rules[j].name==duel_card_definitions[card].name_group)r=&monster_stat_rules[j];check(r!=0);if(!r)continue;
        reset();test_game.turn=3;test_game.active=side;test_game.players[0].hand_count=test_game.players[1].hand_count=0;
        test_game.players[side].monsters[0]=slot(card,side,DUEL_FACEUP_ATTACK);test_game.players[side].monsters[0].serial=99;int equip_atk=0,equip_def=0;
        if(r->op==1||r->op==5){unsigned grave_card=r->op==1?24:simple_record(34);check(grave_card!=0);for(unsigned s=0;s<2;++s){unsigned n=s==side?own:enemy;for(unsigned j=0;j<n;++j)test_game.players[s].grave[test_game.players[s].grave_count++]=grave_card;test_game.players[s].grave[test_game.players[s].grave_count++]=316;}}
        if(r->op==2||r->op==6||r->op==7){unsigned extra=0;
            for(unsigned j=1;j<DUEL_CARD_COUNT;++j)if(duel_card_definitions[j].family==DUEL_MONSTER&&(r->op==2?duel_card_definitions[j].race==r->selector:duel_card_definitions[j].name_group==r->selector)){extra=j;break;}check(extra!=0);
            for(unsigned j=0;j<own;++j)test_game.players[side].monsters[j+1]=slot(extra,side,DUEL_FACEUP_ATTACK);
            for(unsigned j=0;j<enemy;++j)test_game.players[1-side].monsters[j]=slot(extra,1-side,DUEL_FACEUP_DEFENSE);
        }
        if(r->op==8||r->op==9)for(unsigned j=0;j<enemy;++j)test_game.players[1-side].monsters[j]=slot(24,1-side,j%3);
        if(r->op==3){test_game.players[side].hand_count=own;test_game.players[1-side].hand_count=enemy;for(unsigned s=0;s<2;++s)for(unsigned j=0;j<test_game.players[s].hand_count;++j)test_game.players[s].hand[j]=24;}
        if(r->op==4){const DuelEquipRule *equip=0;for(unsigned j=0;j<DUEL_EQUIP_RULE_COUNT;++j)if(duel_equip_can_target(&test_game,duel_equip_rules[j].card,side,0)){equip=&duel_equip_rules[j];break;}check(equip!=0);if(!equip)continue;
            for(unsigned j=0;j<own;++j){test_game.players[side].backrow[j]=slot(equip->card,side,DUEL_FACEUP_ATTACK);test_game.players[side].backrow[j].effect_active=1;test_game.players[side].backrow[j].link_serial=99;equip_atk+=equip->attack;equip_def+=equip->defense;}}
        int attack=duel_card_definitions[card].attack+original_monster_stat_cases[i][4]+equip_atk,defense=duel_card_definitions[card].defense+original_monster_stat_cases[i][5]+equip_def;
        if(attack<0)attack=0;if(defense<0)defense=0;check(duel_attack_value(&test_game,side,0)==attack);check(duel_defense_value(&test_game,side,0)==defense);
        test_game.players[side].monsters[0].position=DUEL_FACEDOWN_DEFENSE;check(duel_attack_value(&test_game,side,0)==duel_card_definitions[card].attack&&duel_defense_value(&test_game,side,0)==duel_card_definitions[card].defense);
    }
    for(unsigned i=0;i<sizeof(original_monster_direct_cases)/sizeof(original_monster_direct_cases[0]);++i){if(original_monster_direct_cases[i][3])continue;
        unsigned card=original_monster_direct_cases[i][0],side=original_monster_direct_cases[i][1],index=original_monster_direct_cases[i][2];reset();test_game.turn=3;test_game.active=side;test_game.phase=DUEL_BATTLE;
        test_game.players[side].monsters[index]=slot(card,side,DUEL_FACEUP_ATTACK);test_game.players[1-side].monsters[0]=slot(24,1-side,DUEL_FACEDOWN_DEFENSE);
        check(duel_can_attack_direct(&test_game,side,index)==original_monster_direct_cases[i][4]);
        if(original_monster_direct_cases[i][4]){check(command(DUEL_ACTION_ATTACK,index,1-side,DUEL_NO_SLOT,0,0)==DUEL_OK);check(test_game.players[1-side].life==8000-duel_card_definitions[card].attack&&test_game.players[1-side].monsters[0].card==24);}
    }
    for(unsigned side=0;side<2;++side){unsigned hayabusa=simple_record(1336);reset();test_game.turn=3;test_game.active=side;test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(hayabusa,side,DUEL_FACEUP_ATTACK);check(duel_attack_limit(&test_game,side,0)==2);
        check(command(DUEL_ACTION_ATTACK,0,1-side,DUEL_NO_SLOT,0,0)==DUEL_OK&&test_game.players[side].monsters[0].attacked==1);
        DuelCommand c;check(duel_ai_decide(&test_game,side,&c)&&c.action==DUEL_ACTION_ATTACK);check(duel_command(&test_game,&c)==DUEL_OK&&test_game.players[side].monsters[0].attacked==2);
        uint32_t hash=duel_state_hash(&test_game);check(command(DUEL_ACTION_ATTACK,0,1-side,DUEL_NO_SLOT,0,0)==DUEL_NOT_ALLOWED&&hash==duel_state_hash(&test_game));check(test_game.players[1-side].life==8000-2*duel_card_definitions[hayabusa].attack);
        check(duel_ai_decide(&test_game,side,&c)&&c.action==DUEL_ACTION_PHASE&&c.target==DUEL_MAIN2);
        for(unsigned position=0;position<3;++position){unsigned beast=simple_record(1341);reset();test_game.turn=3;test_game.active=side;test_game.phase=DUEL_BATTLE;test_game.players[side].monsters[0]=slot(beast,side,DUEL_FACEUP_ATTACK);test_game.players[1-side].monsters[0]=slot(24,1-side,position);
            check(command(DUEL_ACTION_ATTACK,0,1-side,0,0,0)==DUEL_OK);if(position==DUEL_FACEUP_ATTACK)check(test_game.players[1-side].life==8000&&!test_game.players[side].monsters[0].card);
            else check(test_game.players[1-side].life==8000-(duel_card_definitions[beast].attack-duel_card_definitions[24].defense)&&!test_game.players[1-side].monsters[0].card);}
    }
    native_monster_test_result[0]=checks-first;native_monster_test_result[1]=sizeof(original_monster_stat_cases)/sizeof(original_monster_stat_cases[0]);native_monster_test_result[2]=sizeof(original_monster_direct_cases)/sizeof(original_monster_direct_cases[0]);
}
