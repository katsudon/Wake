#ifndef _STRUCT_H_
#define _STRUCT_H_

struct flag_type{
    char *name;
    l_int bit;
    bool settable;
};

struct position_type{
    char *name;
    char *short_name;
};

struct sex_type{
    char *name;
	char *sword;
	char *mword;
	char *eword;
};

struct size_type{
    char *name;
};

struct bit_type{
	const	struct	flag_type *	table;
	char *				help;
};

struct part_type{
	int		bit;
	char *	keyword;
	char *	short_descr;
	char *	long_descr;
	char *	msg;
};
struct morph_type{
	char *race;
	char *name;
	char *short_descr;
	char *long_descr;
};

struct ban_data{
    BAN_DATA *	next;
    bool	valid;
    int	ban_flags;
    int	level;
    char *	name;
};

struct god_type{
     char *      name;
     int      align;
     int      opposite;
};

struct spirit_type{
     char *      name;
     char *      verb;
};

struct charmy_type{
     char *		name;
     char *		short_descr;
     char *		long_descr;
	 int		hpmult;
	 int		pclass;
	 char *		race;
	 int		min_level;
	 char *		description;
};

struct nobility_type{
	int  ref;
	char *  name[3];
};

struct buf_type{
    BUFFER *    next;
    bool        valid;
    int      state;  /* error state of the buffer */
    int      size;   /* size in k */
    char *      string; /* buffer's string */
};

struct mud_type{
	int max_con,t_con,d_con;
	int month,day,season;
	bool coder;
	char * chan_log[MAX_CHAN][25];
};

struct time_info_data{
    int		hour;
    int		ticks;
    int		day;
    int		month;
    int		year;
};

struct weather_data{
    int		change;
    int		sky;
    int		sunlight;
};

struct classwield_data{
    int cNum;
    char wName;
};

struct descriptor_data{
    DESCRIPTOR_DATA *	next;
    DESCRIPTOR_DATA *	snoop_by;
    CHAR_DATA *			character;
    CHAR_DATA *			original;
	bool				school;
    bool				valid;
	bool				ansi;
    bool				fcommand;
    char *				host;
    int				descriptor;
    int				connected;
    char				inbuf		[MSL];//increased for buffer crap
    char				incomm		[MAX_INPUT_LENGTH];
    char				inlast		[MAX_INPUT_LENGTH];
    char *				outbuf;
    char *				showstr_head;
    char *				showstr_point;
    void *				pEdit;
    char **				pString;
    int					repeat;
    int					outsize;
    int					outtop;
    int					editor;
};

struct str_app_type{
    int	tohit;
    int	todam;
    int	carry;
    int	wield;
};

struct int_app_type{
    int	learn;
};

struct wis_app_type{
    int thinger;
};

struct dex_app_type{
    int	defensive;
};

struct end_app_type{
    int	hitp;
    int	shock;
};

struct help_data{
    HELP_DATA *	next;
    HELP_DATA * next_area;
    int			level;
    char *		keyword;
    char *		text;
	char *		syntax;
	char *		further_reading;
	char *		title;
};

struct help_area_data{
	HELP_AREA *	next;
	HELP_DATA *	first;
	HELP_DATA *	last;
	AREA_DATA *	area;
	char *		filename;
	bool		changed;
};

struct shop_data{
    SHOP_DATA *	next;			/* Next shop in list		*/
    int	keeper;			/* Vnum of shop keeper mob	*/
    int	buy_type [MAX_TRADE];	/* Item types shop will buy	*/
    int	profit_buy;		/* Cost multiplier for buying	*/
    int	profit_sell;		/* Cost multiplier for selling	*/
    int	open_hour;		/* First opening hour		*/
    int	close_hour;		/* First closing hour		*/
};

struct rank_type{
	char *name;
	bool recruit;
	bool expel;
	bool promote;
	bool demote;
};

struct guild_type{
	char * name;
	char * who_name;
	char * keywords;
	bool active;
	int index;
	int recall;
	int respawn;
	int area;
	int type;
	bool hidden;
	int rank_last;
	int races_max;
	int races[MAX_RACE];
	rank_type rank[MAX_RANKS];
};

