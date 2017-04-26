#ifndef _MERC_H_
#define _MERC_H_
//Welcome to merc.h
/*
 * Accommodate old non-Ansi compilers.
 */
#include <string>
#include <cstring>
using namespace std;

#if defined(TRADITIONAL)
#define DECLARE_OBJ_FUN( fun )		void fun( )
#define DECLARE_ROOM_FUN( fun )		void fun( )
#define const
#define args( list )			( )
#define DECLARE_DO_FUN( fun )		void fun( )
#define DECLARE_SPEC_FUN( fun )		bool fun( )
#define DECLARE_SPELL_FUN( fun )	void fun( )
#else
#define args( list )			list
#define DECLARE_DO_FUN( fun )		DO_FUN    fun
#define DECLARE_SPEC_FUN( fun )		SPEC_FUN  fun
#define DECLARE_SPELL_FUN( fun )	SPELL_FUN fun
#define DECLARE_OBJ_FUN( fun )		OBJ_FUN	  fun
#define DECLARE_ROOM_FUN( fun )		ROOM_FUN  fun
#endif
/* system calls */
//int unlink();
//int system();
//int unlink(char *);
/*Short scalar types.Diavolo reports AIX compiler has bugs with short types.*/
#if	!defined(FALSE)
#define FALSE 0
#endif

#if	!defined(true)
#define true 1
#endif

#if	defined(_AIX)
	#if	!defined(const)
		#define const
	#endif
	typedef int	sh_int;
	#define unix
	#else
		typedef int sh_int;
#endif

typedef long long l_int;

#define			MAX_MEM_LIST	11
extern const int rgSizeList [MAX_MEM_LIST];

//Make fread_number report stuff
#define fread_number(fp) fread_number_line((fp), __LINE__, __FILE__ )

#define MIL MAX_INPUT_LENGTH
#define UPPER_MAX_CLAN	50
#define LIQ_WATER		0
#define OBJ_VNUM_DUMMY	13
#define MAX_DIR			6
#define NO_FLAG			-99
#define TYPE_UNDEFINED	-1
#define TYPE_HIT		1000
#define MAX_FLAGS		32
#define MAX_TRADE		32

#include "enum.h"

typedef struct	affect_data			AFFECT_DATA;
typedef struct	area_data			AREA_DATA;
typedef struct	ban_data			BAN_DATA;
typedef struct 	buf_type	 		BUFFER;
typedef struct	char_data			CHAR_DATA;
typedef struct	quest_data			QUEST_DATA;
typedef struct	spell_crap			SPELL_CRAP;
typedef struct	descriptor_data		DESCRIPTOR_DATA;
typedef struct	exit_data			EXIT_DATA;
typedef struct	extra_descr_data	EXTRA_DESCR_DATA;
typedef struct	help_data			HELP_DATA;
typedef struct	help_area_data		HELP_AREA;
typedef struct	kill_data			KILL_DATA;
typedef struct	mem_data			MEM_DATA;
typedef struct	mob_index_data		MOB_INDEX_DATA;
typedef struct	note_data			NOTE_DATA;
typedef struct	obj_data			OBJ_DATA;
typedef struct	obj_index_data		OBJ_INDEX_DATA;
typedef struct	pc_data				PC_DATA;
typedef struct	sleep_data			SLEEP_DATA;
typedef struct	reset_data			RESET_DATA;
typedef struct	room_index_data		ROOM_INDEX_DATA;
typedef struct	shop_data			SHOP_DATA;
typedef struct	time_info_data		TIME_INFO_DATA;
typedef struct	mud_type			MUD_TYPE;
typedef struct	weather_data		WEATHER_DATA;
typedef struct  prog_list			PROG_LIST;
typedef struct  prog_code			PROG_CODE;
 
typedef	void DO_FUN		( CHAR_DATA*,char* );
typedef bool SPEC_FUN	( CHAR_DATA* );
typedef void SPELL_FUN	( int,int,CHAR_DATA*,void*,int,int );
typedef void OBJ_FUN	( OBJ_DATA*,char* );
typedef void ROOM_FUN	( ROOM_INDEX_DATA*,char* );

// String Pad
char *  string_pad      args( ( char *string, int nWidth, bool Right_Align) );

// Add comma
char *add_comma(int number);

// String and memory management parameters.
#define	MAX_KEY_HASH		1024
#define MSL					4608
#define MAX_INPUT_LENGTH	256
#define PAGELEN				22

#define MAX_USECLASS		MAX_CLASS
#define MAX_SKILL			590
#define MAX_GROUP			69
#define MAX_IN_GROUP		4
#define MAX_ALIAS			10
#define MAX_PC_RACE			16
#define MAX_SPIRITS			23
#define MAX_RANKS			10

extern short MAX_CLAN;
#define MAX_DAMAGE_MESSAGE	41
#define MAX_LEVEL			120
#define ADMIN   			MAX_LEVEL        //120
#define	DADMIN				(MAX_LEVEL -  4) //116
#define	HEADBUILDER			(MAX_LEVEL -  6) //114
#define HEADIMM				(MAX_LEVEL -  8) //112
#define BUILDER				(MAX_LEVEL - 10) //110
#define JBUILDER			(MAX_LEVEL - 12) //108
#define LEVEL_IMMORTAL		(MAX_LEVEL - 14) //106
#define IMMORTAL    		(MAX_LEVEL - 14) //106
#define DEMI				(MAX_LEVEL - 16) //104
#define KING				(MAX_LEVEL - 18) //102
#define HLEADER				(MAX_LEVEL - 19) //101
#define LEVEL_HERO			(MAX_LEVEL - 20) //100
#define HERO				LEVEL_HERO       //100
#define NEWBIE				10               //10


