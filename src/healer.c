#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "magic.h"

void do_heal(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *mob;
    char arg[MIL];
    int cost,sn;
    SPELL_FUN *spell;
    char *words;	

    /* check for healer */
    for ( mob = ch->in_room->people; mob; mob = mob->next_in_room )
    {
        if ( IS_NPC(mob) && mob->isact(AT_IS_HEALER) )
            break;
    }
 
    if ( mob == NULL )
    {
        send_to_char( "You can't do that here.\n\r", ch );
        return;
    }

    one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        /* display price list */
		act("$N says '{GI offer the following spells:{x'",ch,NULL,mob,TO_CHAR);
		printf_to_char(ch,"  light: cure light wounds      %-4d silver\n\r"
					"  serious: cure serious wounds  %-4d silver\n\r"
					"  critic: cure critical wounds  %-4d silver\n\r"
					"  heal: healing spell	        %-4d silver\n\r"
					"  blind: cure blindness         %-4d silver\n\r"
					"  disease: cure disease         %-4d silver\n\r"
					"  poison:  cure poison	        %-4d silver\n\r"
					"  uncurse: remove curse	        %-4d silver\n\r"
					"  refresh: restore movement     %-4d silver\n\r"
					"  mana:  restore mana	        %-4d silver\n\r"
					" Type heal <type> to be healed.\n\r",
							(1000 * mob->level)/100,
							(1600 * mob->level)/100,
							(2500 * mob->level)/100,
							(5000 * mob->level)/100,
							(2000 * mob->level)/100,
							(1500 * mob->level)/100,
							(2500 * mob->level)/100,
							(5000 * mob->level)/100,
							(1000 * mob->level)/100,
							( 500 * mob->level)/100);
		return;
    }

/*    if (!str_prefix(arg,"light"))
    {
        spell = spell_cure_light;
		sn    = skill_lookup("cure light");
		words = "judicandus dies";
		cost  = (1000 * mob->level)/100;
    }
    else if (!str_prefix(arg,"serious"))
    {
		spell = spell_cure_serious;
		sn    = skill_lookup("cure serious");
		words = "judicandus gzfuajg";
		cost = (1600 * mob->level)/100;
    }
    else if (!str_prefix(arg,"critical"))
    {
		spell = spell_cure_critical;
		sn    = skill_lookup("cure critical");
		words = "judicandus qfuhuqar";
		cost = (2500 * mob->level)/100;
    }
    else if (!str_prefix(arg,"heal"))
    {
		spell = spell_heal;
		sn = skill_lookup("heal");
		words = "pzar";
		cost = (5000 * mob->level)/100;
    }
    else*/ if (!str_prefix(arg,"blindness"))
    {
		spell = spell_cure_blindness;
		sn    = skill_lookup("cure blindness");
      	words = "judicandus noselacri";		
        cost = (2000 * mob->level)/100;
    }
    else if (!str_prefix(arg,"disease"))
    {
		spell = spell_cure_disease;
		sn    = skill_lookup("cure disease");
		words = "judicandus eugzagz";
		cost = (1500 * mob->level)/100;
    }
    else if (!str_prefix(arg,"poison"))
    {
		spell = spell_cure_poison;
		sn    = skill_lookup("cure poison");
		words = "judicandus sausabru";
		cost  = (2500 * mob->level)/100;
    }
    else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
    {
		spell = spell_lift_curse;
		sn    = skill_lookup("remove curse");
		words = "candussido judifgz";
		cost  = (5000 * mob->level)/100;
    }
    else if (!str_prefix(arg,"mana") || !str_prefix(arg,"energize"))
    {
        spell = NULL;
        sn = -1;
        words = "energizer";
        cost = (1000 * mob->level)/100;
    }
    else if (!str_prefix(arg,"refresh") || !str_prefix(arg,"moves"))
    {
		spell =  spell_refresh;
		sn    = skill_lookup("refresh");
		words = "candusima";
		cost  = ( 500 * mob->level)/100;
    }
    else 
    {
		act("$N says 'Type 'heal' for a list of spells.'",ch,NULL,mob,TO_CHAR);
		return;
    }

    if (cost > (ch->gold * 100 + ch->silver))
    {
		act("$N says 'You do not have enough gold for my services.'",ch,NULL,mob,TO_CHAR);
		return;
    }

    WAIT_STATE(ch,PULSE_VIOLENCE);

	cost *= (mob->level/100);

    deduct_cost(ch,cost);
    mob->gold += cost / 100;
    mob->silver += cost % 100;
    act("$n utters the words '$T'.",mob,NULL,words,TO_ROOM);
  
	if (spell == NULL)  /* restore mana trap...kinda hackish */
	{
		ch->modmana(dice(2,8) + mob->level / 3);
		ch->setmana(UMIN(ch->getmana(),ch->getmaxmana()));
		send_to_char("A warm glow passes through you.\n\r",ch);
		return;
	}

	if (sn == -1)
		return;
    
	spell(sn,mob->level,mob,ch,TARGET_CHAR,1);//Nash fix the slvl
}
