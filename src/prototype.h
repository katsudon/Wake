#ifndef _PROTOTYPE_H_
#define _PROTOTYPE_H_

#define CD	CHAR_DATA
#define MID	MOB_INDEX_DATA
#define OD	OBJ_DATA
#define OID	OBJ_INDEX_DATA
#define RID	ROOM_INDEX_DATA
#define SF	SPEC_FUN
#define AD	AFFECT_DATA
#define MPC	MPROG_CODE

#define IS_VALID(data)		((data) != NULL && (data)->valid)
#define VALIDATE(data)		((data)->valid = true)
#define INVALIDATE(data)	((data)->valid = false)
#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c)		( (b) < (a) ? (a) : ((b) > (c) ? (c) : (b)) )
#define LOWER(c)		((c) >= 'A' && (c) <= 'Z' ? (c)+'a'-'A' : (c))
#define UPPER(c)		((c) >= 'a' && (c) <= 'z' ? (c)+'A'-'a' : (c))
#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))
#define IS_NULLSTR(str)		((str) == NULL || (str)[0] == '\0')
#define ENTRE(min,num,max)	( ((min) < (num)) && ((num) < (max)) )
#define SEX(ch) (URANGE(0,(ch)->sex,2))
#define CHECK_POS(a, b, c)	{							\
					(a) = (b);					\
					if ( (a) < 0 )					\
					bug( "CHECK_POS : " c " == %d < 0", a );	\
				}							\

#define ALT_FLAGVALUE_SET( _blargh, _table, _arg )		\
	{							\
		int blah = flag_value( _table, _arg );		\
		_blargh = (blah == NO_FLAG) ? 0 : blah;		\
	}
#define IS_VOWELL(letter)	(LOWER(letter) == 'a' || LOWER(letter) == 'e' || LOWER(letter) == 'i' || LOWER(letter) == 'o' || LOWER(letter) == 'u')
#define IS_NPC(ch)		((ch)->isact(AT_IS_NPC))
#define KILLFUNCT_TRUE(ch,stuff) ({(ch)->send((stuff));return true;})
#define KILLFUNCT_FALSE(ch,stuff) ({(ch)->send((stuff));return false;})
#define KILLFUNCT(ch,stuff) ({(ch)->send((stuff));return;})
#define IS_IMMORTAL(ch)		(get_trust(ch) >= LEVEL_IMMORTAL)
#define IS_HERO(ch)		(get_trust(ch) >= LEVEL_HERO)
#define IS_TRUSTED(ch,level)	(get_trust((ch)) >= (level))
#define IS_AFFECTED(ch, sn)	(affect_compare(ch,sn))
#define IS_SPELL(sn) (skill_table[(sn)].slot == SKILL_SPELL)

#define IS_SKILL_NATURAL(sn) (skill_table[sn].slot == SKILL_NATURAL ? true : false)

#define GET_AGE(ch)		((int) (17 + ((ch)->played + current_time - (ch)->logon )/72000))

#define IS_GOOD(ch)		(ch->alignment >= 350)
#define IS_EVIL(ch)		(ch->alignment <= -350)
#define IS_NEUTRAL(ch)		(!IS_GOOD(ch) && !IS_EVIL(ch))

