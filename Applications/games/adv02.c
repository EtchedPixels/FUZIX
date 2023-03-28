#define NUM_OBJ 67
#define WORDSIZE 3
#define GAME_MAGIC 408
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
const uint8_t startcarried = 0;
const uint8_t maxcar = 6;
const uint8_t treasure = 1;
const uint8_t treasures = 2;
const uint8_t lastloc = 26;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x46\x6C\x61\x74\x20\x69\x6E\x20\x6C\x6F\x6E\x64\x6F\x6E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x61\x6C\x63\x6F\x76\x65",
 { 0, 0, 0, 0, 0, 1 } }, 
		{ 	"\x73\x65\x63\x72\x65\x74\x20\x70\x61\x73\x73\x61\x67\x65\x77\x61\x79",
 { 0, 0, 4, 2, 0, 0 } }, 
		{ 	"\x6D\x75\x73\x74\x79\x20\x61\x74\x74\x69\x63",
 { 0, 0, 0, 3, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x61\x6E\x20\x6F\x70\x65\x6E\x20\x77\x69\x6E\x64\x6F\x77\x0A\x6F\x6E\x20\x74\x68\x65\x20\x6C\x65\x64\x67\x65\x20\x6F\x66\x20\x61\x20\x76\x65\x72\x79\x20\x74\x61\x6C\x6C\x20\x62\x75\x69\x6C\x64\x69\x6E\x67",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x61\x6E\x64\x79\x20\x62\x65\x61\x63\x68\x20\x6F\x6E\x20\x61\x20\x74\x72\x6F\x70\x69\x63\x61\x6C\x20\x69\x73\x6C\x65",
 { 0, 0, 8, 0, 0, 0 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 0, 12, 13, 14, 0, 11 } }, 
		{ 	"\x6D\x65\x61\x64\x6F\x77",
 { 0, 0, 14, 6, 0, 0 } }, 
		{ 	"\x67\x72\x61\x73\x73\x20\x73\x68\x61\x63\x6B",
 { 0, 0, 0, 8, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x6F\x63\x65\x61\x6E",
 { 10, 24, 10, 10, 0, 0 } }, 
		{ 	"\x70\x69\x74",
 { 0, 0, 0, 0, 7, 0 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 7, 0, 14, 13, 0, 0 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 7, 14, 12, 19, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x74\x68\x65\x20\x66\x6F\x6F\x74\x20\x6F\x66\x20\x61\x20\x63\x61\x76\x65\x20\x72\x69\x64\x64\x65\x6E\x20\x68\x69\x6C\x6C\x2C\x20\x61\x20\x70\x61\x74\x68\x77\x61\x79\x0A\x6C\x65\x61\x64\x73\x20\x6F\x6E\x20\x75\x70\x20\x74\x6F\x20\x74\x68\x65\x20\x74\x6F\x70",
 { 0, 0, 0, 8, 0, 0 } }, 
		{ 	"\x74\x6F\x6F\x6C\x20\x73\x68\x65\x64",
 { 17, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x6F\x6E\x67\x20\x68\x61\x6C\x6C\x77\x61\x79",
 { 0, 0, 17, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x63\x61\x76\x65\x72\x6E",
 { 0, 0, 0, 16, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x6F\x70\x20\x6F\x66\x20\x61\x20\x68\x69\x6C\x6C\x2E\x20\x42\x65\x6C\x6F\x77\x20\x69\x73\x20\x50\x69\x72\x61\x74\x65\x73\x20\x49\x73\x6C\x61\x6E\x64\x2E\x20\x41\x63\x72\x6F\x73\x73\x20\x74\x68\x65\x20\x73\x65\x61\x0A\x77\x61\x79\x20\x6F\x66\x66\x20\x69\x6E\x20\x74\x68\x65\x20\x64\x69\x73\x74\x61\x6E\x63\x65\x20\x49\x20\x73\x65\x65\x20\x54\x72\x65\x61\x73\x75\x72\x65\x20\x49\x73\x6C\x61\x6E\x64",
 { 0, 0, 0, 0, 0, 14 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 0, 14, 14, 13, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x62\x6F\x61\x72\x64\x20\x50\x69\x72\x61\x74\x65\x20\x73\x68\x69\x70\x20\x61\x6E\x63\x68\x6F\x72\x65\x64\x20\x6F\x66\x66\x20\x73\x68\x6F\x72\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x68\x65\x20\x62\x65\x61\x63\x68\x20\x61\x74\x20\x2A\x54\x72\x65\x61\x73\x75\x72\x65\x2A\x20\x49\x73\x6C\x61\x6E\x64",
 { 0, 22, 0, 0, 0, 0 } }, 
		{ 	"\x73\x70\x6F\x6F\x6B\x79\x20\x6F\x6C\x64\x20\x67\x72\x61\x76\x65\x79\x61\x72\x64\x20\x66\x69\x6C\x6C\x65\x64\x20\x77\x69\x74\x68\x20\x70\x69\x6C\x65\x73\x0A\x6F\x66\x20\x65\x6D\x70\x74\x79\x20\x61\x6E\x64\x20\x62\x72\x6F\x6B\x65\x6E\x20\x72\x75\x6D\x20\x62\x6F\x74\x74\x6C\x65\x73",
 { 21, 0, 23, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x62\x61\x72\x72\x65\x6E\x20\x66\x69\x65\x6C\x64",
 { 0, 0, 0, 22, 0, 0 } }, 
		{ 	"\x73\x68\x61\x6C\x6C\x6F\x77\x20\x6C\x61\x67\x6F\x6F\x6E\x2E\x0A\x74\x6F\x20\x74\x68\x65\x20\x6E\x6F\x72\x74\x68\x20\x69\x73\x20\x74\x68\x65\x20\x6F\x63\x65\x61\x6E",
 { 10, 6, 6, 6, 0, 0 } }, 
		{ 	"\x73\x61\x63\x6B\x65\x64\x20\x61\x6E\x64\x20\x64\x65\x73\x65\x72\x74\x65\x64\x20\x6D\x6F\x6E\x61\x73\x74\x61\x72\x79",
 { 0, 0, 0, 23, 0, 0 } }, 
		{ 	"\x2A\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x4E\x65\x76\x65\x72\x20\x4E\x65\x76\x65\x72\x20\x4C\x61\x6E\x64",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	1,
	2,
	2,
	0,
	0,
	4,
	1,
	0,
	4,
	0,
	0,
	6,
	9,
	9,
	8,
	24,
	8,
	11,
	11,
	0,
	17,
	10,
	25,
	25,
	9,
	1,
	0,
	0,
	0,
	0,
	1,
	15,
	0,
	17,
	17,
	16,
	0,
	0,
	18,
	17,
	10,
	0,
	0,
	4,
	1,
	0,
	15,
	0,
	6,
	0,
	0,
	6,
	24,
	0,
	15,
	0,
	23,
	0,
	0,
	6,
	1,
	0,
	0,
	0,
	0,
	0,
	0,
};


const char *objtext[] = {
	"\x46\x6C\x69\x67\x68\x74\x20\x6F\x66\x20\x73\x74\x61\x69\x72\x73",
	"\x4F\x70\x65\x6E\x20\x77\x69\x6E\x64\x6F\x77",
	"\x42\x6F\x6F\x6B\x73\x20\x69\x6E\x20\x61\x20\x62\x6F\x6F\x6B\x63\x61\x73\x65",
	"\x4C\x61\x72\x67\x65\x20\x62\x6C\x6F\x6F\x64\x20\x73\x6F\x61\x6B\x65\x64\x20\x62\x6F\x6F\x6B",
	"\x42\x6F\x6F\x6B\x63\x61\x73\x65\x20\x77\x69\x74\x68\x20\x73\x65\x63\x72\x65\x74\x20\x70\x61\x73\x73\x61\x67\x65\x20\x62\x65\x79\x6F\x6E\x64",
	"\x50\x69\x72\x61\x74\x65\x27\x73\x20\x64\x75\x66\x66\x65\x6C\x20\x62\x61\x67",
	"\x53\x69\x67\x6E\x20\x73\x61\x79\x73\x3A\x20\x22\x42\x72\x69\x6E\x67\x20\x2A\x54\x52\x45\x41\x53\x55\x52\x45\x53\x2A\x20\x68\x65\x72\x65\x2C\x20\x73\x61\x79\x3A\x20\x53\x43\x4F\x52\x45\x22",
	"\x45\x6D\x70\x74\x79\x20\x62\x6F\x74\x74\x6C\x65",
	"\x55\x6E\x6C\x69\x74\x20\x74\x6F\x72\x63\x68",
	"\x4C\x69\x74\x20\x74\x6F\x72\x63\x68",
	"\x4D\x61\x74\x63\x68\x65\x73",
	"\x53\x6D\x61\x6C\x6C\x20\x73\x68\x69\x70\x27\x73\x20\x6B\x65\x65\x6C\x20\x61\x6E\x64\x20\x6D\x61\x73\x74",
	"\x57\x69\x63\x6B\x65\x64\x20\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x70\x69\x72\x61\x74\x65",
	"\x54\x72\x65\x61\x73\x75\x72\x65\x20\x63\x68\x65\x73\x74",
	"\x4D\x6F\x6E\x67\x6F\x6F\x73\x65",
	"\x52\x75\x73\x74\x79\x20\x61\x6E\x63\x68\x6F\x72",
	"\x47\x72\x61\x73\x73\x20\x73\x68\x61\x63\x6B",
	"\x4D\x65\x61\x6E\x20\x61\x6E\x64\x20\x68\x75\x6E\x67\x72\x79\x20\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x63\x72\x6F\x63\x6F\x64\x69\x6C\x65\x73",
	"\x4C\x6F\x63\x6B\x65\x64\x20\x64\x6F\x6F\x72",
	"\x4F\x70\x65\x6E\x20\x64\x6F\x6F\x72\x20\x77\x69\x74\x68\x20\x68\x61\x6C\x6C\x20\x62\x65\x79\x6F\x6E\x64",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x73\x61\x69\x6C\x73",
	"\x46\x69\x73\x68",
	"\x2A\x44\x55\x42\x4C\x45\x4F\x4E\x53\x2A",
	"\x44\x65\x61\x64\x6C\x79\x20\x6D\x61\x6D\x62\x61\x20\x73\x6E\x61\x6B\x65\x73",
	"\x50\x61\x72\x72\x6F\x74",
	"\x42\x6F\x74\x74\x6C\x65\x20\x6F\x66\x20\x72\x75\x6D",
	"\x52\x75\x67",
	"\x52\x69\x6E\x67\x20\x6F\x66\x20\x6B\x65\x79\x73",
	"\x4F\x70\x65\x6E\x20\x74\x72\x65\x61\x73\x75\x72\x65\x20\x63\x68\x65\x73\x74",
	"\x53\x65\x74\x20\x6F\x66\x20\x70\x6C\x61\x6E\x73",
	"\x52\x75\x67",
	"\x43\x6C\x61\x77\x20\x68\x61\x6D\x6D\x65\x72",
	"\x4E\x61\x69\x6C\x73",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x70\x72\x65\x63\x75\x74\x20\x6C\x75\x6D\x62\x65\x72",
	"\x54\x6F\x6F\x6C\x20\x73\x68\x65\x64",
	"\x4C\x6F\x63\x6B\x65\x64\x20\x64\x6F\x6F\x72",
	"\x4F\x70\x65\x6E\x20\x64\x6F\x6F\x72\x20\x77\x69\x74\x68\x20\x70\x69\x74\x20\x62\x65\x79\x6F\x6E\x64",
	"\x50\x69\x72\x61\x74\x65\x20\x73\x68\x69\x70",
	"\x52\x6F\x63\x6B\x20\x77\x61\x6C\x6C\x20\x77\x69\x74\x68\x20\x6E\x61\x72\x72\x6F\x77\x20\x63\x72\x61\x63\x6B\x20\x69\x6E\x20\x69\x74",
	"\x4E\x61\x72\x72\x6F\x77\x20\x63\x72\x61\x63\x6B\x20\x69\x6E\x20\x74\x68\x65\x20\x72\x6F\x63\x6B",
	"\x53\x61\x6C\x74\x20\x77\x61\x74\x65\x72",
	"\x53\x6C\x65\x65\x70\x69\x6E\x67\x20\x70\x69\x72\x61\x74\x65",
	"\x42\x6F\x74\x74\x6C\x65\x20\x6F\x66\x20\x73\x61\x6C\x74\x20\x77\x61\x74\x65\x72",
	"\x52\x75\x6D\x20\x62\x6F\x74\x74\x6C\x65\x20\x73\x6D\x61\x73\x68\x65\x64\x20\x69\x6E\x74\x6F\x20\x70\x69\x65\x63\x65\x73\x2E\x0A\x53\x69\x67\x6E\x20\x22\x4F\x70\x70\x6F\x73\x69\x74\x65\x20\x6F\x66\x20\x4C\x49\x47\x48\x54\x20\x69\x73\x20\x55\x6E\x6C\x69\x67\x68\x74\x22",
	"\x53\x61\x66\x65\x74\x79\x20\x73\x6E\x65\x61\x6B\x65\x72\x73",
	"\x4D\x61\x70",
	"\x53\x68\x6F\x76\x65\x6C",
	"\x4D\x6F\x75\x6C\x64\x79\x20\x6F\x6C\x64\x20\x62\x6F\x6E\x65\x73",
	"\x53\x61\x6E\x64",
	"\x42\x6F\x74\x74\x6C\x65\x73\x20\x6F\x66\x20\x72\x75\x6D",
	"\x2A\x52\x41\x52\x45\x20\x53\x54\x41\x4D\x50\x53\x2A",
	"\x4C\x61\x67\x6F\x6F\x6E",
	"\x54\x68\x65\x20\x74\x69\x64\x65\x20\x69\x73\x20\x6F\x75\x74",
	"\x54\x68\x65\x20\x74\x69\x64\x65\x20\x69\x73\x20\x63\x6F\x6D\x69\x6E\x67\x20\x69\x6E",
	"\x57\x61\x74\x65\x72\x20\x77\x69\x6E\x67\x73",
	"\x46\x6C\x6F\x74\x73\x61\x6D\x20\x61\x6E\x64\x20\x6A\x65\x74\x73\x61\x6D",
	"\x4D\x6F\x6E\x61\x73\x74\x61\x72\x79",
	"\x57\x6F\x6F\x64\x65\x6E\x20\x62\x6F\x78",
	"\x44\x65\x61\x64\x20\x73\x71\x75\x69\x72\x72\x65\x6C",
	"\x53\x69\x67\x6E\x20\x69\x6E\x20\x74\x68\x65\x20\x73\x61\x6E\x64\x20\x73\x61\x79\x73\x3A\x0A\x22\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x50\x69\x72\x61\x74\x65\x73\x20\x49\x73\x6C\x61\x6E\x64\x2C\x20\x77\x61\x74\x63\x68\x20\x6F\x75\x74\x20\x66\x6F\x72\x20\x74\x68\x65\x20\x74\x69\x64\x65\x21\x22",
	"\x53\x61\x63\x6B\x20\x6F\x66\x20\x63\x72\x61\x63\x6B\x65\x72\x73",
	"\x4E\x6F\x74\x65",
	"\x53\x6D\x61\x6C\x6C\x20\x61\x64\x76\x65\x72\x74\x69\x73\x69\x6E\x67\x20\x66\x6C\x79\x65\x72",
	"\x42\x75\x72\x6E\x74\x20\x6F\x75\x74\x20\x74\x6F\x72\x63\x68",
	"",
	"",
	"",
};
const char *msgptr[] = {
	"",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x73\x74\x72\x61\x6E\x67\x65\x20\x73\x6F\x75\x6E\x64",
	"\x54\x68\x65\x20\x62\x6F\x6F\x6B\x20\x69\x73\x20\x69\x6E\x20\x62\x61\x64\x20\x63\x6F\x6E\x64\x74\x69\x6F\x6E\x20\x62\x75\x74\x20\x49\x20\x63\x61\x6E\x20\x6D\x61\x6B\x65\x20\x6F\x75\x74\x20\x74\x68\x65\x20\x74\x69\x74\x6C\x65\x3A\x0A\x22\x54\x72\x65\x61\x73\x75\x72\x65\x20\x49\x73\x6C\x61\x6E\x64\x22\x2E\x20\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x77\x6F\x72\x64\x20\x77\x72\x69\x74\x74\x65\x6E\x20\x69\x6E\x20\x62\x6C\x6F\x6F\x64\x20\x69\x6E\x20\x74\x68\x65\x0A\x66\x6C\x79\x6C\x65\x61\x66\x3A\x20\x22\x59\x4F\x48\x4F\x22\x20\x61\x6E\x64\x20\x61\x20\x6D\x65\x73\x73\x61\x67\x65\x3A\x0A\x0A\x22\x4C\x6F\x6E\x67\x20\x4A\x6F\x68\x6E\x20\x53\x69\x6C\x76\x65\x72\x20\x6C\x65\x66\x74\x20\x32\x20\x74\x72\x65\x61\x73\x75\x72\x65\x73\x20\x6F\x6E\x20\x54\x72\x65\x61\x73\x75\x72\x65\x20\x49\x73\x6C\x61\x6E\x64\x22",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x73",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x74\x68\x65\x72\x65\x20\x61\x6C\x6C\x20\x72\x69\x67\x68\x74\x2E\x20\x4D\x61\x79\x62\x65\x20\x49\x20\x73\x68\x6F\x75\x6C\x64",
	"\x54\x68\x61\x74\x27\x73\x20\x6E\x6F\x74\x20\x76\x65\x72\x79\x20\x73\x6D\x61\x72\x74",
	"\x49\x20\x6D\x61\x79\x20\x6E\x65\x65\x64\x20\x74\x6F\x20\x73\x61\x79\x20\x61\x20\x4D\x41\x47\x49\x43\x20\x77\x6F\x72\x64\x20\x68\x65\x72\x65\x21",
	"\x45\x76\x65\x72\x79\x74\x68\x69\x6E\x67\x20\x73\x70\x69\x6E\x73\x20\x61\x72\x6F\x75\x6E\x64\x20\x61\x6E\x64\x20\x73\x75\x64\x64\x65\x6E\x6C\x79\x20\x49\x27\x6D\x20\x65\x6C\x73\x65\x77\x68\x65\x72\x65\x2E\x2E\x2E",
	"\x54\x6F\x72\x63\x68\x20\x69\x73\x20\x6C\x69\x74",
	"\x49\x20\x77\x61\x73\x20\x77\x72\x6F\x6E\x67\x2C\x20\x49\x20\x67\x75\x65\x73\x73\x20\x69\x74\x73\x20\x6E\x6F\x74\x20\x61\x20\x6D\x6F\x6E\x67\x6F\x6F\x73\x65\x20\x63\x61\x75\x73\x65\x20\x74\x68\x65\x20\x73\x6E\x61\x6B\x65\x73\x20\x62\x69\x74\x20\x69\x74\x21",
	"\x49\x27\x6D\x20\x73\x6E\x61\x6B\x65\x20\x62\x69\x74",
	"\x50\x61\x72\x72\x6F\x74\x20\x61\x74\x74\x61\x63\x6B\x73\x20\x73\x6E\x61\x6B\x65\x73\x20\x61\x6E\x64\x20\x64\x72\x69\x76\x65\x73\x20\x74\x68\x65\x6D\x20\x6F\x66\x66",
	"\x50\x69\x72\x61\x74\x65\x20\x77\x6F\x6E\x27\x74\x20\x6C\x65\x74\x20\x6D\x65",
	"\x49\x74\x73\x20\x6C\x6F\x63\x6B\x65\x64",
	"\x49\x74\x73\x20\x6F\x70\x65\x6E",
	"\x54\x68\x65\x72\x65\x20\x61\x72\x65\x20\x61\x20\x73\x65\x74\x20\x6F\x66\x20\x70\x6C\x61\x6E\x73\x20\x69\x6E\x20\x69\x74",
	"\x4E\x6F\x74\x20\x77\x68\x69\x6C\x65\x20\x49\x27\x6D\x20\x63\x61\x72\x72\x79\x69\x6E\x67\x20\x69\x74",
	"\x43\x72\x6F\x63\x73\x20\x73\x74\x6F\x70\x20\x6D\x65",
	"\x53\x6F\x72\x72\x79\x20\x49\x20\x63\x61\x6E\x27\x74",
	"\x57\x72\x6F\x6E\x67\x20\x67\x61\x6D\x65\x20\x79\x6F\x75\x20\x73\x69\x6C\x6C\x79\x20\x67\x6F\x6F\x73\x65\x21",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x68\x61\x76\x65\x20\x69\x74",
	"\x50\x69\x72\x61\x74\x65\x20\x67\x72\x61\x62\x73\x20\x72\x75\x6D\x20\x61\x6E\x64\x20\x73\x63\x75\x74\x74\x6C\x65\x73\x20\x6F\x66\x66\x20\x63\x68\x6F\x72\x74\x6C\x69\x6E\x67",
	"\x2E\x2E\x2E\x49\x20\x74\x68\x69\x6E\x6B\x20\x69\x74\x73\x20\x6D\x65\x2C\x20\x48\x65\x65\x20\x48\x65\x65\x2E",
	"\x49\x74\x73\x20\x6E\x61\x69\x6C\x65\x64\x20\x74\x6F\x20\x74\x68\x65\x20\x66\x6C\x6F\x6F\x72\x21",
	"\x59\x6F\x68\x6F\x20\x68\x6F\x20\x61\x6E\x64\x20\x61\x20\x2E\x2E\x2E",
	"\x4E\x6F\x2C\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x69\x73\x20\x6D\x69\x73\x73\x69\x6E\x67\x21",
	"\x49\x74\x20\x77\x61\x73\x20\x61\x20\x74\x69\x67\x68\x74\x20\x73\x71\x75\x65\x65\x7A\x65\x21",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x77\x6F\x6E\x27\x74\x20\x66\x69\x74",
	"\x53\x69\x6E\x63\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x69\x73\x20\x68\x61\x70\x70\x65\x6E\x69\x6E\x67",
	"\x49\x20\x73\x6C\x69\x70\x70\x65\x64\x20\x61\x6E\x64\x20\x66\x65\x6C\x6C\x2E\x2E\x2E",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x66\x61\x6C\x6C\x73\x20\x6F\x75\x74",
	"\x54\x68\x65\x79\x27\x72\x65\x20\x70\x6C\x61\x6E\x73\x20\x74\x6F\x20\x62\x75\x69\x6C\x64\x20\x74\x68\x65\x20\x4A\x6F\x6C\x6C\x79\x20\x52\x6F\x67\x65\x72\x20\x28\x61\x20\x50\x69\x72\x61\x74\x65\x20\x73\x68\x69\x70\x21\x29\x0A\x59\x6F\x75\x27\x6C\x6C\x20\x6E\x65\x65\x64\x3A\x20\x68\x61\x6D\x6D\x65\x72\x2C\x20\x6E\x61\x69\x6C\x73\x2C\x20\x6C\x75\x6D\x62\x65\x72\x2C\x20\x61\x6E\x63\x68\x6F\x72\x2C\x20\x73\x61\x69\x6C\x73\x2C\x20\x61\x6E\x64\x20\x61\x20\x6B\x65\x65\x6C\x2E",
	"\x49\x27\x76\x65\x20\x6E\x6F\x20\x63\x6F\x6E\x74\x61\x69\x6E\x65\x72",
	"\x49\x74\x20\x73\x6F\x61\x6B\x73\x20\x69\x6E\x74\x6F\x20\x74\x68\x65\x20\x67\x72\x6F\x75\x6E\x64",
	"\x54\x6F\x6F\x20\x64\x72\x79\x2C\x20\x66\x69\x73\x68\x20\x76\x61\x6E\x69\x73\x68\x2E",
	"\x50\x69\x72\x61\x74\x65\x20\x61\x77\x61\x6B\x65\x6E\x73\x20\x61\x6E\x64\x20\x73\x61\x79\x73\x20\x22\x41\x79\x65\x20\x6D\x61\x74\x65\x79\x20\x77\x65\x20\x62\x65\x20\x63\x61\x73\x74\x69\x6E\x67\x20\x6F\x66\x66\x20\x73\x6F\x6F\x6E\x22\x0A\x48\x65\x20\x74\x68\x65\x6E\x20\x56\x41\x4E\x49\x53\x48\x45\x53\x21",
	"\x57\x68\x61\x74\x20\x61\x20\x77\x61\x73\x74\x65\x2E\x2E\x2E",
	"\x49\x27\x76\x65\x20\x6E\x6F\x20\x63\x72\x65\x77",
	"\x50\x69\x72\x61\x74\x65\x20\x73\x61\x79\x73\x3A\x20\x22\x41\x79\x65\x20\x6D\x61\x74\x65\x79\x20\x77\x65\x20\x62\x65\x20\x6E\x65\x65\x64\x69\x6E\x67\x20\x61\x20\x6D\x61\x70\x20\x66\x69\x72\x73\x74\x22\x2E",
	"\x41\x66\x74\x65\x72\x20\x61\x20\x64\x61\x79\x20\x61\x74\x20\x73\x65\x61\x20\x77\x65\x20\x73\x65\x74\x20\x61\x6E\x63\x68\x6F\x72\x20\x6F\x66\x66\x20\x6F\x66\x20\x61\x20\x73\x61\x6E\x64\x79\x20\x62\x65\x61\x63\x68\x2E\x0A\x20\x41\x6C\x6C\x20\x41\x73\x68\x6F\x72\x65\x20\x77\x68\x6F\x27\x73\x20\x67\x6F\x69\x6E\x67\x20\x41\x73\x68\x6F\x72\x65\x2E\x2E\x2E",
	"\x54\x72\x79\x3A\x20\x22\x57\x45\x49\x47\x48\x20\x41\x4E\x43\x48\x4F\x52\x22",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x6D\x61\x70\x20\x69\x6E\x20\x69\x74",
	"\x49\x74\x73\x20\x61\x20\x6D\x61\x70\x20\x74\x6F\x20\x54\x72\x65\x61\x73\x75\x72\x65\x20\x49\x73\x6C\x61\x6E\x64\x2E\x20\x41\x74\x20\x74\x68\x65\x20\x62\x6F\x74\x74\x6F\x6D\x20\x69\x74\x20\x73\x61\x79\x73\x3A\x0A\x20\x20\x20\x22\x33\x30\x20\x70\x61\x63\x65\x73\x20\x74\x68\x65\x6E\x20\x64\x69\x67\x21\x22",
	"\x2A\x20\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x6E\x75\x6D\x62\x65\x72\x20\x32\x3A\x20\x22\x70\x69\x72\x61\x74\x65\x20\x61\x64\x76\x65\x6E\x74\x75\x72\x65\x22\x0A\x20\x20\x62\x79\x20\x41\x6C\x65\x78\x69\x73\x20\x26\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x2C\x20\x64\x65\x64\x69\x63\x61\x74\x65\x64\x3A\x20\x54\x65\x64\x20\x48\x65\x65\x72\x65\x6E\x20\x26\x20\x50\x61\x75\x6C\x20\x53\x68\x61\x72\x6C\x61\x6E\x64\x0A\x52\x65\x6D\x65\x6D\x62\x65\x72\x20\x79\x6F\x75\x20\x63\x61\x6E\x20\x61\x6C\x77\x61\x79\x73\x20\x61\x73\x6B\x20\x66\x6F\x72\x20\x22\x68\x65\x6C\x70\x22\x2E\x0A",
	"\x49\x74\x73\x20\x65\x6D\x70\x74\x79",
	"\x49\x27\x76\x65\x20\x6E\x6F\x20\x70\x6C\x61\x6E\x73\x21",
	"\x6F\x70\x65\x6E\x20\x69\x74\x3F",
	"\x67\x6F\x20\x74\x68\x65\x72\x65\x3F",
	"\x49\x20\x66\x6F\x75\x6E\x64\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21",
	"\x49\x20\x64\x69\x64\x6E\x27\x74\x20\x66\x69\x6E\x64\x20\x61\x6E\x79\x74\x68\x69\x6E\x67",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x73\x65\x65\x20\x69\x74\x20\x68\x65\x72\x65",
	"\x4F\x4B\x20\x49\x20\x77\x61\x6C\x6B\x65\x64\x20\x6F\x66\x66\x20\x33\x30\x20\x70\x61\x63\x65\x73\x2E",
	"\x43\x4F\x4E\x47\x52\x41\x54\x55\x4C\x41\x54\x49\x4F\x4E\x53\x20\x21\x21\x21\x0A\x20\x42\x75\x74\x20\x79\x6F\x75\x72\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x69\x73\x20\x6E\x6F\x74\x20\x6F\x76\x65\x72\x20\x79\x65\x74\x2E\x2E\x2E\x0A",
	"\x52\x65\x61\x64\x69\x6E\x67\x20\x65\x78\x70\x61\x6E\x64\x73\x20\x74\x68\x65\x20\x6D\x69\x6E\x64",
	"\x54\x68\x65\x20\x50\x61\x72\x72\x6F\x74\x20\x63\x72\x79\x73\x3A",
	"\x22\x43\x68\x65\x63\x6B\x20\x74\x68\x65\x20\x62\x61\x67\x20\x6D\x61\x74\x65\x79\x22",
	"\x22\x43\x68\x65\x63\x6B\x20\x74\x68\x65\x20\x63\x68\x65\x73\x74\x20\x6D\x61\x74\x65\x79\x22",
	"\x66\x72\x6F\x6D\x20\x74\x68\x65\x20\x6F\x74\x68\x65\x72\x20\x73\x69\x64\x65\x21",
	"\x4F\x70\x65\x6E\x20\x74\x68\x65\x20\x62\x6F\x6F\x6B\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x6D\x75\x6C\x74\x69\x70\x6C\x65\x20\x65\x78\x69\x74\x73\x20\x68\x65\x72\x65\x21",
	"\x43\x72\x6F\x63\x73\x20\x65\x61\x74\x20\x66\x69\x73\x68\x20\x61\x6E\x64\x20\x6C\x65\x61\x76\x65",
	"\x49\x27\x6D\x20\x75\x6E\x64\x65\x72\x77\x61\x74\x65\x72\x2C\x20\x49\x20\x67\x75\x65\x73\x73\x20\x49\x20\x64\x6F\x6E\x27\x74\x20\x73\x77\x69\x6D\x20\x77\x65\x6C\x6C\x2E\x20\x42\x6C\x75\x62\x20\x42\x6C\x75\x62\x2E\x2E\x2E",
	"\x22\x50\x69\x65\x63\x65\x73\x20\x6F\x66\x20\x65\x69\x67\x68\x74\x22",
	"\x49\x74\x73\x20\x73\x74\x75\x63\x6B\x20\x69\x6E\x20\x74\x68\x65\x20\x73\x61\x6E\x64",
	"\x4F\x4B",
	"\x50\x69\x72\x61\x74\x65\x20\x73\x61\x79\x73\x3A\x20\x22\x41\x79\x65\x20\x6D\x65\x20\x42\x75\x63\x6B\x65\x72\x6F\x6F\x2C\x20\x77\x65\x20\x62\x65\x20\x77\x61\x69\x74\x69\x6E\x67\x20\x66\x6F\x72\x20\x74\x68\x65\x20\x74\x69\x64\x65\x20\x74\x6F\x0A\x20\x63\x6F\x6D\x65\x20\x69\x6E\x21\x22",
	"\x54\x68\x65\x20\x74\x69\x64\x65\x20\x69\x73\x20\x6F\x75\x74",
	"\x54\x68\x65\x20\x74\x69\x64\x65\x20\x69\x73\x20\x63\x6F\x6D\x69\x6E\x67\x20\x69\x6E",
	"\x41\x62\x6F\x75\x74\x20\x32\x30\x20\x70\x6F\x75\x6E\x64\x73\x2E\x20\x54\x72\x79\x3A\x20\x22\x53\x45\x54\x20\x53\x41\x49\x4C\x22",
	"\x22\x54\x69\x64\x65\x73\x20\x62\x65\x20\x61\x20\x63\x68\x61\x6E\x67\x69\x6E\x67\x20\x6D\x61\x74\x65\x79\x22",
	"\x4E\x6F\x74\x65\x20\x68\x65\x72\x65\x20\x73\x61\x79\x73\x3A\x20\x22\x49\x20\x62\x65\x20\x6C\x69\x6B\x69\x6E\x67\x20\x70\x61\x72\x72\x6F\x74\x73\x2C\x20\x74\x68\x65\x79\x20\x62\x65\x20\x73\x6D\x61\x72\x74\x20\x6D\x61\x74\x65\x79\x21\x22",
	"\x50\x69\x72\x61\x74\x65\x20\x66\x6F\x6C\x6C\x6F\x77\x73\x20\x6D\x65\x20\x61\x73\x68\x6F\x72\x65\x20\x61\x73\x20\x69\x66\x20\x65\x78\x70\x65\x63\x74\x69\x6E\x67\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67",
	"\x43\x6C\x69\x6D\x62\x20\x73\x74\x61\x69\x72\x73\x2E\x2E\x2E",
	"\x47\x6F\x74\x20\x61\x6E\x79\x74\x68\x69\x6E\x67\x20\x74\x6F\x20\x65\x61\x74\x20\x6D\x61\x74\x65\x79\x3F",
	"\x50\x61\x72\x72\x6F\x74\x20\x61\x74\x74\x61\x63\x6B\x73\x20\x63\x72\x6F\x63\x73\x20\x62\x75\x74\x20\x69\x73\x20\x62\x65\x61\x74\x65\x6E\x20\x6F\x66\x66",
	"\x42\x69\x72\x64\x20\x66\x6C\x79\x73\x20\x6F\x66\x66\x20\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x76\x65\x72\x79\x20\x75\x6E\x68\x61\x70\x70\x79",
	"\x50\x61\x72\x72\x6F\x74\x20\x61\x74\x65\x20\x61\x20\x63\x72\x61\x63\x6B\x65\x72\x2E",
	"\x59\x75\x6D\x6D\x79",
	"\x49\x20\x68\x65\x61\x72\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x6E\x6F\x77",
	"\x50\x69\x72\x61\x74\x65\x20\x73\x61\x79\x73\x3A\x0A\x22\x46\x69\x72\x73\x74\x20\x59\x65\x65\x20\x62\x65\x20\x67\x65\x74\x74\x69\x6E\x67\x20\x74\x68\x61\x74\x20\x41\x43\x43\x55\x52\x53\x45\x44\x20\x74\x68\x69\x6E\x67\x20\x6F\x66\x66\x20\x6D\x65\x20\x73\x68\x69\x70\x21\x22\x0A",
	"\x72\x65\x61\x64\x20\x69\x74\x3F",
	"\x41\x73\x6B\x20\x66\x6F\x72\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x6E\x75\x6D\x62\x65\x72\x20\x33\x3A\x20\x22\x4D\x49\x53\x53\x49\x4F\x4E\x20\x49\x4D\x50\x4F\x53\x53\x49\x42\x4C\x45\x22\x20\x61\x74\x20\x79\x6F\x75\x72\x0A\x20\x66\x61\x76\x6F\x72\x69\x74\x65\x20\x63\x6F\x6D\x70\x75\x74\x65\x72\x20\x64\x65\x61\x6C\x65\x72\x2E\x20\x49\x66\x20\x74\x68\x65\x79\x20\x44\x4F\x4E\x27\x54\x20\x63\x61\x72\x72\x79\x20\x22\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x22\x0A\x20\x68\x61\x76\x65\x20\x74\x68\x65\x6D\x20\x63\x61\x6C\x6C\x3A\x20\x31\x2D\x33\x30\x35\x2D\x38\x36\x32\x2D\x36\x39\x31\x37\x20\x20\x54\x4F\x44\x41\x59\x21\x20\x22\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x22\x20\x61\x6C\x73\x6F\x0A\x20\x73\x75\x70\x70\x6F\x72\x74\x73\x20\x6C\x6F\x77\x65\x72\x20\x63\x61\x73\x65\x21\x0A",
	"\x49\x27\x6D\x20\x6E\x6F\x74\x20\x66\x65\x65\x6C\x69\x6E\x67\x20\x64\x65\x73\x74\x72\x75\x63\x74\x69\x76\x65\x21",
	"\x22\x43\x68\x65\x63\x6B\x20\x74\x68\x65\x20\x62\x6F\x6F\x6B\x2C\x20\x6D\x61\x74\x65\x79\x21\x22",
	"\x41\x6C\x6C\x20\x72\x69\x67\x68\x74\x2C\x20\x50\x4F\x4F\x46\x20\x74\x68\x65\x20\x47\x41\x4D\x45\x20\x69\x73\x20\x64\x65\x73\x74\x72\x6F\x79\x65\x64\x21",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x6B\x6E\x6F\x77\x20\x77\x68\x65\x72\x65\x20\x74\x6F\x20\x6C\x6F\x6F\x6B\x21",
	"\x49\x74\x73\x20\x73\x74\x75\x63\x6B",
	"",
};


const uint8_t status[] = {
	146, 80, 
	2, 21, 2, 17, 0, 21, 0, 17, 
	110, 59, 59, 
	143, 80, 
	2, 23, 2, 24, 0, 23, 
	104, 112, 11, 55, 
	182, 
	1, 26, 12, 27, 0, 27, 9, 11, 0, 11, 
	1, 53, 58, 
	133, 3, 
	3, 24, 
	104, 112, 
	169, 
	4, 14, 8, 15, 
	57, 76, 
	176, 
	8, 1, 3, 33, 3, 20, 0, 2, 
	58, 
	183, 
	8, 2, 0, 1, 0, 33, 0, 37, 0, 11, 
	60, 55, 72, 102, 
	169, 
	8, 1, 0, 1, 
	25, 60, 
	173, 
	8, 0, 0, 35, 0, 36, 
	72, 68, 
	183, 
	8, 2, 0, 2, 0, 32, 0, 20, 0, 15, 
	60, 55, 55, 55, 
	134, 19, 
	4, 26, 
	65, 28, 63, 
	137, 40, 
	4, 5, 6, 44, 
	29, 61, 
	151, 80, 
	2, 12, 2, 25, 0, 41, 0, 4, 0, 12, 
	62, 21, 67, 59, 
	178, 
	8, 0, 0, 7, 0, 4, 0, 25, 
	68, 62, 59, 
	145, 25, 
	1, 21, 6, 42, 0, 21, 0, 10, 
	34, 62, 
	169, 
	9, 6, 0, 6, 
	43, 58, 
	150, 50, 
	2, 12, 2, 49, 0, 41, 0, 22, 0, 12, 
	62, 21, 59, 
	137, 35, 
	3, 24, 9, 3, 
	104, 105, 
	137, 7, 
	3, 24, 9, 12, 
	104, 106, 
	142, 50, 
	4, 24, 13, 53, 6, 54, 
	117, 111, 61, 
	137, 50, 
	4, 10, 6, 54, 
	111, 61, 
	150, 10, 
	9, 10, 0, 52, 0, 53, 0, 15, 0, 55, 
	72, 72, 67, 
	145, 10, 
	8, 10, 0, 52, 0, 53, 9, 4, 
	72, 67, 
	170, 
	3, 24, 8, 0, 
	104, 119, 68, 
	164, 
	8, 0, 
	68, 
	169, 
	4, 5, 8, 15, 
	57, 76, 
	145, 80, 
	2, 23, 2, 14, 0, 14, 0, 58, 
	9, 72, 
	146, 80, 
	2, 17, 2, 24, 0, 24, 0, 13, 
	124, 125, 62, 
	149, 30, 
	3, 24, 12, 60, 12, 12, 0, 24, 0, 60, 
	125, 75, 
	136, 20, 
	3, 24, 3, 60, 
	126, 
	141, 25, 
	14, 61, 3, 24, 3, 3, 
	104, 133, 
	178, 
	8, 16, 0, 63, 0, 9, 0, 16, 
	72, 60, 76, 
};
const uint8_t actions[] = {
	15, 8, 23, 
	7, 5, 0, 5, 1, 3, 
	70, 7, 54, 64, 
	17, 50, 30, 
	4, 20, 2, 12, 3, 3, 3, 45, 
	18, 129, 
	19, 37, 20, 
	3, 8, 3, 10, 0, 8, 0, 9, 
	72, 70, 8, 64, 
	14, 41, 20, 
	3, 9, 0, 9, 0, 8, 
	72, 114, 76, 
	4, 42, 0, 
	4, 5, 
	6, 
	13, 10, 29, 
	2, 22, 5, 23, 0, 22, 
	52, 114, 
	10, 10, 29, 
	2, 22, 2, 23, 
	5, 10, 61, 
	10, 1, 33, 
	2, 16, 0, 9, 
	54, 70, 64, 
	8, 10, 38, 
	2, 13, 2, 12, 
	12, 
	13, 10, 38, 
	2, 13, 5, 12, 0, 13, 
	52, 114, 
	8, 39, 38, 
	2, 13, 2, 12, 
	12, 
	8, 39, 38, 
	2, 13, 5, 12, 
	13, 
	22, 41, 38, 
	2, 13, 5, 12, 1, 27, 0, 13, 0, 28, 
	14, 55, 53, 
	4, 39, 38, 
	1, 13, 
	16, 
	4, 27, 38, 
	1, 28, 
	16, 
	5, 27, 38, 
	3, 13, 
	18, 13, 
	22, 27, 38, 
	2, 28, 0, 29, 9, 5, 0, 5, 9, 12, 
	15, 53, 58, 
	22, 27, 38, 
	9, 12, 2, 28, 8, 5, 0, 45, 0, 12, 
	41, 53, 58, 
	8, 27, 38, 
	8, 12, 2, 28, 
	44, 
	9, 27, 18, 
	3, 5, 9, 3, 
	4, 46, 
	8, 27, 18, 
	3, 5, 8, 3, 
	44, 
	4, 39, 37, 
	2, 17, 
	17, 
	5, 39, 37, 
	2, 18, 
	18, 13, 
	4, 39, 37, 
	2, 19, 
	14, 
	10, 1, 9, 
	2, 19, 0, 16, 
	54, 70, 64, 
	8, 41, 37, 
	2, 17, 2, 18, 
	17, 
	23, 41, 37, 
	5, 17, 2, 18, 1, 27, 0, 18, 0, 19, 
	55, 53, 67, 114, 
	6, 23, 11, 
	3, 25, 
	127, 1, 22, 
	0, 25, 0, 
	66, 
	0, 10, 28, 
	66, 
	9, 27, 58, 
	3, 57, 12, 50, 
	4, 46, 
	1, 8, 71, 
	19, 3, 
	5, 30, 10, 
	6, 3, 
	18, 20, 
	0, 33, 0, 
	65, 
	0, 34, 14, 
	71, 
	5, 47, 42, 
	12, 29, 
	18, 45, 
	11, 1, 35, 
	4, 14, 0, 7, 
	54, 56, 70, 64, 
	6, 27, 48, 
	14, 52, 
	4, 47, 117, 
	6, 27, 48, 
	13, 52, 
	4, 47, 116, 
	5, 27, 33, 
	2, 16, 
	4, 47, 
	5, 27, 45, 
	2, 38, 
	4, 47, 
	9, 1, 45, 
	2, 39, 1, 46, 
	18, 27, 
	9, 1, 45, 
	2, 38, 1, 13, 
	18, 27, 
	0, 46, 0, 
	63, 
	5, 10, 26, 
	2, 30, 
	18, 23, 
	23, 10, 41, 
	1, 31, 2, 30, 0, 32, 0, 26, 0, 30, 
	52, 53, 55, 114, 
	9, 1, 45, 
	2, 39, 1, 33, 
	18, 27, 
	20, 47, 42, 
	3, 31, 3, 29, 3, 15, 3, 32, 0, 1, 
	58, 
	0, 47, 42, 
	25, 
	10, 1, 50, 
	2, 36, 0, 11, 
	70, 54, 64, 
	9, 1, 45, 
	2, 38, 1, 3, 
	18, 27, 
	15, 8, 23, 
	4, 5, 0, 6, 1, 3, 
	70, 7, 54, 64, 
	9, 10, 26, 
	3, 26, 0, 26, 
	52, 114, 
	11, 1, 45, 
	2, 38, 0, 17, 
	54, 26, 56, 64, 
	11, 1, 45, 
	2, 39, 0, 18, 
	54, 57, 70, 64, 
	16, 50, 30, 
	4, 20, 2, 12, 13, 52, 9, 4, 
	115, 
	18, 39, 18, 
	3, 5, 0, 10, 9, 3, 0, 3, 
	30, 53, 58, 
	5, 39, 18, 
	8, 3, 
	3, 44, 
	4, 10, 46, 
	6, 7, 
	32, 
	18, 10, 46, 
	2, 40, 1, 7, 0, 7, 0, 42, 
	55, 52, 114, 
	14, 18, 46, 
	1, 42, 0, 42, 0, 7, 
	55, 52, 33, 
	5, 23, 46, 
	2, 40, 
	5, 61, 
	15, 23, 46, 
	1, 42, 0, 42, 0, 7, 
	5, 61, 55, 52, 
	18, 49, 16, 
	2, 41, 0, 41, 0, 12, 0, 20, 
	35, 55, 62, 
	4, 39, 11, 
	3, 25, 
	14, 
	14, 56, 11, 
	3, 25, 0, 25, 0, 7, 
	36, 33, 72, 
	10, 1, 42, 
	2, 37, 0, 20, 
	54, 70, 64, 
	23, 1, 51, 
	4, 20, 8, 4, 0, 21, 0, 12, 2, 12, 
	54, 70, 53, 121, 
	14, 1, 51, 
	4, 20, 9, 4, 0, 6, 
	54, 70, 64, 
	9, 50, 30, 
	4, 20, 5, 12, 
	18, 37, 
	9, 50, 30, 
	4, 20, 12, 45, 
	18, 38, 
	23, 50, 30, 
	4, 20, 9, 4, 0, 4, 0, 37, 0, 21, 
	70, 39, 58, 62, 
	23, 50, 30, 
	4, 20, 8, 4, 0, 4, 0, 37, 0, 6, 
	70, 39, 60, 62, 
	0, 51, 21, 
	40, 
	4, 30, 53, 
	3, 45, 
	42, 
	0, 9, 0, 
	40, 
	9, 10, 10, 
	2, 3, 0, 3, 
	52, 114, 
	14, 39, 10, 
	3, 3, 14, 62, 0, 62, 
	53, 114, 30, 
	14, 1, 51, 
	4, 20, 8, 4, 0, 21, 
	54, 70, 64, 
	10, 1, 68, 
	4, 14, 0, 18, 
	54, 70, 64, 
	8, 10, 39, 
	2, 24, 2, 12, 
	12, 
	10, 10, 39, 
	2, 24, 0, 24, 
	52, 104, 112, 
	10, 1, 44, 
	2, 34, 0, 15, 
	54, 70, 64, 
	9, 1, 24, 
	0, 7, 4, 23, 
	58, 51, 
	5, 1, 24, 
	0, 7, 
	60, 51, 
	19, 52, 0, 
	4, 22, 0, 47, 1, 46, 14, 47, 
	70, 48, 53, 64, 
	4, 23, 45, 
	3, 60, 
	127, 
	19, 52, 0, 
	4, 21, 14, 49, 0, 49, 1, 46, 
	70, 53, 48, 64, 
	6, 55, 0, 
	4, 5, 
	70, 5, 61, 
	23, 52, 0, 
	4, 23, 8, 7, 0, 57, 1, 46, 14, 57, 
	70, 53, 48, 64, 
	9, 10, 41, 
	3, 32, 0, 32, 
	52, 114, 
	10, 1, 13, 
	4, 5, 0, 2, 
	54, 70, 64, 
	5, 42, 0, 
	4, 2, 
	103, 6, 
	4, 30, 34, 
	3, 29, 
	31, 
	10, 41, 37, 
	2, 35, 1, 27, 
	18, 13, 107, 
	5, 39, 37, 
	2, 35, 
	18, 13, 
	8, 39, 37, 
	0, 0, 2, 36, 
	14, 
	10, 1, 48, 
	2, 51, 0, 24, 
	54, 70, 64, 
	14, 1, 66, 
	2, 0, 4, 1, 0, 2, 
	54, 70, 64, 
	22, 10, 10, 
	4, 2, 0, 3, 0, 2, 0, 4, 5, 4, 
	52, 1, 72, 
	13, 10, 32, 
	2, 15, 8, 10, 0, 15, 
	52, 114, 
	9, 10, 32, 
	2, 15, 9, 10, 
	18, 113, 
	7, 10, 32, 
	5, 15, 
	70, 18, 50, 64, 
	13, 56, 11, 
	3, 42, 0, 42, 0, 7, 
	72, 114, 
	10, 1, 15, 
	2, 56, 0, 25, 
	54, 70, 64, 
	0, 9, 42, 
	40, 
	4, 42, 0, 
	4, 14, 
	109, 
	0, 57, 32, 
	118, 
	17, 52, 0, 
	1, 46, 9, 10, 2, 15, 0, 10, 
	58, 114, 
	0, 53, 0, 
	18, 
	13, 39, 58, 
	1, 31, 3, 57, 0, 50, 
	30, 53, 
	0, 35, 16, 
	12, 
	4, 42, 0, 
	2, 17, 
	123, 
	2, 8, 0, 
	114, 85, 3, 
	4, 42, 0, 
	4, 6, 
	109, 
	13, 63, 0, 
	8, 10, 0, 52, 0, 53, 
	72, 119, 
	8, 42, 0, 
	4, 9, 2, 12, 
	24, 
	9, 52, 0, 
	1, 46, 0, 7, 
	49, 60, 
	4, 42, 0, 
	4, 1, 
	122, 
	10, 1, 8, 
	2, 4, 0, 3, 
	54, 70, 64, 
	4, 30, 10, 
	3, 3, 
	2, 
	0, 23, 0, 
	18, 
	0, 42, 0, 
	3, 
	10, 1, 13, 
	2, 1, 0, 5, 
	54, 70, 64, 
	1, 54, 0, 
	18, 136, 
	1, 31, 0, 
	0, 128, 
	9, 10, 38, 
	3, 28, 0, 28, 
	52, 114, 
	18, 39, 10, 
	13, 62, 3, 3, 0, 61, 14, 61, 
	114, 53, 30, 
	6, 27, 10, 
	3, 3, 
	4, 130, 46, 
	4, 30, 74, 
	3, 61, 
	120, 
	5, 27, 53, 
	3, 45, 
	4, 130, 
	5, 30, 75, 
	3, 62, 
	70, 131, 
	9, 39, 10, 
	13, 61, 3, 3, 
	3, 44, 
	5, 27, 75, 
	3, 62, 
	4, 130, 
	0, 17, 0, 
	18, 
	1, 59, 0, 
	18, 132, 
	5, 27, 13, 
	2, 1, 
	4, 47, 
	5, 27, 42, 
	2, 37, 
	4, 47, 
	1, 64, 76, 
	134, 63, 
	1, 35, 0, 
	18, 132, 
	6, 10, 78, 
	2, 23, 
	5, 10, 61, 
	1, 67, 13, 
	18, 137, 
	1, 55, 0, 
	114, 3, 
	22, 63, 0, 
	9, 10, 0, 52, 0, 53, 0, 15, 0, 55, 
	72, 72, 114, 
	2, 27, 0, 
	76, 114, 135, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84,
71, 79, 32,
195, 76, 73,
215, 65, 76,
210, 85, 78,
197, 78, 84,
208, 65, 67,
198, 79, 76,
83, 65, 89,
83, 65, 73,
71, 69, 84,
212, 65, 75,
195, 65, 84,
208, 73, 67,
210, 69, 77,
215, 69, 65,
208, 85, 76,
70, 76, 89,
68, 82, 79,
210, 69, 76,
212, 72, 82,
204, 69, 65,
199, 73, 86,
68, 82, 73,
197, 65, 84,
73, 78, 86,
83, 65, 73,
76, 79, 79,
197, 88, 65,
215, 65, 84,
82, 69, 65,
76, 73, 83,
46, 32, 32,
83, 67, 79,
83, 65, 86,
75, 73, 76,
193, 84, 84,
76, 73, 71,
46, 32, 32,
79, 80, 69,
211, 72, 65,
85, 78, 76,
72, 69, 76,
46, 32, 32,
46, 32, 32,
83, 87, 73,
81, 85, 73,
66, 85, 73,
205, 65, 75,
87, 65, 75,
83, 69, 84,
67, 65, 83,
68, 73, 71,
66, 85, 82,
70, 73, 78,
74, 85, 77,
69, 77, 80,
87, 69, 73,
32, 32, 32,
66, 82, 69,
211, 77, 65,
46, 32, 32,
32, 32, 32,
87, 65, 73,
70, 69, 69,
32, 32, 32,
32, 32, 32,
67, 76, 79,
211, 72, 85,
32, 32, 32,
32, 32, 32,
32, 32, 32,
32, 32, 32,
32, 32, 32,
32, 32, 32,
32, 32, 32,
32, 32, 32,
32, 32, 32,
32, 32, 32,
32, 32, 32,
	0,
};
const uint8_t nouns[] = {
65, 78, 89,
78, 79, 82,
83, 79, 85,
69, 65, 83,
87, 69, 83,
85, 80, 32,
68, 79, 87,
46, 32, 32,
80, 65, 83,
72, 65, 76,
66, 79, 79,
66, 79, 84,
210, 85, 77,
87, 73, 78,
71, 65, 77,
77, 79, 78,
80, 73, 82,
65, 82, 79,
66, 65, 71,
196, 85, 70,
84, 79, 82,
79, 70, 70,
77, 65, 84,
89, 79, 72,
51, 48, 32,
76, 85, 77,
82, 85, 71,
75, 69, 89,
73, 78, 86,
68, 85, 66,
83, 65, 73,
70, 73, 83,
65, 78, 67,
83, 72, 65,
80, 76, 65,
67, 65, 86,
83, 73, 71,
68, 79, 79,
67, 72, 69,
80, 65, 82,
72, 65, 77,
78, 65, 73,
66, 79, 65,
211, 72, 73,
83, 72, 69,
67, 82, 65,
87, 65, 84,
211, 65, 76,
76, 65, 71,
212, 73, 68,
80, 73, 84,
83, 72, 79,
194, 69, 65,
77, 65, 80,
80, 65, 67,
66, 79, 78,
72, 79, 76,
83, 65, 78,
66, 79, 88,
83, 78, 69,
67, 82, 65,
211, 65, 67,
80, 73, 69,
75, 69, 69,
70, 76, 79,
202, 69, 84,
83, 84, 65,
213, 80, 83,
80, 65, 84,
200, 73, 76,
89, 79, 72,
65, 87, 65,
194, 85, 78,
80, 73, 69,
78, 79, 84,
70, 76, 89,
68, 69, 83,
67, 82, 79,
83, 78, 65,
84, 82, 69,
	0,
};
const uint8_t automap[] = {
66, 79, 79,
	3,
66, 65, 71,
	5,
66, 79, 84,
	7,
84, 79, 82,
	8,
84, 79, 82,
	9,
77, 65, 84,
	10,
67, 72, 69,
	13,
77, 79, 78,
	14,
65, 78, 67,
	15,
83, 65, 73,
	20,
70, 73, 83,
	21,
68, 85, 66,
	22,
80, 65, 82,
	24,
66, 79, 84,
	25,
82, 85, 71,
	26,
75, 69, 89,
	27,
67, 72, 69,
	28,
80, 76, 65,
	29,
72, 65, 77,
	31,
78, 65, 73,
	32,
76, 85, 77,
	33,
66, 79, 84,
	42,
83, 78, 69,
	44,
77, 65, 80,
	45,
83, 72, 79,
	46,
66, 79, 78,
	47,
83, 65, 78,
	48,
66, 79, 84,
	49,
83, 84, 65,
	50,
87, 73, 78,
	54,
66, 79, 88,
	57,
67, 82, 65,
	60,
78, 79, 84,
	61,
70, 76, 89,
	62,
84, 79, 82,
	63,
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