#ifdef IMC
   #include "imc.h"
#endif
#define ILLEGAL_NAME_FILE	"illegalnames.txt"
#define RESTRING_FILE		"../data/restrings.txt"
#define MAX_GOD				40

#define STAT_MAX			27

#define PULSE_PER_SECOND	4
#define PULSE_VIOLENCE		1
#define PULSE_MOBILE		(  2 * PULSE_PER_SECOND)
#define PULSE_MUSIC			(  6 * PULSE_PER_SECOND)
#define PULSE_TICK			( 60 * PULSE_PER_SECOND)
#define PULSE_AREA			(120 * PULSE_PER_SECOND)


#define ML 	ADMIN
#define DA	DADMIN
#define L1	HEADBUILDER
#define L2	HEADIMM
#define L3	BUILDER
#define L4	JBUILDER
#define L5 	IMMORTAL
#define L6	DEMI
#define L7	KING
#define L8	HLEADER
#define HE	HERO
#define NW	NEWBIE


#define CLEAR		"\x1B[1;37m"
#define C_RED		"\x1B[0;31m"
#define C_GREEN		"\x1B[0;32m"
#define C_YELLOW	"\x1B[0;33m"
#define C_BLUE		"\x1B[0;34m"
#define C_MAGENTA	"\x1B[0;35m"
#define C_CYAN		"\x1B[0;36m"
#define C_WHITE		"\x1B[0;37m"
#define C_D_GREY	"\x1B[1;30m"
#define CB_D_GREY 	"\x1B[1;30m"
#define C_B_RED		"\x1B[1;31m"
#define C_B_GREEN	"\x1B[1;32m"
#define C_B_YELLOW	"\x1B[1;33m"
#define C_B_BLUE	"\x1B[1;34m"
#define C_B_MAGENTA	"\x1B[1;35m"
#define C_B_CYAN	"\x1B[1;36m"
#define C_B_WHITE	"\x1B[1;37m"

#define BAN_SUFFIX		A
#define BAN_PREFIX		B
#define BAN_NEWBIES		C
#define BAN_ALL			D	
#define BAN_PERMIT		E
#define BAN_PERMANENT	F

#define MAX_NSPIRITS 13
#define MAX_NUNDEADS 16
#define MAX_NDEMONS 10

#define NOBLE_CANWHO 5

#define CLAN_JUNIOR 0
#define CLAN_SENIOR 6
#define CLAN_DEPUTY 8
#define CLAN_SECOND 9
#define CLAN_LEADER 10
#define CLAN_FILENAME "../data/clans.txt"
#define char_color(ch) true

extern const char *lookup_clan_status(int);
extern struct clan_type *get_clan_table(int);
extern int get_clan_index	(int);
extern int parse_clan_status(const char*);
extern bool is_independent	(CHAR_DATA*);
extern int strlen_color		(char*);
extern int strlen_colorsp	(char*,int);

