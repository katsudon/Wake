#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"

int buff_mana(CHAR_DATA*);

void print_cp(CHAR_DATA *ch){
	int points,tnl;
	char buf[MSL];
	points = ch->pcdata->points;
	tnl = exp_per_level(ch);
	sprintf(buf,"\n\rCreationPoints: {g%4d{x, TNL: {g%d{x\n\r",points,tnl);
	ch->send(buf);
}

void print_gainlist(CHAR_DATA *ch,bool gained){
	char buf[MSL];
	int sn,col=0;
	bool found = false;
	if (IS_NPC(ch))
	  return;
	ch->send(" _____________________________________________________________________________\n\r");
	ch->send("/ {dName               Lv CP{x| {dName               Lv CP{x| {dName               Lv CP{x\\\n\r");
	for (sn = 0; sn < MAX_GROUP; sn++){
		if (!group_table[sn].name)
			break;
		if (ch->pcdata->group_known[sn] == gained && group_table[sn].cost[ch->pclass] > 0){
			found = true;

			sprintf(buf,"| {x%-18s    {c%-2d{x",group_table[sn].name,group_table[sn].cost[ch->pclass]);
			if ( ++col % 3 == 0){
				strcat(buf,"|\n\r");
				col = 0;
			}
			ch->send(buf);
		}
	}
	if (found){
		if (col == 0)
			;
		else if(col % 2 == 0)
			ch->send("|                         |\n\r");
		else if(col % 1 == 0)
			ch->send("|                         |                         |\n\r");
		ch->send("|_____________________________________________________________________________|\n\r");
		ch->send("| {dName               Lv CP{x| {dName               Lv CP{x| {dName               Lv CP{x|\n\r");
	}
	col = 0;
	for (sn = 0; sn < MAX_SKILL; sn++){
		if (!skill_table[sn].name)
			break;
		if (skill_table[sn].skill_level[ch->pclass] >= LEVEL_IMMORTAL)
			continue;
		if (!IS_SKILL_NATURAL(sn) && skill_table[sn].slot != SKILL_SPELL && ch->pcdata->learned[sn] == gained )  {
			found = true;
			sprintf(buf,"| {x%-18s {c%2d %-2d{x",skill_table[sn].name,skill_table[sn].skill_level[ch->pclass],skill_table[sn].cost[ch->pclass]);
			if ( ++col % 3 == 0){
				strcat(buf,"|\n\r");
				col = 0;
			}
			ch->send(buf);
		}
	}
	if (col == 0)
		;
	else if(col % 2 == 0)
		ch->send("|                         |\n\r");
	else if(col % 1 == 0)
		ch->send("|                         |                         |\n\r");
	ch->send("\\_____________________________________________________________________________/\n\r\n\r");

	if (!found){
		ch->send("No skills found.\n\r");
		return;
	}
}

void print_gained(CHAR_DATA *ch){
	print_gainlist(ch,true);
}

void print_ungained(CHAR_DATA *ch){
	print_gainlist(ch,false);
}