#define IS_AWAKE(ch)		(ch->position > POS_SLEEPING)
#define IS_OUTSIDE(ch)		(!IS_SET((ch)->in_room->room_flags,ROOM_INDOORS) && !sect_table[(ch)->in_room->sector_type].isindoors)
#define WAIT_STATE(ch, npulse)	((ch)->wait = UMAX((ch)->wait, (npulse)))
#define DAZE_STATE(ch, npulse)  ((ch)->daze = UMAX((ch)->daze, (npulse)))
#define get_carry_weight(ch)	((ch)->carry_weight + (ch)->silver/10 + (ch)->gold * 2 / 5)
#define MOUNTED(ch) ((!IS_NPC(ch) && ch->mount && ch->mounted) ? ch->mount : NULL)
#define RIDDEN(ch) ((ch->isact(AT_MOUNT) && ch->ismount && ch->mount && ch->mounted) ? ch->mount : NULL)
#define RIDING(ch)((!ch->isact(AT_MOUNT) && !ch->ismount && ch->mount && ch->mounted) ? ch->mount : NULL)
#define IS_DRUNK(ch) ((ch->pcdata->condition[COND_DRUNK] > 10))
#define act(format,ch,arg1,arg2,type) act_new((format),(ch),(arg1),(arg2),(type),POS_RESTING)
#define HAS_TRIGGER_MOB(ch,trig)	(IS_SET((ch)->pIndexData->mprog_flags,(trig)))
#define HAS_TRIGGER_OBJ(obj,trig) (IS_SET((obj)->pIndexData->oprog_flags,(trig)))
#define HAS_TRIGGER_ROOM(room,trig) (IS_SET((room)->rprog_flags,(trig)))
#define IS_SWITCHED( ch )       ( ch->desc && ch->desc->original )
#define IS_BUILDER(ch, Area)	( !IS_NPC(ch) && !IS_SWITCHED( ch ) && ( ch->pcdata->security >= Area->security || strstr( Area->builders, ch->name ) || strstr( Area->builders, "All" ) ) )
#define CAN_WEAR(obj, part)	(IS_SET((obj)->wear_flags,  (part)))
#define IS_OBJ_STAT(obj, stat)	(IS_SET((obj)->extra_flags, (stat)))
#define WEIGHT_MULT(obj)	((obj)->item_type == ITEM_CONTAINER ? (obj)->value[4] : 100)

#define PERS(ch, looker)	( can_see( looker, (ch) ) ?	(IS_NPC(ch) ? (ch)->short_descr : (ch)->name) : "someone")

/* act_comm.c */
char	*get_pre			args( ( CHAR_DATA *ch, int channel) );
void	do_newchan			args( ( CHAR_DATA *ch, char *argument, int channel) );
bool	smote_check			args( ( CHAR_DATA *ch, char *argument, char *emote, int channel ) );
bool	echo_check			args( ( CHAR_DATA *ch, char *argument, char *emote, int channel ) );
bool	check_chansocial	args( ( CHAR_DATA *ch, char *command, char *argument, int channel ) );
char	*get_emote			args( ( CHAR_DATA *ch, char *argument) );
bool	can_be_sent			args( ( CHAR_DATA *ch, CHAR_DATA *vch, int channel ) );
void  	check_sex			args( ( CHAR_DATA *ch) );
void	add_follower		args( ( CHAR_DATA *ch, CHAR_DATA *master, bool cansee ) );
void	stop_follower		args( ( CHAR_DATA *ch ) );
void 	nuke_pets			args( ( CHAR_DATA *ch ) );
void	die_follower		( CHAR_DATA*,CHAR_DATA* );
bool	is_same_group		args( ( CHAR_DATA *ach, CHAR_DATA *bch ) );
void 	log_f 				args((char * fmt, ...));

/* act_enter.c */
int find_random_exit			( ROOM_INDEX_DATA* );
RID  *get_random_room		args ( (CHAR_DATA *ch) );
RID  *get_random_room_area	args ( (CHAR_DATA *ch) );
/* act_group.c */
CHAR_DATA *get_leader		( CHAR_DATA* );
void	ungroup_char		( CHAR_DATA*,CHAR_DATA* );
void	group_char		( CHAR_DATA*,CHAR_DATA* );
/* act_info.c */
void	set_title			args( ( CHAR_DATA *ch, char *title ) );

/* act_move.c */
void	move_char			args( ( CHAR_DATA *ch, int door, bool follow, bool cSee ) );

/* act_obj.c */
bool	can_loot			args( (CHAR_DATA *ch, OBJ_DATA *obj) );
void	wear_obj			args( (CHAR_DATA *ch, OBJ_DATA *obj, bool fReplace, bool canseethis ) );
void    get_obj				args( ( CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *container,bool csee ) );

/* act_wiz.c */
void	wiznet				args( (char *string, CHAR_DATA *ch, OBJ_DATA *obj, long flag, long flag_skip, int min_level ) );
void	copyover_recover	args((void));
void	global_message		args( ( int minL, int maxL, const char *txt,int) );

/* alias.c */
void 	substitute_alias	args( (DESCRIPTOR_DATA *d, char *input) );

/* ban.c */
bool	check_ban			args( ( char *site, int type) );