/* RT ASCII conversions -- used so we can have letters in this file   this is a stupid, retard-ass ugly way to do things*/
#define A		  	(1 <<  0)
#define B			(1 <<  1)
#define C			(1 <<  2)
#define D			(1 <<  3)
#define E			(1 <<  4)
#define F			(1 <<  5)
#define G			(1 <<  6)
#define H			(1 <<  7)
#define I			(1 <<  8)
#define J			(1 <<  9)
#define K		    (1 << 10)
#define L		 	(1 << 11)
#define M			(1 << 12)
#define N		 	(1 << 13)
#define O			(1 << 14)
#define P			(1 << 15)
#define Q			(1 << 16)
#define R			(1 << 17)
#define S			(1 << 18)
#define T			(1 << 19)
#define U			(1 << 20)
#define V			(1 << 21)
#define W			(1 << 22)
#define X			(1 << 23)
#define Y			(1 << 24)
#define Z			(1 << 25)
#define aa			(1 << 26)
#define bb			(1 << 27)
#define cc			(1 << 28)
#define dd			(1 << 29)
#define ee			(1 << 30)
#define ff			(1 << 31)
#define gg			(1 << 32)
#define hh			(1 << 33)
#define ii			(1 << 34)
#define jj			(1 << 35)
#define kk			(1 << 36)
#define ll			(1 << 37)
#define mm			(1 << 38)
#define nn			(1 << 39)
#define oo			(1 << 40)
#define pp			(1 << 41)
#define qq			(1 << 42)
#define rr			(1 << 43)
#define ss			(1 << 44)
#define tt			(1 << 45)
#define uu			(1 << 46)
#define vv			(1 << 47)
#define ww			(1 << 48)
#define xx			(1 << 49)
#define yy			(1 << 50)
#define zz			(1 << 51)





 #define ITM_GLOW			(A)
 #define ITM_HUM			(B)
 #define ITM_DARK			(C)
 #define ITM_LOCK			(D)
 #define ITM_EVIL			(E)
 #define ITM_INVIS			(F)
 #define ITM_MAGIC			(G)
 #define ITM_NODROP			(H)
 #define ITM_GRIPPED		(I)
 #define ITM_BARBED			(J)
 #define ITM_FORGING		(K)
 #define ITM_ANTI_NEUTRAL	(L)
 #define ITM_NOREMOVE		(M)
 #define ITM_INVENTORY		(N)
 #define ITM_NOPURGE		(O)
 #define ITM_ROT_DEATH		(P)
 #define ITM_VIS_DEATH		(Q)
 #define ITM_LODGED			(R)
 #define ITM_EMBALM			(S)
 #define ITM_NOLOCATE		(T)
 #define ITM_MELT_DROP		(U)
 #define ITM_HAD_TIMER		(V)
 #define ITM_SELL_EXTRACT	(W)
 #define ITM_GRASPED		(X)
 #define ITM_BURN_PROOF		(Y)
 #define ITM_NOUNCURSE		(Z)
 #define ITM_HIDDEN			(aa)
 #define ITM_FLAMING		(bb)
 #define ITM_FROSTED		(cc)
 #define ITM_SHOCKED		(dd)
 #define ITM_SHOCK_PROOF	(ee)
 #define ITM_INLAY			(ff)

 #define EXL_FIGHTER		(A)
 #define EXL_MYSTIC			(B)
 #define EXL_SORCERER		(C)
 #define EXL_ARCHER			(D)
 #define EXL_ASSASSIN		(E)
 #define EXL_BANDIT			(F)
 #define EXL_BERSERKER		(G)
 #define EXL_BRAWLER		(H)
 #define EXL_CAVALIER		(I)
 #define EXL_DEFENDER		(J)
 #define EXL_KNIGHT			(K)
 #define EXL_MARTIALARTIST	(L)
 #define EXL_NOMAD			(M)
 #define EXL_RANGER			(N)
 #define EXL_ROGUE			(O)
 #define EXL_SWASHBUCKLER	(P)
 #define EXL_SWORDFIGHTER	(Q)
 #define EXL_THIEF			(R)
 #define EXL_WARRIOR		(S)
 #define EXL_ALCHEMIST		(T)
 #define EXL_BARD			(U)
 #define EXL_CLERIC			(V)
 #define EXL_DRUID			(W)
 #define EXL_NECROMANCER	(X)
 #define EXL_PRIEST			(Y)
 #define EXL_SHAMAN			(Z)
 #define EXL_SUMMONER		(aa)
 #define EXL_ILLUSIONIST	(bb)
 #define EXL_WIZARD			(cc)
 #define EXL_PYROMANCER		(dd)
 #define EXL_CRYOMANCER		(ee)
 #define EXL_GEOMANCER		(ff)
 #define EXL_MAGE			(gg)
 #define EXL_ANTIMAGE		(hh)
 #define EXL_CONJURER		(ii)
 #define EXL_ENCHANTER		(jj)
 #define EXL_AEROMANCER		(kk)
 #define EXL_HYDROMANCER	(ll)
 #define EXL_ELEMENTALIST	(mm)
 #define EXL_HUMAN			(nn)
 #define EXL_ALGID			(oo)
 #define EXL_DAURIN			(pp)
 #define EXL_QUOLIN			(qq)
 #define EXL_VITHE			(rr)
 #define EXL_GURHA			(ss)
 #define EXL_DEWDOP			(tt)
 #define EXL_DWARF			(uu)

#define OBJ_VNUM_SILVER_ONE				1
#define OBJ_VNUM_GOLD_ONE				2
#define OBJ_VNUM_GOLD_SOME				3
#define OBJ_VNUM_SILVER_SOME			4
#define OBJ_VNUM_COINS					5

#define OBJ_VNUM_CORPSE_NPC				10
#define OBJ_VNUM_CORPSE_PC				11
#define OBJ_VNUM_GUTS					12

#define OBJ_VNUM_WEAR					14
#define OBJ_VNUM_BLOOD					15

#define OBJ_VNUM_MUSHROOM				20
#define OBJ_VNUM_LIGHT_BALL				21
#define OBJ_VNUM_SPRING					22
#define OBJ_VNUM_DISC					23
#define OBJ_VNUM_DARK_BALL				24
#define OBJ_VNUM_PORTAL					25
#define OBJ_VNUM_SCHOOL_MACE			26
#define OBJ_VNUM_SCHOOL_DAGGER			27
#define OBJ_VNUM_SCHOOL_LONGSWORD		28
#define OBJ_VNUM_SCHOOL_SPEAR			29
#define OBJ_VNUM_SCHOOL_STAFF			30
#define OBJ_VNUM_SCHOOL_AXE				31
#define OBJ_VNUM_SCHOOL_FLAIL			32
#define OBJ_VNUM_SCHOOL_WHIP			33
#define OBJ_VNUM_SCHOOL_POLEARM			34
#define OBJ_VNUM_SCHOOL_VEST			35
#define OBJ_VNUM_SCHOOL_SHIELD			36
#define OBJ_VNUM_SCHOOL_BANNER			37
#define OBJ_VNUM_MAP					38
#define OBJ_VNUM_WHISTLE				39
#define OBJ_VNUM_SCHOOL_PANTS			40
#define OBJ_VNUM_ROSE					41
#define OBJ_VNUM_SCHOOL_CAP				44

#define OBJ_VNUM_ARROW					46
#define OBJ_VNUM_SCHOOL_RAPIER			48
#define OBJ_VNUM_NOODLE					49