char *grind_percent_color(int val){
	if (val == 0)
		return "{d";
	else if (val < 10)
		return "{r";
	else if (val < 20)
		return "{R";
	else if (val < 30)
		return "{m";
	else if (val < 40)
		return "{M";
	else if (val < 50)
		return "{y";
	else if (val < 60)
		return "{Y";
	else if (val < 70)
		return "{g";
	else if (val < 80)
		return "{G";
	else if (val < 90)
		return "{b";
	else if (val < 100)
		return "{B";
	else
		return "{C";
}
//nashneedstomakethisnotsuckatungainedandcrap
void print_skillspell(CHAR_DATA *ch,int fall,int sstype,char *word){
	BUFFER *buffer = new_buf();
    char buf[MSL],tbuf[MIL];
	int maxn,i,sn,level=0,cols=0,slvl,learned,unlocked;
	bool first,found=false,sfull = false;

	if(fall == 1)
		maxn = IS_IMMORTAL(ch) ? MAX_LEVEL : LEVEL_HERO;
	else
		maxn = ch->level;

	if(fall == 2)
		sfull = true;
	for(i = 0;i <= maxn;i++){
		first = true;
		cols = 0;
		for (sn = 0; sn < MAX_SKILL && skill_table[sn].name; sn++){
			level = skill_table[sn].skill_level[ch->pclass];
			slvl = ch->pcdata->skill_level[sn];
			learned = ch->pcdata->learned[sn];
			unlocked = ch->pcdata->unlocked[sn];
			if(slvl < 0 || level != i || skill_table[sn].slot != sstype || IS_SKILL_NATURAL(sn))
				continue;

			if((!sfull && fall != 1) && ch->pcdata->skill_level[sn] == 0)
				continue;

			if(skill_table[sn].skill_level[get_class(ch)] > -1){
				found = true;

				if (level <= ch->level){
					if(learned > 0)//sprintf(tbuf,"{d({c%d{d) {x%-18s %s%3d{x%%",UMAX(slvl,0),skill_table[sn].name,grind_percent_color(ch->pcdata->learned[sn]),ch->pcdata->learned[sn]);
						sprintf(tbuf,"{d({c%d{d){x%-18s %s%3d{x%s",
							UMAX(slvl,0),
							skill_table[sn].name,
							sstype == SKILL_SPELL ? "{C" : grind_percent_color(ch->pcdata->learned[sn]),
							sstype == SKILL_SPELL ? skill_table[sn].min_mana : ch->pcdata->learned[sn],
							sstype == SKILL_SPELL ? " mana" : "%");
					else{
						if(unlocked)
							sprintf(tbuf,"{d({c%d{d){x%-18s      {GN/A{x",UMAX(slvl,0),skill_table[sn].name);
						else if(fall == 1)
							sprintf(tbuf,"{d({cx{d)%-18s      {RN/A{x",skill_table[sn].name);
					}
					if(first){
          				sprintf(buf,"\n\rLevel {c%3d{x: %s{x",level,tbuf);
						first = false;
						cols = 0;
					}
					else{
						if(++cols % 2 == 0)
							cols = 0;

						if(cols < 1)
							sprintf(buf,"\n\r           %s",tbuf);
						else
							sprintf(buf,"   %s",tbuf);
					}
				}
				else{
					if(learned > 0)
						sprintf(tbuf,"{d({c%d{d){x%-18s      {cN{x/{gA{x",UMAX(slvl,0),skill_table[sn].name);
					else{
						if(unlocked)
							sprintf(tbuf,"{d({c%d{d){x%-18s      {gN{x/{gG{x",UMAX(slvl,0),skill_table[sn].name);
						else if(fall == 1)
							sprintf(tbuf,"{d({cx{d)%-18s      {rN{x/{gG{x",skill_table[sn].name);
					}
					if(first){
          				sprintf(buf,"\n\rLevel {c%3d{x: %s",level,tbuf);
						first = false;
						cols = 0;
					}
					else{
						if(++cols % 2 == 0)
							cols = 0;

						if(cols < 1)
							sprintf(buf,"\n\r           %s",tbuf);
						else
							sprintf(buf,"   %s",tbuf);
					}
				}
				add_buf(buffer,buf);
			}
		}
	}

	if (!found){
		printf_to_char(ch,"No %s found.\n\r",word);
		return;
	}
	else
		add_buf(buffer,"\n\r\n\r");

    page_to_char(buf_string(buffer),ch);
    free_buf(buffer);
}