/* bit.c */
bool	super_set_bit		( bool*,char*,const struct flag_type*,bool );
void	read_bits			( bool*,int,char* );
void	read_ints			( int*,int,char* );
char	*save_bits			( bool*,int );
char	*save_ints			( int*,int );
void	clone_bits			( bool*,bool*,int );
void	clone_ints			( int*,int*,int );
bool	has_bits			( bool*,char*,const struct flag_type* );

/* comm.c */
void    printf_to_char		args( ( CHAR_DATA *ch, char *fmt, ...) );
void	show_string			args( ( struct descriptor_data *d, char *input) );
void	close_socket		args( ( DESCRIPTOR_DATA *dclose ) );
void	write_to_buffer		args( ( DESCRIPTOR_DATA *d, const char *txt,int length ) );
void    send_to_desc		args( ( const char *txt, DESCRIPTOR_DATA *d ) );
void	send_to_char		args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char		args( ( const char *txt, CHAR_DATA *ch ) );
void	act					args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type ) );
void	act_new				args( ( const char *format, CHAR_DATA *ch, const void *arg1, const void *arg2, int type, int min_pos) );

/* cql.c */
void cql_load_db			( );
void cql_save_char			( CHAR_DATA* );

/* Color stuff by Lope of Loping Through The MUD */
int		color				args( ( char type, CHAR_DATA *ch, char *string ) );
void	colorconv			args( ( char *buffer, const char *txt, CHAR_DATA *ch ) );
void	send_to_char_bw		args( ( const char *txt, CHAR_DATA *ch ) );
void	page_to_char_bw		args( ( const char *txt, CHAR_DATA *ch ) );
void	printf_to_char		args( ( CHAR_DATA *, char *, ... ) );
void	bugf				args( ( char *, ... ) );

/* db.c */
void	reset_area			args( ( AREA_DATA * pArea ) );		/* OLC */
void	reset_room			args( ( ROOM_INDEX_DATA *pRoom ) );	/* OLC */
char *	print_flags			args( ( l_int flag ));
void	boot_db				args( ( void ) );
void	area_update			args( ( void ) );
CD *	create_mobile		args( ( MOB_INDEX_DATA *pMobIndex ) );
void	clone_mobile		args( ( CHAR_DATA *parent, CHAR_DATA *clone) );
OD *	create_object		args( ( OBJ_INDEX_DATA *pObjIndex, int level ) );
void	clone_object		args( ( OBJ_DATA *parent, OBJ_DATA *clone ) );
void	clear_char			args( ( CHAR_DATA *ch ) );
char *	get_extra_descr		args( ( const char *name, EXTRA_DESCR_DATA *ed ) );
MID *	get_mob_index		args( ( int vnum ) );
OID *	get_obj_index		args( ( int vnum ) );
RID *	get_room_index		args( ( int vnum ) );
PROG_CODE *    get_prog_index args( ( int vnum, int type ) );
char	fread_letter		args( ( FILE *fp ) );
int		fread_number_line		args( ( FILE *fp, int, const char * ) );
int		fread_flag			args( ( FILE *fp ) );
char *	fread_string		args( ( FILE *fp ) );
char *  fread_string_eol	args(( FILE *fp ) );
void	fread_to_eol		args( ( FILE *fp ) );
char *	fread_word			args( ( FILE *fp ) );
l_int	flag_convert		args( ( char letter) );
void *	alloc_mem			args( ( int sMem ) );
void *	alloc_perm			args( ( int sMem ) );
void	free_mem			args( ( void *pMem, int sMem ) );
char *	str_dup				args( ( const char *str ) );
void	free_string			args( ( char *pstr ) );
int		number_fuzzy		args( ( int number ) );
int		number_range		args( ( int from, int to ) );
int		number_percent		args( ( void ) );
int		number_door			args( ( void ) );
int		get_random_door		args( ( ROOM_INDEX_DATA* ) );
int		number_bits			args( ( int width ) );
long	number_mm			args( ( void ) );
int		dice				args( ( int number, int size ) );
int		interpolate			args( ( int level, int value_00, int value_32 ) );
void	smash_tilde			args( ( char *str ) );
bool	str_cmp				args( ( const char *astr, const char *bstr ) );
bool	str_prefix			args( ( const char *astr, const char *bstr ) );
bool	str_infix			args( ( const char *astr, const char *bstr ) );
bool	str_suffix			args( ( const char *astr, const char *bstr ) );
char *	capitalize			args( ( const char *str ) );
void	append_file			args( ( CHAR_DATA *ch, char *file, char *str ) );
void	bug					args( ( const char *str, int param ) );
void	log_string			args( ( const char *str ) );
void	tail_chain			args( ( void ) );