#define OBJ_VNUM_SCHOOL_KATANA			51
#define OBJ_VNUM_SCHOOL_SAI				52
#define OBJ_VNUM_SCHOOL_TONFA			53
#define OBJ_VNUM_PIT					54
#define OBJ_VNUM_SCHOOL_GAUNTLET		55
#define OBJ_VNUM_SCHOOL_SCIMITAR		56
#define OBJ_VNUM_SCHOOL_MACHETE			57
#define OBJ_VNUM_SCHOOL_MAINGAUCHE		58
#define OBJ_VNUM_SCHOOL_DIRK			59
#define OBJ_VNUM_SCHOOL_NUNCHAKU		60
#define OBJ_VNUM_SCHOOL_LANCE			61
#define OBJ_VNUM_SCHOOL_BASTARD			63
#define OBJ_VNUM_BONESHIELD				64
#define OBJ_VNUM_ICESHIELD				65
#define OBJ_VNUM_LIGHTNINGSHIELD		66

#define OBJ_VNUM_SCHOOL_QUARTERSTAFF	68
#define OBJ_VNUM_SCHOOL_GLADIUS			69
#define OBJ_VNUM_SCHOOL_LONGBOW			70
#define OBJ_VNUM_SCHOOL_SHORTBOW		71
#define OBJ_VNUM_SCHOOL_BOOTS			72
#define OBJ_VNUM_SCHOOL_BELT			73
#define OBJ_VNUM_SCHOOL_HAMMER			74
#define OBJ_VNUM_FIRE					75
#define OBJ_VNUM_TINDER					76
#define OBJ_VNUM_HERBS					77
#define OBJ_VNUM_SCHOOL_SCYTHE			79
#define OBJ_VNUM_BONES					80
#define OBJ_VNUM_RIBS					82
#define OBJ_VNUM_SCHOOL_CLUB			89
#define OBJ_VNUM_SCHOOL_WARHAMMER		92
#define OBJ_VNUM_SCHOOL_BATTLEAXE		93
#define OBJ_VNUM_INGOT					106
#define OBJ_VNUM_SLAG					110
#define VNUM_SCHOOL						7650



#define TRAP_NULL (A)
#define TRAP_TARGET (B)
#define TRAP_LOG (C)
#define TRAP_EXPLODING (D)
#define TRAP_SNARE (E)
#define TRAP_WEB (F)

#define TRAPT_FLAMING (A)
#define TRAPT_FROST (B)
#define TRAPT_SHOCKING (C)
#define TRAPT_POISON (D)
#define TRAPT_POWERDRAIN (E)

#define ITEM_TAKE (A)
#define ITEM_WEAR_FINGER (B)
#define ITEM_WEAR_NECK (C)
#define ITEM_WEAR_TORSO (D)
#define ITEM_WEAR_HEAD (E)
#define ITEM_WEAR_LEGS (F)
#define ITEM_WEAR_FEET (G)
#define ITEM_WEAR_HANDS (H)
#define ITEM_WEAR_ARMS (I)
#define ITEM_WEAR_SHIELD (J)
#define ITEM_WEAR_ABOUT (K)
#define ITEM_WEAR_WAIST (L)
#define ITEM_WEAR_WRIST (M)
#define ITEM_WIELD (N)
#define ITEM_HOLD (O)
#define ITEM_NO_SAC (P)
#define ITEM_WEAR_FLOAT (Q)
#define ITEM_WEAR_SHOULDER (R)
#define ITEM_WEAR_QUIVER (S)
#define ITEM_WEAR_FACE (T)
#define ITEM_WEAR_TATTOO (U)
#define ITEM_WEAR_ANKLE (V)
#define ITEM_WEAR_EAR (W)
#define ITEM_WEAR_ELBOW (X)
#define ITEM_WEAR_SHIN (Y)
#define ITEM_WEAR_KNEE (Z)
#define ITEM_WEAR_PENDANT (aa)
#define ITEM_WEAR_FLOAT_LIGHT (bb)

#define WEAPON_FLAMING (A)
#define WEAPON_FROST (B)
#define WEAPON_VAMPIRIC (C)
#define WEAPON_SHARP (D)
#define WEAPON_VORPAL (E)
#define WEAPON_TWO_HANDS (F)
#define WEAPON_SHOCKING (G)
#define WEAPON_POISON (H)
#define WEAPON_POWERDRAIN (I)
#define WEAPON_READIED (J)
#define WEAPON_SERRATED (K)
#define WEAPON_HOLY (L)
#define WEAPON_EVIL (M)
#define WEAPON_PESTILENCE (N)

#define GATE_NORMAL_EXIT (A)
#define GATE_NOCURSE (B)
#define GATE_GOWITH (C)
#define GATE_BUGGY (D)
#define GATE_RANDOM (E)
#define GATE_NOENTER (F)

#define STAND_AT (A)
#define STAND_ON (B)
#define STAND_IN (C)
#define SIT_AT (D)
#define SIT_ON (E)
#define SIT_IN (F)
#define REST_AT (G)
#define REST_ON (H)
#define REST_IN (I)
#define SLEEP_AT (J)
#define SLEEP_ON (K)
#define SLEEP_IN (L)
#define PUT_AT (M)
#define PUT_ON (N)
#define PUT_IN (O)
#define PUT_INSIDE (P)
#define SIT_UNDER (Q)
#define REST_UNDER (R)
#define SLEEP_UNDER (S)

#define CONT_CLOSEABLE 1
#define CONT_PICKPROOF 2
#define CONT_CLOSED 4
#define CONT_LOCKED 8
#define CONT_PUT_ON 16

#define ROOM_VNUM_LIMBO 2
#define ROOM_VNUM_CHAT 3
#define ROOM_VNUM_TEMPLE 1
#define ROOM_VNUM_ALTAR 1
#define ROOM_VNUM_SCHOOL 1


