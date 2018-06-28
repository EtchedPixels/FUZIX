/*
  The MIT License

  Copyright (c) 2010 Brian L. Troutwine
  
  Marginally rearranged for Fuzix by Alan Cox 2018

  Permission is hereby granted, free of charge, to any person
  obtaining a copy of this software and associated
  documentation files (the "Software"), to deal in the
  Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software,
  and to permit persons to whom the Software is furnished to
  do so, subject to the following conditions:

  The above copyright notice and this permission notice shall
  be included in all copies or substantial portions of the
  Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
  KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
  WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
  PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
  OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
  OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
  OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <time.h>

#ifdef USE_FLOAT
#define RAND(N) (uint16_t)((double)rand() / ((double)RAND_MAX + 1) * N)
#else
#define RAND(N) int_random(N)
#endif

typedef struct _st city_st;
typedef enum _resp {
  OKAY,     /* Is there a problem here? There's no problem here. */
  EACREAGE, /* Insufficient acres in city. */
  EPOP,     /* Insufficient population in city. */
  EBUSHELS, /* Insufficient bushels in city storage. */
  ESTARVE   /* More than 45% of people starved in a year. */
} resp;

city_st *new_city_st(void);
void free_city_st(city_st *cty);
void add_migrants(city_st *cty);
void rats(city_st *cty);
uint16_t plague(city_st *cty);
uint16_t starvation(city_st *cty);
uint16_t births(city_st *cty);
resp step(city_st *cty);
resp buy_acres(city_st *cty, const uint16_t amnt);
resp sell_acres(city_st *cty, const uint16_t amnt);
resp feed_populace(city_st *cty, const uint16_t amnt);
resp plant_seed(city_st *cty, const uint16_t amnt);

uint16_t population(const city_st *cty);
uint16_t acres(const city_st *cty);
uint16_t yield(const city_st *cty);
uint16_t pests(const city_st *cty);
uint16_t bushels(const city_st *cty);
uint16_t trade_val(const city_st *cty);
uint16_t starved(const city_st *cty);
uint16_t migrated(const city_st *cty);
uint16_t tot_died(const city_st *cty);
uint16_t avg_starved(const city_st *cty);

struct _st {
  uint16_t population;
  uint16_t starved;
  uint16_t migrated;
  uint16_t bushels;
  uint16_t acres;
  uint16_t yield;
  uint16_t pests;
  uint16_t trade_val;
  uint16_t avg_starved;
  uint16_t tot_died;
  uint16_t fed;
  uint16_t planted;
};

uint16_t int_random(uint16_t n)
{
   uint16_t t = time(NULL);
   return ((rand() >> 4) ^ t) % n;
}

city_st *new_city_st(void) {
  city_st *cty = (city_st *) malloc(sizeof(city_st));

  cty->population  = 95;
  cty->migrated    = 5;
  cty->starved     = 0;
  cty->bushels     = 2800;
  cty->acres       = 1000;
  cty->yield       = 3;
  cty->pests       = 200;
  cty->trade_val   = 17+RAND(10);
  cty->avg_starved = 0;
  cty->tot_died    = 0;
  cty->fed         = 0;
  cty->planted     = 0;

  return cty;
}

void free_city_st(city_st *cty) {
  free(cty);
}

void add_migrants(city_st *cty) {
  cty->population += cty->migrated;
  cty->migrated = 0;
}

void rats(city_st *cty) {
  cty->pests = cty->bushels / (RAND(5)+2);
}

uint16_t plague(city_st *cty) {
  uint16_t died = cty->population / (RAND(4)+2);
  cty->population -= died;
  return died;
}

uint16_t starvation(city_st *cty) {
  /* 20 bushels of grain are needed for each citizen to
     live. */
  cty->starved = cty->population - (cty->fed/20);
  return cty->starved;
}

uint16_t births(city_st *cty) {
  /* For each two people living at the end of the year, one
     child is born. Carrying capacity of city is 6000. */
  uint16_t pop = cty->population;
  return pop*(1 - pop/6000);
}

