#ifndef _OLC_H_
#define _OLC_H_
/***************************************************************************
 *  File: olc.h                                                            *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/
/*
 * This is a header file for all the OLC files.  Feel free to copy it into
 * merc.h if you wish.  Many of these routines may be handy elsewhere in
 * the code.  -Jason Dinkel
 */


/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n\r     Port a ROM 2.4 v1.8\n\r"
#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n\r" \
                "     Modified for use with ROM 2.3\n\r"        \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\n\r" \
                "     Modificado para uso en ROM 2.4b6\n\r"	\
                "     Por Ivan Toledo (itoledo@ctcreuna.cl)\n\r"
#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n\r     (Port a ROM 2.4 - Nov 2, 1996)\n\r     Version actual : 1.8 - Sep 8, 1998\n\r"
#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"



/*
 * New typedefs.
 */
typedef	bool OLC_FUN		( CHAR_DATA*,char* );
#define DECLARE_OLC_FUN( fun )	OLC_FUN    fun


/* Command procedures needed ROM OLC */
DECLARE_DO_FUN(    do_help    );
DECLARE_SPELL_FUN( spell_null );





/*
 * Connected states for editor.
 */
#define ED_NONE		0
#define ED_AREA		1
#define ED_ROOM		2
#define ED_OBJECT	3
#define ED_MOBILE	4
#define ED_MPCODE	5
#define ED_HELP		6
#define ED_OPCODE       8
#define ED_RPCODE       9
#define ED_RANK		10


/*
 * Interpreter Prototypes
 */
void aedit			( CHAR_DATA*,char* );
void redit			( CHAR_DATA*,char* );
void medit			( CHAR_DATA*,char* );
void oedit			( CHAR_DATA*,char* );
void mpedit			( CHAR_DATA*,char* );
void opedit			( CHAR_DATA*,char* );
void rpedit			( CHAR_DATA*,char* );
void hedit			( CHAR_DATA*,char* );
void kedit			( CHAR_DATA*,char* );
void fedit			( CHAR_DATA*,char* );
/* OLC Constants*/
#define MAX_MOB	1		/* Default maximum number for resetting mobs */



/* Structure for an OLC editor command. */
struct olc_cmd_type
{
    char * const	name;
    OLC_FUN *		olc_fun;
};



/* Structure for an OLC editor startup command. */
struct	editor_cmd_type
{
    char * const	name;
    DO_FUN *		do_fun;
};



/*
 * Utils.
 */
AREA_DATA *get_vnum_area	( int );
AREA_DATA *get_area_data	( int );
l_int flag_value				( const struct flag_type*,char* );
char *flag_string			( const struct flag_type*,int );
void add_reset				( ROOM_INDEX_DATA*,RESET_DATA*pReset,int );



/*
 * Interpreter Table Prototypes
 */
extern const struct olc_cmd_type	aedit_table[];
extern const struct olc_cmd_type	redit_table[];
extern const struct olc_cmd_type	oedit_table[];
extern const struct olc_cmd_type	medit_table[];
extern const struct olc_cmd_type	mpedit_table[];
extern const struct olc_cmd_type	hedit_table[];
extern const struct olc_cmd_type	opedit_table[];
extern const struct olc_cmd_type	rpedit_table[];
extern const struct olc_cmd_type	kedit_table[];
extern const struct olc_cmd_type	fedit_table[];
/*
 * Editor Commands.
 */
DECLARE_DO_FUN( do_aedit        );
DECLARE_DO_FUN( do_redit        );
DECLARE_DO_FUN( do_oedit        );
DECLARE_DO_FUN( do_medit        );
DECLARE_DO_FUN( do_mpedit		);
DECLARE_DO_FUN( do_hedit		);
DECLARE_DO_FUN( do_opedit       );
DECLARE_DO_FUN( do_rpedit       );
DECLARE_DO_FUN( do_kedit	);

/*
 * General Functions
 */
bool show_commands		( CHAR_DATA*,char* );
bool show_help			( CHAR_DATA*,char* );
bool edit_done			( CHAR_DATA* );
bool show_version		( CHAR_DATA*,char* );

/*
 * Area Editor Prototypes
 */
DECLARE_OLC_FUN( aedit_show		);
DECLARE_OLC_FUN( aedit_create		);
DECLARE_OLC_FUN( aedit_name		);
DECLARE_OLC_FUN( aedit_file		);
DECLARE_OLC_FUN( aedit_age		);
DECLARE_OLC_FUN( aedit_climate		);
DECLARE_OLC_FUN( aedit_reset		);
DECLARE_OLC_FUN( aedit_security		);
DECLARE_OLC_FUN( aedit_builder		);
DECLARE_OLC_FUN( aedit_vnum		);
DECLARE_OLC_FUN( aedit_lrange		);
DECLARE_OLC_FUN( aedit_finished		);
DECLARE_OLC_FUN( aedit_group		);
DECLARE_OLC_FUN( aedit_locked		);
DECLARE_OLC_FUN( aedit_lvnum		);
DECLARE_OLC_FUN( aedit_uvnum		);
DECLARE_OLC_FUN( aedit_credits		);