#define ROOM_DARK (A)
#define ROOM_LIGHT (B)
#define ROOM_NO_MOB (C)
#define ROOM_INDOORS (D)
#define ROOM_ARENA_FOYER (E)
#define ROOM_ARENA (F)
#define ROOM_NOMAP (G)


#define ROOM_PRIVATE (J)
#define ROOM_SAFE (K)
#define ROOM_SOLITARY (L)
#define ROOM_PET_SHOP (M)
#define ROOM_NO_RECALL (N)
#define ROOM_IMP_ONLY (O)
#define ROOM_GODS_ONLY (P)
#define ROOM_HEROES_ONLY (Q)
#define ROOM_NEWBIES_ONLY (R)
#define ROOM_LAW (S)
#define ROOM_NOWHERE (T)
#define ROOM_MOUNT_SHOP (U)
#define ROOM_BANK (V)


#define EX_ISDOOR		(A)
#define EX_CLOSED		(B)
#define EX_LOCKED		(C)
#define EX_SUPERINVIS	(D)
#define EX_PICKPROOF	(E)
#define EX_NOPASS		(F)
#define EX_EASY			(G)
#define EX_HARD			(H)
#define EX_INFURIATING	(I)
#define EX_NOCLOSE		(J)
#define EX_NOLOCK		(K)
#define EX_CLIFFTOP		(L)
#define EX_HIDDEN		(M)
#define EX_SDOOR		(N)
#define EX_ONEWAY		(O)
#define EX_QUIET		(P)
#define EX_RANDOM		(Q)

#define GUILD_DELETED A
#define GUILD_CHANGED B
#define GUILD_INDEPENDENT C /* a "loner" guild */
#define GUILD_IMMORTAL E /* immortal only clan */

#define MEM_CUSTOMER A	
#define MEM_SELLER B
#define MEM_HOSTILE C
#define MEM_AFRAID D

#include "board.h"
             
#define TRIG_ACT (A)
#define TRIG_BRIBE (B)
#define TRIG_DEATH (C)
#define TRIG_ENTRY (D)
#define TRIG_FIGHT (E)
#define TRIG_GIVE (F)
#define TRIG_GREET (G)
#define TRIG_GRALL (H)
#define TRIG_KILL (I)
#define TRIG_HPCNT (J)
#define TRIG_RANDOM (K)
#define TRIG_SPEECH (L)
#define TRIG_EXIT (M)
#define TRIG_EXALL (N)
#define TRIG_DELAY (O)
#define TRIG_SURR (P)
#define TRIG_GET (Q)
#define TRIG_DROP (R)
#define TRIG_SIT (S)
#define TRIG_INTERP (T)
#define TRIG_PUT (U)

