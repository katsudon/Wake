#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "merc.h"
#include "interp.h"
#include "lookup.h"

//Locals
void remove_charmy	( CHAR_DATA*,CHAR_DATA* );

void do_follow(CHAR_DATA *ch,char *argument){
/* RT changed to allow unlimited following and follow the NOFOLLOW rules */
	char arg[MIL];
	CHAR_DATA *victim;

	one_argument(argument,arg);

	if (!arg[0]){
		ch->send("Follow who?\n\r");
		return;
	}

	if(!(victim = get_char_room(ch,NULL,arg))){
		ch->send("They aren't here.\n\r");
		return;
	}

	if (ch->isaff(AF_CHARM) && ch->master){
		act("But you'd rather follow $N!",ch,NULL,ch->master,TO_CHAR);
		return;
	}

	if (victim == ch){
		if (ch->master == NULL){
			send_to_char("You already follow yourself.\n\r",ch);
			return;
		}
		stop_follower(ch);
		return;
	}

	if (!IS_NPC(victim) && victim->isplr(PL_NOFOLLOW) && !IS_IMMORTAL(ch)){
		act("$N doesn't seem to want any followers.\n\r",ch,NULL,victim,TO_CHAR);
		return;
	}

	ch->remplr(PL_NOFOLLOW);

	if (ch->master)
		stop_follower(ch);

	add_follower(ch,victim,true);
}

void add_follower(CHAR_DATA *ch,CHAR_DATA *master,bool cansee){
	if (ch->master != NULL){
		bug("Add_follower: non-null master.",0);
		return;
	}

	ch->master = master;
	//ungroup_char(master,ch);

	if (cansee == true){
		if (can_see(master,ch))
			act("$n now follows you.",ch,NULL,master,TO_VICT);

		act("$n now follows $N.",ch,NULL,master,TO_NOTVICT);
		act("You now follow $N.",ch,NULL,master,TO_CHAR);
	}
}

void stop_follower(CHAR_DATA *ch){
	if (ch->master == NULL){
		bug("Stop_follower: null master.",0);
		return;
	}

	if (ch->isaff(AF_CHARM)){
		ch->remaff(AF_CHARM);
		affect_strip(ch,gsn_charm_person);
		remove_charmy(ch->master,ch);
	}

	if (can_see(ch->master,ch) && ch->in_room != NULL){
		act("$n stops following you.",ch,NULL,ch->master,TO_VICT);
		act("You stop following $N.",ch,NULL,ch->master,TO_CHAR);
		act("$n stops following $N.",ch,NULL,ch->master,TO_NOTVICT);
	}
	if (ch->master->pet == ch)
		ch->master->pet = NULL;

	ch->master = NULL;
	affect_strip(ch,gsn_stalk);
	//ch->leader = NULL;
}

void nuke_pets(CHAR_DATA *ch){
	CHAR_DATA *pet;

	if (IS_NPC(ch))
		return;

	if ((pet = ch->pet) != NULL){
		stop_follower(pet);
		if (pet->in_room != NULL)
    		act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
		extract_char(pet,true);
	}
	ch->pet = NULL;

	if ((pet = ch->mount)){
		if (IS_NPC(pet)){
			do_dismount(ch,"");
			if (pet->in_room)
				act("$N slowly fades away.",ch,NULL,pet,TO_NOTVICT);
			else
				log_f("void nuke_pets: Extracting %s's nullROOM pet, %s.",ch->name,pet->short_descr);
			ch->mounted = false;
			extract_char(pet,true);
		}
		else
			log_f("void nuke_pets: %s was using PC %s as a horse...",ch->name,pet->name);
	}
	ch->mount = NULL;
}