struct class_type{
	char *	name;				// the full name of the class
	char 	who_name [4];		// Three-letter name for 'who'
	int	attr_prime;				// Prime attribute
	int	skill_adept;			// Maximum skill level
	int	thac0_00;				// Thac0 for level  0
	int	thac0_32;				// Thac0 for level 32
	int	hp_min;					// Min hp gained on leveling
	int	hp_max;					// Max hp gained on leveling
	int	mp_min;					// Min mp gained on leveling
	int	mp_max;					// Max mp gained on leveling
	int	mv_min;					// Min mv gained on leveling
	int	mv_max;					// Max mv gained on leveling
	int	fMana;					// Class gains mana on level
	int	ctier;					// Class gains mana on level
	int	align;					// The base align rating for the class
	int previous;
	bool	amp;
	char *	msg_self;			// cast message for the caster
	char *	msg_other;			// cast message for others
	bool	active;				// If the class is accessible
	bool	becomes[MAX_CLASS];
	int	attribute[MAX_STATS];	// STAT attributes
		int	armor;					// Class gains mana on level
};

struct item_type{
	int		type;
	char *	name;
};

struct weapon_type{
    char *	name;
    int	vnum;
    int	type;
	int	size;
    int	*agsn;
    int	gsn;
	bool	candual;
	int	dice;
	int	dsize;
};

struct wiznet_type{
	char *	name;
	int 	flag;
	int		level;
};

struct attack_type{
	char *	name;			/* name */
	char *	noun;			/* message */
	int   	damage;			/* damage class */
};

struct weight_type{
	char * name;
	l_int flag;
	int races[MAX_RACE];
};

struct hair_type{
	char * name;
	l_int flag;
	int clas[MAX_PC_RACE];
};

struct eye_type{
	char * name;
	long flag;
	int clas[MAX_PC_RACE];
};

struct height_type{
	char * name;
	long flag;
	int clas;
};

struct race_type{
	char *	name;			/* call name of the race */
	bool	pc_race;		/* can be chosen by pcs */
	char *	aff;			/* aff bits for the race */
	char *	act;			/* act bits for the race */
	char *	off;			/* off bits for the race */
	char *	res[MAX_RES];	/* res bits for the race */
	char *	form;			/* default form flag for the race */
	char *	part;			/* default parts for the race */
};

struct pc_race_type  /* additional data for pc races */{
    char *	name;			/* MUST be in race_type */
    char 	who_name[9];
    int	points;			/* cost in points of the race */
    int	class_mult[MAX_CLASS];	/* exp multiplier for class, * 100 */
    char *	skills[5];		/* bonus skills for the race */
    char *	sskills[5];		/* secondary bonus skills for the race */
    int 	stats[MAX_STATS];	/* starting stats */
    int	max_stats[MAX_STATS];	/* maximum stats */
    int	class_use[MAX_CLASS];
	int	home_use[50];//nash fix
    int	size;			/* aff bits for the race */
};

struct spec_type{
    char * 	name;			/* special function name */
    SPEC_FUN *	function;		/* the function */
};

struct note_data{
    NOTE_DATA *	next;
    bool 	valid;
    int	type;
    char *	sender;
    char *	date;
    char *	to_list;
    char *	subject;
    char *	text;
    time_t  	date_stamp;
	time_t	expire;
};

struct affect_data{
	CHAR_DATA	*	parent;
    AFFECT_DATA *	next;
    bool			valid;
	int			id;
    int			where;
    int			type;
    int			level;
    int			duration;
    int			location;
    int			modifier;
	int			slvl;
    l_int		bitvector;
};

struct kill_data{
    int		number;
    int		killed;
};

struct mob_stuff{
    char		*	short_descr;
    char		*	long_descr;
    char		*	description;
	bool			lefty;
    int				pclass;
	int			pcounter;
	int			dcounter;
	bool			act[100];
	bool			affected_by[MAX_AFF];
	bool			off_bits[MAX_OFF];
	bool			def_bits[MAX_DEF];
	int				res[MAX_RES];
	int				dummyres[MAX_RES];
	bool			form[MAX_FRM];
	bool			parts[MAX_PRT];
	bool			wiz[MAX_WIZ];
	bool			comm[MAX_CMM];
    int			alignment;
    int			level;
    int			hitroll;
    int			sex;
    int			race;
    int			size;
    int			perm_stat[MAX_STATS];
	bool			remaff(int);
	bool			remact(int);
	bool			remplr(int);
	bool			remcomm(int);
	bool			remoff(int);
	bool			remdef(int);
	bool			rempart(int);
	bool			remform(int);
	bool			remwiz(int);

	bool			setaff(int);
	bool			setact(int);
	bool			setplr(int);
	bool			setcomm(int);
	bool			setoff(int);
	bool			setdef(int);
	bool			setpart(int);
	bool			setform(int);
	bool			setwiz(int);

	bool			isaff(int);
	bool			isform(int);
	bool			ispart(int);
	bool			isact(int);
	bool			isplr(int);
	bool			isoff(int);
	bool			isdef(int);
	bool			iscomm(int);
	bool			iswiz(int);