extern sh_int gsn_dispersion_field;
extern sh_int gsn_shunt;
extern sh_int gsn_eimbue;
extern sh_int gsn_detect_invis;
extern sh_int gsn_energy_crash;
extern sh_int gsn_reject_magic;
extern sh_int gsn_occult_wisdom;
extern sh_int gsn_will_force;
extern sh_int gsn_mortal_coil;
extern sh_int gsn_abjuring_intellect;
extern sh_int gsn_assassinate;
extern sh_int gsn_paranoia;
extern sh_int gsn_mithtril_bash;
extern sh_int gsn_ambush;
extern sh_int gsn_backfist;
extern sh_int gsn_backstab;
extern sh_int gsn_dodge;
extern sh_int gsn_caltrops;
extern sh_int gsn_mounteddodge;
extern sh_int gsn_feint;
extern sh_int gsn_feralslash;
extern sh_int gsn_sidestep;
extern sh_int gsn_rebound;
extern sh_int gsn_ricochet;
extern sh_int gsn_evade;
extern sh_int gsn_duck;
extern sh_int gsn_mark_of_the_beast;
extern sh_int gsn_primal_roar;
extern sh_int gsn_rabid;
extern sh_int gsn_envenom;
extern sh_int gsn_lore;
extern sh_int gsn_falconry;
extern sh_int gsn_graverob;
extern sh_int gsn_preparationattack;
extern sh_int gsn_ready;
extern sh_int gsn_hide;
extern sh_int gsn_penumbralveil;
extern sh_int gsn_peek;
extern sh_int gsn_pick_lock;
extern sh_int gsn_sneak;
extern sh_int gsn_sonic_impact;
extern sh_int gsn_fuse;
extern sh_int gsn_alertness;
extern sh_int gsn_awareness;
extern sh_int gsn_survival;
extern sh_int gsn_firstaid;
extern sh_int gsn_blend;
extern sh_int gsn_drunkfighting;
extern sh_int gsn_leadership;
extern sh_int gsn_overdrive;
extern sh_int gsn_outmaneuver;
extern sh_int gsn_desertcover;
extern sh_int gsn_steal;
extern sh_int gsn_swap;
extern sh_int gsn_vanish;
extern sh_int gsn_weapons_knowledge;
extern sh_int gsn_slip;
extern sh_int gsn_mithril_bash;
extern sh_int gsn_palm;
extern sh_int gsn_cutpurse;
extern sh_int gsn_dual_wield;
extern sh_int gsn_disarm;
extern sh_int gsn_swipe;
extern sh_int gsn_enhanced_damage;
extern sh_int gsn_embalm;
extern sh_int gsn_poison_ivy;
extern sh_int gsn_defender;
extern sh_int gsn_improved_accuracy;
extern sh_int gsn_heavy_hand;
extern sh_int gsn_martial_arts;
extern sh_int gsn_rage;
extern sh_int gsn_swordsmanship;
extern sh_int gsn_swordefficiency;
extern sh_int gsn_swordmastery;
extern sh_int gsn_deathblow;
extern sh_int gsn_lancemastery;
extern sh_int gsn_kick;
extern sh_int gsn_kindred_spirits;
extern sh_int gsn_kickaxe;
extern sh_int gsn_kickroundhouse;
extern sh_int gsn_kickhigh;
extern sh_int gsn_kicklow;
extern sh_int gsn_pierce;
extern sh_int gsn_cleave;
extern sh_int gsn_punch;
extern sh_int gsn_punchuppercut;
extern sh_int gsn_punchcross;
extern sh_int gsn_punchjab;
extern sh_int gsn_punchtigerfang;
extern sh_int gsn_unending_strikes;
extern sh_int gsn_fist_of_fury;
extern sh_int gsn_dragon_fist;
extern sh_int gsn_lion_combo;
extern sh_int gsn_fierce_palm;
extern sh_int gsn_twin_tiger_fang;
extern sh_int gsn_flurry;
extern sh_int gsn_strike;
extern sh_int gsn_crush;
extern sh_int gsn_mighty_swing;
extern sh_int gsn_smash;
extern sh_int gsn_heavy_smash;
extern sh_int gsn_pommel_strike;
extern sh_int gsn_shieldstrike;
extern sh_int gsn_shadowstrike;
extern sh_int gsn_sharpen;
extern sh_int gsn_paralyzingfear;
extern sh_int gsn_conceal;
extern sh_int gsn_culmination;
extern sh_int gsn_traps;
extern sh_int gsn_downstrike;
extern sh_int gsn_trample;
extern sh_int gsn_final_strike;
extern sh_int gsn_parting_shot;
extern sh_int gsn_facekick;
extern sh_int gsn_sap;
extern sh_int gsn_strangle;
extern sh_int gsn_bandage;
extern sh_int gsn_parry;
extern sh_int gsn_trompement;
extern sh_int gsn_absorb;
extern sh_int gsn_damage_reduction;
extern sh_int gsn_lead_stance;
extern sh_int gsn_spell_resistance;
extern sh_int gsn_elude;
extern sh_int gsn_heavy_armor;
extern sh_int gsn_medium_armor;
extern sh_int gsn_light_armor;
extern sh_int gsn_rescue;
extern sh_int gsn_guard;
extern sh_int gsn_fortify;
extern sh_int gsn_grip;
extern sh_int gsn_grasp;
extern sh_int gsn_barb;
extern sh_int gsn_second_attack;
extern sh_int gsn_third_attack;
extern sh_int gsn_counter;
extern sh_int gsn_counter_spin;
extern sh_int gsn_riposte;
extern sh_int gsn_coule;
extern sh_int gsn_critical;
extern sh_int gsn_cast_efficiency;
extern sh_int gsn_mental_focus;
extern sh_int gsn_magical_mastery;
extern sh_int gsn_elemental_mastery;
extern sh_int gsn_spell_proficiency;
extern sh_int gsn_spellcraft;
extern sh_int gsn_arcane_knowledge;
extern sh_int gsn_marksmanship;
extern sh_int gsn_blindness;
extern sh_int gsn_charm_person;
extern sh_int gsn_curse;
extern sh_int gsn_invis;
extern sh_int gsn_mass_invis;
extern sh_int gsn_plague;
extern sh_int gsn_malady;
extern sh_int gsn_rupture;
extern sh_int gsn_narcolepsy;
extern sh_int gsn_poison;
extern sh_int gsn_venom;
extern sh_int gsn_morph_serpent;
extern sh_int gsn_morph_canine;
extern sh_int gsn_morph_feline;
extern sh_int gsn_morph_ursa;
extern sh_int gsn_morph_avian;
extern sh_int gsn_morph_aquatic;
extern sh_int gsn_phase;
extern sh_int gsn_lurk;
extern sh_int gsn_slither;
extern sh_int gsn_maul;
extern sh_int gsn_trail;
extern sh_int gsn_pounce;
extern sh_int gsn_scratch;
extern sh_int gsn_clench;
extern sh_int gsn_craze;
extern sh_int gsn_aerial_strike;
extern sh_int gsn_ascend;
extern sh_int gsn_animal_magnetism;
extern sh_int gsn_earth_wisdom;
extern sh_int gsn_salve;
extern sh_int gsn_resist_blights;
extern sh_int gsn_tranquility;
extern sh_int gsn_ascend;
extern sh_int gsn_ravage;
extern sh_int gsn_catalepsy;
extern sh_int gsn_nightmare;
extern sh_int gsn_nausea;
extern sh_int gsn_sleep;
extern sh_int gsn_fly;
extern sh_int gsn_sanctuary;
extern sh_int gsn_prismshotarray;
extern sh_int gsn_spellshield;
extern sh_int gsn_trueshot;
extern sh_int gsn_strafe;
extern sh_int gsn_sharp_shooting;
extern sh_int gsn_headshot;
extern sh_int gsn_stalk;
extern sh_int gsn_doubleshot;
extern sh_int gsn_arrow_shower;
extern sh_int gsn_raptor;
extern sh_int gsn_stamina;
extern sh_int gsn_arrow_infusion;
extern sh_int gsn_instinct;

extern sh_int gsn_exotic;
extern sh_int gsn_clubs;
extern sh_int gsn_swords;
extern sh_int gsn_daggers;
extern sh_int gsn_polearms;
extern sh_int gsn_bludgeons;
extern sh_int gsn_cleavers;
extern sh_int gsn_lashes;
extern sh_int gsn_bows;
extern sh_int gsn_martial_arms;
extern sh_int gsn_staffs;
extern sh_int gsn_sabers;
extern sh_int gsn_shortswords;
extern sh_int gsn_lances;

