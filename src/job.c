#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "merc.h"
#include "recycle.h"
#include "interp.h"



int get_tier(CHAR_DATA *ch,int sn){
	switch(skill_table[sn].skill_level[ch->pclass]){
	case 1:
		return 0;
	case 10:
		return 1;
	case 25:
		return 2;
	case 50:
		return 3;
	default:
		return 4;
	}
}
int get_subgains(CHAR_DATA *ch,int sn){
	int tier = get_tier(ch,sn);
	switch(tier){
		case 0:
			return ch->pcdata->s_studies[0] + ch->pcdata->s_studies[1] + ch->pcdata->s_studies[2] + ch->pcdata->s_studies[3];
		case 1:
			return ch->pcdata->s_studies[1] + ch->pcdata->s_studies[2] + ch->pcdata->s_studies[3];
		case 2:
			return ch->pcdata->s_studies[2] + ch->pcdata->s_studies[3];
		case 3:
			return ch->pcdata->s_studies[3];
		default:
			ch->send("Big uh oh.\n\r");
			return 0;
	};
}
void burn_subgains(CHAR_DATA *ch,int sn){
	int tier = get_tier(ch,sn),tn = -1;
	switch(tier){
		case 0:
			if(ch->pcdata->s_studies[0] > 0)				tn = 0;
			else if(ch->pcdata->s_studies[1] > 0)			tn = 1;
			else if(ch->pcdata->s_studies[2] > 0)			tn = 2;
			else if(ch->pcdata->s_studies[3] > 0)			tn = 3;
			break;
		case 1:
			if(ch->pcdata->s_studies[1] > 0)				tn = 1;
			else if(ch->pcdata->s_studies[2] > 0)			tn = 2;
			else if(ch->pcdata->s_studies[3] > 0)			tn = 3;
			break;
		case 2:
			if(ch->pcdata->s_studies[2] > 0)				tn = 2;
			else if(ch->pcdata->s_studies[3] > 0)			tn = 3;
			break;
		case 3:
			if(ch->pcdata->s_studies[3] > 0)				tn = 3;
			break;
		default:
			ch->send("Big uh oh.\n\r");
			return;
	};
	if(tn >= 0){
		if(ch->pcdata->s_studies[tn] > 1)
			printf_to_char(ch,"Using one saved study from tier %d.\n\r",tn);
		else if(ch->pcdata->s_studies[tn] > 0)
			printf_to_char(ch,"Using the {Rlast {xstudy from tier %d.\n\r",tn);
		else{
			ch->pcdata->studies--;
		}
	}
	ch->pcdata->s_studies[tn]--;
}
void unlock_skill(CHAR_DATA *ch,int sn){
	printf_to_char(ch,"You have unlocked the skill '{C%s{x'!\n\r",skill_table[sn].name);
	act("$n has unlocked the skill '{C$T{x'!",ch,NULL,skill_table[sn].name,TO_ROOM);
	ch->pcdata->skill_level[sn] = 0;
	ch->pcdata->unlocked[sn] = true;
}

bool check_skill_needs(CHAR_DATA *ch,int sn){
	int tsn,n;
	if(ch->pcdata->skill_level[sn] == -1)
		return false;

	for(n = 0; n < 10; n++){
		tsn = skill_table[sn].skl[ch->pclass].unlock_by[n].sn;
		if(tsn > 0){
			if(skill_table[sn].skl[ch->pclass].unlock_by[n].level > ch->pcdata->skill_level[tsn]){
				return false;
			}
		}
	}
	return true;
}

void check_skill_lock(CHAR_DATA *ch,int sn){
	int n,tsn;

	for(n = 0; n < 10; n++){//lockspleases
		tsn = skill_table[sn].skl[ch->pclass].locks[n].sn;
		if(ch->pcdata->skill_level[tsn] == -1 && ch->pcdata->unlocked[tsn] == false)
			continue;
		if(tsn > 0){
			printf_to_char(ch,"You have {Rlocked{x the skill '{C%s{x'!\n\r",skill_table[tsn].name);
			act("$n has locked the skill '{C$T{x'!",ch,NULL,skill_table[tsn].name,TO_ROOM);
			ch->pcdata->skill_level[tsn] = -1;
			ch->pcdata->unlocked[tsn] = false;
		}
	}
}

