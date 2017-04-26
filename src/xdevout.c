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
bool saves_dispel	( int,int,int );
bool check_poverty	( CHAR_DATA* );


void spell_bless(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim;
	AFFECT_DATA af;

	victim = (CHAR_DATA *) vo;

	if (is_affected(victim,sn)){
		if(victim == ch)
			ch->send("You are already blessed.\n\r");
		else
			act("$N already has divine favor.",ch,NULL,victim,TO_CHAR);
		return;
	}

	
	if(ch->alignment < 0)
		affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_DAMROLL,slvl * 2,0);
	else if(ch->alignment > 0)
		affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_AC,slvl * -10,0);
	else
		affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_HITROLL,slvl * 2,0);

	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_SAVING_SPELL,-slvl,0);

	if(ch->alignment > 0)
		send_to_char("You feel righteous.\n\r",victim);
	if(ch->alignment < 0)
		send_to_char("You feel wicked.\n\r",victim);
	else
		send_to_char("You feel empowered.\n\r",victim);

	if (ch != victim)
		act("You grant $N divine favor.",ch,NULL,victim,TO_CHAR);
	return;
}

void do_firstaid(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim;
	int skill = get_skill(ch,gsn_firstaid),dam;

	dam = (get_skill(ch,gsn_firstaid)/5) * ch->pcdata->skill_level[gsn_firstaid];

	if(skill < 1){
		ch->send("You wield a bandage as your weapon.\n\r");
		act("$n wields a bandage as $s weapon.",ch,NULL,NULL,TO_ROOM);
	}

	if(!argument[0])
		victim = ch;
	else
		if(!(victim = grab_char(ch,argument,true)))
			return;

	if(ch->move < 20){
		ch->send("You are too tired to do this.\n\r");
		return;
	}
	if(!victim || victim->hit == victim->max_hit){
		ch->send("They seem fine.\n\r");
		return;
	}

	if(roll_chance(ch,skill)){
		dam = UMAX(dam,10);
		dam *= -1;
		check_improve(ch,gsn_firstaid,true,1);
		damage(ch,victim,dam,gsn_firstaid,DAM_NONE,true);
		ch->move -= 20;
		if(ch != victim){
			act("You tend to $N's wounds.",ch,NULL,victim,TO_CHAR);
			act("$n tends to your wounds.",ch,NULL,victim,TO_VICT);
			act("$n tends to $N's wounds.",ch,NULL,victim,TO_NOTVICT);
		}
		else{
			act("You tend to your wounds.",ch,NULL,NULL,TO_CHAR);
			act("$n tends to $s wounds.",ch,NULL,NULL,TO_ROOM);
		}
	}
	else{
		ch->send("You failed.\n\r");
		check_improve(ch,gsn_firstaid,false,1);
	}
	WAIT_STATE(ch,skill_table[gsn_firstaid].beats);
}

int crunch_devotion(CHAR_DATA *ch,CHAR_DATA *victim,int dam){//nash mod this to buff damage too
	int sn = gsn_devotion,skill = get_skill(ch,sn);
	if(skill < 1)
		return dam;
	if(number_percent() < skill){
		dam += get_curr_stat(ch,STAT_FTH) * ch->getslvl(sn) / 10;
		if(ch != victim){
			dam -= get_curr_stat(victim,STAT_FTH);
			dam += victim->alignment > 0 ? get_curr_stat(victim,STAT_FTH) : victim->alignment < 0 ? -1 * get_curr_stat(victim,STAT_FTH) : get_curr_stat(victim,STAT_FTH) / 2;
		}
		check_improve(ch,sn,true,1);
		return dam;
	}
	check_improve(ch,sn,false,1);
	return dam;
}

void spell_imbue(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl){
	CHAR_DATA *victim;
	AFFECT_DATA af;

	victim = (CHAR_DATA *) vo;

	if (is_affected(victim,sn)){
		if(victim == ch)
			ch->send("You are already imbued.\n\r");
		else
			act("$N already has been imbued.",ch,NULL,victim,TO_CHAR);
		return;
	}

	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_HITROLL,slvl,0);
	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_DAMROLL,slvl,0);
	affect_set(victim,TO_AFFECTS,sn,level,slvl,6+level,APPLY_AC,-10 * slvl,0);

	send_to_char("Your body feels stronger.\n\r",victim);

	if (ch != victim)
		act("You imbue $N.",ch,NULL,victim,TO_CHAR);
}