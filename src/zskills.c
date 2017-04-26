#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "magic.h"

/*
 * The skill and spell table.
 * Slot numbers must never be changed as they appear in #OBJECTS sections. Davion likes that!
 */

//look at that!

struct	skill_type	skill_table	[MAX_SKILL]	={
	{ "reserved",{},{},{},{},
		0, TAR_IGNORE, POS_STANDING, NULL, 0, 0, 0,
		"", "", "", ""
	},

	{ "acid blast",{},{},{},{},
		spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 22, 14,
		"acid blast", "!AcidBlast!", "!AcidBlast!", "san bakuha"
	},

	{ "agues echo",{},{},{},{},
		spell_agues_echo, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"!AguesEcho!", "The beasty finally fades away.", "!AguesEcho!", "NULL"
	},

	{ "aid",{},{},{},{},
		spell_aid, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 50, 18,
		"aid", "!Aid!", "!Aid!", "NULL"
	},

	{ "alarm",{},{},{},{},
		spell_alarm, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"", "Your alarm wears off.", "!Alarm!", "NULL"
	},

	{ "amplify damage",{},{},{},{},
		spell_amplify_damage, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"amp damage", "You feel tougher.", "!AMPDAM!", "NULL"
	},

	{ "animal companion",{},{},{},{},
		spell_animal_companion, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"", "You feel less weary.", "!AnimalCompanion!", "NULL"
	},

	{ "animate corpse",{},{},{},{},
		spell_animate_corpse, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 200, 24,
		"!AnimateCorpse!", "You feel less weary.", "!RaiseUndead!", "NULL"
	},

	{ "arcane burst",{},{},{},{},
		spell_arcane_burst, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 50, 20,
		"",	"!ArcaneBurst!",	"", "NULL"
	},

	{ "arcane knowledge",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_arcane_knowledge, SKILL_NORMAL, 0, 0,
		"", "!ArcaneKnowledge!", "", ""
	},

	{ "armor",{},{},{},{},
		spell_armor, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"!Armor!", "You feel less armored.", "!Armor!", "kikou"
	},

	{ "armageddon",{},{},{},{},
		spell_armageddon, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 200, 20,
		"{darmageddon{x", "!ARGMAGEDDON!", "!armageddon!", "NULL"
	},

	{ "arms of gaia",{},{},{},{},
		spell_arms_of_gaia, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 200, 20,
		"", "!ArmsOfGaia!", "!ArmsOfGaia!", "NULL"
	},

	{ "asphyxiation",{},{},{},{},
		spell_asphyxiation, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"mental attack", "Your head clears and you drift off to a normal sleep.", "!asphyxiation!", "NULL"
	},

	{ "awaken blood",{},{},{},{},
		spell_awaken_blood, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"{rblood awakening{x", "Your blood finally calms.", "!AwakenBlood!", "NULL"
	},

	{ "bind",{},{},{},{},
		spell_bind, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 80, 20,
		"!bind!", "You can walk again.", "!Bind!", "NULL"
	},

	{ "black hole",{},{},{},{},
		spell_black_hole, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 160, 16,
		"{mblack hole{x", "Your body is freed.", "!BlackHole!", "NULL"
	},

	{ "black stake",{},{},{},{},
		spell_blackstake, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"{dblack stake{x", "!blackstake!", "", "NULL"
	},

	{ "bless",{},{},{},{},
		spell_bless, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"!Bless!", "You feel less righteous.", "$p's holy aura fades.", "kiyomeru"
	},

	{ "blindness",{},{},{},{},
		spell_blindness, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_blindness, SKILL_SPELL, 5, 12,
		"!Blindness!", "You can see again.", "!Blindness!", "moomoku"
	},

	{ "blizzard",{},{},{},{},
		spell_blizzard, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 60, 16,
		"{Cblizzard{x", "You feel less cold.", "!Blizzard!", "NULL"
	},

	{ "blood clot",{},{},{},{},
		spell_blood_clot, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 40, 16,
		"{rblood clot{x", "You feel the pressure in your veins release.", "!BloodClot!", "NULL"
	},

	{ "bloodhound",{},{},{},{},
		spell_bloodhound, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_rupture, SKILL_SPELL, 25, 16,
		"bloodhound", "You finally get the bleeding to stop.", "", "NULL"
	},

	{ "blur",{},{},{},{},
		spell_blur, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 24,
		"!Blur!", "Your vision clears.", "!Blur!", "NULL"
	},

	{ "bone armor",{},{},{},{},
		spell_bonearmor, TAR_OBJ_INV, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"!BoneArmor!", "!BoneArmor!", "!BoneArmor!", "NULL"
	},

	{ "bone shield",{},{},{},{},
		spell_boneshield, TAR_OBJ_INV, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"!BoneShield!", "!BoneShield!",	"!BoneShield!", "NULL"
	},

	{ "bonestorm",{},{},{},{},
		spell_bonestorm, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 40, 12,
		"magic bone", "!Bonestorm!", "!Bonestorm!", "NULL"
	},

	{ "bound recall",{},{},{},{},
		spell_bound_recall, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"!BoundRecall!", "!BoundRecall!", "!BoundRecall!", "NULL"
	},

	{ "bright light",{},{},{},{},
		spell_bright_light, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 20, 18,
		"!BrightLight!", "!BrightLight!",	"!BrightLight!", "NULL"
	},

	{ "brilliant flare",{},{},{},{},
		spell_brilliant_flare, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"", "!BrilliantFlare!", "", "NULL"
	},

	{ "burn",{},{},{},{},
		spell_burn,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"burn", "!Burn!", "!Burn!", "NULL"
	},

	{ "call demon",{},{},{},{},
		spell_calldemon, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 250, 24,
		"!CallDemon!", "!CallDemon!", "!CallDemon!", "NULL"
	},

	{ "call lightning",{},{},{},{},
		spell_call_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"lightning arc", "!CallLightning!", "!CallLightning!", "yobu denkou"
	},

	{ "calm",{},{},{},{},
		spell_calm, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"!Calm!", "You have lost your peace of mind.", "!Calm!", "shizuka na"
	},

	{ "cancellation",{},{},{},{},
		spell_cancellation, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"!Cancellation", "!Cancellation!", "!Cancellation!", "sakoju suru"
	},

	{ "catalepsy",{},{},{},{},
		spell_catalepsy, TAR_OBJ_CHAR_OFF, POS_FIGHTING, &gsn_catalepsy, SKILL_SPELL, 20, 12,
		"catalepsy", "You feel less jerky.", "The jerky on $p dries up.", "NULL"
	},

	{ "caustic strike",{},{},{},{},
		spell_caustic_strike, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"caustic strike", "!CausticStrike!", "", "NULL"
	},

	{ "celestial strike",{},{},{},{},
		spell_celestial_strike, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"{dc{xe{dl{xe{ds{xt{di{xa{dl s{xt{dr{xi{dk{xe", "!CelestialStrike!", "", "NULL"
	},

	{ "chain lightning",{},{},{},{},
		spell_chain_lightning, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 25, 12,
		"lightning", "!ChainLightning!", "!ChainLightning!", "kusari denkou"
	}, 

	{ "change sex",{},{},{},{},
		spell_change_sex, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"!ChangeSex", "Your body feels familiar again.", "!ChangeSex!", "kawari sei"
	},

	{ "charge shot",{},{},{},{},
		spell_charge_shot, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 4,
		"charged shot", "!ChargeShot!", "", "NULL"
	},

	{ "charm person",{},{},{},{},
		spell_charm_person, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_charm_person, SKILL_SPELL, 5, 12,
		"!CharmPerson!", "You feel more self-confident.", "$n seems more in control.", "mibae hito"
	},

	{ "chill touch",{},{},{},{},
		spell_chill_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"chilling touch", "You feel less cold.", "!ChillTouch!", "okan souhou"
	},

	{ "claw of shade",{},{},{},{},
		spell_claw_of_shade, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 17,
		"{dclaw of shade{x", "You feel less cold.", "", "NULL"
	},

	{ "cleanse",{},{},{},{},
		spell_cleanse, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 30, 12,
		"!CLEANSE!", "!Cleanse!", "!CLEANSE!", "NULL"
	},

	{ "color spray",{},{},{},{},
		spell_color_spray, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"color spray", "!ColorSpray!", "!ColorSpray!", "iro himatsu"
	},

	{ "consecrate",{},{},{},{},
		spell_consecrate, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 24,
		"smite", "!Consecrate!", "", "NULL"
	},

	{ "continual dark",{},{},{},{},
		spell_continual_dark, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 7, 12,
		"!ContinualDark!", "!ContinualDark!",	"!ContinualDark!", "NULL"
	},

	{ "continual light",{},{},{},{},
		spell_continual_light, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 7, 12,
		"!ContinualLight!", "!ContinualLight!",	"!ContinualLight!", "taema no nai akari"
	},

	{ "control weather",{},{},{},{},
		spell_control_weather, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 25, 12,
		"!ControlWeather!", "!ControlWeather!", "!ControlWeather!", "toosei seru tenki"
	},

	{ "create food",{},{},{},{},
		spell_create_food, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"!CreateFood!", "!CreateFood!", "!CreateFood!", "juritsu tabemono"
	},

	{ "create rose",{},{},{},{},
		spell_create_rose, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 30, 12,
		"!CreateRose!", "!CreateRose!", "!CreateRose!", "juritsu bara"
	},

	{ "create spring",{},{},{},{},
		spell_create_spring, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 20, 12,
		"!CreateSpring!", "!CreateSpring!", "!CreateSpring!", "juritsu izumi"
	},

	{ "create water",{},{},{},{},
		spell_create_water, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"!CreateWater!", "!CreateWater!", "!CreateWater!", "juritsu mizu"
	},

	{ "cure blindness",{},{},{},{},
		spell_cure_blindness, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 5, 12,
		"!CureBlindness!", "!CureBlindness!", "!CureBlindness!", "naoru moomoku"
	},

	{ "cure disease",{},{},{},{},
		spell_cure_disease, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 20, 12,
		"!CureDisease!", "!CureDisease!", "!CureDisease!", "naoru pesuto"
	},

	{ "cure poison",{},{},{},{},
		spell_cure_poison, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"!CurePoison!", "!CurePoison!", "!CurePoison!", "naoru doku"
	},

	{ "curse",{},{},{},{},
		spell_curse, TAR_OBJ_CHAR_OFF, POS_FIGHTING, &gsn_curse, SKILL_SPELL, 20, 12,
		"curse", "The curse wears off.", "$p is no longer impure.", "bachi"
	},

	{ "dark glare",{},{},{},{},
		spell_dark_glare, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 35, 12,
		"{ddark {rglare{x", "You feel more mobile.", "!DarkGlare!", "NULL"
	},

	{ "dark legion",{},{},{},{},
		spell_darklegion, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 50, 12,
		"!DarkLegion!", "You're no longer weary.", "!DarkLegion!", "NULL"
	},

	{ "death cloak",{},{},{},{},
		spell_death_cloak, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 75, 10,
		"death cloak", "!DeathCloak!", "", "NULL"
	},

	{ "dampening ward",{},{},{},{},
		spell_dampening_ward, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 18,
		"dampening ward", "!DampeningWard!", "", "NULL"
	},

	{ "decay",{},{},{},{},
		spell_decay, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 24,
		"{ddecay{x", "!Decay!", "", "NULL"
	},

	{ "defensive shield",{},{},{},{},
		spell_defensive_shield, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"", "You feel less armored.", "!DShield!", "NULL"
	},

	{ "defile",{},{},{},{},
		spell_defile, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 24,
		"", "!Defile!", "", "NULL"
	},