void check_skill_unlock(CHAR_DATA *ch,int sn){
	int n,tsn;
	for(n = 0; n < 10; n++){//unlockspleases
		tsn = skill_table[sn].skl[ch->pclass].unlocks[n].sn;
		if(tsn > 0){
			if(ch->pcdata->unlocked[tsn])//ch->pcdata->skill_level[tsn] >= 0 && ch->pcdata->learned[tsn] >= 0)
				continue;
			if(check_skill_needs(ch,tsn))
				unlock_skill(ch,tsn);
		}
	}
}

void impose_job(CHAR_DATA *ch, int job){
	int sn;

	ch->pclass = job;
	ch->alignment = classes[ch->pclass].align;

	for(sn = 0;sn<MAX_SKILL;sn++){
		if(ch->pcdata->unlocked[sn] == true){
			continue;
		}
		if(skill_table[sn].cost[job] > 0){//available
			ch->pcdata->skill_level[sn] = 0;
			ch->pcdata->learned[sn] = 0;
			ch->pcdata->unlocked[sn] = true;
		}
		else if (skill_table[sn].cost[job] == 0){//possible
			if(ch->pcdata->skill_level[sn] == 0 && ch->pcdata->unlocked[sn] == true)
				continue;
				ch->pcdata->skill_level[sn] = 0;
		}
		else{//no luck
			ch->pcdata->skill_level[sn] = -1;
			ch->pcdata->unlocked[sn] = false;
		}
	}
	for(sn = 0;sn<MAX_SKILL;sn++)
		check_skill_unlock(ch,sn);
}

void gain_skill(CHAR_DATA *ch,int sn){
	ch->pcdata->skill_level[sn]++;
	ch->pcdata->learned[sn] = UMAX(5,ch->pcdata->learned[sn]);
	burn_subgains(ch,sn);
	if(ch->pcdata->skill_level[sn] == 1){
		printf_to_char(ch,"You gained the skill '{C%s{x'!\n\r",skill_table[sn].name);
		act("$n gained the skill '{C$T{x'!",ch,NULL,skill_table[sn].name,TO_ROOM);
		printf_to_char(ch,"You studied the skill '{C%s{x' to level {R%d{x!\n\r",skill_table[sn].name,ch->pcdata->skill_level[sn]);
		act("$n studied the skill '{C$T{x'!",ch,NULL,skill_table[sn].name,TO_ROOM);
	}
	else{
		printf_to_char(ch,"You studied the skill '{C%s{x' to level {R%d{x!\n\r",skill_table[sn].name,ch->pcdata->skill_level[sn]);
		act("$n studied the skill '{C$T{x'!",ch,NULL,skill_table[sn].name,TO_ROOM);
	}
	check_skill_unlock(ch,sn);
	check_skill_lock(ch,sn);
}