/* db2.c */
void	load_classes		args( (void) );
void	load_guilds			();

/* effect.c */
void	acid_effect			args( (void *vo, int level, int dam, int target) );
void	cold_effect			args( (void *vo, int level, int dam, int target) );
void	fire_effect			args( (void *vo, int level, int dam, int target) );
void	poison_effect		args( (void *vo, int level, int dam, int target) );
void	shock_effect		args( (void *vo, int level, int dam, int target) );
void  	check_traps			args( ( CHAR_DATA*,OBJ_DATA* ) );
int		dam_res				( CHAR_DATA*,CHAR_DATA*,int,int );

/* fight.c */
bool	victim_die			( CHAR_DATA*,CHAR_DATA* );
bool	skill_damage		( CHAR_DATA*,CHAR_DATA*,int,int,int,bool );
bool 	is_safe				args( (CHAR_DATA *ch, CHAR_DATA *victim ) );
bool 	is_safe_spell		args( (CHAR_DATA *ch, CHAR_DATA *victim, bool area ) );
void	violence_update		args( ( void ) );
void	multi_hit			( CHAR_DATA*,CHAR_DATA*,int,bool );
bool	damage				args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam, int dt, int clas, bool show ) );
bool    damage_old			args( ( CHAR_DATA *ch, CHAR_DATA *victim, int dam,int dt, int clas, bool show ) );
void	update_pos			( CHAR_DATA*,CHAR_DATA* );
void	stop_fighting		args( ( CHAR_DATA *ch, bool fBoth ) );
void	check_killer		args( ( CHAR_DATA *ch, CHAR_DATA *victim) );