	{ "degeneration",{},{},{},{},
		spell_degeneration, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 24,
		"!Degeneration!", "Your body slowly recovers..", "!Degeneration!", "NULL"
	},

	{ "demon scythe",{},{},{},{},
		spell_demonscythe, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"!DemonScythe!", "!DemonScythe!", "!DemonScythe!", "NULL"
	},

	{ "demonfire",{},{},{},{},
		spell_demonfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"torments", "!Demonfire!", "!Demonfire!", "akuma hi"
	},

	{ "detect hidden",{},{},{},{},
		spell_detect_hidden, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"",	"You feel less aware of your surroundings.", "", "tantei hitoshirezu"
	},

	{ "detect invis",{},{},{},{},
		spell_detect_invis, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"",	"You no longer see invisible objects.",	"", "tantei me ni tsukanai"
	},

	{ "detect magic",{},{},{},{},
		spell_detect_magic, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"",	"The detect magic wears off.",	"", "tantei jujustu"
	},

	{ "detect poison",{},{},{},{},
		spell_detect_poison, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"",	"!Detect Poison!",	"", "tantei doku"
	},

	{ "disable talisman",{},{},{},{},
		spell_disable_talisman, TAR_CHAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 35, 12,
		"", "You feel more mobile.", "!DisableTalisman!", "NULL"
	},

	{ "dispel magic",{},{},{},{},
		spell_dispel_magic, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"",	"!Dispel Magic!", "", "dokeru jujustu"
	},

	{ "divine arrow",{},{},{},{},
		spell_divine_arrow, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"{Cdivine arrow{x", "!DivineArrow!", "", "NULL"
	},

	{ "divine strike",{},{},{},{},
		spell_divine_strike, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"{Cdivine strike{x", "!DivineStrike!", "!DivineStrike!", "NULL"
	},

	{ "double strike",{},{},{},{},
		spell_double_strike, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 30, 12,
		"double strike", "You feel the doubleness of your weapon go byebye.", "", "nibai ni utsu koto"
	},

	{ "drown",{},{},{},{},
		spell_drown, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"drown", "!Drown!", "", "NULL"
	},

	{ "dull spell",{},{},{},{},
		spell_dull_spell, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 35, 12,
		"", "You feel more mobile.", "!DullSpell!", "NULL"
	},

	{ "earthquake",{},{},{},{},
		spell_earthquake, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"earthquake", "!Earthquake!", "", "jishin"
	},

	{ "eclipse",{},{},{},{},
		spell_eclipse, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"", "!Eclipse!", "", "NULL"
	},

	{ "enchant object",{},{},{},{},
		spell_enchant_object, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"",	"!EnchantObject!", "", "NULL"
	},

	{ "encumber",{},{},{},{},
		spell_encumber, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 70, 20,
		"", "You feel gravity like you again.", "", "NULL"
	},

	{ "energy bomb",{},{},{},{},
		spell_energy_bomb, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 10,
		"blast of energy", "!EnergyBomb!", "", "NULL"
	},

	{ "energy drain",{},{},{},{},
		spell_energy_drain, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 35, 12,
		"energy drain", "!Energy Drain!", "", "seishou gesuidou"
	},

	{ "energy shield",{},{},{},{},
		spell_energy_shield, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"energy shield", "The shield of energy around you flickers and fades.", "", "NULL"
	},

	{ "enfeeble",{},{},{},{},
		spell_enfeeble, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 19,
		"enfeeble", "You feel your mind slowly clear up.", "", "NULL"
	},

	{ "epicenter",{},{},{},{},
		spell_epicenter, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"epicenter", "!Epicenter!", "", "NULL"
	},

	{ "equilibrium",{},{},{},{},
		spell_equilibrium, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 50, 20,
		"",	"You feel less weary.",	"", "NULL"
	},

	{ "extinguish",{},{},{},{},
		spell_extinguish, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 6,
		"extinguish", "!Extinguish!", "!Extinguish!", "NULL"
	},

	{ "extraction",{},{},{},{},
		spell_extraction, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 17,
		"extraction", "!Extraction!", "", "NULL"
	},

	{ "faerie fire",{},{},{},{},
		spell_faerie_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 5, 12,
		"faerie fire", "The pink aura around you fades away.", "", "yousei hi"
	},

	{ "fear",{},{},{},{},
		spell_fear, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 25, 19,
		"fear", "!Fear!", "", "NULL"
	},

	{ "fireball",{},{},{},{},
		spell_fireball, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"fireball", "!Fireball!", "", "hidama"
	},

	{ "fireproof",{},{},{},{},
		spell_fireproof, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 10, 12,
		"",	"", "$p's protective aura fades.", "hi hogo"
	},

	{ "fissure",{},{},{},{},
		spell_fissure, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 45, 12,
		"fissure",	"!Fissure!", "!Fissure!", "NULL"
	},

	{ "flammeria",{},{},{},{},
		spell_flammeria, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 60, 16,
		"{Rflammeria{x", "!FLAMERIA!", "!FLAMMERIA!", "NULL"
	},

	{ "flamestrike",{},{},{},{},
		spell_flamestrike, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"flamestrike", "!Flamestrike!", "", "honoo higyoo"
	},

	{ "flora",{},{},{},{},
		spell_flora, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 25, 12,
		"!flora!", "!flora!", "!flora!", "NULL"
	},

	{ "fly",{},{},{},{},
		spell_fly, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 10, 18,
		"", "You slowly float to the ground.", "", "hae"
	},

	{ "floating disc",{},{},{},{},
		spell_floating_disc, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 40, 24,
		"", "!Floating disc!", "", "ukabu enban"
	},

	{ "focus",{},{},{},{},
		spell_focus, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 50, 18,
		"!Focus!", "You feel the energy coarsing through you fade back to normal.", "!Focus!", "NULL"
	},

	{ "force bolt",{},{},{},{},
		spell_forcebolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 70, 4,
		"bolt of force", "!Forcebolt!", "", "inazuma no chikara"
	},

	{ "force void",{},{},{},{},
		spell_force_void, TAR_OBJ_CHAR_OFF, POS_FIGHTING, NULL, SKILL_SPELL, 25, 16,
		"force void", "Your energy feel normal again.", "!ForceVoid!", "NULL"
	},

	{ "forest mist",{},{},{},{},
		spell_forest_mist, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 40, 24,
		"forest mist", "!ForestMist!", "", "NULL"
	},

	{ "freeze",{},{},{},{},
		spell_freeze, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 40, 12,
		"freeze", "!Freeze!", "!Freeze!", "NULL"
	},

	{ "frenzy",{},{},{},{},
		spell_frenzy, TAR_CHAR_DEFENSIVE, POS_STANDING,	NULL, SKILL_SPELL, 30, 24,
		"", "Your rage ebbs.", "", "ikari"
	},

	{ "friendly aura",{},{},{},{},
		spell_friendly_aura, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 100, 18,
		"", "You become less likeable", "", "NULL"
	},

	{ "frost field",{},{},{},{},
		spell_frost_field, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"{cf{Cr{co{Cs{ct {Cf{ci{Ce{cl{Cd{x", "!FrostField!",	"", "NULL"
	},

	{ "gadzap",{},{},{},{},
		spell_gadzap, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 16,
		"gadzap", "!Gadzap!", "", "NULL"
	},

	{ "gate",{},{},{},{},
		spell_gate, TAR_CHAR_WORLD, POS_FIGHTING, NULL, SKILL_SPELL, 80, 12,
		"", "!Gate!", "", "mon"
	},

	{ "ghoul touch",{},{},{},{},
		spell_ghoul_touch, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 16,
		"undead touch", "You feel less sick.", "", "NULL"
	},

	{ "giant strength",{},{},{},{},
		spell_giant_strength, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 20, 12,
		"",	"You feel weaker.",	"", "ooki na tairyuko"
	},

	{ "grove sanctuary",{},{},{},{},
		spell_grove_sanctuary, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"", "!GroveSanctuary!", "!GroveSanctuary!", "NULL"
	},

	{ "guarding spirit",{},{},{},{},
		spell_guarding_spirit, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 50, 23,
		"", "A guarding spirit quietly leaves you.", "", "NULL"
	},

	{ "guiding light",{},{},{},{},
		spell_guiding_light, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"guiding light", "!GuidingLight!", "", " NULL"
	},

	{ "gush",{},{},{},{},
		spell_gush, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 25, 12,
		"{Bwater gush{x", "!Gush!", "!Gush!", "NULL"
	},

	{ "gust",{},{},{},{},
		spell_gust, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 40, 12,
		"gust", "!Gust!", "!Gust!", "NULL"
	},

	{ "harm",{},{},{},{},
		spell_harm, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"harm", "!Harm!", "", "NULL"
	},

	{ "haste",{},{},{},{},
		spell_haste, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"", "You feel yourself slow down.", "", "isogi"
	},

	{ "hawk awareness",{},{},{},{},
		spell_detect_invis, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"",	"You feel the spirit of the hawk leave your body.",	"", "taka no shiryoku"
	},

	{ "heal minor",{},{},{},{},
		spell_heal_minor, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 10, 12,
		"minor heal", "!MinorHeal!", "", "NULL"
	},

	{ "heal light",{},{},{},{},
		spell_heal_light, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"light heal", "!LightHeal!", "", "NULL"
	},

	{ "heal serious",{},{},{},{},
		spell_heal_serious, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50,	12,
		"major heal", "!MajorHeal!", "", "NULL"
	},

	{ "heal critical",{},{},{},{},
		spell_heal_critical, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 60, 12,
		"serious heal", "!SeriousHeal!", "", "NULL"
	},

	{ "heat metal",{},{},{},{},
		spell_heat_metal, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 25, 18,
		"spell", "!Heat Metal!", "", "atatameru kane"
	},

	{ "hellfire",{},{},{},{},
		spell_hellfire, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"{rh{Re{rl{Rl{rf{Ri{rr{Re{x", "!Hellfire!", "", "NULL"
	},

	{ "holy smite",{},{},{},{},
		spell_holy_smite, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"{Choly smite{x", "!HolySmite!", "", "NULL"
	},

	{ "holy word",{},{},{},{},
		spell_holy_word, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 200, 24,
		"divine wrath", "!Holy Word!", "", "shinsei na kotoba"
	},

	{ "icelance",{},{},{},{},
		spell_acid_blast, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"{Cicelance{x", "!IceLance!", "!IceLance!", "NULL"
	},

	{ "identify",{},{},{},{},
		spell_identify, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 12, 12,
		"", "!Identify!", "", "aidentifai"
	},

	{ "imbue",{},{},{},{},
		spell_imbue, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"!imbue!", "You feel less imbued.", "!Imbue!", "NULL"
	},

	{ "imbue earth",{},{},{},{},
		spell_imbue_earth, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_eimbue, SKILL_SPELL, 25, 12,
		"!imbue!", "Your body becomes less earthen.", "!ImbueEarth!", "NULL"
	},

	{ "imbue fire",{},{},{},{},
		spell_imbue_fire, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_eimbue, SKILL_SPELL, 25, 12,
		"!imbue!", "Your body becomes less firey.", "!ImbueFire!", "NULL"
	},

	{ "imbue water",{},{},{},{},
		spell_imbue_water, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_eimbue, SKILL_SPELL, 25, 12,
		"!imbue!", "Your body becomes less water.", "!ImbueWater!", "NULL"
	},

	{ "imbue wind",{},{},{},{},
		spell_imbue_wind, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_eimbue, SKILL_SPELL, 25, 12,
		"!imbue!", "Your body becomes less wind.", "!ImbueWind!", "NULL"
	},

	{ "imbue ice",{},{},{},{},
		spell_imbue_ice, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_eimbue, SKILL_SPELL, 25, 12,
		"!imbue!", "Your body becomes less ice.", "!ImbueIce!", "NULL"
	},