	bool			massaff(char*);
	bool			massact(char*);
	bool			massplr(char*);
	bool			masscomm(char*);
	bool			massoff(char*);
	bool			massdef(char*);
	bool			masspart(char*);
	bool			massform(char*);
	bool			masswiz(char*);

	bool			hasaffs(char*);
	bool			hasacts(char*);
	bool			hasplrs(char*);
	bool			hascomms(char*);
	bool			hasoffs(char*);
	bool			hasdefs(char*);
	bool			hasparts(char*);
	bool			hasforms(char*);
	bool			haswizs(char*);

	bool			toggleaffs(char*);
	bool			toggleacts(char*);
	bool			toggleplrs(char*);
	bool			togglecomms(char*);
	bool			toggleoffs(char*);
	bool			toggledefs(char*);
	bool			toggleparts(char*);
	bool			toggleforms(char*);
	bool			togglewizs(char*);

	void			nerf_res(int,int);
	void			buff_res(int,int);
	void			reset_res(int);
};

struct mob_index_data : public mob_stuff{
    MOB_INDEX_DATA *	next;
    SPEC_FUN *		spec_fun;
    SHOP_DATA *		pShop;
	PROG_LIST *		mprogs;
    AREA_DATA *		area;		/* OLC */
    int			vnum;
    int			group;
    bool			new_format;
    int			count;
    int			killed;
    char *			player_name;
    int			hit[3];
    int			mana[3];
    int			damage[3];
    int			ac[4];
    int 			dam_type;
    int			start_pos;
    int			default_pos;
    int				wealth;
	int guild;
    char *			material;
	int					trainer;
	char *			icpmsg,*ocpmsg,*cmsg;
	l_int			mprog_flags;
};

struct mem_data{
    MEM_DATA 	*next;
    bool	valid;
    int		id;
    int 	reaction;
    time_t 	when;
	int dam;
};

struct spell_crap{
	int				chargetime;
	CHAR_DATA		*	chargevict;
    int				spelltimer;
    int				spellslvl;
	int					spellfailed;
	int				spellsn;
	void			*	spellvo;
	CHAR_DATA		*	spellvictim;
	int					spelltarget;
	int					spellcost;
	//code a "set" method here
};

struct char_data : public mob_stuff, public spell_crap{
public:
    CHAR_DATA			*next,
						*next_in_room,

						*master,
						*leader,

						*challenge,
						*fighting,

						*reply,
						*ltell,

						*pet,
						*mount,

						*mprog_target,

						*guardby,
						*guarding,
						*revenging,
						*mark,

						*next_charmy,
						*prev_charmy,
						*charmy_first,

						*group_request;

    AREA_DATA		*	zone;
    OBJ_DATA		*	carrying,*on;
    MEM_DATA		*	memory;
    SPEC_FUN		*	spec_fun;
    PC_DATA			*	pcdata;
	AFFECT_DATA		*	affected;
    MOB_INDEX_DATA	*	pIndexData;
    ROOM_INDEX_DATA *	in_room,*was_in_room;
    DESCRIPTOR_DATA	*	desc;
    int				mana,antimana,max_mana,max_antimana,hit,max_hit,move,max_move;
	int				guild;
	int				true_race,morph;
    char		*	prompt;
    char		*	prefix;
    char		*	name;
    bool			valid;
    long			id,mid;//mid is memory id and is basically the char's index in the char_list, id is the db id
	int				imprint;
	int				trainer;
	char *			icpmsg,*ocpmsg,*cmsg;
	int				death_timer,rottimer;
    int				version;
    int				foyervnum;
    int				group;
	int				battleprompt;
    int				trust;
    int				played;
    int				lines;
    time_t			logon;
    int				timer;
    int				wait;
    int				daze;
	int				credits;
	int				kills[KILL_MAX];
	char			combo[5];
	int				looks[P_MAX];
	int				meteorhits;
    int				gold,bankgold,silver,banksilver;
    int				exp,texp;
    int				invis_level,incog_level;
    int				position;
    int				carry_weight,carry_number;
    int				saving_throw,saving_spell_throw;
    int				damroll;
    int				armor[4];
	int				worn_armor[5];
    int				wimpy;
    int				mod_stat[MAX_STATS];
    int				max_stat[MAX_STATS];
    char		*	material;
    int				damage[3];
    int				dam_type;
    int				start_pos,default_pos;
    int				mprog_delay;
	bool			mounted,ismount;
	int				hometown,god,petition,rank,nobility;
	int					bashwait;

	int					spiritguide;

