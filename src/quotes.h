#ifndef _QUOTES_H_
#define _QUOTES_H_
/***********************************************************************
 * Quote was written by Robbert of The Inquisition MUD.  Use of this   *
 * code is held to the same licensing agreements of ROM, Diku, et al.  *
 * If you use this code, either in its entirety or as a basis for      *
 * something of your own, this header must be included with the file.  *
 ***********************************************************************/ 

//PEW PEW
void do_Qquote(CHAR_DATA *ch){
  int num;

  num = number_range(1,11);

  switch(num)
  {
    case 1:
        send_to_char("'The most important scientific revolutions all include, as their only common feature, the dethronement of human arrogance from one pedestal after another of previous convictions about our centrality in the cosmos.'\n\r-Stephen Jay Gould\n\r", ch);
        break;
    case 2:
        send_to_char("'Nearly all men can stand adversity, but if you want to test a man's character, give him power.'\n\r-Abraham Lincoln\n\r", ch);
        break;
    case 3:
        send_to_char("'Last night I dreamed I ate a ten-pound marshmallow, and when I woke up the pillow was gone.'\n\r-Tommy Cooper\n\r", ch);
        break;
    case 4:
        send_to_char("'In Paris they simply stared when I spoke to them in French; I never did succeed in making those idiots understand their language.'\n\r-Mark Twain\n\r", ch);
        break;
    case 5:
        send_to_char("'Last night I stayed up late playing poker with tarot cards, I got a full house and four people died.'\n\r-Steven Wright\n\r", ch);
        break;
    case 6:
        send_to_char("'Electricity is actually made up of extremely tiny particles called electrons, that you cannot see with the naked eye unless you have been drinking.'\n\r-Dave Barry\n\r", ch);
        break;
    case 7:
        send_to_char("'I don't know anything about music. In my line you don't have to.'\n\r-Elvis Presley\n\r", ch);
        break;
    case 8:
        send_to_char("'Whatever you do will be insignificant, but it is very important that you do it.'\n\r-Mahatma Gandhi\n\r", ch);
        break;
    case 9:
        send_to_char("'You have to know how to accept rejection and reject acceptance.'\n\r-Ray Bradbury\n\r", ch);
        break;
    case 10:
        send_to_char("'Indeed, history is nothing more than a tableu of crimes and misfortunes.'\n\r-Voltaire\n\r", ch);
        break;
    case 11:
        send_to_char("'Somebody forgot to turn off my program. I'm a doctor, not a lightbulb.'\n\r-Robert Picardo\n\r", ch);
        break;
    }

  return;
}
#endif