/* RT spells and skills show the players spells (or skills) */
void do_spells(CHAR_DATA *ch, char *argument){
	int type = 0;
    if (IS_NPC(ch))
      return;
	ch->send("1\n\r");
	if(!str_cmp(argument,"all"))
		type = 1;
	else if(!str_cmp(argument,"full"))
		type = 2;
	print_skillspell(ch,type,SKILL_SPELL,"spells");
}
void do_skills(CHAR_DATA *ch, char *argument){
	int type = 0;
    if (IS_NPC(ch))
      return;
	ch->send("1\n\r");
	if(!str_cmp(argument,"all"))
		type = 1;
	else if(!str_cmp(argument,"full"))
		type = 2;
	print_skillspell(ch,type,SKILL_NORMAL,"skills");
}

int exp_per_level(CHAR_DATA *ch){
    int expl,inc,prev,lvl = ch->level;

    if (IS_NPC(ch))
		return 2000;

	if (ch->level >= LEVEL_HERO)
		return 0;

    expl = 200;

	prev = ((lvl-1) * 0.02) * (100 + (3 *pow(ch->pcdata->points,2)) + pow(lvl-1,2));

	expl += 3 * pow(ch->pcdata->points,2);
	inc = pow(ch->level,2);

    expl += inc;
	expl += prev;

    return UMAX(expl,2000);
}

/* checks for skill improvement */
void check_improve(CHAR_DATA *ch,int sn,bool success,int multiplier){
	int chance,rating;

	if (IS_NPC(ch) || sn < 1)
		return;

	if (ch->level < skill_table[sn].skill_level[ch->pclass]
	||  (skill_table[sn].skill_level[ch->pclass] < 1/* && skill_table[sn].skill_level == 0*/)
	||  ch->pcdata->learned[sn] == 0
	||  ch->pcdata->learned[sn] == 100)
		return;  /* skill is not known */ 

	/* check to see if the character has a chance to learn */
	if(skill_table[sn].skill_level[ch->pclass] < 1)
		rating = 1;
	else
		rating = UMAX(skill_table[sn].cost[ch->pclass],1);

	chance = 1.5 * int_app[get_curr_stat(ch,STAT_INT)].learn;
	if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"{rimprove({c%d{x)",chance);
	chance *= multiplier;
	if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"({c%d{x)",chance);
	chance /= rating * 2;
	if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"({c%d{x)",chance);
	//chance += ch->level;

	if(chance == 0)//Error trap
		return;

	if (ch->iscomm(CM_DEBUG))printf_to_char(ch,"({c%d{x)\n\r",chance);
	if (number_range(1,1000) > chance)
		return;

	/* now that the character has a CHANCE to learn, see if they really have */	

	if (success)
	{
		chance = URANGE(5,100 - ch->pcdata->learned[sn], 95);
		if (number_percent() < chance)
		{
			if(++ch->pcdata->learned[sn] == 100)
				printf_to_char(ch,"You have {Rmastered {G%s{x!\n\r", skill_table[sn].name);
			else
				printf_to_char(ch,"You have become better at {G%s{x!\n\r", skill_table[sn].name);
			gain_exp(ch,2 * rating);
		}
	}
	else
	{
		chance = number_range(ch->pcdata->learned[sn]/4,ch->pcdata->learned[sn]);
		if (number_percent() < chance)
		{
			ch->pcdata->learned[sn] += number_range(1,3);
			ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn],100);
			if (ch->pcdata->learned[sn] == 100)
				printf_to_char(ch,"You learn from your mistakes, and you master {G%s{x!\n\r",skill_table[sn].name);
			else
				printf_to_char(ch,"You learn from your mistakes, and your {G%s {xskill improves.\n\r",skill_table[sn].name);
			gain_exp(ch,2 * rating);
		}
	}
}

void skillspell_add(CHAR_DATA *ch,const char *name,bool message,bool cost){
	int sn,gn;

	if (IS_NPC(ch))
		return;

	sn = skillspell_prefix(name);

	if (sn != -1){
		if (ch->pcdata->learned[sn] > 0){
			if (message)
				ch->send("You already have that skill.\n\r");
		}
		else if (cost && skill_table[sn].cost[ch->pclass] < 1){
			if (message)
				ch->send("That skill is not available to your class.\n\r");
		}
		else{
			if (message)
				printf_to_char(ch,"Adding the skill '{G%s{x'.\n\r",skill_table[sn].name);
			if (cost)
				ch->pcdata->points += skill_table[sn].cost[ch->pclass];
			ch->pcdata->learned[sn] = 1;
		}
		return;
	}
}