extern sh_int gsn_shield_block;
extern sh_int gsn_dual_shield;
extern sh_int gsn_bash;
extern sh_int gsn_shieldbash;
extern sh_int gsn_shieldtoss;
extern sh_int gsn_shieldmastery;
extern sh_int gsn_shieldfighting;
extern sh_int gsn_shift;
extern sh_int gsn_divert;
extern sh_int gsn_eavesdrop;
extern sh_int gsn_devour;
extern sh_int gsn_devotion;
extern sh_int gsn_untangle;
extern sh_int gsn_throwingknife;
extern sh_int gsn_throwingdart;
extern sh_int gsn_throwingaxe;
extern sh_int gsn_piety;
extern sh_int gsn_dogma;
extern sh_int gsn_sacred_guardian;
extern sh_int gsn_faith;
extern sh_int gsn_reverence;
extern sh_int gsn_true_belief;
extern sh_int gsn_poverty;
extern sh_int gsn_florentine;
extern sh_int gsn_poisoncloud;
extern sh_int gsn_blindingdust;
extern sh_int gsn_endure;
extern sh_int gsn_durability;
extern sh_int gsn_bravery;
extern sh_int gsn_icebound;
extern sh_int gsn_disappear;
extern sh_int gsn_fervor;
extern sh_int gsn_resilience;
extern sh_int gsn_hydrobiology;
extern sh_int gsn_thalassic_aura;
extern sh_int gsn_frozen_blood;
extern sh_int gsn_uplifting_spirit;
extern sh_int gsn_amiability;
extern sh_int gsn_return;
extern sh_int gsn_regeneration;
extern sh_int gsn_introversion;
extern sh_int gsn_innerpeace;
extern sh_int gsn_fluidmotion;
extern sh_int gsn_berserk;
extern sh_int gsn_reckless_abandon;
extern sh_int gsn_heightened_senses;
extern sh_int gsn_catnap;
extern sh_int gsn_counterfeit;
extern sh_int gsn_distract;
extern sh_int gsn_stealth;
extern sh_int gsn_mark;
extern sh_int gsn_gouge;
extern sh_int gsn_needle;
extern sh_int gsn_nerve;
extern sh_int gsn_neural_agent;
extern sh_int gsn_dual_backstab;
extern sh_int gsn_engarde;
extern sh_int gsn_feedback;
extern sh_int gsn_dirt;
extern sh_int gsn_mounteddirt;
extern sh_int gsn_disabletraps;
extern sh_int gsn_detecttraps;
extern sh_int gsn_combatives;
extern sh_int gsn_trip;
extern sh_int gsn_holytouch;
extern sh_int gsn_zeal;
extern sh_int gsn_divinetouch;
extern sh_int gsn_divinity;
extern sh_int gsn_purity;
extern sh_int gsn_mustang_heart;
extern sh_int gsn_lastrites;
extern sh_int gsn_kneeshot;
extern sh_int gsn_snare;
extern sh_int gsn_fast_healing;
extern sh_int gsn_lance_fighting;
extern sh_int gsn_haggle;
extern sh_int gsn_meditation;
extern sh_int gsn_mirage;
extern sh_int gsn_scrolls;
extern sh_int gsn_wands;
extern sh_int gsn_recall;
extern sh_int gsn_riding;
extern sh_int gsn_horsemanship;
extern sh_int gsn_charge;
extern sh_int gsn_hunt;
extern sh_int gsn_quickescape;
extern sh_int gsn_smelt;
extern sh_int gsn_whirlwind;
extern sh_int gsn_dualwind;
extern sh_int gsn_improvise;
extern sh_int gsn_brawler;
extern sh_int gsn_blind_fighting;
extern sh_int gsn_combat_proficiency;
extern sh_int gsn_furor;
extern sh_int gsn_paired_attack;
extern sh_int gsn_double_blitz;
extern sh_int gsn_batter;
extern sh_int gsn_bulwark;
extern sh_int gsn_defensive_stance;
extern sh_int gsn_warrior_heart;
extern sh_int gsn_thrust;
extern sh_int gsn_cavalry;
extern sh_int gsn_leverage;
extern sh_int gsn_numbing_force;
extern sh_int gsn_withdraw;
extern sh_int gsn_concussive_blow;
extern sh_int gsn_sword_blitz;
extern sh_int gsn_cranial_strike;
extern sh_int gsn_slashing_volley;
extern sh_int gsn_rising_cleave;
extern sh_int gsn_divider;
extern sh_int gsn_lop;
extern sh_int gsn_fissure;
extern sh_int gsn_rift;
extern sh_int gsn_hack;
extern sh_int gsn_breach;
extern sh_int gsn_destrier;
extern sh_int gsn_spiraling_spear;
extern sh_int gsn_spur;
extern sh_int gsn_equestrian;
extern sh_int gsn_courser;
extern sh_int gsn_charger;
extern sh_int gsn_barding;
extern sh_int gsn_hobby;
extern sh_int gsn_errant;
extern sh_int gsn_hotblooded_horse;
extern sh_int gsn_coldblooded_horse;
extern sh_int gsn_daurin;
extern sh_int gsn_warcry;