/*
 * Room Editor Prototypes
 */
DECLARE_OLC_FUN( redit_show			);
DECLARE_OLC_FUN( redit_create		);
DECLARE_OLC_FUN( redit_name			);
DECLARE_OLC_FUN( redit_desc			);
DECLARE_OLC_FUN( redit_ed			);
DECLARE_OLC_FUN( redit_format		);
DECLARE_OLC_FUN( redit_north		);
DECLARE_OLC_FUN( redit_south		);
DECLARE_OLC_FUN( redit_east			);
DECLARE_OLC_FUN( redit_west			);
DECLARE_OLC_FUN( redit_up			);
DECLARE_OLC_FUN( redit_down			);
DECLARE_OLC_FUN( redit_mreset		);
DECLARE_OLC_FUN( redit_oreset		);
DECLARE_OLC_FUN( redit_mlist		);
DECLARE_OLC_FUN( redit_rlist		);
DECLARE_OLC_FUN( redit_olist		);
DECLARE_OLC_FUN( redit_mshow		);
DECLARE_OLC_FUN( redit_oshow		);
DECLARE_OLC_FUN( redit_heal			);
DECLARE_OLC_FUN( redit_arenavn			);
DECLARE_OLC_FUN( redit_arenaviewvn			);
DECLARE_OLC_FUN( redit_mana			);
DECLARE_OLC_FUN( redit_owner		);
DECLARE_OLC_FUN( redit_room			);
DECLARE_OLC_FUN( redit_sector		);
DECLARE_OLC_FUN( redit_reset_list		);
DECLARE_OLC_FUN( redit_addrprog		);
DECLARE_OLC_FUN( redit_delrprog		);


/*
 * Object Editor Prototypes
 */
DECLARE_OLC_FUN( oedit_show		);
DECLARE_OLC_FUN( oedit_create		);
DECLARE_OLC_FUN( oedit_keywords		);
DECLARE_OLC_FUN( oedit_short		);
DECLARE_OLC_FUN( oedit_lore		);
DECLARE_OLC_FUN( oedit_long		);
DECLARE_OLC_FUN( oedit_addaffect	);
DECLARE_OLC_FUN( oedit_addapply		);
DECLARE_OLC_FUN( oedit_delaffect	);
DECLARE_OLC_FUN( oedit_value0		);
DECLARE_OLC_FUN( oedit_value1		);
DECLARE_OLC_FUN( oedit_value2		);
DECLARE_OLC_FUN( oedit_value3		);
DECLARE_OLC_FUN( oedit_value4		);
DECLARE_OLC_FUN( oedit_weight		);
DECLARE_OLC_FUN( oedit_cost		);
DECLARE_OLC_FUN( oedit_droprate		);
DECLARE_OLC_FUN( oedit_timer		);
DECLARE_OLC_FUN( oedit_ed		);
DECLARE_OLC_FUN( oedit_addoprog		);
DECLARE_OLC_FUN( oedit_deloprog		);

DECLARE_OLC_FUN( oedit_extra            );
DECLARE_OLC_FUN( oedit_exclude            );
DECLARE_OLC_FUN( oedit_armortype		);
DECLARE_OLC_FUN( oedit_weapon		);
DECLARE_OLC_FUN( oedit_wear             );
DECLARE_OLC_FUN( oedit_type             );
DECLARE_OLC_FUN( oedit_affect           );
DECLARE_OLC_FUN( oedit_material		);
DECLARE_OLC_FUN( oedit_level            );
DECLARE_OLC_FUN( oedit_condition        );
DECLARE_OLC_FUN( oedit_autoweapon );
DECLARE_OLC_FUN( oedit_autoarmor );
/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN( medit_show		);
DECLARE_OLC_FUN( medit_create		);
DECLARE_OLC_FUN( medit_keywords		);
DECLARE_OLC_FUN( medit_short		);
DECLARE_OLC_FUN( medit_long		);
DECLARE_OLC_FUN( medit_shop		);
DECLARE_OLC_FUN( medit_desc		);
DECLARE_OLC_FUN( medit_level		);
DECLARE_OLC_FUN( medit_guild		);
DECLARE_OLC_FUN( medit_class		);
DECLARE_OLC_FUN( medit_align		);
DECLARE_OLC_FUN( medit_spec		);