	{ "imbue electric",{},{},{},{},
		spell_imbue_electric, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_eimbue, SKILL_SPELL, 25, 12,
		"!imbue!", "Your body becomes less electric.", "!ImbueElectric!", "NULL"
	},

	{ "imprint",{},{},{},{},
		spell_imprint, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 50, 24,
		"", "!Imprint!", "", "NULL"
	},

	{ "infravision",{},{},{},{},
		spell_infravision, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 5, 18,
		"", "You no longer see in the dark.", "", "niiro shiryoku"
	},

	{ "insomnia",{},{},{},{},
		spell_insomnia, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 10, 24,
		"!INSOMNIA!", "Your mind becomes less restless.", "!Insonmia!", "NULL"
	},

	{ "interrupt",{},{},{},{},
		spell_interrupt, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 40, 5,
		"spell interruption", "!Interrupt!", "!Interrupt!", "NULL"
	},

	{ "invigorate",{},{},{},{},
		spell_invigorate, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"", "!Invigorate!", "", "NULL"
	},

	{ "invisibility",{},{},{},{},
		spell_invis, TAR_OBJ_CHAR_DEF, POS_STANDING, &gsn_invis, SKILL_SPELL, 5, 12,
		"", "You are no longer invisible.",	"$p fades into view.", "me ni tsukanai"
	},

	{ "leaf barrage",{},{},{},{},
		spell_leaf_barrage, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"{gleaf barrage{x", "!LeafBarrage!", "!LeafBarrage!", "NULL"
	},

	{ "lethargy",{},{},{},{},
		spell_lethargy, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 45, 12,
		"lethargy", "You feel lighter.", "!LETHARGY!", "NULL"
	},

	{ "lift curse",{},{},{},{},
		spell_lift_curse, TAR_OBJ_CHAR_DEF, POS_STANDING, NULL, SKILL_SPELL, 20, 12,
		"", "!LiftCurse!", "", "NULL"
	},

	{ "light strike",{},{},{},{},
		spell_light_strike, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"light strike", "!LightStrike!", "", "NULL"
	},

	{ "lightning bolt",{},{},{},{},
		spell_lightning_bolt, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"lightning bolt", "!LightningBolt!",	"", "NULL"
	},

	{ "living armor",{},{},{},{},
		spell_living_armor, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"", "!LivingArmor!", "", "NULL"
	},

	{ "locate object",{},{},{},{},
		spell_locate_object, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 20, 18,
		"", "!Locate Object!", "", "mitsukeru mono"
	},

	{ "lucent rays",{},{},{},{},
		spell_lucent_rays, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"{mlucent ray{x", "!LucentRays!", "", "NULL"
	},

	{ "lunar grace",{},{},{},{},
		spell_lunar_grace, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"", "!LunarGrace!", "", "NULL"
	},

	{ "magic armor",{},{},{},{},
		spell_magic_armor, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 200, 24,
		"magic armor", "!Magic Armor!",	"", "NULL"
	},

	{ "magical lock",{},{},{},{},
		spell_magical_lock, TAR_DOOR, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"", "!MagicLock!", "", "NULL"
	},

	{ "magic missile",{},{},{},{},
		spell_magic_missile, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"magic missile", "!Magic Missile!",	"", "NULL"
	},

	{ "malady",{},{},{},{},
		spell_malady, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_malady, SKILL_SPELL, 10, 12,
		"malady", "You feel your malady slowly fade.", "", "NULL"
	},

	{ "mass invis",{},{},{},{},
		spell_mass_invis, TAR_IGNORE, POS_STANDING, &gsn_mass_invis, SKILL_SPELL, 20, 24,
		"", "You are no longer invisible.", "", " taishuu me ni tsukanai"
	},

	{ "mercurial mind",{},{},{},{},
		spell_mercurial_mind, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 6,
		"", "Your brain feels normal.", "", "NULL"
	},

	{ "meteor storm",{},{},{},{},
		spell_meteor_storm, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"meteor", "!MeteorStorm!", "", " NULL"
	},

	{ "missile swarm",{},{},{},{},
		spell_missile_swarm, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"magic missile", "!MissileSwarm!",	"", "NULL"
	},

	{ "moonbeam",{},{},{},{},
		spell_moonbeam, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"{mmoon beam{x", "!MoonBeam!", "", "NULL"
	},

	{ "mud pack",{},{},{},{},
		spell_mud_pack, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"{Ymud pack{x", "!MudPack!", "", "NULL"
	},

	{ "muddle",{},{},{},{},
		spell_muddle, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 60, 26,
		"!Muddle!", "Your mind clears up.", "!Muddle!", "NULL"
	},

	{ "mystic wall",{},{},{},{},
		spell_mystic_wall, TAR_DOOR, POS_FIGHTING, NULL, SKILL_SPELL, 60, 12,
		"!MysticWall!", "!MysticWall!.", "!MysticWall!", "NULL"
	},

	{ "narcolepsy",{},{},{},{},
		spell_narcolepsy, TAR_OBJ_CHAR_OFF, POS_FIGHTING, &gsn_narcolepsy, SKILL_SPELL, 20, 12,
		"narcolepsy", "Your mind finally feels more awake.", "", "NULL"
	},

	{ "natural curse",{},{},{},{},
		spell_natural_curse, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"natural curse", "!NaturalCurse!", "", "NULL"
	},

	{ "nausea",{},{},{},{},
		spell_nausea, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_nausea, SKILL_SPELL, 30, 12,
		"NULLBAD", "Your mind clears.",	"", "NULL"
	},

	{ "necrosis",{},{},{},{},
		spell_necrosis, TAR_OBJ_CHAR_OFF, POS_FIGHTING, NULL, SKILL_SPELL, 40, 12,
		"necrosis", "Your body slowly regenerates.", "", "NULL"
	},

	{ "nether storm",{},{},{},{},
		spell_nether_storm, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 20,
		"{dNether Storm{x", "!NETHERSTORM!", "!NETHERSTORM!", "NULL"
	},

	{ "nexus",{},{},{},{},
		spell_nexus, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 150, 36,
		"", "!Nexus!", "", "nibai kadoguchi"
	},

	{ "nightmare",{},{},{},{},
		spell_nightmare, TAR_OBJ_CHAR_OFF, POS_FIGHTING, &gsn_nightmare, SKILL_SPELL, 20, 12,
		"nightmare", "You feel less scared.", "The scary on $p dries up.", "NULL"
	},

	{ "nightvision",{},{},{},{},
		spell_nightvision, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 40, 20,
		"nightvision", "Your eyes return to normal.", "", "NULL"
	},

	{ "oakmail",{},{},{},{},
		spell_oakmail, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"", "!Oakmail!", "", "NULL"
	},

	{ "omega",{},{},{},{},
		spell_omega, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"omega", "!omega!", "", "NULL"
	},

	{ "omniblast",{},{},{},{},
		spell_omniblast, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"omniblast", "!omniblast!", "", "NULL"
	},

	{ "overgrowth",{},{},{},{},
		spell_overgrowth, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 25, 12,
		"!overgrowth!", "!Overgrowth!", "!Overgrowth!", "NULL"
	},

	{ "overpower",{},{},{},{},
		spell_overpower, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"", "You feel yourself weaken a bit.", "", "NULL"
	},

	{ "petal storm",{},{},{},{},
		spell_petal_storm, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 80, 20,
		"{gpetal storm{x", "!PETALSTORM!", "!PETALSTORM!", "NULL"
	},

	{ "phantom sword",{},{},{},{},
		spell_phantom_sword, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 20, 12,
		"phantom strike", "Your phantom sword fades into nothingness.", "", "obake no kogeki"
	},

	{ "phase",{},{},{},{},
		spell_phase, TAR_CHAR_SELF, POS_STANDING, &gsn_phase, SKILL_SPELL, 50, 12,
		"PHASE", "Your body feels more dense.",	"", "NULL"
	},

	{ "phasic cone",{},{},{},{},
		spell_phasic_cone, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 18,
		"{Rp{Bh{Ya{Gs{Ce{Cd {Gc{Yo{Bn{Re", "!PhasicCone!", "", "NULL"
	},

	{ "plague",{},{},{},{},
		spell_plague, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_plague, SKILL_SPELL, 20, 12,
		"sickness", "Your sores vanish.", "", "pesuto"
	},

	{ "pleasant climate",{},{},{},{},
		spell_pleasant_climate, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"", "!PleasantClimate!", "", "NULL"
	},

	{ "poison",{},{},{},{},
		spell_poison, TAR_OBJ_CHAR_OFF, POS_FIGHTING, &gsn_poison, SKILL_SPELL, 20, 12,
		"poison", "You feel less sick.", "The poison on $p dries up.", "doku"
	},

	{ "poison ivy",{},{},{},{},
		spell_poison_ivy, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_poison_ivy, SKILL_SPELL, 30, 12,
		"poison ivy", "Your poison rashes fade.", "!PoisonIvy!", "NULL"
	},

	{ "polarity",{},{},{},{},
		spell_polarity, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 1, 20,
		"{rpolarity burst", "!Polarity!", "NULL", "NULL"
	},

	{ "portal",{},{},{},{},
		spell_portal, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"", "!Portal!", "", "kadoguchi"
	},

	{ "power rush",{},{},{},{},
		spell_power_rush, TAR_CHAR_SELF, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"", "!PowerRush!", "!PowerRush!", "NULL"
	},

	{ "prayer",{},{},{},{},
		spell_prayer, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"!prayer!", "!prayer1!", "!prayer2!", "NULL"
	},

	{ "prism shot array",{},{},{},{},
		spell_prismshotarray, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_prismshotarray, SKILL_SPELL, 80, 12,
		"prism shot", "You feel a little better.", "NULL", "NULL"
	},

	{ "purify",{},{},{},{},
		spell_purify, TAR_OBJ_CHAR_DEF, POS_STANDING, NULL, SKILL_SPELL, 25, 12,
		"", "!PURIFY!",	"!PURIFY!", "NULL"
	},

	{ "putrid explosion",{},{},{},{},
		spell_putrid_explosion, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 30,
		"{dputrid explosion{x", "!Putrid Explosion!", "", "NULL"
	},

	{ "quagmire",{},{},{},{},
		spell_quagmire, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 40, 24,
		"", "!Quagmire!", "", "NULL"
	},

	{ "quench",{},{},{},{},
		spell_quench, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 10, 10,
		"", "!Quench", "!Quench!", "NULL"
	},

	{ "ravage",{},{},{},{},
		spell_ravage, TAR_OBJ_CHAR_OFF, POS_FIGHTING, &gsn_ravage, SKILL_SPELL, 25, 20,
		"ravage", "Your body is less tormented.", "", "NULL"
	},

	{ "ray of truth",{},{},{},{},
		spell_ray_of_truth, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"ray of truth", "!Ray of Truth!", "", "ichimatsu no shinri"
	},

	{ "recharge",{},{},{},{},
		spell_recharge, TAR_OBJ_INV, POS_STANDING, NULL, SKILL_SPELL, 60, 24,
		"", "!Recharge!", "", "tsumo mata"
	},

	{ "refresh",{},{},{},{},
		spell_refresh, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 12, 18,
		"refresh", "!Refresh!", "", "rifureshu"
	},

	{ "recovery",{},{},{},{},
		spell_recovery, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 50, 18,
		"!regen!", "You feel the warmth in your body fade.", "!REGEN!", "NULL"
	},

	{ "renew",{},{},{},{},
		spell_renew, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 175, 18,
		"renewal", "!renew!", "", "NULL"
	},

	{ "restoration",{},{},{},{},
		spell_restoration, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 36,
		"restoration", "!Restoration!", "", "NULL"
	},