void skillspell_remove(CHAR_DATA *ch,const char *name,bool message,bool cost){
	int sn;

	sn = skillspell_prefix(name);

	if (sn != -1){
		if (ch->pcdata->learned[sn] < 1){
			if (message)
			ch->send("You already don't have that skill.\n\r");
		}
		else{
			if (skill_table[sn].cost[ch->pclass] == 0){
				if(message)
					ch->send("You cannot drop this skill.\n\r");
			}
			else{
				if (message)
					printf_to_char(ch,"Removing the skill '{G%s{x'.\n\r",skill_table[sn].name);
				if (cost)
					ch->pcdata->points -= skill_table[sn].cost[ch->pclass];
				ch->pcdata->learned[sn] = 0;
			}
		}
		return;
	}
}

void do_train(CHAR_DATA *ch,char *argument){
	char arg[MIL],buf[MIL],zbuf[MIL];
	int addz,tostat = -1;
	int cend = get_curr_stat(ch,STAT_END),cstr = get_curr_stat(ch,STAT_STR),cdex = get_curr_stat(ch,STAT_AGI);
	int hpmin = classes[ch->pclass].hp_min,hpmax = classes[ch->pclass].hp_max;
	int mvmin = classes[ch->pclass].mv_min,mvmax = classes[ch->pclass].mv_max;
	int mpmin = classes[ch->pclass].mp_min,mpmax = classes[ch->pclass].mp_max;
	bool found = false;
	CHAR_DATA *mob;

	for (mob = ch->in_room->people; mob; mob = mob->next_in_room )
		if(IS_NPC(mob) && mob->trainer > 0)
			break;

	if(!mob || IS_NPC(ch)){
		ch->send("There is no trainer near you.\n\r");
		return;
	}

	one_argument(argument,arg);

	if(!arg[0]){
		printf_to_char(ch,"You have {R%d {xtrain%s and {R%d {xstatpoint%s. You may train:\n\r",ch->pcdata->trains,ch->pcdata->trains == 1 ? "" : "s",ch->pcdata->statpoints,ch->pcdata->statpoints == 1 ? "" : "s");
		ch->send("{d/-|{RTrains{d|-------------------------------------------------\\\n\r");
		if(classes[get_class(ch)].ctier == 3){
			if(classes[ch->pclass].amp)
				sprintf(zbuf,"Antimana");
			else
				sprintf(zbuf,"Mana");
			ch->send("| {xAttribute        Current                                 {d|\n\r");
			ch->printf("|  {xHealth         ({R%6d{x)                                 {d|\n\r",ch->max_hit);
			ch->printf("|  {x%-9s      ({R%6d{x)                                 {d|\n\r",zbuf,ch->getmaxmana());
			ch->printf("|  {xMove           ({R%6d{x)                                 {d|\n\r",ch->max_move);
		}
		else
			ch->send("| {xYou may not train hp, mp or mv until your final class.   {d|\n\r");
		ch->send("\\----------------------------------------------------------/{x\n\r");
		ch->send("{d/-|{RStatpoints{d|---------------------------------------------\\\n\r");
		ch->send("| {xStat  Base   Eq'd  EqMax                                 {d|\n\r");
		for(int stat = 0;stat < MAX_STATS;stat++){
			if(ch->max_stat[stat] < STAT_MAX){
				ch->printf("|  {x%s  ({c%2d{x)   ({C%2d{x)   ({R%2d{x)                                 {d|\n\r",abbrev_stat_flags[stat].name,ch->perm_stat[stat],get_curr_stat(ch,stat),ch->max_stat[stat]);
				found = true;
			}
		}
		if(!found)
			ch->send("|  {xNone                                                    {d|\n\r");
		ch->send("\\----------------------------------------------------------/{x\n\r");
		if(found)
			ch->send("\n\r");
		return;
	}

	if(ch->pcdata->trains < 1 && ch->pcdata->statpoints < 1){
		ch->send("You need at least one train or statpoint.\n\r");
		return;
	}
	if(!str_prefix(arg,"health") || !str_prefix(arg,"hp")){
		addz = number_range(hpmin,hpmax);

		ch->max_hit += addz;
		ch->pcdata->perm_hit += addz;//NASHNEEDSBUFFHP
		ch->pcdata->trains--;

		printf_to_char(ch,"Your hp has increased by %d.",addz);
		act("$n increases $s health!",ch,NULL,NULL,TO_ROOM);
	}
	else if(!str_prefix(arg,"mana") || !str_prefix(arg,"mp")){
		addz = number_range(mpmin,mpmax);

		ch->max_mana += addz;
		ch->pcdata->perm_mana += addz;
		ch->pcdata->trains--;

		printf_to_char(ch,"Your mp has increased by %d.",addz);
		act("$n increases $s power!",ch,NULL,NULL,TO_ROOM);
	}
	else if(!str_prefix(arg,"move") || !str_prefix(arg,"mv")){
		addz = number_range(mvmin,mvmax);

		ch->max_move += addz;
		ch->pcdata->perm_move += addz;
		ch->pcdata->trains--;

		printf_to_char(ch,"Your mv has increased by %d.",addz);
		act("$n increases $s durability!",ch,NULL,NULL,TO_ROOM);
	}
	else{
		if((tostat = abbrev_stat_lookup(arg)) < 0)
			tostat = stat_lookup(arg);
		if(tostat == -1){
			sprintf(buf,"You have {R%d {xtrain%s and {R%d {xstatpoints.",ch->pcdata->trains,ch->pcdata->trains == 1 ? "" : "s");
			do_function(mob,&do_sayto,buf);
			return;
		}
		if(ch->max_stat[tostat] == STAT_MAX){
			ch->printf("%s says to you, '{GYou cannot train your %s any further.{x'\n\r",mob->short_descr,stat_flags[tostat].name);
			return;
		}
		ch->max_stat[tostat]++;
		ch->pcdata->statpoints--;
		ch->printf("The maximum you can equip %s to has increased.",stat_flags[tostat].name);
		sprintf(buf,"$n increases $s %s max!",stat_flags[tostat].name);
		act(buf,ch,NULL,NULL,TO_ROOM);
	}
}