void die_follower(CHAR_DATA *ch,CHAR_DATA *victim){
	CHAR_DATA *fch;

	if (ch->master){
		if (ch->master->pet == ch)
    		ch->master->pet = NULL;
		stop_follower(ch);
	}

	//ungroup_char(victim,ch);

	if (!victim){
		for (fch = char_list;fch != NULL;fch = fch->next){
			if(fch->isaff(AF_STALK))
				continue;
			if (fch->master == ch)
				stop_follower(fch);
			if (get_leader(fch) == ch)
				fch->leader = fch;
		}
	}
	else{//Pray this never gets called
		ch->send("You just got served!\n\r");
		if (fch->master == ch && get_leader(fch) == ch){
			stop_follower(fch);
			ungroup_char(fch->leader,fch);
		}
		else{
			if (fch->master == ch)
				stop_follower(fch);
			else if (get_leader(fch) == ch)
				fch->leader = fch;
			else
				ch->send("They're not following you.\n\r");
		}
	}
}

void do_order(CHAR_DATA *ch,char *argument){
	char buf[MSL],arg[MIL],arg2[MIL];
	CHAR_DATA *victim,*och,*och_next;
	bool found,fAll;

	return;//Nashboo

	argument = one_argument(argument,arg);
	one_argument(argument,arg2);

	if(!str_cmp(arg2,"delete") || !str_cmp(arg2,"mob")){
		send_to_char("That will NOT be done.\n\r",ch);
		return;
	}

	if(!arg[0] || !argument[0]){
		send_to_char( "Order whom to do what?\n\r", ch );
		return;
	}

	if(ch->isaff(AF_CHARM)){
		send_to_char("You feel like taking, not giving, orders.\n\r",ch);
		return;
	}

	if(!str_cmp(arg,"all")){
		fAll = true;
		victim = NULL;
	}
	else{
		fAll = false;

		if(!strcmp(arg,"mount")){
			if (!ch->mount){
				send_to_char("Your don't have a mount.\n\r",ch);
				return;
			}

			if (ch->mount->in_room != ch->in_room){
				send_to_char("Your mount isn't here!\n\r",ch);
				return;
			}
			else
				victim = ch->mount;
		}
		else if ((victim = get_char_room(ch,NULL,arg)) == NULL){
			send_to_char("They aren't here.\n\r",ch);
			return;
		}

		if (victim == ch){
			send_to_char("Aye aye, right away!\n\r",ch);
			return;
		}

/*		if (victim->mount == ch){
			if (!mount_success(ch,victim,false)){
				act("$N ignores your orders.",ch,NULL,victim,TO_CHAR);
				return;
			}
			else{
				printf_to_char(victim,"%s orders you to \'%s\'.",ch->name,argument);
				interpret(victim,argument);
				return;
			}
		}
		else{
			if (IS_NPC(victim)){
				if (!victim->isaff(AF_CHARM)){
					send_to_char("They're not charmed!\n\r",ch);
					return;
				}
				if (victim->master != ch){
					send_to_char("Do it yourself!\n\r",ch);
					return;
				}
				found = true;
			}
			else{
				if (IS_IMMORTAL(victim) && victim->trust >= ch->trust){
					send_to_char("Their power is too great for you to control!\n\r",ch);
					return;
				}
				found = true;
			}
		}*/
	}

	found = false;
	for ( och = ch->in_room->people; och != NULL; och = och_next ){
		och_next = och->next_in_room;

		if (och->isaff(AF_CHARM) && och->master == ch && (fAll || och == victim)){
			found = true;
			sprintf(buf,"$n orders you to '%s'.",argument);
			act(buf,ch,NULL,och,TO_VICT);
			interpret(och,argument);
		}
	}

	if (found){
		WAIT_STATE(ch,PULSE_VIOLENCE);
		send_to_char("Ok.\n\r",ch);
	}
	else
		send_to_char("You have no followers here.\n\r",ch);
	return;
}

