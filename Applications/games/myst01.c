#define NUM_OBJ 49
#define WORDSIZE 4
#define GAME_MAGIC 463
#include <stdint.h>

struct location {
  const char *text;
  uint8_t exit[6];
};

const char toomuch[] = { "I am carrying too much. " };
const char dead[] = { "I am dead.\n" };
const char stored_msg[] = { "I have stored " };
const char stored_msg2[] = { " treasures. On a scale of 0 to 100, that rates " };
const char dotnewline[] = { ".\n" };
const char newline[] = { "\n" };
const char carrying[] = { "I am carrying:\n" };
const char dashstr[] = { " - " };
const char nothing[] = { "nothing" };
const char lightout[] = { "My light has run out." };
const char lightoutin[] = { "My light runs out in " };
const char turns[] = { "turns" };
const char turn[] = { "turn" };
const char whattodo[] = { "\nTell me what to do ? " };
const char prompt[] = { "\n> " };
const char dontknow[] = { "You use word(s) I don't know! " };
const char givedirn[] = { "Give me a direction too. " };
const char darkdanger[] = { "Dangerous to move in the dark! " };
const char brokeneck[] = { "I fell down and broke my neck. " };
const char cantgo[] = { "I can't go in that direction. " };
const char dontunderstand[] = { "I don't understand your command. " };
const char notyet[] = { "I can't do that yet. " };
const char beyondpower[] = { "It is beyond my power to do that. " };
const char okmsg[] = { "O.K. " };
const char whatstr[] = { "What ? " };
const char itsdark[] = { "I can't see. It is too dark!" };
const char youare[] = { "I am in a " };
const char nonestr[] = { "none" };
const char obexit[] = { "\nObvious exits: " };
const char canalsosee[] = { "I can also see: " };
const char playagain[] = { "Do you want to play again Y/N: " };
const char invcond[] = { "INVCOND" };
const char *exitmsgptr[] = {
  "North",
  "South",
  "East",
  "West",
  "Up",
  "Down"
};



/*
 * 
 *	Game database follows below. Although linked into the same asm
 *	file to make life easier this is just "mere aggregation" for
 *	convenience, due to limits in the tool chain and the game licence
 *	not the GPL applies to the game database.
 */
