
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
* Local Functions
*/
bool		check_poverty			( CHAR_DATA* );

void spell_divine_arrow(int sn,int level, CHAR_DATA *ch,void *vo,int target,int slvl)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, align;

	if (IS_EVIL(ch) )
	{
		victim = ch;
		ch->send("You stagger under the arrow's impact!\n\r");
	}

	if (victim != ch)
	{
		act("$n raises $s hand, and a glowing arrow shoots forth!",ch,NULL,NULL,TO_ROOM);
		ch->send("You raise your hand and an arrow of energy shoots forth!\n\r");
	}

	if (IS_GOOD(victim))
	{
		act("$n seems unharmed by the arrow.",victim,NULL,victim,TO_ROOM);
		victim->send("You seem to absorb the arrow's energy.\n\r");
		return;
	}

	dam - dice(level, 10);
	if ( saves_spell(level,victim,DAM_HOLY) )
		dam /= 2;

	align = victim->alignment;
	align -= 350;
	
	if (align < -1000)
			align = -1000 + (align + 1000) / 3;

	dam = (dam * align * align) / 1000000;

	skill_damage(ch,victim,dam,sn,DAM_HOLY,true);
	spell_blindness(gsn_blindness,3 * level / 4,ch,(void *)victim,TARGET_CHAR,slvl);
}

void spell_holy_smite(int sn,int level,CHAR_DATA *ch,void *vo, int target,int slvl){
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam, align;
	
	if (IS_EVIL(ch))
	{
		victim = ch;
		ch->send("The energy lashes back against you.\n\r");
	}

	if (victim != ch)
	{
		act("$n calls the name of $s diety and a blinding force comes down from the heavens!",ch,NULL,NULL,TO_ROOM);
		ch->send("You raise your hand and a blinding force comes down from the heavens!\n\r");
	}

	if (IS_GOOD(victim))
	{
		act("$n seems barely affected by the diety's power.",victim,NULL,victim,TO_ROOM);
		victim->send("The force seems to have little affect on you.\n\r");
		return;
	}
	else if (IS_NEUTRAL(victim))
	{	
		dam = number_range(level*1.5,level*3);
		if (!saves_spell(level,victim,DAM_HOLY) )
			dam *= 2;
	}
	else
		dam = number_range(level*2,level*4);

	dam = dam * ch->level / 100;

	skill_damage(ch,victim,dam,sn,DAM_HOLY,true);
}

void spell_wrath(int sn, int level, CHAR_DATA *ch, void *vo, int target,int slvl)
{
	CHAR_DATA *victim = (CHAR_DATA *) vo;
	int dam;

	if (victim != ch)
	{
		act("$n prays silently then stretches out $s holy symbol and suddenly a bolt of lightning strikes $N!",ch,NULL,victim,TO_ROOM);
		act("You see $n pray and use $s holy symbol as suddenly lightning strikes you!",ch,NULL,victim,TO_VICT);
		ch->send("Your diety responds with striking your foe with lightning!\n\r");
	}
	dam = dice(45,10);
		if(check_poverty(ch))
			dam *= 1.5;
	if ( saves_spell(level, victim, DAM_NEGATIVE) )
		dam /= 2;
	skill_damage(ch,victim,dam,sn,DAM_NEGATIVE,true);
}

void do_lastrites(CHAR_DATA *ch,char *argument)
{
	char arg[MSL];
	OBJ_DATA *corpse;
	AFFECT_DATA af;

	one_argument(argument,arg);

	if (!arg[0])
		ch->send("To what?\n\r");
	else if ((corpse = get_obj_list(ch,arg,ch->in_room->contents)) == NULL || corpse->item_type != ITEM_CORPSE_NPC)
	{
		ch->send("Their corpse is... not here.\n\r");
		return;
	}

	if (corpse->contains)
	{
		ch->send("The gods would not accept a bogged down corpse.\n\r");
		return;
	}

	if (number_percent() < get_skill(ch,gsn_lastrites) * .8)
	{
		act("You ceremoniously sacrifice $p to $T.",ch,corpse,god_table[ch->god].name,TO_CHAR);
		act("$n ceremoniously sacrifices $p to $s god.",ch,corpse,NULL,TO_ROOM);
		ch->modmana(50);
		check_improve(ch,gsn_lastrites,true,2);
	}
	else
	{
		check_improve(ch,gsn_lastrites,false,4);
		printf_to_char(ch,"You fail to impress %s, and %s disintegrates.\n\r",god_table[ch->god].name,corpse->short_descr);
	}
	WAIT_STATE(ch,skill_table[gsn_lastrites].beats);
	extract_obj(corpse);
}

void spell_divine_strike(int sn,int level,CHAR_DATA *ch,void *vo,int target,int slvl)
{
	CHAR_DATA *gch,*gchnext;
	int dam;
	ch->send("You call out your diety's name and holy energy explodes about you!\n\r");
	act("$n calls out $s diety's name and holy energy explodes about the area!",ch,NULL,NULL,TO_ROOM);
	for ( gch = ch->in_room->people; gch != NULL; gch = gchnext)
	{
		gchnext = gch->next_in_room;
		dam = dice(50,10);
		if(check_poverty(ch))
			dam *= 1.5;
		if (saves_spell(level*1.5,gch,DAM_HOLY) )
			dam  /= 2;
		skill_damage(ch,gch,dam,sn,DAM_HOLY,true);
		ch->send("You call out your diety's name and holy energy explodes about you!\n\r");
		act("$n strikes $N!",ch,NULL,gch,TO_NOTVICT);
		act("Holy energy explodes within you!",ch,NULL,gch,TO_VICT);
	}
}

void do_zeal(CHAR_DATA *ch,char *argument)
{
	AFFECT_DATA af;

	if (ch->isaff(AF_SLOW))
	{
		if (!check_dispel(ch->level,ch,skill_lookup("slow")))
		{
			ch->send("You couldn't build up enough faith.\n\r");
			return;
		}
		ch->send("You feel yourself moving less slowly.\n\r");
		act("$n is moving less slowly.",ch,NULL,NULL,TO_ROOM);
		return;
	}

	if (ch->isaff(AF_HASTE) || ch->isoff(OF_FAST))
	{
		send_to_char("You can't move any faster!\n\r",ch);
		return;
	}

	affect_set(ch,TO_AFFECTS,gsn_zeal,ch->level,ch->getslvl(gsn_zeal),ch->level/10,APPLY_AGI,2,AF_BERSERK);
	ch->send("You feel yourself moving more quickly.\n\r");
	act("$n is moving more quickly.",ch,NULL,NULL,TO_ROOM);
}
