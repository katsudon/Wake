#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include "merc.h"
#include "interp.h"
#include "magic.h"
#include "recycle.h"
#include "lookup.h"
#include "olc.h"
#include "db.h"

char * const where_name []={
    "{c<{wused as light{c>{x     ",
    "{c<{wworn on head{c>{x      ",
    "{c<{wworn in the ear{c>{x   ",
    "{c<{wworn in the ear{c>{x   ",
    "{c<{wworn on face{c>{x      ",
    "{c<{wworn around neck{c>{x  ",
    "{c<{wworn around neck{c>{x  ",
    "{c<{wworn as a quiver{c>{x  ",
    "{c<{wover the shoulder{c>{x ",
    "{c<{wworn as a pendant{c>{x ",
    "{c<{wworn about body{c>{x   ",
    "{c<{wworn on torso{c>{x     ",
	"{W({Rlodged in a rib{W){x   ",
    "{c<{wtattooed on{c>{x       ",
    "{c<{wworn on arms{c>{x      ",
	"{W({Rlodged in an arm{W){x  ",
    "{c<{wworn on the elbow{c>{x ",
    "{c<{wworn on the elbow{c>{x ",
    "{c<{wworn on the wrist{c>{x ",
    "{c<{wworn on the wrist{c>{x ",
    "{c<{wworn on hands{c>{x     ",
    "{c<{wworn on finger{c>{x    ",
    "{c<{wworn on finger{c>{x    ",
    "{c<{wwield{c>{x             ",
    "{c<{wdual wield{c>{x        ",
    "{c<{wheld{c>{x              ",
    "{c<{wheld{c>{x              ",
    "{c<{wworn as shield{c>{x    ",
    "{c<{wworn about waist{c>{x  ",
    "{c<{wworn on legs{c>{x      ",
	"{W({Rlodged in a knee{c>{x  ",
    "{c<{wworn on the knee{c>{x  ",
    "{c<{wworn on the knee{c>{x  ",
    "{c<{wworn on the shin{c>{x  ",
    "{c<{wworn on the shin{c>{x  ",
    "{c<{wworn on the ankle{c>{x ",
    "{c<{wworn on the ankle{c>{x ",
    "{c<{wworn on feet{c>{x      ",
    "{c<{wfloating nearby{c>{x   ",
    "{c<{wfloating nearby{c>{x   "
};

/*
 * Local functions.
 */
bool	check_instinct		( CHAR_DATA* );
bool	check_thalassic		( CHAR_DATA* );
void	GenerateDetailMap	( CHAR_DATA *ch, char *descr );
char *	format_obj_to_char	( OBJ_DATA*,CHAR_DATA*,bool );
void	look_sky			( CHAR_DATA* );
void	show_list_to_char	( OBJ_DATA*,CHAR_DATA*,bool,bool );
void	show_char_to_char_0	( CHAR_DATA*,CHAR_DATA* );
void	show_char_to_char_1	( CHAR_DATA*,CHAR_DATA*,bool );
void	show_char_to_char	( CHAR_DATA*,CHAR_DATA* );
bool	check_blind			( CHAR_DATA* );
void	check_max_con		( );
void	display_map			( CHAR_DATA* );


char *format_obj_to_char(OBJ_DATA *obj,CHAR_DATA *ch,bool fShort){
	static char buf[MSL];
	char buf2[10];

	buf[0] = '\0';

	if ((fShort && (obj->short_descr == NULL || obj->short_descr[0] == '\0')) || (obj->description == NULL || obj->description[0] == '\0'))
		return buf;

	if (IS_OBJ_STAT(obj,ITM_INVIS))									strcat(buf,"{x({CInvisible{x) ");
	if (IS_OBJ_STAT(obj,ITM_HIDDEN))								strcat(buf,"{x({dHidden{x) ");
	if (IS_OBJ_STAT(obj,ITM_EVIL))									strcat(buf,"{x({RRed Aura{x) "  );
	if (IS_OBJ_STAT(obj,ITM_INLAY))									strcat(buf,"{x({yInlayed{x) "  );
	if (IS_OBJ_STAT(obj,ITM_GLOW))									strcat(buf,"{x({CGlowing{x) ");
	if (IS_OBJ_STAT(obj,ITM_HUM))									strcat(buf,"{x({cHumming{x) ");
	if (IS_OBJ_STAT(obj,ITM_FLAMING))								strcat(buf,"{x({RBurning{x) ");
	if (IS_OBJ_STAT(obj,ITM_FROSTED))								strcat(buf,"{x({CFrosted{x) ");
	if (IS_OBJ_STAT(obj,ITM_BARBED))								strcat(buf,"{x({dB{xa{drb{xe{dd{x) ");
	if (IS_OBJ_STAT(obj,ITM_SHOCKED))								strcat(buf,"{x({YElectric{x) ");
	if (IS_OBJ_STAT(obj,ITM_FORGING))								strcat(buf,"{x({RGlowing{x) ");
	if(obj->item_type == ITEM_ARROW && obj->value[3] > 0){
		sprintf(buf2,"({R%d{x) ",obj->value[3]+1);
		strcat(buf,buf2);
	}

	if (ch->isaff(AF_DETECT_MAGIC) && IS_OBJ_STAT(obj,ITM_MAGIC))	strcat(buf,"{x({RM{Ya{Mg{Bi{Cc{Ga{Rl{x) ");

	if (fShort){
		if (obj->short_descr)
			strcat(buf,obj->short_descr);
	}
	else{
		if (obj->description)
			strcat(buf,obj->description);
	}

	return buf;
}

/*
 * Show a list to a character.
 * Can concatenate duplicated items.
 */
void show_list_to_char( OBJ_DATA *list, CHAR_DATA *ch, bool fShort, bool fShowNothing ){
	OBJ_DATA *obj;
	BUFFER *output;
	char buf[MSL],**prgpstrShow,*pstrShow;
	int nShow,*prgnShow,iShow,count;
	bool fCombine;

	if (ch->desc == NULL)
		return;

	// Alloc space for output lines.
	output = new_buf();

	count = 0;
	for ( obj = list; obj != NULL; obj = obj->next_content )
		count++;
	prgpstrShow	= (char**)alloc_mem(count * sizeof(char*));
	prgnShow    = (int*)alloc_mem(count * sizeof(int));
	nShow		= 0;

	/*
	 * Format the list of objects.
	 */
	for (obj = list;obj != NULL;obj = obj->next_content){
		if (obj->wear_loc == WEAR_NONE && can_see_obj(ch,obj) && (!IS_OBJ_STAT(obj,ITM_INLAY) || ch->isplr(PL_HOLYLIGHT))){
			pstrShow = format_obj_to_char(obj,ch,fShort);

			fCombine = false;

			if (IS_NPC(ch) || ch->iscomm(CM_COMBINE)){
				// Look for duplicates, case sensitive.
				for (iShow = nShow - 1;iShow >= 0;iShow--){
					if (!strcmp(prgpstrShow[iShow],pstrShow)){
						prgnShow[iShow]++;
						fCombine = true;
						break;
					}
				}
			}

			// Couldn't combine, or didn't want to.
			if (!fCombine){
				prgpstrShow [nShow] = str_dup(pstrShow);
				prgnShow    [nShow] = 1;
				nShow++;
			}
		}
	}

	// Output the formatted list.
	for (iShow = 0;iShow < nShow;iShow++){
		if (!prgpstrShow[iShow][0]){
			free_string(prgpstrShow[iShow]);
			continue;
		}

		if (IS_NPC(ch) || ch->iscomm(CM_COMBINE)){
			if (prgnShow[iShow] != 1){
				sprintf(buf,"(%2d) ",prgnShow[iShow]);
				add_buf(output,buf);
			}
			else
				add_buf(output,"    ");
		}
		add_buf(output,prgpstrShow[iShow]);
		add_buf(output,"\n\r");
		free_string(prgpstrShow[iShow]);
	}
	//show magical items where applicable
	for (obj = list;obj != NULL;obj = obj->next_content){
		if(obj->wear_loc == WEAR_NONE
		&& !can_see_obj(ch,obj)
		&& (!IS_OBJ_STAT(obj,ITM_INLAY) || ch->isplr(PL_HOLYLIGHT))
		&& (IS_OBJ_STAT(obj,ITM_MAGIC) || IS_OBJ_STAT(obj,ITM_INVIS))){
			if (ch->isaff(AF_DETECT_MAGIC)){
				sprintf(buf,"    {wA magically obscured object.{x\n\r");
				add_buf(output,buf);
			}
		}
	}

	if (fShowNothing && nShow == 0){
		if (IS_NPC(ch) || ch->iscomm(CM_COMBINE))
			send_to_char("     ",ch);
		send_to_char("Nothing.\n\r",ch);
	}
	page_to_char(buf_string(output),ch);

	// Clean up.
	free_buf(output);
	free_mem(prgpstrShow,count * sizeof(char *));
	free_mem(prgnShow,count * sizeof(int));
}

