#define NUM_OBJ 66
#define WORDSIZE 4
#define GAME_MAGIC 427
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
const uint8_t startlamp = 150;
const uint8_t lightfill = 150;
const uint8_t startcarried = 3;
const uint8_t maxcar = 6;
const uint8_t treasure = 0;
const uint8_t treasures = 0;
const uint8_t lastloc = 36;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"\x43\x6F\x70\x79\x72\x69\x67\x68\x74\x20\x28\x63\x29\x20\x31\x39\x38\x32\x20\x42\x2E\x48\x6F\x77\x61\x72\x74\x68",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x66\x69\x65\x6C\x64",
 { 0, 2, 6, 12, 0, 0 } }, 
		{ 	"\x66\x69\x65\x6C\x64",
 { 1, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x74\x68\x65\x20\x72\x6F\x61\x64\x73\x69\x64\x65",
 { 0, 4, 0, 0, 0, 0 } }, 
		{ 	"\x4C\x61\x79\x2D\x62\x79",
 { 3, 17, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x73\x69\x64\x65\x20\x6D\x79\x20\x63\x61\x72",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x65\x6E\x63\x6C\x6F\x73\x75\x72\x65",
 { 0, 0, 22, 1, 0, 0 } }, 
		{ 	"\x54\x65\x6E\x74\x20\x65\x6E\x74\x72\x61\x6E\x63\x65",
 { 8, 0, 19, 9, 0, 0 } }, 
		{ 	"\x43\x69\x72\x63\x75\x73\x20\x72\x69\x6E\x67",
 { 23, 7, 13, 21, 0, 0 } }, 
		{ 	"\x53\x74\x6F\x72\x65\x20\x72\x6F\x6F\x6D",
 { 0, 0, 7, 33, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x74\x72\x61\x70\x65\x7A\x65\x20\x73\x74\x61\x6E\x64",
 { 0, 0, 0, 0, 0, 8 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x54\x72\x61\x70\x65\x7A\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x20\x6D\x61\x69\x6E\x74\x65\x6E\x61\x6E\x63\x65\x20\x77\x61\x67\x6F\x6E",
 { 0, 0, 1, 0, 0, 0 } }, 
		{ 	"\x43\x69\x72\x63\x75\x73\x20\x72\x69\x6E\x67",
 { 0, 0, 26, 8, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x54\x69\x67\x68\x74\x72\x6F\x70\x65\x20\x73\x74\x61\x6E\x64",
 { 0, 0, 0, 0, 0, 13 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x54\x69\x67\x68\x74\x72\x6F\x70\x65",
 { 16, 14, 36, 36, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x54\x69\x67\x68\x74\x72\x6F\x70\x65\x20\x73\x74\x61\x6E\x64",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x74\x68\x65\x20\x72\x65\x61\x72\x20\x6F\x66\x20\x6D\x79\x20\x43\x61\x72",
 { 4, 0, 0, 0, 0, 0 } }, 
		{ 	"\x4D\x61\x69\x6E\x74\x65\x6E\x61\x6E\x63\x65\x20\x57\x61\x67\x6F\x6E",
 { 0, 0, 12, 0, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 20, 7, 0, 0 } }, 
		{ 	"\x4C\x61\x72\x67\x65\x20\x54\x61\x6E\x6B",
 { 0, 0, 0, 19, 0, 0 } }, 
		{ 	"\x4D\x65\x73\x73\x20\x72\x6F\x6F\x6D",
 { 0, 0, 8, 0, 0, 0 } }, 
		{ 	"\x46\x69\x65\x6C\x64",
 { 0, 0, 0, 6, 0, 0 } }, 
		{ 	"\x43\x69\x72\x63\x75\x73\x20\x72\x69\x6E\x67",
 { 0, 8, 0, 24, 0, 0 } }, 
		{ 	"\x43\x69\x72\x63\x75\x73\x20\x52\x69\x6E\x67",
 { 0, 0, 23, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x43\x61\x6E\x6E\x6F\x6E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x43\x69\x72\x63\x75\x73\x20\x72\x69\x6E\x67",
 { 0, 0, 0, 13, 0, 0 } }, 
		{ 	"\x54\x69\x67\x65\x72\x27\x73\x20\x63\x61\x67\x65",
 { 0, 0, 0, 26, 0, 0 } }, 
		{ 	"\x67\x6C\x6F\x6F\x6D\x79\x20\x70\x61\x73\x73\x61\x67\x65",
 { 0, 29, 0, 0, 27, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x65\x71\x75\x69\x70\x6D\x65\x6E\x74\x20\x72\x6F\x6F\x6D",
 { 28, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x65\x68\x69\x6E\x64\x20\x61\x20\x50\x61\x6E\x65\x6C",
 { 29, 0, 0, 31, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x61\x6C\x63\x6F\x76\x65",
 { 0, 0, 30, 0, 0, 0 } }, 
		{ 	"\x43\x6C\x6F\x73\x65\x74",
 { 0, 33, 0, 0, 0, 0 } }, 
		{ 	"\x53\x74\x6F\x72\x65\x20\x72\x6F\x6F\x6D",
 { 0, 0, 9, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x68\x69\x67\x68\x20\x50\x6C\x61\x74\x66\x6F\x72\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x61\x66\x65\x74\x79\x20\x4E\x65\x74",
 { 0, 0, 0, 0, 0, 26 } }, 
		{ 	"\x2A\x53\x65\x6E\x64\x20\x69\x6E\x20\x74\x68\x65\x20\x43\x6C\x6F\x77\x6E\x73\x21\x20\x20\x49\x27\x6D\x20\x44\x45\x41\x44\x21",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	4,
	0,
	0,
	0,
	6,
	12,
	0,
	18,
	0,
	0,
	0,
	22,
	0,
	0,
	9,
	0,
	0,
	0,
	33,
	0,
	32,
	21,
	0,
	8,
	0,
	0,
	8,
	13,
	26,
	19,
	20,
	0,
	0,
	27,
	0,
	0,
	0,
	0,
	30,
	0,
	31,
	14,
	0,
	16,
	0,
	10,
	24,
	25,
	0,
	23,
	0,
	255,
	0,
	16,
	0,
	0,
	1,
	2,
	255,
	255,
	3,
	34,
	0,
	33,
	29,
	17,
};


const char *objtext[] = {
	"\x4D\x79\x20\x43\x61\x72",
	"\x46\x6C\x61\x73\x68\x6C\x69\x67\x68\x74",
	"\x44\x65\x61\x64\x20\x46\x6C\x61\x73\x68\x6C\x69\x67\x68\x74",
	"\x53\x70\x61\x6E\x6E\x65\x72",
	"\x47\x65\x6E\x65\x72\x61\x74\x6F\x72",
	"\x44\x6F\x6F\x72",
	"\x4F\x70\x65\x6E\x20\x44\x6F\x6F\x72",
	"\x4C\x6F\x63\x6B\x65\x72",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x4C\x6F\x63\x6B\x65\x72",
	"\x4C\x49\x54\x20\x46\x6C\x61\x73\x68\x6C\x69\x67\x68\x74",
	"\x48\x61\x63\x6B\x73\x61\x77",
	"\x53\x68\x6F\x76\x65\x6C",
	"\x53\x74\x61\x72\x74\x69\x6E\x67\x20\x48\x61\x6E\x64\x6C\x65",
	"\x48\x65\x61\x76\x79\x20\x49\x72\x6F\x6E\x20\x42\x61\x72\x73",
	"\x43\x68\x65\x73\x74",
	"\x4F\x70\x65\x6E\x20\x43\x68\x65\x73\x74",
	"\x54\x68\x69\x6E\x20\x53\x6C\x69\x70\x70\x65\x72\x73",
	"\x53\x6C\x69\x70\x70\x65\x72\x73\x20\x28\x77\x6F\x72\x6E\x29",
	"\x43\x6C\x6F\x77\x6E\x27\x73\x20\x43\x6F\x73\x74\x75\x6D\x65",
	"\x43\x6C\x6F\x77\x6E\x27\x73\x20\x43\x6F\x73\x74\x75\x6D\x65\x20\x28\x77\x6F\x72\x6E\x29",
	"\x54\x72\x61\x69\x6E\x65\x72\x27\x73\x20\x57\x68\x69\x70",
	"\x46\x72\x65\x65\x7A\x65\x72",
	"\x46\x69\x73\x68",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x52\x6F\x70\x65",
	"\x53\x61\x66\x65\x74\x79\x20\x4E\x65\x74",
	"\x53\x61\x66\x65\x74\x79\x20\x4E\x65\x74\x20\x28\x65\x72\x65\x63\x74\x65\x64\x29",
	"\x4C\x61\x64\x64\x65\x72",
	"\x4C\x61\x64\x64\x65\x72",
	"\x43\x61\x67\x65",
	"\x4C\x61\x72\x67\x65\x20\x54\x61\x6E\x6B",
	"\x57\x61\x74\x65\x72",
	"\x53\x65\x61\x2D\x4C\x69\x6F\x6E",
	"\x53\x6E\x6F\x72\x6B\x65\x6C",
	"\x53\x6C\x65\x65\x70\x69\x6E\x67\x20\x54\x69\x67\x65\x72",
	"\x41\x6E\x67\x72\x79\x20\x54\x69\x67\x65\x72",
	"\x54\x72\x61\x70\x64\x6F\x6F\x72",
	"\x42\x75\x74\x74\x6F\x6E",
	"\x4F\x70\x65\x6E\x20\x48\x61\x74\x63\x68",
	"\x54\x77\x6F\x20\x54\x65\x72\x6D\x69\x6E\x61\x6C\x73",
	"\x53\x68\x6F\x72\x74\x65\x64\x20\x54\x65\x72\x6D\x69\x6E\x61\x6C\x73",
	"\x42\x6C\x75\x65\x70\x72\x69\x6E\x74",
	"\x53\x74\x65\x65\x6C\x20\x54\x69\x67\x68\x74\x72\x6F\x70\x65",
	"\x53\x74\x65\x65\x6C\x20\x43\x61\x62\x6C\x65",
	"\x4D\x65\x74\x61\x6C\x20\x42\x61\x72",
	"\x48\x6F\x6C\x65\x20\x69\x6E\x20\x43\x61\x6E\x76\x61\x73",
	"\x54\x72\x61\x70\x65\x7A\x65",
	"\x48\x75\x67\x65\x20\x43\x61\x6E\x6E\x6F\x6E",
	"\x53\x6D\x61\x6C\x6C\x20\x4C\x65\x76\x65\x72",
	"\x53\x69\x67\x6E",
	"\x43\x6C\x6F\x77\x6E",
	"\x4E\x6F\x74\x65",
	"\x50\x65\x74\x72\x6F\x6C\x20\x43\x61\x6E\x20\x28\x45\x6D\x70\x74\x79\x29",
	"\x50\x65\x74\x72\x6F\x6C\x20\x43\x61\x6E\x20\x28\x66\x75\x6C\x6C\x29",
	"\x54\x69\x67\x68\x74\x72\x6F\x70\x65\x20\x61\x6E\x63\x68\x6F\x72\x61\x67\x65",
	"\x43\x72\x61\x73\x68\x20\x48\x65\x6C\x6D\x65\x74",
	"\x43\x72\x61\x73\x68\x20\x48\x65\x6C\x6D\x65\x74\x20\x28\x77\x6F\x72\x6E\x29",
	"\x43\x69\x72\x63\x75\x73\x20\x54\x65\x6E\x74",
	"\x52\x6F\x61\x64",
	"\x43\x61\x72\x20\x4B\x65\x79\x73",
	"\x50\x65\x6E\x6B\x6E\x69\x66\x65",
	"\x46\x69\x65\x6C\x64",
	"\x43\x61\x6E\x76\x61\x73\x20\x72\x6F\x6F\x66",
	"\x4E\x6F\x74\x65",
	"\x43\x6C\x6F\x73\x65\x74",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x50\x61\x6E\x65\x6C",
	"\x50\x65\x74\x72\x6F\x6C\x20\x66\x69\x6C\x6C\x65\x72\x20\x68\x6F\x6C\x65",
};
const char *msgptr[] = {
	"",
	"\x52\x6F\x6C\x6C\x20\x75\x70\x21\x20\x52\x6F\x6C\x6C\x20\x75\x70\x21",
	"\x54\x68\x65\x20\x43\x49\x52\x43\x55\x53\x20\x69\x73\x20\x69\x6E\x20\x74\x6F\x77\x6E\x21",
	"\x49\x20\x73\x65\x65",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C",
	"\x49\x74\x20\x73\x61\x79\x73\x3A\x2D",
	"\x53\x6F\x72\x72\x79",
	"\x49\x74\x27\x73\x20\x6F\x75\x74\x20\x6F\x66\x20\x66\x75\x65\x6C\x21",
	"\x42\x2D\x6F\x2D\x6F\x2D\x6F\x2D\x6F\x2D\x6D\x2D\x6D\x21",
	"\x53\x2D\x70\x2D\x6C\x2D\x61\x2D\x61\x2D\x74\x2D\x74\x21\x21",
	"\x49\x74\x27\x73\x20\x6C\x6F\x63\x6B\x65\x64\x21",
	"\x49\x27\x76\x65\x20\x6E\x6F\x20\x6B\x65\x79\x73\x21",
	"\x4E\x6F\x74\x68\x69\x6E\x67",
	"\x48\x61\x70\x70\x65\x6E\x65\x64\x21",
	"\x48\x65\x61\x76\x79\x20\x69\x72\x6F\x6E\x20\x62\x61\x72\x73\x20\x63\x72\x61\x73\x68\x20\x64\x6F\x77\x6E\x21",
	"\x49\x27\x6D\x20\x54\x72\x61\x70\x70\x65\x64\x21",
	"\x4F\x2E\x6B\x2E",
	"\x49\x20\x73\x6D\x61\x73\x68\x20\x6D\x79\x20\x68\x65\x61\x64\x20\x6F\x6E\x20\x6C\x61\x6E\x64\x69\x6E\x67\x21",
	"\x49\x27\x6D\x20\x44\x45\x41\x44\x21",
	"\x54\x68\x65\x20\x43\x68\x65\x73\x74\x20\x6C\x69\x64\x20\x66\x6C\x69\x65\x73\x20\x6F\x70\x65\x6E\x21",
	"\x54\x68\x65\x20\x54\x69\x67\x65\x72",
	"\x63\x6F\x77\x65\x72\x73\x20\x61\x77\x61\x79\x21",
	"\x54\x68\x65\x20\x53\x65\x61\x2D\x4C\x69\x6F\x6E\x20\x74\x6F\x73\x73\x65\x73\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21",
	"\x54\x68\x65\x20\x43\x6C\x6F\x77\x6E\x20\x72\x75\x6E\x73\x20\x61\x77\x61\x79\x20\x66\x72\x6F\x6D\x20\x6D\x65",
	"\x22\x59\x6F\x75\x20\x6D\x75\x73\x74\x20\x64\x65\x73\x74\x72\x6F\x79\x20\x74\x68\x69\x73\x20\x70\x6C\x61\x63\x65\x22",
	"\x22\x57\x65\x20\x61\x72\x65\x20\x53\x70\x69\x72\x69\x74\x73\x20\x2D\x20\x44\x6F\x6F\x6D\x65\x64\x20\x62\x79\x20\x45\x76\x69\x6C\x22",
	"\x22\x44\x6F\x6F\x6D\x65\x64\x20\x66\x6F\x72\x65\x76\x65\x72\x20\x74\x6F\x20\x74\x68\x69\x73\x20\x43\x69\x72\x63\x75\x73\x22",
	"\x48\x65\x20\x70\x6F\x69\x6E\x74\x73\x20\x74\x6F\x20\x74\x68\x65\x20\x66\x6C\x6F\x6F\x72\x21",
	"\x22\x44\x65\x74\x6F\x6E\x61\x74\x69\x6F\x6E\x20\x70\x72\x69\x6D\x65\x64\x20\x77\x68\x65\x6E\x20\x54\x65\x72\x6D\x69\x6E\x61\x6C\x73\x22",
	"\x22\x4F\x6E\x20\x63\x6F\x6E\x74\x72\x6F\x6C\x20\x70\x61\x6E\x65\x6C\x20\x61\x72\x65\x20\x73\x68\x6F\x72\x74\x65\x64\x22",
	"\x49\x20\x77\x61\x6C\x6B\x65\x64\x20\x6F\x66\x66\x20\x74\x68\x65\x20\x54\x69\x67\x68\x74\x72\x6F\x70\x65\x21",
	"\x22\x41\x55\x54\x4F\x2D\x44\x45\x54\x4F\x4E\x41\x54\x49\x4F\x4E\x20\x50\x52\x49\x4D\x45\x44\x22",
	"\x42\x65\x6C\x6C\x73\x20\x61\x6E\x64\x20\x73\x69\x72\x65\x6E\x73\x20\x61\x72\x65\x20\x73\x6F\x75\x6E\x64\x69\x6E\x67\x21",
	"\x49\x20\x73\x6C\x69\x70\x70\x65\x64\x20\x6F\x66\x66\x20\x74\x68\x65\x20\x54\x69\x67\x68\x74\x72\x6F\x70\x65\x21",
	"\x57\x68\x65\x65\x65\x65\x65\x65\x21",
	"\x49\x27\x6D\x20\x73\x70\x6C\x61\x74\x74\x65\x72\x65\x64\x20\x61\x67\x61\x69\x6E\x73\x74\x20\x74\x68\x65\x20\x43\x61\x6E\x76\x61\x73\x21",
	"\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67\x20\x69\x74\x21",
	"\x49\x20\x6E\x65\x65\x64\x20\x50\x65\x74\x72\x6F\x6C\x21",
	"\x47\x65\x6E\x65\x72\x61\x74\x6F\x72\x20\x77\x6F\x6E\x27\x74\x20\x73\x74\x61\x72\x74",
	"\x47\x65\x6E\x65\x72\x61\x74\x6F\x72\x20\x73\x70\x6C\x75\x74\x74\x65\x72\x73\x20\x69\x6E\x74\x6F\x20\x6C\x69\x66\x65\x21",
	"\x54\x68\x65\x20\x62\x61\x74\x74\x65\x72\x79\x20\x69\x73\x20\x66\x6C\x61\x74\x21",
	"\x49\x20\x64\x72\x69\x76\x65\x20\x61\x77\x61\x79\x20\x61\x74\x20\x66\x75\x6C\x6C\x20\x73\x70\x65\x65\x64",
	"\x61\x6E\x64\x20\x61\x76\x6F\x69\x64\x20\x74\x68\x65\x20\x65\x78\x70\x6C\x6F\x73\x69\x6F\x6E\x2E\x20\x49\x20\x73\x75\x63\x63\x65\x65\x64\x65\x64\x21",
	"\x43\x6C\x6F\x77\x6E\x20\x73\x61\x79\x73\x3A\x20\x22\x4E\x6F\x74\x20\x48\x65\x72\x65\x21\x22",
	"\x43\x6C\x6F\x77\x6E\x20\x68\x65\x6C\x70\x73\x20\x6D\x65\x20\x65\x72\x65\x63\x74\x20\x74\x68\x65\x20\x4E\x65\x74",
	"\x49\x74\x73\x20\x74\x6F\x6F\x20\x68\x65\x61\x76\x79\x21",
	"\x22\x4D\x41\x52\x56\x4F\x20\x2D\x20\x54\x48\x45\x20\x48\x55\x4D\x41\x4E\x20\x43\x41\x4E\x4E\x4F\x4E\x42\x41\x4C\x4C\x22",
	"\x48\x75\x67\x65\x20\x65\x78\x70\x6C\x6F\x73\x69\x6F\x6E",
	"\x52\x69\x70\x73\x20\x6D\x65\x20\x74\x6F\x20\x70\x65\x69\x63\x65\x73\x21",
	"\x22\x44\x45\x54\x4F\x4E\x41\x54\x49\x4F\x4E\x20\x49\x4E",
	"\x54\x55\x52\x4E\x53\x22",
	"\x43\x6C\x6F\x77\x6E\x20\x72\x75\x6E\x73\x20\x6F\x66\x66\x21",
	"\x49\x27\x6D\x20\x6E\x6F\x74\x20\x63\x61\x72\x72\x79\x69\x6E\x67\x20\x69\x74\x21",
	"\x57\x68\x61\x74\x20\x61\x20\x66\x6F\x75\x6C\x20\x6D\x6F\x75\x74\x68\x20\x79\x6F\x75\x20\x68\x61\x76\x65\x21",
	"\x55\x73\x65\x20\x79\x6F\x75\x72\x20\x77\x69\x74\x73\x20\x61\x6E\x64\x20\x45\x58\x41\x4D\x69\x6E\x65\x20\x74\x68\x69\x6E\x67\x73\x21",
	"\x59\x6F\x75\x20\x74\x68\x69\x6E\x6B\x20\x74\x68\x69\x73\x20\x69\x73\x20\x61\x20\x67\x61\x6D\x65\x20\x3F",
	"\x55\x67\x68\x21",
	"\x49\x27\x76\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x74\x6F\x20\x63\x75\x74\x20\x69\x74\x20\x77\x69\x74\x68\x21",
	"\x49\x74\x20\x66\x65\x6C\x6C\x20\x74\x6F\x20\x74\x68\x65\x20\x67\x72\x6F\x75\x6E\x64\x21",
	"\x49\x74\x27\x73\x20\x61\x6C\x72\x65\x61\x64\x79\x20\x6C\x69\x74\x21",
	"\x49\x74\x27\x73\x20\x6E\x6F\x74\x20\x6C\x69\x74\x21",
	"\x54\x68\x65\x20\x77\x61\x79\x20\x69\x73\x20\x62\x61\x72\x72\x65\x64\x21",
	"\x49\x74\x27\x73\x20\x61\x6C\x72\x65\x61\x64\x79\x20\x72\x75\x6E\x6E\x69\x6E\x67\x21",
	"\x49\x74\x20\x6E\x65\x65\x64\x73\x20\x66\x69\x78\x69\x6E\x67\x20\x2D\x20\x61\x20\x63\x61\x62\x6C\x65\x20\x69\x73\x20\x6D\x69\x73\x73\x69\x6E\x67\x21",
	"\x48\x65\x20\x6C\x6F\x6F\x6B\x73\x20\x68\x75\x6E\x67\x72\x79\x21",
	"\x59\x6F\x75\x27\x72\x65\x20\x6A\x6F\x6B\x69\x6E\x67\x21",
	"\x46\x6F\x72\x63\x65\x64\x20\x69\x74\x20\x6F\x70\x65\x6E\x21",
	"\x4D\x79\x20\x74\x61\x6E\x6B\x20\x69\x73\x20\x6E\x6F\x77\x20\x66\x75\x6C\x6C\x21",
	"\x49\x20\x63\x61\x6E\x6E\x6F\x74\x20\x6C\x65\x61\x76\x65\x20\x74\x68\x65\x20\x73\x70\x69\x72\x69\x74\x73\x20\x74\x6F\x20\x73\x75\x66\x66\x65\x72\x21",
	"\x54\x72\x79\x20\x74\x6F\x20\x62\x65\x20\x73\x65\x72\x69\x6F\x75\x73\x21",
	"\x4B\x65\x79\x73\x20\x64\x6F\x6E\x27\x74\x20\x66\x69\x74\x21",
	"\x48\x6F\x77\x20\x64\x65\x73\x74\x72\x75\x63\x74\x69\x76\x65\x21",
};


const uint8_t status[] = {
	171, 
	9, 1, 0, 1, 
	1, 86, 2, 58, 
	169, 
	4, 7, 9, 2, 
	56, 64, 
	169, 
	4, 7, 8, 2, 
	57, 64, 
	165, 
	4, 1, 
	57, 64, 
	170, 
	8, 3, 0, 1, 
	81, 77, 73, 
	203, 
	19, 0, 7, 5, 
	9, 48, 49, 61, 
	199, 
	16, 0, 
	33, 32, 50, 78, 
	201, 
	16, 0, 0, 1, 
	81, 51, 
	168, 
	4, 15, 0, 4, 
	58, 
	168, 
	4, 16, 0, 4, 
	60, 
	168, 
	4, 14, 0, 4, 
	60, 
	178, 
	4, 15, 6, 17, 14, 25, 0, 4, 
	60, 61, 34, 
	177, 
	4, 15, 6, 17, 13, 25, 0, 35, 
	34, 54, 
	174, 
	4, 36, 8, 4, 14, 25, 
	31, 61, 63, 
	177, 
	4, 36, 8, 4, 13, 25, 0, 35, 
	31, 54, 
	173, 
	13, 25, 13, 49, 0, 49, 
	59, 102, 
	182, 
	6, 19, 2, 49, 4, 23, 0, 49, 0, 8, 
	88, 102, 62, 
	172, 
	1, 19, 14, 25, 0, 49, 
	53, 
	168, 
	4, 36, 9, 4, 
	63, 
	179, 
	4, 7, 8, 2, 14, 13, 0, 13, 
	53, 64, 15, 16, 
	172, 
	8, 16, 14, 2, 0, 2, 
	53, 
	182, 
	6, 19, 2, 49, 4, 8, 0, 49, 0, 23, 
	88, 102, 62, 
	172, 
	4, 16, 14, 42, 0, 41, 
	53, 
	172, 
	4, 14, 14, 42, 0, 41, 
	53, 
};
const uint8_t actions[] = {
	21, 41, 76, 
	4, 34, 14, 44, 1, 59, 0, 44, 0, 61, 
	72, 17, 
	9, 10, 71, 
	2, 54, 0, 54, 
	52, 17, 
	4, 10, 71, 
	1, 55, 
	37, 
	9, 10, 51, 
	2, 18, 0, 18, 
	52, 17, 
	4, 10, 51, 
	1, 19, 
	37, 
	9, 10, 14, 
	2, 16, 0, 16, 
	52, 17, 
	4, 10, 14, 
	1, 17, 
	37, 
	4, 18, 71, 
	1, 55, 
	37, 
	9, 18, 71, 
	1, 54, 0, 54, 
	53, 17, 
	4, 18, 71, 
	6, 54, 
	103, 
	4, 18, 14, 
	1, 17, 
	37, 
	9, 18, 14, 
	1, 16, 0, 16, 
	53, 17, 
	4, 18, 14, 
	6, 16, 
	103, 
	4, 18, 51, 
	1, 19, 
	37, 
	9, 18, 51, 
	1, 18, 0, 18, 
	53, 17, 
	4, 18, 51, 
	6, 18, 
	103, 
	13, 1, 7, 
	2, 56, 14, 13, 0, 7, 
	54, 17, 
	9, 1, 92, 
	4, 2, 0, 3, 
	54, 17, 
	13, 1, 20, 
	1, 58, 2, 0, 0, 5, 
	54, 17, 
	9, 1, 64, 
	2, 41, 0, 15, 
	54, 17, 
	9, 51, 60, 
	4, 13, 0, 14, 
	54, 17, 
	9, 51, 60, 
	4, 8, 0, 10, 
	54, 17, 
	8, 62, 59, 
	7, 26, 5, 49, 
	7, 
	12, 62, 59, 
	1, 24, 2, 49, 7, 26, 
	44, 
	12, 62, 59, 
	1, 24, 4, 26, 5, 49, 
	46, 
	14, 34, 58, 
	3, 23, 0, 23, 0, 24, 
	72, 3, 4, 
	9, 10, 59, 
	2, 24, 0, 24, 
	52, 17, 
	9, 18, 59, 
	1, 24, 0, 24, 
	53, 17, 
	8, 41, 64, 
	2, 59, 6, 10, 
	108, 
	23, 41, 64, 
	2, 41, 1, 10, 0, 41, 0, 42, 0, 13, 
	59, 62, 17, 109, 
	23, 62, 59, 
	3, 24, 2, 49, 4, 26, 0, 24, 0, 25, 
	59, 53, 64, 45, 
	13, 26, 14, 
	1, 16, 0, 16, 0, 17, 
	72, 17, 
	13, 26, 51, 
	1, 18, 0, 18, 0, 19, 
	72, 17, 
	13, 26, 71, 
	1, 54, 0, 54, 0, 55, 
	72, 17, 
	13, 78, 29, 
	1, 1, 0, 1, 0, 9, 
	72, 17, 
	4, 78, 29, 
	13, 9, 
	110, 
	14, 79, 29, 
	3, 9, 0, 9, 0, 1, 
	72, 17, 64, 
	4, 79, 29, 
	14, 9, 
	111, 
	23, 24, 22, 
	4, 17, 14, 3, 1, 58, 0, 3, 0, 1, 
	53, 53, 3, 4, 
	8, 24, 22, 
	4, 17, 6, 58, 
	12, 
	8, 18, 7, 
	4, 7, 2, 13, 
	112, 
	9, 18, 7, 
	4, 7, 0, 1, 
	54, 17, 
	5, 24, 15, 
	2, 14, 
	13, 14, 
	13, 94, 15, 
	4, 9, 9, 8, 0, 8, 
	20, 58, 
	9, 18, 20, 
	4, 5, 0, 4, 
	54, 17, 
	9, 1, 83, 
	4, 3, 0, 2, 
	54, 17, 
	4, 48, 0, 
	6, 11, 
	7, 
	14, 48, 0, 
	4, 22, 14, 12, 0, 12, 
	53, 3, 4, 
	0, 48, 0, 
	7, 
	9, 45, 35, 
	4, 6, 6, 12, 
	7, 39, 
	4, 45, 35, 
	8, 2, 
	113, 
	8, 45, 35, 
	4, 6, 9, 5, 
	114, 
	8, 45, 35, 
	4, 6, 8, 6, 
	38, 
	23, 45, 35, 
	4, 6, 1, 12, 8, 5, 9, 6, 0, 2, 
	58, 17, 64, 40, 
	4, 78, 29, 
	13, 2, 
	41, 
	15, 83, 82, 
	2, 36, 14, 37, 0, 37, 
	53, 4, 14, 64, 
	9, 83, 82, 
	2, 36, 13, 37, 
	13, 14, 
	23, 64, 42, 
	2, 38, 1, 3, 0, 38, 0, 39, 0, 3, 
	17, 72, 59, 73, 
	211, 
	0, 3, 0, 1, 0, 25, 0, 1, 
	58, 81, 79, 81, 
	7, 33, 39, 
	1, 40, 
	17, 6, 29, 30, 
	7, 33, 94, 
	1, 62, 
	6, 25, 26, 27, 
	15, 34, 69, 
	2, 49, 14, 62, 0, 62, 
	53, 3, 4, 28, 
	4, 34, 69, 
	2, 49, 
	28, 
	15, 65, 0, 
	4, 20, 14, 31, 0, 31, 
	17, 3, 4, 53, 
	4, 65, 0, 
	4, 20, 
	17, 
	8, 34, 47, 
	2, 31, 14, 32, 
	115, 
	14, 30, 47, 
	1, 22, 0, 22, 0, 32, 
	59, 53, 23, 
	14, 34, 41, 
	4, 29, 14, 36, 0, 36, 
	53, 3, 4, 
	9, 34, 41, 
	4, 29, 13, 36, 
	3, 5, 
	9, 1, 81, 
	2, 37, 0, 30, 
	17, 54, 
	4, 34, 62, 
	4, 27, 
	116, 
	23, 92, 62, 
	4, 27, 9, 7, 0, 33, 0, 34, 0, 35, 
	72, 53, 88, 73, 
	195, 
	88, 21, 49, 61, 
	9, 50, 54, 
	1, 20, 7, 27, 
	5, 14, 
	22, 50, 54, 
	1, 20, 9, 7, 4, 27, 0, 7, 0, 35, 
	58, 53, 73, 
	203, 
	0, 33, 0, 34, 
	72, 17, 21, 22, 
	9, 1, 63, 
	2, 35, 0, 28, 
	54, 17, 
	19, 34, 73, 
	2, 46, 14, 48, 0, 48, 0, 54, 
	53, 53, 3, 4, 
	5, 33, 95, 
	2, 48, 
	6, 47, 
	9, 1, 73, 
	4, 24, 0, 25, 
	54, 17, 
	19, 81, 74, 
	2, 47, 1, 55, 13, 44, 0, 2, 
	9, 88, 54, 35, 
	15, 81, 74, 
	2, 47, 13, 44, 6, 55, 
	9, 88, 18, 61, 
	11, 81, 74, 
	2, 47, 14, 44, 
	9, 88, 36, 61, 
	21, 41, 75, 
	4, 34, 14, 44, 1, 59, 0, 44, 0, 61, 
	72, 17, 
	22, 34, 15, 
	2, 14, 14, 16, 14, 17, 8, 8, 0, 16, 
	53, 3, 4, 
	9, 1, 46, 
	2, 29, 0, 20, 
	17, 54, 
	14, 34, 18, 
	2, 21, 14, 22, 0, 22, 
	53, 3, 4, 
	9, 34, 35, 
	4, 6, 9, 5, 
	3, 114, 
	22, 95, 35, 
	9, 5, 1, 42, 1, 3, 0, 5, 0, 42, 
	17, 58, 59, 
	9, 1, 63, 
	2, 45, 0, 11, 
	17, 54, 
	18, 57, 63, 
	4, 11, 17, 45, 0, 34, 0, 45, 
	35, 54, 53, 
	18, 57, 63, 
	4, 11, 18, 45, 0, 10, 0, 45, 
	35, 54, 53, 
	11, 66, 0, 
	4, 16, 14, 25, 
	35, 88, 10, 61, 
	14, 66, 0, 
	4, 16, 13, 25, 0, 35, 
	35, 88, 54, 
	11, 66, 0, 
	4, 14, 14, 25, 
	35, 88, 10, 61, 
	14, 66, 0, 
	4, 14, 13, 25, 0, 35, 
	35, 88, 54, 
	11, 66, 0, 
	4, 10, 14, 25, 
	35, 88, 10, 61, 
	14, 66, 0, 
	4, 10, 13, 25, 0, 35, 
	35, 88, 54, 
	11, 66, 0, 
	4, 34, 14, 25, 
	35, 88, 10, 61, 
	14, 66, 0, 
	4, 34, 13, 25, 0, 35, 
	35, 88, 54, 
	0, 66, 0, 
	7, 
	8, 1, 11, 
	2, 5, 6, 43, 
	11, 
	18, 24, 11, 
	2, 5, 1, 43, 0, 5, 0, 6, 
	17, 117, 72, 
	9, 1, 11, 
	2, 6, 0, 18, 
	17, 54, 
	4, 34, 16, 
	2, 7, 
	11, 
	14, 34, 16, 
	2, 8, 14, 10, 0, 10, 
	53, 3, 4, 
	17, 24, 16, 
	2, 7, 1, 43, 0, 7, 0, 8, 
	72, 117, 
	8, 24, 16, 
	2, 7, 6, 43, 
	12, 
	21, 56, 24, 
	2, 4, 1, 51, 1, 32, 0, 51, 0, 52, 
	72, 17, 
	9, 43, 20, 
	4, 5, 9, 9, 
	7, 38, 
	15, 43, 20, 
	4, 5, 8, 9, 8, 3, 
	9, 42, 43, 63, 
	23, 54, 46, 
	4, 17, 1, 52, 0, 9, 0, 52, 0, 51, 
	17, 58, 72, 118, 
	9, 94, 15, 
	2, 14, 8, 8, 
	13, 14, 
	9, 1, 56, 
	2, 63, 0, 32, 
	17, 54, 
	9, 1, 61, 
	2, 28, 0, 27, 
	17, 54, 
	9, 10, 58, 
	2, 23, 0, 23, 
	52, 17, 
	8, 18, 58, 
	1, 23, 0, 23, 
	53, 
	4, 1, 7, 
	13, 13, 
	112, 
	12, 43, 20, 
	4, 5, 8, 9, 9, 3, 
	119, 
	13, 28, 14, 
	1, 17, 0, 16, 0, 17, 
	72, 17, 
	13, 28, 51, 
	1, 19, 0, 18, 0, 19, 
	72, 17, 
	13, 28, 71, 
	1, 55, 0, 54, 0, 55, 
	72, 17, 
	4, 1, 76, 
	4, 17, 
	120, 
	8, 25, 11, 
	4, 12, 1, 58, 
	121, 
	4, 1, 48, 
	4, 20, 
	17, 
	0, 74, 0, 
	122, 
	4, 54, 93, 
	1, 51, 
	38, 
	0, 9, 0, 
	106, 
	0, 7, 0, 
	63, 
	0, 23, 0, 
	66, 
	0, 8, 0, 
	71, 
	1, 34, 0, 
	3, 5, 
	0, 1, 0, 
	7, 
	0, 85, 0, 
	104, 
	3, 58, 0, 
	17, 85, 5, 14, 
	0, 24, 0, 
	7, 
	0, 67, 0, 
	107, 
	0, 6, 0, 
	105, 
	0, 94, 0, 
	122, 
	0, 72, 0, 
	7, 
	0, 51, 0, 
	7, 
	0, 33, 0, 
	7, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
197, 78, 84, 69,
215, 65, 76, 75,
210, 85, 78, 32,
208, 82, 79, 67,
72, 69, 76, 80,
81, 85, 73, 84,
83, 65, 86, 69,
83, 67, 79, 82,
71, 69, 84, 32,
212, 65, 75, 69,
208, 73, 67, 75,
199, 82, 65, 66,
211, 69, 73, 90,
200, 79, 76, 68,
207, 66, 84, 65,
211, 78, 65, 84,
68, 82, 79, 80,
204, 69, 65, 86,
196, 69, 80, 79,
208, 85, 84, 32,
212, 72, 82, 79,
73, 78, 86, 69,
79, 80, 69, 78,
85, 78, 76, 79,
87, 69, 65, 82,
196, 79, 78, 32,
82, 69, 77, 79,
196, 79, 70, 70,
70, 69, 69, 68,
71, 73, 86, 69,
207, 70, 70, 69,
82, 69, 65, 68,
69, 88, 65, 77,
204, 79, 79, 75,
201, 78, 83, 80,
208, 69, 82, 85,
211, 69, 65, 82,
83, 65, 87, 32,
200, 65, 67, 75,
67, 85, 84, 32,
211, 76, 73, 67,
68, 82, 73, 86,
211, 84, 69, 69,
83, 84, 65, 82,
84, 85, 82, 78,
195, 82, 65, 78,
68, 73, 71, 32,
211, 72, 79, 86,
67, 82, 65, 67,
67, 76, 73, 77,
193, 83, 67, 69,
70, 73, 82, 69,
70, 73, 76, 76,
210, 69, 80, 76,
83, 89, 80, 72,
83, 87, 73, 78,
83, 65, 89, 32,
193, 83, 75, 32,
210, 69, 77, 65,
212, 65, 76, 75,
69, 82, 69, 67,
69, 77, 80, 84,
83, 72, 79, 82,
83, 87, 73, 77,
74, 85, 77, 80,
69, 65, 84, 32,
212, 65, 83, 84,
206, 73, 66, 66,
77, 65, 75, 69,
205, 65, 78, 85,
75, 73, 76, 76,
193, 84, 84, 65,
83, 77, 65, 83,
194, 82, 69, 65,
196, 69, 83, 84,
210, 85, 73, 78,
76, 73, 71, 72,
85, 78, 76, 73,
197, 88, 84, 73,
80, 85, 76, 76,
212, 85, 71, 32,
80, 85, 83, 72,
208, 82, 69, 83,
70, 85, 67, 75,
208, 73, 83, 83,
211, 72, 73, 84,
194, 79, 76, 76,
194, 65, 83, 84,
195, 85, 78, 84,
194, 85, 71, 71,
77, 79, 86, 69,
211, 72, 73, 70,
75, 73, 67, 75,
70, 73, 88, 32,
210, 69, 80, 65,
205, 69, 78, 68,
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
84, 69, 78, 84,
195, 73, 82, 67,
205, 65, 82, 81,
87, 65, 71, 79,
68, 79, 79, 82,
72, 65, 67, 75,
211, 65, 87, 32,
83, 76, 73, 80,
67, 72, 69, 83,
76, 79, 67, 75,
195, 85, 80, 66,
82, 69, 70, 82,
198, 82, 69, 69,
67, 65, 82, 32,
193, 85, 84, 79,
66, 79, 79, 84,
212, 82, 85, 78,
80, 69, 84, 82,
199, 65, 83, 79,
198, 85, 69, 76,
83, 80, 65, 78,
215, 82, 69, 78,
70, 76, 65, 83,
212, 79, 82, 67,
204, 65, 77, 80,
75, 69, 89, 83,
71, 65, 85, 71,
205, 69, 84, 69,
71, 69, 78, 69,
83, 72, 79, 86,
72, 65, 78, 68,
211, 84, 65, 82,
66, 76, 85, 69,
65, 76, 67, 79,
80, 65, 78, 69,
84, 69, 82, 77,
197, 76, 69, 67,
69, 81, 85, 73,
80, 65, 83, 83,
84, 65, 78, 75,
83, 69, 65, 45,
87, 65, 84, 69,
83, 78, 79, 82,
66, 65, 82, 83,
67, 79, 83, 84,
207, 85, 84, 70,
195, 76, 79, 84,
87, 72, 73, 80,
212, 82, 65, 73,
67, 76, 79, 83,
70, 73, 83, 72,
82, 79, 80, 69,
78, 69, 84, 32,
76, 65, 68, 68,
67, 65, 71, 69,
84, 73, 71, 69,
84, 82, 65, 80,
84, 73, 71, 72,
67, 65, 66, 76,
200, 65, 87, 83,
84, 82, 65, 80,
211, 87, 73, 78,
67, 76, 79, 87,
202, 69, 83, 84,
72, 69, 76, 77,
195, 82, 65, 83,
67, 65, 78, 78,
76, 69, 86, 69,
67, 65, 78, 86,
72, 79, 76, 69,
66, 65, 82, 32,
195, 82, 79, 87,
76, 65, 89, 66,
83, 84, 65, 78,
72, 65, 84, 67,
66, 85, 84, 84,
70, 73, 69, 76,
79, 70, 70, 32,
217, 79, 85, 32,
65, 82, 79, 85,
210, 79, 85, 78,
82, 73, 78, 71,
80, 69, 78, 75,
203, 78, 73, 70,
71, 65, 77, 69,
82, 79, 65, 68,
67, 65, 78, 32,
78, 79, 84, 69,
83, 73, 71, 78,
65, 78, 67, 72,
32, 32, 32, 32,
	0,
};
const uint8_t automap[] = {
70, 76, 65, 83,
	1,
70, 76, 65, 83,
	2,
83, 80, 65, 78,
	3,
70, 76, 65, 83,
	9,
72, 65, 67, 75,
	10,
83, 72, 79, 86,
	11,
72, 65, 78, 68,
	12,
87, 72, 73, 80,
	20,
70, 73, 83, 72,
	22,
83, 78, 79, 82,
	32,
66, 76, 85, 69,
	40,
67, 65, 66, 76,
	42,
66, 65, 82, 32,
	43,
78, 79, 84, 69,
	50,
67, 65, 78, 32,
	51,
67, 65, 78, 32,
	52,
75, 69, 89, 83,
	58,
80, 69, 78, 75,
	59,
78, 79, 84, 69,
	62,
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
