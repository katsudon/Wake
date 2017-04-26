#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "interp.h"

/*
 * Local functions.
 */
enum{ CRAFT_INGOT = 0
};
struct craft_type{
	int ctype;//gather,modify,combine,embellish
};