resp step(city_st *cty) {
  uint16_t died = 0;
  died += starvation(cty);
  if (died > ((cty->population * 9) / 20))
    return ESTARVE;
  cty->avg_starved = (died + 9*cty->avg_starved)/10; // EMA

  if (RAND(15) == 0)
    died += plague(cty);
  cty->tot_died += died;

  cty->population += births(cty);

  cty->migrated  = RAND(cty->population)/10;
  cty->trade_val = 17+RAND(10);
  cty->yield = RAND(10)+1;

  cty->bushels += cty->yield*cty->planted;
  rats(cty);
  return OKAY;
}

resp buy_acres(city_st *cty, const uint16_t amnt) {
  if (cty->trade_val*amnt > cty->bushels)
    return EBUSHELS;
  cty->bushels -= (cty->trade_val)*amnt;
  cty->acres   += amnt;
  return OKAY;
}

resp sell_acres(city_st *cty, const uint16_t amnt) {
  if (amnt > cty->acres)
    return EACREAGE;
  cty->bushels += (cty->trade_val)*amnt;
  cty->acres   -= amnt;
  return OKAY;
}

resp feed_populace(city_st *cty, const uint16_t amnt) {
  if (amnt > cty->bushels)
    return EBUSHELS;
  cty->bushels -= amnt;
  cty->fed = amnt;
  return OKAY;
}

resp plant_seed(city_st *cty, const uint16_t amnt) {
  if (amnt > cty->acres)
    return EACREAGE;
  else if (amnt > cty->bushels)
    return EBUSHELS;
  else if (amnt > 10*cty->population)
    return EPOP;

  cty->bushels -= amnt;
  cty->planted = amnt;
  return OKAY;
}

uint16_t population(const city_st *cty) {
  return cty->population;
}

uint16_t acres(const city_st *cty) {
  return cty->acres;
}

uint16_t yield(const city_st *cty) {
  return cty->yield;
}

uint16_t pests(const city_st *cty) {
  return cty->pests;
}

uint16_t bushels(const city_st *cty) {
  return cty->bushels;
}

uint16_t trade_val(const city_st *cty) {
  return cty->trade_val;
}

uint16_t starved(const city_st *cty) {
  return cty->starved;
}

uint16_t migrated(const city_st *cty) {
  return cty->migrated;
}

uint16_t tot_died(const city_st *cty) {
  return cty->tot_died;
}

uint16_t avg_starved(const city_st *cty) {
  return cty->avg_starved;
}

uint16_t input(void);

void retire(void);
void storm_out(void);
void ejected(void);
void game_end(const city_st *cty);
void nero_end(void);
void not_so_bad_end(const city_st *cty);

uint16_t input(void) {
  char input[16], *endptr;
  uint16_t q;
  if (fgets(input, 16, stdin) == NULL)
   exit(EXIT_FAILURE);
  errno = 0;
  q = (uint16_t) strtol(input, &endptr, 10);
  if ((errno == ERANGE && (q == LONG_MAX || q == LONG_MIN))
      || (errno != 0 && q == 0)) {
    perror("strtol");
    exit(EXIT_FAILURE);
  }
  if (endptr == input) {
    fprintf(stderr, "No digits were found\n");
    exit(EXIT_FAILURE);
  }
  return q;
}

/*
   Client functions

   Someday I dream of being in curses.
 */
void retire(void) {
  printf("So long for now.\n");
  exit(EXIT_SUCCESS);
}

void storm_out(void) {
  printf("Hamurabi: I cannot do what you wish.\n");
  printf("Get yourself another steward!!!!!\n");
  retire();
}

void ejected(void) {
  printf("Due to this extreme mismanagement you have not only\n");
  printf("been impeached and thrown out of office but you have\n");
  printf("also been declared 'National Fink' !!\n");
  retire();
}

void game_end(const city_st *cty) {
  uint16_t l = (uint16_t) (acres(cty)/(population(cty)));
  printf("In your 10-year term of office %d percent of the\n",
         avg_starved(cty));
  printf("population starved per year on average, i.e., a total of\n");
  printf("%d people died!!\n\n", tot_died(cty));
  printf("You started with 10 acres per person and ended with\n");
  printf("%d acres per person.\n\n", l);

  if ((avg_starved(cty)>33) || (l<7)) ejected();
  if ((avg_starved(cty)>10) || (l<9)) nero_end();
  if ((avg_starved(cty)>3)  || (l<10)) not_so_bad_end(cty);
}