	{ "resuscitate",{},{},{},{},
		spell_resuscitate, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 200, 26,
		"resuscitation", "!resuscitate!", "", "NULL"
	},

	{ "reveal",{},{},{},{},
		spell_reveal, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 12, 12,
		"reveal", "!reveal!", "", "yousei enmu"
	},

	{ "reverse polarity",{},{},{},{},
		spell_reverse_polarity, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 20,
		"{breverse polarity burst", "!ReversePolarity!", "NULL", "NULL"
	},

	{ "revive",{},{},{},{},
		spell_revive, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 150, 20,
		"revival", "!revive!", "", "NULL"
	},

	{ "rock spike",{},{},{},{},
		spell_rock_spike, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"{yrock spike", "!RockSpike!", "!RockSpike!", "NULL"
	},

	{ "sacrifice",{},{},{},{},
		spell_sacrifice, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 1, 16,
		"Sacrifice", "", "", "null"
	},

	{ "sanctuary",{},{},{},{},
		spell_sanctuary, TAR_CHAR_DEFENSIVE, POS_STANDING, &gsn_sanctuary, SKILL_SPELL, 75, 12,
		"", "The white aura around your body fades.", "", "reihaidoo"
	},

	{ "scarification",{},{},{},{},
		spell_scarification, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 60, 18,
		"self mutilation", "Your scars magically dissapear.", "", "NULL"
	},

	{ "scourge",{},{},{},{},
		spell_scourge, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 160, 16,
		"{rscourge{x", "You feel more clean.", "!Scourge!", "NULL"
	},

	{ "shadow fiend",{},{},{},{},
		spell_shadowfiend, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 25, 20,
		"{dshadow fiend{x", "!Shadowfiend!", "", "NULL"
	},

	{ "shield",{},{},{},{},
		spell_shield, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 12, 18,
		"", "Your force shield shimmers then fades away.", "", "tate"
	},

	{ "shield of shadows",{},{},{},{},
		spell_shield_of_shadows, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 12,
		"", "!shadowshield!", "", "NULL"
	},

	{ "shield of thorns",{},{},{},{},
		spell_shield_of_thorns, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"", "!ShieldOfThorns!", "", "NULL"
	},

	{ "shock",{},{},{},{},
		spell_shock, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 10,
		"shock", "!Shock!", "", "NULL"
	},

	{ "silence",{},{},{},{},
		spell_silence, TAR_OBJ_CHAR_OFF, POS_FIGHTING, NULL, SKILL_SPELL, 25, 12,
		"silence", "Your voice returns.", "!Silence!", "NULL"
	},

	{ "sleep",{},{},{},{},
		spell_sleep, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_sleep, SKILL_SPELL, 15, 12,
		"", "You feel less tired.", "", "nemuri"
	},

	{ "slow",{},{},{},{},
		spell_slow, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 30, 12,
		"", "You feel yourself speed up.", "", "osoi"
	},

	{ "smokescreen",{},{},{},{},
		spell_smokescreen, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"", "!SmokeScreen!", "!SmokeScreen!", "NULL"
	},

	{ "solar benison",{},{},{},{},
		spell_solar_benison, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"", "!SolarBenison!", "", "NULL"
	},

	{ "solar rays",{},{},{},{},
		spell_solar_rays, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"{Ysolar ray{x", "!SolarRays!", "", "NULL"
	},

	{ "sonic boom",{},{},{},{},
		spell_sonic_boom, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 12,
		"sonic boom", "!SonicBoom!", "", "NULL"
	},

	{ "soothe",{},{},{},{},
		spell_soothe, TAR_CHAR_DEFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 45, 16,
		"!Soothe!", "!Soothe", "!Soothe!", "NULL"
	},

	{ "spell shield",{},{},{},{},
		spell_spell_shield, TAR_CHAR_SELF, POS_STANDING,&gsn_spellshield, SKILL_SPELL, 75, 16,
		"",	"Your magic resilience vanishes.",	"", "NULL"
	},

	{ "spirit fire",{},{},{},{},
		spell_spirit_fire, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"spirit fire", "The burns on your body slowly heal.", "", "NULL"
	},

	{ "spirit guide",{},{},{},{},
		spell_spirit_guide, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 50, 12,
		"spirit guide", "!Spirit guide!", "", "NULL"
	},

	{ "spirit snow",{},{},{},{},
		spell_spirit_snow, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"spirit snow", "Your limbs slowly and painfully thaw out.", "", "NULL"
	},

	{ "spirit storm",{},{},{},{},
		spell_spirit_storm, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"storm spirit", "!Spirit storm!", "", "NULL"
	},

	{ "spirit wind",{},{},{},{},
		spell_spirit_wind, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"spirit wind", "!Spirit wind!", "", "NULL"
	},

	{ "spiritual radiance",{},{},{},{},
		spell_spiritual_radiance, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 50, 12,
		"spiritual radiance", "The blinding white in your vision fades back to normal.", "", "NULL"
	},

	{ "squall",{},{},{},{},
		spell_squall, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"squall", "!Squall!", "!Squall!", "NULL"
	},

	{ "stellar curse",{},{},{},{},
		spell_stellar_curse, TAR_CHAR_DEFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 16,
		"", "!StellarCurse!", "", "NULL"
	},

	{ "stone skin",{},{},{},{},
		spell_stone_skin, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 12, 18,
		"", "Your skin feels soft again.", "", "hifu no ishi"
	},

	{ "lion strength",{},{},{},{},
		spell_lion_strength, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 10, 10,
		"", "You feel the lion within your soul fade.",	"", "shishi no tsuyosa"
	},

	{ "summon",{},{},{},{},
		spell_summon, TAR_CHAR_WORLD, POS_STANDING, NULL, SKILL_SPELL, 50, 12,
		"", "!Summon!", "", "shookan suru"
	},

	{ "summon spirit",{},{},{},{},
		spell_summonspirit, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 75, 24,
		"!SummonSpirit!", "!SummonSpirit!", "!SummonSpirit!", "NULL"
	},

	{ "sunbeam",{},{},{},{},
		spell_sunbeam, TAR_CHAR_OFFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 50, 16,
		"{Ysun beam{x", "!SunBeam!", "!SunBeam!", "NULL"
	},

	{ "teleport",{},{},{},{},
		spell_teleport, TAR_CHAR_SELF, POS_FIGHTING, NULL, SKILL_SPELL, 35, 12,
		"", "!Teleport!", "", "hakobu"
	},

	{ "tempest",{},{},{},{},
		spell_tempest, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 60, 16,
		"{Bt{ce{Bm{cp{Be{cs{Bt{x", "!Tempest!", "!Tempest!", "NULL"
	},

	{ "terror",{},{},{},{},
		spell_fear, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 25, 19,
		"terror", "!Terror!", "", "NULL"
	},

	{ "thunderstorm",{},{},{},{},
		spell_thunderstorm, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"{Ythunderstorm{x", "!ThunderStorm!",	"", "NULL"
	},

	{ "tidal wave",{},{},{},{},
		spell_tidal_wave, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"{Btidal wave{x", "!TidalWave!",	"", "NULL"
	},

	{ "tornado",{},{},{},{},
		spell_tornado, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 12,
		"{wt{xo{wr{xn{wa{xd{wo{x", "!Tornado!",	"", "NULL"
	},

	{ "tranquility",{},{},{},{},
		spell_tranquility, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 50, 18,
		"", "You are less tranquil.", "", "NULL"
	},

	{ "turn undead",{},{},{},{},
		spell_turn_undead, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"turn", "!TurnUndead!", "!TurnUndead!", "NULL"
	},

	{ "unholy gate",{},{},{},{},
		spell_unholygate, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 50, 40,
		"{ddark shadow{x", "!UnholyGate!", "", "NULL"
	},

	{ "venom dart",{},{},{},{},
		spell_venom_dart, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"{gvenom {xdart", "!VenomDart!", "", "NULL"
	},

	{ "ventriloquate",{},{},{},{},
		spell_ventriloquate, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 5, 12,
		"", "!Ventriloquate!", "", "uso no hanashite"
	},

	{ "vine lash",{},{},{},{},
		spell_vine_lash, TAR_CHAR_OFFENSIVE, POS_STANDING, NULL, SKILL_SPELL, 25, 12,
		"vine lash", "!VineLash!", "", "NULL"
	},

	{ "vitalize",{},{},{},{},
		spell_vitalize, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"", "!Vitalize!", "", "NULL"
	},

	{ "volatile burst",{},{},{},{},
		spell_volatile_burst, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 22, 14,
		"volatile burst", "!VolatileBurst!", "!VolatileBurst!", "NULL"
	},

	{ "wave",{},{},{},{},
		spell_wave, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 80, 16,
		"{Bwave{x", "!Wave!", "!Wave!", "NULL"
	},

	{ "weaken",{},{},{},{},
		spell_weaken, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 20, 12,
		"spell", "You feel stronger.", "", "yowaku suru"
	},

	{ "wind burst",{},{},{},{},
		spell_wind_burst, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 25, 16,
		"wind burst", "!WindBurst!", "!WindBurst!", "NULL"
	},

	{ "windwalk",{},{},{},{},
		spell_windwalk, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 16,
		"windwalk", "!WindWalk!", "!WindWalk!", "NULL"
	},

	{ "wolf speed",{},{},{},{},
		spell_wolf_speed, TAR_CHAR_SELF, POS_STANDING, NULL, SKILL_SPELL, 10, 10,
		"",			"You feel the wolf within your soul fade.",	"", "okami no hayameru"
	},

	{ "word of recall",{},{},{},{},
		spell_word_of_recall, TAR_CHAR_SELF, POS_RESTING, NULL, SKILL_SPELL, 5, 12,
		"", "!Word of Recall!",	"", "kotoba no yobikaesu"
	},

	{ "wrath",{},{},{},{},
		spell_wrath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 15, 12,
		"{RWrath{x", "!Wrath!",	"", "NULL"
	},

	{ "venom",{},{},{},{},
		spell_venom, TAR_IGNORE, POS_FIGHTING, &gsn_venom, SKILL_SPELL, 15, 12,
		"venom", "You feel substantially better.", "", "NULL"
	},

	{ "yggdrassil prayer",{},{},{},{},
		spell_yggdrassil_prayer, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 100, 24,
		"", "!YggdrassilPrayer!", "", "NULL"
	},

	{ "zephyr wall",{},{},{},{},
		spell_zephyr_wall, TAR_IGNORE, POS_STANDING, NULL, SKILL_SPELL, 40, 24,
		"", "!ZephyrWall!", "", "NULL"
	},

	/*
	* Dragon breath
	*/
	{ "acid breath",{},{},{},{},
		spell_acid_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 100, 24,
		"blast of acid", "!Acid Breath!", "", "kokyuu no san"
	},

	{ "fire breath",{},{},{},{},
		spell_fire_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 200, 24,
		"blast of flame", "The smoke leaves your eyes.", "", "kokyuu no hi"
	},

	{ "frost breath",{},{},{},{},
		spell_frost_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 125, 24,
		"blast of frost", "!Frost Breath!",	"", "kokyuu no shimo"
	},

	{ "gas breath",{},{},{},{},
		spell_gas_breath, TAR_IGNORE, POS_FIGHTING, NULL, SKILL_SPELL, 175, 24,
		"blast of gas", "!Gas Breath!", "", "kokyuu no kitai"
	},

	{ "lightning breath",{},{},{},{},
		spell_lightning_breath, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 150, 24,
		"blast of lightning", "!Lightning Breath!", "", "kokyuu no denkou"
	},

	{ "general purpose",{},{},{},{},
		spell_general_purpose, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 0, 12,
		"general purpose ammo", "!General Purpose Ammo!", "", "ippan no mokuteki"
	},