/* handler.c */
void affect_join			( CHAR_DATA*,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector );
void affect_set				( CHAR_DATA*,CHAR_DATA*,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector );
void affect_set				( CHAR_DATA*,ROOM_INDEX_DATA*,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector );
void affect_set				( CHAR_DATA*,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector );
void affect_set				( OBJ_DATA*,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector );
void affect_set				( ROOM_INDEX_DATA*,int towhere,int type,int level,int slvl,int duration,int location,int modifier,int bitvector );
void	purge_affect		( int,CHAR_DATA*,int,int );
bool	roll_chance			( CHAR_DATA*,int );
int		grab_skill			( CHAR_DATA*,int );
AD  	*affect_find		( AFFECT_DATA*,int );
void	affect_check		( CHAR_DATA*,int,int );
int		count_users			( OBJ_DATA* );
void 	deduct_cost			( CHAR_DATA*,int );
void	affect_enchant		( OBJ_DATA* );
int 	check_immune		( CHAR_DATA*,int );
bool 	material_metal		( OBJ_DATA* );
int		material_lookup		( const char* );
int		weapon_lookup		( const char* );
int		weapon_type			( const char* );
int		trapy_type			( const char* );
char 	*weapon_name		( int );
char 	*trap_name			( int );
char	*item_name			( int );
int		attack_lookup		( const char* );
int		climate_lookup		( const char* );
long	wiznet_lookup		( const char* );
bool	is_clan				( CHAR_DATA* );
int		class_lookup		( const char* );
int		stat_lookup			( const char* );
int		abbrev_stat_lookup	( const char* );
char *	abbrev_stat_name	( int );
int		guild_lookup		( const char* );
int		home_lookup			( const char* );
bool	is_old_mob			( CHAR_DATA* );
int		get_skill			( CHAR_DATA*,int );
int		get_weapon_sn		( CHAR_DATA*,bool );
int		get_weapon_skill	( CHAR_DATA*,int );
int     get_age				( CHAR_DATA* );
void	reset_char			( CHAR_DATA* );
int		get_trust			( CHAR_DATA* );
int		get_curr_stat		( CHAR_DATA*,int );
int 	get_max_train		( CHAR_DATA*,int );
int		can_carry_n			( CHAR_DATA* );
int		can_carry_w			( CHAR_DATA* );
bool	is_name				( char*,char* );
bool    is_full_name		( const char*,char* );
bool	is_exact_name		( char*,char* );
void	affect_to_char		( CHAR_DATA*,AFFECT_DATA* );
void	affect_to_obj		( OBJ_DATA*,AFFECT_DATA* );
void	affect_remove		( CHAR_DATA*,AFFECT_DATA* );
void	affect_remove_obj	( OBJ_DATA*,AFFECT_DATA* );
void	affect_strip		( CHAR_DATA*,int );
bool	is_affected			( CHAR_DATA*,int );
void	affect_join			( CHAR_DATA*,AFFECT_DATA* );
void	char_from_room		( CHAR_DATA* );
void	char_to_room		( CHAR_DATA*,ROOM_INDEX_DATA* );
void	obj_to_char			( OBJ_DATA*,CHAR_DATA* );
void	obj_from_char		( OBJ_DATA* );
int		apply_ac			( CHAR_DATA*,OBJ_DATA*,int,int );
OD *	get_eq_char			( CHAR_DATA*,int );
OD *	get_it_char			( CHAR_DATA*,int );
OD *	get_eq_hold			( CHAR_DATA* );
void	equip_char			args( ( CHAR_DATA *ch, OBJ_DATA *obj, int iWear ) );
void	unequip_char		args(( CHAR_DATA *ch, OBJ_DATA *obj ) );
int		count_obj_list		args(( OBJ_INDEX_DATA *obj, OBJ_DATA *list ) );
void	obj_from_room		args(( OBJ_DATA *obj ) );
void	obj_to_room			args(( OBJ_DATA *obj, ROOM_INDEX_DATA *pRoomIndex ) );
void	obj_to_obj			args(( OBJ_DATA *obj, OBJ_DATA *obj_to ) );
void	obj_from_obj		args(( OBJ_DATA *obj ) );
void	extract_obj			args(( OBJ_DATA *obj ) );
void	extract_char		args(( CHAR_DATA *ch, bool fPull ) );
CD *	get_char_room		args(( CHAR_DATA *ch, ROOM_INDEX_DATA *room, char *argument ) );
CD *	get_char_around		args(( CHAR_DATA *ch, char *argument, int mdepth, bool throughdoor ) );
CD *	get_char_world		args(( CHAR_DATA *ch, char *argument ) );
OD *	get_obj_type		args(( OBJ_INDEX_DATA *pObjIndexData ) );
OD *	get_obj_list		args(( CHAR_DATA *ch, char *argument, OBJ_DATA *list ) );
OD *	get_obj_carry		args(( CHAR_DATA*,char*,CHAR_DATA* ));
OD *	get_obj_wear		args(( CHAR_DATA*,char*,bool ));
OD *	get_obj_here		args(( CHAR_DATA*,ROOM_INDEX_DATA*,char* ));
OD *	get_obj_world		args(( CHAR_DATA*,char* ));
OD *	get_obj_id			args(( int ));
OD *	create_money		args(( int gold, int silver ) );
int		get_obj_number		args(( OBJ_DATA *obj ) );
int		get_obj_weight		args(( OBJ_DATA *obj ) );
int		get_true_weight		(OBJ_DATA*,bool);
bool	room_is_dark		args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	is_room_owner		args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *room) );
bool	room_is_private		args( ( ROOM_INDEX_DATA *pRoomIndex ) );
bool	can_see				args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_move		args( ( CHAR_DATA *ch, CHAR_DATA *victim ) );
bool	can_see_obj			args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
bool	can_see_room		args( ( CHAR_DATA *ch, ROOM_INDEX_DATA *pRoomIndex) );
bool	can_drop_obj		args( ( CHAR_DATA *ch, OBJ_DATA *obj ) );
char *	affect_loc_name		( int );
char *	affect_bit_name		( int );
char *	affect_bits_name	( bool* );
char *	extra_bit_name		( l_int );
char *	exclude_bit_name	( l_int );
char * 	wear_bit_name		( int );
char *	act_bit_name		( int );
char *	act_bits_name		( bool* );
char *	off_bit_name		( int );
char *	off_bits_name		( bool* );
char *	def_bit_name		( int );
char *	def_bits_name		( bool* );
char *	armortype_bit_name	( int );
char *  res_bit_name		( int );
char *  res_bits_name		( int* );
char * 	form_bit_name		( int );
char * 	form_bits_name		( bool* );
char *	part_bit_name		( int );
char *	part_bits_name		( bool* );
char *	weapon_bit_name		( int );
char *	weapon_bits_name	( bool* );
char *  comm_bit_name		( int );
char *  comm_bits_name		( bool* );
char *	cont_bit_name		( int );
int     god_lookup			( const char* );
CD	 *	get_char_area		( CHAR_DATA*,char* );
void	affect_to_room		( ROOM_INDEX_DATA*,AFFECT_DATA* );
void	affect_remove_room	( ROOM_INDEX_DATA*,AFFECT_DATA* );