void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch ){
	int i;
	char buf[MSL],message[MSL];

	buf[0] = '\0';
	if (RIDDEN(victim)){
		if (ch != RIDDEN(victim))
			strcat(buf,"(Ridden) ");
		else
			strcat(buf,"(Your mount) ");
	}

	if(victim->iscomm(CM_AFK)){
		if(!IS_NPC(victim) && victim->pcdata->afk[0])	strcat(buf,"{x[{YAFK{x] ");
		else											strcat(buf,"{x[{yAFK{x] ");
	}
		 if(victim->iscomm(CM_BUSY)								) 	strcat(buf,"{x[{YIN OLC{x] ");
		 if(victim->isaff(AF_INVISIBLE) /*||
		(victim->isaff(AF_PHASE) && number_range(1,6) > affect_find(victim->affected,gsn_phase)->modifier)*/) 	strcat(buf,"{x({CInvisible{x) ");
		 if(victim->invis_level >=	LEVEL_HERO					) 	strcat(buf,"{x({GWizi{x) ");
		 if(victim->isaff(AF_HIDE) && !victim->isaff(AF_SNEAK)	) 	strcat(buf,"{x({wHide{x) ");
	else if(!victim->isaff(AF_HIDE) && victim->isaff(AF_SNEAK)	) 	strcat(buf,"{x({wSneak{x) ");
	else if(victim->isaff(AF_HIDE) && victim->isaff(AF_SNEAK)	) 	strcat(buf,"{x({dHide{x) ");
		 if(victim->isaff(AF_PERFECTSNEAK)						)	strcat(buf,"{x({dVeiled{x) ");
		 if(victim->isaff(AF_CAMOFLAGE)							) 	strcat(buf,"{x({yBlending{x) ");
		 if(victim->isaff(AF_CHARM)								)	strcat(buf,"{x({mCharmed{x) ");
		 if(victim->isaff(AF_PHASE)							)	strcat(buf,"{x({CTranslucent{x) ");
		 if(victim->isaff(AF_WINDWALK)							)	strcat(buf,"{x({wFloating{x) ");
		 if(victim->isaff(AF_FAERIE_FIRE)						)	strcat(buf,"{x({MPink Aura{x) ");
		 if(victim->isaff(AF_SANCTUARY)							)	strcat(buf,"{x({wWhite Aura{x) ");
		 if(!IS_NPC(victim) && victim->isplr(PL_KILLER)			)	strcat(buf,"{x({RKILLER{x) ");
		 if(!IS_NPC(victim) && victim->isplr(PL_THIEF)			)	strcat(buf,"{x({YTHIEF{x) ");
		 if(check_thalassic(victim))									strcat(buf,"({RGlowing{x) ");

	if(victim->position == victim->start_pos && !victim->fighting && victim->long_descr[0] != '\0'){
		strcat(buf,victim->long_descr);
		send_to_char(buf,ch);
		return;
	}

	if(victim->iscomm(CM_MORPH)){
		strcat(buf,victim->long_descr);
		send_to_char(buf,ch);
		return;
	}
	if(IS_NPC(ch))
		strcat(buf,capitalize(PERS(victim,ch)));
	else
		strcat(buf,PERS(victim,ch));

	if(victim->fighting){
		strcat(buf," is here, fighting ");
		if (!victim->fighting)
			strcat(buf,"thin air??");
		else if (victim->fighting == ch)
			strcat(buf,"{YYOU{x!");
		else if (victim->in_room == victim->fighting->in_room){
			strcat(buf,PERS(victim->fighting,ch));
			strcat(buf,".");
		}
		else
			strcat(buf,"someone who left??");
	}
	else{
		if (!IS_NPC(victim) && !ch->iscomm(CM_BRIEF) && victim->position == POS_STANDING && !victim->on && !victim->fighting){
			if(IS_VOWELL(race_table[victim->race].name[0]))
				strcat(buf,", an ");
			else
				strcat(buf,", a ");
			strcat(buf, race_table[victim->race].name);
			strcat(buf," with ");
			if (hair_table[victim->looks[P_HAIR]].name != NULL){
				if (victim->looks[P_SHAIR] == S_HAIR_BALD)
					strcat(buf,"a bald head and ");
				else if (victim->looks[P_SHAIR] == S_HAIR_SHAVED)
					strcat(buf,"a shaved head and ");
				else{
					strcat(buf,hair_table[victim->looks[P_HAIR]].name);
					strcat(buf," hair and ");
				}
			}
			else
				strcat(buf,"{Rb{Cu{Bg{Gg{Ye{Md{x");
			if (eye_table[victim->looks[P_EYE]].name != NULL)
				strcat(buf,eye_table[victim->looks[P_EYE]].name);
			else
				strcat(buf,"{Rb{Cu{Bg{Gg{Ye{Md{x");
			strcat(buf," eyes");
		}
		switch (victim->position){
			case POS_DEAD:     strcat(buf," is {RDEAD{x!!");              break;
			case POS_MORTAL:   strcat(buf," is {rmortally wounded{x.");   break;
			case POS_INCAP:    strcat(buf," is {dincapacitated{x.");      break;
			case POS_STUNNED:  strcat(buf," is lying here {ystunned{x."); break;
			case POS_SLEEPING: 
				if (victim->on != NULL){
					if (IS_SET(victim->on->value[2],SLEEP_AT)){
						sprintf(message," is sleeping at %s.", victim->on->short_descr);
						strcat(buf,message);
					}
					else if (IS_SET(victim->on->value[2],SLEEP_ON)){
						sprintf(message," is sleeping on %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else if (IS_SET(victim->on->value[2],SLEEP_UNDER)){
						sprintf(message," is sleeping under %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else{
						sprintf(message," is sleeping in %s.",victim->on->short_descr);
						strcat(buf,message);
					}
				}
				else 
					strcat(buf," is sleeping here.");
				break;
			case POS_RESTING:  
				if (victim->on != NULL){
					if (IS_SET(victim->on->value[2],REST_AT)){
						sprintf(message," is resting at %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else if (IS_SET(victim->on->value[2],REST_ON)){
						sprintf(message," is resting on %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else if (IS_SET(victim->on->value[2],REST_UNDER)){
						sprintf(message," is resting under %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else{
						sprintf(message," is resting in %s.",victim->on->short_descr);
						strcat(buf,message);
					}
				}
				else
					strcat(buf," is resting here.");
				break;
			case POS_SITTING:  
				if (victim->on != NULL){
					if (IS_SET(victim->on->value[2],SIT_AT)){
						sprintf(message," is sitting at %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else if (IS_SET(victim->on->value[2],SIT_ON)){
						sprintf(message," is sitting on %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else if (IS_SET(victim->on->value[2],SIT_UNDER)){
						sprintf(message," is sitting under %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else{
						sprintf(message," is sitting in %s.",victim->on->short_descr);
						strcat(buf,message);
					}
				}
				else
					strcat(buf," is sitting here.");
				break;
			case POS_STANDING: 
				if (victim->on){
					if (IS_SET(victim->on->value[2],STAND_AT)){
						sprintf(message," is standing at %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else if (IS_SET(victim->on->value[2],STAND_ON)){
						sprintf(message," is standing on %s.",victim->on->short_descr);
						strcat(buf,message);
					}
					else{
						sprintf(message," is standing in %s.",victim->on->short_descr);
						strcat(buf,message);
					}
				}
				else if(MOUNTED(victim)){
					strcat(buf," is here, riding ");
					strcat(buf,MOUNTED(victim)->short_descr);
					strcat(buf,".");
				}
				else
					strcat(buf," is here.");
				break;
			case POS_FIGHTING:;
				strcat(buf," Whoah bug?");
				break;
		}
	}

	strcat(buf,"\n\r");
	buf[0] = UPPER(buf[0]);
	ch->send(buf);
	if (victim->isaff(AF_MIRAGE))
		for (i=1;i<4;i++)
			ch->send(buf);
}

void show_char_to_char_1(CHAR_DATA *victim,CHAR_DATA *ch,bool isexamine){
	OBJ_DATA *obj;
	char buf[MSL];
	int iWear, percent;
	bool found;

	if ( can_see(victim,ch) ){
		if (ch == victim)
			act("$n looks at $mself.",ch,NULL,NULL,TO_ROOM);
		else{
			if (isexamine){
				act("$n examines you.",ch,NULL,victim,TO_VICT);
				act("$n examines $N.",ch,NULL,victim,TO_NOTVICT);
			}
			else{
				act("$n looks at you.",ch,NULL,victim,TO_VICT);
				act("$n looks at $N.",ch,NULL,victim,TO_NOTVICT);
			}
		}
	}

	if ( victim->description[0] != '\0' )
		send_to_char(victim->description,ch);
	else
		act("You see nothing special about $M.",ch,NULL,victim,TO_CHAR);

	if ( MOUNTED(victim) )
		printf_to_char(ch,"%s is riding %s.\n\r",victim->name,MOUNTED(victim)->short_descr);
	if ( RIDDEN(victim) )
		printf_to_char(ch,"%s is being ridden by %s.\n\r",victim->short_descr,RIDDEN(victim)->name);

	if ( victim->max_hit > 0 )
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	send_to_char("{x",ch);
	strcpy(buf,PERS(victim,ch));

		 if (percent >= 100)	strcat(buf," is in {Gexcellent {xcondition.\n\r");
	else if (percent >= 90)		strcat(buf," has a {gfew scratches{x.\n\r");
	else if (percent >= 75)		strcat(buf," has some {gsmall wounds and bruises{x.\n\r");
	else if (percent >=  50)	strcat(buf," has {Yquite a few wounds{x.\n\r");
	else if (percent >= 30)		strcat(buf," has some {ybig nasty wounds{x.\n\r");
	else if (percent >= 15)		strcat(buf," looks {Rpretty hurt{x.\n\r");
	else if (percent >= 0 )		strcat(buf," is in {rawful {xcondition.\n\r");
	else						strcat(buf," is bleeding to death.\n\r");

	buf[0] = UPPER(buf[0]);
	send_to_char(buf,ch);

	if (isexamine){
		found = false;
		for ( iWear = 0; iWear < MAX_WEAR; iWear++ ){
			if ((obj = get_eq_char(victim,iWear)) && can_see_obj(ch,obj)){
				if (!found){
					send_to_char("\n\r",ch);
					act("$N is using:",ch,NULL,victim,TO_CHAR);
					found = true;
				}
				send_to_char(where_name[iWear],ch);
				send_to_char(format_obj_to_char(obj,ch,true),ch);
				send_to_char("\n\r",ch);
			}
		}

		if (victim != ch){
			if(!IS_NPC(ch)){
				if (number_percent() < get_skill(ch,gsn_peek) * .8){
					if(!IS_NPC(victim) || (IS_NPC(victim) && !victim->isact(AT_NOPEEK))){
						send_to_char("\n\rYou peek at the inventory:\n\r",ch);
						check_improve(ch,gsn_peek,true,4);
						show_list_to_char(victim->carrying,ch,true,true);
					}
				}
			}
		}
	}
	else if (!IS_NPC(victim)){
		if (victim->looks[P_SHAIR] == S_HAIR_BALD)
			sprintf(buf,"a bald head");
		else if (victim->looks[P_SHAIR] == S_HAIR_SHAVED)
			sprintf(buf,"a shaved head");
		else
			sprintf(buf,"%s, %s hair",sub_hair_table[victim->looks[P_SHAIR]].name,hair_table[victim->looks[P_HAIR]].name);
		if(!victim->description[0]){
			printf_to_char(ch,"\n\r%s is %s %s,%s%s %s %s.\n\r%s has %s and %s eyes.\n\r",
				PERS(victim,ch),
				IS_VOWELL(height_table[victim->looks[P_HEIGHT]].name[0]) ? "an" : "a",
				height_table[victim->looks[P_HEIGHT]].name,
				victim->looks[P_HEIGHT] == HEIGHT_AVERAGE && victim->looks[P_WEIGHT] == WEIGHT_AVERAGE ? "" : " ",
				victim->looks[P_HEIGHT] == HEIGHT_AVERAGE && victim->looks[P_WEIGHT] == WEIGHT_AVERAGE ? "" : weight_table[victim->looks[P_WEIGHT]].name,
				sex_table[victim->sex].name,
				race_table[victim->race].name,
				capitalize(sex_table[victim->sex].eword),
				buf,
				eye_table[victim->looks[P_EYE]].name);
		}
		else{
			printf_to_char(ch,"\n\r%s is a %s with %s and %s eyes.\n\r",
				PERS(victim,ch),
				race_table[victim->race].name,
				buf,
				eye_table[victim->looks[P_EYE]].name);
		}
	}
}

void show_char_to_char( CHAR_DATA *list, CHAR_DATA *ch ){
    CHAR_DATA *rch;

	for(rch = list;rch;rch = rch->next_in_room){
		if(rch == ch || ( RIDDEN(rch) && rch->in_room == RIDDEN(rch)->in_room && RIDDEN(rch) != ch))
			continue;

		if(get_trust(ch) < rch->invis_level)
			continue;

		if(can_see(ch,rch)){
			show_char_to_char_0(rch,ch);
			if(MOUNTED(rch) && (rch->in_room == MOUNTED(rch)->in_room))
				show_char_to_char_0(MOUNTED(rch),ch);
			if(!IS_NPC(ch) && IS_DRUNK(ch) && number_percent() < ch->pcdata->condition[COND_DRUNK]){
				show_char_to_char_0(rch,ch);
				if(MOUNTED(rch) && (rch->in_room == MOUNTED(rch)->in_room))
					show_char_to_char_0(MOUNTED(rch),ch);
			}
		}
		else{
			if(room_is_dark(ch->in_room) && ch->isaff(AF_INFRARED))
				send_to_char("You see glowing red eyes watching YOU!\n\r",ch);
			else if(rch->isaff(AF_INVISIBLE) && ch->isaff(AF_DETECT_MAGIC)){
				AFFECT_DATA *af = affect_find(ch->affected,AF_DETECT_MAGIC);
				if((af && number_range(1,10) <= af->modifier) || number_percent() <= 50)
					ch->send("A mystically shrouded figure.\n\r");
			}
		}
	}
	if(ch->isaff(AF_AGUE))
		send_to_char("A little {Mpink quasit {xdances crazily around you.\n\r",ch);
} 

bool check_blind(CHAR_DATA *ch){
	if (!IS_NPC(ch) && ch->isplr(PL_HOLYLIGHT))
		return true;

	if (ch->isaff(AF_BLIND)){
		send_to_char("You can't see a thing!\n\r",ch);
		return false;
	}

	return true;
}

void do_scroll(CHAR_DATA *ch, char *argument){
	char arg[MIL];
	int lines;

	one_argument(argument,arg);

	if (arg[0] == '\0'){
		if (ch->lines == 0)
			send_to_char("You do not page long messages.\n\r",ch);
		else
			printf_to_char(ch,"You currently display %d lines per page.\n\r",ch->lines + 2);
		return;
	}

	if (!is_number(arg)){
		send_to_char("You must provide a number.\n\r",ch);
		return;
	}

	lines = atoi(arg);

	if (lines == 0){
		send_to_char("Paging disabled.\n\r",ch);
		ch->lines = 0;
		return;
	}

	if (lines < 10 || lines > 100){
		send_to_char("You must provide a reasonable number.\n\r",ch);
		return;
	}

	printf_to_char(ch,"Scroll set to %d lines.\n\r",lines);
	ch->lines = lines - 2;
}
static int socials_sort(const void *v1, const void *v2){
    int i1 = *(int *) v1, i2 = *(int *) v2;
    return strcmp(social_table[i1].name, social_table[i2].name);
}

void sort_socials(){
	return;
	int count = 0,social[maxSocial];
	// Prepare for the quicksort.
	for (count = 0; social_table[count].name[0] != '\0'; ++count)
		social[count] = count;
	// Here use a quick-sort to get them in order. Yay.
	qsort(social,count,sizeof(social_type),socials_sort);
}

void do_socials(CHAR_DATA *ch, char *argument){
    char *buf;
    int tcount = 0, match = 0, col = 0,count = 0,social[maxSocial];

    for (count = 0; social_table[count].name[0] != '\0'; ++count)
        social[count] = count;
	tcount = count;

    // Loop through all entrii of our newly aquisitioned social array.
    for (match = 0; match < tcount; ++match) {
        //Perfect fit.
        //count = strlen(social_table[social[match]].name) + 1;
        //if (count < 13)
            //count = 13; // Needs to be at least this size.. :)
        buf = (char*)malloc(tcount * sizeof(char));
        sprintf(buf, "%-12s", social_table[social[match]].name);
        send_to_char(buf, ch);
        if (++col % 6 == 0)
            send_to_char("\r\n", ch);
        free(buf);
    }
    if (col % 6 != 0)
        send_to_char ("\n\r", ch);
}

void do_motd(CHAR_DATA *ch, char *argument){
    do_function(ch,&do_help,"motd");
}

void do_imotd(CHAR_DATA *ch, char *argument){
    do_function(ch,&do_help,"imotd");
}

void do_rules(CHAR_DATA *ch, char *argument)
{
    do_function(ch,&do_help,"rules");
}

void do_story(CHAR_DATA *ch, char *argument){
    do_function(ch,&do_help,"story");
}

void do_wizlist(CHAR_DATA *ch, char *argument){
    do_function(ch,&do_help,"wizlist");
}

void do_autolist(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
	  return;

	send_to_char("   action     status\n\r",ch);
	send_to_char("---------------------\n\r",ch);
	printf_to_char(ch,"AutoExit       %s{x\n\r",ch->isplr(PL_AUTOEXIT) ? "{GON" : "{ROFF");
	printf_to_char(ch,"AutoWeather    %s{x\n\r",ch->isplr(PL_AUTOWEATHER) ? "{GON" : "{ROFF");
	printf_to_char(ch,"AutoGold       %s{x\n\r",ch->isplr(PL_AUTOGOLD) ? "{GON" : "{ROFF");
	printf_to_char(ch,"AutoLoot       %s{x\n\r",ch->isplr(PL_AUTOLOOT) ? "{GON" : "{ROFF");
	printf_to_char(ch,"AutoSac        %s{x\n\r",ch->isplr(PL_AUTOSAC) ? "{GON" : "{ROFF");
	printf_to_char(ch,"AutoMap        %s{x\n\r",ch->isplr(PL_AUTOMAP) ? "{GON" : "{ROFF");
	printf_to_char(ch,"AutoSplit      %s{x\n\r",ch->isplr(PL_AUTOSPLIT) ? "{GON" : "{ROFF");
	printf_to_char(ch,"AutoAssist     %s{x\n\r",ch->isplr(PL_AUTOASSIST) ? "{GON" : "{ROFF");
	printf_to_char(ch,"AutoExits      %s{x\n\r",!ch->iscomm(CM_ROMEXITS) ? "{GWAKE" : "{RROM");
	printf_to_char(ch,"AutoLook       %s{x\n\r",!ch->iscomm(CM_ROMLOOK) ? "{GWAKE" : "{RROM");
	printf_to_char(ch,"Compact Mode   %s{x\n\r",ch->iscomm(CM_COMPACT) ? "{GON" : "{ROFF");
	printf_to_char(ch,"Prompt         %s{x\n\r",ch->iscomm(CM_PROMPT) ? "{GON" : "{ROFF");
	printf_to_char(ch,"Combine Items  %s{x\n\r",ch->iscomm(CM_COMBINE) ? "{GON" : "{ROFF");
	if (IS_IMMORTAL(ch))
	printf_to_char(ch,"HolyLight      %s{x\n\r",ch->isplr(PL_HOLYLIGHT) ? "{GON" : "{ROFF");
	printf_to_char(ch,"NoLoot         %s{x\n\r",!ch->isplr(PL_CANLOOT) ? "{GON" : "{ROFF");
	printf_to_char(ch,"NoSummon       %s{x\n\r",ch->isplr(PL_NOSUMMON) ? "{GON" : "{ROFF");
	printf_to_char(ch,"NoFollow       %s{x\n\r",ch->isplr(PL_NOFOLLOW) ? "{GON" : "{ROFF");
}

void do_autoassist(CHAR_DATA *ch,char *argument){
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_AUTOASSIST))
		send_to_char("You will now automatically assist in combat.\n\r",ch);
	else{
		send_to_char("You will no longer assist automatically in combat.\n\r",ch);
		ch->remplr(PL_AUTOASSIST);
	}
}

void do_autoexit(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
		return;

	if (!argument[0]){
		if (ch->setplr(PL_AUTOEXIT))
			send_to_char("Exits will now be displayed. 'autoexit <rom/wake>' to toggle display.\n\r",ch);
		else{
			send_to_char("Exits will no longer be displayed. 'autoexit <rom/wake>' to toggle display.\n\r",ch);
			ch->remplr(PL_AUTOEXIT);
		}
		return;
	}
	if (!str_prefix(argument,"rom")){
		ch->send("ROM classic exits will be displayed.\n\r");
		ch->setcomm(CM_ROMEXITS);
	}
	else if (!str_prefix(argument,"wake")){
		ch->send("Wake exits will be displayed.\n\r");
		ch->remcomm(CM_ROMEXITS);
	}
}

void do_autolook(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
		return;

	if (!argument[0]){
		/*if (ch->setplr(PL_AUTOEXIT))
			send_to_char("Exits will now be displayed. 'autoexit <rom/wake>' to toggle display.\n\r",ch);
		else
		{
			send_to_char("Exits will no longer be displayed. 'autoexit <rom/wake>' to toggle display\n\r",ch);
			ch->remplr(PL_AUTOEXIT);
		}*/
		ch->send("Type 'autolook <rom/wake>' to toggle the look command.\n\r");
		return;
	}
	if (!str_prefix(argument,"wake")){
		ch->send("Wake looking will be used.\n\r");
		ch->setcomm(CM_ROMLOOK);
	}
	else if (!str_prefix(argument,"rom")){
		ch->send("You will use ROM looking style.\n\r");
		ch->remcomm(CM_ROMLOOK);
	}
}

void do_autogold(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_AUTOGOLD))
		send_to_char("Automatic gold looting set.\n\r",ch);
	else{
		send_to_char("Autogold removed.\n\r",ch);
		ch->remplr(PL_AUTOGOLD);
	}
}

void do_autoloot(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_AUTOLOOT))
		send_to_char("Automatic corpse looting set.\n\r",ch);
	else{
		send_to_char("Autolooting removed.\n\r",ch);
		ch->remplr(PL_AUTOLOOT);
	}
}

void do_automap(CHAR_DATA *ch,char *argument){
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_AUTOMAP))
		send_to_char("Automatic map display set.\n\r",ch);
	else{
		send_to_char("Automapping removed.\n\r",ch);
		ch->remplr(PL_AUTOMAP);
	}
}

void do_autosac(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_AUTOSAC))
		send_to_char("Automatic corpse sacrificing set.\n\r",ch);
	else{
		send_to_char("Autosacrificing removed.\n\r",ch);
		ch->remplr(PL_AUTOSAC);
	}
}