void nero_end(void) {
  printf("Your heavy handed performance smacks of Nero and Ivan IV.\n");
  printf("The people (remaining) find you an unpleasant ruler, and,\n");
  printf("frankly, hate your guts!\n");
  retire();
}

void not_so_bad_end(const city_st *cty) {
  printf("Your performance could have been somewhat better, but\n");
  printf("really wasn't too bad at all. ");
  printf("%d people would ", RAND(population(cty)));
  printf("dearly like to see you assassinated but we all have our\n");
  printf("trivial problems.\n");
  retire();
}

int main(void) {

  city_st *cty = new_city_st();
  uint16_t inp, year;

  puts("Try your hand at governing Ancient Sumeria\n"
       "successfully for a 10 year term of office.\n");
  for (year = 0; year < 11; year++) {
    printf("Hamurabi: I beg to report to you,\nin year %d, ", year);
    printf("%d people starved %d came to the city.\n",
           starved(cty), migrated(cty));

    add_migrants(cty);

    if (RAND(2) == 1) {
      printf("A horrible plague struck! Many people have died!\n");
      plague(cty);
    }

    printf("\nPopulation is now %d\n", population(cty));
    printf("The city now owns %d acres\n", acres(cty));
    printf("You have harvested %d bushels per acre.\n",
           yield(cty));
    printf("Rats ate %d bushels.\n\n",
           pests(cty));
    printf("You now have %d bushels in store.\n\n",
           bushels(cty));
    printf("Land is trading at %d bushels per acre.\n",
           trade_val(cty));
  buy_acres:
    printf("How many acres do you wish to buy?\n");
    inp = input();
    if (inp < 0) storm_out();
    else if (inp == 0) goto sell_acres;
    if (buy_acres(cty, inp) != 0) {
      printf("Hamurabi: Think again. You have only %d",
             bushels(cty));
      printf(" bushels of grain. Now then,\n");
      goto buy_acres;
    }
    goto feed_people;

  sell_acres:
    printf("How many acres do you wish to sell?\n");
    inp = input();
    if (inp < 0) storm_out();
    else if (inp == 0) goto feed_people;
    if (sell_acres(cty, inp) != 0) {
      printf("Hamurabi: Think again. You have only %d acres.",
             acres(cty));
      printf(" Now then,\n");
      goto sell_acres;
    }

  feed_people:
    printf("How many bushels do you wish to feed your people?\n");
    printf("(No starvation: %d bushels.)\n",
           population(cty)*20);
    inp = input();
    if (inp < 0) storm_out();
    else if (inp == 0) goto plant_seed;
    if (feed_populace(cty, inp) != 0) {
      printf("Hamurabi: Think again. We only have %d bushels.",
             bushels(cty));
      printf(" Now then,\n");
      goto feed_people;
    }

  plant_seed:
    printf("How many acres do you wish to plant with seed?\n");
    printf("(MAX: %d acres)\n", 10*population(cty));
    inp = input();
    if (inp < 0) storm_out();
    else if (inp == 0) goto progress;
    switch(plant_seed(cty, inp)) {
    case EACREAGE: {
      printf("Hamurabi: Think again. You have only %d acres.",
             acres(cty));
      printf(" Now then,\n");
      goto plant_seed;
    } break;
    case EPOP: {
      printf("But you only have %d people to tend the fields.",
             population(cty));
      printf(" Now then,\n");
      goto plant_seed;
    } break;
    case EBUSHELS: {
      printf("Hamurabi: Think again. We only have %d bushels.",
             bushels(cty));
      printf(" Now then,\n");
      goto plant_seed;
    } break;
    case ESTARVE:
    case OKAY: break;
    }

  progress:
    if (step(cty) == ESTARVE) {
      printf("You starved %d people in one year!!!\n",
             starved(cty));
      ejected();
    }

  }

  game_end(cty);
  free_city_st(cty);
}
