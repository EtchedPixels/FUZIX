#define NUM_OBJ 78
#define WORDSIZE 4
#define GAME_MAGIC 223
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
const uint8_t startlamp = 175;
const uint8_t lightfill = 175;
const uint8_t startcarried = 0;
const uint8_t maxcar = 7;
const uint8_t treasure = 15;
const uint8_t treasures = 13;
const uint8_t lastloc = 38;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x47\x68\x6F\x73\x74\x20\x54\x6F\x77\x6E",
 { 0, 0, 17, 2, 0, 0 } }, 
		{ 	"\x47\x68\x6F\x73\x74\x20\x54\x6F\x77\x6E",
 { 0, 0, 1, 3, 0, 0 } }, 
		{ 	"\x47\x68\x6F\x73\x74\x20\x54\x6F\x77\x6E",
 { 0, 0, 2, 4, 0, 0 } }, 
		{ 	"\x47\x68\x6F\x73\x74\x20\x54\x6F\x77\x6E",
 { 0, 0, 3, 6, 0, 0 } }, 
		{ 	"\x73\x74\x61\x62\x6C\x65",
 { 0, 4, 0, 0, 0, 0 } }, 
		{ 	"\x72\x6F\x61\x64",
 { 0, 0, 4, 7, 0, 0 } }, 
		{ 	"\x66\x6F\x72\x6B\x20\x69\x6E\x20\x74\x68\x65\x20\x72\x6F\x61\x64",
 { 9, 8, 6, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x72\x69\x64\x67\x65\x20\x61\x62\x6F\x76\x65\x20\x61\x20\x6E\x61\x72\x72\x6F\x77\x20\x72\x61\x76\x69\x6E\x65\x20\x49\x20\x73\x65\x65\x0A\x6D\x6F\x75\x6E\x74\x61\x69\x6E\x73\x20\x69\x6E\x20\x74\x68\x65\x20\x64\x69\x73\x74\x61\x6E\x63\x65",
 { 7, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x66\x69\x65\x6C\x64",
 { 0, 7, 0, 0, 0, 0 } }, 
		{ 	"\x72\x61\x76\x69\x6E\x65",
 { 0, 0, 0, 0, 8, 0 } }, 
		{ 	"\x73\x61\x6C\x6F\x6F\x6E",
 { 0, 2, 0, 0, 0, 0 } }, 
		{ 	"\x68\x69\x64\x64\x65\x6E\x20\x6F\x66\x66\x69\x63\x65",
 { 0, 0, 11, 0, 0, 0 } }, 
		{ 	"\x4A\x61\x69\x6C",
 { 1, 0, 0, 0, 0, 0 } }, 
		{ 	"\x63\x65\x6C\x6C",
 { 13, 0, 0, 0, 0, 0 } }, 
		{ 	"\x44\x72\x79\x20\x47\x6F\x6F\x64\x73\x20\x73\x74\x6F\x72\x65",
 { 2, 0, 0, 0, 0, 0 } }, 
		{ 	"\x42\x61\x72\x62\x65\x72\x73\x68\x6F\x70",
 { 0, 1, 0, 0, 0, 0 } }, 
		{ 	"\x72\x6F\x61\x64",
 { 0, 0, 20, 1, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x72\x69\x64\x67\x65\x20\x61\x62\x6F\x76\x65\x20\x61\x20\x72\x61\x76\x69\x6E\x65",
 { 0, 0, 0, 32, 0, 0 } }, 
		{ 	"\x73\x74\x61\x6C\x6C",
 { 0, 5, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x42\x6F\x6F\x74\x20\x68\x69\x6C\x6C",
 { 0, 0, 0, 17, 0, 0 } }, 
		{ 	"\x4D\x69\x6E\x65",
 { 0, 0, 0, 0, 10, 22 } }, 
		{ 	"\x4D\x69\x6E\x65",
 { 0, 23, 0, 0, 21, 0 } }, 
		{ 	"\x4D\x69\x6E\x65",
 { 22, 0, 0, 0, 0, 0 } }, 
		{ 	"\x54\x65\x6C\x65\x67\x72\x61\x70\x68\x20\x6F\x66\x66\x69\x63\x65",
 { 0, 3, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x62\x61\x63\x6B\x20\x6F\x66\x20\x27\x4F\x6C\x65\x20\x50\x61\x69\x6E\x74",
 { 0, 0, 0, 0, 0, 19 } }, 
		{ 	"\x77\x61\x72\x6D\x20\x62\x65\x64",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x68\x6F\x74\x65\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 28, 0, 0 } }, 
		{ 	"\x6C\x6F\x62\x62\x79",
 { 3, 0, 27, 0, 0, 0 } }, 
		{ 	"\x67\x72\x61\x76\x65",
 { 0, 0, 0, 0, 20, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x66\x6C\x61\x74\x20\x6F\x6E\x20\x6D\x79\x20\x62\x61\x63\x6B\x20\x69\x6E\x20\x61\x20\x6D\x61\x6E\x75\x72\x65\x20\x70\x69\x6C\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x74\x6F\x72\x65\x72\x6F\x6F\x6D",
 { 19, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x6F\x75\x6E\x74\x61\x69\x6E\x20\x74\x72\x61\x69\x6C",
 { 0, 0, 18, 0, 0, 0 } }, 
		{ 	"\x6C\x69\x6E\x65\x20\x73\x68\x61\x63\x6B",
 { 0, 32, 0, 0, 0, 0 } }, 
		{ 	"\x72\x6F\x6F\x74\x20\x63\x65\x6C\x6C\x61\x72",
 { 0, 0, 0, 0, 33, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x65\x68\x69\x6E\x64\x20\x74\x68\x65\x20\x63\x6F\x75\x6E\x74\x65\x72",
 { 28, 0, 0, 0, 0, 0 } }, 
		{ 	"\x68\x69\x64\x64\x65\x6E\x20\x63\x61\x6E\x79\x6F\x6E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x74\x65\x65\x70\x65\x65",
 { 36, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x6F\x74\x20\x6F\x66\x20\x74\x72\x6F\x75\x62\x6C\x65\x21",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	1,
	1,
	2,
	4,
	5,
	5,
	15,
	0,
	0,
	0,
	10,
	0,
	10,
	15,
	11,
	19,
	0,
	0,
	15,
	19,
	0,
	11,
	0,
	0,
	0,
	22,
	21,
	13,
	0,
	35,
	13,
	2,
	15,
	25,
	16,
	0,
	0,
	0,
	0,
	0,
	3,
	0,
	0,
	3,
	20,
	0,
	29,
	29,
	0,
	24,
	0,
	0,
	0,
	24,
	35,
	0,
	12,
	0,
	31,
	0,
	0,
	0,
	33,
	0,
	32,
	0,
	0,
	34,
	28,
	0,
	0,
	0,
	14,
	36,
	0,
	37,
	37,
};


const char *objtext[] = {
	"\x57\x6F\x72\x6E\x20\x6F\x75\x74\x20\x66\x69\x64\x64\x6C\x65\x20\x73\x74\x72\x69\x6E\x67\x73",
	"\x42\x61\x72\x62\x65\x72\x73\x68\x6F\x70",
	"\x4A\x61\x69\x6C",
	"\x53\x61\x6C\x6F\x6F\x6E",
	"\x53\x74\x61\x62\x6C\x65",
	"\x53\x74\x61\x6C\x6C",
	"\x4D\x61\x6E\x75\x72\x65\x20\x70\x69\x6C\x65",
	"\x53\x68\x6F\x76\x65\x6C",
	"\x57\x68\x69\x74\x65\x20\x63\x72\x79\x73\x74\x61\x6C\x73",
	"\x4C\x69\x74\x20\x63\x61\x6E\x64\x6C\x65",
	"\x59\x65\x6C\x6C\x6F\x77\x20\x70\x6F\x77\x64\x65\x72",
	"\x53\x61\x67\x65\x62\x72\x75\x73\x68\x20\x63\x68\x61\x72\x63\x6F\x61\x6C",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x67\x75\x6E\x70\x6F\x77\x64\x65\x72",
	"\x45\x6E\x74\x72\x61\x6E\x63\x65\x20\x74\x6F\x20\x61\x20\x6D\x69\x6E\x65",
	"\x4D\x61\x74\x63\x68\x65\x73",
	"\x4D\x69\x72\x72\x6F\x72",
	"\x27\x6F\x6C\x65\x20\x50\x61\x69\x6E\x74",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x6D\x69\x72\x72\x6F\x72",
	"\x48\x6F\x6C\x65\x20\x69\x6E\x20\x74\x68\x65\x20\x77\x61\x6C\x6C",
	"\x43\x6F\x6D\x70\x61\x73\x73",
	"\x48\x6F\x72\x73\x65\x73\x68\x6F\x65",
	"\x4D\x61\x6C\x65\x20\x63\x6F\x77\x20\x6D\x61\x6E\x75\x72\x65",
	"\x4C\x61\x72\x67\x65\x20\x62\x65\x6C\x6C",
	"\x57\x72\x61\x69\x74\x68\x6C\x69\x6B\x65\x20\x66\x69\x67\x75\x72\x65\x20\x70\x6C\x61\x79\x69\x6E\x67\x20\x65\x71\x75\x61\x6C\x6C\x79\x20\x67\x68\x6F\x73\x74\x6C\x79\x20\x70\x69\x61\x6E\x6F",
	"\x50\x69\x61\x6E\x6F\x20\x77\x69\x74\x68\x20\x73\x65\x74\x20\x6F\x66\x20\x6B\x65\x79\x73",
	"\x4D\x61\x70",
	"\x55\x6E\x6C\x69\x74\x20\x63\x61\x6E\x64\x6C\x65",
	"\x2A\x53\x49\x4C\x56\x45\x52\x20\x42\x55\x4C\x4C\x45\x54\x2A",
	"\x2A\x47\x4F\x4C\x44\x45\x4E\x20\x44\x45\x52\x52\x49\x4E\x47\x45\x52\x2A",
	"\x4F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
	"\x53\x69\x67\x6E\x2D\x22\x52\x69\x6E\x67\x20\x66\x6F\x72\x20\x52\x4F\x4F\x4D\x20\x73\x65\x72\x76\x69\x63\x65\x22",
	"\x4C\x6F\x63\x6B\x65\x64\x20\x64\x6F\x6F\x72",
	"\x44\x72\x79\x2D\x47\x6F\x6F\x64\x73\x20\x73\x74\x6F\x72\x65",
	"\x53\x69\x67\x6E",
	"\x2A\x53\x49\x4C\x56\x45\x52\x20\x53\x50\x55\x52\x53\x2A",
	"\x53\x74\x65\x74\x73\x6F\x6E\x20\x68\x61\x74",
	"\x57\x68\x69\x63\x68\x20\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67",
	"\x53\x6D\x61\x6C\x6C\x20\x6B\x65\x79",
	"\x52\x6F\x6C\x6C\x20\x6F\x66\x20\x74\x61\x70\x65",
	"\x54\x61\x70\x65\x64\x20\x75\x70\x20\x6D\x69\x72\x72\x6F\x72",
	"\x2A\x47\x4F\x4C\x44\x20\x4E\x55\x47\x47\x45\x54\x2A",
	"\x54\x65\x6C\x65\x67\x72\x61\x70\x68\x20\x6F\x66\x66\x69\x63\x65",
	"\x45\x6D\x70\x74\x79\x20\x6D\x61\x74\x63\x68\x62\x6F\x6F\x6B",
	"\x42\x65\x64",
	"\x48\x6F\x74\x65\x6C",
	"\x52\x61\x74\x74\x6C\x65\x73\x6E\x61\x6B\x65",
	"\x4F\x70\x65\x6E\x20\x67\x72\x61\x76\x65",
	"\x2A\x47\x4F\x4C\x44\x20\x43\x4F\x49\x4E\x2A",
	"\x50\x75\x72\x70\x6C\x65\x20\x77\x6F\x72\x6D",
	"\x50\x75\x72\x70\x6C\x65\x20\x53\x6C\x69\x6D\x65",
	"\x54\x65\x6C\x65\x67\x72\x61\x70\x68\x20\x6B\x65\x79",
	"\x32\x20\x6C\x6F\x6F\x73\x65\x20\x77\x69\x72\x65\x73",
	"\x53\x70\x6C\x69\x63\x65\x64\x20\x77\x69\x72\x65",
	"\x50\x69\x65\x63\x65\x73\x20\x6F\x66\x20\x77\x69\x72\x65",
	"\x4C\x61\x72\x67\x65\x20\x73\x61\x66\x65",
	"\x2A\x43\x41\x53\x48\x42\x4F\x58\x2A",
	"\x2A\x20\x24\x32\x30\x30\x20\x2A",
	"\x2A\x4F\x52\x49\x45\x4E\x54\x41\x4C\x20\x47\x4F\x20\x42\x4F\x41\x52\x44\x2A",
	"\x48\x6F\x6C\x65\x20\x6B\x69\x63\x6B\x65\x64\x20\x69\x6E\x20\x77\x61\x6C\x6C\x20\x26\x20\x62\x61\x72\x65\x20\x68\x6F\x6F\x66\x20\x70\x72\x69\x6E\x74",
	"\x4B\x65\x67\x20\x6F\x66\x20\x6E\x61\x69\x6C\x73",
	"\x45\x6D\x70\x74\x79\x20\x6B\x65\x67",
	"\x4B\x65\x67\x20\x6F\x66\x20\x67\x75\x6E\x70\x6F\x77\x64\x65\x72",
	"\x4E\x61\x69\x6C\x73",
	"\x54\x65\x6C\x65\x67\x72\x61\x70\x68\x20\x6B\x65\x79",
	"\x53\x6D\x6F\x6B\x69\x6E\x67\x20\x6F\x70\x65\x6E\x20\x73\x61\x66\x65",
	"\x4C\x69\x6E\x65\x20\x73\x68\x61\x63\x6B",
	"\x50\x6C\x61\x6E\x6B",
	"\x48\x6F\x6C\x65\x20\x69\x6E\x20\x66\x6C\x6F\x6F\x72",
	"\x2A\x50\x45\x4C\x54\x53\x2A",
	"\x43\x6F\x75\x6E\x74\x65\x72",
	"\x47\x68\x6F\x73\x74\x6C\x79\x20\x73\x71\x75\x61\x72\x65\x20\x64\x61\x6E\x63\x65",
	"\x2A\x53\x49\x4C\x56\x45\x52\x20\x43\x55\x50\x2A",
	"\x2A\x42\x41\x47\x20\x47\x4F\x4C\x44\x20\x44\x55\x53\x54\x2A",
	"\x48\x61\x6D\x6D\x65\x72",
	"\x54\x65\x65\x50\x65\x65",
	"\x49\x6E\x64\x69\x61\x6E\x20\x67\x68\x6F\x73\x74",
	"\x2A\x53\x41\x43\x52\x45\x44\x20\x54\x4F\x4D\x20\x54\x4F\x4D\x2A",
	"\x2A\x54\x55\x52\x51\x55\x4F\x49\x53\x45\x20\x4E\x45\x43\x4B\x4C\x41\x43\x45\x2A",
};
const char *msgptr[] = {
	"",
	"\x4F\x4B",
	"\x49\x20\x73\x65\x65",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x39\x20\x22\x47\x48\x4F\x53\x54\x20\x54\x4F\x57\x4E\x22",
	"\x62\x79\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x20\x64\x65\x64\x69\x63\x61\x74\x65\x64\x3A\x20\x74\x68\x65\x20\x43\x68\x65\x72\x65\x6E\x73",
	"\x49\x27\x76\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x74\x6F\x20\x64\x6F\x20\x74\x68\x61\x74\x20\x77\x69\x74\x68",
	"\x53\x6F\x72\x72\x79\x20\x49\x20\x63\x61\x6E\x27\x74",
	"\x49\x20\x66\x6F\x75\x6E\x64",
	"\x57\x65\x61\x72\x3F",
	"\x42\x41\x52\x46\x21",
	"\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67",
	"\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C",
	"\x72\x65\x6D\x69\x6E\x64\x73\x20\x6D\x65\x20\x6F\x66",
	"\x59\x6F\x75\x27\x76\x65\x20\x6D\x61\x64\x65",
	"\x42\x4F\x4E\x55\x53\x20\x70\x6F\x69\x6E\x74\x73\x20\x6F\x75\x74\x20\x6F\x66\x20\x61\x20\x70\x6F\x73\x73\x69\x62\x6C\x65\x20\x35\x30\x20\x69\x6E",
	"\x69\x74\x73\x20\x66\x75\x6C\x6C\x20\x6F\x66\x20\x73\x61\x67\x65\x20\x62\x72\x75\x73\x68\x2C\x20\x74\x75\x6D\x62\x6C\x65\x77\x65\x65\x64\x20\x26\x20\x69\x73\x20\x69\x6D\x70\x61\x73\x73\x61\x62\x6C\x65",
	"\x6D\x6F\x76\x65\x73",
	"\x54\x68\x65\x20\x64\x72\x79\x20\x77\x61\x73\x68\x20\x69\x73\x20\x62\x75\x72\x6E\x74\x20\x63\x6C\x65\x61\x72",
	"\x66\x6C\x79\x69\x6E\x67\x20\x67\x6C\x61\x73\x73\x20\x73\x6C\x69\x63\x65\x73\x20\x6D\x65\x20\x75\x70",
	"\x43\x72\x61\x63\x6B\x21",
	"\x6E\x6F\x72\x74\x68",
	"\x69\x74\x73\x20\x70\x6F\x69\x6E\x74\x69\x6E\x67\x20\x74\x6F\x20\x74\x68\x65",
	"\x68\x6F\x72\x73\x65\x73\x68\x6F\x65",
	"\x44\x6F\x6F\x72\x73\x20\x63\x6C\x6F\x73\x65\x64\x2C\x20\x77\x69\x6E\x64\x6F\x77\x73\x20\x62\x61\x72\x72\x65\x64\x21",
	"\x77\x69\x74\x68\x20\x77\x68\x61\x74\x3F",
	"\x49\x6E\x73\x69\x64\x65\x20\x62\x6F\x6C\x74\x20\x69\x73\x20\x6C\x61\x74\x63\x68\x65\x64\x21",
	"\x4D\x61\x67\x6E\x65\x74\x21",
	"\x20\x64\x6F\x65\x73\x6E\x27\x74\x20\x73\x65\x65\x6D\x20\x74\x6F\x20\x64\x6F\x20\x69\x74\x21",
	"\x44\x69\x6E\x67\x2D\x44\x69\x6E\x67",
	"\x42\x65\x6C\x6C\x20\x72\x69\x6E\x67\x73\x20\x73\x6F\x6D\x65\x77\x68\x65\x72\x65",
	"\x4D\x79\x20\x68\x61\x6E\x64\x20\x70\x61\x73\x73\x65\x73\x20\x74\x68\x72\x75\x21",
	"\x47\x68\x6F\x73\x74\x20\x73\x74\x61\x6E\x64\x73\x2C\x20\x62\x6F\x77\x73\x2C\x20\x76\x61\x6E\x69\x73\x68\x65\x73\x21",
	"\x53\x69\x6C\x6C\x79\x2C\x20\x77\x72\x6F\x6E\x67\x20\x74\x79\x70\x65\x20\x6B\x65\x79\x73\x21",
	"\x4F\x64\x64",
	"\x67\x6F\x65\x73\x20\x74\x68\x75\x6E\x6B\x21",
	"\x66\x61\x6C\x6C\x73\x20\x6F\x75\x74",
	"\x43\x61\x6E\x64\x6C\x65\x20\x62\x6C\x65\x77\x20\x6F\x75\x74\x21",
	"\x76\x65\x72\x79\x20\x70\x72\x65\x74\x74\x79",
	"\x73\x68\x6F\x6F\x74\x73\x20\x73\x74\x72\x65\x61\x6D\x20\x6F\x66\x20\x77\x61\x74\x65\x72",
	"\x69\x74\x20\x61\x6C\x72\x65\x61\x64\x79\x20\x69\x73",
	"\x44\x72\x6F\x70\x20\x2A\x54\x52\x45\x41\x53\x55\x52\x45\x53\x2A\x20\x74\x68\x65\x6E\x20\x53\x43\x4F\x52\x45",
	"\x49\x20\x68\x65\x61\x72",
	"\x57\x68\x61\x74\x3F",
	"\x54\x6F\x6F\x20\x73\x74\x65\x65\x70\x21",
	"\x66\x65\x65\x6C\x73\x20\x73\x74\x72\x61\x6E\x67\x65",
	"\x69\x74\x73\x20\x6D\x79\x20\x73\x69\x7A\x65",
	"\x44\x6F\x6E\x27\x74\x20\x63\x6F\x6C\x6C\x65\x63\x74\x20\x24\x32\x30\x30\x20\x74\x68\x65\x6E\x20\x64\x6F\x6E\x27\x74\x20\x70\x61\x73\x73\x20\x47\x4F\x21\x20\x43\x4F\x4E\x54\x52\x41\x50\x4F\x53\x49\x54\x49\x56\x45\x2E",
	"\x68\x61\x70\x70\x65\x6E\x73",
	"\x48\x6F\x77\x3F",
	"\x49\x74\x73\x20\x6D\x69\x6E\x65\x2C\x20\x64\x69\x67\x20\x72\x6F\x6F\x66\x21",
	"\x47\x68\x6F\x73\x74\x6C\x79\x20\x76\x6F\x69\x63\x65\x20\x77\x68\x69\x73\x70\x65\x72\x73\x3A",
	"\x56\x61\x69\x6E\x2E\x2E\x2E",
	"\x4D\x61\x74\x63\x68\x20\x66\x6C\x61\x72\x65\x73\x20\x75\x70\x2E\x2E\x2E",
	"\x2E\x2E\x2E\x61\x6E\x64\x20\x74\x68\x65\x6E\x20\x67\x6F\x65\x73\x20\x6F\x75\x74",
	"\x49\x74\x73\x20\x67\x65\x74\x74\x69\x6E\x67",
	"\x53\x75\x6E\x73\x65\x74\x21",
	"\x49\x20\x66\x65\x65\x6C\x20\x72\x65\x66\x72\x65\x73\x68\x65\x64",
	"\x4E\x69\x67\x68\x74\x73\x20\x61\x72\x65\x20\x63\x6F\x6C\x64\x21\x20\x49\x20\x63\x61\x75\x67\x68\x74\x20\x50\x6E\x65\x75\x6D\x6F\x6E\x69\x61",
	"\x54\x72\x79\x3A\x20\x53\x4C\x45\x45\x50",
	"\x69\x74\x20\x77\x6F\x72\x6B\x65\x64\x21",
	"\x69\x74\x73\x20\x67\x6F\x6E\x65",
	"\x73\x6E\x61\x6B\x65\x20\x77\x6F\x6E\x27\x74\x20\x6C\x65\x74\x20\x6D\x65",
	"\x54\x68\x61\x74\x20\x66\x65\x6C\x74\x20\x67\x6F\x6F\x64\x21",
	"\x54\x6F\x6F\x20\x62\x69\x67\x21",
	"\x69\x74\x73\x20\x6C\x6F\x63\x6B\x65\x64\x21",
	"\x47\x6F\x6F\x64\x20\x4D\x6F\x72\x6E\x69\x6E\x67",
	"\x47\x6F\x6F\x64\x20\x6E\x69\x67\x68\x74",
	"\x49\x74\x73\x20\x66\x69\x72\x65\x70\x72\x6F\x6F\x66",
	"\x48\x65\x20\x77\x6F\x6E\x27\x74\x20\x62\x75\x64\x67\x65",
	"\x6C\x65\x66\x74\x21",
	"\x64\x61\x72\x6B\x21",
	"\x54\x68\x72\x75\x20\x74\x68\x65\x20\x48\x61\x74\x21",
	"\x49\x27\x6D\x20\x73\x6E\x61\x6B\x65\x20\x62\x69\x74",
	"\x49\x27\x6D\x20\x6E\x6F\x74\x20\x41\x6C\x69\x63\x65\x21",
	"\x48\x65\x20\x62\x75\x63\x6B\x73\x2C\x20\x49\x27\x6D\x20\x74\x68\x72\x6F\x77\x6E",
	"\x57\x68\x6F\x6F\x73\x68\x21",
	"\x54\x4F\x4F\x20\x43\x4C\x4F\x53\x45\x21\x20\x50\x69\x65\x63\x65\x73\x20\x6F\x66\x20\x6D\x65\x20\x72\x61\x69\x6E\x20\x64\x6F\x77\x6E\x20\x66\x6F\x72\x20\x64\x61\x79\x73\x21",
	"\x42\x6F\x6F\x6D\x21",
	"\x2E\x2E\x2E\x20\x2E\x2E\x2E\x2E\x20\x2E\x2D\x20\x20\x2D\x2E\x2D\x20\x20\x2E\x20\x20\x2D\x20\x2D\x2D\x2D\x20\x20\x2E\x2D\x2D\x2E\x20\x20\x2E\x2D\x2D\x2E\x20\x20\x2E\x20\x20\x2E\x2D\x2E",
	"\x53\x50\x41\x52\x4B\x21\x20\x2A\x20\x2A\x20\x2A\x20\x2A",
	"\x43\x6C\x69\x63\x6B",
	"\x49\x73\x20\x68\x65\x20\x61\x20\x67\x68\x6F\x73\x74\x3F\x20\x41\x6E\x79\x77\x61\x79\x20\x77\x72\x6F\x6E\x67\x20\x69\x64\x65\x61\x2E",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x6E\x6F\x74\x20\x65\x76\x65\x6E\x20\x61\x20\x6B\x65\x79\x68\x6F\x6C\x65\x21",
	"\x6D\x61\x73\x6B\x69\x6E\x67\x20\x74\x61\x70\x65",
	"\x46\x6C\x6F\x6F\x72\x62\x6F\x61\x72\x64\x73\x20\x73\x75\x72\x65\x20\x61\x72\x65\x20\x63\x72\x65\x61\x6B\x79",
	"\x4C\x6F\x6F\x73\x65\x20\x70\x6C\x61\x6E\x6B\x20\x68\x65\x72\x65",
	"\x73\x61\x6C\x74\x20\x70\x65\x74\x65\x72",
	"\x49\x20\x77\x6F\x6E\x20\x61\x20\x70\x72\x69\x7A\x65\x21",
	"\x72\x65\x61\x63\x68\x20\x61\x20\x6E\x61\x69\x6C",
	"\x6D\x61\x79\x20\x6E\x65\x65\x64\x20\x74\x6F\x20\x73\x61\x79\x20\x61\x20\x6D\x61\x67\x69\x63\x20\x77\x6F\x72\x64\x20\x68\x65\x72\x65",
	"\x61\x66\x74\x65\x72\x20\x61\x20\x6C\x6F\x6E\x67\x20\x72\x69\x64\x65",
	"\x68\x65\x20\x72\x69\x64\x65\x73\x20\x6F\x66\x66\x20\x77\x69\x74\x68\x6F\x75\x74\x20\x6D\x65",
	"\x47\x65\x72\x6F\x6E\x69\x6D\x6F\x20\x73\x61\x79\x73\x3A\x20\x22\x49\x74\x73\x20\x65\x61\x73\x79\x21\x20\x48\x61\x70\x70\x79\x20\x4C\x61\x6E\x64\x69\x6E\x67\x73\x21\x22",
	"\x69\x73\x20\x6D\x69\x73\x73\x69\x6E\x67",
	"\x72\x6F\x74\x74\x65\x6E\x20\x65\x67\x67\x73",
	"\x2E",
	"\x69\x6E\x20\x73\x61\x6C\x6F\x6F\x6E",
	"\x73\x63\x61\x72\x65\x64\x20\x74\x68\x65\x6D\x20\x6F\x66\x66\x21",
};


const uint8_t status[] = {
	142, 3, 
	13, 9, 0, 9, 0, 26, 
	36, 72, 76, 
	175, 
	2, 70, 3, 9, 0, 70, 
	59, 76, 10, 147, 
	179, 
	9, 1, 0, 0, 0, 2, 0, 12, 
	79, 73, 81, 79, 
	198, 
	0, 6, 
	81, 3, 4, 
	211, 
	0, 1, 0, 25, 0, 3, 0, 170, 
	58, 79, 81, 79, 
	207, 
	0, 7, 0, 24, 0, 11, 
	81, 54, 80, 54, 
	206, 
	0, 1, 0, 1, 0, 1, 
	79, 87, 54, 
	171, 
	0, 1, 0, 7, 
	82, 81, 73, 77, 
	205, 
	15, 25, 9, 15, 16, 0, 
	104, 120, 
	207, 
	15, 0, 0, 17, 9, 17, 
	105, 56, 76, 58, 
	207, 
	0, 7, 0, 3, 0, 3, 
	81, 81, 77, 81, 
	201, 
	4, 10, 9, 17, 
	57, 76, 
	180, 
	8, 17, 14, 70, 4, 11, 12, 9, 0, 70, 
	53, 
	180, 
	8, 17, 7, 11, 14, 70, 0, 70, 0, 11, 
	62, 
	136, 20, 
	16, 225, 0, 6, 
	58, 
	137, 20, 
	0, 3, 9, 12, 
	81, 73, 
	206, 
	15, 0, 0, 23, 0, 11, 
	28, 29, 62, 
	215, 
	15, 0, 0, 30, 0, 4, 0, 10, 0, 4, 
	79, 81, 79, 81, 
	196, 
	0, 3, 
	81, 
	172, 
	9, 17, 13, 70, 0, 70, 
	59, 
	170, 
	13, 23, 0, 4, 
	81, 77, 73, 
	201, 
	15, 0, 0, 23, 
	59, 70, 
	196, 
	0, 4, 
	81, 
	133, 4, 
	14, 24, 
	50, 51, 
	175, 
	8, 18, 0, 18, 0, 6, 
	60, 81, 77, 73, 
	204, 
	15, 0, 0, 14, 0, 42, 
	72, 
	196, 
	0, 6, 
	81, 
	170, 
	1, 61, 1, 9, 
	127, 126, 61, 
	142, 15, 
	4, 2, 14, 71, 13, 70, 
	41, 10, 146, 
	172, 
	1, 36, 6, 35, 0, 36, 
	55, 
	174, 
	8, 16, 0, 16, 0, 9, 
	60, 59, 76, 
	165, 
	1, 45, 
	122, 61, 
	168, 
	4, 38, 0, 8, 
	58, 
	168, 
	8, 8, 0, 9, 
	58, 
	174, 
	8, 9, 0, 9, 0, 2, 
	60, 73, 81, 
	199, 
	0, 2, 
	13, 78, 14, 81, 
	194, 
	78, 16, 65, 
	164, 
	8, 8, 
	63, 
	165, 
	0, 2, 
	81, 73, 
	205, 
	8, 6, 0, 3, 0, 6, 
	83, 60, 
	205, 
	8, 2, 0, 3, 0, 2, 
	82, 60, 
	205, 
	8, 3, 0, 10, 0, 3, 
	82, 60, 
	205, 
	8, 5, 0, 1, 0, 5, 
	82, 60, 
	196, 
	0, 2, 
	81, 
};
const uint8_t actions[] = {
	0, 40, 100, 
	48, 
	0, 1, 76, 
	108, 
	15, 30, 7, 
	0, 2, 0, 2, 0, 2, 
	81, 83, 81, 71, 
	9, 31, 0, 
	0, 8, 0, 9, 
	58, 58, 
	5, 38, 0, 
	6, 7, 
	6, 5, 
	18, 38, 10, 
	1, 7, 2, 6, 0, 8, 14, 8, 
	7, 10, 53, 
	15, 10, 10, 
	2, 6, 0, 21, 0, 6, 
	9, 1, 52, 58, 
	0, 1, 107, 
	48, 
	6, 43, 15, 
	3, 10, 
	1, 12, 144, 
	23, 40, 0, 
	3, 8, 3, 10, 3, 11, 0, 8, 0, 10, 
	1, 73, 59, 59, 
	201, 
	0, 11, 0, 12, 
	59, 53, 
	200, 
	15, 80, 0, 3, 
	58, 
	9, 1, 103, 
	4, 8, 9, 10, 
	6, 15, 
	14, 1, 103, 
	4, 8, 8, 10, 0, 10, 
	1, 54, 70, 
	23, 46, 0, 
	4, 8, 9, 10, 0, 10, 3, 14, 0, 18, 
	1, 17, 58, 58, 
	6, 73, 21, 
	2, 15, 
	19, 18, 61, 
	5, 25, 21, 
	2, 15, 
	1, 37, 
	11, 33, 117, 
	2, 75, 0, 30, 
	48, 142, 54, 73, 
	201, 
	15, 100, 0, 3, 
	76, 58, 
	10, 1, 61, 
	2, 18, 0, 12, 
	1, 54, 76, 
	10, 25, 22, 
	3, 19, 3, 20, 
	1, 21, 22, 
	9, 1, 43, 
	2, 2, 9, 11, 
	6, 23, 
	5, 61, 26, 
	2, 23, 
	6, 30, 
	23, 66, 0, 
	2, 23, 0, 12, 0, 23, 0, 24, 0, 2, 
	58, 72, 58, 31, 
	8, 10, 31, 
	2, 24, 5, 37, 
	32, 
	18, 1, 43, 
	2, 2, 8, 11, 0, 13, 13, 56, 
	1, 54, 76, 
	11, 64, 0, 
	2, 24, 9, 13, 
	1, 33, 10, 34, 
	9, 64, 0, 
	2, 24, 8, 13, 
	1, 37, 
	9, 50, 65, 
	2, 2, 9, 11, 
	24, 25, 
	19, 58, 23, 
	2, 2, 9, 11, 3, 20, 0, 11, 
	73, 26, 58, 109, 
	200, 
	15, 30, 0, 2, 
	58, 
	19, 61, 25, 
	3, 22, 4, 27, 0, 43, 14, 43, 
	125, 2, 10, 53, 
	9, 58, 0, 
	2, 2, 9, 11, 
	84, 27, 
	19, 50, 87, 
	2, 24, 0, 13, 0, 25, 9, 13, 
	10, 35, 58, 53, 
	23, 46, 34, 
	3, 26, 3, 14, 0, 26, 0, 9, 0, 18, 
	1, 72, 76, 58, 
	14, 69, 34, 
	3, 9, 0, 26, 0, 9, 
	1, 72, 76, 
	4, 71, 0, 
	2, 23, 
	37, 
	7, 61, 25, 
	3, 22, 
	1, 28, 50, 51, 
	5, 56, 39, 
	3, 28, 
	1, 39, 
	10, 1, 42, 
	2, 32, 0, 15, 
	1, 54, 76, 
	5, 68, 45, 
	3, 33, 
	1, 40, 
	17, 50, 65, 
	2, 31, 0, 31, 0, 29, 3, 37, 
	1, 72, 
	10, 1, 65, 
	2, 29, 0, 14, 
	1, 54, 76, 
	4, 50, 65, 
	2, 31, 
	114, 
	4, 32, 0, 
	0, 9, 
	58, 
	10, 1, 9, 
	2, 4, 0, 5, 
	54, 76, 1, 
	10, 1, 64, 
	2, 3, 0, 11, 
	54, 76, 1, 
	6, 25, 22, 
	3, 19, 
	1, 21, 20, 
	10, 1, 46, 
	2, 1, 0, 16, 
	54, 76, 1, 
	2, 71, 0, 
	1, 41, 11, 
	22, 58, 99, 
	3, 60, 3, 62, 0, 62, 0, 60, 0, 59, 
	59, 72, 1, 
	10, 76, 103, 
	4, 8, 0, 18, 
	1, 54, 76, 
	5, 1, 103, 
	4, 18, 
	6, 43, 
	10, 76, 103, 
	4, 18, 0, 8, 
	1, 54, 76, 
	0, 50, 98, 
	39, 
	1, 43, 10, 
	9, 6, 
	19, 80, 68, 
	3, 35, 14, 37, 0, 37, 0, 2, 
	53, 58, 10, 35, 
	9, 10, 31, 
	3, 37, 0, 37, 
	52, 1, 
	10, 83, 68, 
	1, 35, 0, 36, 
	74, 1, 73, 
	198, 
	14, 37, 
	33, 10, 44, 
	17, 82, 21, 
	3, 38, 2, 15, 0, 15, 0, 39, 
	1, 72, 
	18, 73, 21, 
	2, 39, 0, 39, 0, 17, 0, 18, 
	1, 72, 53, 
	6, 25, 68, 
	3, 35, 
	1, 2, 45, 
	10, 1, 63, 
	2, 5, 0, 19, 
	1, 54, 76, 
	5, 111, 26, 
	2, 23, 
	6, 30, 
	5, 111, 87, 
	2, 23, 
	6, 30, 
	4, 88, 0, 
	4, 8, 
	48, 
	19, 38, 44, 
	3, 7, 4, 23, 14, 40, 0, 40, 
	53, 7, 10, 73, 
	200, 
	15, 80, 0, 3, 
	58, 
	5, 68, 33, 
	3, 25, 
	1, 49, 
	10, 1, 73, 
	3, 41, 0, 24, 
	1, 54, 76, 
	10, 1, 75, 
	2, 16, 0, 25, 
	1, 54, 76, 
	6, 10, 75, 
	2, 16, 
	42, 6, 113, 
	10, 1, 23, 
	2, 16, 0, 25, 
	1, 54, 76, 
	14, 46, 18, 
	3, 14, 0, 6, 0, 18, 
	81, 73, 58, 
	194, 
	102, 88, 88, 
	202, 
	8, 15, 0, 14, 
	57, 76, 58, 
	198, 
	8, 14, 
	88, 88, 88, 
	202, 
	8, 14, 0, 14, 
	60, 56, 76, 
	197, 
	0, 6, 
	81, 103, 
	10, 33, 55, 
	4, 25, 9, 20, 
	1, 85, 118, 
	11, 1, 50, 
	2, 13, 0, 21, 
	56, 54, 76, 1, 
	15, 89, 0, 
	0, 7, 0, 175, 0, 7, 
	81, 79, 81, 73, 
	199, 
	0, 17, 
	116, 60, 88, 88, 
	199, 
	4, 26, 
	115, 106, 57, 76, 
	197, 
	7, 26, 
	107, 61, 
	204, 
	14, 0, 0, 0, 0, 11, 
	62, 
	10, 10, 5, 
	4, 26, 0, 27, 
	1, 54, 76, 
	10, 1, 52, 
	2, 43, 0, 26, 
	1, 54, 76, 
	10, 1, 53, 
	2, 44, 0, 28, 
	1, 54, 76, 
	15, 57, 0, 
	3, 45, 3, 28, 0, 45, 
	59, 38, 109, 110, 
	4, 65, 77, 
	3, 45, 
	111, 
	4, 38, 0, 
	3, 45, 
	111, 
	13, 38, 0, 
	5, 45, 0, 46, 4, 20, 
	1, 53, 
	10, 1, 14, 
	2, 46, 0, 29, 
	1, 54, 76, 
	19, 65, 79, 
	3, 48, 0, 48, 0, 49, 0, 5, 
	1, 72, 112, 58, 
	4, 10, 31, 
	3, 50, 
	6, 
	6, 93, 31, 
	14, 52, 
	1, 11, 47, 
	14, 90, 41, 
	3, 51, 0, 51, 0, 52, 
	1, 72, 73, 
	200, 
	15, 30, 0, 3, 
	58, 
	21, 93, 20, 
	0, 51, 2, 54, 14, 51, 14, 52, 14, 53, 
	1, 53, 
	5, 10, 20, 
	2, 54, 
	6, 113, 
	5, 50, 20, 
	2, 54, 
	6, 114, 
	9, 10, 41, 
	3, 53, 0, 53, 
	52, 1, 
	4, 93, 20, 
	2, 54, 
	1, 
	4, 93, 75, 
	2, 16, 
	118, 
	19, 38, 0, 
	4, 9, 14, 12, 14, 10, 0, 10, 
	1, 53, 7, 10, 
	19, 1, 43, 
	2, 2, 8, 11, 14, 56, 0, 13, 
	50, 54, 76, 46, 
	14, 97, 89, 
	14, 56, 0, 56, 3, 57, 
	53, 50, 109, 
	4, 93, 87, 
	2, 24, 
	1, 
	14, 10, 41, 
	3, 51, 0, 51, 0, 53, 
	59, 74, 1, 
	8, 50, 65, 
	2, 2, 8, 11, 
	39, 
	19, 98, 0, 
	0, 45, 0, 7, 0, 45, 0, 7, 
	82, 81, 83, 81, 
	4, 83, 67, 
	1, 34, 
	1, 
	0, 99, 0, 
	48, 
	15, 25, 18, 
	3, 14, 0, 6, 0, 6, 
	81, 78, 119, 81, 
	0, 76, 0, 
	42, 
	18, 46, 33, 
	3, 14, 0, 18, 0, 25, 3, 25, 
	58, 59, 1, 
	13, 46, 33, 
	3, 9, 3, 25, 0, 25, 
	1, 59, 
	14, 46, 13, 
	2, 12, 3, 9, 0, 12, 
	1, 59, 125, 
	19, 46, 13, 
	2, 12, 3, 14, 0, 18, 0, 12, 
	58, 1, 59, 125, 
	11, 46, 13, 
	3, 61, 3, 9, 
	1, 127, 126, 61, 
	11, 46, 13, 
	3, 61, 3, 14, 
	1, 127, 126, 61, 
	11, 93, 31, 
	13, 52, 2, 50, 
	130, 129, 128, 73, 
	199, 
	3, 61, 
	129, 127, 126, 61, 
	202, 
	3, 12, 0, 12, 
	129, 125, 59, 
	11, 33, 55, 
	4, 25, 8, 20, 
	1, 85, 73, 140, 
	198, 
	0, 20, 
	124, 141, 60, 
	202, 
	0, 36, 0, 16, 
	54, 76, 59, 
	0, 10, 62, 
	66, 
	0, 60, 0, 
	66, 
	0, 85, 0, 
	6, 
	22, 58, 13, 
	3, 60, 3, 12, 0, 60, 0, 12, 0, 61, 
	59, 72, 1, 
	1, 33, 0, 
	1, 85, 
	0, 29, 0, 
	6, 
	4, 46, 0, 
	3, 14, 
	117, 
	4, 46, 0, 
	3, 9, 
	117, 
	1, 46, 0, 
	6, 5, 
	5, 57, 0, 
	3, 28, 
	1, 38, 
	22, 115, 0, 
	1, 34, 4, 25, 0, 30, 0, 58, 0, 19, 
	124, 54, 62, 
	1, 18, 19, 
	0, 8, 
	5, 64, 87, 
	2, 23, 
	6, 30, 
	11, 100, 77, 
	2, 45, 3, 35, 
	1, 121, 122, 61, 
	18, 93, 52, 
	2, 43, 0, 38, 9, 19, 0, 19, 
	1, 53, 58, 
	1, 1, 21, 
	6, 123, 
	18, 103, 98, 
	3, 59, 0, 60, 0, 59, 0, 62, 
	1, 72, 53, 
	18, 103, 98, 
	3, 61, 0, 60, 0, 61, 0, 12, 
	1, 72, 53, 
	4, 10, 13, 
	3, 12, 
	48, 
	0, 66, 0, 
	1, 
	4, 93, 52, 
	2, 43, 
	1, 
	10, 93, 31, 
	3, 63, 13, 52, 
	130, 80, 73, 
	210, 
	2, 61, 0, 41, 0, 64, 0, 52, 
	72, 127, 59, 
	200, 
	2, 12, 0, 12, 
	59, 
	193, 
	80, 76, 
	9, 93, 31, 
	3, 63, 14, 52, 
	11, 47, 
	0, 80, 101, 
	131, 
	0, 73, 0, 
	6, 
	4, 1, 75, 
	4, 25, 
	139, 
	4, 25, 65, 
	2, 2, 
	132, 
	2, 111, 0, 
	1, 11, 47, 
	5, 82, 0, 
	3, 38, 
	133, 27, 
	10, 1, 61, 
	2, 58, 0, 31, 
	1, 54, 76, 
	11, 1, 105, 
	2, 65, 0, 33, 
	1, 54, 76, 134, 
	10, 25, 108, 
	4, 33, 14, 66, 
	1, 2, 135, 
	14, 10, 109, 
	14, 66, 0, 66, 0, 67, 
	74, 53, 1, 
	9, 10, 109, 
	3, 66, 0, 66, 
	52, 1, 
	10, 1, 61, 
	2, 67, 0, 34, 
	1, 54, 76, 
	6, 43, 95, 
	3, 8, 
	1, 12, 136, 
	10, 1, 119, 
	2, 69, 0, 35, 
	1, 54, 76, 
	10, 10, 5, 
	4, 30, 0, 5, 
	1, 54, 76, 
	15, 105, 0, 
	2, 70, 14, 71, 0, 71, 
	1, 74, 37, 137, 
	3, 105, 0, 
	1, 37, 11, 47, 
	15, 25, 20, 
	2, 64, 14, 72, 0, 72, 
	1, 53, 7, 10, 
	22, 110, 0, 
	3, 62, 2, 16, 3, 20, 0, 20, 3, 73, 
	1, 59, 73, 
	201, 
	0, 62, 0, 20, 
	59, 58, 
	9, 110, 0, 
	2, 16, 3, 59, 
	6, 138, 
	5, 38, 0, 
	1, 7, 
	7, 11, 
	10, 1, 115, 
	2, 74, 0, 37, 
	1, 54, 76, 
	15, 61, 116, 
	3, 76, 14, 75, 0, 75, 
	127, 127, 53, 109, 
	10, 61, 116, 
	3, 76, 13, 75, 
	0, 127, 127, 
	6, 110, 0, 
	2, 16, 
	6, 10, 143, 
	5, 61, 26, 
	2, 75, 
	6, 30, 
	5, 107, 98, 
	3, 60, 
	1, 24, 
	3, 25, 0, 
	1, 2, 11, 76, 
	2, 40, 0, 
	6, 10, 143, 
	4, 1, 23, 
	4, 25, 
	139, 
	1, 115, 0, 
	11, 47, 
	2, 58, 0, 
	6, 84, 27, 
	1, 1, 10, 
	9, 6, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
215, 65, 76, 75,
210, 85, 78, 32,
195, 76, 73, 77,
197, 78, 84, 69,
205, 79, 85, 78,
210, 73, 68, 69,
46, 32, 32, 32,
46, 32, 32, 32,
71, 69, 84, 32,
212, 65, 75, 69,
208, 73, 67, 75,
199, 82, 65, 66,
195, 79, 76, 76,
204, 73, 70, 84,
46, 32, 32, 32,
46, 32, 32, 32,
68, 82, 79, 80,
208, 85, 84, 32,
212, 72, 82, 79,
204, 69, 65, 86,
210, 69, 77, 79,
212, 79, 83, 83,
46, 32, 32, 32,
76, 79, 79, 75,
197, 88, 65, 77,
196, 69, 83, 67,
195, 72, 69, 67,
72, 69, 76, 80,
83, 65, 86, 69,
81, 85, 73, 84,
83, 67, 79, 82,
83, 65, 89, 32,
217, 69, 76, 76,
211, 67, 82, 69,
215, 72, 73, 83,
211, 73, 78, 71,
68, 73, 71, 32,
211, 72, 79, 86,
77, 73, 88, 32,
205, 65, 75, 69,
46, 32, 32, 32,
83, 77, 69, 76,
212, 65, 83, 84,
197, 65, 84, 32,
66, 85, 82, 78,
201, 71, 78, 73,
204, 73, 71, 72,
46, 32, 32, 32,
79, 80, 69, 78,
213, 78, 76, 79,
76, 79, 67, 75,
46, 32, 32, 32,
67, 76, 79, 83,
211, 72, 85, 84,
76, 79, 65, 68,
83, 72, 79, 79,
87, 73, 84, 72,
213, 83, 69, 32,
73, 78, 86, 69,
72, 73, 84, 32,
210, 73, 78, 71,
194, 69, 65, 84,
80, 76, 65, 89,
75, 73, 76, 76,
67, 76, 65, 80,
193, 80, 80, 76,
82, 69, 65, 68,
85, 78, 76, 73,
197, 88, 84, 73,
76, 73, 83, 84,
200, 69, 65, 82,
66, 82, 69, 65,
211, 77, 65, 83,
194, 85, 83, 84,
74, 85, 77, 80,
204, 69, 65, 80,
67, 85, 84, 32,
211, 76, 73, 67,
83, 72, 65, 75,
215, 65, 86, 69,
84, 65, 80, 69,
87, 69, 65, 82,
208, 85, 84, 79,
70, 73, 78, 68,
204, 79, 67, 65,
211, 69, 69, 75,
67, 82, 79, 83,
83, 76, 69, 69,
67, 79, 78, 78,
211, 80, 76, 73,
210, 69, 80, 65,
77, 79, 86, 69,
208, 85, 83, 72,
208, 82, 69, 83,
212, 65, 80, 32,
80, 65, 83, 83,
87, 65, 73, 84,
67, 76, 69, 65,
67, 79, 86, 69,
32, 32, 32, 32,
46, 32, 32, 32,
69, 77, 80, 84,
211, 80, 73, 76,
68, 65, 78, 67,
32, 32, 32, 32,
70, 73, 76, 76,
32, 32, 32, 32,
32, 32, 32, 32,
83, 72, 79, 69,
75, 78, 79, 67,
212, 79, 85, 67,
198, 69, 69, 76,
203, 73, 83, 83,
83, 80, 85, 82,
203, 73, 67, 75,
32, 32, 32, 32,
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
71, 65, 77, 69,
65, 82, 79, 85,
83, 84, 65, 66,
77, 65, 78, 85,
208, 73, 76, 69,
72, 73, 76, 76,
71, 85, 78, 80,
71, 82, 65, 86,
83, 85, 76, 70,
208, 79, 87, 68,
67, 72, 65, 82,
77, 65, 84, 67,
79, 78, 32, 32,
83, 65, 70, 69,
77, 73, 82, 82,
67, 79, 77, 80,
72, 79, 82, 83,
205, 65, 71, 78,
66, 69, 76, 76,
71, 72, 79, 83,
198, 73, 71, 85,
72, 65, 78, 68,
198, 69, 69, 84,
72, 69, 65, 68,
75, 69, 89, 32,
203, 69, 89, 83,
77, 65, 80, 32,
67, 65, 78, 68,
204, 73, 71, 72,
77, 85, 83, 73,
66, 85, 76, 76,
83, 84, 65, 66,
68, 69, 82, 82,
199, 85, 78, 32,
87, 73, 82, 69,
83, 84, 79, 82,
74, 65, 73, 76,
82, 79, 79, 70,
83, 73, 71, 78,
66, 65, 82, 66,
84, 85, 77, 66,
211, 65, 71, 69,
194, 82, 85, 83,
77, 73, 78, 69,
197, 78, 84, 82,
66, 69, 68, 32,
72, 79, 84, 69,
82, 79, 79, 77,
71, 73, 68, 68,
199, 73, 84, 84,
199, 73, 68, 89,
199, 73, 84, 89,
67, 79, 73, 78,
84, 79, 87, 78,
72, 79, 76, 69,
73, 78, 86, 69,
83, 84, 65, 76,
83, 65, 76, 79,
68, 79, 79, 82,
87, 73, 78, 68,
83, 80, 85, 82,
72, 65, 84, 32,
83, 84, 82, 73,
84, 65, 80, 69,
210, 79, 76, 76,
87, 65, 76, 76,
84, 69, 76, 69,
207, 70, 70, 73,
80, 65, 73, 78,
83, 76, 69, 69,
83, 78, 65, 75,
210, 65, 84, 84,
87, 79, 82, 77,
211, 76, 73, 77,
83, 72, 79, 86,
66, 79, 88, 32,
195, 65, 83, 72,
82, 79, 65, 68,
70, 79, 82, 75,
86, 65, 73, 78,
80, 73, 65, 78,
36, 50, 48, 48,
71, 79, 32, 32,
194, 79, 65, 82,
80, 65, 84, 72,
72, 69, 76, 76,
72, 73, 32, 32,
70, 73, 69, 76,
67, 82, 89, 83,
71, 79, 76, 68,
206, 85, 71, 71,
75, 69, 71, 32,
78, 65, 73, 76,
70, 85, 83, 69,
84, 79, 80, 80,
76, 79, 66, 66,
82, 65, 86, 73,
210, 73, 68, 71,
83, 72, 65, 67,
84, 82, 65, 73,
77, 79, 85, 78,
70, 76, 79, 79,
80, 76, 65, 78,
80, 69, 76, 84,
67, 85, 80, 32,
66, 65, 71, 32,
196, 85, 83, 84,
72, 65, 77, 77,
84, 69, 69, 80,
84, 79, 77, 32,
72, 79, 87, 32,
78, 69, 67, 75,
67, 79, 85, 78,
	0,
};
const uint8_t automap[] = {
83, 84, 82, 73,
	0,
83, 72, 79, 86,
	7,
67, 82, 89, 83,
	8,
67, 65, 78, 68,
	9,
83, 85, 76, 70,
	10,
67, 72, 65, 82,
	11,
77, 65, 84, 67,
	14,
67, 79, 77, 80,
	19,
72, 79, 82, 83,
	20,
77, 65, 78, 85,
	21,
66, 69, 76, 76,
	22,
77, 65, 80, 32,
	25,
67, 65, 78, 68,
	26,
66, 85, 76, 76,
	27,
68, 69, 82, 82,
	28,
83, 73, 71, 78,
	30,
83, 73, 71, 78,
	33,
83, 80, 85, 82,
	34,
72, 65, 84, 32,
	35,
75, 69, 89, 32,
	37,
84, 65, 80, 69,
	38,
71, 79, 76, 68,
	40,
77, 65, 84, 67,
	42,
83, 78, 65, 75,
	45,
67, 79, 73, 78,
	47,
87, 79, 82, 77,
	48,
87, 79, 82, 77,
	49,
87, 73, 82, 69,
	53,
66, 79, 88, 32,
	55,
36, 50, 48, 48,
	56,
71, 79, 32, 32,
	57,
75, 69, 71, 32,
	59,
75, 69, 71, 32,
	60,
75, 69, 71, 32,
	61,
78, 65, 73, 76,
	62,
80, 76, 65, 78,
	66,
80, 69, 76, 84,
	68,
67, 85, 80, 32,
	71,
66, 65, 71, 32,
	72,
72, 65, 77, 77,
	73,
84, 79, 77, 32,
	76,
78, 69, 67, 75,
	77,
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