DECLARE_OLC_FUN( medit_sex		);
DECLARE_OLC_FUN( medit_act		);
DECLARE_OLC_FUN( medit_affect		);
DECLARE_OLC_FUN( medit_ac		);
DECLARE_OLC_FUN( medit_form		);
DECLARE_OLC_FUN( medit_part		);
DECLARE_OLC_FUN( medit_res		);
DECLARE_OLC_FUN( medit_material		);
DECLARE_OLC_FUN( medit_off		);
DECLARE_OLC_FUN( medit_def		);
DECLARE_OLC_FUN( medit_size		);
DECLARE_OLC_FUN( medit_hitdice		);
DECLARE_OLC_FUN( medit_manadice		);
DECLARE_OLC_FUN( medit_damdice		);
DECLARE_OLC_FUN( medit_race		);
DECLARE_OLC_FUN( medit_position		);
DECLARE_OLC_FUN( medit_stat		);
DECLARE_OLC_FUN( medit_gold		);
DECLARE_OLC_FUN( medit_hitroll		);
DECLARE_OLC_FUN( medit_autoeasy		);
DECLARE_OLC_FUN( medit_autohard		);
DECLARE_OLC_FUN( medit_autoset		);
DECLARE_OLC_FUN( medit_damtype		);
DECLARE_OLC_FUN( medit_group		);
DECLARE_OLC_FUN( medit_addmprog		);
DECLARE_OLC_FUN( medit_delmprog		);
DECLARE_OLC_FUN( medit_trainer		);
DECLARE_OLC_FUN( medit_icpmsg		);
DECLARE_OLC_FUN( medit_ocpmsg		);
DECLARE_OLC_FUN( medit_cmsg		);

/* Mobprog editor */
DECLARE_OLC_FUN( mpedit_create		);
DECLARE_OLC_FUN( mpedit_code		);
DECLARE_OLC_FUN( mpedit_show		);
DECLARE_OLC_FUN( mpedit_list		);

/* Editor de helps */
DECLARE_OLC_FUN( hedit_keyword		);
DECLARE_OLC_FUN( hedit_text		);
DECLARE_OLC_FUN( hedit_new		);
DECLARE_OLC_FUN( hedit_level		);
DECLARE_OLC_FUN( hedit_delete		);
DECLARE_OLC_FUN( hedit_show		);
DECLARE_OLC_FUN( hedit_list		);

/*
 * Macros
 */
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))

/* Return pointers to what is being edited. */
#define EDIT_MOB(Ch, Mob)	( Mob = (MOB_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_OBJ(Ch, Obj)	( Obj = (OBJ_INDEX_DATA *)Ch->desc->pEdit )
#define EDIT_ROOM(Ch, Room)	( Room = Ch->in_room )
#define EDIT_AREA(Ch, Area)	( Area = (AREA_DATA *)Ch->desc->pEdit )
#define EDIT_MPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )
#define EDIT_OPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )
#define EDIT_RPCODE(Ch, Code)   ( Code = (PROG_CODE*)Ch->desc->pEdit )

/*
 * Prototypes
 */
/* mem.c - memory prototypes. */
RESET_DATA	*	new_reset_data			( void );
void			free_reset_data			( RESET_DATA* );
AREA_DATA	*	new_area				( void );
void			free_area				( AREA_DATA* );
EXIT_DATA	*	new_exit				( void );
void				free_exit			( EXIT_DATA* );
EXTRA_DESCR_DATA *			new_extra_descr		( void );
void				free_extra_descr	( EXTRA_DESCR_DATA* );
ROOM_INDEX_DATA *	new_room_index		( void );
void				free_room_index		( ROOM_INDEX_DATA* );
AFFECT_DATA	*		new_affect			( void );
void				free_affect			( AFFECT_DATA* );
SHOP_DATA	*		new_shop			( void );
void				free_shop			( SHOP_DATA* );
OBJ_INDEX_DATA	*	new_obj_index		( void );
void				free_obj_index		( OBJ_INDEX_DATA* );
MOB_INDEX_DATA	*	new_mob_index		( void );
void				free_mob_index		( MOB_INDEX_DATA* );

void				show_liqlist		( CHAR_DATA* );
void				show_damlist		( CHAR_DATA* );
char *				prog_type_to_name	( int );
PROG_LIST      *	new_mprog			( void );
void				free_mprog			( PROG_LIST* );
PROG_CODE	*		new_mpcode			( void );
void				free_mpcode			( PROG_CODE* );
PROG_LIST	*		new_oprog			( void );
void				free_oprog			( PROG_LIST* );
PROG_LIST	*		new_rprog			( void );
void				free_rprog			( PROG_LIST* );
PROG_CODE	*		new_opcode			( void );
void				free_opcode			( PROG_CODE* );
PROG_CODE	*		new_rpcode			( void );
void				free_rpcode			( PROG_CODE* );


/* Objprog editor */
DECLARE_OLC_FUN( opedit_create		);
DECLARE_OLC_FUN( opedit_code		);
DECLARE_OLC_FUN( opedit_show		);
DECLARE_OLC_FUN( opedit_list		);

/* Roomprog editor */
DECLARE_OLC_FUN( rpedit_create		);
DECLARE_OLC_FUN( rpedit_code		);
DECLARE_OLC_FUN( rpedit_show		);
DECLARE_OLC_FUN( rpedit_list		);

DECLARE_OLC_FUN( redit_delete             );
DECLARE_OLC_FUN( oedit_delete             );
DECLARE_OLC_FUN( medit_delete             );
#endif