	{ "high explosive",{},{},{},{},
		spell_high_explosive, TAR_CHAR_OFFENSIVE, POS_FIGHTING, NULL, SKILL_SPELL, 0, 12,
		"high explosive ammo", "!High Explosive Ammo!", "", "taki bakuhatsuteki"
	},


	/* combat and weapons skills */


	{ "recall",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_recall, SKILL_NORMAL, 0, 12,
		"", "!Recall!", "", ""
	},

	{ "fast healing",{},{},{},{},
		spell_null, TAR_IGNORE, POS_SLEEPING, &gsn_fast_healing, SKILL_NORMAL, 0, 0,
		"", "!Fast Healing!", "", ""
	},

	{ "haggle",{},{},{},{},
		spell_null, TAR_IGNORE, POS_RESTING, &gsn_haggle, SKILL_NORMAL, 0, 0,
		"", "!Haggle!", "", ""
	},

	{ "punch",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_punch, SKILL_NORMAL, 0, 20,
		"punch", "!Punch!", "", ""
	},

	{ "riding",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_riding, SKILL_NORMAL, 0,	12,
		"", "!Riding!", "", ""
	},


	{ "exotic",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_exotic, SKILL_NORMAL, 0, 0,
		"{do{xl{wd d{xu{ds{wt{x", "!exotic!", "", ""
	},
	{ "clubs",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_clubs, SKILL_NORMAL, 0, 0,
		"", "!clubs!", "", ""
	},
	{ "swords",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_swords, SKILL_NORMAL, 0, 0,
		"", "!swords!", "", ""
	},
	{ "daggers",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_daggers, SKILL_NORMAL, 0, 0,
		"", "!Daggers!", "", ""
	},
	{ "polearms",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_polearms, SKILL_NORMAL, 0, 0,
		"", "!Polearms!", "", ""
	},
	{ "bludgeons",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_bludgeons, SKILL_NORMAL, 0, 0,
		"", "!Bludgeons!", "", ""
	},
	{ "cleavers",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_cleavers, SKILL_NORMAL, 0, 0,
		"", "!Cleavers!", "", ""
	},
	{ "lashes",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_lashes, SKILL_NORMAL, 0, 0,
		"", "!Lashes!", "", ""
	},
	{ "bows",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_bows, SKILL_NORMAL, 0, 0,
		"", "!bows!", "", ""
	},
	{ "martial arms",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_martial_arms, SKILL_NORMAL, 0, 0,
		"", "!MartialArms!", "", ""
	},
	{ "staffs",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_staffs, SKILL_NORMAL, 0, 0,
		"", "!staffs!", "", ""
	},
	{ "sabers",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_sabers, SKILL_NORMAL, 0, 0,
		"", "!Sabers!", "", ""
	},
	{ "shortswords",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shortswords, SKILL_NORMAL, 0, 0,
		"", "!Shortswords!", "", ""
	},
	{ "lances",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_lances, SKILL_NORMAL, 0, 0,
		"", "!Lances!", "", ""
	},

	{ "throwing axe",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_throwingaxe, SKILL_NORMAL, 0, 0,
		"axe throw", "!TAxe!", "", ""
	},

	{ "throwing dart",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_throwingdart, SKILL_NORMAL, 0, 0,
		"dart throw", "!TDart!", "", ""
	},

	{ "throwing knife",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_throwingknife, SKILL_NORMAL, 0, 0,
		"knife throw", "!TKnife!", "", ""
	},

	{ "abjuring intellect",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_abjuring_intellect, SKILL_NORMAL, 0, 0,
		"", "!AbjuringIntellect!", "", ""
	},

	{ "absorb",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_absorb, SKILL_NORMAL, 0, 0,
		"", "!Absorb!", "", ""
	},

	{ "alertness",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_STANDING, &gsn_alertness, SKILL_NORMAL, 0, 12,
		"", "You become less alert to your surroundings...", "", ""
	},

	{ "ambush",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_STANDING, &gsn_ambush, SKILL_NORMAL, 0, 20,
		"ambush", "!Ambush!", "", ""
	},

	{ "animal magnetism",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_STANDING, &gsn_animal_magnetism, SKILL_NORMAL, 0, 20,
		"", "!AnimalMagnetism!", "", ""
	},

	{ "arrow infusion",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_arrow_infusion, SKILL_NORMAL, 0, 12,
		"arrow infusion", "!ArrowInfusion!", "", ""
	},

	{ "arrow shower",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_arrow_shower, SKILL_NORMAL, 0, 12,
		"arrow shower", "!ArrowShower!", "", ""
	},

	{ "axe kick",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kickaxe, SKILL_NORMAL, 0, 12,
		"axe kick", "!AxeKick!", "", ""
	},

	{ "assassinate",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_STANDING, &gsn_assassinate, SKILL_NORMAL, 0, 24,
		"assassination", "!Assassinate!", "", ""
	},

	{ "awareness",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_STANDING, &gsn_awareness, SKILL_NORMAL, 0, 26,
		"", "You become less aware of your surroundings...", "", ""
	},

	{ "backfist",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_STANDING, &gsn_backfist, SKILL_NORMAL, 0, 12,
		"backfist", "!Backfist!", "", ""
	},

	{ "backstab",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_STANDING, &gsn_backstab, SKILL_NORMAL, 0, 24,
		"backstab", "!Backstab!", "", ""
	},

	{ "bandage",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_bandage, SKILL_NORMAL, 0, 12,
		"NULL", "Your bandages become too dirty to use.", "", ""
	},

	{ "barb",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_barb, SKILL_NORMAL, 0, 36,
		"", "!Barp!", "The barbs on $p seem to dull out.", ""
	},

	{ "barding",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_barding, SKILL_NORMAL, 0, 12,
		"", "!barding!", "", ""
	},

	{ "bash",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_FIGHTING, &gsn_bash, SKILL_NORMAL, 0, 24,
		"bash", "!Bash!", "", ""
	},

	{ "batter",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_batter, SKILL_NORMAL, 0, 24,
		"batter", "!Batter!", "", ""
	},

	{ "berserk",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_FIGHTING, &gsn_berserk, SKILL_NORMAL, 0, 24,
		"", "You feel your pulse slow down.", "", ""
	},

	{ "blend",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_blend, SKILL_NORMAL, 0, 12,
		"", "You slowly become unblenderized.", "", ""
	},

	{ "blind fighting",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_blind_fighting, SKILL_NORMAL, 0, 24,
		"", "!blindfighting!", "", ""
	},

	{ "blinding dust",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_blindingdust, SKILL_NORMAL, 0, 40,
		"blinding dust", "You can see!", "", ""
	},

	{ "brawler",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_brawler, SKILL_NORMAL, 0, 24,
		"", "!Brawler!", "", ""
	},

	{ "breach",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_breach, SKILL_NORMAL, 0, 12,
		"breach", "!Breach!", "", ""
	},

	{ "bulwark",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_bulwark, SKILL_NORMAL, 0, 24,
		"", "!Bulwark!", "", ""
	},

	{ "caltrops",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_STANDING, &gsn_caltrops, SKILL_NORMAL, 0, 24,
		"caltrops", "!Caltrops!", "", ""
	},

	{ "cast efficiency",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_cast_efficiency, SKILL_NORMAL, 0, 0,
		"", "!Cast Efficiency!", "", ""
	},

	{ "catnap",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_catnap, SKILL_NORMAL, 0, 12,
		"", "!catnap!", "", ""
	},

	{ "cavalry",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_cavalry, SKILL_NORMAL, 0, 12,
		"", "!Cavalry!", "", ""
	},

	{ "charge",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_charge, SKILL_NORMAL, 0, 40,
		"mounted charge", "!Charge!", "", ""
	},

	{ "charger",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_charger, SKILL_NORMAL, 0, 12,
		"", "!charger!", "", ""
	},

	{ "cleave",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_cleave, SKILL_NORMAL, 0, 24,
		"cleave", "!Cleave!", "", ""
	},

	{ "coldblooded horse",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_coldblooded_horse, SKILL_NORMAL, 0, 12,
		"", "!ColdBloodedHorse!", "", ""
	},

	{ "combat proficiency",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_combat_proficiency, SKILL_NORMAL, 0, 24,
		"", "!CombatProficiency", "", ""
	},

	{ "combatives",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_combatives, SKILL_NORMAL, 0, 0,
		"", "!Combatives!", "", ""
	},

	{ "conceal",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_conceal, SKILL_NORMAL, 0, 12,
		"", "$p becomes more visible.", "", ""
	},

	{ "concussive blow",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_concussive_blow, SKILL_NORMAL, 0, 12,
		"", "!ConcussiveBlow!", "", ""
	},

	{ "coule",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_coule, SKILL_NORMAL, 0, 0,
		"coule", "!Coule!", "", ""
	},

	{ "counter",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_FIGHTING, &gsn_counter, SKILL_NORMAL, 0, 0,
		"counterattack", "!Counter!", "", ""
	},

	{ "counterfeit",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_counterfeit, SKILL_NORMAL, 0, 12,
		"", "!Counterfeit!", "", ""
	},

	{ "courser",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_courser, SKILL_NORMAL, 0, 12,
		"", "!Courser!", "", ""
	},

	{ "cranial strike",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_cranial_strike, SKILL_NORMAL, 0, 12,
		"", "!CranialStrike!", "", ""
	},

	{ "critical strike",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_critical, SKILL_NORMAL, 0, 0,
		"", "!Critical Strike!", "", ""
	},

	{ "cross punch",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_punchcross, SKILL_NORMAL, 0, 10,
		"right cross", "!Cross!", "", ""
	},

	{ "crush",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_crush, SKILL_NORMAL, 0, 20,
		"crush", "!Crush!", "", ""
	},

	{ "culmination",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_culmination, SKILL_NORMAL, 0, 24,
		"", "!Culmination!", "", ""
	},

	{ "cutpurse",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_cutpurse, SKILL_NORMAL, 0, 24,
		"", "!Cutpurse!", "", ""
	},

	{ "damage reduction",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_damage_reduction, SKILL_NORMAL, 0, 0,
		"", "!DamageReduction!", "", ""
	},

	{ "deathblow",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_deathblow, SKILL_NORMAL, 0, 0,
		"deathblow", "!deathblow!", "", ""
	},

	{ "defender",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_defender, SKILL_NORMAL, 0, 0,
		"", "!Defender!", "", ""
	},

	{ "defensive stance",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_defensive_stance, SKILL_NORMAL, 0, 24,
		"", "!DefensiveStance!", "", ""
	},

	{ "desert cover",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_desertcover, SKILL_NORMAL, 0, 12,
		"", "You are no longer one with the desert.", "", ""
	},

	{ "destrier",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_destrier, SKILL_NORMAL, 0, 12,
		"", "!destrier!", "", ""
	},

	{ "detect traps",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_detecttraps, SKILL_NORMAL, 0, 24,
		"detecttrap", "detecttrap.", "", ""
	},

	{ "devotion",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_devotion, SKILL_NORMAL, 0, 24,
		"devotion", "!Devotion!", "", ""
	},

	{ "devour corpses",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_devour, SKILL_NORMAL, 0, 24,
		"devour", "!devour!", "", ""
	},

	{ "dirt kicking",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dirt, SKILL_NORMAL,	0,	24,
		"kicked dirt", "You rub the dirt out of your eyes.", "", ""
	},

	{ "disable traps",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_disabletraps, SKILL_NORMAL, 0, 24,
		"disabletrap", "disabletrap.", "", ""
	},

	{ "disarm",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_disarm, SKILL_NORMAL, 0, 24,
		"", "!Disarm!", "", ""
	},