void group_show(CHAR_DATA *ch){
	CHAR_DATA *gch,*leader;
	char buf[MIL],buf2[MIL],buf3[MIL];

	leader = get_leader(ch);

	if(IS_NPC(ch))
		sprintf(buf,"%s's group:",PERS(leader,ch));
	else
		sprintf(buf,"%s's group:",ch->name);
	ch->send(" ____________________________________________________________________________\n\r");
	printf_to_char(ch,"/   %-27s                                              \\",buf);
	ch->send("\n\r|____________________________________________________________________________|\n\r");

	for(gch = char_list; gch != NULL; gch = gch->next ){
		if (is_same_group(gch,ch)){
			sprintf(buf, "%d{xhp",gch->max_hit);
			sprintf(buf2,"%d{xmp",gch->getmaxmana());
			sprintf(buf3,"%d{xmv",gch->max_move);
			printf_to_char(ch,"{x[{c%3d {g%3s{x] %-24s {c%5d{x/{C%-9s {g%5d{x/{G%-9s {m%5d{x/{M%-9s |\n\r",
				gch->level,
				classes[gch->pclass].who_name,
				capitalize(PERS(gch,ch)),
				gch->hit,buf,
				gch->getmana(),buf2,
				gch->move,buf3);
		}
	}
	ch->send("|____________________________________________________________________________|\n\r");
	ch->send("\\____________________________________________________________________________/\n\r");
}

void group_join(CHAR_DATA *ch,CHAR_DATA *victim){
	if(!victim){
		send_to_char("They aren't anywhere.\n\r",ch);
		return;
	}
	if(get_leader(ch) != ch){
		send_to_char("You are in a group already. Type 'leave' to leave it.\n\r",ch);
		return;
	}
	if (victim->level > ch->level + 10){
		ch->send("You are too weak for that group!\n\r");
		return;
	}
	if (victim->level < ch->level-10){
		ch->send("You are too strong for that group.\n\r");
		return;
	}
	if(ch->isaff(AF_CHARM)){
		ch->send("You are too charmed.\n\r",ch);
		return;
	}

	ch->group_request = victim;
	act_new("$n asks to join $N's group.",ch,NULL,victim,TO_NOTVICT,POS_RESTING);
	act_new("You ask to join $N's group.",ch,NULL,victim,TO_CHAR,POS_SLEEPING);
	act_new("$n asks to join your group.",ch,NULL,victim,TO_VICT,POS_SLEEPING);
}
void group_leave(CHAR_DATA *ch){
}

void group_add(CHAR_DATA *ch,CHAR_DATA *victim){
}
void group_remove(CHAR_DATA *ch,CHAR_DATA *victim){
}
void group_cancel(CHAR_DATA *ch){
}

void do_group(CHAR_DATA *ch,char *argument){
	char arg[MIL];
	CHAR_DATA *victim = get_char_world(ch,argument);

	argument = one_argument(argument,arg);

	if(!arg[0] || !str_cmp(arg,"show")){
		group_show(ch);
		return;
	}
	if(IS_NPC(ch))
		return;

	victim = grab_char(ch,argument,false);
	if(!str_cmp(arg,"join"))
		group_join(ch,victim);
	else if(!str_cmp(arg,"leave"))
		group_leave(ch);
	else if(!str_cmp(arg,"add"))
		group_add(ch,victim);
	else if(!str_cmp(arg,"remove"))
		group_remove(ch,victim);
	else if(!str_cmp(arg,"cancel"))
		group_cancel(ch);
}