void do_change(CHAR_DATA *ch,char *argument){
	char buf[MIL];
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
	if(classes[get_class(ch)].ctier == 3){
		ch->send("You are already at the highest level of training.\n\r");
		return;
	}
	if(!classes[get_class(ch)].becomes[trainer->trainer]){
		ch->send("This trainer cannot help you advance.\n\r");
		return;
	}

	switch(classes[get_class(ch)].ctier){
	case 0:
		if(ch->level != 10){ch->send("You must be at least level 10 in order to proceed.\n\r");	return;}
		else break;
	case 1:
		if(ch->level != 25){ch->send("You must be at least level 25 in order to proceed.\n\r");	return;}
		else break;
	case 2:
		if(ch->level != 50){ch->send("You must be at least level 50 in order to proceed.\n\r");return;}
		else break;
	default:
		ch->send("You cannot change jobs.\n\r");return;
	}

	if(pc_race_table[ch->race].class_use[trainer->trainer] != 1){
		ch->send("This class is unavailable to your race.\n\r");
		return;
	}
	if(!classes[trainer->trainer].active){
		ch->send("That class is currently disabled.\n\r");
		return;
	}

	if(argument[0]){
		if(ch->pcdata->change_ready){
			ch->pcdata->change_ready = false;
			ch->send("Cancelling job change request.\n\r");
		}
		else
			ch->send("The syntax is just 'change'.\n\r");
		return;
	}
	if(!ch->pcdata->change_ready){
		sprintf(buf,"%s %s",ch->name,trainer->icpmsg);do_function(trainer,&do_sayto,buf);
		sprintf(buf,"%s",trainer->ocpmsg);do_function(trainer,&do_osay,buf);
		ch->pcdata->change_ready = true;
		return;
	}
	else{
		if(ch->pcdata->studies > 0){
			printf_to_char(ch,"Your remaining %d study points are being stored for use on the %s class.\n\r",classes[get_class(ch)].name);
			ch->pcdata->s_studies[classes[get_class(ch)].ctier] += ch->pcdata->studies;
			ch->pcdata->studies = 0;
		}
		impose_job(ch,trainer->trainer);
		//ch->exp -= exp_per_level(ch);
		sprintf(buf,"%s %s",ch->name,trainer->cmsg);do_function(trainer,&do_sayto,buf);
		ch->pcdata->studies += classes[get_class(ch)].ctier * 3;
		ch->printf("You've been rewarded %d study points to get you started.\n\r",ch->pcdata->studies);
		ch->pcdata->change_ready = false;
	}
}

