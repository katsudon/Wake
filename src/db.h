#ifndef _DB_H_
#define _DB_H_
extern bool						fBootDb;
extern int						newmobs;
extern int						newobjs;
extern MOB_INDEX_DATA 	*		mob_index_hash          [MAX_KEY_HASH];
extern OBJ_INDEX_DATA 	*		obj_index_hash          [MAX_KEY_HASH];
extern int						top_mob_index;
extern int						top_obj_index;
extern int  					top_affect;
extern int						top_ed;
extern AREA_DATA 	*			area_first;
extern int						maxSocial;
extern int						MAX_GUILD;
extern int						MAX_HOMETOWN;
void convert_mob				( MOB_INDEX_DATA* );
void convert_obj				(OBJ_INDEX_DATA* );
extern void assign_area_vnum	( int );
void convert_objects			( void );
void convert_object				( OBJ_INDEX_DATA* );

#define MAGIC_NUM 52571214
#define GET_UNSET(flag1,flag2)	(~(flag1)&((flag1)|(flag2)))
#endif
