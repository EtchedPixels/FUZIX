#define NUM_OBJ 49
#define WORDSIZE 4
#define GAME_MAGIC 133
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
const uint8_t treasure = 0;
const uint8_t treasures = 0;
const uint8_t lastloc = 30;
const uint8_t startloc = 30;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 3, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 0, 6, 0, 2, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 5, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 0, 0, 6, 4, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 3, 0, 7, 5, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 9, 0, 0, 6, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 19, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 25, 7, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 25, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 21, 0, 0, 0, 0, 0 } }, 
		{ 	"\x72\x65\x64\x20\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 19, 13, 0, 0, 0 } }, 
		{ 	"\x72\x65\x64\x20\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 12, 0, 0 } }, 
		{ 	"\x63\x72\x61\x6D\x70\x65\x64\x20\x6D\x65\x74\x61\x6C\x20\x61\x72\x65\x61",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x73\x74\x6F\x72\x61\x67\x65\x20\x68\x6F\x6C\x64",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x74\x72\x65\x61\x64\x6D\x69\x6C\x6C",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x6F\x70\x65\x6E\x20\x64\x69\x73\x70\x6C\x61\x79\x20\x63\x61\x73\x65",
 { 0, 0, 8, 0, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 12, 8, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x61\x69\x6E\x20\x63\x6F\x6E\x74\x72\x6F\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 21, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 22, 11, 20, 23, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x6D\x65\x74\x61\x6C\x20\x63\x61\x62\x69\x6E",
 { 0, 21, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x64\x6F\x72\x6D\x69\x74\x6F\x72\x79",
 { 0, 0, 21, 27, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x66\x65\x62\x6F\x61\x74\x20\x68\x61\x6E\x67\x61\x72\x20\x6F\x70\x65\x6E\x20\x74\x6F\x0A\x6F\x75\x74\x65\x72\x20\x73\x70\x61\x63\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 10, 9, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D\x0A\x77\x69\x74\x68\x20\x70\x6F\x72\x63\x65\x6C\x61\x69\x6E\x20\x6D\x61\x63\x68\x69\x6E\x65\x72\x79",
 { 0, 0, 23, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x6F\x74\x20\x6F\x66\x20\x74\x72\x6F\x75\x62\x6C\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	26,
	26,
	1,
	2,
	0,
	0,
	4,
	0,
	0,
	0,
	0,
	20,
	0,
	0,
	0,
	10,
	12,
	0,
	8,
	10,
	20,
	11,
	0,
	0,
	11,
	0,
	0,
	0,
	16,
	8,
	0,
	17,
	15,
	15,
	2,
	0,
	2,
	15,
	15,
	15,
	11,
	11,
	0,
	22,
	23,
	22,
	9,
	9,
	0,
};


const char *objtext[] = {
	"\x4D\x61\x63\x68\x69\x6E\x65\x72\x79",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x63\x6F\x6E\x73\x6F\x6C\x65",
	"\x46\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x63\x6F\x6E\x73\x6F\x6C\x65",
	"\x50\x6C\x61\x6E\x74\x73",
	"\x46\x6C\x6F\x77\x65\x72",
	"\x46\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64",
	"\x42\x61\x6E\x64\x61\x6E\x6E\x61",
	"\x57\x68\x69\x63\x68\x20\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67",
	"\x6C\x69\x74\x65\x20\x67\x6F\x65\x73\x20\x68\x65\x72\x65\x2E\x2E\x2E\x2E\x2E",
	"\x43\x6F\x72\x70\x73\x65",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x63\x6F\x6E\x73\x6F\x6C\x65",
	"\x4E\x65\x61\x6E\x64\x65\x72\x74\x68\x61\x6C",
	"\x77\x68\x69\x63\x68\x20\x49\x27\x6D\x20\x64\x72\x61\x67\x67\x69\x6E\x67",
	"\x4C\x6F\x6E\x67\x20\x67\x6C\x6F\x77\x69\x6E\x67\x20\x6D\x65\x74\x61\x6C\x20\x74\x68\x72\x65\x61\x64",
	"\x46\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x63\x6F\x6E\x73\x6F\x6C\x65",
	"\x4D\x65",
	"\x48\x79\x64\x72\x6F\x70\x6F\x6E\x69\x63\x73",
	"\x42\x6C\x69\x6E\x6B\x69\x6E\x67\x20\x72\x65\x64\x20\x6C\x69\x67\x68\x74\x20\x6F\x76\x65\x72\x20\x66\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64",
	"\x56\x69\x65\x77\x73\x63\x72\x65\x65\x6E",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x63\x6F\x6E\x73\x6F\x6C\x65",
	"\x47\x6C\x6F\x77\x69\x6E\x67\x20\x74\x68\x72\x65\x61\x64\x20\x74\x69\x65\x64\x20\x74\x6F\x20\x6C\x65\x76\x65\x72",
	"\x4C\x6F\x6F\x73\x65\x20\x65\x6E\x64\x20\x6F\x66\x20\x67\x6C\x6F\x77\x69\x6E\x67\x20\x74\x68\x72\x65\x61\x64",
	"\x54\x72\x65\x61\x64\x6D\x69\x6C\x6C",
	"\x77\x69\x74\x68\x20\x74\x68\x72\x65\x61\x64\x20\x74\x69\x65\x64\x20\x74\x6F\x20\x69\x74",
	"\x4D\x65\x74\x65\x72\x20\x6C\x6F\x6F\x70\x20\x6F\x66\x20\x67\x6C\x6F\x77\x69\x6E\x67\x20\x74\x68\x72\x65\x61\x64",
	"\x44\x61\x72\x6B\x20\x68\x6F\x6C\x65",
	"\x52\x61\x69\x6C\x69\x6E\x67",
	"\x44\x69\x73\x70\x6C\x61\x79\x20\x63\x61\x73\x65",
	"\x77\x68\x69\x63\x68\x20\x69\x73\x20\x6F\x70\x65\x6E",
	"\x42\x75\x74\x74\x6F\x6E",
	"\x50\x69\x72\x61\x74\x65",
	"\x77\x68\x69\x63\x68\x20\x69\x73\x20\x6D\x6F\x74\x69\x6F\x6E\x6C\x65\x73\x73",
	"\x4C\x61\x72\x67\x65\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x62\x6C\x6F\x63\x6B",
	"\x47\x4C\x4F\x57\x49\x4E\x47\x20\x6C\x61\x72\x67\x65\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x62\x6C\x6F\x63\x6B",
	"\x4C\x61\x72\x67\x65\x20\x72\x61\x64\x69\x61\x6E\x74\x20\x67\x6C\x6F\x77\x69\x6E\x67\x20\x6E\x65\x6F\x6E\x20\x73\x69\x67\x6E",
	"\x44\x69\x73\x70\x6C\x61\x79\x20\x63\x61\x73\x65\x73",
	"\x47\x69\x61\x6E\x74\x20\x62\x6F\x78\x65\x73\x20\x26\x20\x63\x72\x61\x74\x65\x73",
	"\x50\x75\x6C\x73\x61\x74\x69\x6E\x67\x20\x72\x65\x64\x20\x66\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64",
	"\x4C\x61\x72\x67\x65\x20\x6D\x65\x64\x69\x63\x69\x6E\x65\x20\x62\x61\x6C\x6C",
	"\x46\x72\x65\x65\x73\x74\x61\x6E\x64\x69\x6E\x67\x20\x70\x75\x6E\x63\x68\x69\x6E\x67\x20\x62\x61\x67",
	"\x53\x63\x72\x61\x70\x73\x20\x6F\x66\x20\x63\x6C\x6F\x74\x68",
	"\x53\x6F\x66\x74\x20\x70\x6C\x61\x74\x66\x6F\x72\x6D",
	"\x52\x6F\x77\x73\x20\x6F\x66\x20\x73\x6F\x66\x74\x20\x70\x6C\x61\x74\x66\x6F\x72\x6D\x73",
	"\x53\x6D\x61\x6C\x6C\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x77\x61\x66\x65\x72",
	"\x46\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64",
	"\x77\x68\x69\x63\x68\x20\x69\x73\x20\x77\x65\x61\x6B\x6C\x79\x20\x66\x6C\x69\x63\x6B\x65\x72\x69\x6E\x67",
	"\x41\x6C\x69\x65\x6E\x20\x64\x65\x76\x69\x63\x65",
};
const char *msgptr[] = {
	"",
	"\x4F\x4B",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x31\x31\x20\x22\x53\x41\x56\x41\x47\x45\x20\x49\x53\x4C\x41\x4E\x44\x20\x50\x41\x52\x54\x20\x32\x22\x0A\x62\x79\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x20\x26\x20\x52\x75\x73\x73\x20\x57\x65\x74\x6D\x6F\x72\x65\x2E\x20\x44\x65\x64\x69\x63\x61\x74\x65\x64\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x46\x61\x6E\x73\x0A\x65\x76\x65\x72\x79\x77\x68\x65\x72\x65\x2E",
	"\x42\x6C\x69\x6E\x64\x69\x6E\x67\x20\x66\x6C\x61\x73\x68\x20\x6F\x66\x20\x6C\x69\x67\x68\x74",
	"\x49\x27\x6D\x20\x73\x74\x61\x72\x6B\x20\x6E\x61\x6B\x65\x64",
	"\x62\x75\x74\x74\x6F\x6E\x2C\x64\x69\x61\x6C\x73\x2C\x6C\x65\x76\x65\x72",
	"\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C",
	"\x65\x78\x63\x65\x70\x74",
	"\x6D\x65\x74\x61\x6C\x6C\x69\x63\x20\x76\x6F\x69\x63\x65\x20\x77\x68\x69\x73\x70\x65\x72\x73\x20\x69\x6E\x20\x6D\x79\x20\x6D\x69\x6E\x64\x3A",
	"\x6F\x6E\x65\x20\x69\x74\x65\x6D",
	"\x77\x72\x6F\x6E\x67",
	"\x22\x56\x6F\x63\x61\x6C\x69\x7A\x65\x20\x70\x61\x73\x73\x77\x6F\x72\x64\x20\x50\x6C\x65\x61\x73\x65\x22",
	"\x22\x52\x65\x61\x64\x79\x22",
	"\x61\x6C\x69\x65\x6E\x20\x73\x63\x72\x69\x70\x74",
	"\x48\x6F\x77\x3F",
	"\x64\x6F\x65\x73\x6E\x27\x74\x20\x77\x6F\x72\x6B",
	"\x73\x6F\x72\x72\x79",
	"\x49\x20\x73\x65\x65",
	"\x68\x61\x70\x70\x65\x6E\x73",
	"\x54\x69\x6D\x65\x20\x70\x61\x72\x61\x64\x6F\x78\x20\x73\x68\x61\x74\x74\x65\x72\x73\x20\x72\x65\x61\x6C\x69\x74\x79",
	"\x67\x6C\x6F\x77\x69\x6E\x67",
	"\x69\x74",
	"\x56\x61\x63\x75\x75\x6D\x21",
	"\x6C\x75\x6E\x67\x73\x20\x65\x78\x70\x6C\x6F\x64\x65\x20\x69\x6E\x20\x72\x65\x64\x20\x62\x75\x62\x62\x6C\x69\x6E\x67\x20\x72\x75\x69\x6E",
	"\x41\x72\x67\x68",
	"\x41\x53\x50\x48\x59\x58\x49\x41\x54\x45\x44",
	"\x49\x20\x62\x72\x65\x61\x74\x68\x65\x64",
	"\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67",
	"\x49\x20\x66\x65\x65\x6C",
	"\x73\x65\x64\x61\x74\x65\x64\x21",
	"\x62\x72\x69\x65\x66\x6C\x79",
	"\x4E\x65\x61\x6E\x64\x65\x72\x74\x68\x61\x6C",
	"\x74\x6F\x6F\x20\x6C\x61\x72\x67\x65",
	"\x61\x74\x74\x61\x63\x6B\x73",
	"\x6D\x6F\x76\x65\x73",
	"\x6D\x65\x74\x61\x6C\x6C\x69\x63\x20\x74\x68\x72\x65\x61\x64\x73",
	"\x22\x50\x72\x65\x70\x61\x72\x65\x20\x52\x61\x79\x73\x68\x69\x65\x6C\x64\x2C\x20\x50\x53\x59\x43\x48\x4F\x54\x52\x41\x4E\x53\x46\x49\x47\x55\x52\x41\x54\x49\x4F\x4E\x22",
	"\x74\x68\x69\x63\x6B\x20\x68\x65\x61\x64\x65\x64",
	"\x22\x4E\x6F\x74\x22",
	"\x42\x72\x61\x69\x6E\x20\x66\x72\x69\x65\x64\x21",
	"\x73\x74\x72\x61\x6E\x67\x65",
	"\x62\x6F\x64\x79",
	"\x73\x61\x79\x73",
	"\x49\x20\x66\x61\x69\x6E\x74\x65\x64",
	"\x41\x77\x61\x6B\x65\x21",
	"\x77\x6F\x6E\x27\x74\x20\x62\x75\x64\x67\x65",
	"\x4C\x69\x67\x68\x74\x20\x6D\x65\x73\x6D\x65\x72\x69\x7A\x65\x73\x20\x6D\x65",
	"\x54\x6F\x20\x77\x68\x61\x74\x3F",
	"\x73\x6C\x69\x70\x73\x20\x6F\x66\x66",
	"\x4C\x61\x72\x67\x65\x20\x54\x20\x73\x68\x61\x70\x65\x64\x20\x68\x61\x6E\x64\x6C\x65\x2E",
	"\x75\x74\x74\x65\x72\x20\x62\x6C\x61\x63\x6B\x6E\x65\x73\x73\x20\x69\x6E\x20\x63\x65\x6E\x74\x65\x72\x21",
	"\x54\x72\x79\x3A\x20\x22\x47\x4F\x22",
	"\x49\x27\x6D\x20\x68\x6F\x6C\x64\x69\x6E\x67\x20\x69\x74",
	"\x43\x6C\x6F\x73\x65\x75\x70\x73\x20\x73\x68\x6F\x77\x20\x44\x69\x6E\x6F\x73\x61\x75\x72\x73\x20\x65\x76\x65\x72\x79\x77\x68\x65\x72\x65\x21",
	"\x42\x75\x74\x74\x6F\x6E\x20\x61\x74\x20\x62\x6F\x74\x74\x6F\x6D\x2E",
	"\x43\x61\x73\x65\x20\x63\x6C\x6F\x73\x65\x73\x2C\x20\x73\x6F\x6D\x65\x20\x67\x61\x73\x20\x63\x6F\x6D\x65\x73\x20\x6F\x75\x74\x20\x61\x6E\x64\x20\x49\x27\x6D\x20\x73\x65\x64\x61\x74\x65\x64\x21",
	"\x43\x6F\x6E\x67\x72\x61\x74\x75\x6C\x61\x74\x69\x6F\x6E\x73\x21\x20\x59\x6F\x75\x27\x76\x65\x20\x66\x69\x6E\x69\x73\x68\x65\x64\x20\x53\x61\x76\x61\x67\x65\x20\x49\x73\x6C\x61\x6E\x64\x20\x73\x75\x63\x63\x65\x73\x73\x66\x75\x6C\x6C\x79\x21\x0A\x52\x65\x66\x65\x72\x20\x74\x6F\x20\x69\x6E\x73\x74\x72\x75\x63\x74\x69\x6F\x6E\x73\x20\x66\x6F\x72\x20\x66\x6F\x6C\x6C\x6F\x77\x69\x6E\x67\x20\x63\x6F\x64\x65\x64\x20\x6D\x65\x73\x73\x61\x67\x65\x3A\x0A\x55\x20\x4C\x20\x4B\x20\x23\x20\x24\x20\x2B\x20\x53\x20\x28\x20\x56\x20\x41\x20\x30\x20\x33\x20\x35\x20\x44\x20\x42\x20\x49\x20\x48\x0A\x4F\x20\x3B\x20\x37\x20\x45\x20\x38\x20\x2F\x20\x4D\x20\x50\x20\x34\x20\x4A\x20\x31\x20\x54\x20\x51\x20\x40\x20\x46\x20\x25\x20\x47\x0A\x57\x20\x58\x20\x36\x20\x43\x20\x3F\x20\x59\x20\x2C\x20\x5A\x20\x3D\x20\x26\x20\x52\x20\x2A\x20\x2D\x20\x4E\x20\x32\x20\x29\x20\x39",
	"\x69\x73\x20\x61\x20\x72\x6F\x62\x6F\x74\x21",
	"\x45\x58\x50\x4C\x4F\x53\x49\x4F\x4E\x21",
	"\x66\x6C\x79\x69\x6E\x67\x20\x61\x70\x61\x72\x74",
	"\x22\x43\x6F\x6D\x70\x61\x72\x65\x64\x20\x74\x6F\x20\x77\x68\x61\x74\x20\x79\x6F\x75\x27\x72\x65\x20\x61\x62\x6F\x75\x74\x20\x74\x6F\x20\x67\x6F\x20\x74\x68\x72\x6F\x75\x67\x68\x20\x79\x6F\x75\x27\x6C\x6C\x20\x74\x68\x69\x6E\x6B\x20\x50\x61\x72\x74\x20\x49\x20\x77\x61\x73\x20\x61\x20\x70\x69\x65\x63\x65\x20\x6F\x66\x20\x63\x61\x6B\x65\x21\x20\x47\x6F\x6F\x64\x20\x4C\x75\x63\x6B\x20\x61\x6E\x64\x20\x41\x64\x69\x65\x75\x22\x20\x2D\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x2E\x0A",
	"\x54\x69\x6D\x65\x6D\x61\x63\x68\x69\x6E\x65\x20\x73\x65\x74\x20\x66\x6F\x72\x20\x31\x30\x30\x2C\x30\x30\x30\x20\x79\x65\x61\x72\x73\x20\x69\x6E\x20\x70\x61\x73\x74\x20\x61\x6E\x64\x0A\x6E\x65\x61\x72\x65\x73\x74\x20\x70\x6C\x61\x6E\x65\x74\x61\x72\x79\x20\x62\x6F\x64\x79\x2E",
	"\x49\x20\x63\x61\x6E\x27\x74",
	"\x72\x65\x61\x64\x20\x69\x74\x21",
	"\x49\x20\x63\x61\x6E",
	"\x61\x6E\x20\x61\x74\x6F\x6D",
	"\x22\x57\x41\x52\x4E\x49\x4E\x47\x20\x2D\x20\x4E\x6F\x6E\x2D\x74\x72\x61\x6E\x73\x66\x69\x67\x75\x72\x65\x64\x20\x63\x6F\x6E\x74\x72\x6F\x6C\x6C\x65\x72\x20\x6F\x66\x20\x73\x65\x65\x64\x20\x73\x70\x65\x63\x69\x6D\x65\x6E\x73\x0A\x68\x61\x73\x20\x62\x65\x65\x6E\x20\x72\x65\x6C\x65\x61\x73\x65\x64\x22",
	"\x72\x6F\x77\x20\x61\x66\x74\x65\x72\x20\x72\x6F\x77\x20\x6F\x66",
	"\x22\x50\x75\x6C\x6C\x20\x6C\x65\x76\x65\x72\x20\x74\x6F\x20\x72\x65\x76\x65\x72\x73\x65\x20\x54\x72\x61\x6E\x73\x66\x69\x67\x75\x72\x61\x74\x69\x6F\x6E\x20\x50\x72\x6F\x63\x65\x73\x73\x22",
	"\x38\x20\x65\x6C\x65\x63\x74\x72\x6F\x6E\x73",
	"\x32\x39\x33\x20\x65\x6C\x65\x63\x74\x72\x6F\x6E\x73",
	"\x68\x61\x73",
	"\x70\x69\x63\x74\x75\x72\x65\x20\x6F\x66",
	"\x74\x68\x69\x73\x20\x73\x70\x61\x63\x65\x73\x68\x69\x70\x20\x6E\x6F\x77\x20\x6F\x72\x62\x69\x74\x69\x6E\x67\x20\x45\x61\x72\x74\x68\x2E",
	"\x48\x75\x72\x72\x69\x63\x61\x6E\x65\x20\x77\x69\x6E\x64\x73\x20\x73\x75\x63\x6B\x20\x6D\x65\x20\x69\x6E\x74\x6F\x20\x6F\x75\x74\x65\x72\x73\x70\x61\x63\x65\x21",
	"\x49\x20\x71\x75\x69\x63\x6B\x6C\x79\x20\x66\x72\x65\x65\x7A\x65\x20\x74\x6F\x20\x64\x65\x61\x74\x68\x21",
	"\x45\x78\x61\x6D\x69\x6E\x65\x20\x69\x74\x65\x6D\x73\x20\x26\x20\x72\x65\x61\x64\x20\x6D\x65\x73\x73\x61\x67\x65\x73\x20\x63\x61\x72\x65\x66\x75\x6C\x6C\x79\x21",
	"\x54\x72\x79\x3A\x20\x22\x42\x52\x45\x41\x54\x48\x45\x20\x4F\x55\x54\x22",
	"\x43\x61\x70\x74\x61\x69\x6E\x27\x73\x20\x4C\x4F\x47\x3A\x0A\x57\x6F\x75\x6E\x64\x65\x64\x20\x69\x6E\x20\x6D\x75\x74\x69\x6E\x79\x2C\x20\x73\x74\x72\x65\x6E\x67\x74\x68\x20\x66\x61\x69\x6C\x69\x6E\x67\x2E\x20\x48\x75\x6D\x61\x6E\x6F\x69\x64\x20\x73\x65\x65\x64\x20\x63\x6F\x6C\x6F\x6E\x79\x20\x6E\x6F\x74\x0A\x73\x74\x61\x72\x74\x65\x64\x2C\x20\x67\x69\x61\x6E\x74\x20\x72\x65\x70\x74\x69\x6C\x65\x73\x20\x6D\x75\x73\x74\x20\x62\x65\x20\x64\x65\x73\x74\x72\x6F\x79\x65\x64\x20\x66\x6F\x72\x20\x73\x70\x65\x63\x69\x65\x73\x20\x73\x75\x72\x76\x69\x76\x61\x6C\x2E",
	"\x54\x69\x6D\x65\x53\x77\x61\x70\x20\x50\x6C\x61\x6E\x20\x69\x6D\x70\x6C\x65\x6D\x65\x6E\x74\x65\x64\x20\x77\x69\x74\x68\x20\x52\x4F\x42\x4F\x50\x49\x52\x41\x54\x45\x20\x61\x73\x20\x63\x61\x74\x61\x6C\x79\x73\x74\x20\x61\x67\x65\x6E\x74\x2E\x0A\x49\x66\x20\x79\x6F\x75\x20\x73\x63\x61\x6E\x6E\x69\x6E\x67\x20\x74\x68\x69\x73\x20\x61\x72\x65\x20\x6E\x6F\x74\x20\x6F\x66\x20\x54\x48\x49\x53\x20\x54\x49\x4D\x45\x20\x74\x68\x65\x6E\x20\x74\x68\x65\x20\x6D\x69\x73\x73\x69\x6F\x6E\x0A\x63\x61\x6E\x20\x73\x74\x69\x6C\x6C\x20\x73\x75\x63\x63\x65\x65\x64\x21",
	"\x49\x27\x6D\x20\x6A\x65\x74\x74\x69\x73\x6F\x6E\x69\x6E\x67\x20\x73\x65\x6C\x66\x20\x66\x72\x6F\x6D\x20\x61\x69\x72\x6C\x6F\x63\x6B\x2E\x20\x47\x6F\x6F\x64\x20\x6C\x75\x63\x6B\x20\x26\x20\x66\x6F\x72\x67\x69\x76\x65\x20\x6D\x65\x20\x66\x6F\x72\x0A\x69\x6E\x76\x6F\x6C\x76\x69\x6E\x67\x20\x79\x6F\x75\x2E",
	"\x22\x4D\x61\x69\x6E\x20\x6C\x61\x6E\x64\x69\x6E\x67\x20\x73\x65\x71\x75\x65\x6E\x63\x65\x20\x73\x65\x74\x20\x66\x6F\x72\x20\x6C\x65\x76\x65\x72\x20\x70\x75\x6C\x6C\x21\x22",
	"\x52\x6F\x62\x6F\x74\x20\x50\x69\x72\x61\x74\x65\x20\x6C\x6F\x6F\x6B\x73\x20\x73\x74\x72\x61\x6E\x67\x65\x6C\x79\x20\x61\x74\x20\x6D\x65\x2C\x20\x70\x69\x63\x6B\x73\x20\x6D\x65\x20\x75\x70\x0A\x26\x20\x63\x61\x72\x72\x69\x65\x73\x20\x6D\x65\x20\x6F\x66\x66",
	"\x73\x68\x61\x70\x65\x64\x20\x66\x6F\x72\x20\x73\x74\x72\x61\x6E\x67\x65\x20\x6E\x61\x72\x72\x6F\x77\x20\x61\x6C\x69\x65\x6E\x20\x62\x6F\x64\x69\x65\x73",
	"\x73\x68\x69\x70\x27\x73\x20\x65\x6E\x67\x69\x6E\x65\x73\x2C\x20\x6C\x69\x66\x65\x20\x73\x75\x70\x70\x6F\x72\x74",
	"\x66\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64\x20\x72\x65\x73\x65\x74",
	"\x72\x65\x64\x20\x26\x20\x62\x6C\x75\x65\x20\x73\x6C\x69\x64\x65\x20\x73\x77\x69\x74\x63\x68\x65\x73",
	"\x75\x73\x65\x20\x63\x6F\x6C\x6F\x72",
	"\x61\x6C\x69\x65\x6E\x20\x76\x6F\x69\x63\x65\x20\x73\x61\x79\x73\x20\x22\x46\x52\x45\x45\x48\x4A\x4C\x4C\x47\x46\x52\x45\x45\x55\x59\x45\x22",
	"\x48\x69\x67\x68\x20\x70\x69\x74\x63\x68\x65\x64\x20\x65\x6C\x65\x63\x74\x72\x6F\x6E\x69\x63\x20\x77\x68\x69\x6E\x65",
	"\x26\x20\x61\x6E\x6F\x74\x68\x65\x72\x20\x6C\x65\x73\x73\x20\x6F\x62\x76\x69\x6F\x75\x73\x20\x64\x65\x76\x69\x63\x65",
	"\x63\x6F\x6D\x62\x69\x6E\x65\x64\x20\x6C\x61\x76\x61\x74\x6F\x72\x79\x20\x26\x20\x6D\x65\x73\x73\x20\x68\x61\x6C\x6C\x21\x20\x52\x65\x6D\x69\x6E\x64\x73\x20\x6D\x65\x20\x6F\x66\x20\x6D\x79\x0A\x6F\x6C\x64\x20\x73\x63\x68\x6F\x6F\x6C\x27\x73\x20\x63\x61\x66\x65\x74\x65\x72\x69\x61\x21\x20\x55\x47\x48\x21",
	"\x4F\x55\x43\x48\x21",
	"\x4D\x65\x20\x77\x61\x6E\x64\x65\x72\x73\x20\x6F\x66\x66\x21",
	"\x22\x50\x75\x6C\x6C\x20\x6C\x65\x76\x65\x72\x20\x70\x6C\x65\x61\x73\x65\x22",
	"",
};


const uint8_t status[] = {
	168, 
	8, 21, 0, 7, 
	60, 
	168, 
	4, 24, 0, 7, 
	58, 
	176, 
	4, 9, 13, 47, 8, 21, 0, 7, 
	60, 
	169, 
	8, 1, 9, 2, 
	10, 63, 
	171, 
	9, 1, 0, 1, 
	2, 58, 8, 11, 
	181, 
	4, 18, 3, 34, 0, 35, 0, 34, 0, 26, 
	72, 58, 
	181, 
	4, 8, 3, 34, 0, 35, 0, 34, 0, 26, 
	72, 58, 
	181, 
	4, 1, 3, 34, 0, 35, 0, 34, 0, 26, 
	72, 58, 
	181, 
	4, 11, 3, 34, 0, 35, 0, 34, 0, 26, 
	72, 58, 
	183, 
	8, 26, 0, 26, 0, 3, 0, 5, 0, 3, 
	60, 81, 79, 81, 
	164, 
	1, 40, 
	24, 
	172, 
	6, 7, 1, 8, 0, 8, 
	59, 
	170, 
	8, 18, 0, 1, 
	81, 77, 73, 
	196, 
	15, 2, 
	24, 
	211, 
	15, 0, 8, 7, 0, 6, 0, 18, 
	60, 60, 88, 88, 
	199, 
	9, 18, 
	88, 88, 25, 61, 
	214, 
	15, 0, 0, 5, 8, 18, 0, 18, 9, 7, 
	26, 60, 60, 
	196, 
	0, 1, 
	81, 
	175, 
	4, 10, 8, 21, 13, 47, 
	124, 125, 61, 19, 
	175, 
	8, 6, 8, 7, 9, 5, 
	22, 88, 88, 73, 
	195, 
	88, 88, 23, 61, 
	169, 
	8, 7, 0, 6, 
	22, 58, 
	172, 
	8, 6, 9, 7, 0, 6, 
	60, 
	143, 8, 
	8, 11, 9, 10, 3, 12, 
	31, 33, 30, 61, 
	172, 
	9, 11, 3, 12, 0, 11, 
	58, 
	175, 
	8, 10, 0, 11, 0, 2, 
	60, 81, 77, 73, 
	202, 
	15, 4, 3, 12, 
	31, 30, 34, 
	200, 
	15, 0, 0, 10, 
	60, 
	196, 
	0, 2, 
	81, 
	148, 50, 
	7, 13, 13, 12, 9, 10, 0, 12, 14, 10, 
	53, 
	134, 8, 
	13, 10, 
	66, 19, 61, 
	174, 
	9, 28, 13, 17, 0, 4, 
	81, 73, 77, 
	200, 
	15, 0, 0, 28, 
	58, 
	196, 
	0, 4, 
	81, 
	138, 30, 
	3, 12, 9, 10, 
	31, 42, 24, 
	177, 
	9, 10, 1, 13, 0, 13, 0, 12, 
	59, 53, 
	172, 
	4, 13, 2, 12, 0, 13, 
	58, 
	176, 
	4, 13, 5, 12, 0, 13, 5, 17, 
	60, 
	172, 
	4, 13, 2, 17, 0, 13, 
	58, 
	176, 
	8, 7, 3, 17, 0, 17, 0, 10, 
	72, 
	181, 
	8, 7, 1, 13, 0, 12, 0, 10, 0, 13, 
	72, 59, 
	177, 
	4, 1, 2, 17, 0, 17, 0, 2, 
	143, 62, 
	178, 
	3, 23, 5, 21, 0, 23, 5, 21, 
	75, 35, 48, 
	183, 
	14, 33, 0, 17, 7, 17, 0, 7, 7, 30, 
	132, 54, 76, 60, 
	174, 
	7, 30, 13, 35, 0, 3, 
	81, 77, 73, 
	198, 
	15, 0, 
	108, 19, 61, 
	196, 
	0, 3, 
	81, 
	167, 
	1, 45, 
	8, 128, 88, 73, 
	195, 
	88, 88, 88, 129, 
	195, 
	88, 88, 88, 130, 
	180, 
	14, 33, 8, 25, 12, 17, 14, 10, 0, 22, 
	58, 
	177, 
	4, 18, 2, 17, 0, 17, 0, 2, 
	143, 62, 
	168, 
	4, 30, 8, 2, 
	63, 
};
const uint8_t actions[] = {
	10, 58, 68, 
	3, 48, 9, 7, 
	1, 73, 138, 
	206, 
	4, 2, 9, 4, 0, 4, 
	58, 8, 12, 
	207, 
	4, 12, 9, 14, 0, 14, 
	58, 8, 36, 12, 
	10, 23, 73, 
	3, 48, 9, 15, 
	1, 17, 136, 
	5, 58, 12, 
	3, 48, 
	139, 73, 
	203, 
	3, 33, 0, 33, 
	59, 88, 88, 88, 
	9, 10, 73, 
	3, 48, 0, 48, 
	1, 52, 
	13, 10, 73, 
	4, 26, 14, 48, 0, 48, 
	1, 52, 
	11, 7, 9, 
	3, 3, 8, 12, 
	40, 114, 113, 111, 
	4, 58, 55, 
	3, 48, 
	137, 
	11, 7, 9, 
	3, 21, 8, 12, 
	122, 49, 122, 115, 
	11, 23, 46, 
	4, 26, 9, 15, 
	1, 17, 134, 73, 
	196, 
	14, 48, 
	140, 
	0, 37, 0, 
	126, 
	0, 68, 0, 
	127, 
	1, 1, 70, 
	16, 112, 
	0, 17, 28, 
	14, 
	10, 23, 17, 
	2, 1, 9, 15, 
	1, 17, 5, 
	10, 23, 17, 
	2, 11, 9, 15, 
	1, 17, 5, 
	4, 27, 19, 
	2, 11, 
	1, 
	4, 27, 19, 
	2, 21, 
	1, 
	23, 1, 11, 
	2, 2, 4, 18, 0, 2, 0, 2, 0, 7, 
	54, 53, 58, 73, 
	200, 
	3, 35, 0, 27, 
	60, 
	5, 1, 11, 
	3, 39, 
	40, 112, 
	23, 1, 11, 
	8, 24, 4, 2, 0, 18, 0, 2, 0, 7, 
	54, 53, 60, 73, 
	200, 
	3, 34, 0, 27, 
	58, 
	23, 1, 11, 
	9, 24, 4, 2, 0, 1, 0, 2, 0, 7, 
	54, 53, 60, 73, 
	200, 
	3, 34, 0, 27, 
	58, 
	9, 1, 11, 
	2, 19, 9, 15, 
	16, 46, 
	23, 1, 11, 
	2, 15, 4, 10, 0, 11, 0, 15, 0, 7, 
	54, 53, 60, 76, 
	23, 1, 11, 
	2, 15, 4, 11, 0, 10, 0, 15, 0, 7, 
	54, 53, 58, 76, 
	19, 10, 42, 
	9, 7, 0, 1, 0, 5, 0, 1, 
	81, 79, 81, 73, 
	201, 
	0, 18, 0, 5, 
	58, 60, 
	23, 1, 11, 
	2, 6, 4, 4, 0, 8, 0, 6, 0, 7, 
	54, 53, 60, 76, 
	23, 1, 11, 
	2, 6, 4, 8, 0, 4, 0, 6, 0, 7, 
	54, 53, 58, 76, 
	15, 38, 0, 
	8, 8, 9, 7, 0, 8, 
	43, 60, 88, 88, 
	23, 38, 0, 
	0, 8, 9, 7, 0, 5, 0, 18, 9, 8, 
	1, 58, 60, 60, 
	19, 39, 27, 
	0, 8, 9, 7, 0, 5, 0, 18, 
	1, 58, 60, 60, 
	6, 56, 47, 
	1, 26, 
	1, 32, 48, 
	19, 39, 28, 
	9, 5, 0, 5, 0, 18, 0, 1, 
	58, 58, 81, 73, 
	206, 
	0, 5, 0, 1, 9, 8, 
	79, 81, 1, 
	215, 
	0, 7, 0, 1, 0, 8, 8, 8, 9, 12, 
	79, 81, 60, 1, 
	215, 
	0, 17, 0, 1, 0, 8, 8, 8, 8, 12, 
	79, 81, 60, 1, 
	23, 1, 11, 
	2, 2, 4, 1, 0, 2, 0, 2, 0, 7, 
	54, 53, 58, 73, 
	200, 
	3, 35, 0, 27, 
	60, 
	19, 39, 29, 
	9, 7, 0, 1, 0, 5, 0, 1, 
	81, 79, 81, 73, 
	201, 
	0, 18, 0, 5, 
	58, 60, 
	19, 29, 13, 
	0, 2, 0, 7, 9, 2, 9, 18, 
	58, 52, 73, 3, 
	199, 
	0, 1, 
	4, 7, 9, 54, 
	15, 29, 16, 
	9, 2, 0, 2, 0, 1, 
	3, 58, 4, 54, 
	9, 27, 19, 
	2, 1, 0, 47, 
	1, 59, 
	9, 23, 17, 
	3, 3, 9, 15, 
	1, 5, 
	10, 27, 19, 
	3, 3, 9, 4, 
	1, 8, 11, 
	17, 27, 19, 
	2, 16, 8, 13, 8, 14, 8, 12, 
	8, 144, 
	5, 23, 9, 
	9, 15, 
	1, 13, 
	7, 23, 24, 
	9, 15, 
	1, 17, 21, 20, 
	23, 23, 30, 
	9, 9, 3, 4, 0, 9, 0, 5, 0, 4, 
	27, 58, 75, 76, 
	15, 46, 31, 
	3, 5, 9, 7, 9, 18, 
	1, 28, 30, 29, 
	11, 40, 31, 
	3, 5, 0, 5, 
	29, 28, 25, 61, 
	13, 48, 31, 
	3, 5, 0, 5, 9, 7, 
	59, 73, 
	199, 
	9, 18, 
	29, 28, 25, 61, 
	202, 
	2, 12, 0, 10, 
	58, 31, 29, 
	210, 
	2, 12, 0, 2, 0, 40, 0, 2, 
	81, 79, 81, 
	5, 10, 32, 
	2, 12, 
	14, 32, 
	19, 66, 32, 
	2, 12, 0, 12, 0, 13, 8, 10, 
	1, 52, 52, 66, 
	13, 48, 31, 
	3, 5, 0, 5, 8, 7, 
	1, 59, 
	11, 51, 32, 
	3, 12, 9, 10, 
	31, 33, 30, 61, 
	6, 1, 71, 
	4, 24, 
	1, 125, 61, 
	4, 58, 32, 
	3, 12, 
	1, 
	10, 23, 22, 
	3, 37, 9, 15, 
	17, 117, 31, 
	14, 18, 32, 
	1, 12, 0, 12, 0, 13, 
	1, 53, 59, 
	14, 56, 47, 
	3, 7, 0, 7, 0, 8, 
	1, 52, 52, 
	9, 57, 47, 
	3, 8, 0, 8, 
	1, 59, 
	18, 59, 47, 
	3, 7, 0, 8, 0, 7, 0, 14, 
	1, 59, 72, 
	10, 23, 17, 
	2, 16, 9, 15, 
	1, 17, 5, 
	14, 51, 66, 
	3, 41, 0, 41, 0, 42, 
	1, 108, 72, 
	14, 27, 19, 
	2, 16, 8, 13, 9, 14, 
	1, 8, 11, 
	11, 27, 19, 
	2, 16, 9, 13, 
	8, 36, 38, 12, 
	6, 1, 60, 
	2, 27, 
	1, 80, 76, 
	23, 27, 19, 
	2, 16, 8, 13, 8, 14, 0, 12, 9, 12, 
	1, 3, 73, 59, 
	198, 
	6, 8, 
	39, 61, 63, 
	211, 
	0, 17, 0, 8, 0, 7, 0, 14, 
	53, 59, 59, 60, 
	203, 
	0, 13, 0, 12, 
	54, 58, 56, 76, 
	203, 
	0, 19, 0, 13, 
	28, 40, 58, 60, 
	200, 
	3, 4, 0, 4, 
	59, 
	206, 
	0, 4, 0, 25, 0, 4, 
	81, 79, 81, 
	9, 23, 46, 
	4, 27, 9, 15, 
	17, 141, 
	6, 61, 38, 
	8, 15, 
	1, 57, 76, 
	11, 23, 36, 
	9, 15, 8, 12, 
	1, 17, 31, 41, 
	5, 69, 47, 
	3, 23, 
	1, 47, 
	23, 29, 34, 
	2, 16, 8, 13, 9, 14, 9, 12, 0, 14, 
	58, 8, 36, 12, 
	2, 63, 38, 
	1, 56, 76, 
	9, 40, 30, 
	3, 4, 0, 4, 
	1, 59, 
	10, 58, 68, 
	3, 48, 8, 7, 
	1, 6, 18, 
	1, 27, 20, 
	16, 45, 
	1, 66, 20, 
	16, 45, 
	15, 23, 44, 
	3, 18, 14, 4, 0, 4, 
	17, 27, 53, 76, 
	5, 69, 47, 
	3, 14, 
	1, 47, 
	9, 23, 17, 
	3, 21, 9, 15, 
	17, 5, 
	9, 23, 20, 
	3, 21, 9, 15, 
	17, 49, 
	23, 71, 20, 
	3, 21, 0, 14, 0, 22, 3, 14, 0, 23, 
	59, 53, 53, 1, 
	18, 71, 20, 
	3, 21, 0, 23, 3, 23, 0, 22, 
	59, 53, 1, 
	18, 71, 59, 
	3, 21, 3, 23, 0, 23, 0, 25, 
	59, 53, 1, 
	23, 71, 59, 
	3, 21, 3, 14, 0, 14, 0, 25, 0, 23, 
	59, 53, 53, 1, 
	23, 70, 47, 
	3, 23, 0, 25, 0, 22, 0, 23, 0, 14, 
	59, 59, 72, 1, 
	23, 70, 47, 
	3, 25, 0, 23, 0, 22, 0, 25, 0, 14, 
	59, 59, 72, 1, 
	11, 23, 14, 
	3, 21, 9, 15, 
	1, 21, 121, 119, 
	13, 70, 47, 
	3, 26, 0, 26, 0, 14, 
	72, 1, 
	11, 23, 14, 
	3, 34, 9, 15, 
	1, 21, 121, 120, 
	13, 71, 47, 
	3, 14, 0, 26, 0, 14, 
	72, 1, 
	5, 69, 47, 
	3, 26, 
	16, 15, 
	5, 69, 47, 
	3, 14, 
	16, 15, 
	10, 23, 47, 
	3, 26, 9, 15, 
	1, 17, 50, 
	7, 1, 47, 
	1, 26, 
	1, 28, 27, 30, 
	5, 1, 47, 
	2, 26, 
	1, 73, 
	211, 
	2, 26, 4, 2, 0, 15, 0, 27, 
	80, 54, 53, 76, 
	211, 
	2, 26, 4, 20, 0, 26, 0, 27, 
	80, 54, 53, 76, 
	207, 
	2, 26, 0, 14, 0, 27, 
	80, 54, 53, 76, 
	11, 66, 32, 
	3, 12, 9, 10, 
	31, 33, 30, 61, 
	7, 42, 47, 
	3, 26, 
	1, 28, 27, 40, 
	5, 7, 58, 
	3, 36, 
	17, 110, 
	11, 23, 57, 
	3, 34, 9, 15, 
	17, 122, 115, 109, 
	9, 18, 53, 
	3, 28, 0, 20, 
	1, 60, 
	18, 23, 41, 
	8, 19, 3, 10, 0, 19, 0, 7, 
	9, 60, 53, 
	10, 1, 59, 
	2, 24, 0, 16, 
	1, 54, 76, 
	10, 10, 53, 
	3, 28, 0, 20, 
	1, 58, 102, 
	14, 8, 0, 
	4, 16, 9, 20, 0, 11, 
	1, 54, 76, 
	14, 9, 0, 
	4, 16, 9, 20, 0, 11, 
	1, 54, 76, 
	13, 8, 0, 
	4, 16, 13, 25, 13, 22, 
	16, 45, 
	23, 9, 0, 
	4, 16, 13, 25, 13, 22, 9, 21, 0, 21, 
	58, 49, 34, 30, 
	17, 9, 0, 
	4, 16, 13, 25, 13, 22, 8, 21, 
	16, 45, 
	5, 66, 47, 
	3, 23, 
	16, 45, 
	10, 23, 22, 
	2, 29, 9, 15, 
	17, 104, 73, 
	197, 
	9, 23, 
	17, 31, 
	19, 27, 19, 
	2, 29, 9, 23, 0, 23, 0, 30, 
	58, 53, 17, 73, 
	199, 
	0, 12, 
	53, 31, 8, 116, 
	10, 27, 19, 
	2, 31, 9, 22, 
	105, 19, 61, 
	10, 27, 19, 
	2, 31, 8, 22, 
	105, 106, 63, 
	6, 23, 56, 
	3, 32, 
	40, 21, 107, 
	19, 27, 19, 
	3, 3, 8, 4, 9, 24, 0, 24, 
	1, 58, 3, 73, 
	205, 
	8, 27, 0, 25, 0, 35, 
	58, 59, 
	7, 23, 54, 
	3, 20, 
	17, 122, 123, 73, 
	196, 
	9, 25, 
	103, 
	19, 27, 19, 
	3, 3, 8, 24, 8, 4, 0, 24, 
	1, 60, 3, 73, 
	205, 
	8, 27, 0, 25, 0, 35, 
	58, 59, 
	10, 8, 0, 
	4, 16, 8, 20, 
	1, 21, 34, 
	10, 9, 0, 
	4, 16, 8, 20, 
	1, 21, 34, 
	10, 1, 22, 
	2, 30, 0, 17, 
	1, 54, 76, 
	15, 23, 32, 
	3, 29, 9, 23, 9, 15, 
	1, 17, 37, 31, 
	12, 23, 32, 
	3, 12, 9, 15, 8, 10, 
	29, 
	13, 29, 0, 
	9, 7, 8, 12, 9, 18, 
	1, 24, 
	11, 23, 32, 
	3, 12, 9, 15, 
	1, 17, 37, 31, 
	11, 23, 47, 
	3, 7, 9, 15, 
	6, 7, 20, 35, 
	14, 7, 9, 
	2, 1, 8, 12, 9, 15, 
	1, 17, 135, 
	14, 7, 9, 
	2, 11, 8, 12, 9, 15, 
	1, 17, 131, 
	15, 7, 9, 
	2, 16, 8, 12, 9, 15, 
	40, 114, 113, 118, 
	0, 6, 0, 
	51, 
	0, 9, 0, 
	51, 
	0, 8, 0, 
	51, 
	2, 71, 0, 
	16, 15, 48, 
	1, 51, 0, 
	16, 15, 
	4, 45, 0, 
	9, 15, 
	66, 
	4, 10, 7, 
	9, 15, 
	66, 
	1, 33, 0, 
	16, 15, 
	13, 27, 19, 
	2, 29, 14, 30, 0, 30, 
	1, 53, 
	13, 29, 0, 
	9, 7, 9, 12, 9, 18, 
	1, 85, 
	1, 35, 8, 
	1, 71, 
	1, 36, 0, 
	1, 63, 
	2, 42, 0, 
	1, 28, 6, 
	1, 62, 0, 
	1, 85, 
	6, 7, 0, 
	9, 15, 
	16, 112, 113, 
	5, 27, 32, 
	3, 12, 
	21, 34, 
	9, 27, 19, 
	3, 30, 0, 30, 
	1, 59, 
	23, 1, 11, 
	2, 47, 4, 9, 0, 24, 0, 46, 0, 47, 
	1, 54, 53, 53, 
	23, 1, 11, 
	2, 47, 4, 24, 0, 9, 0, 46, 0, 47, 
	1, 54, 53, 53, 
	18, 1, 11, 
	5, 47, 4, 9, 0, 24, 0, 46, 
	1, 54, 53, 
	18, 1, 11, 
	5, 47, 4, 24, 0, 9, 0, 46, 
	1, 54, 53, 
	1, 61, 0, 
	16, 112, 
	4, 23, 70, 
	9, 15, 
	133, 
	9, 23, 60, 
	2, 27, 9, 15, 
	17, 50, 
	18, 66, 47, 
	3, 7, 0, 8, 0, 7, 0, 14, 
	1, 59, 72, 
	5, 1, 74, 
	4, 24, 
	16, 112, 
	15, 39, 0, 
	9, 7, 0, 5, 0, 18, 
	1, 60, 60, 26, 
	5, 73, 64, 
	3, 40, 
	1, 142, 
	8, 23, 35, 
	9, 15, 3, 17, 
	73, 
	196, 
	9, 28, 
	29, 
	206, 
	8, 19, 0, 7, 0, 19, 
	9, 53, 60, 
	196, 
	8, 28, 
	44, 
	7, 23, 0, 
	9, 15, 
	1, 17, 6, 76, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
197, 78, 84, 69,
195, 82, 65, 87,
32, 32, 32, 32,
32, 32, 32, 32,
74, 85, 77, 80,
82, 69, 65, 68,
87, 65, 76, 75,
82, 85, 78, 32,
71, 69, 84, 32,
212, 65, 75, 69,
199, 82, 65, 66,
208, 73, 67, 75,
200, 79, 76, 68,
195, 65, 84, 67,
46, 32, 32, 32,
87, 79, 82, 75,
68, 82, 79, 80,
204, 69, 65, 86,
208, 85, 84, 32,
210, 69, 76, 69,
199, 73, 86, 69,
76, 79, 79, 75,
197, 88, 65, 77,
196, 69, 83, 67,
211, 69, 69, 32,
80, 85, 83, 72,
208, 82, 69, 83,
83, 65, 89, 32,
211, 80, 69, 65,
212, 65, 76, 75,
193, 83, 75, 32,
85, 83, 69, 32,
215, 73, 84, 72,
83, 65, 86, 69,
81, 85, 73, 84,
72, 69, 76, 80,
72, 89, 80, 69,
66, 82, 69, 65,
69, 65, 84, 32,
212, 65, 83, 84,
70, 69, 69, 76,
212, 79, 85, 67,
32, 32, 32, 32,
73, 78, 86, 69,
83, 77, 69, 76,
211, 78, 73, 70,
67, 82, 85, 83,
195, 82, 85, 77,
211, 77, 65, 83,
72, 73, 84, 32,
212, 69, 65, 82,
210, 73, 80, 32,
208, 85, 78, 67,
46, 32, 32, 32,
87, 69, 65, 82,
82, 69, 77, 79,
77, 79, 86, 69,
85, 78, 82, 65,
213, 78, 68, 79,
79, 80, 69, 78,
84, 72, 73, 78,
67, 76, 79, 83,
211, 72, 85, 84,
32, 32, 32, 32,
68, 82, 65, 71,
208, 85, 76, 76,
69, 88, 72, 65,
84, 73, 69, 32,
85, 78, 84, 73,
84, 79, 32, 32,
83, 67, 79, 82,
75, 73, 67, 75,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
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
73, 78, 86, 69,
71, 65, 77, 69,
68, 73, 65, 76,
211, 67, 82, 73,
70, 73, 69, 76,
66, 76, 85, 69,
49, 50, 51, 32,
65, 84, 79, 77,
208, 73, 67, 84,
52, 55, 52, 32,
67, 79, 78, 84,
195, 79, 78, 83,
66, 85, 84, 84,
76, 69, 86, 69,
200, 65, 78, 68,
68, 73, 83, 80,
195, 65, 83, 69,
77, 69, 84, 65,
212, 85, 78, 78,
210, 79, 79, 77,
68, 69, 69, 80,
79, 85, 84, 32,
73, 78, 32, 32,
80, 76, 65, 78,
70, 76, 79, 87,
78, 69, 65, 78,
195, 65, 86, 69,
70, 82, 69, 69,
77, 69, 32, 32,
83, 69, 76, 70,
205, 89, 83, 69,
69, 89, 69, 83,
70, 82, 69, 69,
65, 82, 71, 72,
67, 79, 82, 80,
66, 82, 69, 65,
80, 65, 83, 83,
72, 89, 68, 82,
76, 73, 71, 72,
77, 65, 67, 72,
66, 65, 78, 68,
212, 72, 82, 69,
204, 79, 79, 80,
201, 84, 83, 69,
197, 78, 68, 32,
195, 69, 78, 84,
82, 65, 73, 76,
86, 73, 69, 87,
83, 76, 73, 68,
80, 73, 82, 65,
66, 76, 79, 67,
83, 73, 71, 78,
84, 82, 69, 65,
72, 79, 76, 69,
66, 79, 88, 69,
195, 82, 65, 84,
194, 79, 88, 32,
66, 65, 76, 76,
205, 69, 68, 73,
66, 65, 71, 32,
195, 76, 79, 84,
82, 69, 68, 32,
87, 65, 70, 69,
80, 76, 65, 84,
79, 85, 84, 69,
211, 80, 65, 67,
68, 69, 86, 73,
76, 73, 70, 69,
70, 65, 83, 84,
83, 76, 79, 87,
72, 65, 78, 71,
69, 78, 71, 73,
32, 32, 32, 32,
	0,
};
const uint8_t automap[] = {
80, 76, 65, 78,
	4,
70, 76, 79, 87,
	5,
66, 65, 78, 68,
	7,
67, 79, 82, 80,
	10,
66, 65, 78, 68,
	14,
77, 69, 32, 32,
	17,
66, 65, 78, 68,
	23,
66, 65, 78, 68,
	26,
66, 76, 79, 67,
	34,
66, 76, 79, 67,
	35,
66, 65, 76, 76,
	40,
66, 65, 71, 32,
	41,
66, 65, 71, 32,
	42,
87, 65, 70, 69,
	45,
68, 69, 86, 73,
	48,
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
