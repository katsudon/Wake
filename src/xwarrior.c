#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "lookup.h"

/*
 * Local functions.
 */
int melee_hit(CHAR_DATA*,CHAR_DATA*,int,bool);

int check_improved_accuracy(CHAR_DATA *ch,int loN){
	int skill = get_skill(ch,gsn_improved_accuracy);

	if(skill < 1)
		return loN;

	if(roll_chance(ch,skill * .75)){
		loN *= 100 + (skill/10);
		loN /= 100;
		check_improve(ch,gsn_improved_accuracy,true,1);
	}
	else
		check_improve(ch,gsn_improved_accuracy,false,1);
	return loN;
}

int heavy_hand(CHAR_DATA *ch,int dam){
	OBJ_DATA *wield;
	int skill = get_skill(ch,gsn_heavy_hand);

	if(skill < 1 || !(wield = get_eq_char(ch,WEAR_WIELD)) || !is_weapon_blunt(wield))
		return dam;

	skill *= .5;

	skill += get_curr_stat(ch,STAT_STR) / 10;

	if(roll_chance(ch,skill)){
		check_improve(ch,gsn_heavy_hand,true,1);
		dam += (dam * skill / 100);
	}
	else
		check_improve(ch,gsn_heavy_hand,false,1);
	return dam;
}

void final_strike(CHAR_DATA *ch,CHAR_DATA *victim){
	OBJ_DATA *wield;
	int skill = get_skill(ch,gsn_final_strike),dam;

	if(skill < 1 || !(wield = get_eq_char(ch,WEAR_WIELD)))
		return;

	if(roll_chance(ch,skill *.5)){
		dam = melee_hit(ch,victim,dice(wield->value[1],wield->value[2]),false);
		dam += ch->perm_stat[STAT_STR];//why?
		act("As you flee, you take one final strike at $N!",ch,NULL,victim,TO_CHAR);
		act("As $n flees, $e take one final strike at you!",ch,NULL,victim,TO_VICT);
		act("As $n flees, $e take one final strike at $N!",ch,NULL,victim,TO_NOTVICT);
		damage(ch,victim,dam,gsn_final_strike,attack_table[wield->value[3]].damage,true);
		check_improve(ch,gsn_final_strike,true,1);
	}
	else
		check_improve(ch,gsn_final_strike,false,1);
}