void do_split(CHAR_DATA *ch,char *argument){
	char buf[MSL],arg1[MIL],arg2[MIL];
	CHAR_DATA *gch;
	int members,amount_gold = 0,amount_silver = 0,share_gold,share_silver,extra_gold,extra_silver;

	argument = one_argument(argument,arg1);
	one_argument(argument,arg2);

	if(!arg1[0]){
		send_to_char("Split how much?\n\r",ch);
		return;
	}

	amount_silver = atoi(arg1);

	if(arg2[0])
		amount_gold = atoi(arg2);

	if(amount_gold < 0 || amount_silver < 0){
		send_to_char("Your group wouldn't like that.\n\r",ch);
		return;
	}

	if (amount_gold == 0 && amount_silver == 0){
		send_to_char("You hand out zero coins, but no one notices.\n\r",ch);
		return;
	}

	if(ch->gold <  amount_gold || ch->silver < amount_silver){
		send_to_char("You don't have that much to split.\n\r",ch);
		return;
	}

	members = 0;
	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room )
		if (is_same_group(gch,ch) && !IS_NPC(gch))
			members++;

	if (members < 2){
		//send_to_char("Just keep it all.\n\r",ch);
		return;
	}
		
	share_silver = amount_silver / members;
	extra_silver = amount_silver % members;

	share_gold   = amount_gold / members;
	extra_gold   = amount_gold % members;

	if (!share_gold && !share_silver){
		send_to_char("Don't even bother, cheapskate.\n\r",ch);
		return;
	}

	ch->silver	-= amount_silver;
	ch->silver	+= share_silver + extra_silver;
	ch->gold 	-= amount_gold;
	ch->gold 	+= share_gold + extra_gold;

	if (share_silver > 0)
		printf_to_char(ch,"You split {w%d silver {xcoins. Your share is {w%d silver{x.\n\r",amount_silver,share_silver + extra_silver);

	if (share_gold > 0)
		printf_to_char(ch,"You split {Y%d gold {xcoins. Your share is {Y%d gold{x.\n\r",amount_gold,share_gold + extra_gold);

	if (share_gold == 0)
		sprintf(buf,"$n splits {w%d silver {xcoins. Your share is {w%d silver{x.", amount_silver,share_silver);
	else if (share_silver == 0)
		sprintf(buf,"$n splits {Y%d gold {xcoins. Your share is {Y%d gold{x.",amount_gold,share_gold);
	else
		sprintf(buf, "$n splits {w%d silver {xand {Y%d gold {xcoins, giving you {w%d silver {xand {Y%d gold{x.\n\r", amount_silver,amount_gold,share_silver,share_gold);

	for ( gch = ch->in_room->people; gch != NULL; gch = gch->next_in_room ){
		if ( gch != ch && is_same_group(gch,ch) && !IS_NPC(gch)){
			act(buf,ch,NULL,gch,TO_VICT);
			gch->gold += share_gold;
			gch->silver += share_silver;
		}
	}
}

void do_gtell(CHAR_DATA *ch,char *argument){
	CHAR_DATA *gch;

	if(!argument[0]){
		send_to_char("Tell your group what?\n\r",ch);
		return;
	}

	act_new("You tell the group '{d$T{x'",ch,NULL,argument,TO_CHAR,POS_DEAD);

	if (ch->iscomm(CM_NOTELL)){
		send_to_char("Your message didn't get through!\n\r",ch);
		return;
	}

	for ( gch = char_list; gch != NULL; gch = gch->next )
		if (is_same_group(gch,ch))
			act_new("$n tells the group '{d$t{x'",ch,argument,gch,TO_VICT,POS_SLEEPING);
}

/*
 * It is very important that this be an equivalence relation:
 * (1) A ~ A
 * (2) if A ~ B then B ~ A
 * (3) if A ~ B  and B ~ C, then A ~ C
 */
bool is_same_group(CHAR_DATA *ach,CHAR_DATA *bch){
    if(!ach || !bch)
		return false;

	ach = get_leader(ach);
	bch = get_leader(bch);
    return ach == bch;
}

void do_mobgroupcheck(CHAR_DATA *ch,char *argument){
	CHAR_DATA *victim = get_char_room(ch,NULL,argument);
	if(!victim)
		return;
	printf_to_char(ch,"%s leader: %s\n\r",victim->name,get_leader(victim)->name);
}

void ungroup_char(CHAR_DATA *ch,CHAR_DATA *victim){
	victim->leader = victim;
}

void group_char(CHAR_DATA *ch,CHAR_DATA *victim){
	victim->leader = ch;
}

CHAR_DATA *get_leader(CHAR_DATA *ch){
	if(IS_NPC(ch)){
		if(ch->leader)
			return ch->leader;
		else
			return ch;
	}
	if(ch->leader != ch)
		return ch->leader;
	return ch;
}