	int					shp,smp,smv;
	int					charmy_levels;
	int					charmy_count;

	void send(const char*,...);
	void printf(char*,...);
	bool israce(char*);
	int getmana();
	void setmana(int);
	void modmana(int);
	int getmaxmana();
	void setmaxmana(int);
	void modmaxmana(int);

	int getslvl(int);
	int gettruemana();
	void settruemana(int);
	void modtruemana(int);
	int gettruemaxmana();
	void settruemaxmana(int);
	void modtruemaxmana(int);

	int gettrueantimana();
	void settrueantimana(int);
	void modtrueantimana(int);
	int gettruemaxantimana();
	void settruemaxantimana(int);
	void modtruemaxantimana(int);

	void manadamage(int);
};

struct pc_data{
    PC_DATA *		next;
    BUFFER * 		buffer;
	OBJ_DATA *		forge;
    BOARD_DATA *	board;
    NOTE_DATA *		in_progress;
    time_t			last_note[MAX_BOARD];
	ROOM_INDEX_DATA*		map_point;
    bool			valid;
	bool		has_petdb,has_mountdb;
    char *			pwd;
    char *			bamfin;
    char *			bamfout;
    char *			title;
    char *			lname;
	char *			pretitle;
    int			perm_hit;
    int			perm_mana;
    int			perm_antimana;
    int			perm_move;
    int			true_sex;
    int				last_level;
    int			condition[4];
    int			learned[MAX_SKILL];
    int			skill_level[MAX_SKILL];
	bool			unlocked[MAX_SKILL];
    bool			group_known[MAX_GROUP];
    int			points;
    bool			confirm_delete;
    char *			alias[MAX_ALIAS];
    char * 			alias_sub[MAX_ALIAS];
    int 			security;
    int			weight,height,hair,eye;
	int			eavesdropping;
	int			statpoints,practices,s_practices,trains,studies,s_studies[4];
	int			timezone;
	char*			afk;
	char*			created,*lastlogin;
	bool			change_ready;
	int			temp_item_buf;

#ifdef IMC
    IMC_CHARDATA *imcchardata;
#endif
	void			up_skill(int,int,int);
	void			lock_skill(int);
};

/* Data for generating characters -- only used during generation */

struct liq_type{
    char *	liq_name;
    char *	liq_color;
    int	liq_affect[5];
};

struct extra_descr_data{
    EXTRA_DESCR_DATA *next;
	int id;
    bool valid;
    char *keyword;
    char *description;
};

struct obj_stuff{
	char *				name,*short_descr,*description,*material,*lore;
	AFFECT_DATA *		affected;
		l_int				extra_flags;
		int			armortype_flags;
		l_int				wear_flags;
		l_int				exclude_flags;
	int			id;
	int 				exclude;
	int					cont_count;
	int 				condition;
	int 				droprate;
	int				level;
	int				weight;
	int				item_type;
	int					cost;
	int				timer;
	int					value[5];
	bool				wflags[MAX_WPN];
};

struct obj_index_data : public obj_stuff{
    OBJ_INDEX_DATA *	next;
    EXTRA_DESCR_DATA *	extra_descr;
	PROG_LIST *			oprogs;
	long				oprog_flags;
    AREA_DATA *			area;
    bool				new_format;
    int				vnum;
    int				reset_num;
    int				count;
};

struct obj_data : public obj_stuff{
    OBJ_DATA *			next_content,*contains,*in_obj,*on,*next;
    CHAR_DATA *			carried_by;
    EXTRA_DESCR_DATA *	extra_descr;
    OBJ_INDEX_DATA *	pIndexData;
    ROOM_INDEX_DATA *	in_room;
	CHAR_DATA *			oprog_target;
	int				oprog_delay;
    bool				valid;
    bool				enchanted;
    char *				owner;
    int				wear_loc;
};

struct exit_data{
    union{
		ROOM_INDEX_DATA *	to_room;
		int			vnum;
    } u1;
    int		exit_info;
    int		key;
    char *		keyword;
    char *		description;
    EXIT_DATA *		next;
		int			rs_flags;
	int			orig_door;
};

/*
 * Reset commands:
 *   '*': comment
 *   'M': read a mobile 
 *   'O': read an object
 *   'P': put object in object
 *   'G': give object to mobile
 *   'E': equip object to mobile
 *   'D': set state of door
 *   'R': randomize room exits
 *   'S': stop (end of list)
 */
struct reset_data{
    RESET_DATA *	next;
    char		command;
    int		arg1;
    int		arg2;
    int		arg3;
    int		arg4;
};