/* used to get new skills */
void do_gain(CHAR_DATA *ch, char *argument){
	CHAR_DATA *trainer;
	char buf[MIL];
	bool found = false,zfound = false;
	int sn,slvl,cols;

	if(IS_NPC(ch)){
		ch->send("Cute...\n\r");
		return;
	}

	if(!argument[0] || !str_prefix(argument,"list")){
		ch->send("{d/-|{CMastered Skills{d|---------------------------------------------------------\\\n\r");
		cols = 0;
		for(sn = 0;sn<MAX_SKILL && skill_table[sn].name && !IS_SKILL_NATURAL(sn);sn++){
			if(ch->pcdata->unlocked[sn] == false || ch->pcdata->skill_level[sn] < 5)
				continue;
			printf_to_char(ch,"%s {G%d {x%-22s",cols == 0 ? "|" : "",ch->pcdata->skill_level[sn],skill_table[sn].name);
			found = true;
			if(++cols > 2){
				ch->send("{d|\n\r");
				cols = 0;
			}
		}
		if(!found)
			ch->send("| None                                                                      |\n\r");
		else{
			if(cols == 1)
				printf_to_char(ch,"%50s{d|\n\r","");
			else if(cols == 2)
				printf_to_char(ch,"%25s{d|\n\r","");
			else if(cols == 0)
				;//printf_to_char(ch,"%72s{d|\n\r","");
			cols = 0;
		}
		ch->send("{d\\___________________________________________________________________________/\n\r");
		ch->send("/-|{GGained Skills{d|-----------------------------------------------------------\\\n\r");
		for(int l = 0;l < 5;l++){
			zfound = false;
			cols = 0;
			for(sn = 0;sn<MAX_SKILL && skill_table[sn].name && !IS_SKILL_NATURAL(sn);sn++){
				if(ch->pcdata->unlocked[sn] == false || ch->pcdata->skill_level[sn] >= 5 || ch->pcdata->skill_level[sn] < 1)
					continue;
				if(ch->pcdata->skill_level[sn] > 0 && ch->pcdata->skill_level[sn] < 5 && get_tier(ch,sn) == l){
					if(!zfound){
						zfound = true;
						printf_to_char(ch,"|{xTier {G%d{d:                                                                    |\n\r",l);
					}
					printf_to_char(ch,"%s {R%d {x%-22s",cols == 0 ? "|" : "",ch->pcdata->skill_level[sn],skill_table[sn].name);
					found = true;
					if(++cols > 2){
						ch->send("{d|\n\r");
						cols = 0;
					}
				}
			}
			if(cols == 1)
				printf_to_char(ch,"%50s{d|\n\r","");
			else if(cols == 2)
				printf_to_char(ch,"%25s{d|\n\r","");
			else if(cols == 0)
				;//printf_to_char(ch,"%72s{d|\n\r","");
		}
		if(!found)
			ch->send("| None                                                                      |\n\r");
		else{
			if(cols == 1)
				printf_to_char(ch,"%50s{d|\n\r","");
			else if(cols == 2)
				printf_to_char(ch,"%25s{d|\n\r","");
			else if(cols == 0)
				;//printf_to_char(ch,"%72s{d|\n\r","");
			cols = 0;
		}
		ch->send("{d\\___________________________________________________________________________/\n\r");
		ch->send("/-|{RUnGained Skills{d|---------------------------------------------------------\\\n\r");
		cols = 0;
		found = false;
		for(int l = 0;l < 4;l++){
			zfound = false;
			cols = 0;
			for(sn = 0;sn<MAX_SKILL;sn++){
				if (!skill_table[sn].name || IS_SKILL_NATURAL(sn))
					break;
				if(ch->pcdata->unlocked[sn] == false || ch->pcdata->skill_level[sn] > 0)
					continue;
				if(get_tier(ch,sn) == l){
					if(!zfound){
						zfound = true;
						printf_to_char(ch,"|{xTier {G%d{d:                                                                    |\n\r",l);
					}
					printf_to_char(ch,"%s {R%d {x%-22s",cols == 0 ? "|" : "",ch->pcdata->skill_level[sn],skill_table[sn].name);
					found = true;
					if(++cols > 2){
						ch->send("{d|\n\r");
						cols = 0;
					}
				}
			}
			if(cols == 1)
				printf_to_char(ch,"%50s{d|\n\r","");
			else if(cols == 2)
				printf_to_char(ch,"%25s{d|\n\r","");
			else if(cols == 0)
				;//printf_to_char(ch,"%72s{d|\n\r","");
		}
		if(!found)
			ch->send("| None                                                                      |\n\r");
		else{
			if(cols == 1)
				printf_to_char(ch,"%50s{d|\n\r","");
			else if(cols == 2)
				printf_to_char(ch,"%25s{d|\n\r","");
			else if(cols == 0)
				;//printf_to_char(ch,"%72s{d|\n\r","");
			cols = 0;
		}
		ch->send("{d\\___________________________________________________________________________/\n\r");
		ch->send("/-|{rPreRequisites Not Met{d|---------------------------------------------------\\\n\r");
		found = false;
		for(sn = 0;sn<MAX_SKILL;sn++){
			if (!skill_table[sn].name || IS_SKILL_NATURAL(sn))
				break;
			if(ch->pcdata->skill_level[sn] == 0 && ch->pcdata->unlocked[sn] == false){
				if(cols == 0)
					ch->send("|");
				printf_to_char(ch," {d0{x %-22s",skill_table[sn].name);
				found = true;
				if(++cols > 2){
					ch->send("{d|\n\r");
					cols = 0;
				}
			}
		}
		if(!found)
			ch->send("| None                                                                      |\n\r");
		else{
			if(cols == 1)
				printf_to_char(ch,"%50s{d|\n\r","");
			else if(cols == 2)
				printf_to_char(ch,"%25s{d|\n\r","");
			else if(cols == 0)
				;//printf_to_char(ch,"%72s{d|\n\r","");
			cols = 0;
		}
		ch->send("{d\\___________________________________________________________________________/\n\r{d");
		printf_to_char(ch,"\n\r{xYou have {R%d {xskill points.\n\r",ch->pcdata->studies);
		if(ch->pcdata->s_studies[0] > 0 || ch->pcdata->s_studies[1] > 0 || ch->pcdata->s_studies[2] > 0 || ch->pcdata->s_studies[3] > 0)
			printf_to_char(ch,"You have {R%d {xtier0, {R%d{x tier1, {R%d{x tier2, {R%d{x tier3 saved skill points.\n\r",
				ch->pcdata->s_studies[0],ch->pcdata->s_studies[1],ch->pcdata->s_studies[2],ch->pcdata->s_studies[3]);
	}
	else{
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
		if(IS_SKILL_NATURAL(sn)){
			ch->send("You cannot improve natural skills.\n\r");
			return;
		}
		if(ch->pcdata->skill_level[sn] >= 0 && ch->pcdata->unlocked[sn] == true){
			if(ch->pcdata->skill_level[sn] >= 5){
				ch->send("That skill is already at max level.\n\r");
				return;
			}
			if(ch->pcdata->studies < 1 && get_subgains(ch,sn) < 1){
				ch->send("You do not have enough studies.\n\r");
				return;
			}
			gain_skill(ch,sn);
		}
		else{
			int tsn,n;
			if(ch->pcdata->skill_level[sn] == 0){
				ch->send("You have not met the prerequisites for that skill:\n\r");
				for(n = 0; n < 9;n++){
					if(!(tsn = skill_table[sn].skl[ch->pclass].unlock_by[n].sn))
						break;
					printf_to_char(ch," -{C%s {xlevel {R%d{x\n\r",skill_table[tsn].name,skill_table[sn].skl[ch->pclass].unlock_by[n].level);
				}
			}
			else
				ch->send("That skill is not available to you.\n\r");
			return;
		}
	}
}