void do_autosplit(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_AUTOSPLIT))
		send_to_char("Automatic gold splitting set.\n\r",ch);
	else{
		send_to_char("Autosplitting removed.\n\r",ch);
		ch->remplr(PL_AUTOSPLIT);
	}
}

void do_autoweather(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_AUTOWEATHER))
		send_to_char("Automatic weather reporting set.\n\r",ch);
	else{
		send_to_char("Autoweather removed.\n\r",ch);
		ch->remplr(PL_AUTOWEATHER);
	}
}

void do_brief(CHAR_DATA *ch, char *argument){
	if (ch->setcomm(CM_BRIEF))
		send_to_char("Short descriptions activated.\n\r",ch);
	else{
		send_to_char("Full descriptions activated.\n\r",ch);
		ch->remcomm(CM_BRIEF);
	}
}

void do_compact(CHAR_DATA *ch, char *argument){
	if (ch->setcomm(CM_COMPACT))
		send_to_char("Compact mode set.\n\r",ch);
	else{
		send_to_char("Compact mode removed.\n\r",ch);
		ch->remcomm(CM_COMPACT);
	}
}

void do_show(CHAR_DATA *ch, char *argument){
	if (ch->setcomm(CM_SHOW_AFFECTS))
		send_to_char("Affects will now be shown in score.\n\r",ch);
	else{
		send_to_char("Affects will no longer be shown in score.\n\r",ch);
		ch->remcomm(CM_SHOW_AFFECTS);
	}
}

void do_prompt(CHAR_DATA *ch, char *argument){
	char buf[MSL],arg[MSL],arg2[MSL];

	argument = one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Syntax is: prompt <on/off/default>\n\r");
		ch->send("           prompt main <prompt>\n\r");
		ch->send("           prompt battle <brief/full/default/status>\n\r");
		ch->send("Type HELP PROMPT for a listing for main prompt.\n\r");
		return;
	}

	if(!strcmp(arg,"on")){
		ch->setcomm(CM_PROMPT);
		send_to_char("You will see prompts.\n\r",ch);
		return;
	}
	else if(!strcmp(arg,"off")){
		send_to_char("You will not see prompts.\n\r",ch);
		ch->remcomm(CM_PROMPT);
		return;
	}
	else if(!strcmp(arg,"default")){
		strcpy(buf,"{x[{G%h{x/{g%H{xhp {C%m{x/{c%M{xmp {B%v{xmv {Y%X{xtnl][{y%e{x]{x");
		free_string(ch->prompt);
		ch->prompt = str_dup(buf);
		printf_to_char(ch,"Prompt set to %s\n\r",ch->prompt);
		ch->send("Battleprompt set to DEFAULT mode.\n\r");
		ch->battleprompt = PROMPT_DEFAULT;
		return;
	}
	else if(!strcmp(arg,"main")){
		if (strlen_color(argument) > 160)
			argument[strlen_colorsp(argument,160)] = '\0';
		strcpy(buf,argument);
		smash_tilde(buf);
		if (str_suffix("%c",buf))
			strcat(buf," ");
		strcat(buf,"{x");
		free_string(ch->prompt);
		ch->prompt = str_dup(buf);
		printf_to_char(ch,"Prompt set to %s\n\r",ch->prompt);
		return;
	}
	else if(!strcmp(arg,"battle")){
		argument = one_argument(argument,arg2);
		if(!strcmp(arg2,"brief")){
			ch->send("Setting battleprompt to BRIEF mode.\n\r");
			ch->battleprompt = PROMPT_BRIEF;
		}
		if(!strcmp(arg2,"full")){
			ch->send("Setting battleprompt to FULL mode.\n\r");
			ch->battleprompt = PROMPT_FULL;
		}
		if(!strcmp(arg2,"default")){
			ch->send("Setting battleprompt to DEFAULT mode.\n\r");
			ch->battleprompt = PROMPT_DEFAULT;
		}
		if(!strcmp(arg2,"status"))
			printf_to_char(ch,"Current battleprompt setting is: %d\n\r",ch->battleprompt);
		return;
	}
	do_function(ch,&do_prompt,"");
}

void do_combine(CHAR_DATA *ch, char *argument){
	if (ch->setcomm(CM_COMBINE))
		send_to_char("Combined inventory selected.\n\r",ch);
	else{
		send_to_char("Long inventory selected.\n\r",ch);
		ch->remcomm(CM_COMBINE);
	}
}

void do_noloot(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_CANLOOT))
		send_to_char("Your corpse may now be looted.\n\r",ch);
	else{
		send_to_char("Your corpse is now safe from thieves.\n\r",ch);
		ch->remplr(PL_CANLOOT);
	}
}

void do_nofollow(CHAR_DATA *ch, char *argument){
	CHAR_DATA *victim;
	char arg[MSL];
	one_argument(argument,arg);

	if(arg[0]){
		if (!(victim = get_char_world(NULL,arg)))
			ch->send("They don't exist...?\n\r");
		else{
			if(victim->isaff(AF_STALK)){
				ch->send("You seem unable to stop them.\n\r");
				return;
			}
			if( ch->pet && ch->pet == victim ){
				ch->pet->master = NULL;
				ch->pet = NULL;
			}
			stop_follower(victim);
		}
		return;
	}
	if (IS_NPC(ch))
		return;

	if (ch->setplr(PL_NOFOLLOW)){
		send_to_char("You no longer accept followers.\n\r",ch);
		die_follower(ch,NULL);
	}
	else{
		send_to_char("You now accept followers.\n\r",ch);
		ch->remplr(PL_NOFOLLOW);
	}
}