const uint8_t startlamp = 200;
const uint8_t lightfill = 200;
const uint8_t startcarried = 1;
const uint8_t maxcar = 6;
const uint8_t treasure = 0;
const uint8_t treasures = 0;
const uint8_t lastloc = 31;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"\x28\x43\x29\x20\x31\x39\x38\x32\x20\x42\x2E\x48\x2E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x64\x65\x6E\x73\x65\x20\x53\x50\x4F\x4F\x4B\x59\x20\x46\x6F\x72\x65\x73\x74",
 { 2, 4, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x20\x53\x74\x72\x65\x61\x6D",
 { 6, 1, 0, 3, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x20\x54\x72\x65\x65",
 { 5, 0, 2, 0, 0, 0 } }, 
		{ 	"\x74\x61\x6E\x67\x6C\x65\x20\x6F\x66\x20\x50\x52\x49\x43\x4B\x4C\x59\x20\x42\x72\x69\x61\x72\x73",
 { 1, 0, 0, 0, 0, 0 } }, 
		{ 	"\x63\x6C\x65\x61\x72\x69\x6E\x67\x20\x62\x79\x20\x61\x20\x43\x61\x62\x69\x6E",
 { 0, 3, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x20\x50\x61\x74\x68",
 { 0, 2, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x46\x6F\x72\x65\x73\x74\x20\x72\x6F\x61\x64",
 { 8, 6, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x46\x6F\x72\x65\x73\x74\x20\x72\x6F\x61\x64",
 { 9, 7, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x20\x50\x6F\x6E\x64",
 { 10, 8, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x61\x20\x43\x61\x73\x74\x6C\x65\x20\x4D\x6F\x61\x74",
 { 0, 9, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x61\x20\x50\x6F\x72\x74\x63\x75\x6C\x6C\x69\x73",
 { 0, 10, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x6F\x70\x20\x74\x68\x65\x20\x42\x61\x74\x74\x6C\x65\x6D\x65\x6E\x74\x73",
 { 0, 11, 0, 0, 0, 13 } }, 
		{ 	"\x43\x6F\x75\x72\x74\x79\x61\x72\x64\x2E\x20\x49\x20\x73\x65\x65\x20\x61\x6E\x20\x41\x72\x63\x68\x77\x61\x79",
 { 0, 0, 0, 0, 12, 0 } }, 
		{ 	"\x53\x74\x72\x61\x77\x20\x73\x74\x72\x65\x77\x6E\x20\x73\x68\x65\x64",
 { 0, 0, 15, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x61\x20\x48\x75\x67\x65\x20\x64\x6F\x6F\x72",
 { 0, 13, 16, 14, 0, 0 } }, 
		{ 	"\x64\x65\x72\x65\x6C\x69\x63\x74\x20\x53\x74\x61\x62\x6C\x65",
 { 0, 0, 0, 15, 0, 0 } }, 
		{ 	"\x72\x6F\x6F\x6D\x20\x62\x65\x66\x6F\x72\x65\x20\x61\x6E\x20\x6F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
 { 0, 0, 0, 0, 14, 0 } }, 
		{ 	"\x54\x6F\x72\x74\x75\x72\x65\x20\x43\x68\x61\x6D\x62\x65\x72",
 { 26, 27, 0, 17, 0, 0 } }, 
		{ 	"\x43\x61\x62\x69\x6E\x20\x77\x69\x74\x68\x20\x68\x6F\x6C\x65\x20\x69\x6E\x20\x66\x6C\x6F\x6F\x72",
 { 0, 0, 0, 5, 0, 20 } }, 
		{ 	"\x43\x61\x76\x65",
 { 24, 22, 0, 0, 19, 0 } }, 
		{ 	"\x43\x61\x76\x65",
 { 20, 0, 0, 0, 0, 0 } }, 
		{ 	"\x43\x61\x76\x65\x20\x62\x65\x66\x6F\x72\x65\x20\x61\x20\x70\x61\x64\x6C\x6F\x63\x6B\x65\x64\x20\x64\x6F\x6F\x72",
 { 21, 0, 0, 0, 0, 0 } }, 
		{ 	"\x53\x74\x6F\x72\x65\x20\x72\x6F\x6F\x6D",
 { 22, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x20\x76\x61\x73\x74\x20\x4C\x61\x6B\x65",
 { 0, 20, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x73\x61\x69\x6C\x69\x6E\x67\x20\x74\x68\x65\x20\x4C\x61\x6B\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x4D\x61\x69\x64\x27\x73\x20\x72\x6F\x6F\x6D",
 { 0, 18, 0, 0, 0, 0 } }, 
		{ 	"\x53\x6F\x72\x63\x65\x72\x65\x72\x27\x73\x20\x77\x6F\x72\x6B\x72\x6F\x6F\x6D",
 { 18, 0, 28, 0, 0, 0 } }, 
		{ 	"\x62\x61\x72\x65\x20\x73\x6C\x69\x6D\x79\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 27, 0, 0 } }, 
		{ 	"\x63\x6F\x6C\x64\x20\x72\x6F\x6F\x6D\x20\x66\x75\x6C\x6C\x20\x6F\x66\x20\x53\x74\x61\x74\x75\x65\x73",
 { 0, 15, 0, 0, 0, 0 } }, 
		{ 	"\x54\x72\x65\x65",
 { 0, 0, 0, 0, 0, 3 } }, 
		{ 	"\x4D\x45\x53\x53\x21\x21\x20\x20\x49\x27\x6D\x20\x44\x45\x41\x44\x21",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	1,
	19,
	18,
	0,
	0,
	24,
	16,
	23,
	0,
	0,
	0,
	29,
	13,
	7,
	16,
	0,
	27,
	26,
	1,
	0,
	0,
	255,
	0,
	0,
	14,
	6,
	21,
	29,
	28,
	19,
	22,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	30,
	0,
	0,
};


const char *objtext[] = {
	"\x53\x68\x69\x6E\x79\x20\x53\x77\x6F\x72\x64",
	"\x4F\x6C\x64\x20\x43\x6C\x6F\x61\x6B",
	"\x42\x61\x72\x72\x65\x6C",
	"\x48\x61\x6D\x6D\x65\x72",
	"\x53\x61\x6C\x74",
	"\x4B\x65\x79",
	"\x48\x75\x67\x65\x20\x79\x65\x6C\x6C\x6F\x77\x20\x43\x72\x61\x62",
	"\x48\x75\x6E\x74\x69\x6E\x67\x20\x48\x6F\x72\x6E",
	"\x52\x61\x66\x74",
	"\x4C\x49\x54\x20\x4F\x69\x6C\x20\x4C\x61\x6D\x70",
	"\x4A\x65\x77\x65\x6C\x6C\x65\x64\x20\x4B\x6E\x69\x66\x65",
	"\x47\x4F\x4C\x44\x45\x4E\x20\x42\x41\x54\x4F\x4E",
	"\x50\x61\x72\x63\x68\x6D\x65\x6E\x74",
	"\x4B\x6E\x69\x67\x68\x74\x20\x69\x6E\x20\x64\x61\x72\x6B\x20\x41\x72\x6D\x6F\x75\x72",
	"\x57\x6F\x6F\x64\x65\x6E\x20\x53\x74\x61\x66\x66",
	"\x4F\x6C\x64\x20\x48\x65\x6C\x6D\x65\x74",
	"\x47\x6F\x6C\x64\x65\x6E\x20\x52\x69\x6E\x67",
	"\x47\x4C\x4F\x57\x49\x4E\x47\x20\x51\x75\x61\x72\x74\x7A",
	"\x4D\x69\x72\x72\x6F\x72",
	"\x52\x6F\x74\x74\x69\x6E\x67\x20\x6C\x65\x61\x76\x65\x73",
	"\x50\x6F\x6F\x6C\x20\x6F\x66\x20\x4F\x69\x6C",
	"\x48\x6F\x6C\x65\x20\x69\x6E\x20\x46\x6C\x6F\x6F\x72",
	"\x4D\x61\x74\x63\x68\x65\x73",
	"\x57\x45\x54\x20\x4D\x61\x74\x63\x68\x65\x73",
	"\x43\x6F\x69\x6C\x20\x6F\x66\x20\x52\x6F\x70\x65",
	"\x4F\x69\x6C\x20\x4C\x61\x6D\x70",
	"\x53\x61\x76\x61\x67\x65\x20\x57\x6F\x6C\x66",
	"\x48\x75\x67\x65\x20\x53\x6C\x75\x67\x73",
	"\x48\x69\x64\x65\x6F\x75\x73\x20\x47\x6F\x72\x67\x6F\x6E",
	"\x55\x67\x6C\x79\x20\x4C\x69\x7A\x61\x72\x64\x2D\x6D\x61\x6E",
	"\x4F\x69\x6C\x20\x73\x6F\x64\x64\x65\x6E\x20\x52\x61\x67",
	"\x50\x61\x64\x6C\x6F\x63\x6B",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x50\x61\x64\x6C\x6F\x63\x6B",
	"\x44\x72\x79\x20\x52\x61\x67",
	"\x43\x6C\x6F\x61\x6B\x20\x28\x77\x6F\x72\x6E\x29",
	"\x48\x65\x6C\x6D\x65\x74\x20\x28\x77\x6F\x72\x6E\x29",
	"\x52\x69\x6E\x67\x20\x28\x77\x6F\x72\x6E\x29",
	"\x4C\x75\x6D\x70\x20\x6F\x66\x20\x51\x75\x61\x72\x74\x7A",
	"\x48\x61\x6E\x67\x69\x6E\x67\x20\x52\x6F\x70\x65",
	"\x45\x6D\x70\x74\x79\x20\x4C\x61\x6D\x70",
	"\x44\x65\x61\x64\x20\x57\x6F\x6C\x66",
	"\x53\x61\x6C\x74\x65\x64\x20\x53\x6C\x75\x67\x73",
	"\x47\x6F\x72\x67\x6F\x6E\x20\x28\x73\x74\x6F\x6E\x65\x29",
	"\x44\x65\x61\x64\x20\x4C\x69\x7A\x61\x72\x64\x2D\x6D\x61\x6E",
	"\x4F\x70\x65\x6E\x20\x44\x6F\x6F\x72",
	"\x20",
	"\x48\x6F\x6C\x6C\x6F\x77\x20\x69\x6E\x20\x54\x72\x75\x6E\x6B",
	"\x48\x6F\x6C\x65\x20\x69\x6E\x20\x66\x6C\x6F\x6F\x72",
	"\x47\x72\x61\x69\x6E\x73\x20\x6F\x66\x20\x53\x61\x6C\x74",
};
const char *msgptr[] = {
	"\x20",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x4D\x59\x53\x54\x45\x52\x49\x4F\x55\x53\x20\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x53",
	"\x50\x72\x65\x70\x61\x72\x65\x20\x79\x6F\x75\x72\x73\x65\x6C\x66\x20\x66\x6F\x72\x20\x61\x6E\x79\x74\x68\x69\x6E\x67\x21",
	"\x54\x68\x69\x73\x20\x69\x73\x27\x6E\x74\x20\x61\x20\x46\x6F\x6F\x74\x62\x61\x6C\x6C\x20\x47\x61\x6D\x65",
	"\x54\x72\x79\x20\x45\x58\x41\x4D\x69\x6E\x69\x6E\x67\x20\x74\x68\x69\x6E\x67\x73\x2E\x2E",
	"\x43\x72\x61\x62\x20\x65\x61\x74\x73\x20\x53\x6C\x75\x67\x73\x20\x74\x68\x65\x6E\x20\x69\x67\x6E\x6F\x72\x65\x73\x20\x6D\x65\x21",
	"\x48\x65\x27\x73\x20\x65\x61\x74\x65\x6E\x21",
	"\x49\x74\x27\x73\x20\x74\x72\x6F\x64\x64\x65\x6E\x20\x69\x6E\x74\x6F\x20\x74\x68\x65\x20\x67\x72\x6F\x75\x6E\x64\x21",
	"\x48\x65\x20\x6F\x6E\x6C\x79\x20\x77\x61\x6E\x74\x73\x20\x74\x6F\x20\x65\x61\x74\x20\x4D\x45\x21",
	"\x43\x72\x61\x62\x73\x20\x6C\x69\x6B\x65\x20\x53\x61\x6C\x74\x65\x64\x20\x53\x6C\x75\x67\x73",
	"\x54\x68\x65\x20\x43\x72\x61\x62\x20\x73\x74\x6F\x70\x73\x20\x6D\x65\x21",
	"\x49\x20\x6E\x65\x65\x64\x20\x61\x20\x73\x68\x61\x72\x70\x20\x6F\x62\x6A\x65\x63\x74",
	"\x49\x20\x6E\x65\x65\x64\x20\x4D\x61\x67\x69\x63\x20\x68\x65\x72\x65",
	"\x53\x6D\x61\x73\x68\x20\x74\x68\x65\x20\x4C\x6F\x63\x6B\x21",
	"\x54\x72\x79\x20\x74\x68\x65\x20\x50\x61\x72\x63\x68\x6D\x65\x6E\x74",
	"\x4D\x61\x79\x62\x65\x20\x74\x68\x65\x20\x43\x72\x61\x62\x20\x69\x73\x20\x68\x75\x6E\x67\x72\x79",
	"\x54\x68\x65\x20\x51\x75\x61\x72\x74\x7A\x20\x6D\x61\x79\x20\x62\x65\x20\x6F\x66\x20\x75\x73\x65",
	"\x48\x6F\x6C\x64\x20\x69\x74\x21\x20\x41\x20\x4D\x69\x72\x72\x6F\x72\x20\x74\x68\x61\x74\x20\x69\x73\x21",
	"\x54\x72\x79\x20\x47\x65\x73\x74\x75\x72\x65\x73\x20\x61\x6E\x64\x20\x4D\x61\x67\x69\x63",
	"\x57\x65\x61\x72\x20\x74\x68\x65\x20\x48\x65\x6C\x6D\x65\x74",
	"\x49\x74\x27\x73\x20\x6C\x6F\x63\x6B\x65\x64\x21",
	"\x49\x74\x27\x73\x20\x6F\x70\x65\x6E\x21",
	"\x20",
	"\x54\x68\x65\x20\x48\x61\x6E\x64\x20\x73\x74\x6F\x70\x73\x20\x6D\x6F\x76\x69\x6E\x67\x21",
	"\x49\x20\x73\x65\x65\x20\x77\x72\x69\x74\x69\x6E\x67",
	"\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x52\x75\x6E\x65\x73\x20\x6F\x6E\x20\x69\x74",
	"\x20",
	"\x49\x20\x73\x65\x65\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C",
	"\x4C\x69\x7A\x61\x72\x64\x2D\x6D\x61\x6E\x20\x73\x74\x61\x62\x73\x20\x6D\x65\x21",
	"\x4D\x61\x74\x63\x68\x65\x73\x20\x61\x72\x65\x20\x61\x63\x72\x6F\x73\x73\x20\x74\x68\x65\x20\x4D\x6F\x61\x74",
	"\x49\x74\x20\x63\x61\x74\x63\x68\x65\x73\x21\x20\x49\x74\x27\x73\x20\x73\x65\x63\x75\x72\x65",
	"\x4C\x61\x6D\x70\x20\x69\x73\x20\x6E\x6F\x77\x20\x4C\x49\x54",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x74\x6F\x20\x6C\x69\x67\x68\x74\x20\x69\x74\x20\x77\x69\x74\x68",
	"\x20",
	"\x49\x74\x27\x73\x20\x4C\x49\x54\x21",
	"\x57\x69\x74\x68\x20\x57\x45\x54\x20\x4D\x61\x74\x63\x68\x65\x73\x20\x3F",
	"\x49\x74\x27\x73\x20\x6E\x6F\x74\x20\x6C\x6F\x63\x6B\x65\x64\x21",
	"\x4C\x6F\x63\x6B\x20\x69\x73\x20\x74\x6F\x6F\x20\x72\x75\x73\x74\x79",
	"\x49\x27\x76\x65\x20\x6E\x6F\x20\x6B\x65\x79",
	"\x49\x74\x27\x73\x20\x6E\x6F\x74\x20\x6C\x69\x74\x21",
	"\x51\x75\x61\x72\x74\x7A\x20\x67\x6C\x6F\x77\x73\x20\x66\x69\x65\x72\x63\x65\x6C\x79",
	"\x54\x68\x65\x20\x64\x69\x6D\x73\x20\x61\x67\x61\x69\x6E\x21",
	"\x20",
	"\x20",
	"\x52\x75\x6E\x65\x73\x20\x73\x61\x79\x2E\x2E\x4D\x61\x67\x69\x63\x20\x77\x6F\x72\x64\x20\x41\x4B\x59\x52\x5A",
	"\x52\x75\x6E\x65\x73\x20\x61\x72\x65\x20\x75\x6E\x72\x65\x61\x64\x61\x62\x6C\x65",
	"\x53\x41\x49\x4C\x20\x4C\x41\x4B\x45\x2E\x2E\x42\x4C\x4F\x57\x20\x48\x4F\x52\x4E\x2E\x2E\x54\x48\x52\x2E\x2E\x54\x68\x61\x74\x73\x20\x61\x6C\x6C\x21",
	"\x20",
	"\x46\x75\x74\x69\x6C\x65\x21",
	"\x20",
	"\x57\x69\x74\x68\x20\x62\x61\x72\x65\x20\x68\x61\x6E\x64\x73\x20\x3F",
	"\x20",
	"\x20",
	"\x43\x6C\x69\x6D\x62\x69\x6E\x67\x20\x61\x20\x63\x6F\x69\x6C\x20\x6F\x66\x20\x72\x6F\x70\x65\x20\x69\x73\x20\x64\x69\x66\x66\x69\x63\x75\x6C\x74\x21",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x73",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x69\x73\x20\x68\x61\x70\x70\x65\x6E\x69\x6E\x67\x21",
	"\x51\x75\x61\x72\x74\x7A\x20\x65\x6D\x69\x74\x73\x20\x61\x20\x64\x61\x7A\x7A\x6C\x69\x6E\x67\x20\x62\x65\x61\x6D\x21",
	"\x49\x74\x20\x68\x69\x74\x73\x20\x74\x68\x65\x20\x4C\x69\x7A\x61\x72\x64\x2D\x6D\x61\x6E",
	"\x4F\x2E\x4B\x2E",
	"\x49\x20\x63\x61\x6E\x20\x6F\x6E\x6C\x79\x20\x6B\x69\x6C\x6C\x20\x68\x69\x6D\x20\x6F\x6E\x63\x65\x21",
	"\x54\x68\x72\x6F\x75\x67\x68\x20\x74\x68\x65\x20\x48\x65\x61\x72\x74\x21",
	"\x57\x69\x74\x68\x20\x62\x61\x72\x65\x20\x68\x61\x6E\x64\x73\x20\x3F",
	"\x57\x68\x79\x20\x64\x6F\x20\x74\x68\x61\x74\x20\x3F",
	"\x49\x20\x6E\x65\x65\x64\x20\x4F\x49\x4C",
	"\x49\x74\x27\x73\x20\x6E\x6F\x74\x20\x65\x6D\x70\x74\x79\x21",
	"\x49\x27\x6D\x20\x6E\x6F\x20\x41\x72\x73\x6F\x6E\x69\x73\x74\x21",
	"\x54\x68\x65\x20\x53\x6C\x75\x67\x73\x20\x73\x68\x72\x69\x76\x65\x6C\x20\x75\x70\x21",
	"\x48\x61\x6E\x64\x20\x6D\x6F\x76\x65\x73\x20\x61\x77\x61\x79\x21",
	"\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67\x20\x69\x74\x21",
	"\x4F\x55\x43\x48\x21\x20\x54\x68\x61\x74\x20\x77\x61\x73\x20\x48\x4F\x54\x21",
	"\x54\x72\x79\x20\x65\x78\x70\x6C\x6F\x72\x69\x6E\x67",
	"\x4F\x69\x6C\x20\x66\x6C\x61\x72\x65\x73\x20\x75\x70\x21",
	"\x52\x61\x67\x20\x69\x73\x20\x74\x6F\x6F\x20\x64\x72\x79\x21",
	"\x41\x6C\x6C\x20\x6F\x76\x65\x72\x20\x74\x68\x65\x20\x46\x6C\x6F\x6F\x72\x21",
	"\x4C\x61\x6D\x70\x20\x69\x73\x20\x6E\x6F\x77\x20\x66\x75\x6C\x6C",
	"\x20",
	"\x41\x20\x6B\x65\x79\x20\x4D\x61\x74\x65\x72\x69\x61\x6C\x69\x73\x65\x73\x21",
	"\x54\x68\x65\x20\x77\x61\x74\x65\x72\x73\x20\x72\x69\x70\x70\x6C\x65\x21",
	"\x41\x20\x48\x61\x6E\x64\x20\x72\x69\x73\x65\x73\x21",
	"\x49\x74\x27\x73\x20\x69\x6D\x70\x6F\x73\x73\x69\x62\x6C\x65",
	"\x49\x74\x27\x73\x20\x74\x6F\x6F\x20\x73\x6D\x61\x6C\x6C\x21",
	"\x41\x20\x47\x65\x6E\x69\x65\x20\x61\x70\x70\x65\x61\x72\x73\x2E\x2E\x77\x61\x76\x65\x73\x20\x68\x69\x73\x20\x68\x61\x6E\x64",
	"\x20",
	"\x53\x61\x79\x73\x20\x4D\x61\x74\x63\x68\x65\x73\x20\x61\x72\x65\x20\x64\x72\x79\x20\x74\x68\x65\x6E\x20\x76\x61\x6E\x69\x73\x68\x65\x73\x21",
	"\x53\x6F\x72\x72\x79",
	"\x47\x6F\x72\x67\x6F\x6E\x27\x73\x20\x73\x74\x61\x72\x65\x20\x74\x75\x72\x6E\x73\x20\x6D\x65\x20\x74\x6F\x20\x73\x74\x6F\x6E\x65\x21",
	"\x57\x45\x4C\x4C\x20\x44\x4F\x4E\x45\x21\x20\x59\x6F\x75\x20\x68\x61\x76\x65\x20\x73\x75\x63\x63\x65\x65\x64\x65\x64\x21",
	"\x59\x6F\x75\x20\x61\x72\x65\x20\x61\x20\x48\x65\x72\x6F\x21",
	"\x57\x6F\x6C\x66\x20\x73\x74\x6F\x70\x73\x20\x6D\x65\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x48\x6F\x6C\x6C\x6F\x77\x20\x68\x65\x72\x65\x21",
	"\x4C\x61\x6D\x70\x20\x69\x73\x20\x6E\x6F\x77\x20\x6F\x75\x74",
	"\x49\x20\x63\x61\x6E\x27\x74\x20\x65\x76\x65\x6E\x20\x74\x6F\x75\x63\x68\x20\x68\x69\x6D\x21",
	"\x44\x6F\x6F\x72\x20\x63\x72\x65\x61\x6B\x73\x20\x6F\x70\x65\x6E\x2E\x2E",
	"\x44\x6F\x6F\x72\x20\x69\x73\x20\x6E\x6F\x77\x20\x75\x6E\x6C\x6F\x63\x6B\x65\x64",
	"\x4B\x6E\x69\x67\x68\x74\x20\x73\x74\x6F\x70\x73\x20\x6D\x65\x21",
	"\x48\x6D\x6D\x2E\x2E\x4E\x69\x63\x65\x20\x6D\x65\x6C\x6C\x6F\x77\x20\x73\x6F\x75\x6E\x64\x21",
	"\x48\x6F\x77\x20\x72\x65\x76\x6F\x6C\x74\x69\x6E\x67\x21",
	"\x47\x6F\x20\x77\x61\x73\x68\x20\x79\x6F\x75\x72\x20\x6D\x6F\x75\x74\x68\x20\x6F\x75\x74\x21",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x65\x64\x21",
};


const uint8_t status[] = {
	171, 
	9, 1, 0, 1, 
	58, 1, 86, 2, 
	175, 
	4, 29, 9, 2, 2, 28, 
	64, 88, 136, 61, 
	165, 
	4, 20, 
	56, 64, 
	165, 
	4, 17, 
	56, 64, 
	165, 
	4, 19, 
	57, 64, 
	165, 
	4, 14, 
	57, 64, 
	165, 
	4, 24, 
	57, 64, 
	166, 
	1, 11, 
	137, 138, 63, 
	183, 
	4, 29, 8, 2, 2, 28, 0, 28, 0, 42, 
	88, 149, 59, 53, 
	177, 
	8, 16, 9, 11, 0, 39, 0, 11, 
	53, 58, 
	169, 
	1, 17, 0, 17, 
	120, 53, 
	178, 
	4, 24, 2, 41, 0, 7, 0, 41, 
	58, 59, 5, 
	164, 
	4, 31, 
	63, 
};
const uint8_t actions[] = {
	9, 1, 41, 
	4, 5, 0, 19, 
	54, 64, 
	12, 1, 65, 
	4, 13, 1, 34, 0, 15, 
	54, 
	8, 1, 65, 
	4, 13, 6, 34, 
	145, 
	17, 26, 16, 
	3, 25, 1, 22, 0, 9, 0, 25, 
	72, 32, 
	8, 26, 16, 
	3, 25, 6, 22, 
	33, 
	8, 26, 16, 
	3, 25, 1, 23, 
	36, 
	17, 15, 66, 
	4, 14, 14, 47, 0, 47, 0, 14, 
	62, 27, 
	12, 1, 26, 
	4, 14, 2, 47, 0, 17, 
	54, 
	15, 28, 16, 
	1, 9, 0, 9, 0, 25, 
	72, 109, 141, 64, 
	4, 26, 16, 
	3, 9, 
	35, 
	8, 28, 16, 
	6, 9, 6, 22, 
	40, 
	14, 3, 33, 
	1, 30, 9, 16, 2, 9, 
	122, 61, 64, 
	18, 3, 33, 
	2, 39, 1, 30, 0, 33, 0, 30, 
	72, 125, 73, 
	206, 
	0, 39, 0, 25, 0, 9, 
	72, 69, 59, 
	18, 3, 33, 
	1, 30, 0, 33, 0, 30, 0, 20, 
	72, 53, 124, 
	8, 33, 22, 
	1, 16, 1, 35, 
	45, 
	4, 33, 22, 
	1, 36, 
	119, 
	13, 50, 22, 
	1, 36, 0, 36, 0, 16, 
	109, 72, 
	13, 50, 8, 
	1, 34, 0, 34, 0, 1, 
	109, 72, 
	13, 50, 76, 
	1, 35, 0, 35, 0, 15, 
	109, 72, 
	8, 44, 29, 
	2, 26, 6, 0, 
	112, 
	4, 9, 29, 
	2, 26, 
	8, 
	10, 1, 56, 
	4, 17, 0, 18, 
	54, 64, 56, 
	13, 48, 22, 
	1, 16, 0, 16, 0, 36, 
	109, 72, 
	8, 33, 22, 
	1, 16, 6, 35, 
	46, 
	13, 48, 76, 
	1, 15, 0, 15, 0, 35, 
	109, 72, 
	4, 33, 19, 
	1, 12, 
	47, 
	21, 15, 9, 
	3, 2, 14, 4, 14, 48, 13, 27, 0, 4, 
	27, 53, 
	4, 15, 22, 
	1, 16, 
	25, 
	9, 52, 24, 
	1, 18, 0, 2, 
	109, 58, 
	13, 15, 61, 
	4, 1, 14, 0, 14, 45, 
	27, 53, 
	8, 1, 44, 
	4, 6, 2, 26, 
	139, 
	9, 1, 44, 
	4, 6, 0, 7, 
	54, 109, 
	17, 44, 29, 
	2, 26, 1, 0, 0, 26, 0, 40, 
	72, 111, 
	18, 48, 8, 
	3, 1, 0, 1, 0, 34, 0, 34, 
	72, 74, 109, 
	22, 22, 28, 
	4, 3, 1, 24, 0, 24, 0, 38, 0, 38, 
	31, 72, 53, 
	22, 22, 28, 
	4, 11, 1, 24, 0, 24, 0, 38, 0, 38, 
	31, 72, 53, 
	14, 38, 28, 
	4, 3, 3, 38, 0, 30, 
	54, 64, 140, 
	13, 38, 28, 
	4, 11, 3, 38, 0, 12, 
	54, 64, 
	4, 38, 28, 
	2, 24, 
	104, 
	17, 15, 38, 
	4, 30, 14, 36, 14, 16, 0, 16, 
	53, 27, 
	13, 10, 18, 
	2, 11, 8, 9, 0, 11, 
	109, 74, 
	21, 36, 40, 
	4, 4, 1, 0, 14, 24, 14, 38, 0, 24, 
	53, 27, 
	8, 36, 40, 
	4, 4, 1, 0, 
	109, 
	9, 10, 28, 
	2, 24, 0, 24, 
	52, 109, 
	22, 10, 28, 
	3, 38, 7, 30, 7, 12, 0, 24, 0, 38, 
	109, 72, 73, 
	196, 
	0, 24, 
	52, 
	17, 22, 28, 
	1, 24, 7, 3, 7, 11, 0, 24, 
	53, 109, 
	13, 54, 22, 
	1, 16, 14, 5, 0, 5, 
	53, 127, 
	23, 54, 22, 
	1, 16, 3, 23, 0, 23, 0, 22, 0, 22, 
	72, 53, 132, 134, 
	17, 22, 27, 
	1, 22, 4, 10, 0, 22, 0, 11, 
	30, 62, 
	13, 2, 0, 
	4, 10, 6, 22, 0, 11, 
	54, 64, 
	14, 38, 38, 
	4, 3, 3, 38, 0, 30, 
	54, 64, 140, 
	21, 2, 0, 
	4, 10, 1, 22, 0, 22, 0, 23, 0, 11, 
	72, 54, 
	5, 54, 22, 
	1, 16, 
	109, 105, 
	4, 15, 20, 
	1, 14, 
	25, 
	4, 44, 29, 
	2, 40, 
	110, 
	0, 15, 40, 
	51, 
	8, 33, 67, 
	1, 14, 1, 35, 
	45, 
	4, 15, 19, 
	1, 12, 
	24, 
	8, 33, 20, 
	1, 14, 1, 35, 
	45, 
	8, 33, 20, 
	1, 14, 6, 35, 
	46, 
	8, 33, 67, 
	1, 16, 6, 35, 
	46, 
	8, 33, 67, 
	1, 14, 6, 35, 
	46, 
	8, 33, 67, 
	1, 16, 1, 35, 
	45, 
	13, 2, 0, 
	4, 11, 0, 10, 6, 22, 
	54, 64, 
	22, 2, 0, 
	4, 11, 1, 22, 0, 22, 0, 23, 0, 10, 
	72, 54, 64, 
	17, 22, 27, 
	4, 11, 1, 22, 0, 22, 0, 10, 
	30, 62, 
	4, 44, 0, 
	4, 13, 
	142, 
	0, 4, 0, 
	116, 
	18, 42, 20, 
	1, 14, 4, 27, 14, 37, 0, 4, 
	58, 109, 106, 
	23, 32, 71, 
	4, 27, 8, 4, 14, 37, 0, 37, 0, 17, 
	72, 149, 41, 42, 
	18, 42, 23, 
	4, 28, 2, 29, 0, 29, 0, 43, 
	107, 108, 72, 
	17, 15, 32, 
	4, 28, 2, 43, 14, 10, 0, 10, 
	27, 53, 
	17, 31, 56, 
	4, 15, 8, 5, 9, 6, 0, 6, 
	143, 58, 
	18, 58, 56, 
	4, 15, 1, 5, 9, 5, 0, 5, 
	58, 109, 144, 
	8, 58, 56, 
	4, 15, 8, 5, 
	37, 
	8, 31, 56, 
	4, 15, 8, 6, 
	21, 
	8, 31, 56, 
	4, 15, 9, 5, 
	20, 
	12, 58, 56, 
	4, 15, 9, 5, 6, 5, 
	39, 
	14, 1, 56, 
	4, 15, 8, 6, 0, 29, 
	54, 109, 64, 
	10, 15, 32, 
	4, 28, 2, 29, 
	29, 61, 64, 
	4, 74, 16, 
	3, 25, 
	115, 
	18, 54, 22, 
	1, 36, 13, 23, 0, 23, 0, 22, 
	72, 132, 134, 
	13, 54, 22, 
	1, 36, 14, 5, 0, 5, 
	127, 53, 
	5, 54, 22, 
	1, 36, 
	105, 109, 
	9, 7, 14, 
	1, 7, 7, 25, 
	146, 105, 
	0, 59, 0, 
	148, 
	0, 57, 0, 
	147, 
	4, 10, 11, 
	2, 48, 
	7, 
	4, 1, 38, 
	4, 30, 
	131, 
	0, 34, 0, 
	113, 
	23, 65, 11, 
	5, 27, 1, 4, 0, 4, 0, 48, 0, 48, 
	72, 53, 109, 105, 
	22, 18, 11, 
	1, 4, 4, 21, 2, 27, 0, 27, 0, 41, 
	117, 72, 73, 
	196, 
	0, 4, 
	59, 
	22, 65, 11, 
	1, 4, 4, 21, 2, 27, 0, 27, 0, 41, 
	117, 72, 73, 
	196, 
	0, 4, 
	59, 
	4, 36, 0, 
	7, 4, 
	49, 
	1, 15, 72, 
	28, 64, 
	8, 31, 56, 
	4, 22, 2, 31, 
	20, 
	21, 66, 34, 
	4, 22, 2, 31, 1, 3, 0, 31, 0, 32, 
	72, 73, 
	201, 
	0, 44, 0, 22, 
	62, 109, 
	9, 58, 56, 
	4, 22, 2, 31, 
	38, 130, 
	8, 31, 56, 
	4, 22, 2, 32, 
	21, 
	8, 58, 56, 
	4, 22, 2, 32, 
	37, 
	13, 1, 56, 
	4, 22, 2, 32, 0, 23, 
	54, 64, 
	18, 9, 13, 
	4, 24, 1, 41, 0, 7, 0, 41, 
	58, 59, 5, 
	4, 74, 16, 
	3, 39, 
	114, 
	8, 1, 58, 
	4, 24, 9, 7, 
	10, 
	17, 1, 58, 
	4, 24, 8, 7, 1, 8, 0, 25, 
	54, 64, 
	17, 8, 58, 
	4, 24, 8, 7, 1, 8, 0, 25, 
	54, 64, 
	8, 9, 13, 
	4, 24, 8, 7, 
	6, 
	23, 7, 14, 
	4, 25, 1, 7, 14, 11, 0, 8, 0, 11, 
	58, 53, 128, 129, 
	22, 22, 17, 
	4, 25, 1, 10, 8, 8, 0, 9, 0, 10, 
	58, 59, 23, 
	204, 
	4, 25, 8, 8, 8, 9, 
	88, 
	13, 10, 11, 
	4, 19, 2, 4, 0, 4, 
	52, 109, 
	13, 18, 11, 
	7, 21, 1, 4, 0, 4, 
	53, 109, 
	9, 10, 11, 
	2, 4, 0, 4, 
	52, 109, 
	12, 13, 0, 
	4, 24, 9, 7, 1, 41, 
	15, 
	12, 13, 0, 
	4, 24, 9, 7, 6, 41, 
	9, 
	8, 13, 0, 
	4, 28, 2, 29, 
	16, 
	12, 13, 0, 
	4, 27, 2, 17, 9, 4, 
	18, 
	8, 13, 0, 
	4, 22, 2, 31, 
	13, 
	8, 13, 0, 
	4, 25, 9, 8, 
	14, 
	12, 13, 0, 
	4, 25, 8, 8, 9, 9, 
	11, 
	4, 13, 0, 
	4, 15, 
	17, 
	8, 13, 0, 
	1, 15, 1, 12, 
	19, 
	8, 13, 0, 
	1, 15, 1, 14, 
	19, 
	9, 13, 0, 
	1, 15, 1, 16, 
	0, 19, 
	12, 13, 0, 
	4, 27, 2, 17, 8, 4, 
	12, 
	1, 13, 0, 
	4, 121, 
	0, 70, 0, 
	71, 
	4, 18, 8, 
	1, 34, 
	119, 
	4, 18, 76, 
	1, 35, 
	119, 
	4, 18, 22, 
	1, 36, 
	119, 
	0, 53, 0, 
	3, 
	1, 42, 0, 
	109, 105, 
	9, 18, 8, 
	1, 1, 0, 1, 
	53, 109, 
	9, 18, 22, 
	1, 16, 0, 16, 
	53, 109, 
	9, 18, 76, 
	1, 15, 0, 15, 
	53, 109, 
	2, 32, 0, 
	109, 85, 105, 
	0, 1, 0, 
	135, 
	12, 10, 18, 
	4, 25, 2, 11, 9, 9, 
	118, 
	4, 3, 33, 
	1, 33, 
	123, 
	22, 73, 33, 
	1, 33, 2, 20, 0, 30, 0, 33, 0, 20, 
	72, 59, 109, 
	0, 15, 0, 
	28, 
	0, 48, 0, 
	135, 
	0, 40, 0, 
	135, 
	0, 14, 0, 
	66, 
	0, 6, 0, 
	63, 
	0, 7, 0, 
	135, 
	0, 13, 0, 
	4, 
	4, 8, 0, 
	4, 25, 
	109, 
	0, 8, 0, 
	135, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
83, 87, 73, 77,
83, 81, 85, 69,
66, 85, 82, 78,
80, 79, 85, 82,
81, 85, 73, 84,
66, 76, 79, 87,
83, 65, 73, 76,
70, 69, 69, 68,
71, 69, 84, 32,
212, 65, 75, 69,
208, 73, 67, 75,
72, 69, 76, 80,
73, 78, 86, 69,
76, 79, 79, 75,
197, 88, 65, 77,
211, 69, 65, 82,
68, 82, 79, 80,
208, 85, 84, 32,
204, 69, 65, 86,
199, 73, 86, 69,
84, 72, 82, 79,
211, 76, 73, 78,
195, 72, 85, 67,
200, 85, 82, 76,
76, 73, 71, 72,
201, 71, 78, 73,
85, 78, 76, 73,
196, 79, 85, 83,
197, 88, 84, 73,
79, 80, 69, 78,
83, 65, 89, 32,
82, 69, 65, 68,
74, 85, 77, 80,
204, 69, 65, 80,
67, 72, 79, 80,
195, 85, 84, 32,
67, 76, 73, 77,
193, 83, 67, 69,
77, 79, 86, 69,
211, 72, 73, 70,
87, 65, 86, 69,
211, 72, 65, 75,
75, 73, 76, 76,
196, 69, 83, 84,
197, 88, 84, 69,
193, 84, 84, 65,
87, 69, 65, 82,
196, 79, 78, 32,
82, 69, 77, 79,
196, 79, 70, 70,
72, 79, 76, 68,
83, 67, 79, 82,
82, 85, 66, 32,
208, 79, 76, 73,
211, 72, 73, 78,
69, 65, 84, 32,
85, 78, 76, 79,
83, 87, 69, 65,
198, 85, 67, 75,
194, 65, 76, 76,
194, 79, 76, 76,
211, 72, 73, 84,
194, 65, 83, 84,
83, 80, 82, 73,
83, 77, 65, 83,
200, 73, 84, 32,
194, 82, 69, 65,
200, 65, 77, 77,
83, 65, 86, 69,
211, 84, 79, 82,
77, 79, 80, 32,
83, 79, 65, 75,
70, 73, 76, 76,
210, 69, 80, 76,
210, 69, 70, 73,
32, 32, 32, 32,
32, 32, 32, 32,
	0,
};
const uint8_t nouns[] = {
65, 78, 89, 32,
78, 79, 82, 84,
83, 79, 85, 84,
69, 65, 83, 84,
87, 69, 83, 84,
85, 80, 32, 32,
68, 79, 87, 78,
83, 87, 79, 82,
67, 76, 79, 65,
66, 65, 82, 82,
72, 65, 77, 77,
83, 65, 76, 84,
75, 69, 89, 32,
67, 82, 65, 66,
72, 79, 82, 78,
82, 65, 70, 84,
76, 65, 77, 80,
75, 78, 73, 70,
66, 65, 84, 79,
80, 65, 82, 67,
83, 84, 65, 70,
215, 79, 79, 68,
82, 73, 78, 71,
81, 85, 65, 82,
77, 73, 82, 82,
80, 79, 79, 76,
72, 79, 76, 69,
77, 65, 84, 67,
82, 79, 80, 69,
87, 79, 76, 70,
83, 76, 85, 71,
71, 79, 82, 71,
76, 73, 90, 65,
82, 65, 71, 32,
80, 65, 68, 76,
83, 84, 65, 84,
83, 84, 79, 78,
83, 84, 82, 69,
84, 82, 69, 69,
200, 79, 76, 76,
66, 82, 73, 65,
67, 65, 66, 73,
200, 85, 84, 32,
87, 73, 78, 68,
80, 65, 84, 72,
82, 79, 65, 68,
80, 79, 78, 68,
80, 79, 82, 84,
66, 65, 84, 84,
67, 79, 85, 82,
83, 84, 65, 66,
72, 79, 76, 69,
84, 79, 82, 84,
83, 84, 65, 73,
80, 65, 83, 83,
67, 65, 86, 69,
68, 79, 79, 82,
83, 84, 79, 82,
76, 65, 75, 69,
87, 79, 82, 75,
83, 79, 82, 67,
76, 69, 65, 86,
208, 73, 76, 69,
77, 79, 65, 84,
66, 79, 65, 82,
65, 82, 67, 72,
83, 84, 82, 65,
82, 85, 78, 69,
75, 78, 73, 71,
198, 73, 71, 85,
197, 69, 82, 73,
65, 75, 89, 82,
65, 82, 79, 85,
193, 66, 79, 85,
210, 79, 85, 78,
82, 69, 69, 68,
72, 69, 76, 77,
79, 70, 70, 32,
67, 65, 83, 84,
	0,
};
const uint8_t automap[] = {
83, 87, 79, 82,
	0,
67, 76, 79, 65,
	1,
66, 65, 82, 82,
	2,
72, 65, 77, 77,
	3,
83, 65, 76, 84,
	4,
75, 69, 89, 32,
	5,
72, 79, 82, 78,
	7,
82, 65, 70, 84,
	8,
76, 65, 77, 80,
	9,
75, 78, 73, 70,
	10,
80, 65, 82, 67,
	12,
83, 84, 65, 70,
	14,
72, 69, 76, 77,
	15,
82, 73, 78, 71,
	16,
81, 85, 65, 82,
	17,
77, 73, 82, 82,
	18,
77, 65, 84, 67,
	22,
77, 65, 84, 67,
	23,
82, 79, 80, 69,
	24,
76, 65, 77, 80,
	25,
83, 76, 85, 71,
	27,
82, 65, 71, 32,
	30,
82, 65, 71, 32,
	33,
81, 85, 65, 82,
	37,
76, 65, 77, 80,
	39,
83, 76, 85, 71,
	41,
	0,
};
#include <stdio.h>	/* Not really used but needed for perror */
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <termios.h>

#ifdef __linux__
#include <stdio.h>
#endif

static jmp_buf restart;

struct savearea {
  uint16_t magic;
  uint8_t carried;
  uint8_t lighttime;
  uint8_t location;
  uint8_t objloc[NUM_OBJ];
  uint8_t roomsave[6];
  uint8_t savedroom;
  uint32_t bitflags;
  int16_t counter;
  int16_t counter_array[16];
};

static char linebuf[81];
static char *nounbuf;
static char wordbuf[WORDSIZE + 1];

static uint8_t verb;
static uint8_t noun;
static const uint8_t *linestart;
static uint8_t linematch;
static uint8_t actmatch;
static uint8_t continuation;
static uint16_t *param;
static uint16_t param_buf[5];
static uint8_t redraw;

static struct savearea game;

static void error(const char *p);

#define VERB_GO		1
#define VERB_GET	10
#define VERB_DROP	18

#define LIGHTOUT	16
#define DARKFLAG	15
#define LIGHT_SOURCE	9

/* Define this because 1 << n might be 16bit */
#define ONEBIT		((uint32_t)1)

#define REDRAW		1
#define REDRAW_MAYBE	2

#ifdef CONFIG_IO_CURSES

#include <curses.h>

#define REDRAW_MASK	(REDRAW|REDRAW_MAYBE)

static char wbuf[81];
static int wbp = 0;
static uint8_t rows, cols;
static int xpos = 0, ypos = 0;
static int bottom;
static WINDOW *topwin, *botwin, *curwin;

static void flush_word(void)
{
  wbuf[wbp] = 0;
  waddstr(curwin, wbuf);
  xpos += wbp;
  wbp = 0;
}

static void new_line(void)
{
  xpos = 0;
  if (curwin == topwin)
    ypos++;
  else {
    scroll(curwin);
    ypos = bottom;
  }
  wmove(curwin, ypos, xpos);
}

static void char_out(char c)
{
  if (c == '\n') {
    flush_word();
    new_line();
    return;
  }
  if (c != ' ') {
    if (wbp < 80)
      wbuf[wbp++] = c;
    return;
  }
  if (xpos + wbp >= cols)
    new_line();
  flush_word();
  waddch(curwin, ' ');
  xpos++;
}

static void strout_lower(const char *p)
{
  while(*p)
    char_out(*p++);
}

static void strout_lower_spc(const char *p)
{
  strout_lower(p);
  char_out(' ');
}

static void decout_lower(uint16_t v)
{
#ifdef __linux__
  char buf[9];
  snprintf(buf, 8, "%d", v);	/* FIXME: avoid expensive snprintf */
  strout_lower(buf);
#else
  strout_lower(_itoa(v));
#endif
}

static void strout_upper(const char *p)
{
  strout_lower(p);
}

static char readchar(void)
{
  wrefresh(botwin);
  return wgetch(botwin);
}

static void line_input(int m)
{
  int c;
  char *p = linebuf;

  do {
    wmove(botwin, ypos, xpos);
    wrefresh(botwin);
    c = wgetch(botwin);
    if (c == 8 || c == 127) {
      if (p > linebuf) {
        xpos--;
        mvwaddch(botwin, ypos, xpos, ' ');
        p--;
      }
      continue;
    }
    if (c > 31 && c < 127) {
      if (p < linebuf + 80 && xpos < cols - 1) {
        *p++ = c;
        mvwaddch(botwin, ypos, xpos, c);
        xpos++;
      }
      continue;
    }
  }
  while (c != 13 && c != 10);
  *p = 0;
  new_line();
}

static int saved_x;

static void begin_upper(void)
{
  saved_x = xpos;
  curwin = topwin;
  werase(topwin);
  ypos = 0;
  xpos = 0;
}

static void end_upper(void)
{
  flush_word();
  curwin = botwin;
  xpos = saved_x;
  ypos = bottom;
  wrefresh(topwin);
}

static void display_init(void)
{
  int trow;

  initscr();
  noecho();
  cbreak();
  nonl();

  getmaxyx(stdscr, rows, cols);

  if (rows < 16)
    error("display too small");

  trow = 10;
  if (rows / 2 < 10)
    trow = rows / 2;
  bottom = rows - trow;

  topwin = newwin(trow, cols, 0, 0);
  botwin = newwin(bottom--, cols, trow, 0);
  if (!topwin || !botwin)
    error("curses");
  scrollok(botwin, TRUE);
  curwin = botwin;
  new_line();
}

static void display_exit(void)
{
  endwin();
}

#elif defined(CONFIG_IO_CUSS)

/* ---- */

#include <termcap.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>

/* A mini look alike to David Given's libcuss. If useful will probably
   become a library. For now pasted around to experiment */

uint_fast8_t screenx, screeny, screen_height, screen_width;

static char *t_go, *t_clreol, *t_clreos;
static char conbuf[64];
static char *conp = conbuf;

extern void con_puts(const char *s);

/* Queue a character to the output buffer */
static int conq(int c)
{
	if (conp == conbuf + sizeof(conbuf)) {
		write(1, conbuf, sizeof(conbuf));
		conp = conbuf;
	}
	*conp++ = (uint8_t) c;
	return 0;
}

/* Make sure the output buffer is written */
void con_flush(void)
{
	write(1, conbuf, conp - conbuf);
	conp = conbuf;
}

static const char hex[] = "0123456789ABCDEF";

/* Put a character to the screen. We handle unprintables and tabs */
void con_putc(char c)
{
	if (c == '\t') {
		uint8_t n = 8 - (screenx & 7);
		while (n--)
			con_putc(' ');
		return;
	}
	if (c > 127) {
		con_puts("\\x");
		con_putc(hex[c >> 4]);
		con_putc(hex[c & 0x0F]);
		return;
	} else if (c == 127) {
		con_puts("^?");
		return;
	}
	if (c < 32) {
		con_putc('^');
		c += '@';
	}
	conq(c);
	screenx++;
	if (screenx == screen_width) {
		screenx = 0;
		screeny++;
	}
}

/* Write a termcap string out */
static void con_twrite(char *p, int n)
{
#if !defined(__linux__)
	tputs(p, n, conq);
#else
	while (*p)
		conq(*p++);
#endif
}

/* Write a string of symbols including quoting */
void con_puts(const char *s)
{
	char c;
	while ((c =  *s++) != 0)
		con_putc(c);
}

/* Add a newline */
void con_newline(void)
{
	if (screeny >= screen_height)
		return;
	conq('\n');
	screenx = 0;
	screeny++;
}

/* We need to optimize this but firstly we need to fix our
   tracking logic as we use con_goto internally but don't track
   that verus the true user values */
void con_force_goto(uint_fast8_t y, uint_fast8_t x)
{
	con_twrite(tgoto(t_go, x, y), 2);
	screenx = x;
	screeny = y;
}

void con_goto(uint_fast8_t y, uint_fast8_t x)
{
#if 0
	if (screenx == x && screeny == y)
		return;
	if (screeny == y && x == 0) {
		conq('\r');
		screenx = 0;
		return;
	}
	if (screeny == y - 1 && x == 0) {
		con_newline();
		return;
	}
#endif	
	con_force_goto(y, x);
}

/* Clear to end of line */
void con_clear_to_eol(void)
{
	if (screenx == screen_width - 1)
		return;
	if (t_clreol)
		con_twrite(t_clreol, 1);
	else {
		uint_fast8_t i;
		/* Write spaces. This tends to put the cursor where
		   we want it next time too. Might be worth optimizing ? */
		for (i = screenx; i < screen_width; i++)
			con_putc(' ');
	}
}

/* Clear to the bottom of the display */

void con_clear_to_bottom(void)
{
	/* Most terminals have a clear to end of screen */
	if (t_clreos)
		con_twrite(t_clreos, screen_height);
	/* If not then clear each line, which may in turn emit
	   a lot of spaces in desperation */
	else {
		uint_fast8_t i;
		for (i = 0; i < screen_height; i++) {
			con_goto(i, 0);
			con_clear_to_eol();
		}
	}
	con_force_goto(0, 0);
}

void con_clear(void)
{
	con_goto(0, 0);
	con_clear_to_bottom();
}

int con_scroll(int n)
{
	if (n == 0)
		return 0;
	/* For now we don't do backscrolls: FIXME */
	if (n < 0)
		return 1;
	/* Scrolling down we can do */
	con_force_goto(screen_height - 1, 0);
	while (n--)
		conq('\n');
	con_force_goto(screeny, screenx);
	return 0;
}

/* TODO: cursor key handling */
int con_getch(void)
{
	uint8_t c;
	con_flush();
	if (read(0, &c, 1) != 1)
		return -1;
	return c;
}

int con_size(uint8_t c)
{
	if (c == '\t')
		return 8 - (screenx & 7);
	/* We will leave unicode out 8) */
	if (c > 127)
		return 4;
	if (c < 32 || c == 127)
		return 2;
	return 1;
}

static int do_read(int fd, void *p, int len)
{
	int l;
	if ((l = read(fd, p, len)) != len) {
		if (l < 0)
			perror("read");
		else
			write(2, "short read from tchelp.\n", 25);
		return -1;
	}
	return 0;
}

static char *tnext(char *p)
{
	return p + strlen(p) + 1;
}

static int tty_init(void)
{
	int fd[2];
	pid_t pid;
	int ival[3];
	int status;

	if (pipe(fd) < 0) {
		perror("pipe");
		return -1;
	}

	pid = fork();
	if (pid == -1) {
		perror("fork");
		return -1;
	}

	if (pid == 0) {
		close(fd[0]);
		dup2(fd[1], 1);
		execl("/usr/lib/tchelp", "tchelp", "li#co#cm$ce$cd$cl$", NULL);
		perror("tchelp");
		_exit(1);
	}
	close(fd[1]);
	waitpid(pid, &status, 0);

	do_read(fd[0], ival, sizeof(int));
	if (ival[0] == 0)
		return -1;
	do_read(fd[0], ival + 1, 2 * sizeof(int));

	ival[0] -= 2 * sizeof(int);
	t_go = sbrk((ival[0] + 3) & ~3);

	if (t_go == (void *) -1) {
		perror("sbrk");
		return -1;
	}

	if (do_read(fd[0], t_go, ival[0]))
		return -1;

	close(fd[0]);
	t_clreol = tnext(t_go);
	t_clreos = tnext(t_clreol);
	if (*t_clreos == 0)	/* No clr eos - try for clr/home */
		t_clreos++;	/* cl cap if present */
	if (*t_go == 0) {
		write(2, "Insufficient terminal features.\n", 32);
		return -1;
	}
	/* TODO - screen sizes */
	screen_height = ival[1];
	screen_width = ival[2];
	/* need to try WINSZ and VT ioctls */
	return 0;
}

static struct termios con_termios, old_termios;

void con_exit(void)
{
	tcsetattr(0, TCSANOW, &old_termios);
}

int con_init(void)
{
	int n;
	static struct winsize w;
	if (tty_init())
		return -1;
	if (tcgetattr(0, &con_termios) == -1)
		return -1;
	memcpy(&old_termios, &con_termios, sizeof(struct termios));
	atexit(con_exit);
	con_termios.c_lflag &= ~(ICANON | ECHO | ISIG);
	con_termios.c_iflag &= ~(IXON);
	con_termios.c_cc[VMIN] = 1;
	con_termios.c_cc[VTIME] = 0;
	if (tcsetattr(0, TCSANOW, &con_termios) == -1)
		return -1;
#ifdef VTSIZE
	n = ioctl(0, VTSIZE, 0);
	if (n != -1) {
		screen_width = n & 0xFF;
		screen_height = (n >> 8) & 0xFF;
	}
#endif
	if (ioctl(0, TIOCGWINSZ, &w) == 0) {
		if (w.ws_col)
			screen_width = w.ws_col;
		if (w.ws_row)
			screen_height = w.ws_row;
	}
	return 0;
}


/* ---- */

/* Glue to the library */

#define REDRAW_MASK	0

static char wbuf[81];
static int wbp = 0;
static int upper;

static void display_exit(void)
{
  con_newline();
  con_flush();
}

static void display_init(void)
{
  if (con_init())
    exit(1);
  con_clear();
  con_goto(screen_height - 1, 0);
}

static void flush_word(void)
{
  if (screenx)
    con_putc(' ');
  wbuf[wbp] = 0;
  con_puts(wbuf);
  wbp = 0;
}

static void move_on(void)
{
    /* Move on a line in the correct manner */
    if (upper) {
      con_clear_to_eol();
      con_newline();
    } else {
      con_scroll(1);
      con_goto(screen_height - 1, 0);
    }
}

static void char_out(char c)
{
  if (c != ' ' && c != '\n') {
    if (wbp < 80)
      wbuf[wbp++] = c;
    return;
  }
  /* Does the word not fit ? */
  if (screenx + wbp + 1 >= screen_width)
    move_on();
  /* Write out the word */
  flush_word();
  if (c == '\n')
    move_on();
}

static void strout_lower(const char *p)
{
  while(*p)
    char_out(*p++);
}

static void strout_lower_spc(const char *p)
{
  strout_lower(p);
  char_out(' ');
}

static void decout_lower(uint16_t v)
{
#ifdef __linux__
  char buf[9];
  snprintf(buf, 8, "%d", v);	/* FIXME: avoid expensive snprintf */
  strout_lower(buf);
#else
  strout_lower(_itoa(v));
#endif
}

static void strout_upper(const char *p)
{
  strout_lower(p);
}

static void action_look(void);

static void line_input(int m)
{
  int c;
  char *p = linebuf;

  if (m == 0)
    action_look();

  do {
    c = con_getch();
    if (c == 8 || c == 127) {
      if (p > linebuf) {
        con_goto(screen_height - 1, screenx - 1);
        con_putc(' ');
        con_goto(screen_height - 1, screenx - 1);
        p--;
      }
      continue;
    }
    if (c > 31 && c < 127) {
      if (p < linebuf + 80 && screenx < screen_width - 1) {
        *p++ = c;
        con_putc(c);
      }
      continue;
    }
  }
  while (c != 13 && c != 10);
  *p = 0;
  con_scroll(1);
  con_goto(screen_height - 1, 0);
}

static char readchar(void)
{
  line_input(1);
  return *linebuf;
}


static uint8_t ly, lx;

static void begin_upper(void)
{
  ly = screeny;
  lx = screenx;
  flush_word();
  con_goto(0,0);
  upper = 1;
}

char xbuf[] = "<@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@><@>";

static void end_upper(void)
{
  flush_word();
  con_clear_to_eol();
  con_newline();
  upper = 0;
  xbuf[screen_width] = 0;
  con_puts(xbuf);  
  con_goto(ly, lx);
}

#else

#include <stdio.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <termios.h>

#define REDRAW_MASK	REDRAW

static char wbuf[80];
static int wbp = 0;
static int xpos = 0;

static void display_init(void)
{
  char *c;
#ifdef TIOCGWINSZ
  struct winsize w;
  if (ioctl(0, TIOCGWINSZ, &w) != -1) {
    rows = w.ws_row;
    cols = w.ws_col;
    return;
  }
#elif VTSIZE
  int16_t v = ioctl(0, VTSIZE, 0);
  if (v != -1) {
    rows =  v >> 8;
    cols = v;
    return;
  }
#endif
  c = getenv("COLS");
  rows = 25;
  cols = c ? atoi(c): 80;
  if (cols == 0)
    cols = 80;
}

static void display_exit(void)
{
}

static void flush_word(void)
{
  write(1, wbuf, wbp);
  xpos += wbp;
  wbp = 0;
}

static void char_out(char c)
{
  if (c == '\n') {
    flush_word();
    write(1, "\n", 1);
    xpos = 0;
    return;
  }
  if (c != ' ') {
    if (wbp < 80)
      wbuf[wbp++] = c;
    return;
  }
  if (xpos + wbp >= cols) {
    xpos = 0;
    write(1,"\n", 1);
  }
  flush_word();
  write(1," ", 1);
  xpos++;
}

static void strout_lower(const char *p)
{
  while(*p)
    char_out(*p++);
}

static void strout_lower_spc(const char *p)
{
  strout_lower(p);
  char_out(' ');
}

static void decout_lower(uint16_t v)
{
#ifdef __linux__
  char buf[9];
  snprintf(buf, 8, "%d", v);	/* FIXME: avoid expensive snprintf */
  strout_lower(buf);
#else
  strout_lower(_itoa(v));
#endif
}

static void strout_upper(const char *p)
{
  strout_lower(p);
}


static void line_input(int m)
{
  int l = read(0, linebuf, sizeof(linebuf));
  if (l < 0)
    error("read");
  linebuf[l] = 0;
  if (l && linebuf[l-1] == '\n')
    linebuf[l-1] = 0;
}

static char readchar(void)
{
  line_input(0);
  return *linebuf;
}

static void begin_upper(void)
{
  strout_upper("\n\n\n\n");
}

static void end_upper(void)
{
  uint8_t l = cols;
  char_out('\n');
  while(l--)
    char_out('-');
  char_out('\n');
}



#endif

/******************** Common code ******************/

static uint8_t yes_or_no(void)
{
  char c;
  do {
    c = readchar();
    if (c == 'Y'  || c == 'y' || c == 'J' || c == 'j')
      return 1;
  } while(c != -1 && c != 'N' && c != 'n');
  return 0;
}

static void exit_game(uint8_t code)
{
  display_exit();
  exit(code);
}

static void error(const char *p)
{
  display_exit();
  write(2, p, strlen(p));
  exit(1);
}

static uint8_t random_chance(uint8_t v)
{
  v = v + v + (v >> 1);	/* scale as 0-249 */
  if (((rand() >> 3) & 0xFF) <= v)
    return 1;
  return 0;
}

static char *skip_spaces(char *p)
{
  while(*p && isspace(*p))
    p++;
  return p;
}

static char *copyword(char *p)
{
  char *t = wordbuf;
  p = skip_spaces(p);
  memset(wordbuf, ' ', WORDSIZE+1);
  while (*p && !isspace(*p) && t < wordbuf + WORDSIZE)
    *t++ = *p++;
  while(*p && !isspace(*p))
    p++;
  return p;
}

static int wordeq(const uint8_t *a, const char *b, uint8_t l)
{
  while(l--)
    if ((*a++ & 0x7F) != toupper(*b++))
      return 0;
  return 1;
}

static uint8_t whichword(const uint8_t *p)
{
  uint8_t code = 0;
  uint8_t i = 0;

  if (*wordbuf == 0 || *wordbuf == ' ')
    return 0;		/* No word */
  i--;
  
  do {
    i++;
    if (!(*p & 0x80))
      code = i;
    if (wordeq(p, wordbuf, WORDSIZE))
      return code;
    p += WORDSIZE;
  } while(*p != 0);
  return 255;
}

static void scan_noun(char *x)
{
  x = skip_spaces(x);
  nounbuf = x;
  copyword(x);
  noun = whichword(nouns);
}

static void scan_input(void)
{
  char *x = copyword(linebuf);
  verb = whichword(verbs);
  scan_noun(x);
}

void abbrevs(void)
{
  char *x = skip_spaces(linebuf);
  const char *p = NULL;
  if (x[1] != 0 && x[1] != ' ')
    return;
  switch(toupper(*x)) {
    case 'N': 
      p = "NORTH";
      break;
    case 'E':
      p = "EAST";
      break;
    case 'S':
      p = "SOUTH";
      break;
    case 'W':
      p = "WEST";
      break;
    case 'U':
      p = "UP";
      break;
    case 'D':
      p = "DOWN";
      break;
    case 'I':
      p = "INVEN";
      break;
  }
  if (p)
    strcpy(linebuf, p);
}
  
static const uint8_t *run_conditions(const uint8_t *p, uint8_t n)
{
  uint8_t i;
  
  for (i = 0; i < n; i++) {
    uint8_t opc = *p++;
    uint16_t par = *p++ | ((opc & 0xE0) >> 5);
    uint8_t op = game.objloc[par];
    opc &= 0x1F;

    switch(opc) {
      case 0:
        *param++ = par;
        break;
      case 1:
        if (op != 255)
          return NULL;
        break;
      case 2:
        if (op != game.location)
          return NULL;
        break;
      case 3:
        if (op != 255 && op != game.location)
          return NULL;
        break;
      case 4:
        if (game.location != par)
          return NULL;
        break;
      case 5:
        if (op == game.location)
          return NULL;
        break;
      case 6:
        if (op == 255)
          return NULL;
        break;
      case 7:
        if (game.location == par)
          return NULL;
        break;
      case 8:
        if (!(game.bitflags & (ONEBIT << par)))
          return NULL;
        break;
      case 9:
        if (game.bitflags & (ONEBIT << par))
          return NULL;
        break;
      case 10:
        if (!game.carried)
          return NULL;
        break;
      case 11:
        if (game.carried)
          return NULL;
        break;
      case 12:
        if (op == 255 || op == game.location)
          return NULL;
        break;
      case 13:
        if (op == 0)
          return NULL;
        break;
      case 14:
        if (op != 0)
          return NULL;
        break;
      case 15:
        if (game.counter > par)
          return NULL;
        break;
      case 16:
        if (game.counter < par)
          return NULL;
        break;
      case 17:
        if (op != objinit[par]) 
          return NULL;
        break;
      case 18:
        if (op == objinit[par])
          return NULL;
        break;
      case 19:
        if (game.counter != par)
          return NULL;
        break;
      default:
        error("BADCOND");
    }
  }
  return p;
}

uint8_t islight(void)
{
  uint8_t l = game.objloc[LIGHT_SOURCE];
  if (!(game.bitflags & (ONEBIT << DARKFLAG)))
    return 1;
  if (l == 255 || l == game.location)
    return 1;
  return 0;
}

static void action_look(void)
{
  const uint8_t *e;
  const char *p;
  uint8_t c;
  uint8_t f = 1;
  const char **op = objtext;

  redraw = 0;

  begin_upper();

  if (!islight()) {
    strout_upper(itsdark);
    end_upper();
    return;
  }
  p = locdata[game.location].text;
  e = locdata[game.location].exit;
  if (*p == '*')
    p++;
  else
    strout_upper(youare);
  strout_upper(p);
  strout_upper(newline);
  strout_upper(obexit);

  for (c = 0; c < 6; c++) {
    if (*e++) {
      if (f)
        f = 0;
      else
        strout_upper(dashstr);
      strout_upper(exitmsgptr[c]);
    }
  }
  if (f)
    strout_upper(nonestr);
  strout_upper(dotnewline);
  f = 1;
  e = game.objloc;
  while(e < game.objloc + NUM_OBJ) {
    if (*e++ == game.location) {
      if (f) {
        strout_upper(canalsosee);
        f = 0;
      } else
        strout_upper(dashstr);
      strout_upper(*op);
    }
    op++;
  }
  end_upper();
}

static void action_delay(void)
{
  sleep(2);
}

static void action_dead(void)
{
  strout_lower(dead);
  game.bitflags &= ~(ONEBIT << DARKFLAG);
  game.location = lastloc;
  action_look();
}

static void action_quit(void)
{
  strout_lower(playagain);
  if (yes_or_no())
    longjmp(restart, 0);
  exit_game(0);
}

static void action_score(void)
{
  uint8_t *p = game.objloc;
  const char **m = objtext;
  uint8_t t = 0, s = 0;

  while(p < game.objloc + NUM_OBJ) {
    if (*m[0] == '*') {
      t++;
      if (*p == treasure)
        s++;
    }
    m++;
    p++;
  }

  strout_lower(stored_msg);
  decout_lower(s);
  strout_lower(stored_msg2);
  decout_lower((s * (uint16_t)100) / t);
  strout_lower(dotnewline);
  if (s == t)
    action_quit();
}

static void action_inventory(void)
{
  uint8_t *p = game.objloc;
  const char **m = objtext;
  uint8_t f = 1;

  strout_lower(carrying);
  if (game.carried == 0)
    strout_lower(nothing);
  else {  
    while(p < game.objloc + NUM_OBJ) {
      if (*p == 255) {
        if (!f)
          strout_lower(dashstr);
        else
          f = 0;
        strout_lower(*m);
      }
      m++;
      p++;
    }
  }
  strout_lower(dotnewline);
}

static char *filename(void)
{
  strout_lower("File name ? ");
  line_input(1);
  return skip_spaces(linebuf);
}

static void action_save(void)
{
  int fd;
  char *p = filename();
  if (*p == 0)
    return;
  game.magic = GAME_MAGIC;
  fd = open(p, O_WRONLY|O_CREAT|O_TRUNC, 0600);
  if (fd == -1 || write(fd, &game, sizeof(game)) != sizeof(game) || close(fd) == -1)
    strout_lower("Save failed.\n");
  close(fd);	/* Double closing is safe for non error path */
}

static int action_restore(void)
{
  while(1) {
    char *p = filename();
    int fd;

    if (*p == 0)
      return 0;

    fd = open(p, O_RDONLY, 0600);

    if (fd != -1 && read(fd, &game, sizeof(game)) == sizeof(game) && close(fd) != -1 &&
        game.magic == GAME_MAGIC)
      return 1;

    strout_lower("Load failed.\n");
    close(fd);
  }
}
  
static void moveitem(uint8_t i, uint8_t l)
{
  uint8_t *p = game.objloc + i;
  if (*p == game.location)
    redraw |= REDRAW_MAYBE;
  if (l == game.location)
    redraw |= REDRAW;
  *p = l;
}

static void run_actions(const uint8_t *p, uint8_t n)
{
  uint8_t i;

  for (i = 0; i < n; i++) {
    uint8_t a = *p++;
    uint8_t tmp;
    uint16_t tmp16;

    if (a < 50) {
      strout_lower_spc(msgptr[a]);
      continue;
    }
    if (a > 102 ) {
      strout_lower_spc(msgptr[a - 50]);
      continue;
    }
    switch(a) {
      case 51:	/* nop - check */
        break;
      case 52:	/* Get */
        if (game.carried >= maxcar)
          strout_lower(toomuch);
        else
          moveitem(*param++, 255);
        break;
      case 53: /* Drop */
        moveitem(*param++, game.location);
        break;
      case 54: /* Go */
        game.location = *param++;
        redraw |= REDRAW;
        break;
      case 55: /* Destroy */
      case 59: /* ?? */
        moveitem(*param++, 0);
        break;
      case 56:	/* Set dark flag */
        game.bitflags |= (ONEBIT << DARKFLAG);
        break;
      case 57:	/* Clear dark flag */
        game.bitflags &= ~(ONEBIT << DARKFLAG);
        break;
      case 58:	/* Set bit */
        game.bitflags |= (ONEBIT << *param++);
        break;
      /* 59 see 55 */
      case 60:	/* Clear bit */
        game.bitflags &= ~(ONEBIT << *param++);
        break;
      case 61:	/* Dead */
        action_dead();
        break;
      case 64:	/* Look */
      case 76:	/* Also Look ?? */
        action_look();
        break;
      case 62:	/* Place obj, loc */
        tmp = *param++;
        moveitem(tmp, *param++);
        break;
      case 63:	/* Game over */
        action_quit();
      case 65:	/* Score */
        action_score();
        break;
      case 66:	/* Inventory */
        action_inventory();
      case 67:	/* Set bit 0 */
        game.bitflags |= (ONEBIT << 0);
        break;
      case 68:	/* Clear bit 0 */
        game.bitflags &= ~(ONEBIT << 0);
        break;
      case 69:	/* Refill lamp */
        game.lighttime = lightfill;
        game.bitflags &= ~(ONEBIT << LIGHTOUT);
        moveitem(LIGHT_SOURCE, 255);
        break;
      case 70:	/* Wipe lower */
        /* TODO */
        break;
      case 71:	/* Save */
        action_save();
        break;
      case 72:	/* Swap two objects */
        tmp = game.objloc[*param];
        moveitem(*param, game.objloc[param[1]]);
        moveitem(param[1], tmp);
        param += 2;
        break;
      case 73:
        continuation = 1;
        break;
      case 74:	/* Get without weight rule */
        moveitem(*param++, 255);
        break;
      case 75:	/* Put one item by another */
        moveitem(*param, game.objloc[param[1]]);
        param += 2;
        break;
      case 77:	/* Decrement counter */
        if (game.counter >= 0)
          game.counter--;
        break;
      case 78:	/* Display counter */
        decout_lower(game.counter);
        break;
      case 79:	/* Set counter */
        game.counter = *param++;
        break;
      case 80:	/* Swap player and saved room */
        tmp = game.savedroom;
        game.savedroom = game.location;
        game.location = tmp;
        redraw |= REDRAW;
        break;
      case 81:	/* Swap counter and counter n */
        tmp16 = game.counter;
        game.counter = game.counter_array[*param];
        game.counter_array[*param++] = tmp16;
        break;
      case 82:	/* Add to counter */
        game.counter += *param++;
        break;
      case 83:	/* Subtract from counter */
        game.counter -= *param++;
        if (game.counter < 0)
          game.counter = -1;
        break;
      case 84:	/* Print noun, newline */
        strout_lower(nounbuf);
        /* Fall through */
      case 86:	/* Print newline */
        strout_lower(newline);
        break;
      case 85:	/* Print noun */ 
        strout_lower(nounbuf);
        break;
      case 87: /* Swap player and saveroom array entry */
        tmp16 = *param++;
        tmp = game.roomsave[tmp16];
        game.roomsave[tmp16] = game.location;
        if (tmp != game.location) {
          game.location = tmp;
          redraw |= REDRAW;
        }
        break;
      case 88:
        action_delay();
        break;
      case 89:
        param++;		/* SAGA etc specials */
        break;
      default:
        error("BADACT");
    }
  }
}

void next_line(void)
{
  uint8_t c = *linestart++;
  if (!(c & 0x80))
    linestart += 2;	/* Skip verb/noun */
  else if (!(c & 0x60))
    linestart++;	/* Skip random value */
  linestart += (c & 3) + 1;	/* Actions 1 - 4 */
  c >>= 1;
  c &= 0x0E;		/* 2 x conditions */
  linestart += c;
}

void run_line(const uint8_t *ptr, uint8_t c, uint8_t a)
{
  memset(param_buf, 0, sizeof(param_buf));
  param = param_buf;
  if (c)
    ptr = run_conditions(ptr, c);
  if (ptr) {
    actmatch = 1;
    param = param_buf;
    run_actions(ptr, a);
  }
  next_line();
}

void run_table(const uint8_t *tp)
{
  continuation = 0;
  linestart = tp;
  while(1) {
    uint8_t hdr;
    uint8_t c, a;
    tp = linestart;
    hdr = *tp++;
    c = (hdr >> 2) & 0x07;
    a = (hdr & 3) + 1;
    
/*    printf("H%02X c = %d a = %d\n", hdr, c, a); */
    if (hdr == 255)
      return;		/* End of table */
    if (hdr & 0x80) {
      if (hdr & 0x40) {	/* Auto 0 */
        if (continuation)
          run_line(tp, c, a);
        else
          next_line();
        continue;
      }
      continuation = 0;
      if (!(hdr & 0x20)) {	/* Auto number */
        if (random_chance(*tp++))
          run_line(tp, c, a);
        else
          next_line();
        continue;
      }
      run_line(tp, c, a);
    } else {
      if (actmatch)
        return;
/*      printf("VN %d %d\n", *tp, tp[1]); */
      linematch = 1;
      continuation = 0;
      if (*tp++ == verb && (*tp == noun || *tp == 0))
        run_line(tp+1, c, a);
      else
        next_line();
    }
  }
}

uint8_t autonoun(uint8_t loc)
{
  const uint8_t *p = automap;
  if (*wordbuf == ' ' || *wordbuf == 0)
    return 255;
  while(*p) {
    if (strncasecmp((const char *)p, wordbuf, WORDSIZE) == 0 && game.objloc[p[WORDSIZE]] == loc)
      return p[WORDSIZE];
    p += WORDSIZE + 1;
  }
  return 255;
}
  
void run_command(void)
{
  uint8_t tmp;
  run_table(actions);
  if (actmatch)
    return;
  if (verb == VERB_GET) {		/* Get */
    if (noun == 0)
      strout_lower(whatstr);
    else if (game.carried >= maxcar)
      strout_lower(toomuch);
    else {
      tmp = autonoun(game.location);
      if (tmp == 255)
        strout_lower(beyondpower);
      else
        moveitem(tmp, 255);
    }
    actmatch = 1;
    return;
  }
  if (verb == VERB_DROP) {		/* Drop */
    if (noun == 0)
      strout_lower(whatstr);
    else {
      tmp = autonoun(255);
      if (tmp == 255)
        strout_lower(beyondpower);
      else
        moveitem(tmp, game.location);
    }
    actmatch = 1;
    return;
  }
}

void process_light(void)
{
  uint8_t l;
  if ((l = game.objloc[LIGHT_SOURCE]) == 0)
    return;
  if (game.lighttime == 255)
    return;
  if (!--game.lighttime) {
    game.bitflags &= ~(ONEBIT << LIGHTOUT);	/* Check clear ! */
    if (l == 255 || l == game.location) {
      strout_lower(lightout);
      redraw |= REDRAW_MAYBE;
      return;
    }
  }
  if (game.lighttime > 25)
    return;
  strout_lower(lightoutin);
  decout_lower(game.lighttime);
  strout_lower(game.lighttime == 1 ? turn : turns);
}

void main_loop(void)
{
  uint8_t first = 1;
  char *p;

  action_look();
  
  while (1) {
    if (!first)
      process_light();
    else
      first = 0;
    verb = 0;
    noun = 0;

    run_table(status);

    if (redraw & REDRAW_MASK)
      action_look();
    strout_lower(whattodo);

    do {
      do {
        strout_lower(prompt);
        line_input(0);
        abbrevs();
        p = skip_spaces(linebuf);
      }
      while(*p == 0);

      scan_noun(p);
      if (noun && noun <= 6) {
        verb = VERB_GO;
        break;
      }
      scan_input();
      if (verb == 255)
        strout_lower(dontknow);
    } while (verb == 255);
    
    if (verb == VERB_GO) {
      if (!noun) {
        strout_lower(givedirn);
        continue;
      }
      if (noun <= 6) {
        uint8_t light = islight();
        uint8_t dir;

        if (!light)
          strout_lower(darkdanger);
        dir = locdata[game.location].exit[noun - 1];
        if (!dir) {
          if (!light) {
            strout_lower(brokeneck);
            action_delay();
            action_dead();
            continue;
          }
          strout_lower(cantgo);
          continue;
        }
        game.location = dir;
        redraw |= REDRAW;
        continue;
      }
    }
    linematch = 0;
    actmatch = 0;
    run_command();
    if (actmatch)
      continue;
    if (linematch) {
      strout_lower(notyet);
      continue;
    }
    strout_lower(dontunderstand);
  }
}

void start_game(void)
{
  memcpy(game.objloc, objinit, sizeof(game.objloc));
  game.bitflags = 0;
  game.counter = 0;
  memset(game.counter_array, 0, sizeof(game.counter_array));
  game.savedroom = 0;
  memset(game.roomsave, 0, sizeof(game.roomsave));
  game.location = startloc;
  game.lighttime = startlamp;
  game.carried = startcarried;
}

int main(int argc, char *argv[])
{
  display_init();
  setjmp(restart);
  strout_lower("Restore a saved game ? ");
  if (!yes_or_no() || !action_restore())
    start_game();
  main_loop();
}