void reset_skills(CHAR_DATA *ch){
	if(IS_NPC(ch))
		return;

	/*for(int iWear = 0;iWear < MAX_WEAR; iWear++){
		if((get_eq_char(ch,iWear)) && (get_eq_char(ch,iWear)->wear_loc != WEAR_TATTOO) && (get_eq_char(ch,iWear)->wear_loc != WEAR_FLOAT_LIGHT)){
			ch->send("You must be naked to do this.\n\r");
			return;
		}
	}*/

	for(int i = 0;i < MAX_SKILL;i++){
		ch->pcdata->learned[i] = 0;
		ch->pcdata->skill_level[i] = 0;
		ch->pcdata->unlocked[i] = false;
	}
	impose_job(ch,ch->pclass);
	ch->pcdata->studies = 0;
	if(ch->level < 11){
		ch->pcdata->s_studies[0] = (ch->level / 2) + (ch->level / 5);
	}
	else if(ch->level < 26){
		ch->pcdata->s_studies[0] = (10 / 2) + (10 / 5);
		ch->pcdata->s_studies[1] = ((ch->level - 10) / 2) + ((ch->level - 10) / 5);
	}
	else if(ch->level < 51){
		ch->pcdata->s_studies[0] = (10 / 2) + (10 / 5);
		ch->pcdata->s_studies[1] = (15 / 2) + (15 / 5);
		ch->pcdata->s_studies[2] = ((ch->level - 25) / 2) + ((ch->level - 25) / 5);
	}
	else if(ch->level < 106){
		ch->pcdata->s_studies[0] = (10 / 2) + (10 / 5);
		ch->pcdata->s_studies[1] = (15 / 2) + (15 / 5);
		ch->pcdata->s_studies[2] = (25 / 2) + (25 / 5);
		ch->pcdata->s_studies[3] = ((ch->level - 50) / 2) + ((ch->level - 50) / 5);
	}
	else{
		ch->send("Uhhh what?\n\r");
		return;
	}
	ch->pcdata->up_skill(5,100,gsn_recall);
	ch->pcdata->up_skill(1,classes[CLASS_TENDERFOOT].skill_adept,gsn_clubs);
	for (int i = 0; i < 5; i++)
		if(pc_race_table[ch->race].skills[i] && str_cmp(pc_race_table[ch->race].skills[i],"none"))
			ch->pcdata->up_skill(5,100,skill_lookup(pc_race_table[ch->race].skills[i]));

	ch->pcdata->practices += ch->pcdata->s_practices;
	printf_to_char(ch,"Refunding {R%d {xtier0 studies, {R%d {xtier1 studies, {R%d {xtier2 studies, {R%d {xtier3 studies, and {B%d {xpractices\n\r",
		ch->pcdata->s_studies[0],ch->pcdata->s_studies[1],ch->pcdata->s_studies[2],ch->pcdata->s_studies[3],ch->pcdata->s_practices);
	ch->pcdata->s_practices = 0;
}