void do_nosummon(CHAR_DATA *ch, char *argument){
	if (IS_NPC(ch)){
		if (ch->res[RS_SUMMON]){
			ch->buff_res(RS_SUMMON,0);
			send_to_char("You are now immune to summoning.\n\r",ch);
		}
		else{
			send_to_char("You are no longer immune to summon.\n\r",ch);
			ch->reset_res(RS_SUMMON);
		}
	}
	else{
		if (ch->setplr(PL_NOSUMMON))
			send_to_char("You are now immune to summoning.\n\r",ch);
		else{
			send_to_char("You are no longer immune to summon.\n\r",ch);
			ch->remplr(PL_NOSUMMON);
		}
	}
}

void do_skygaze( CHAR_DATA *ch, char *argument )
{
	if ( !IS_OUTSIDE(ch) )
	{
		send_to_char("You can't see the sky indoors.\n\r",ch);
		return;
	}
	else
	{
		look_sky(ch);
		return;
	}
}

void format_rdesc(ROOM_INDEX_DATA *room){
	char buf[MSL];
	return;
}

void show_imprint(CHAR_DATA *ch){
	ROOM_INDEX_DATA *location;
	for(OBJ_DATA *stone = ch->carrying;stone;stone = stone->next_content){
		if(stone->item_type == ITEM_WARP_STONE && stone->value[0] > 0){
			if (!(location = get_room_index(stone->value[0]))){
				ch->send("A buggy stone!!!\n\r");
				continue;
			}
			ch->printf("{G%s{x",location->name);
			if((IS_IMMORTAL(ch) && (IS_NPC(ch) || ch->isplr(PL_HOLYLIGHT))) || IS_BUILDER(ch, ch->in_room->area))
				ch->printf(" [{wRoom {g%d{x]",location->vnum);
			ch->printf("%s %s{x",sect_table[location->sector_type].color,location->description);
		}
	}
}