/* interp.c */
void	interpret			( CHAR_DATA*,char* );
bool	is_number			( char* );
int		number_argument		( char*,char* );
int		mult_argument		( char*,char* );
char *	one_argument		( char*,char* );

/* magic.c */
bool	check_dispel		( int,CHAR_DATA*,int );
void	spell_damage		( CHAR_DATA*,CHAR_DATA*,int,int,int,bool );
int		find_spell			( CHAR_DATA*,const char* );
int		find_spell_list		( const char* );
int 	mana_cost 			( CHAR_DATA*,int,int );
int		skill_lookup		( const char* );
int		skill_prefix		( const char*,bool );
int		skillspell_prefix	( const char* );
bool	saves_spell			( int,CHAR_DATA*,int );
bool	saves_skill			( int,CHAR_DATA*,int );
void	obj_cast_spell		( int,int,CHAR_DATA*,CHAR_DATA*,OBJ_DATA* );

/* mob_prog.c */
void	program_flow		( sh_int vnum, char *source, CHAR_DATA *mob,OBJ_DATA*,ROOM_INDEX_DATA*,CHAR_DATA *ch, const void *arg1,const void *arg2, int numlines );
PROG_CODE *get_prog_by_vnum (int vnum);
void	p_act_trigger		( char*,CHAR_DATA*,OBJ_DATA*,ROOM_INDEX_DATA*,CHAR_DATA*,const void*,const void*,int );
bool	p_percent_trigger	( CHAR_DATA*,OBJ_DATA*,ROOM_INDEX_DATA*,CHAR_DATA*,const void*,const void*,int );
void	p_bribe_trigger		( CHAR_DATA*,CHAR_DATA*,int );
bool	p_exit_trigger		( CHAR_DATA*,int,int );
void	p_give_trigger		( CHAR_DATA*,OBJ_DATA*,ROOM_INDEX_DATA*,CHAR_DATA*,OBJ_DATA*,int );
void 	p_greet_trigger		( CHAR_DATA*,int );
void	p_hprct_trigger		( CHAR_DATA*,CHAR_DATA* );

/* mob_cmds.c */
void	mob_interpret		( CHAR_DATA*,char* );
void	obj_interpret		( OBJ_DATA*,char* );
void	room_interpret		( ROOM_INDEX_DATA*,char* );

/* save.c */
void	save_char_obj		( CHAR_DATA* );
bool	load_char_obj		( DESCRIPTOR_DATA*,char* );

/* skills.c */
void	print_cp			( CHAR_DATA* );
void	print_gained		( CHAR_DATA* );
void	print_ungained		( CHAR_DATA* );
void 	list_group_costs	( CHAR_DATA* );
void    list_group_known	( CHAR_DATA* );
int 	exp_per_level		( CHAR_DATA* );
void 	check_improve		( CHAR_DATA*,int,bool,int );
void 	skillspell_add		( CHAR_DATA*,const char*,bool,bool );
void	skillspell_remove	( CHAR_DATA*,const char*,bool,bool );

/* social-edit.c */
void load_social_table();
void save_social_table();

/* special.c */
SF *	spec_lookup			( const char* );
char *	spec_name			( SPEC_FUN* );

/* teleport.c */
RID *	room_by_name		( char*,int,bool );

