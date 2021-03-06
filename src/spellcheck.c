// If you have any problems with this please instant message me at jeffreybasurto or
// send me an email at auadmin@feltain.org
#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <aspell.h>
#include "merc.h"



char *one_word(char*,char*);
// You need to compile with the aspell library linking. In your
// make file make sure it has -laspell
//
// Otherwise you get undefined value errors!

AspellConfig *spell_config;


// We'll set up our aspell checkers. We need to call this function
// once and only once per boot. Add it in boot_db() in most muds.
void init_aspell() {
    //A new configuration class. It's global.
    // I'm quite sure we won't be freeing it.
    spell_config = new_aspell_config();

    // Set up our dictionary settings. English US class dictionary.
    aspell_config_replace(spell_config, "lang", "en_US.multi");
}

// new_checker() creates an spellchecking object with unique settings and
// library. You possibly may want to manage quarks for each user in what ever
// application you use this in. For what I'm using it in we only need one
// generic checker! It's only called once.
AspellSpeller *new_checker() {
    AspellCanHaveError * possible_err = new_aspell_speller(spell_config);
    AspellSpeller * spell_checker = 0;

    if (aspell_error_number(possible_err) != 0)
        log_f((char*)aspell_error_message(possible_err));
    else
        spell_checker = to_aspell_speller(possible_err);


    return spell_checker;
}
// This function takes a string and returns an array of strings. Each string is a suggestion.
// The array is null terminated just like a string. max_return is the number of suggestions
// you want to be returned max. You must free each string returned along with the array.

char **spell_check(char *needle, int max_return) {
    int correct;
    static AspellSpeller *spell_checker;
    int count = 0;
    char **strings = (char **) malloc(sizeof(char *) * (max_return+1));


    strings[max_return] = 0;
    strings[0] = 0;

    if (!spell_checker)
        spell_checker = new_checker();

    // negative 1 if it is NULL terminated. Finds if it is true or not.
    correct = aspell_speller_check(spell_checker, needle, -1);

    // If not correct let's build a list of possible words.
    if (!correct) {
        const AspellWordList * suggestions = aspell_speller_suggest(spell_checker, needle, -1);
        AspellStringEnumeration * elements = aspell_word_list_elements(suggestions);
        const char * word;


        while ((word = aspell_string_enumeration_next(elements)) != NULL) {
            // Add an entire word to our list.
            strings[count] = strdup(word);
            ++count;
            if (count >= max_return)
                break;
        }
        strings[count] = 0;
        delete_aspell_string_enumeration(elements);
    }
    return strings;
}

// Checks a string and returns a list of all errors in the entire string.
// Good for descriptions and such.
// string_spellcheck(ch, some_String, false);
void string_spellcheck(CHAR_DATA *ch, char *pString, bool replace) {
    int errors = 0;
    char word[MSL];
    char **words;
    int i;

	if (!pString[0]){
		ch->send("No string to check.\n\r");
		return;
	}

    while(1) {
       pString = one_word(pString, word);
       if (!word[0])
           break;

       words = spell_check(word, 2);

       if (words[0]) {
           if (errors == 0)
               send_to_char("Wrong words:", ch);
           printf_to_char(ch, " %s", word);
           ++errors;
       }
    }
    if (errors) {
        send_to_char("\r\n", ch);
        printf_to_char(ch, "%d errors found.\r\n", errors);
    }
    else
        send_to_char("All words correct.\r\n", ch);

    for(i = 0;words[i];++i)
        free(words[i]);
    free(words);

}


// Command function to spellcheck a word.
void do_spellcheck(CHAR_DATA *ch, char *argument) {
    char **words;
    char arg1[MSL], arg2[MSL];
    int i;
    argument = one_argument (argument, arg1);
    argument = one_argument(argument, arg2);
    if (!arg1[0]) {
        send_to_char("Spellcheck what word?\r\n", ch);
        return;
    }

    if (!strcmp(arg1, "room")) {
        string_spellcheck(ch, ch->in_room->description, false);
        return;
    }
    if (arg2[0] && is_number(arg2))
        words = spell_check(arg1, atoi(arg2));
    else
        words = spell_check(arg1, 3);

    if (!words[0]) {
        send_to_char("That's the right spelling.\r\n", ch);
        return;
    }

    send_to_char("That's not a word.  Do you mean one of these?\r\n", ch);
    for(i = 0;words[i];++i) {
        printf_to_char(ch, "%s\n\r", words[i]);
        free(words[i]);
    }
    free(words);
}

// Epp. Forgot to include this at first. It's like one_arg() but finds words based on being
// in a sentence format. Good for parsing each word in a sentence...like in a spellchecker.
char *one_word (char *argument, char *arg_first) {

    while (isspace (*argument) || *argument == '.' || *argument == ',' || *argument == '!'
           || *argument == '\n' || *argument == '\r')
        argument++;

    while (*argument != '\0') {
        if(isspace(*argument)|| *argument == '.' || *argument == ','
                || *argument == '!' || *argument == '\n' || *argument == '\r')
            break;

        *arg_first = *argument;

        arg_first++;
        argument++;
    }

    *arg_first = '\0';

    while (isspace (*argument) || *argument == '.' || *argument == ','
            || *argument == '!' || *argument == '\n' || *argument == '\r')
        argument++;

    return argument;
}


