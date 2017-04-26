#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */
char *fix_string( const char *str );

void qedit_show(){
}
void save_quests(FILE *fp,AREA_DATA *pArea){
	PROG_CODE *pMprog;
	int i;

	fprintf(fp,"#QUESTS\n");
	for(i = pArea->min_vnum;i <= pArea->max_vnum;++i)
		if ((pMprog = get_prog_index(i,PRG_MPROG))){
			fprintf(fp,"#%d\n",i);
			fprintf(fp,"%s~\n",fix_string(pMprog->code));
		}
	fprintf(fp,"#0\n\n");
}
void load_quests(FILE *fp){
	PROG_CODE *pMprog;

	if (area_last == NULL){
		bug("Load_mobprogs: no #AREA seen yet.",0);
		exit(1);
	}

	for (;;){
		sh_int vnum;
		char letter;

		letter = fread_letter(fp);
		if (letter != '#'){
			bug("Load_mobprogs: # not found.",0);
			exit(1);
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		//fBootDb = false;NASHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
		if (get_prog_index(vnum,PRG_MPROG) != NULL){
			bug("Load_mobprogs: vnum %d duplicated.",vnum);
			exit(1);
		}
		//fBootDb = true;NASHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH

		pMprog = (PROG_CODE *)alloc_perm(sizeof(*pMprog));
		pMprog->vnum = vnum;
		pMprog->code = fread_string(fp);
		if (mprog_list == NULL)
			mprog_list = pMprog;
		else{
			pMprog->next = mprog_list;
			mprog_list = pMprog;
		}
		//top_mprog_index++;NASHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH
	}
}