/* update.c */
void	advance_level		( CHAR_DATA*,bool );
void	gain_exp			( CHAR_DATA*,int );
void	gain_condition		( CHAR_DATA*,int,int );
void	update_handler		( void );
/*mem_mob.c*/
void mob_remember			( CHAR_DATA*,CHAR_DATA*,int,int );
void mem_fade				( CHAR_DATA* );
MEM_DATA *get_mem_data		( CHAR_DATA*,CHAR_DATA* );
/* mount.c */
int     mount_chance		( CHAR_DATA*,CHAR_DATA*);
void    do_mount			( CHAR_DATA*,char* );
void    do_dismount			( CHAR_DATA*,char* );
void	do_buy_mount		( CHAR_DATA*,char* );

/* string.c */
void	string_edit			( CHAR_DATA*,char** );
void    string_append		( CHAR_DATA*,char** );
char *	string_replace		( char*,char*,char* );
void    string_add			( CHAR_DATA*,char* );
char *  format_string		( char* );
char *  first_arg			( char*,char*,bool );
char *	string_unpad		( char* );
char *	string_proper		( char* );
bool	s_prefix			( string,string );
bool	s_exact				( string,string );

/* olc.c */
bool	run_olc_editor		( DESCRIPTOR_DATA* );
char *	olc_ed_name			( CHAR_DATA* );
char *	olc_ed_vnum			( CHAR_DATA* );

/* lookup.c */
int		GET_AC				( CHAR_DATA*,int );
int		GET_HITROLL			( CHAR_DATA* );
int		GET_DAMROLL			( CHAR_DATA* );
int		race_lookup			( const char* );
int		pc_race_lookup		( const char* );
int		item_lookup			( const char* );
int		liq_lookup			( const char* );
int		forge_lookup		( const char* );
int		pick_lookup			( const char* );
int		get_class			( CHAR_DATA* );
bool	is_class			( CHAR_DATA*, int);
bool	check_armor			( CHAR_DATA*,OBJ_DATA* );
//xrace.c
int		crunch_durability			( CHAR_DATA*,int );
bool	check_durability			( CHAR_DATA* );
void	check_bravery				( CHAR_DATA* );
int		check_icebound				( CHAR_DATA* );
void	do_disappear				( CHAR_DATA*,char* );
int		crunch_resilience			( CHAR_DATA*,int );
int		do_thalassic_aura			( CHAR_DATA*,int );
int		uplifting_hrdr				( CHAR_DATA*,int );
int		uplifting_ac				( CHAR_DATA*,int );
bool	check_amiability			( CHAR_DATA* );
void	vithe_return				( CHAR_DATA*,CHAR_DATA*,int );
void	vithe_regeneration			( CHAR_DATA* );
/* other */
void	init_race				( CHAR_DATA* );
void	check_dehorsing			( CHAR_DATA* );
void	unSneak					( CHAR_DATA* );
void	unSleep					( CHAR_DATA* );
bool	do_detecttraps			( CHAR_DATA* );
bool	check_eavesdrop			( CHAR_DATA* );
void	show_weather			( CHAR_DATA* );
bool	np_morethan				( int );
bool	np_lessthan				( int );
int		calcReflex				( CHAR_DATA*,CHAR_DATA*,int );
void	check_deathblow			( CHAR_DATA*,CHAR_DATA* );
bool	check_mithril_bash		( CHAR_DATA*,CHAR_DATA* );
bool	check_alertness			( CHAR_DATA* );
bool	check_awareness			( CHAR_DATA*,CHAR_DATA*,int );
bool	check_awareness			( CHAR_DATA*,OBJ_DATA* );
bool	check_fluidmotion		( CHAR_DATA* );
int		check_weapons_knowledge	( CHAR_DATA*,int,bool );
bool	check_duck				( CHAR_DATA*,CHAR_DATA*,OBJ_DATA*,bool );
int		check_piety				( CHAR_DATA*,int,int );
bool	IS_WEAPON_STAT			( OBJ_DATA*,int );
void	SET_WFLAG				( OBJ_DATA*,int );
void	REMOVE_WFLAG			( OBJ_DATA*,int );
void	strip_moveskill			( CHAR_DATA* );
void	break_spell				( CHAR_DATA* );
#undef	CD
#undef	MID
#undef	OD
#undef	OID
#undef	RID
#undef	SF
#undef AD

bool	is_same_guild		( CHAR_DATA*,CHAR_DATA* );

int	get_hometown ( char* );
bool is_excluded(CHAR_DATA *ch,OBJ_DATA *obj);
#endif

