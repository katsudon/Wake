#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"
#include "interp.h"


class n_pool{
	int curr;
	int maxn;

	void modn(int);
};

void n_pool :: modn(int amount){
	curr += amount;
	if (curr > maxn)
		curr = maxn;
}

int char_data :: getmana(){
	if (classes[pclass].amp)
		return antimana;
	else
		return mana;
}

int char_data :: getslvl(int sn){
	if (act[AT_IS_NPC])
		return 2;
	else
		return pcdata->skill_level[sn];
}

void char_data :: setmana(int value){
	if (classes[pclass].amp)
		antimana = value;
	else
		mana = value;
}

int char_data :: gettruemana(){
	return mana;
}
int char_data :: gettrueantimana(){
	return antimana;
}
int char_data :: gettruemaxmana(){
	return max_mana;
}
int char_data :: gettruemaxantimana(){
	return max_antimana;
}

void char_data :: settruemana(int value){
	mana = value;
}
void char_data :: settrueantimana(int value){
	antimana = value;
}
void char_data :: settruemaxmana(int value){
	max_mana = value;
}
void char_data :: settruemaxantimana(int value){
	max_antimana = value;
}

void char_data :: modtruemana(int value){
	mana += value;
}
void char_data :: modtrueantimana(int value){
	antimana += value;
}
void char_data :: modtruemaxmana(int value){
	max_mana += value;
}
void char_data :: modtruemaxantimana(int value){
	max_antimana += value;
}

void char_data :: modmana(int value){
	if (classes[pclass].amp)
		antimana += value;
	else
		mana += value;
}

int char_data :: getmaxmana(){
	if (classes[pclass].amp)
		return max_antimana;
	else
		return max_mana;
}

void char_data :: setmaxmana(int value){
	if (classes[pclass].amp)
		max_antimana = value;
	else
		max_mana = value;
}

void char_data :: modmaxmana(int value){
	if (classes[pclass].amp)
		max_antimana += value;
	else
		max_mana += value;
}

void char_data :: manadamage(int value){
	mana -= value;
	mana = UMAX(0,mana);
}
//NASHGROUNDRULES if level is <0 the skill is unpickable
void pc_data :: up_skill(int level,int percent,int gsn){
	/*if(level > -1){
		if(percent > 0){
			learned[gsn] = percent;
			skill_level[gsn] = level;
		}
		else
			skill_level[gsn] = level;
	}
	else{
		if(percent > 0)
			learned[gsn] = percent;
		else;
	}*/

	learned[gsn] = percent;
	skill_level[gsn] = level;
}

void pc_data :: lock_skill(int gsn){
	skill_level[gsn] = -1;
}