	{ "distract",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_distract, SKILL_NORMAL, 0, 23,
		"", "!Distract!", "", ""
	},

	{ "divert",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_divert, SKILL_NORMAL, 0, 24,
		"divert", "!divert!", "", ""
	},

	{ "divider",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_divider, SKILL_NORMAL, 0, 12,
		"divider", "!Divider!", "", ""
	},

	{ "divine touch",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_divinetouch, SKILL_NORMAL, 0, 12,
		"NULL", "!DivineTouch!", "", ""
	},

	{ "divinity",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_divinity, SKILL_NORMAL, 0, 12,
		"NULL", "!Divinity!", "", ""
	},

	{ "dodge",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dodge, SKILL_NORMAL, 0, 0,
		"", "!Dodge!", "", ""
	},

	{ "dogma",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dogma, SKILL_NORMAL, 0, 0,
		"", "!Dogma!", "", ""
	},

	{ "double blitz",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_double_blitz, SKILL_NORMAL, 0, 19,
		"double blitz", "!DoubleBlitz!", "", ""
	},

	{ "doubleshot",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_doubleshot, SKILL_NORMAL, 0, 19,
		"doubleshot", "!DoubleShot!", "", ""
	},

	{ "downstrike",{},{},
		{},
		{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_downstrike, SKILL_NORMAL, 0, 12,
		"downstrike", "!Downstrike!", "", ""
	},

	{ "dragon fist",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dragon_fist, SKILL_NORMAL, 0, 24,
		"dragon fist", "!dragonfist!", "", ""
	},

	{ "dual backstab",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dual_backstab, SKILL_NORMAL, 0, 0,
		"dual backstab", "!DualBackstab!", "", ""
	},

	{ "dual shield",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_dual_shield,	SKILL_NORMAL, 0, 0,
		"", "!DualShield!", "", ""
	},

	{ "dual wield",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_dual_wield, SKILL_NORMAL, 0, 24,
		"", "!Dual Wield!", "", ""
	},

	{ "duck",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_duck, SKILL_NORMAL, 0, 0,
		"", "!Duck!", "", ""
	},

	{ "earth wisdom",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_earth_wisdom, SKILL_NORMAL, 0, 24,
		"", "!EarthWisdom!", "", ""
	},

	{ "eavesdrop",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_eavesdrop, SKILL_NORMAL, 0, 24,
		"eavesdrop", "!eavesdrop!", "", ""
	},

	{ "elemental mastery",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_elemental_mastery, SKILL_NORMAL, 0, 0,
		"", "!Elemental Mastery!", "", ""
	},

	{ "elude",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_elude, SKILL_NORMAL, 0, 0,
		"", "!Elude!", "", ""
	},

	{ "embalm",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_embalm, SKILL_NORMAL, 0, 0,
		"", "!Embalm!", "", ""
	},

	{ "endure",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_endure, SKILL_NORMAL, 0, 24,
		"endure", "!Endure!", "", ""
	},

	{ "energy crash",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_energy_crash, SKILL_NORMAL, 0, 0,
		"", "!EnergyCrash!", "", ""
	},

	{ "enhanced damage",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_enhanced_damage, SKILL_NORMAL, 0, 0,
		"", "!Enhanced Damage!", "", ""
	},

	{ "envenom",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_RESTING, &gsn_envenom, SKILL_NORMAL, 0, 36,
		"", "!Envenom!", "", ""
	},

	{ "equestrian",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_equestrian, SKILL_NORMAL, 0, 12,
		"", "!Equestrian!", "", ""
	},

	{ "errant",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_errant, SKILL_NORMAL, 0, 12,
		"", "!Errant!", "", ""
	},

	{ "escape",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_quickescape, SKILL_NORMAL, 0, 36,
		"", "!Escape!", "", ""
	},

	{ "evade",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_evade, SKILL_NORMAL, 0, 16,
		"", "!Evade!", "", ""
	},

	{ "face kick",{},{},
		{},
		{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_facekick, SKILL_NORMAL, 0, 12,
		"kick to the face", "!FaceKick!", "", ""
	},

	{ "faith",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_FIGHTING, &gsn_faith, SKILL_NORMAL, 0, 24,
		"", "!Faith!", "", ""
	},

	{ "falconry",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_falconry, SKILL_NORMAL, 10, 12,
		"", "!Falconry!", "", ""
	},

	{ "feedback",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_feedback, SKILL_SPELL, 50, 16,
		"feedback",	"Your mana repulsion slowly slips from your body.",	"", "NULL"
	},

	{ "feint",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_feint, SKILL_NORMAL, 0, 0,
		"", "!Feint!", "", ""
	},

	{ "feral slash",{},{},
		{},
		{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_feralslash, SKILL_NORMAL, 0, 16,
		"feral slash", "!FeralSlash!", "", ""
	},

	{ "fierce palm",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_fierce_palm, SKILL_NORMAL, 0, 24,
		"fierce palm", "!fiercepalm!", "", ""
	},

	{ "final strike",{},{},{},{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_final_strike, SKILL_NORMAL, 0, 12,
		"final strike", "!LastStrike!", "", ""
	},

	{ "first aid",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_firstaid, SKILL_NORMAL, 0, 20,
		"first aid", "", "", ""
	},

	{ "fissure",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_fissure, SKILL_NORMAL, 0, 12,
		"fissure", "!Fissure!", "", ""
	},

	{ "fist of fury",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_fist_of_fury, SKILL_NORMAL, 0, 24,
		"fist of fury", "!fistoffury!", "", ""
	},

	{ "florentine",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_florentine, SKILL_NORMAL, 0, 24,
		"", "!Florentine!", "", ""
	},

	{ "fluid motion",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_fluidmotion, SKILL_NORMAL, 0, 24,
		"", "!FluidMotion!", "", ""
	},

	{ "flurry",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_flurry, SKILL_NORMAL, 0, 6,
		"", "!Flurry!", "", ""
	},

	{ "furor",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_furor, SKILL_NORMAL, 0, 24,
		"", "!Furor!", "", ""
	},

	{ "fortify",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_fortify, SKILL_NORMAL, 0, 24,
		"", "You take a more offensive stance.", "", ""
	},

	{ "fuse",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_fuse, SKILL_NORMAL, 0, 24,
		"", "!FUSE!", "", ""
	},

	{ "graverob",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_graverob, SKILL_NORMAL, 0, 20,
		"", "!Graveyard!", "", ""
	},

	{ "gouge",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_gouge, SKILL_NORMAL, 0, 18,
		"gouge", "!Gouge!", "", ""
	},

	{ "grasp",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_grasp, SKILL_NORMAL, 0, 36,
		"", "!Grasp!", "Your second hand slips from $p's handle.", ""
	},

	{ "grip",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_grip, SKILL_NORMAL, 0, 36,
		"", "!Grip!", "Your hand loses its grip on $p.", ""
	},

	{ "guard",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_guard, SKILL_NORMAL, 0, 12,
		"", "!Guard!", "", ""
	},

	{ "hack",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_hack, SKILL_NORMAL, 0, 12,
		"hack", "!Hack!", "", ""
	},

	{ "headshot",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_headshot, SKILL_NORMAL, 0, 0,
		"", "!Headshot!", "", ""
	},

	{ "heavy armor",{},{},{},{},
		spell_null, TAR_IGNORE, POS_SLEEPING, &gsn_heavy_armor, SKILL_NORMAL, 0, 0,
		"", "!HeavyArmor!", "", ""
	},

	{ "heavy hand",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_heavy_hand, SKILL_NORMAL, 0, 0,
		"", "!HeavyHand!", "", ""
	},

	{ "heavy smash",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_heavy_smash, SKILL_NORMAL, 0, 24,
		"heavy smash", "More MORE smashy?", "", ""
	},

	{ "heightened senses",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_heightened_senses, SKILL_NORMAL, 0, 24,
		"", "!HeightenedSenses!", "", ""
	},

	{ "hide",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_RESTING, &gsn_hide, SKILL_NORMAL, 0, 12,
		"", "You sneeze and lose your cover.", "", ""
	},

	{ "high kick",{},{},
		{},
		{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kickhigh, SKILL_NORMAL, 0, 12,
		"high kick", "!HighKick!", "", ""
	},

	{ "hobby",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_hobby, SKILL_NORMAL, 0, 12,
		"", "!hobby!", "", ""
	},

	{ "holy touch",{},{},
		{},
		{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_holytouch, SKILL_NORMAL, 0, 12,
		"NULL", "!HolyTouch!", "", ""
	},

	{ "horsemanship",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_horsemanship, SKILL_NORMAL, 0, 12,
		"", "!Horsemanship!", "", ""
	},

	{ "hotblooded horse",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_hotblooded_horse, SKILL_NORMAL, 0, 12,
		"", "!HotbloodedHorse!", "", ""
	},

	{ "hunt",{},{},{},{},
		spell_null, TAR_IGNORE,		POS_STANDING,	&gsn_hunt,		SKILL_NORMAL,	0,	12,
		"",			"!Hunt!",		"", ""
	},

	{ "improved accuracy",{},{},{},	{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_improved_accuracy, SKILL_NORMAL, 0, 0,
		"", "!ImprovedAccuracy!", "", ""
	},

	{ "improvise",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_improvise, SKILL_NORMAL, 0, 12,
		"improvise", "improvise", "", ""
	},

	{ "inner peace",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_innerpeace, SKILL_NORMAL, 0, 24,
		"", "!InnerPeace!", "", ""
	},

	{ "instinct",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_instinct, SKILL_NORMAL, 0, 12,
		"instinct", "!Instinct!", "", ""
	},

	{ "introversion",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_introversion, SKILL_NORMAL, 0, 24,
		"introversion", "!Introversion!", "", ""
	},

	{ "jab",{},{},
		{},
		{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_punchjab, SKILL_NORMAL, 0, 9,
		"jab", "!jab!", "", ""
	},

	{ "kick",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kick, SKILL_NORMAL, 0, 20,
		"kick", "!Kick!", "", ""
	},

	{ "kindred spirits",{},{},
		{},
		{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kindred_spirits, SKILL_NORMAL, 0, 12,
		"shared damage", "!KindredSpirits!", "", ""
	},

	{ "kneeshot",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_kneeshot, SKILL_NORMAL, 0, 24,
		"kneeshot", "!Kneeshot!", "", ""
	},

	{ "lance fighting",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_SLEEPING, &gsn_lance_fighting, SKILL_NORMAL, 0, 0,
		"", "!LanceFighting!", "", ""
	},

	{ "lance mastery",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_lancemastery, SKILL_NORMAL, 0, 0,
		"", "!Lancemastery!", "", ""
	},

	{ "last rites",{},{},
		{},
		{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_lastrites, SKILL_NORMAL, 0, 12,
		"", "!LastRites!", "", ""
	},

	{ "lead stance",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_lead_stance, SKILL_NORMAL, 0, 0,
		"", "!LeadStance!", "", ""
	},

	{ "leadership",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_leadership, SKILL_NORMAL, 0,	24,
		"",			"You become another nameless Joe...",	"", ""
	},

	{ "leverage",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_cavalry, SKILL_NORMAL, 0, 12,
		"", "!Leverage!", "", ""
	},

	{ "light armor",{},{},{},{},
		spell_null, TAR_IGNORE, POS_SLEEPING, &gsn_light_armor, SKILL_NORMAL, 0, 0,
		"", "!LightArmor!", "", ""
	},

	{ "lion combo",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_lion_combo, SKILL_NORMAL, 0, 24,
		"roar", "!lioncombo!", "", ""
	},

	{ "lop",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_lop, SKILL_NORMAL, 0, 12,
		"lop", "!Lop!", "", ""
	},

	{ "lore",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_RESTING, &gsn_lore, SKILL_NORMAL, 0, 26,
		"", "!lore!", "", ""
	},

	{ "low kick",{},{},
		{},
		{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kicklow, SKILL_NORMAL, 0, 12,
		"low kick", "!LowKick!", "", ""
	},

	{ "magical mastery",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_magical_mastery, SKILL_NORMAL, 0, 0,
		"", "!MagicalMastery!", "", ""
	},

	{ "mark",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_mark, SKILL_NORMAL, 0, 12,
		"", "!Mark!", "", ""
	},

	{ "mark of the beast",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_mark_of_the_beast, SKILL_NORMAL, 0, 0,
		"", "!MarkOfTheBeast!", "", ""
	},

	{ "marksmanship",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_marksmanship, SKILL_NORMAL, 0, 0,
		"", "!Marksmanship!", "", ""
	},

	{ "martial arts",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_martial_arts, SKILL_NORMAL, 0, 0,
		"combo attack", "!Martial Arts!", "", ""
	},

	{ "meditation",{},{},{},{},
		spell_null, TAR_IGNORE, POS_SLEEPING, &gsn_meditation, SKILL_NORMAL, 0, 0,
		"", "Meditation", "", ""
	},

	{ "medium armor",{},{},{},{},
		spell_null, TAR_IGNORE, POS_SLEEPING, &gsn_medium_armor, SKILL_NORMAL, 0, 0,
		"", "!MediumArmor!", "", ""
	},

	{ "mental focus",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_mental_focus, SKILL_NORMAL, 0, 0,
		"", "!MentalFocus!", "", ""
	},

	{ "mighty swing",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_mighty_swing, SKILL_NORMAL, 0, 24,
		"mighty swing", "MIGHTY SWING", "", ""
	},

	{ "mirage",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_mirage, SKILL_NORMAL, 30, 12,
		"mirage", "Your illusions seem to fade away.", "", ""
	},

	{ "mithril bash",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_mithril_bash, SKILL_NORMAL, 0, 12,
		"enhanced bash", "!MithrilBash!", "", ""
	},

	{ "mounted dirtkick",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_FIGHTING, &gsn_mounteddirt, SKILL_NORMAL, 0, 24,
		"kicked dirt", "You rub the dirt out of your eyes.", "", ""
	},

	{ "morph serpent",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_morph_serpent, SKILL_SPELL, 15, 12,
		"NULL", "!SnakeForm!", "", "NULL"
	},

	{ "morph canine",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_morph_canine, SKILL_SPELL, 15, 12,
		"NULL", "!CanineForm!",	"", "NULL"
	},

	{ "morph feline",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_morph_feline, SKILL_SPELL, 15, 12,
		"NULL", "!FelineForm!", "", "NULL"
	},

	{ "morph ursa",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_morph_ursa, SKILL_SPELL, 15, 12,
		"NULL", "!UrsaForm!", "", "NULL"
	},

	{ "morph avian",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_morph_avian, SKILL_SPELL, 15, 12,
		"NULL", "!AvianForm!", "", "NULL"
	},

	{ "morph aquatic",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_morph_aquatic, SKILL_SPELL, 15, 12,
		"NULL", "!AquaticForm!", "", "NULL"
	},

	{ "mortal coil",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_mortal_coil, SKILL_NORMAL, 0, 0,
		"", "!MortalCoil!", "", ""
	},

	{ "mounted dodge",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_mounteddodge, SKILL_NORMAL, 0, 0,
		"", "!MDodge!", "", ""
	},

	{ "mustang heart",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_mustang_heart, SKILL_NORMAL, 40, 40,
		"", "!MustangHeart!", "", ""
	},

	{ "needle",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_needle, SKILL_NORMAL, 0, 12,
		"", "!Needle!", "", ""
	},

	{ "nerve",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_nerve, SKILL_NORMAL, 0, 18,
		"", "!Nerve!", "", ""
	},

	{ "neural agent",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_neural_agent, SKILL_NORMAL, 0, 24,
		"", "!NeuralAgent!", "", ""
	},

	{ "numbing force",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_numbing_force, SKILL_NORMAL, 0, 12,
		"", "!NumbingForce!", "", ""
	},

	{ "occult wisdom",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_occult_wisdom, SKILL_NORMAL, 0, 0,
		"", "!OccultWisdom!", "", ""
	},

	{ "outmaneuver",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_outmaneuver, SKILL_NORMAL, 0, 10,
		"", "!OUTMANEUVER!", "", ""
	},

	{ "overdrive",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_overdrive, SKILL_NORMAL, 0, 10,
		"", "Your body painfully returns to normal.", "", ""
	},

	{ "paired attack",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_paired_attack, SKILL_NORMAL, 0, 24,
		"paired attack", "!PairedAttack!", "", ""
	},

	{ "palm",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_palm, SKILL_NORMAL, 0, 24,
		"", "!Palm!", "", ""
	},

	{ "paralyzing fear",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_paralyzingfear, SKILL_NORMAL, 0, 12,
		"paralyzing fear", "ParalyzingFear", "", ""
	},

	{ "paranoia",{},{},{},{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_paranoia, SKILL_NORMAL, 0, 12,
		"paranoia", "You become less paranoid.", "", ""
	},

	{ "parry",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_parry, SKILL_NORMAL, 0, 0,
		"", "!Parry!", "", ""
	},

	{ "parting shot",{},{},{},{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_parting_shot, SKILL_NORMAL, 0, 12,
		"parting shot", "!PartingShot!", "", ""
	},

	{ "peek",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_peek, SKILL_NORMAL, 0, 0,
		"", "!Peek!", "", ""
	},

	{ "penumbral veil",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_RESTING, &gsn_penumbralveil, SKILL_NORMAL, 0, 12,
		"", "!PenumbralVeil!", "", ""
	},

	{ "pick lock",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_pick_lock, SKILL_NORMAL, 0, 12,
		"", "!Pick!", "", ""
	},

	{ "pierce",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_pierce, SKILL_NORMAL, 0, 30,
		"pierce", "!Pierce!", "", ""
	},

	{ "piety",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_piety, SKILL_NORMAL, 0, 24,
		"piety", "!Piety!", "", ""
	},

	{ "poison cloud",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_poisoncloud, SKILL_NORMAL, 0, 40,
		"poisonous cloud", "You feel much better!", "", ""
	},

	{ "pommel strike",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_pommel_strike, SKILL_NORMAL, 0, 20,
		"pommel strike", "PommelStrike", "", ""
	},

	{ "poverty",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_poverty, SKILL_NORMAL, 0, 24,
		"poverty", "!Poverty!", "", ""
	},

	{ "preparation attack",{},{},{},{},
		spell_null, TAR_IGNORE, POS_RESTING, &gsn_preparationattack, SKILL_NORMAL, 0, 18,
		"counter", "!PreparationAttack!", "", ""
	},

	{ "primal roar",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_primal_roar, SKILL_NORMAL, 0, 0,
		"", "!PrimalRoar!", "", ""
	},

	{ "purity",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_purity, SKILL_NORMAL, 0, 12,
		"NULL", "!Purity!", "", ""
	},

	{ "rabid",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_rabid, SKILL_NORMAL, 0, 0,
		"", "!Rabid!", "", ""
	},

	{ "rage",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_rage, SKILL_NORMAL, 0, 0,
		"", "!Rage!", "", ""
	},

	{ "raptor",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_raptor, SKILL_NORMAL, 0, 0,
		"raptor strike", "!Raptor!", "", ""
	},

	{ "ready",{},{},{},{},
		spell_null, TAR_IGNORE, POS_RESTING, &gsn_ready, SKILL_NORMAL, 0, 12,
		"", "!Ready!", "", ""
	},

	{ "rebound",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_rebound, SKILL_NORMAL, 0, 18,
		"rebound", "!Rebound!", "", ""
	},

	{ "reckless abandon",{},{},{},{},
		spell_null, TAR_IGNORE,	POS_FIGHTING, &gsn_reckless_abandon, SKILL_NORMAL, 0, 24,
		"", "You slowly calm down.", "", ""
	},

	{ "reject magic",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_reject_magic, SKILL_NORMAL, 0, 0,
		"", "!RejectMagic!", "", ""
	},

	{ "rescue",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_rescue, SKILL_NORMAL, 0, 12,
		"", "!Rescue!", "", ""
	},

	{ "resist blights",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_resist_blights, SKILL_NORMAL, 0, 12,
		"", "!ResistBlights!", "", ""
	},

	{ "reverence",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_reverence, SKILL_NORMAL, 0, 24,
		"reverence", "!Reverence!", "", ""
	},

	{ "ricochet",{},{},
		{},
		{},
		spell_null, TAR_IGNORE,		POS_FIGHTING, &gsn_ricochet, SKILL_NORMAL, 0, 18,
		"ricochet", "!Ricochet!", "", ""
	},

	{ "rift",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_rift, SKILL_NORMAL, 0, 12,
		"", "!rift!", "", ""
	},

	{ "riposte",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_riposte, SKILL_NORMAL, 0, 0,
		"riposte", "!Riposte!", "", ""
	},

	{ "rising cleave",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_rising_cleave, SKILL_NORMAL, 0, 12,
		"", "!RisingCleave!", "", ""
	},

	{ "roundhouse kick",{},{},
		{},
		{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_kickroundhouse, SKILL_NORMAL, 0, 12,
		"roundhouse kick", "!RoundhouseKick!", "", ""
	},

	{ "rupture",{},{},
		{},
		{},
		spell_null, TAR_IGNORE,	POS_FIGHTING, &gsn_rupture, SKILL_NORMAL, 0, 12,
		"arterial strike", "Your wounds finally heal.", "", ""
	},

	{ "sacred guardian",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_sacred_guardian, SKILL_NORMAL, 0, 0,
		"", "!SacredGuardian!", "", ""
	},

	{ "salve",{},{},
		{},
		{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_salve, SKILL_NORMAL, 0, 12,
		"", "!Salve!", "", ""
	},

	{ "sap",{},{},
		{},
		{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_sap, SKILL_NORMAL, 0, 12,
		"sap", "You slowly regain consciousness.", "", ""
	},

	{ "scrolls",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_scrolls, SKILL_NORMAL, 0, 24,
		"", "!Scrolls!", "", ""
	},

	{ "second attack",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_second_attack, SKILL_NORMAL, 0, 0,
		"", "!Second Attack!", "", ""
	},

	{ "shadow strike",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_shadowstrike, SKILL_NORMAL, 0, 12,
		"shadowstrike", "ShadowStrike", "", ""
	},

	{ "sharp shooting",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_sharp_shooting, SKILL_NORMAL, 0, 12,
		"sharpshooting", "!SharpShooting!", "", ""
	},

	{ "sharpen",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_sharpen, SKILL_NORMAL, 0, 12,
		"", "Sharpen", "$p seems less sharp.", ""
	},

	{ "shield block",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shield_block, SKILL_NORMAL, 0, 0,
		"", "!Shield!", "", ""
	},

	{ "shield fighting",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shieldfighting, SKILL_NORMAL, 0, 24,
		"shield", "!ShieldFight!", "", ""
	},

	{ "shieldtoss",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shieldtoss, SKILL_NORMAL, 0, 36,
		"thrown shield", "!ShieldToss!", "", ""
	},

	{ "shield mastery",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shieldmastery, SKILL_NORMAL, 0, 36,
		"", "!ShieldMastery!", "", ""
	},

	{ "shieldbash",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shieldbash, SKILL_NORMAL, 0, 24,
		"shieldbash", "!ShieldBash!", "", ""
	},

	{ "shieldstrike",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_shieldstrike, SKILL_NORMAL, 0, 20,
		"shield strike", "ShieldStrike", "", ""
	},

	{ "shift",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shift, SKILL_NORMAL, 0, 18,
		"", "!Shift!",		"", ""
	},

	{ "shunt",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_shunt, SKILL_NORMAL, 0, 0,
		"", "!Shunt!", "", ""
	},

	{ "sidestep",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_sidestep, SKILL_NORMAL, 0, 0,
		"", "!Sidestep!", "", ""
	},

	{ "slashing volley",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_slashing_volley, SKILL_NORMAL, 0, 12,
		"", "!SlashingVolley!", "", ""
	},

	{ "slip",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_slip, SKILL_NORMAL, 0, 24,
		"", "!Slip!", "", ""
	},

	{ "smash",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_smash, SKILL_NORMAL, 0, 18,
		"smash", "More smashy?", "", ""
	},

	{ "smelting",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_smelt, SKILL_NORMAL, 0, 24,
		"", "!Smeleting!", "", ""
	},

	{ "snare",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_snare, SKILL_NORMAL, 10, 28,
		"snare", "You finally manage to break free of the ensnarement.", "", ""
	},

	{ "sneak",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_sneak, SKILL_NORMAL, 0, 12,
		"", "You no longer feel stealthy.", "", ""
	},

	{ "sonic impact",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_sonic_impact, SKILL_NORMAL, 0, 12,
		"sonic impact", "!SonicImpact!", "", ""
	},

	{ "spell proficiency",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_spell_proficiency, SKILL_NORMAL, 0, 0,
		"", "!SpellProficiency!", "", ""
	},

	{ "spell resistance",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_spell_resistance, SKILL_NORMAL, 0, 0,
		"", "!SpellResistance!", "", ""
	},

	{ "spellcraft",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_spellcraft, SKILL_NORMAL, 0, 0,
		"", "!SpellCraft!", "", ""
	},

	{ "spinning counter",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_counter_spin, SKILL_NORMAL, 0, 0,
		"counter spin", "!CounterSpin!", "", ""
	},

	{ "spiraling spear",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_spiraling_spear, SKILL_NORMAL, 0, 0,
		"spiraling spear", "!SpiralingSpear!", "", ""
	},

	{ "spur",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_spur, SKILL_NORMAL, 0, 12,
		"", "!Spur!", "", ""
	},

	{ "stalk",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_stalk, SKILL_NORMAL, 0, 12,
		"stalk", "!Stalk!", "", ""
	},

	{ "stamina",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_stamina, SKILL_NORMAL, 0, 12,
		"NULL", "!Stamina!", "", ""
	},

	{ "stance",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_engarde, SKILL_NORMAL, 0, 24,
		"", "You take a more neutral stance.", "", ""
	},

	{ "steal",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_steal, SKILL_NORMAL, 0, 24,
		"", "!Steal!", "", ""
	},

	{ "stealth",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_stealth, SKILL_NORMAL, 0, 12,
		"", "!Stealth!", "", ""
	},

	{ "strafe",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_strafe, SKILL_NORMAL, 0, 12,
		"strafe", "!Strafe!", "", ""
	},

	{ "strangle",{},{},
		{},
		{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_strangle, SKILL_NORMAL, 0, 16,
		"strangle", "You slowly regain consciousness.", "", ""
	},

	{ "strike",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_strike, SKILL_NORMAL, 0, 12,
		"strike", "Less strikey?", "", ""
	},

	{ "survival",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_survival, SKILL_NORMAL, 0, 24,
		"", "", "", ""
	},

	{ "swap",{},{},
		{},
		{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_swap, SKILL_NORMAL, 0, 20,
		"", "!Swap!", "", ""
	},

	{ "swipe",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_swipe, SKILL_NORMAL, 0, 24,
		"", "!Swipe!", "", ""
	},

	{ "sword blitz",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_sword_blitz, SKILL_NORMAL, 0, 12,
		"", "!SwordBlitz!", "", ""
	},

	{ "sword efficiency",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_swordefficiency, SKILL_NORMAL, 0, 0,
		"", "!SwordEfficiency!", "", ""
	},

	{ "sword mastery",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_swordmastery, SKILL_NORMAL, 0, 0,
		"", "!Swordmastery!", "", ""
	},

	{ "swordsmanship",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_swordsmanship, SKILL_NORMAL, 0, 0,
		"", "!Swordsmanship!", "", ""
	},

	{ "third attack",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_third_attack, SKILL_NORMAL, 0, 0,
		"", "!Third Attack!", "", ""
	},

	{ "thrust",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_thrust, SKILL_NORMAL, 0, 18,
		"thrust", "!Thrust!", "", ""
	},

	{ "tigerfang",{},{},{},{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_punchtigerfang, SKILL_NORMAL, 0, 20,
		"Tiger Fang", "!Tigerfang!", "", ""
	},

	{ "trample",{},{},{},{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_trample, SKILL_NORMAL, 0, 12,
		"trample", "!Trample!", "", ""
	},

	{ "tranquility",{},{},{},{},
		spell_null, TAR_CHAR_OFFENSIVE, POS_STANDING, &gsn_tranquility, SKILL_NORMAL, 0, 12,
		"", "!Tranquility!", "", ""
	},

	{ "traps",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_traps, SKILL_NORMAL, 0, 24,
		"trap", "You feel the effects of your entrapment dissipate.", "", ""
	},

	{ "trip",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_trip, SKILL_NORMAL, 0, 19,
		"trip", "!Trip!", "", ""
	},

	{ "trompement",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_trompement, SKILL_NORMAL, 0, 0,
		"", "!Trompement!", "", ""
	},

	{ "true belief",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_true_belief, SKILL_NORMAL, 0, 0,
		"", "!TrueBelief!", "", ""
	},

	{ "trueshot",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_trueshot, SKILL_NORMAL, 0, 0,
		"", "!Trueshot!", "", ""
	},

	{ "twin tiger fang",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_twin_tiger_fang, SKILL_NORMAL, 0, 24,
		"twin tiger fang", "!twintigerfang!", "", ""
	},

	{ "uppercut",{},{},{},{},
		spell_null,	TAR_CHAR_OFFENSIVE, POS_FIGHTING, &gsn_punchuppercut, SKILL_NORMAL, 0, 12,
		"uppercut", "!Uppercut!", "", ""
	},

	{ "unending strikes",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_unending_strikes, SKILL_NORMAL, 0, 24,
		"strike", "!unendingstrikes!", "", ""
	},

	{ "untangle",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_untangle, SKILL_NORMAL, 0, 24,
		"untangle", "!untangle!", "", ""
	},

	{ "vanish",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_vanish, SKILL_NORMAL, 0, 36,
		"", "!vanish!", "", ""
	},

	{ "wands",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_wands, SKILL_NORMAL, 0, 12,
		"", "!Wands!", "", ""
	},

	{ "warrior heart",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_warrior_heart, SKILL_NORMAL, 0, 30,
		"", "Your body returns to normal as your heartbeat slows.", "", ""
	},

	{ "weapons knowledge",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_weapons_knowledge, SKILL_NORMAL, 0, 36,
		"", "!WeaponsKnowledge!", "", ""
	},

	{ "whirlwind",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_whirlwind, SKILL_NORMAL, 0, 10,
		"whirlwind", "whirlwind", "", ""
	},

	{ "will force",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_will_force, SKILL_NORMAL, 0, 0,
		"", "!WillForce!", "", ""
	},

	{ "dualwind",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_dualwind, SKILL_NORMAL, 0, 6,
		"dualwind", "!DualWind!", "", ""
	},

	{ "withdraw",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_withdraw, SKILL_NORMAL, 0, 12,
		"", "!Withdraw!", "", ""
	},

	{ "zeal",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_zeal, SKILL_NORMAL, 0, 24,
		"", "Less zealous.", "", ""
	},
//
	{ "lurk",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_lurk, SKILL_NORMAL, 0, 24,
		"", "You stop lurking.", "", ""
	},

	{ "slither",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_slither, SKILL_NORMAL, 0, 24,
		"", "!Slither!", "", ""
	},

	{ "maul",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_maul, SKILL_NORMAL, 0, 24,
		"", "!Maul!", "", ""
	},

	{ "stalk",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_stalk, SKILL_NORMAL, 0, 24,
		"", "!Stalk!", "", ""
	},

	{ "pounce",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_pounce, SKILL_NORMAL, 0, 24,
		"", "!Pounce!", "", ""
	},

	{ "scratch",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_scratch, SKILL_NORMAL, 0, 24,
		"", "!Scratch!", "", ""
	},

	{ "clench",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_clench, SKILL_NORMAL, 0, 24,
		"", "!Clench!", "", ""
	},

	{ "craze",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_craze, SKILL_NORMAL, 0, 24,
		"", "!Craze!", "", ""
	},

	{ "aerial strike",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_aerial_strike, SKILL_NORMAL, 0, 24,
		"", "!AerialStrike!", "", ""
	},

	{ "ascend",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_ascend, SKILL_NORMAL, 0, 24,
		"", "!Ascend!", "", ""
	},
//
	{ "bravery",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_bravery, SKILL_NATURAL, 0, 24,
		"bravery", "!bravery!", "", ""
	},
	{ "durability",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_durability, SKILL_NATURAL, 0, 24,
		"durability", "!durability!", "", ""
	},
//
	{ "frozen blood",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_frozen_blood, SKILL_NATURAL, 0, 50,
		"frozen blood", "!FrozenBlood!", "", ""
	},
	{ "icebound",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_icebound, SKILL_NATURAL, 0, 24,
		"icebound", "!IceBound!", "", ""
	},
//
	{ "hydrobiology",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_hydrobiology, SKILL_NATURAL, 0, 24,
		"swim", "!swim!", "", ""
	},
	{ "thalassic aura",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_thalassic_aura, SKILL_NATURAL, 0, 24,
		"", "!thalassicAura!", "", ""
	},
//
	{ "disappear",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_disappear, SKILL_NATURAL, 0, 12,
		"disappear", "!disappear!", "", ""
	},
	{ "resilience",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_resilience, SKILL_NATURAL, 0, 24,
		"resilience", "!Resilience!", "", ""
	},
//
	{ "amiability",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_amiability, SKILL_NATURAL, 0, 24,
		"", "!amiability!", "", ""
	},
	{ "drunken fighting",{},{},{},{},
		spell_null, TAR_IGNORE, POS_STANDING, &gsn_drunkfighting, SKILL_NATURAL, 0, 12,
		"", "", "", ""
	},
//

	{ "regeneration",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_regeneration, SKILL_NATURAL, 0, 24,
		"regeneration", "!regeneration!", "", ""
	},
	{ "return",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_return, SKILL_NATURAL, 0, 24,
		"returned damage", "!return!", "", ""
	},
//
	{ "fervor",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_fervor, SKILL_NATURAL, 0, 12,
		"fervor", "!fervor!", "", ""
	},
	{ "warcry",{},{},{},{},
		spell_null, TAR_IGNORE, POS_FIGHTING, &gsn_warcry, SKILL_NATURAL, 0, 24,
		"warcry", "!Warcry!", "", ""
	},
//
	{ "daurin",{},{},{},{},//nahlmey
		spell_null, TAR_IGNORE, POS_RESTING, &gsn_daurin, SKILL_NATURAL, 0,0,
		"!daurin!", "!daurin!", "!daurin!", "!daurin!"
	},
	{ NULL,{},{},{},{},spell_null,TAR_IGNORE,POS_RESTING,NULL,SKILL_NATURAL,0,0,NULL,NULL,NULL,NULL}
};

struct sgroup_type group_table[MAX_GROUP] ={
	{	"null default",			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{}},
	{	"basics blank",			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{}},
	{	"basics tenderfoot",	{0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{"club","recall"}},
	{	NULL,					{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},{}}
};