struct area_data{
    AREA_DATA *	next;
	MOB_INDEX_DATA * mob_list;
	OBJ_INDEX_DATA * obj_list;
	ROOM_INDEX_DATA * room_list;
    HELP_AREA *	helps;
    char *		file_name;
    char *		name;
    char *		credits;
	int			finished;
    int		age;
    int		nplayer;
    int		low_range;
    int		high_range;
    int 		min_vnum;
    int		max_vnum;
    int		lowlevel;
    int		highlevel;
    int		area_group;
    bool		empty;
	int 		climate;
    char *		builders;
    int			vnum;
	int		arenaviewervnum;
	int			area_flags;
    int			security;
	bool		locked;
};

struct room_index_data{
	AFFECT_DATA			*affected;
    AREA_DATA			*area;
    CHAR_DATA			*people,*rprog_target;
    EXIT_DATA			*exit[6];
    EXTRA_DESCR_DATA	*extra_descr;
    OBJ_DATA			*contents;
    PROG_LIST			*rprogs;
    RESET_DATA			*reset_first,*reset_last;
    ROOM_INDEX_DATA		*next,*next_track,*track_came_from;
    long				rprog_flags;
	bool				affected_by[MAX_RAF];
    int				rprog_delay;
    char				*name;
    char				*description;
    char				*owner;
    int					vnum;
    int					room_flags;
    int				light;
    int				sector_type;
    int				heal_rate;
    int 				mana_rate;
    int				arenavn;
    int				arenaviewvn;
	bool				israf(int);
	bool				remraf(int);
	bool				setraf(int);
};

struct skill_lock{
	int sn;
	int level;
};
struct skill_crap{
     skill_lock     locks[10];
     skill_lock     unlocks[10];
     skill_lock     unlock_by[10];
};


struct skill_type{
	char *		name;

	int			max[MAX_CLASS];
	int		skill_level[MAX_CLASS];
	int			cost[MAX_CLASS];
	skill_crap        skl[MAX_CLASS];
	SPELL_FUN *	spell_fun;
	int		target;
	int		minimum_position;
	int *	pgsn;
	int		slot;
	int		min_mana;
	int		beats;
	char *		noun_damage;
	char *		msg_off;
	char *		msg_obj;
	char *		cst_prs;

};


struct prog_list{
    int                 trig_type;
    char *              trig_phrase;
    int              vnum;
    char *              code;
    PROG_LIST *         next;
    bool                valid;
};

struct prog_code{
    int              vnum;
    char *              code;
    PROG_CODE *         next;
};

struct social_type{
    char		name[20];
    char *		char_no_arg;
    char *		others_no_arg;
    char *		char_found;
    char *		others_found;
    char *		vict_found;
    char *		char_not_found;
    char *		char_auto;
    char *		others_auto;
    char *		char_obj;
    char *		others_obj;
};

struct hometown_type{
	char	*name;
	int		guild;
	int		recall;
	int		death;
	int		map;
	int		donor;
	bool	canuse;
	int		id;
};

struct lang_type{
	char	*name;
};

struct sect_type{
	char	*	name;
	char	*	color;
	int			bit;
	int		dirtkick;
	int		mvdeduction;
	int		manaheal;
	int		hpheal;
	int		mvheal;
	bool		isindoors;
	bool		settable;
};

struct excl_type{
	int		bit;
	int		wclass;
	char	*	race;
	int			subnum;
	bool		isclass;
	
};

struct weapon_shit{
	char	*	msgc[4];
	char	*	msgr[4];
};

struct full_class_type{
	char	*	name;
	char	*	filename;
};

struct material_type{
	char	*	name;
	bool		metal;
	bool		forge;
	bool		mill;
};

struct sgroup_type{
	char*		name;
	int			cost[MAX_CLASS];
	char*		skills[MAX_IN_GROUP];
};

struct sleep_data{
	SLEEP_DATA *next;
	SLEEP_DATA *prev;
	CHAR_DATA *ch;
	CHAR_DATA *mob;
	PROG_CODE *prog;
	int valid;
	int vnum;
	int line;
	int timer;
};

/*
 * Drunk struct
 */
struct struckdrunk{
	int	min_drunk_level;
	int	number_of_rep;
	char	*replacement[11];
};

struct player_type{
	char *name;
	DESCRIPTOR_DATA *descriptor;
	bool online;
};
struct group_type{
	CHAR_DATA *leader,*group_first;
};
struct lore_type{
	char *fame,*plan,*rp_prefs,*alts,*themesong,*quote;
	char *off_hours,*temperament,*vacation,*real_name,*doing;
	int age;
};
#endif