extern const	struct	str_app_type	str_app			[STAT_MAX+1];
extern const	struct	int_app_type	int_app			[STAT_MAX+1];
extern const	struct	wis_app_type	wis_app			[STAT_MAX+1];
extern const	struct	dex_app_type	dex_app			[STAT_MAX+1];
extern const	struct	end_app_type	end_app			[STAT_MAX+1];
extern			struct	class_type		classes			[MAX_CLASS];;
extern			struct	guild_type		* guilds;
extern			struct	hometown_type	* hometowns;
extern const	struct	weapon_type		weapon_table	[];
extern const	struct  item_type		item_table		[];
extern const	struct	wiznet_type		wiznet_table	[];
extern const	struct	attack_type		attack_table	[];
extern const	struct  race_type		race_table		[];
extern			struct	pc_race_type	pc_race_table	[];
extern const	struct	spec_type		spec_table		[];
extern const	struct	liq_type		liq_table		[];
extern const	struct	int_app_type	weapon_percent	[];
extern			struct	skill_type		skill_table		[MAX_SKILL];
extern			struct  sgroup_type      group_table		[MAX_GROUP];
extern const	struct  weight_type        eye_table		[];
extern const	struct  weight_type       hair_table		[];
extern const	struct  weight_type		sub_hair_table	[];
extern const	struct  weight_type     height_table	[];
extern const	struct  weight_type     weight_table	[];
extern			struct	social_type *	social_table;
extern const	struct	god_type		god_table		[MAX_GOD];
extern const	struct	spirit_type		spirit_table	[MAX_SPIRITS];
extern const	struct	charmy_type		ndemon_table	[MAX_NDEMONS];
extern const	struct	charmy_type		nundead_table	[MAX_NUNDEADS];
extern const	struct	charmy_type		nspirit_table	[MAX_NSPIRITS];
extern const	struct	nobility_type	nobility_flags		[MAX_NOBILITY];
extern const	struct	clanstatus_type	clanstatus_flags	[];
extern const	struct	full_class_type		class_files	[MAX_CLASS+1];
//extern const	struct	full_class_type		guild_files	[MAX_GUILD]; //wtf is this? <- this is a remnant from the flatfile reader. It's never called... it just stores the filenames

//Global variables.
extern DESCRIPTOR_DATA	*	descriptor_list;
extern TIME_INFO_DATA		time_info;
extern MUD_TYPE				mud;
extern WEATHER_DATA			weather_info;
extern KILL_DATA			kill_table [];
extern NOTE_DATA		*	note_free;
extern HELP_DATA		*	help_first;
extern SHOP_DATA		*	shop_first;
extern CHAR_DATA		*	char_list;
extern QUEST_DATA		*	quest_list;
extern PROG_CODE		*	mprog_list;
extern PROG_CODE		*	rprog_list;
extern PROG_CODE		*	oprog_list;
extern OBJ_DATA			*	object_list;
extern OBJ_DATA			*	obj_free;
extern time_t				current_time;
extern sh_int				display;
extern FILE				*	fpReserve;
extern char					bug_buf [];
extern char					log_buf [];
extern bool					fLogAll;
extern bool					double_exp;
extern int					current_weather[MAX_AREATYPES];
extern bool					MOBtrigger;
extern int					global_exp;
/*
 * OS-dependent declarations.
 * These are all very standard library functions,
 *   but some systems have incomplete or non-ansi header files.
 */
#if	defined(sequent)
//char *		crypt	args( ( const char *key, const char *salt ) );
int			fclose	args( ( FILE *stream ) );
int	fprintf	args( ( FILE *stream, const char *format, ... ) );
int			fread	args( ( void *ptr, int size, int n, FILE *stream ) );
int			fseek	args( ( FILE *stream, long offset, int ptrname ) );
void		perror	args( ( const char *s ) );
int			ungetc	args( ( int c, FILE *stream ) );
#endif

#define PLAYER_DIR		"../player/"
#define LOCKER_DIR		"../locker/"
#define TEMP_FILE		"../player/romtmp"
#define NULL_FILE		"/dev/null"

#define AREA_LIST		"area.lst"
#define CLASS_DIR		"../data/class/"
#define GUILD_DIR		"../data/guild/"
#define BUG_FILE		"bugs.txt"
#define TYPO_FILE		"typos.txt"
#define SHUTDOWN_FILE	"shutdown.txt"
#define BAN_FILE		"ban.txt"
#define MUSIC_FILE		"music.txt"
#define TOTALCON		"../data/totalcon.txt"

#include "struct.h"
#include "tables.h"
#include "prototype.h"
#include "buffer.h"

extern char *	const	dir_name        [];
extern const	sh_int	rev_dir         [];
extern const	struct	spec_type	spec_table	[];

extern int	top_affect;
extern int	top_area;
extern int	top_ed;
extern int	top_exit;
extern int	top_help;
extern int	top_mob_index;
extern int	top_obj_index;
extern int	top_reset;
extern int	top_room;
extern int	top_shop;
extern int	top_vnum_mob;
extern int	top_vnum_obj;
extern int	top_vnum_room;
extern char	str_empty       [1];

extern AREA_DATA *			area_first;
extern AREA_DATA *			area_last;
extern SHOP_DATA *			shop_last;
extern MOB_INDEX_DATA *		mob_index_hash  [MAX_KEY_HASH];
extern OBJ_INDEX_DATA *		obj_index_hash  [MAX_KEY_HASH];
extern ROOM_INDEX_DATA *	room_index_hash [MAX_KEY_HASH];

extern const struct	hometown_type	hometown_table	[];
extern HELP_DATA *help_last;
#endif