int get_prac(CHAR_DATA *ch,int sn){
	int num = 25;
	num = num * get_curr_stat(ch,STAT_INT)*2 / (100 / (IS_SPELL(sn) + 1));
	num = number_range(num,num * 1.5);
	return num;
}

void do_practice(CHAR_DATA *ch,char *argument){
	CHAR_DATA *trainer;
	char buf[MIL];
	int sn,n,cols = 0;

	if(IS_NPC(ch)){
		ch->send("Cute...\n\r");
		return;
	}

	if(!argument[0]){
		for(sn = 0;sn<MAX_SKILL;sn++){
			if(IS_SKILL_NATURAL(sn) || ch->pcdata->skill_level[sn] < 1 || ch->pcdata->learned[sn] < 1)
				continue;
			sprintf(buf," %-18s %s%3d{x%% ",skill_table[sn].name,grind_percent_color(ch->pcdata->learned[sn]),ch->pcdata->learned[sn]);
			printf_to_char(ch,"%-30s",buf);
			
			if(++cols >= 3){
				cols = 0;
				ch->send("\n\r");
			}
		}
		if(cols != 0)
			ch->send("\n\r");
		if(!IS_NPC(ch))
			printf_to_char(ch,"You have {R%d {xpractices.\n\r",ch->pcdata->practices);
		return;
	}

	for (trainer = ch->in_room->people; trainer; trainer = trainer->next_in_room )
		if (IS_NPC(trainer) && trainer->trainer > 0)
			break;

	if(!trainer || !IS_NPC(trainer) || trainer->trainer < 1){
		ch->send("There are no viable trainers nearby.\n\r");
		return;
	}

	if(get_class(ch) != trainer->trainer){
		ch->send("This trainer cannot help you advance.\n\r");
		return;
	}

	if((sn = skill_lookup(argument)) == -1){
		ch->send("That is not a skill.\n\r");
		return;
	}

	if(ch->pcdata->skill_level[sn] > 0 && ch->pcdata->learned[sn] > 0){
		if(ch->pcdata->learned[sn] >= classes[ch->pclass].skill_adept){
			printf_to_char(ch,"You have already mastered %s.\n\r",skill_table[sn].name);
			return;
		}

		if(ch->pcdata->practices < 1){
			ch->send("You do not have any practices.\n\r");
			return;
		}

		ch->pcdata->learned[sn] = UMIN(ch->pcdata->learned[sn] + get_prac(ch,sn),classes[ch->pclass].skill_adept);
		ch->pcdata->practices--;
		ch->pcdata->s_practices++;
		if(ch->pcdata->learned[sn] == classes[ch->pclass].skill_adept){
			printf_to_char(ch,"You have {Ra very good grasp{x of the skill '{C%s{x'!\n\r",skill_table[sn].name);
			act("$n has {Ra very good grasp{x of the skill {C$T{x!",ch,NULL,skill_table[sn].name,TO_ROOM);
		}
		else{
			printf_to_char(ch,"You practice the skill {C%s{x.\n\r",skill_table[sn].name,ch->pcdata->skill_level[sn]);
			act("$n practices {C$T{x.",ch,NULL,skill_table[sn].name,TO_ROOM);
		}
	}
}