void do_look(CHAR_DATA *ch,char *argument){
	EXIT_DATA *pexit;
	ROOM_INDEX_DATA *location,*oldplace;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	char arg1[MIL], arg2[MIL], arg3[MIL], *pdesc;
	int door,number,count;

	if(!str_cmp(argument,"imprint")){
		show_imprint(ch);
		return;
	}
	if(!ch->desc)
		return;
	if(ch->position < POS_SLEEPING){
		send_to_char("You can't see anything but stars!\n\r",ch);
		return;
	}
	if(ch->position == POS_SLEEPING){
		send_to_char("You can't see anything, you're sleeping!\n\r",ch);
		return;
	}
	if(!check_blind(ch))
		return;
	if(!IS_NPC(ch) && !ch->isplr(PL_HOLYLIGHT) && !can_see_room(ch,ch->in_room)){
		ch->send("{dIt is pitch black ... \n\r");
		return;
	}

	argument = one_argument(argument,arg1);
	argument = one_argument(argument,arg2);
	number = number_argument(arg1,arg3);
	count = 0;

	if(!arg1[0] || !str_cmp(arg1,"auto")){
		if(ch->isplr(PL_AUTOMAP) && !IS_SET(ch->in_room->room_flags,ROOM_NOMAP))
			;
		else
			printf_to_char(ch,"{G%s{x",ch->in_room->name);

		if((IS_IMMORTAL(ch) && (IS_NPC(ch) || ch->isplr(PL_HOLYLIGHT))) || IS_BUILDER(ch, ch->in_room->area))
			printf_to_char(ch," [{wRoom {g%d{x]",ch->in_room->vnum);
		send_to_char("\n\r",ch);

		if(!arg1[0] || (!IS_NPC(ch) && !ch->iscomm(CM_BRIEF))){
			if(ch->isplr(PL_AUTOMAP) && !IS_SET(ch->in_room->room_flags,ROOM_NOMAP))
				display_map(ch);
			else
				printf_to_char(ch,"%s %s{x",sect_table[ch->in_room->sector_type].color,ch->in_room->description);
		}

		if (ch->isplr(PL_AUTOWEATHER) && IS_OUTSIDE(ch))
			show_weather(ch);

		if (ch->in_room->israf(RAF_FROSTFIELD))
			ch->send("\n\r   {CFrost explodes everywhere.{x\n\r");
		if (ch->in_room->israf(RAF_REGENERATION))
			ch->send("\n\r   {wA warm glow envelopes the room.{x\n\r");
		if (ch->in_room->israf(RAF_MAGIC_DAMPEN) && ch->isaff(AF_DETECT_MAGIC))
			ch->send("\n\r   {cA magical seal has been placed here.{x\n\r");
		if (ch->in_room->israf(RAF_FEEDBACK) && ch->isaff(AF_DETECT_MAGIC))
			ch->send("\n\r   {rA death ward seems to pulse throughout the room.{x\n\r");
		if (!IS_NPC(ch) && ch->isplr(PL_AUTOEXIT))
			do_function(ch,&do_exits,"auto");

		show_list_to_char(ch->in_room->contents,ch,false,false);
		show_char_to_char(ch->in_room->people,ch);
		return;
	}

	if (!str_cmp(arg1,"i") || !str_cmp(arg1,"in") || !str_cmp(arg1,"on")){
		if (!arg2[0]){
			send_to_char("Look in what?\n\r",ch);
			return;
		}

		if ((obj = get_obj_here(ch,NULL,arg2)) == NULL){
			send_to_char("You do not see that here.\n\r",ch);
			return;
		}

		switch (obj->item_type){
			default:
				send_to_char("That is not a container.\n\r",ch);
				break;
			case ITEM_DRINK_CON:
				if ( obj->value[1] <= 0 )
					send_to_char("It is empty.\n\r",ch);
				else
					printf_to_char(ch,"It's %sfilled with a %s liquid.\n\r",
						obj->value[1] < obj->value[0] / 4 ? "less than half-" : obj->value[1] < 3 * obj->value[0] / 4 ? "about half-" : "more than half-",
						liq_table[obj->value[2]].liq_color);
				break;
			case ITEM_QUIVER:
				if ( obj->value[0] <= 0 )
					send_to_char("The quiver is out of arrows.\n\r",ch);
				else if (obj->value[0] == 1 )
					send_to_char("The quiver has 1 arrow remaining in it.\n\r",ch);
				else if (obj->value[0] > 1 )
					printf_to_char(ch,"The quiver has %d arrows in it.\n\r",obj->value[0]);
				break;
			case ITEM_PORTAL:
				oldplace = ch->in_room;
				if (IS_SET(obj->value[2],GATE_RANDOM) || obj->value[3] == -1)
					location = get_random_room(ch);
				else if (IS_SET(obj->value[2],GATE_BUGGY) && (number_percent() < 5))
					location = get_random_room(ch);
				else
					location = get_room_index(obj->value[3]);
				if (location == NULL || location == oldplace || !can_see_room(ch,location) || (room_is_private(location) && !IS_TRUSTED(ch,ADMIN)))
				   act("$p doesn't seem to go anywhere.",ch,obj,NULL,TO_CHAR);
				else{
					char_from_room(ch);
					char_to_room(ch,location);
					do_function(ch,&do_look,"auto");
					char_from_room(ch);
					char_to_room(ch,oldplace);
				}
				break;
			case ITEM_CONTAINER:
			case ITEM_LOCKER:
			case ITEM_CORPSE_NPC:
			case ITEM_CORPSE_PC:
				if ( IS_SET(obj->value[1],CONT_CLOSED) )
					send_to_char("It is closed.\n\r",ch);
				else{
					act("$p holds:",ch,obj,NULL,TO_CHAR);
					show_list_to_char(obj->contains,ch,true,true);
				}
				break;
			case ITEM_FORGE:
				act("$p holds:",ch,obj,NULL,TO_CHAR);
				show_list_to_char(obj->contains,ch,true,true);
				break;
		}
		return;
	}

	if ((victim = get_char_room(ch,NULL,arg1))){
		show_char_to_char_1(victim,ch,!ch->iscomm(CM_ROMLOOK));
		return;
	}

	for ( obj = ch->carrying; obj != NULL; obj = obj->next_content ){
		if (can_see_obj(ch,obj)){
			pdesc = get_extra_descr(arg3,obj->extra_descr);
			if (pdesc != NULL){
    			if (++count == number){
					send_to_char(pdesc,ch);
					return;
    			}
    			else
					continue;
			}
 			pdesc = get_extra_descr(arg3,obj->pIndexData->extra_descr);
			if (pdesc != NULL){
    			if (++count == number){
					send_to_char(pdesc,ch);
					return;
     			}
				else
					continue;
			}
			if (is_name(arg3,obj->name) && ++count == number){
    			send_to_char(obj->description,ch);
    			send_to_char("\n\r",ch);
				return;
			}
		}
	}

	for ( obj = ch->in_room->contents; obj != NULL; obj = obj->next_content ){
		if ( can_see_obj(ch,obj) ){
			pdesc = get_extra_descr( arg3, obj->extra_descr );
			if ( pdesc != NULL && ++count == number){
				send_to_char(pdesc,ch);
				return;
    		}

			pdesc = get_extra_descr(arg3,obj->pIndexData->extra_descr);
			if ( pdesc != NULL && ++count == number){
				send_to_char(pdesc,ch);
				return;
    		}

			if ( is_name(arg3,obj->name) && ++count == number){
				send_to_char(obj->description,ch);
				send_to_char("\n\r",ch);
				return;
			}
		}
	}

	pdesc = get_extra_descr(arg3,ch->in_room->extra_descr);
	if (pdesc != NULL && ++count == number){
		send_to_char(pdesc,ch);
		return;
	}

	if (count > 0 && count != number){
		if (count == 1)
			printf_to_char(ch,"You only see one %s here.\n\r",arg3);
		else
			printf_to_char(ch,"You only see %d of those here.\n\r",count);
		return;
	}

		 if ( !str_cmp(arg1,"n") || !str_cmp(arg1,"north") ) door = 0;
	else if ( !str_cmp(arg1,"e") || !str_cmp(arg1,"east" ) ) door = 1;
	else if ( !str_cmp(arg1,"s") || !str_cmp(arg1,"south") ) door = 2;
	else if ( !str_cmp(arg1,"w") || !str_cmp(arg1,"west" ) ) door = 3;
	else if ( !str_cmp(arg1,"u") || !str_cmp(arg1,"up"   ) ) door = 4;
	else if ( !str_cmp(arg1,"d") || !str_cmp(arg1,"down" ) ) door = 5;
	else{
		send_to_char("You do not see that here.\n\r",ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL){
		send_to_char("Nothing special there.\n\r",ch);
		return;
	}

	if (pexit->description != NULL && pexit->description[0] != '\0')
		send_to_char(pexit->description,ch);
	else
		send_to_char("Nothing special there.\n\r",ch);

	if (pexit->keyword != NULL && pexit->keyword[0] != '\0' && pexit->keyword[0] != ' '){
		if (IS_SET(pexit->exit_info,EX_CLOSED))
			act("The $d is closed.",ch,NULL,pexit->keyword,TO_CHAR);
		else if (IS_SET(pexit->exit_info,EX_ISDOOR))
			act("The $d is open.",ch,NULL,pexit->keyword,TO_CHAR);
	}
}

void do_read(CHAR_DATA *ch, char *argument){do_function(ch,&do_look,argument);}

void do_examine(CHAR_DATA *ch,char *argument){
	char buf[MSL], arg[MIL];
	OBJ_DATA *obj;
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if(!arg[0]){
		send_to_char("Examine what or who?\n\r",ch);
		return;
	}

	if((obj = get_obj_here(ch,NULL,arg))){
		switch(obj->item_type){
			default:
				do_function(ch,&do_look,arg);
				break;
			case ITEM_MONEY:
				if (obj->value[0] == 0){
					if (obj->value[1] == 0)
						printf_to_char(ch,"Odd...there's no coins in the pile.\n\r");
					else if (obj->value[1] == 1)
						printf_to_char(ch,"Wow. One gold coin.\n\r");
					else
						printf_to_char(ch,"There are %d gold coins in the pile.\n\r", obj->value[1]);
				}
				else if (obj->value[1] == 0){
					if (obj->value[0] == 1)
						printf_to_char(ch,"Wow. One silver coin.\n\r");
					else
						printf_to_char(ch,"There are %d silver coins in the pile.\n\r", obj->value[0]);
				}
				else
					printf_to_char(ch,"There are %d gold and %d silver coins in the pile.\n\r", obj->value[1],obj->value[0]);
				break;
			case ITEM_DRINK_CON:
			case ITEM_CONTAINER:
			case ITEM_CORPSE_NPC:
			case ITEM_CORPSE_PC:
				sprintf(buf,"in %s",argument);
				do_function(ch,&do_look,buf);
				break;
		}
	}

	if((victim = get_char_room(ch,NULL,arg))){
		show_char_to_char_1(victim,ch,false);
		return;
	}
}

char * grind_dir(CHAR_DATA *ch,int door){
    char buf[512];
    const char *dir_wake[] = {"N","E","S","W","U","D"};
    const char *dir_rom[] = {"north","east","south","west","up","down"};
	char doorname[50],buf2[MIL],doorthing[50];
	ROOM_INDEX_DATA *room;
	EXIT_DATA *pexit = ch->in_room->exit[door];

	buf[0] = '\0';
	buf2[0] = '\0';

	sprintf(doorthing,"%s",ch->iscomm(CM_ROMEXITS) ? dir_rom[door] : dir_wake[door]);
	if(pexit && pexit->u1.to_room){
		if(IS_SET(pexit->u1.to_room->room_flags,ROOM_NEWBIES_ONLY) && ch->level > 9 && !IS_IMMORTAL(ch)){
			sprintf(buf,"{d({g-{d){x");
			return buf;
		}

		if (IS_SET(pexit->exit_info,EX_SUPERINVIS)){
			if(ch->isplr(PL_HOLYLIGHT))
				sprintf(doorname,"{C%s",doorthing);
			else
				return str_dup("{d({g-{d)");
		}
		else if(IS_SET(pexit->exit_info,EX_HIDDEN)){
			if(ch->isaff(AF_DETECT_HIDDEN) || ch->isplr(PL_HOLYLIGHT))
				sprintf(doorname,"{d%s",doorthing);
			else
				return str_dup("{d({g-{d)");
		}
		else if ((IS_SET(pexit->exit_info,EX_SDOOR) && IS_SET(pexit->exit_info,EX_CLOSED))){
			if((ch->isaff(AF_DETECT_HIDDEN) || ch->isplr(PL_HOLYLIGHT)))
				sprintf(doorname,"{d%s",doorthing);
			else
				return str_dup("{d({g-{d)");
		}
		else if (IS_SET(pexit->exit_info,EX_CLIFFTOP))
			sprintf(doorname,"{y%s",doorthing);
		else
			sprintf(doorname,"{G%s",doorthing);


		if (IS_SET(pexit->exit_info,EX_CLOSED)){
			if (IS_SET(pexit->exit_info,EX_HIDDEN) || IS_SET(pexit->exit_info,EX_SDOOR)){
				if (ch->isaff(AF_DETECT_HIDDEN) || ch->isplr(PL_HOLYLIGHT)){
					if(!IS_SET(pexit->exit_info,EX_SUPERINVIS) || IS_IMMORTAL(ch)){
						if (IS_SET(pexit->exit_info,EX_LOCKED))
							sprintf(buf2,"{d<%s{d>",doorname);
						else
							sprintf(buf2,"{d[%s{d]",doorname);
					}
				}
			}
			else{
				if(!IS_SET(pexit->exit_info,EX_SUPERINVIS) || ch->isplr(PL_HOLYLIGHT)){
					if (IS_SET(pexit->exit_info,EX_LOCKED))
						sprintf(buf2,"{w<%s{w>",doorname);
					else
						sprintf(buf2,"{w[%s{w]",doorname);
				}
			}
		}
		else{
			if(!IS_SET(pexit->exit_info,EX_SUPERINVIS) || ch->isplr(PL_HOLYLIGHT)){
				if (!IS_SET(pexit->exit_info,EX_ISDOOR)){
					if (ch->iscomm(CM_ROMEXITS))
						sprintf(buf2,"%s",doorname);
					else
						sprintf(buf2," %s ",doorname);
				}
				else
					sprintf(buf2,"{d)%s{d(",doorname);
			}
		}
	}
	strcat(buf,buf2);

	if (!ch->iscomm(CM_ROMEXITS)){
		if (!buf[0])
			sprintf(buf,"{d({g-{d)");
	}
	else{
		if(buf[0]){
			sprintf(buf2," %s",buf);
			sprintf(buf,"%s",buf2);
		}
	}
	return buf;
}

void crunch_exits(CHAR_DATA *ch){
	char ndir[50],sdir[50],edir[50],wdir[50],udir[50],ddir[50];
	sprintf(ndir,"%s",grind_dir(ch,DIR_NORTH));
	sprintf(edir,"%s",grind_dir(ch,DIR_EAST));
	sprintf(sdir,"%s",grind_dir(ch,DIR_SOUTH));
	sprintf(wdir,"%s",grind_dir(ch,DIR_WEST));
	sprintf(udir,"%s",grind_dir(ch,DIR_UP));
	sprintf(ddir,"%s",grind_dir(ch,DIR_DOWN));
	if (!ch->iscomm(CM_ROMEXITS)){
		ch->send(         "{d+---------%s{d---%s{d-+\n\r",ndir,udir);
		printf_to_char(ch,"{d|{cExits {x%s   %s    {d|\n\r",wdir,edir);
		printf_to_char(ch,"{d+---------%s{d---%s{d-+{x\n\r\n\r",sdir,ddir);
	}
	else{
		printf_to_char(ch,"[{RExits{x: %s%s%s%s%s%s {x]\n\r\n\r",ndir,sdir,wdir,edir,udir,ddir);
	}
}

void do_exits(CHAR_DATA *ch,char *argument){
	extern char * const dir_name[];
	char buf[MSL], buf2[MSL],tbuf[MSL];
	EXIT_DATA *pexit;
	bool found = false, fAuto = !str_cmp(argument,"auto");
	int door;

	if (!check_blind(ch) && !check_instinct(ch))
		return;

	if (fAuto){
		crunch_exits(ch);
		return;
	}

	buf[0] = '\0';
	for (door = 0;door <= 5;door++){
		if ((pexit = ch->in_room->exit[door]) != NULL && pexit->u1.to_room != NULL && can_see_room(ch,pexit->u1.to_room)){
			if(IS_SET(pexit->u1.to_room->room_flags,ROOM_NEWBIES_ONLY) && ch->level > 9 && !IS_IMMORTAL(ch))
				continue;
			if(!found){
				found = true;
				if (IS_IMMORTAL(ch))
					sprintf(buf,"Obvious exits from room %d:\n\r",ch->in_room->vnum);
				else
					sprintf(buf,"Obvious exits:\n\r");
			}
			sprintf(tbuf,"{x%-5s - {G%s{x",capitalize(dir_name[door]),room_is_dark(pexit->u1.to_room) ? "Too dark to tell" : pexit->u1.to_room->name);
			strcat(buf,tbuf);
			if (IS_IMMORTAL(ch)){
				sprintf(tbuf," {x(room {R%d{x)\n\r",pexit->u1.to_room->vnum);
				strcat(buf,tbuf);
			}
			else
				strcat(buf,"{x\n\r");
		}
	}

	if (!found)
		strcat(buf,fAuto ? " none" : "None.\n\r");

	send_to_char(buf,ch);
	return;
}

void do_worth(CHAR_DATA *ch,char *argument)
{
    if (IS_NPC(ch))
    {
		printf_to_char(ch,"You have %d gold and %d silver.\n\r", ch->gold,ch->silver);
		return;
    }

    printf_to_char(ch,"You have %d gold, %d silver, and %d experience (%d exp to level).\n\r",
		ch->gold + ch->bankgold,
		ch->silver + ch->banksilver,
		ch->exp,
		exp_per_level(ch) - ch->exp);
    return;
}

void do_affects(CHAR_DATA *ch,char *argument){
	AFFECT_DATA *paf, *paf_last = NULL;

	if (ch->affected != NULL){
		send_to_char("You are affected by the following:\n\r",ch);
		for ( paf = ch->affected; paf != NULL; paf = paf->next ){
			if (paf_last != NULL && paf->type == paf_last->type)
				if (ch->level >= 20)
					send_to_char("                      ",ch);
				else
					continue;
			else{
				if(paf->where == TO_AFFECTS && paf->location == APPLY_NONE && paf->bitvector == AF_WEARY)
					printf_to_char(ch,"Weariness For: %-15s",skill_table[paf->type].name);
				else{
					if (skill_table[paf->type].slot == SKILL_SPELL)
    					printf_to_char(ch,"%d Spell: %-15s",paf->id,skill_table[paf->type].name);
					else
						printf_to_char(ch,"%d Skill: %-15s",paf->id,skill_table[paf->type].name);
				}
			}

			if (ch->level >= 20){
				printf_to_char(ch,": modifies {w%s {xby {w%d {x", affect_loc_name(paf->location), paf->modifier);
				if (paf->duration == -1)
					printf_to_char(ch,"permanently");
				else
					printf_to_char(ch,"for {w%d {xticks",paf->duration);
			}

			send_to_char("\n\r",ch);
			paf_last = paf;
		}
	}
	else 
		send_to_char("You are not affected by any spells.\n\r",ch);
}

void do_score(CHAR_DATA * ch,char *argument){
	char buf[MSL],buf2[MSL];

	if (IS_NPC(ch))
		return;

	if(!IS_NPC(ch) && argument[0] && !str_prefix(argument,"extended")){
		printf_to_char(ch,"Created: %s\n\r",ch->pcdata->created);

		if (hair_table[ch->looks[P_HAIR]].name != NULL)
			if (ch->looks[P_SHAIR] == S_HAIR_BALD)
				ch->send("Hair: None(Bald)       ");
			else if (ch->looks[P_SHAIR] == S_HAIR_SHAVED)
				ch->send("Hair: None(Shaved)     ");
			else
				printf_to_char(ch,"You have %s %s hair, and ",sub_hair_table[ch->looks[P_SHAIR]].name,hair_table[ch->looks[P_HAIR]].name);
		else
			ch->send("You have {Rb{Cu{Bg{Gg{Ye{Md{x hair ");

		if (eye_table[ch->looks[P_EYE]].name != NULL)
			printf_to_char(ch,"%s eyes.\n\r",eye_table[ch->looks[P_EYE]].name);
		else
			ch->send("and {Rb{Cu{Bg{Gg{Ye{Md{x eyes.\n\r");
		printf_to_char(ch,"Build: %s %s,%s%s %s.\n\r",
			IS_VOWELL(height_table[ch->looks[P_HEIGHT]].name[0]) ? "an" : "a",
			height_table[ch->looks[P_HEIGHT]].name,
			ch->looks[P_HEIGHT] == HEIGHT_AVERAGE && ch->looks[P_WEIGHT] == WEIGHT_AVERAGE ? "" : " ",
			ch->looks[P_HEIGHT] == HEIGHT_AVERAGE && ch->looks[P_WEIGHT] == WEIGHT_AVERAGE ? "" : weight_table[ch->looks[P_WEIGHT]].name,
			weight_table[ch->looks[P_WEIGHT]].name,
			sex_table[ch->sex].name,
			race_table[ch->race].name,
			capitalize(sex_table[ch->sex].eword));
		printf_to_char(ch,"Logon: %d %d %s\n\r",ch->logon,current_time,(char*)ctime(&ch->logon));
		printf_to_char(ch,"You are a %s.\n\r",ch->lefty ? "lefty" : "righty");
		if(ch->pcdata->condition[COND_HUNGER] == 0)			ch->send("You are starving.\n\r");
		else if(ch->pcdata->condition[COND_HUNGER] < 10)	ch->send("You are really hungry.\n\r");
		else if(ch->pcdata->condition[COND_HUNGER] < 20)	ch->send("You are pretty hungry.\n\r");
		else if(ch->pcdata->condition[COND_HUNGER] < 30)	ch->send("You are kind of hungry.\n\r");
		else if(ch->pcdata->condition[COND_HUNGER] < 40)	ch->send("You are not very hungry.\n\r");
		else												ch->send("You are hungry.\n\r");

		if(ch->pcdata->condition[COND_THIRST] == 0)			ch->send("You are dying of thirst.\n\r");
		else if(ch->pcdata->condition[COND_THIRST] < 10)	ch->send("You are really thirsty.\n\r");
		else if(ch->pcdata->condition[COND_THIRST] < 20)	ch->send("You are pretty thirsty.\n\r");
		else if(ch->pcdata->condition[COND_THIRST] < 30)	ch->send("You are feeling a bit thirsty.\n\r");
		else if(ch->pcdata->condition[COND_THIRST] < 40)	ch->send("You your tongue feels a little thick.\n\r");
		else												ch->send("Your mouth is a little dry.\n\r");
		if(IS_DRUNK(ch))
			ch->send("You are drunk.\n\r");
		if (ch->iscomm(CM_SHOW_AFFECTS))
			do_function(ch,&do_affects,"");
		printf_to_char(ch,"Class Tier: %d\n\r",classes[get_class(ch)].ctier);
		ch->send("Stored gains:\n\r");
		printf_to_char(ch,"Tier 0: %d\n\rTier 1: %d\n\rTier 2: %d\n\rTier 3: %d\n\r",ch->pcdata->s_studies[0],ch->pcdata->s_studies[1],ch->pcdata->s_studies[2],ch->pcdata->s_studies[3]);
		printf_to_char(ch,"Saves Practices: %d\n\r",ch->pcdata->s_practices);
		return;
	}


	sprintf(buf,"%s%s%s%s",
		ch->name,
		IS_NPC(ch) ? "" : strcmp(ch->pcdata->lname,"{x") ? " " : "",
		IS_NPC(ch) ? "" : ch->pcdata->lname,
		IS_NPC(ch) ? "" : ch->pcdata->title);
	printf_to_char(ch," {x%s\n\r",buf);
	sprintf(buf,"{c%d {xhours",(current_time - ch->logon)/3600);
	sprintf(buf2,"{c%d {xhours",(ch->played + (int) (current_time - ch->logon)) / 3600);
    ch->send("{d _____________________________________________________________________________\n\r");
    printf_to_char(ch,"/ _____________________  _____________________________________  _____________ \\\n\r");
	printf_to_char(ch,"|/                     \\/                                     \\/             \\|\n\r");
	printf_to_char(ch,"|| {xLevel: {C%-3d          {d|| {xHometown: {G%-20s      {d|| {xOnline:     {d||\n\r",ch->level,IS_NPC(ch) ? "Null" : hometowns[ch->hometown].name);
	printf_to_char(ch,"|| {xClass: {G%-12s {d|| {xWorship : {G%-20s      {d|| {C%5d {xhours {d||\n\r",classes[ch->pclass].name,god_table[ch->god].name,(current_time - ch->logon)/3600);
	printf_to_char(ch,"|| {xRace : {G%-10s   {d|| {xHouse   : {G%-25s {d||             ||\n\r",race_table[ch->race].name,guilds[ch->guild].name);
	printf_to_char(ch,"|| {xSex  : {G%-6s       {d|| {xRank    : {G%-19s       {d|| {xTotal:      {d||\n\r",sex_table[ch->sex].name,IS_NPC(ch) ? "Null" : guilds[ch->guild].rank[ch->rank].name);
	printf_to_char(ch,"|| {xAge  : {C%-4d         {d|| {xNobility: {G%-20s      {d|| {C%5d {xhours {d||\n\r",get_age(ch),nobility_flags[ch->nobility].name[ch->sex],(ch->played + (int) (current_time - ch->logon)) / 3600);
	printf_to_char(ch,"|\\_____________________/\\_____________________________________/\\_____________/|\n\r");
	printf_to_char(ch,"| ___________________  _______________________  ______________  _____________ |\n\r");
	printf_to_char(ch,"|/                   \\/                       \\/              \\/             \\|\n\r");
	printf_to_char(ch,"|| {xHp: {c%6d{x/{C%-6d {d||    {xHitroll: {C%-4d      {d|| {xExp: {Y%-7d {d|| {xSTR  {c%-2d{x({C%2d{x) {d||\n\r",ch->hit,ch->max_hit,GET_HITROLL(ch),ch->exp,ch->perm_stat[STAT_STR],get_curr_stat(ch,STAT_STR));
	printf_to_char(ch,"|| {xMp: {c%6d{x/{C%-6d {d||                       {d||              {d|| {xEND  {c%-2d{x({C%2d{x) {d||\n\r",ch->getmana(),ch->getmaxmana(),ch->perm_stat[STAT_END],get_curr_stat(ch,STAT_END));
	printf_to_char(ch,"|| {xMv: {c%6d{x/{C%-6d {d||    {xDamroll: {C%-4d      {d|| {xTNL: {Y%-7d {d|| {xAGI  {c%-2d{x({C%2d{x) {d||\n\r",ch->move,ch->max_move,IS_NPC(ch) ? 0 : GET_DAMROLL(ch),ch->level < LEVEL_HERO ? exp_per_level(ch) - ch->exp : 0,ch->perm_stat[STAT_AGI],get_curr_stat(ch,STAT_AGI));
	printf_to_char(ch,"||      --=+=--      ||        --=+=--        ||    -=+=-     || {xINT  {c%-2d{x({C%2d{x) {d||\n\r",ch->perm_stat[STAT_INT],get_curr_stat(ch,STAT_INT));
	printf_to_char(ch,"|| {xStudies:    {C%-4d  {d||   {xArmor Class         {d||              || {xRES  {c%-2d{x({C%2d{x) {d||\n\r",IS_NPC(ch) ? 0 : ch->pcdata->studies,ch->perm_stat[STAT_RES],get_curr_stat(ch,STAT_RES));
	printf_to_char(ch,"|| {xTrains:     {C%-4d  {d|| {xSlash:  {c%5d {x({C%5d{x) {d|| {xPK: {C%-8d {d|| {xFTH  {c%-2d{x({C%2d{x) {d||\n\r",IS_NPC(ch) ? 0 : ch->pcdata->trains,ch->armor[AC_SLASH]/10,GET_AC(ch,AC_SLASH),ch->kills[PK],ch->perm_stat[STAT_FTH],get_curr_stat(ch,STAT_FTH));
	printf_to_char(ch,"|| {xPractices:  {C%-4d  {d|| {xPierce: {c%5d {x({C%5d{x) {d|| {xPD: {C%-8d {d|| {xWIS  {c%-2d{x({C%2d{x) {d||\n\r",IS_NPC(ch) ? 0 : ch->pcdata->practices,ch->armor[AC_PIERCE]/10,GET_AC(ch,AC_PIERCE),ch->kills[PD],ch->perm_stat[STAT_WIS],get_curr_stat(ch,STAT_WIS));
	printf_to_char(ch,"|| {xStatpoints: {C%-4d  {d|| {xBash:   {c%5d {x({C%5d{x) {d|| {xMK: {C%-8d {d|| {xCHA  {c%-2d{x({C%2d{x) {d||\n\r",IS_NPC(ch) ? 0 : ch->pcdata->statpoints,ch->armor[AC_BASH]/10,GET_AC(ch,AC_BASH),ch->kills[MK],ch->perm_stat[STAT_CHA],get_curr_stat(ch,STAT_CHA));
	printf_to_char(ch,"|| {xWakepoints: {C%-4d  {d|| {xExotic: {c%5d {x({C%5d{x) {d|| {xMD: {C%-8d {d|| {xLCK  {c%-2d{x({C%2d{x) {d||\n\r",ch->credits,ch->armor[AC_EXOTIC]/10,GET_AC(ch,AC_EXOTIC),ch->kills[MD],ch->perm_stat[STAT_LCK],get_curr_stat(ch,STAT_LCK));
	printf_to_char(ch,"|\\___________________/\\_______________________/\\______________/\\_____________/|\n\r");
	printf_to_char(ch,"| ________________________________________________  _________________________ |\n\r");
	printf_to_char(ch,"|/                                                \\/                         \\|\n\r");
	sprintf(buf,"{xItems: {c%d{x/{C%d",ch->carry_number,can_carry_n(ch));
	sprintf(buf2,"{xWeight: {c%d{x/{C%d",get_carry_weight(ch) / 10, can_carry_w(ch) / 10);
	printf_to_char(ch,"|| {xHeld Gold: {Y%-13d {xSilver: {w%-13d {d|| %-31s {d||\n\r",ch->gold,ch->silver,buf);
	printf_to_char(ch,"|| {xBank Gold: {Y%-13d {xSilver: {w%-13d {d|| %-31s {d||\n\r",ch->bankgold,ch->banksilver,buf2);
	printf_to_char(ch,"|\\________________________________________________/\\_________________________/|\n\r");
	printf_to_char(ch,"\\_____________________________________________________________________________/\n\r{x");
}

char * const day_name [] = {"Virtue", "Justice", "Travel", "Innovation", "Nature","Magic", "Revelry"};

char * const month_name [] =
{//Froesber
    "January", "February", "March", "April", "May","June", "July", "August", "September",
	"October", "November", "December"
};

void do_time(CHAR_DATA *ch,char *argument){
    extern char str_boot_time[];
    char buf[MSL], *suf;
    int day;

    day = time_info.day + 1;

         if (day > 4 && day <  20) suf = "th";
    else if (day % 10 ==  1      ) suf = "st";
    else if (day % 10 ==  2      ) suf = "nd";
    else if (day % 10 ==  3      ) suf = "rd";
    else                           suf = "th";

	sprintf(buf,":%s%d",time_info.ticks * 5 < 10 ? "0" : "",time_info.ticks * 5);
    printf_to_char(ch,"It is %d%s%s%s, Day of %s, %d%s the Month of %s.\n\r",
		(time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
		time_info.ticks == 0 ? "" : buf,
		time_info.ticks == 0 ? " o'clock " : " ",
		time_info.hour >= 12 ? "pm" : "am",
		day_name[day % 7],
		day, suf,
		month_name[time_info.month]);
	printf_to_char(ch,"ROM started up at %s\n\rThe system time is %s.\n\r",
		str_boot_time,
		(char *) ctime(&current_time));
    return;
}

void do_help(CHAR_DATA *ch,char *argument)
{
	HELP_DATA *pHelp;
	BUFFER *output;
	bool found = false;
	char argall[MIL],argone[MIL];
	int level;

	output = new_buf();

	if ( argument[0] == '\0' )
		argument = "summary";

	/* this parts handles help a b so that it returns help 'a b' */
	argall[0] = '\0';
	while (argument[0] != '\0' )
	{
		argument = one_argument(argument,argone);
		if (argall[0] != '\0')
			strcat(argall," ");
		strcat(argall,argone);
	}

	for ( pHelp = help_first; pHelp != NULL; pHelp = pHelp->next )
	{
		level = (pHelp->level < 0) ? -1 * pHelp->level - 1 : pHelp->level;

		if (level > get_trust(ch) )
			continue;

		if (is_name(argall,pHelp->keyword))
		{
			/* add seperator if found */
			if (found)
				add_buf(output,"\n\r============================================================\n\r\n\r");
			if ( pHelp->level >= 0 && str_cmp(argall,"imotd") )
			{
				add_buf(output,pHelp->keyword);
				add_buf(output,"\n\r");
			}

			/* Strip leading '.' to allow initial blanks.*/
			if ( pHelp->text[0] == '.' )
				add_buf(output,pHelp->text+1);
			else
				add_buf(output,pHelp->text);
			found = true;
			/* small hack :) */
			if (ch->desc != NULL && ch->desc->connected != CON_PLAYING)
				break;
		}
	}

	if (!found)
		send_to_char("No help on that word.\n\r",ch);
	else
		page_to_char(buf_string(output),ch);
	free_buf(output);
}

bool IS_HL(CHAR_DATA *ch){
	if(guilds[ch->guild].type == GTYPE_POPULACE)
		return false;
	return (ch->rank == 0);
}

char *printName(CHAR_DATA *wch, bool iswhois){
	static char buf[MSL];
	char icon[5];

	if(IS_IMMORTAL(wch)){
		if(IS_HL(wch))
			sprintf(icon,"{C*");
		else
			sprintf(icon,"{c*");
	}
	else{
		if(IS_HL(wch)){
			if(wch->level == KING)
				sprintf(icon,"{R*");
			else if(wch->level == HLEADER)
				sprintf(icon,"{R+");
			else
				sprintf(icon,"{R=");
		}
		else{
			if(wch->level == LEVEL_HERO)
				sprintf(icon,"{G-");
			else
				sprintf(icon," ");
		}
	}

	if(!iswhois)
		sprintf(buf,"%s {x%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s{x\n\r",
			str_dup(icon),
			wch->isplr(PL_KILLER) ? "{x[{RK{x]" : "",
			wch->isplr(PL_THIEF) ? "[{YT{x]" : "",
			wch->incog_level >= LEVEL_HERO ? "{x({dIncog{x) " : "",
			wch->invis_level >= LEVEL_HERO ? "{x({GWizi{x) " : "",
			wch->iscomm(CM_BUSY) ? "[{RBusy{x] " : "",
			wch->iscomm(CM_AFK) ? (!IS_NPC(wch) && wch->pcdata->afk[0]) ? "{x[{YAFK{x] " : "{x[{yAFK{x] " : "",
			wch->iscomm(CM_DEBUG) ? "{cLAPD{x " : "",
			wch->iscomm(CM_STUPID) ? "{x-{YSTUPID{x-> " : "",
			wch->isplr(PL_ARENA) ? "<{MARENA{x> " : "",
			wch->nobility >= 5 ? nobility_flags[wch->nobility].name[wch->sex] : wch->nobility == 0 ? nobility_flags[wch->nobility].name[wch->sex] : "",
			wch->nobility >= 5 || wch->nobility == 0 ? " " : "",
			wch->name,
			strcmp(wch->pcdata->lname,"{x") ? " " : "",
			strcmp(wch->pcdata->lname,"(null)") ? wch->pcdata->lname : "",
			IS_NPC (wch) ? "" : wch->pcdata->title);
	else{
		sprintf(buf,"[%s] %s%s%s%s%s%s%s%s%s %s {x%s%s%s%s%s%s\n\r",
			wch->race < MAX_PC_RACE ? pc_race_table[wch->race].who_name : "     ",
			wch->isplr(PL_KILLER) ? "[{RK" : "",
			wch->isplr(PL_THIEF) ? "{YT{x]" : "",
			wch->incog_level >= LEVEL_HERO ? "{x({dIncog{x) ": "",
 			wch->invis_level >= LEVEL_HERO ? "{x({GWizi{x) " : "",
			wch->iscomm(CM_BUSY) ? "[{RBUSY{x] " : "",
			wch->iscomm(CM_AFK) ? (!IS_NPC(wch) && wch->pcdata->afk[0]) ? "{x[{YAFK{x] " : "{x[{yAFK{x] " : "",
			wch->iscomm(CM_DEBUG) ? "{Y*{cLAPD{Y*{x " : "",
			wch->iscomm(CM_STUPID) ? "{x-{YSTUPID{x-> " : "",
			wch->isplr(PL_ARENA) ? "<{MARENA{x> " : "",
			icon,
			wch->nobility >= 5 ? nobility_flags[wch->nobility].name[wch->sex] : wch->nobility == 0 ? nobility_flags[wch->nobility].name[wch->sex] : "",
			wch->nobility >= 5 || wch->nobility == 0 ? " " : "",
			wch->name,
			strcmp(wch->pcdata->lname,"{x") ? " " : "",
			strcmp(wch->pcdata->lname,"(null)") ? wch->pcdata->lname : "",
			IS_NPC(wch) ? "" : wch->pcdata->title);
	}
	return buf;
}

void do_whois(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	BUFFER *output;
	DESCRIPTOR_DATA *d;
	bool found = false, printed;

	one_argument(argument,arg);

	if(!arg[0]){
		send_to_char("You must provide a name.\n\r",ch);
		return;
	}

	output = new_buf();
	printed=false;
	for(d = descriptor_list;d;d = d->next){
		CHAR_DATA *wch;
 		if(d->connected != CON_PLAYING)
			continue;
		wch = ( d->original != NULL ) ? d->original : d->character;
 		if(!can_see(ch,wch))
			continue;
		if(!str_prefix(arg,wch->name)){
			if (printed == false)
				send_to_char("The Commonfolk\n\r", ch);
			printf_to_char(ch,"%s%s",wch->petition == ch->guild ? wch->petition != 0 ? "[{YPETITION{x]" : "" : "",printName(wch,true));
			found=true;
		}
	}
	if(!found){
		send_to_char("No one of that name is playing.\n\r",ch);
		return;
	}
}

void do_who(CHAR_DATA *ch,char *argument){
	CHAR_DATA *wch;
    BUFFER *output;
    DESCRIPTOR_DATA *d;
    int wlevel,chan = 0,nMatch,fBCheck,guildlist[MAX_GUILD];
	char buf[MSL],arg[MSL];
	bool group = false;

	one_argument(argument,arg);

	if(mud.coder)
		ch->printf("\n\r    The People on the Wake {RC{Go{Bd{Ye{Mr {CP{Ro{Gr{Bt{x\n\r");
	else
	    printf_to_char(ch,"\n\r    The Citizens of Elantar{x\n\r");
	if(arg[0] != '\0'){
		if(!str_prefix(arg,"group")){
			ch->send("\n\rPlayers in your grouping range:\n\r");
			group = true;
		}
		else if(!str_prefix(arg,"house"))
			;
		else if(!str_prefix(arg,"auction"))
			chan = CHAN_AUCTION;
		else if(!str_prefix(arg,"question"))
			chan = CHAN_QUESTION;
		else if(!str_prefix(arg,"answer"))
			chan = CHAN_ANSWER;
		else if(!str_prefix(arg,"newbie"))
			chan = CHAN_NEWBIE;
		else if(!str_prefix(arg,"chat"))
			chan = CHAN_CHAT;
		else if(!str_prefix(arg,"gossip"))
			chan = CHAN_GOSSIP;
		else
			return;
	}

    nMatch = 0;
    buf[0] = '\0';
    output = new_buf ();

	for(int i = 0;i<MAX_GUILD;i++)
		guildlist[i] = 0;

	for (d = descriptor_list;d;d = d->next){
		if (d->connected != CON_PLAYING)
			continue;
		wch = d->original ? d->original : d->character;
		if (!can_see (ch,wch))
			continue;
		if (group && (ch->level - wch->level < -10 || ch->level - wch->level > 10))
			continue;
		if(guilds[wch->guild].hidden){
			if(ch->guild != wch->guild){
				guildlist[guilds[wch->guild].hidden] = 1;// && (ch->guild == guilds[wch->guild].hidden && ch->rank != 0))
				continue;
			}
		}
		guildlist[wch->guild] = 1;
	}

	for(int i = 0;i<MAX_GUILD;i++){
		if(guildlist[i] == 0)
			continue;
		printf_to_char(ch,"\n\r%s\n\r",guilds[i].who_name);

		for(int j = MAX_LEVEL+1;j > 0;j--){
			for (d = descriptor_list; d != NULL; d = d->next){
				if(d->connected != CON_PLAYING)
					continue;
				wch = (d->original != NULL) ? d->original : d->character;
				if(wch->level != j)
					continue;
				if(!can_see(ch,wch))
					continue;
				if (group && (ch->level - wch->level < -10 || ch->level - wch->level > 10))
					continue;
				if(wch->guild != i){
					if(ch->guild == wch->guild)
						continue;
					//else if(ch->guild == guilds[wch->guild].hidden && ch->rank == 0)
					//	continue;
					else if(!guilds[wch->guild].hidden || guilds[wch->guild].hidden != i)
						continue;
				}
				printf_to_char(ch,"%s%s",wch->petition == ch->guild ? wch->petition != 0 ? "[{YPETITION{x]" : "" : "",printName(wch,false));
				nMatch++;
			}
		}
		
	}
	sprintf(buf,"\n\rPlayers found: {R%d{x/{r%d{x, Most Today: {G%d{x, Most ever: {G%d{x\n\r",nMatch,mud.t_con,mud.d_con,mud.max_con);
    add_buf(output,buf);
    page_to_char(buf_string(output),ch);
    free_buf(output);
	if (double_exp)
		send_to_char("Doubles are currently {Gactive{x.\n\r",ch);
}

void do_count(CHAR_DATA *ch,char *argument){
	int count;
	DESCRIPTOR_DATA *d;
	FILE *fp;

	count = 0;

	for ( d = descriptor_list; d != NULL; d = d->next )
		if (d->connected == CON_PLAYING && can_see(ch,d->character))
			count++;

	if (count >= mud.max_con){
		printf_to_char(ch,"There are %d characters on, the most ever.\n\r",count);
	}
	else
		printf_to_char(ch,"There are %d characters on, the most today was %d, the most ever was %d.\n\r",count,mud.d_con,mud.max_con);
}

void do_inventory(CHAR_DATA *ch,char *argument)
{
    send_to_char("You are carrying:\n\r",ch);
    show_list_to_char(ch->carrying,ch,true,true);
    return;
}

void do_equipment(CHAR_DATA *ch,char *argument){
	OBJ_DATA *obj;
	int iWear;
	bool found;

	send_to_char("You are using:\n\r",ch);
	found = false;
	for ( iWear = 0; iWear < MAX_WEAR; iWear++ ){
		if (!(obj = get_eq_char(ch,iWear)))
			continue;

		send_to_char(where_name[iWear],ch);
		if (can_see_obj(ch,obj)){
			send_to_char(format_obj_to_char(obj,ch,true),ch);
			send_to_char("\n\r",ch);
		}
		else
			send_to_char("something.\n\r",ch);
		found = true;
	}

	if (!found)
		send_to_char("Nothing.\n\r",ch);
}

void do_compare(CHAR_DATA *ch,char *argument)
{
    char arg1[MIL], arg2[MIL], *msg;
    OBJ_DATA *obj1, *obj2;
    int value1, value2;

    argument = one_argument(argument,arg1);
    argument = one_argument(argument,arg2);
    if (arg1[0] == '\0')
    {
		send_to_char("Compare what to what?\n\r",ch);
		return;
    }

    if ((obj1 = get_obj_carry(ch,arg1,ch)) == NULL)
    {
		send_to_char("You do not have that item.\n\r",ch);
		return;
    }

    if (arg2[0] == '\0')
    {
		for (obj2 = ch->carrying; obj2 != NULL; obj2 = obj2->next_content)
			if (obj2->wear_loc != WEAR_NONE &&  can_see_obj(ch,obj2) && obj1->item_type == obj2->item_type && (obj1->wear_flags & obj2->wear_flags & 
~ITEM_TAKE) != 0)
				break;

		if (obj2 == NULL)
		{
			send_to_char("You aren't wearing anything comparable.\n\r",ch);
			return;
		}
    }
    else if ((obj2 = get_obj_carry(ch,arg2,ch) ) == NULL)
    {
		send_to_char("You do not have that item.\n\r",ch);
		return;
    }

    msg		= NULL;
    value1	= 0;
    value2	= 0;

    if (obj1 == obj2)
		msg = "You compare $p to itself.  It looks about the same.";
    else if (obj1->item_type != obj2->item_type)
		msg = "You can't compare $p and $P.";
    else
    {
		switch (obj1->item_type)
		{
			default:
				msg = "You can't compare $p and $P.";
				break;
			case ITEM_ARMOR:
			case ITEM_SHIELD:
				value1 = obj1->value[0] + obj1->value[1] + obj1->value[2];
				value2 = obj2->value[0] + obj2->value[1] + obj2->value[2];
				break;
			case ITEM_WEAPON:
				if (obj1->pIndexData->new_format)
					value1 = (1 + obj1->value[2]) * obj1->value[1];
				else
	    			value1 = obj1->value[1] + obj1->value[2];
				if (obj2->pIndexData->new_format)
					value2 = (1 + obj2->value[2]) * obj2->value[1];
				else
	    			value2 = obj2->value[1] + obj2->value[2];
				break;
		}
    }

    if (msg == NULL)
    {
			 if (value1 == value2) msg = "$p and $P look about the same.";
		else if (value1  > value2) msg = "$p looks better than $P.";
		else                       msg = "$p looks worse than $P.";
    }

    act(msg,ch,obj1,obj2,TO_CHAR);
    return;
}

void do_credits(CHAR_DATA *ch,char *argument)
{
    do_function(ch,&do_help,"diku");
    return;
}

void do_where(CHAR_DATA *ch,char *argument){
	ch->send("You are in:\n\r");
	printf_to_char(ch," %s\n\r",ch->in_room->area->credits);
	printf_to_char(ch,"  Level Range:%d - %d\n\r",ch->in_room->area->low_range,ch->in_room->area->high_range);
}

void do_consider(CHAR_DATA *ch,char *argument){
	char arg[MIL], *msg;
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if(!arg[0]){
		send_to_char("Consider killing whom?\n\r",ch);
		return;
	}
	if(!(victim = get_char_room(ch,NULL,arg))){
		send_to_char("They're not here.\n\r",ch);
		return;
	}
	if(is_safe(ch,victim)){
		send_to_char("Don't even think about it.\n\r",ch);
		return;
	}
	OBJ_DATA *cwield = get_eq_char(ch,WEAR_WIELD),*vwield = get_eq_char(victim,WEAR_WIELD);
	int diff = victim->level - ch->level,chp = ch->hit,vhp = victim->hit,cdam,vdam;

	if(cwield)
		cdam = dice(cwield->value[1],cwield->value[2]);
	else if(get_skill(ch,gsn_combatives) > 0)
		cdam = number_range((get_curr_stat(ch,STAT_STR) * get_skill(ch,gsn_combatives) / 50),ch->getslvl(gsn_combatives) * (get_curr_stat(ch,STAT_STR) * get_skill(ch,gsn_combatives) /75 /2));
	else
		cdam = 1;
	if(vwield)
		vdam = dice(vwield->value[1],vwield->value[2]);
	else
		vdam = dice(victim->damage[DICE_NUMBER],victim->damage[DICE_TYPE]) + victim->damage[DICE_BONUS];
//nash needs to factor for aspd and multihits (dual second etc)
	diff = (chp / vdam) - (vhp / cdam);
		 if ( diff >=  20 ) msg = "You can kill $N naked and weaponless.";
	else if ( diff >=  10 ) msg = "$N is no match for you.";
	else if ( diff >=   5 ) msg = "$N looks like an easy kill.";
	else if ( diff >=  -4 ) msg = "The perfect match!";
	else if ( diff >=  -9 ) msg = "$N says 'Do you feel lucky, punk?'.";
	else if ( diff >= -19 ) msg = "$N laughs at you mercilessly.";
	else                    msg = "Death will thank you for your gift.";

	act(msg,ch,NULL,victim,TO_CHAR);
}

void set_title(CHAR_DATA *ch,char *title)
{
    char buf[MSL];

    if (IS_NPC(ch))
    {
		bug("Set_title: NPC.",0);
		return;
    }

    if (title[0] != '.' && title[0] != ',' && title[0] != '!' && title[0] != '?')
    {
		buf[0] = ' ';
		strcpy(buf + 1,title);
    }
    else
		strcpy(buf,title);

    free_string(ch->pcdata->title);
    ch->pcdata->title = str_dup(buf);
    return;
}

void do_title(CHAR_DATA *ch,char *argument)
{
    if (IS_NPC(ch))
		return;

    if (argument[0] == '\0')
    {
		send_to_char("Change your title to what?\n\r",ch);
		return;
    }

    if (strlen_color(argument) > 45)
		argument[strlen_colorsp(argument,45)] = '\0';

	strcat(argument,"{x");
    smash_tilde(argument);
    set_title(ch,argument);
    send_to_char("Ok.\n\r",ch);
}

void do_newdesc(CHAR_DATA *ch,char *argument)
{
	char arg[MSL];

	one_argument(argument,arg);

	if(IS_NPC(ch))
	{
		send_to_char("You're too stupid to do this.\n\r",ch);
		return;
	}
	if(arg[0] == '\0' || !str_prefix(arg,"show"))
	{
		send_to_char("Your description is:\n\r",ch);
		send_to_char(ch->description ? ch->description : "(None).\n\r",ch);
	}
	else if(!str_cmp(arg,"edit"))
		string_append(ch,&ch->description);
	else
		send_to_char("Syntax: desc edit\n\r",ch);
	return;
}

//Note, this is the OLD method of description editing
void do_description(CHAR_DATA *ch,char *argument)
{
    char buf[MSL];

    if (argument[0] != '\0')
    {
		buf[0] = '\0';
		smash_tilde(argument);

    	if (argument[0] == '-')
    	{
            int len;
            bool found = false;

            if (ch->description == NULL || ch->description[0] == '\0')
            {
                send_to_char("No lines left to remove.\n\r",ch);
                return;
            }

	  	    strcpy(buf,ch->description);

            for (len = strlen_color(buf); len > 0; len--)
            {
                if (buf[len] == '\r')
                {
                    if (!found)  /* back it up */
                    {
                        if (len > 0)
                            len--;
                        found = true;
                    }
                    else /* found the second one */
                    {
                        buf[len + 1] = '\0';
						free_string(ch->description);
						ch->description = str_dup(buf);
						send_to_char("Your description is:\n\r",ch);
						send_to_char(ch->description ? ch->description : "(None).\n\r",ch);
                        return;
                    }
                }
            }
            buf[0] = '\0';
			free_string(ch->description);
			ch->description = str_dup(buf);
			send_to_char("Description cleared.\n\r",ch);
			return;
		}
		if (argument[0] == '+')
		{
			if ( ch->description != NULL )
				strcat(buf,ch->description);
			argument++;
			while ( isspace(*argument) )
				argument++;
		}
		strcat(buf,argument);
		strcat(buf,"\n\r");
		free_string(ch->description);
		ch->description = str_dup(buf);
    }

    send_to_char("Your description is:\n\r",ch);
    send_to_char(ch->description ? ch->description : "(None).\n\r",ch);
    return;
}

void do_report(CHAR_DATA *ch,char *argument)
{
    char buf[MIL];

    printf_to_char(ch,"You say 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'\n\r",
		ch->hit,ch->max_hit,
		ch->getmana(),ch->getmaxmana(),
		ch->move,ch->max_move,
		ch->exp);
    sprintf(buf,"$n says 'I have %d/%d hp %d/%d mana %d/%d mv %d xp.'",
		ch->hit,ch->max_hit,
		ch->getmana(),ch->getmaxmana(),
		ch->move,ch->max_move,
		ch->exp);

    act(buf,ch,NULL,NULL,TO_ROOM);

    return;
}

void do_wimpy(CHAR_DATA *ch,char *argument)
{
	send_to_char("Lazy...\n\r",ch);
}

void do_password(CHAR_DATA *ch,char *argument)
{
    char arg1[MIL], arg2[MIL], *pArg, *pwdnew, *p, cEnd;

    if (IS_NPC(ch))
		return;

    /*
     * Can't use one_argument here because it smashes case.
     * So we just steal all its code.  Bleagh.
     */
    pArg = arg1;
    while (isspace(*argument))
		argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

    while (*argument != '\0')
    {
		if (*argument == cEnd)
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
    }
    *pArg = '\0';

    pArg = arg2;
    while (isspace(*argument))
		argument++;

    cEnd = ' ';
    if (*argument == '\'' || *argument == '"')
		cEnd = *argument++;

    while ( *argument != '\0')
    {
		if ( *argument == cEnd )
		{
			argument++;
			break;
		}
		*pArg++ = *argument++;
    }
    *pArg = '\0';

    if ( arg1[0] == '\0' || arg2[0] == '\0' )
    {
		send_to_char("Syntax: password <old> <new>.\n\r",ch);
		return;
    }

    if ( strcmp(crypt(arg1,ch->pcdata->pwd),ch->pcdata->pwd) )
    {
		WAIT_STATE( ch, 40 );
		send_to_char("Wrong password.  Wait 10 seconds.\n\r",ch);
		return;
    }

    if ( strlen_color(arg2) < 5 )
    {
		send_to_char("New password must be at least five characters long.\n\r",ch);
		return;
    }

    /*
     * No tilde allowed because of player file format.
     */
    pwdnew = crypt(arg2,ch->name);
    for ( p = pwdnew; *p != '\0'; p++ )
    {
		if ( *p == '~' )
		{
			send_to_char("New password not acceptable, try again.\n\r",ch);
			return;
		}
    }

    free_string(ch->pcdata->pwd);
    ch->pcdata->pwd = str_dup(pwdnew);
    cql_save_char(ch);
    send_to_char("Ok.\n\r",ch);
    return;
}

void do_wiznetall (CHAR_DATA * ch, char *argument)
{
    char arg[MAX_INPUT_LENGTH];

    argument = one_argument(argument,arg);

    if (arg[0] == '\0')
    {
        send_to_char ("Syntax:\n\r",ch);
        send_to_char ("  Wiznetall <on/off>\n\r",ch);
        send_to_char ("  On turns all Wiznet Options on\n\r",ch);
        send_to_char ("  Off turns all Wiznet Options off\n\r",ch);
        return;
    }

    if (!str_cmp(arg,"on") || !str_cmp(arg,"char"))
    {
        do_function(ch,&do_wiznetallon,argument);
        return;
    }

    if (!str_cmp(arg,"off"))
    {
        do_function(ch,&do_wiznetalloff,argument);
        return;
    }
    /* echo syntax */
    do_function(ch,&do_wiznetall,"");
}

void do_wiznetallon(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;

	ch->setwiz(WZ_ON);
	ch->setwiz(WZ_PREFIX);
	ch->setwiz(WZ_TICKS);
	ch->setwiz(WZ_LOGINS);
	ch->setwiz(WZ_SITES);
	ch->setwiz(WZ_LINKS);
	ch->setwiz(WZ_NEWBIE);
	ch->setwiz(WZ_SPAM);
	ch->setwiz(WZ_DEATHS);
	ch->setwiz(WZ_PKS);
	ch->setwiz(WZ_RESETS);
	ch->setwiz(WZ_MOBDEATHS);
	ch->setwiz(WZ_FLAGS);
	ch->setwiz(WZ_PENALTIES);
	ch->setwiz(WZ_SACCING);
	ch->setwiz(WZ_LEVELS);
	ch->setwiz(WZ_LOAD);
	ch->setwiz(WZ_RESTORE);
	ch->setwiz(WZ_SNOOPS);
	ch->setwiz(WZ_SWITCHES);
	ch->setwiz(WZ_SECURE);

	send_to_char("{YAll Wiznet Options turned {Gon{x.{x\n\r",ch);
}

void do_wiznetalloff(CHAR_DATA *ch, char *argument)
{
	if (IS_NPC(ch))
		return;

	ch->remwiz(WZ_ON);
	ch->remwiz(WZ_PREFIX);
	ch->remwiz(WZ_TICKS);
	ch->remwiz(WZ_LOGINS);
	ch->remwiz(WZ_SITES);
	ch->remwiz(WZ_LINKS);
	ch->remwiz(WZ_NEWBIE);
	ch->remwiz(WZ_SPAM);
	ch->remwiz(WZ_DEATHS);
	ch->remwiz(WZ_PKS);
	ch->remwiz(WZ_RESETS);
	ch->remwiz(WZ_MOBDEATHS);
	ch->remwiz(WZ_FLAGS);
	ch->remwiz(WZ_PENALTIES);
	ch->remwiz(WZ_SACCING);
	ch->remwiz(WZ_LEVELS);
	ch->remwiz(WZ_LOAD);
	ch->remwiz(WZ_RESTORE);
	ch->remwiz(WZ_SNOOPS);
	ch->remwiz(WZ_SWITCHES);
	ch->remwiz(WZ_SECURE);

	send_to_char("{YAll Wiznet Options turned {Roff{x.\n\r",ch);
}

void do_pretitle( CHAR_DATA *ch, char *argument )
{
	int value;

	if ( IS_NPC(ch) ) 
	{
		send_to_char("Not on NPC's.\n\r",ch);
		return;
	}
	if ( ch->pcdata->pretitle == '\0' ) 
		ch->pcdata->pretitle = "{x";
	if ( argument[0] == '\0' ) 
	{
		ch->pcdata->pretitle = "";
		return;
	}
	if ( strlen_color(argument) > 45 ) 
	{
		argument[45] = '{';
		argument[46] = 'x';
		argument[47] = '\0';
	}
	else 
	{
		value = strlen_color(argument);
		argument[value] = '{';
		argument[value+1] = 'x';
		argument[value+2] = '\0';
	}
	ch->pcdata->pretitle = str_dup(argument);
	send_to_char("Done.\n\r",ch);
	return;
}

void do_illegalname( CHAR_DATA *ch, char *argument)
{
    char strsave[MIL], namelist[MSL], nameread[MSL], name[MSL];
    FILE *fp;
    if ( argument[0] == '\0' )
    {
		send_to_char("syntax: badname <name>.\n\r",ch);
		return;
    }
    fclose(fpReserve);
    sprintf(strsave,"%s",ILLEGAL_NAME_FILE);
    sprintf(name,"%s\n",argument);
    sprintf(namelist,"%s","");
    if ( (fp = fopen(strsave,"r") ) != NULL )
    {
		for ( ; ; )
		{
			fscanf(fp,"%s",nameread);
			if ( !str_cmp(nameread,"END") )
           		break;
			else
			{
				strcat(namelist,nameread);
				strcat(namelist,"\n");
			}
		}
    }
    else
		fp = fopen(NULL_FILE,"r");
    fclose(fp);
    fp = fopen(strsave,"w");
    strcat(namelist,name);
    fprintf(fp,"%s",namelist);
    fprintf(fp,"END");
    fclose(fp);
    fpReserve = fopen(NULL_FILE,"r");
    send_to_char("All set, that name is now illegal.\n\r",ch);
}

void do_race(CHAR_DATA *ch,char *argument){
	int n;
	bool found = false;

	ch->send("Available racial skills:\n\r");
	for(n = 0; skill_table[n].name;n++){
		if (!IS_SKILL_NATURAL(n))
			continue;
		if (grab_skill(ch,n) > 0){
			if (grab_skill(ch,n) > 90)
				printf_to_char(ch," {R-{G%s\n\r",skill_table[n].name);
			else
				printf_to_char(ch," {R-{r%s\n\r",skill_table[n].name);
			found = true;
		}
	}
	if (!found)
		ch->send("None.");
	ch->send("{x\n\r");
}