void do_convert(CHAR_DATA *ch,char *argument){
	CHAR_DATA *trainer;

	if(IS_NPC(ch)){
		ch->send("Cute...\n\r");
		return;
	}

	for (trainer = ch->in_room->people; trainer; trainer = trainer->next_in_room )
		if (IS_NPC(trainer) && trainer->trainer > 0)
			break;
	if(!trainer || !IS_NPC(trainer) || trainer->trainer < 1){
		ch->send("There are no viable trainers nearby.\n\r");
		return;
	}
	if(get_class(ch) != trainer->trainer){
		ch->send("This trainer cannot help you convert.\n\r");
		return;
	}

	if(!str_prefix(argument,"trains")){
		if(ch->pcdata->practices < 10){
			ch->send("You don't have enough practices.\n\r");
			return;
		}
		ch->pcdata->practices -= 10;
		ch->pcdata->trains++;
		ch->send("You convert ten practices into one train.\n\r");
		return;
	}
	else if(!str_prefix(argument,"practices")){
		if(ch->pcdata->trains < 1){
			ch->send("You don't have any trains.\n\r");
			return;
		}
		ch->pcdata->practices += 10;
		ch->pcdata->trains--;
		ch->send("You convert one train into ten practices.\n\r");
		return;
	}
	else
		ch->send("That doesn't work. Syntax: convert <practices/trains>\n\r");
}

void do_rape_skill(CHAR_DATA *ch,char *argument){
	if(!ch->iscomm(CM_DEBUG) || IS_NPC(ch))
		return;

	for(int sn = 0;skill_table[sn].name;sn++){
		if(!ch->pcdata->unlocked[sn] && ch->getslvl(sn) < 0 && ch->pcdata->learned[sn] < 0)
			continue;
		printf_to_char(ch,"%1d %1d %3d %d %s\n\r",ch->pcdata->unlocked[sn],ch->getslvl(sn),ch->pcdata->learned[sn],skill_table[sn].skill_level[ch->pclass],skill_table[sn].name);
	}
}
