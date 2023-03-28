#define NUM_OBJ 72
#define WORDSIZE 4
#define GAME_MAGIC 313
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
const uint8_t startlamp = 1;
const uint8_t lightfill = 1;
const uint8_t startcarried = 0;
const uint8_t maxcar = 10;
const uint8_t treasure = 13;
const uint8_t treasures = 13;
const uint8_t lastloc = 24;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x62\x6F\x74\x74\x6F\x6D\x20\x62\x75\x6E\x6B",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x68\x69\x70\x27\x73\x20\x63\x61\x62\x69\x6E",
 { 0, 0, 0, 0, 4, 18 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x64\x6F\x63\x6B",
 { 5, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x64\x65\x63\x6B",
 { 0, 0, 0, 0, 0, 2 } }, 
		{ 	"\x62\x65\x61\x63\x68\x20\x62\x79\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x68\x69\x6C\x6C",
 { 0, 3, 15, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x6C\x65\x64\x67\x65\x20\x38\x20\x66\x65\x65\x74\x20\x62\x65\x6C\x6F\x77\x20\x68\x69\x6C\x6C\x20\x73\x75\x6D\x6D\x69\x74",
 { 0, 0, 0, 0, 0, 5 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x6F\x70\x20\x6F\x66\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x68\x69\x6C\x6C",
 { 0, 0, 0, 0, 0, 10 } }, 
		{ 	"\x63\x61\x76\x65\x72\x6E",
 { 0, 0, 0, 14, 0, 0 } }, 
		{ 	"\x74\x6F\x6F\x6C\x20\x73\x68\x65\x64",
 { 8, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x53\x6F\x72\x72\x79\x2C\x20\x74\x6F\x20\x65\x78\x70\x6C\x6F\x72\x65\x20\x50\x69\x72\x61\x74\x65\x27\x73\x20\x49\x73\x6C\x65\x20\x79\x6F\x75\x27\x6C\x6C\x20\x6E\x65\x65\x64\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x23\x32",
 { 0, 0, 0, 0, 7, 0 } }, 
		{ 	"\x73\x65\x61",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x75\x6E\x64\x65\x72\x73\x65\x61",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x6D\x75\x67\x67\x6C\x65\x72\x73\x20\x68\x6F\x6C\x64\x20\x69\x6E\x73\x69\x64\x65\x20\x73\x68\x69\x70",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x68\x61\x6C\x6C",
 { 0, 0, 8, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x72\x6F\x63\x6B\x79\x20\x62\x65\x61\x63\x68\x20\x62\x79\x20\x73\x65\x61",
 { 0, 0, 0, 5, 0, 0 } }, 
		{ 	"\x74\x6F\x70\x20\x62\x75\x6E\x6B",
 { 0, 0, 0, 0, 0, 2 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x68\x65\x20\x65\x6E\x67\x69\x6E\x65",
 { 0, 0, 0, 0, 0, 18 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x65\x6E\x67\x69\x6E\x65\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 2, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x63\x61\x74\x77\x61\x6C\x6B\x20\x6F\x6E\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x6F\x66\x20\x73\x68\x69\x70\x20\x62\x79\x20\x70\x6F\x72\x74\x68\x6F\x6C\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6E\x61\x72\x72\x6F\x77\x20\x63\x72\x61\x77\x6C\x77\x61\x79",
 { 0, 0, 21, 13, 0, 0 } }, 
		{ 	"\x73\x68\x69\x70\x27\x73\x20\x68\x6F\x6C\x64",
 { 0, 0, 0, 20, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x62\x65\x61\x6D\x20\x75\x6E\x64\x65\x72\x20\x64\x6F\x63\x6B",
 { 0, 0, 0, 0, 0, 11 } }, 
		{ 	"\x73\x75\x6E\x6B\x65\x6E\x20\x73\x68\x69\x70",
 { 0, 0, 0, 0, 12, 0 } }, 
		{ 	"\x4E\x65\x76\x65\x72\x20\x4E\x65\x76\x65\x72\x20\x4C\x61\x6E\x64",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	7,
	2,
	0,
	5,
	0,
	8,
	9,
	7,
	0,
	0,
	0,
	0,
	2,
	0,
	0,
	0,
	2,
	16,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	3,
	0,
	0,
	0,
	0,
	13,
	21,
	4,
	4,
	0,
	12,
	0,
	16,
	11,
	12,
	0,
	0,
	21,
	3,
	0,
	2,
	0,
	2,
	0,
	0,
	0,
	0,
	0,
	13,
	0,
	0,
	0,
	19,
	0,
	2,
	0,
	0,
	0,
	0,
	23,
	22,
	0,
	0,
	0,
	0,
	0,
	0,
};


const char *objtext[] = {
	"\x4E\x61\x72\x72\x6F\x77\x20\x63\x72\x61\x63\x6B\x20\x69\x6E\x20\x72\x6F\x63\x6B",
	"\x47\x6C\x61\x73\x73\x65\x73",
	"\x77\x68\x69\x63\x68\x20\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67",
	"\x53\x69\x67\x6E",
	"\x49\x73\x6C\x65\x20\x6F\x66\x66\x20\x69\x6E\x20\x64\x69\x73\x74\x61\x6E\x63\x65",
	"\x54\x6F\x6F\x6C\x73\x68\x65\x64",
	"\x48\x61\x6D\x6D\x65\x72",
	"\x53\x6C\x65\x65\x70\x69\x6E\x67\x20\x50\x69\x72\x61\x74\x65",
	"\x42\x6F\x78",
	"\x2E",
	"\x2A\x52\x61\x72\x65\x20\x53\x74\x61\x6D\x70\x73\x2A",
	"\x2A\x44\x6F\x75\x62\x6C\x6F\x6F\x6E\x73\x2A",
	"\x50\x61\x69\x6E\x74\x69\x6E\x67",
	"\x50\x69\x63\x74\x75\x72\x65\x20\x46\x72\x61\x6D\x65",
	"\x50\x69\x63\x74\x75\x72\x65",
	"\x4D\x61\x70",
	"\x42\x75\x6E\x6B\x20\x62\x65\x64",
	"\x46\x61\x63\x65\x20\x6D\x61\x73\x6B",
	"\x77\x68\x69\x63\x68\x20\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67",
	"\x57\x69\x72\x65\x20\x72\x69\x6D",
	"\x4C\x65\x6E\x73\x65\x73",
	"\x52\x65\x64\x20\x61\x6C\x67\x61\x65",
	"\x47\x6C\x75\x65",
	"\x2A\x50\x69\x65\x63\x65\x20\x6F\x66\x20\x41\x6D\x62\x65\x72\x2A",
	"\x42\x6F\x6F\x6B\x6C\x65\x74",
	"\x53\x65\x61",
	"\x44\x6F\x63\x6B\x20\x74\x6F\x20\x74\x68\x65\x20\x45\x61\x73\x74",
	"\x55\x6E\x64\x65\x72\x73\x69\x64\x65\x20\x6F\x66\x20\x44\x6F\x63\x6B",
	"\x42\x6F\x61\x74\x20\x74\x6F\x20\x74\x68\x65\x20\x57\x45\x53\x54",
	"\x49\x6E\x73\x69\x64\x65\x20\x6F\x66\x20\x62\x6F\x61\x74",
	"\x50\x6F\x6F\x6C\x20\x6F\x66\x20\x77\x61\x74\x65\x72",
	"\x41\x6C\x61\x72\x6D\x20\x63\x6C\x6F\x63\x6B",
	"\x44\x6F\x63\x6B",
	"\x53\x65\x61",
	"\x50\x75\x6D\x69\x63\x65\x20\x72\x6F\x63\x6B",
	"\x53\x69\x6C\x74",
	"\x52\x6F\x63\x6B\x79\x20\x62\x65\x61\x63\x68",
	"\x2A\x44\x69\x61\x6D\x6F\x6E\x64\x20\x77\x61\x74\x63\x68\x2A",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x63\x75\x72\x72\x65\x6E\x74",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x63\x75\x72\x72\x65\x6E\x74",
	"\x46\x6C\x6F\x74\x73\x61\x6D\x20\x26\x20\x4A\x65\x74\x73\x61\x6D",
	"\x52\x75\x6D",
	"\x53\x69\x67\x6E",
	"\x42\x6F\x61\x74",
	"\x4F\x79\x73\x74\x65\x72",
	"\x50\x6F\x72\x74\x68\x6F\x6C\x65",
	"\x46\x61\x6E\x20\x62\x6C\x61\x64\x65",
	"\x48\x65\x6C\x6D",
	"\x4F\x70\x65\x6E\x20\x70\x6F\x72\x74\x68\x6F\x6C\x65",
	"\x53\x63\x72\x65\x77\x64\x72\x69\x76\x65\x72",
	"\x52\x75\x69\x6E\x65\x64\x20\x77\x61\x74\x65\x72\x73\x6F\x61\x6B\x65\x64\x20\x70\x61\x69\x6E\x74\x69\x6E\x67",
	"\x2A\x52\x65\x6D\x62\x72\x61\x6E\x64\x74\x20\x50\x61\x69\x6E\x74\x69\x6E\x67\x2A",
	"\x50\x69\x72\x61\x74\x65\x20\x61\x74\x20\x68\x65\x6C\x6D",
	"\x56\x65\x72\x79\x20\x6E\x61\x72\x72\x6F\x77\x20\x63\x72\x61\x77\x6C\x77\x61\x79",
	"\x57\x61\x73\x68\x65\x64\x20\x6F\x75\x74\x20\x73\x69\x67\x6E",
	"\x2A\x47\x6F\x6C\x64\x20\x45\x61\x72\x72\x69\x6E\x67\x2A",
	"\x2A\x52\x61\x72\x65\x20\x42\x6F\x6F\x6B\x2A",
	"\x4F\x69\x6C\x73\x6B\x69\x6E\x20\x72\x61\x69\x6E\x63\x6F\x61\x74",
	"\x57\x61\x74\x65\x72\x70\x72\x6F\x6F\x66\x20\x70\x61\x69\x6E\x74\x69\x6E\x67",
	"\x43\x65\x69\x6C\x69\x6E\x67\x20\x66\x61\x6E",
	"\x2A\x44\x69\x61\x6D\x6F\x6E\x64\x20\x42\x72\x6F\x6F\x63\x68\x2A",
	"\x53\x6D\x61\x6C\x6C\x20\x62\x75\x74\x74\x6F\x6E\x20\x69\x6E\x20\x63\x65\x69\x6C\x69\x6E\x67",
	"\x57\x61\x74\x65\x72\x73\x6F\x61\x6B\x65\x64\x20\x62\x6F\x6F\x6B",
	"\x53\x75\x6E\x6B\x65\x6E\x20\x53\x68\x69\x70",
	"\x2A\x4A\x65\x77\x65\x6C\x65\x64\x20\x63\x68\x65\x73\x74\x2A",
	"\x2A\x53\x69\x6C\x76\x65\x72\x20\x44\x6F\x6C\x6C\x61\x72\x2A",
	"\x2A\x50\x65\x61\x72\x6C\x2A",
	"\x53\x6E\x61\x69\x6C",
	"\x44\x65\x61\x64\x20\x73\x6E\x61\x69\x6C",
	"\x2A\x44\x69\x61\x6D\x6F\x6E\x64\x20\x50\x69\x6E\x2A",
	"\x2A\x44\x69\x61\x6D\x6F\x6E\x64\x20\x52\x69\x6E\x67\x2A",
	"\x53\x6D\x61\x73\x68\x65\x64\x20\x4F\x79\x73\x74\x65\x72",
};
const char *msgptr[] = {
	"",
	"\x4F\x2E\x4B\x2E",
	"\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x31\x34\x20\x62\x79\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x2E",
	"\x53\x6F\x72\x72\x79\x2C",
	"\x45\x76\x65\x72\x79\x74\x68\x69\x6E\x67\x20\x69\x73",
	"\x6E\x6F\x74\x20\x65\x78\x61\x63\x74\x6C\x79\x20\x64\x61\x72\x6B\x2C\x20\x62\x75\x74\x20\x69\x74\x27\x73\x20\x74\x6F\x6F\x20\x46\x55\x5A\x5A\x59\x20\x74\x6F\x20\x73\x65\x65\x21",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x42\x61\x63\x6B\x73\x69\x64\x65\x20\x6F\x66\x20\x50\x69\x72\x61\x74\x65\x27\x73\x20\x49\x73\x6C\x65\x2E",
	"\x49\x20\x73\x65\x65",
	"\x49\x20\x6D\x61\x64\x65\x20\x69\x74\x2E",
	"\x49\x20\x66\x61\x6C\x6C\x2E",
	"\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67",
	"\x61\x20\x66\x72\x61\x6D\x65\x64\x20\x70\x61\x69\x6E\x74\x69\x6E\x67\x20\x6F\x66\x20\x42\x6C\x61\x63\x6B\x62\x65\x61\x72\x64\x2E",
	"\x6D\x61\x70\x20\x6F\x6E\x20\x62\x61\x63\x6B",
	"\x77\x72\x69\x74\x69\x6E\x67",
	"\x53\x61\x69\x6C\x69\x6E\x67\x20\x72\x6F\x75\x74\x65\x20\x74\x6F\x20\x54\x52\x45\x41\x53\x55\x52\x45\x21",
	"\x41\x72\x67\x68\x21",
	"\x49\x20\x64\x72\x6F\x77\x6E\x65\x64\x2E",
	"\x49\x20\x62\x72\x65\x61\x74\x68\x65\x64\x2E",
	"\x6C\x65\x6E\x73\x65\x73\x20\x26\x20\x77\x69\x72\x65\x20\x72\x69\x6D\x2E",
	"\x69\x74\x27\x73\x20\x22\x47\x6C\x6F\x69\x6F\x70\x65\x6C\x74\x69\x73\x20\x46\x75\x72\x63\x61\x74\x61\x22\x20\x6F\x72\x20\x46\x75\x6E\x6F\x72\x69\x2E",
	"\x68\x6F\x77\x20\x74\x6F\x20\x6D\x61\x6B\x65\x20\x67\x6C\x75\x65\x20\x66\x72\x6F\x6D\x20\x46\x75\x6E\x6F\x72\x69\x2E",
	"\x49\x20\x66\x6F\x75\x6E\x64",
	"\x26\x20\x49\x20\x67\x6F\x74\x20\x69\x74\x21",
	"\x69\x6E\x20\x74\x68\x65\x20\x77\x61\x79\x2E",
	"\x47\x6C\x61\x73\x73\x65\x73\x20\x73\x6C\x69\x70\x20\x6F\x66\x66\x2E",
	"\x49\x6E\x20\x32\x20\x77\x6F\x72\x64\x73\x20\x74\x65\x6C\x6C\x20\x6D\x65\x20\x74\x6F\x20\x6F\x72\x20\x69\x6E\x20\x77\x68\x61\x74\x2E",
	"\x49\x20\x63\x61\x6E\x27\x74\x2C",
	"\x48\x55\x48\x3F",
	"\x62\x6F\x61\x74\x27\x73\x20\x62\x6F\x74\x74\x6F\x6D\x2E",
	"\x68\x61\x70\x70\x65\x6E\x73\x2E",
	"\x6E\x6F\x74\x68\x69\x6E\x67",
	"\x73\x70\x65\x63\x69\x61\x6C\x2E",
	"\x49\x20\x68\x65\x61\x72",
	"\x61\x6E\x20\x61\x6C\x61\x72\x6D\x20\x63\x6C\x6F\x63\x6B\x20\x72\x69\x6E\x67\x69\x6E\x67\x20\x73\x6F\x6D\x65\x77\x68\x65\x72\x65\x2E",
	"\x69\x74\x27\x73\x20\x63\x6F\x76\x65\x72\x65\x64\x20\x69\x6E\x20\x61\x6C\x67\x61\x65",
	"\x4D\x6F\x76\x65\x20\x6E\x75\x6D\x62\x65\x72\x3A",
	"\x4C\x65\x61\x76\x65\x20\x2A\x54\x52\x45\x41\x53\x55\x52\x45\x53\x2A\x20\x68\x65\x72\x65\x2E\x20\x53\x43\x4F\x52\x45\x2E",
	"\x70\x69\x6C\x69\x6E\x67\x73\x2E",
	"\x4D\x79\x20\x6D\x61\x73\x6B\x27\x73\x20\x66\x6F\x67\x67\x65\x64\x20\x75\x70\x21",
	"\x49\x20\x73\x74\x69\x72\x72\x65\x64\x20\x69\x74\x20\x75\x70\x2E",
	"\x49\x20\x6F\x70\x65\x6E\x65\x64\x20\x75\x70\x20\x61",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x6B\x6E\x6F\x77\x20\x77\x68\x65\x72\x65\x20\x69\x74\x20\x69\x73\x2E",
	"\x49\x20\x79\x61\x77\x6E\x20\x26\x20\x73\x74\x72\x65\x74\x63\x68\x21",
	"\x4D\x61\x74\x74\x72\x65\x73\x73\x20\x69\x73\x20\x73\x6F\x66\x74\x2E",
	"\x48\x6F\x77\x3F",
	"\x69\x74\x20\x66\x6F\x6C\x6C\x6F\x77\x73\x20\x6D\x65\x21",
	"\x49\x20\x61\x6D\x2E",
	"\x67\x6C\x61\x73\x73\x65\x73",
	"\x53\x77\x69\x6D\x20\x64\x6F\x77\x6E\x3F",
	"\x4D\x79\x20\x6D\x61\x73\x6B\x27\x73\x20\x66\x75\x6C\x6C\x20\x6F\x66\x20\x77\x61\x74\x65\x72\x2E",
	"\x62\x79\x20\x65\x64\x67\x65\x73\x20\x6F\x74\x68\x65\x72\x20\x70\x69\x63\x74\x75\x72\x65\x20\x69\x73\x20\x76\x69\x73\x69\x62\x6C\x65\x20\x62\x65\x6E\x65\x61\x74\x68\x21\x20\x49\x20\x63\x6C\x65\x61\x6E\x65\x64\x20\x74\x6F\x70\x20\x6F\x6E\x65\x20\x6F\x66\x66\x21",
	"\x4E\x6F\x20\x6D\x61\x70\x21",
	"\x4E\x6F\x20\x63\x72\x65\x77\x20\x68\x65\x72\x65\x21",
	"\x50\x69\x72\x61\x74\x65\x20\x64\x72\x69\x6E\x6B\x73\x20\x72\x75\x6D\x20\x26\x20\x68\x65\x61\x64\x73\x20\x74\x6F\x20\x77\x6F\x72\x6B\x2E",
	"\x50\x69\x72\x61\x74\x65\x20\x6C\x6F\x6F\x6B\x73\x20\x61\x72\x6F\x75\x6E\x64\x20\x65\x78\x70\x65\x63\x74\x61\x6E\x74\x6C\x79\x20\x26\x20\x73\x61\x79\x73\x20\x22\x4D\x61\x74\x65\x79\x2C\x20\x79\x6F\x75\x20\x62\x65\x20\x66\x6F\x72\x67\x65\x74\x66\x75\x6C\x20\x66\x6F\x72\x20\x73\x75\x72\x65\x21\x22\x20\x26\x20\x74\x68\x65\x6E\x20\x68\x65\x20\x73\x75\x6C\x6B\x73\x20\x6F\x66\x66\x20\x61\x6E\x67\x72\x69\x6C\x79\x21",
	"\x50\x69\x72\x61\x74\x65\x20\x77\x6F\x6E\x27\x74\x20\x6C\x65\x74\x20\x6D\x65\x2E",
	"\x52\x6F\x77\x3F",
	"\x74\x6F\x6F\x20\x68\x65\x61\x76\x79\x2E",
	"\x54\x72\x79\x20\x49\x4E\x56\x45\x4E\x54\x4F\x52\x59\x20\x6F\x72\x20\x4C\x4F\x4F\x4B",
	"\x74\x68\x65\x20\x6C\x69\x74\x74\x6C\x65\x20\x73\x63\x72\x65\x77\x73\x21",
	"\x74\x68\x65\x72\x65\x27\x73\x20\x6E\x6F\x20\x6D\x61\x74\x74\x72\x65\x73\x73\x20\x68\x65\x72\x65\x2E",
	"\x77\x6F\x6E\x27\x74\x20\x66\x69\x74\x2E",
	"\x49\x27\x76\x65\x20\x73\x74\x6F\x72\x65\x64\x20\x30\x20\x74\x72\x65\x61\x73\x75\x72\x65\x73\x2E",
	"\x46\x6F\x72\x20\x6D\x79\x20\x4D\x6F\x6D\x21",
	"\x69\x74\x27\x73\x20\x6F\x6E\x20\x74\x68\x65\x20\x73\x68\x69\x70\x20\x73\x6F\x6D\x65\x77\x68\x65\x72\x65\x2E",
	"\x69\x74\x27\x73\x20\x6F\x6E\x20\x4F\x70\x68\x74\x68\x61\x6C\x6D\x6F\x6C\x6F\x67\x79\x2E\x20\x53\x71\x75\x69\x6E\x74\x69\x6E\x67\x20\x63\x61\x6E\x20\x68\x65\x6C\x70\x20\x77\x69\x74\x68\x20\x4D\x79\x6F\x70\x69\x61\x2E",
	"\x49\x74\x27\x73\x20\x6F\x64\x64\x21",
	"\x47\x6F\x6F\x64\x20\x69\x64\x65\x61\x21\x20\x42\x75\x74\x20\x2E\x2E\x2E",
	"\x57\x68\x79\x20\x62\x6F\x74\x68\x65\x72\x3F",
	"\x49\x20\x6C\x6F\x73\x74",
	"\x61\x20\x73\x68\x69\x70\x20\x77\x69\x74\x68\x20\x66\x6C\x6F\x6F\x64\x6C\x69\x67\x68\x74\x73\x20\x6C\x69\x74",
	"\x74\x68\x65\x20\x66\x61\x6E\x27\x73\x20\x74\x75\x72\x6E\x69\x6E\x67\x2E",
	"\x6C\x61\x72\x67\x65\x20\x62\x6C\x61\x64\x65\x20\x66\x61\x6E\x20\x68\x61\x73\x20\x73\x74\x6F\x70\x70\x65\x64\x2E",
	"\x62\x61\x74\x74\x65\x72\x79\x27\x73\x20\x64\x65\x61\x64\x21",
	"\x62\x72\x69\x67\x68\x74\x6C\x79\x2E",
	"\x64\x69\x6D\x6C\x79\x2E",
	"\x6E\x6F\x74\x20\x61\x74\x20\x61\x6C\x6C\x21",
	"\x43\x6C\x69\x63\x6B\x21",
	"\x61\x6D\x70\x73\x20\x6C\x65\x66\x74\x2E",
	"\x50\x69\x72\x61\x74\x65\x20\x73\x61\x79\x73\x3A",
	"\x69\x74\x27\x73\x20\x55\x72\x6F\x73\x61\x6C\x70\x69\x6E\x78\x20\x43\x69\x6E\x65\x72\x61\x2E",
	"\x4F\x79\x73\x74\x65\x72\x20\x44\x72\x69\x6C\x6C\x20\x64\x6F\x65\x73\x20\x74\x68\x65\x20\x74\x72\x69\x63\x6B\x21",
	"\x4C\x69\x64\x20\x62\x72\x6F\x6B\x65\x20\x6F\x66\x66\x21",
	"\x49\x27\x6D\x20\x6E\x6F\x20\x73\x61\x69\x6C\x6F\x72\x21",
	"\x66\x6C\x6F\x6F\x64\x6C\x69\x67\x68\x74\x73\x20\x6C\x69\x74",
	"\x4C\x6F\x6E\x67\x20\x74\x72\x69\x70\x2C\x20\x6E\x69\x67\x68\x74\x20\x66\x65\x6C\x6C\x21",
	"\x4F\x75\x74\x20\x6F\x66\x20\x67\x61\x73\x21",
	"\x26\x20\x6C\x6F\x73\x74\x20\x61\x74\x20\x73\x65\x61\x21",
	"\x4C\x69\x74\x65\x72\x73\x20\x6C\x65\x66\x74\x2E",
	"\x6C\x61\x72\x67\x65\x20\x6F\x70\x65\x6E\x69\x6E\x67\x2E",
};


const uint8_t status[] = {
	181, 
	8, 3, 8, 29, 9, 16, 0, 3, 0, 31, 
	60, 58, 
	177, 
	8, 31, 9, 29, 0, 3, 0, 31, 
	58, 60, 
	168, 
	8, 29, 16, 0, 
	77, 
	176, 
	8, 4, 1, 10, 0, 10, 0, 14, 
	62, 
	168, 
	9, 2, 9, 3, 
	57, 
	149, 8, 
	9, 21, 1, 18, 9, 22, 0, 22, 0, 18, 
	58, 60, 
	176, 
	1, 67, 9, 5, 0, 67, 0, 68, 
	72, 
	174, 
	0, 1, 0, 1, 0, 1, 
	81, 82, 81, 
	174, 
	9, 1, 0, 40, 0, 3, 
	79, 81, 73, 
	195, 
	56, 70, 64, 2, 
	207, 
	0, 1, 0, 2, 0, 18, 
	58, 58, 58, 113, 
	211, 
	0, 1, 0, 0, 0, 1, 0, 20, 
	81, 79, 81, 79, 
	172, 
	4, 13, 2, 42, 0, 25, 
	58, 
	172, 
	8, 5, 1, 12, 0, 24, 
	58, 
	172, 
	8, 5, 1, 14, 0, 24, 
	58, 
	172, 
	8, 5, 1, 51, 0, 24, 
	58, 
	180, 
	8, 24, 13, 51, 14, 50, 0, 50, 0, 51, 
	72, 
	181, 
	3, 17, 6, 18, 8, 4, 0, 22, 0, 23, 
	60, 58, 
	168, 
	1, 31, 0, 19, 
	58, 
	172, 
	4, 13, 5, 42, 0, 25, 
	60, 
	173, 
	9, 13, 13, 63, 0, 63, 
	59, 76, 
	174, 
	8, 6, 0, 2, 0, 1, 
	81, 83, 73, 
	196, 
	15, 3, 
	15, 
	210, 
	15, 0, 9, 4, 0, 6, 0, 17, 
	17, 60, 60, 
	196, 
	0, 2, 
	81, 
	183, 
	8, 4, 1, 2, 0, 2, 0, 2, 0, 18, 
	55, 58, 60, 73, 
	193, 
	70, 24, 
	183, 
	9, 4, 0, 10, 0, 11, 0, 12, 0, 13, 
	60, 60, 60, 60, 
	173, 
	9, 4, 0, 14, 0, 20, 
	60, 60, 
	176, 
	8, 5, 1, 42, 0, 42, 0, 54, 
	72, 
	181, 
	8, 13, 8, 4, 14, 32, 4, 12, 0, 63, 
	53, 76, 
	183, 
	8, 23, 1, 18, 0, 2, 0, 22, 0, 18, 
	49, 58, 60, 60, 
	174, 
	8, 22, 1, 18, 0, 2, 
	38, 58, 56, 
	176, 
	8, 5, 1, 56, 0, 62, 0, 56, 
	72, 
	136, 15, 
	8, 2, 4, 1, 
	42, 
	169, 
	8, 4, 9, 6, 
	16, 61, 
	182, 
	6, 1, 1, 2, 0, 2, 0, 2, 0, 18, 
	55, 58, 60, 
	182, 
	6, 17, 1, 18, 0, 18, 0, 2, 0, 18, 
	55, 58, 60, 
	175, 
	9, 18, 8, 3, 0, 18, 
	56, 58, 70, 64, 
	169, 
	9, 19, 9, 7, 
	32, 33, 
	165, 
	8, 2, 
	4, 5, 
	178, 
	8, 18, 9, 2, 0, 18, 9, 3, 
	57, 60, 76, 
	174, 
	9, 18, 8, 2, 0, 18, 
	56, 58, 76, 
	145, 25, 
	8, 4, 8, 20, 0, 20, 0, 13, 
	60, 58, 
	173, 
	8, 29, 15, 0, 0, 29, 
	60, 79, 
	168, 
	15, 6, 0, 27, 
	58, 
	172, 
	16, 6, 14, 46, 0, 27, 
	60, 
	164, 
	4, 24, 
	63, 
	175, 
	8, 28, 0, 20, 0, 3, 
	79, 81, 77, 73, 
	201, 
	15, 0, 0, 28, 
	60, 79, 
	196, 
	0, 3, 
	81, 
};
const uint8_t actions[] = {
	14, 27, 71, 
	4, 2, 8, 2, 3, 1, 
	1, 21, 47, 
	5, 27, 0, 
	8, 2, 
	3, 26, 
	11, 87, 64, 
	4, 18, 15, 0, 
	3, 26, 7, 123, 
	9, 87, 64, 
	4, 18, 0, 3, 
	73, 81, 
	199, 
	15, 0, 
	3, 30, 29, 136, 
	201, 
	16, 0, 0, 28, 
	1, 58, 
	196, 
	0, 3, 
	81, 
	10, 27, 100, 
	13, 56, 3, 64, 
	1, 7, 132, 
	10, 73, 11, 
	9, 28, 2, 52, 
	129, 44, 106, 
	15, 1, 43, 
	13, 48, 10, 0, 6, 2, 
	3, 26, 10, 111, 
	19, 27, 70, 
	8, 4, 8, 11, 14, 67, 0, 67, 
	7, 10, 22, 74, 
	5, 27, 15, 
	2, 52, 
	3, 105, 
	23, 27, 24, 
	9, 2, 13, 15, 3, 14, 0, 14, 0, 51, 
	1, 7, 50, 72, 
	4, 10, 5, 
	7, 1, 
	46, 
	14, 1, 43, 
	4, 19, 11, 0, 0, 2, 
	1, 54, 76, 
	14, 1, 43, 
	2, 48, 11, 0, 0, 19, 
	1, 54, 76, 
	13, 57, 43, 
	0, 48, 0, 45, 2, 45, 
	1, 72, 
	4, 52, 0, 
	8, 5, 
	48, 
	0, 92, 0, 
	44, 
	19, 24, 0, 
	8, 2, 4, 2, 2, 1, 0, 1, 
	21, 10, 22, 74, 
	11, 46, 61, 
	6, 2, 6, 18, 
	1, 21, 30, 23, 
	7, 27, 90, 
	9, 7, 
	1, 7, 78, 128, 
	21, 59, 34, 
	3, 6, 3, 44, 8, 30, 0, 44, 0, 71, 
	1, 72, 
	17, 77, 32, 
	3, 17, 6, 18, 8, 23, 0, 23, 
	1, 60, 
	10, 27, 3, 
	8, 4, 8, 10, 
	7, 37, 23, 
	4, 41, 0, 
	9, 2, 
	66, 
	4, 10, 51, 
	9, 2, 
	66, 
	10, 1, 64, 
	4, 18, 0, 17, 
	1, 54, 76, 
	15, 34, 32, 
	1, 18, 0, 18, 0, 2, 
	1, 59, 58, 56, 
	15, 34, 7, 
	1, 2, 0, 2, 0, 2, 
	1, 59, 58, 56, 
	0, 1, 57, 
	41, 
	4, 61, 7, 
	3, 2, 
	1, 
	6, 24, 0, 
	4, 16, 
	1, 7, 110, 
	1, 27, 52, 
	1, 45, 
	14, 1, 11, 
	2, 43, 0, 4, 0, 7, 
	54, 76, 60, 
	4, 71, 0, 
	8, 25, 
	65, 
	1, 72, 0, 
	3, 26, 
	0, 57, 61, 
	1, 
	19, 24, 62, 
	2, 35, 8, 14, 14, 11, 0, 11, 
	21, 10, 22, 74, 
	23, 43, 0, 
	3, 46, 4, 5, 14, 41, 13, 7, 0, 41, 
	21, 10, 22, 74, 
	10, 44, 15, 
	2, 7, 0, 7, 
	1, 59, 73, 
	196, 
	5, 41, 
	104, 
	210, 
	2, 41, 0, 41, 0, 52, 0, 47, 
	59, 72, 103, 
	15, 10, 30, 
	9, 6, 0, 6, 0, 2, 
	1, 58, 73, 81, 
	205, 
	8, 17, 0, 8, 0, 2, 
	79, 81, 
	205, 
	9, 17, 0, 3, 0, 2, 
	79, 81, 
	10, 45, 47, 
	0, 17, 0, 6, 
	1, 58, 60, 
	10, 47, 0, 
	0, 17, 0, 6, 
	1, 58, 60, 
	11, 45, 0, 
	0, 6, 0, 17, 
	1, 60, 60, 17, 
	2, 79, 0, 
	1, 30, 29, 
	23, 36, 7, 
	3, 1, 6, 18, 0, 1, 0, 2, 0, 2, 
	1, 74, 74, 60, 
	6, 86, 31, 
	3, 3, 
	1, 7, 6, 
	11, 39, 66, 
	4, 6, 0, 7, 
	1, 8, 54, 76, 
	11, 39, 5, 
	4, 6, 0, 7, 
	1, 8, 54, 76, 
	6, 39, 0, 
	4, 6, 
	1, 9, 61, 
	11, 39, 9, 
	4, 7, 0, 6, 
	1, 54, 9, 76, 
	6, 39, 0, 
	4, 7, 
	1, 9, 61, 
	2, 39, 0, 
	3, 30, 29, 
	19, 1, 13, 
	4, 7, 0, 8, 0, 3, 0, 16, 
	56, 54, 58, 58, 
	19, 1, 13, 
	4, 8, 0, 7, 0, 3, 0, 16, 
	54, 60, 60, 76, 
	9, 1, 22, 
	4, 8, 0, 9, 
	1, 54, 
	5, 75, 57, 
	12, 31, 
	3, 41, 
	19, 27, 62, 
	3, 18, 2, 35, 0, 22, 0, 2, 
	7, 58, 58, 39, 
	22, 61, 32, 
	8, 5, 3, 17, 6, 18, 0, 22, 0, 21, 
	1, 60, 60, 
	19, 27, 15, 
	3, 7, 14, 8, 14, 10, 0, 8, 
	7, 10, 22, 74, 
	15, 27, 15, 
	3, 7, 14, 55, 0, 55, 
	7, 10, 22, 74, 
	19, 57, 21, 
	3, 8, 3, 6, 0, 10, 14, 10, 
	1, 7, 10, 53, 
	22, 34, 17, 
	3, 12, 0, 12, 0, 13, 0, 14, 3, 49, 
	1, 72, 53, 
	23, 34, 103, 
	3, 1, 3, 49, 0, 1, 0, 20, 0, 19, 
	72, 74, 119, 109, 
	11, 27, 24, 
	3, 14, 0, 15, 
	7, 12, 22, 74, 
	6, 27, 76, 
	3, 15, 
	1, 7, 13, 
	22, 78, 32, 
	9, 4, 3, 17, 6, 18, 0, 21, 0, 22, 
	1, 58, 60, 
	4, 86, 76, 
	3, 15, 
	14, 
	5, 48, 44, 
	9, 0, 
	3, 26, 
	6, 27, 24, 
	3, 12, 
	1, 7, 11, 
	10, 1, 10, 
	4, 5, 0, 6, 
	1, 54, 76, 
	10, 1, 28, 
	4, 2, 0, 1, 
	1, 54, 76, 
	2, 27, 31, 
	1, 7, 13, 
	6, 27, 7, 
	3, 1, 
	1, 7, 18, 
	23, 34, 8, 
	3, 1, 3, 49, 0, 1, 0, 20, 0, 19, 
	72, 74, 119, 109, 
	14, 50, 36, 
	3, 21, 0, 21, 0, 22, 
	1, 72, 8, 
	15, 27, 50, 
	3, 21, 14, 23, 0, 23, 
	21, 10, 74, 22, 
	6, 27, 50, 
	3, 21, 
	1, 7, 19, 
	6, 86, 38, 
	1, 24, 
	1, 7, 20, 
	7, 36, 32, 
	1, 2, 
	3, 26, 10, 23, 
	15, 36, 32, 
	3, 17, 0, 17, 0, 18, 
	1, 74, 74, 73, 
	204, 
	8, 8, 0, 2, 9, 23, 
	60, 
	13, 32, 8, 
	3, 22, 3, 20, 0, 9, 
	25, 58, 
	21, 53, 32, 
	3, 22, 3, 20, 8, 9, 3, 17, 6, 18, 
	1, 73, 
	206, 
	0, 20, 0, 8, 0, 9, 
	55, 58, 60, 
	10, 53, 0, 
	8, 9, 0, 9, 
	3, 26, 60, 
	15, 27, 101, 
	9, 7, 0, 3, 0, 3, 
	81, 78, 81, 138, 
	10, 48, 6, 
	8, 5, 0, 4, 
	1, 73, 58, 
	210, 
	1, 46, 0, 46, 0, 11, 0, 27, 
	53, 119, 10, 
	211, 
	2, 29, 0, 12, 0, 12, 0, 29, 
	54, 58, 55, 76, 
	211, 
	2, 26, 0, 12, 0, 10, 0, 26, 
	54, 58, 55, 76, 
	210, 
	2, 28, 0, 12, 0, 13, 0, 28, 
	54, 58, 55, 
	211, 
	2, 36, 0, 12, 0, 14, 0, 36, 
	54, 58, 55, 76, 
	211, 
	2, 40, 0, 12, 0, 20, 0, 40, 
	54, 58, 55, 76, 
	211, 
	2, 27, 0, 12, 0, 11, 0, 27, 
	54, 58, 55, 76, 
	10, 27, 81, 
	2, 59, 9, 27, 
	1, 7, 121, 
	22, 34, 84, 
	14, 46, 8, 27, 2, 59, 3, 49, 0, 46, 
	1, 22, 74, 
	19, 1, 45, 
	2, 26, 0, 3, 0, 5, 0, 26, 
	1, 54, 60, 59, 
	23, 1, 11, 
	2, 29, 0, 29, 0, 13, 0, 5, 0, 7, 
	59, 54, 60, 60, 
	23, 1, 11, 
	2, 28, 0, 28, 0, 4, 0, 5, 0, 7, 
	59, 54, 60, 60, 
	23, 48, 5, 
	8, 4, 8, 20, 0, 4, 0, 11, 0, 40, 
	60, 54, 53, 76, 
	23, 48, 5, 
	8, 4, 8, 11, 0, 4, 0, 11, 0, 27, 
	60, 54, 53, 76, 
	23, 48, 5, 
	8, 4, 8, 10, 0, 4, 0, 11, 0, 26, 
	60, 54, 53, 76, 
	23, 48, 5, 
	8, 4, 8, 13, 0, 4, 0, 11, 0, 28, 
	60, 54, 53, 76, 
	11, 48, 5, 
	8, 4, 8, 12, 
	3, 26, 10, 23, 
	23, 48, 5, 
	8, 4, 8, 14, 0, 4, 0, 11, 0, 36, 
	60, 54, 53, 76, 
	23, 48, 44, 
	8, 4, 8, 12, 0, 4, 0, 11, 0, 29, 
	60, 54, 53, 76, 
	19, 1, 39, 
	4, 3, 0, 5, 0, 11, 0, 26, 
	58, 54, 53, 76, 
	23, 1, 39, 
	4, 13, 0, 5, 0, 11, 0, 29, 0, 7, 
	58, 54, 53, 58, 
	19, 48, 4, 
	8, 4, 8, 13, 0, 13, 0, 12, 
	1, 60, 58, 76, 
	19, 48, 3, 
	8, 4, 8, 10, 0, 10, 0, 11, 
	1, 60, 58, 76, 
	19, 48, 4, 
	8, 4, 8, 11, 0, 11, 0, 10, 
	1, 60, 58, 76, 
	19, 48, 3, 
	8, 4, 8, 12, 0, 12, 0, 13, 
	1, 60, 58, 76, 
	11, 48, 3, 
	8, 4, 8, 11, 
	3, 26, 10, 23, 
	18, 27, 43, 
	4, 2, 15, 6, 8, 29, 16, 0, 
	7, 134, 125, 
	14, 27, 43, 
	4, 2, 16, 6, 8, 29, 
	7, 134, 124, 
	10, 48, 0, 
	8, 4, 0, 20, 
	1, 58, 76, 
	23, 27, 60, 
	3, 34, 14, 21, 14, 22, 0, 21, 9, 8, 
	7, 34, 22, 74, 
	23, 1, 39, 
	4, 4, 0, 5, 0, 11, 0, 28, 0, 7, 
	58, 54, 53, 58, 
	19, 1, 39, 
	4, 15, 0, 5, 0, 11, 0, 36, 
	58, 54, 53, 76, 
	15, 64, 28, 
	14, 70, 0, 70, 2, 16, 
	21, 10, 22, 74, 
	10, 1, 63, 
	4, 2, 0, 16, 
	1, 54, 76, 
	10, 1, 66, 
	4, 2, 0, 16, 
	1, 54, 76, 
	15, 27, 65, 
	3, 37, 0, 1, 0, 1, 
	35, 81, 78, 81, 
	14, 27, 5, 
	8, 4, 8, 12, 9, 0, 
	1, 7, 28, 
	19, 1, 49, 
	2, 36, 0, 36, 0, 5, 0, 15, 
	59, 60, 54, 76, 
	4, 1, 52, 
	2, 39, 
	46, 
	6, 86, 31, 
	3, 42, 
	1, 7, 36, 
	13, 10, 60, 
	4, 15, 14, 34, 0, 34, 
	74, 1, 
	9, 10, 60, 
	3, 34, 0, 34, 
	74, 1, 
	15, 64, 83, 
	14, 24, 4, 1, 0, 24, 
	21, 10, 22, 74, 
	15, 27, 11, 
	8, 4, 8, 12, 8, 0, 
	7, 28, 7, 139, 
	3, 49, 0, 
	1, 85, 30, 29, 
	15, 27, 64, 
	4, 17, 14, 49, 0, 49, 
	21, 10, 22, 74, 
	15, 1, 45, 
	2, 32, 0, 3, 0, 7, 
	1, 54, 58, 76, 
	4, 48, 0, 
	8, 5, 
	1, 
	9, 16, 0, 
	9, 7, 9, 19, 
	32, 114, 
	4, 1, 52, 
	2, 38, 
	46, 
	5, 43, 0, 
	3, 46, 
	21, 30, 
	6, 73, 11, 
	6, 15, 
	3, 26, 51, 
	0, 9, 0, 
	63, 
	1, 48, 0, 
	30, 29, 
	10, 27, 3, 
	8, 4, 8, 11, 
	1, 7, 37, 
	10, 1, 11, 
	2, 63, 0, 23, 
	1, 54, 76, 
	1, 55, 53, 
	1, 71, 
	13, 27, 11, 
	8, 4, 8, 12, 9, 0, 
	1, 116, 
	19, 24, 62, 
	2, 35, 8, 12, 14, 44, 0, 44, 
	21, 10, 22, 74, 
	7, 73, 11, 
	5, 52, 
	3, 26, 102, 133, 
	1, 69, 0, 
	3, 26, 
	5, 24, 0, 
	4, 1, 
	1, 43, 
	1, 33, 0, 
	3, 41, 
	2, 27, 95, 
	1, 7, 130, 
	2, 16, 0, 
	32, 30, 31, 
	0, 1, 66, 
	44, 
	1, 46, 0, 
	30, 29, 
	9, 57, 93, 
	3, 44, 0, 30, 
	44, 58, 
	2, 10, 83, 
	3, 26, 107, 
	2, 27, 71, 
	3, 26, 108, 
	14, 1, 16, 
	2, 53, 11, 0, 0, 20, 
	1, 54, 76, 
	11, 1, 16, 
	2, 53, 10, 0, 
	3, 26, 10, 111, 
	5, 71, 0, 
	9, 25, 
	1, 112, 
	0, 44, 5, 
	46, 
	10, 10, 5, 
	4, 1, 0, 2, 
	1, 54, 76, 
	14, 86, 38, 
	3, 56, 9, 3, 9, 2, 
	1, 7, 115, 
	23, 1, 39, 
	4, 19, 0, 5, 0, 11, 0, 28, 0, 7, 
	58, 54, 53, 58, 
	23, 80, 0, 
	8, 2, 9, 3, 9, 4, 6, 18, 0, 2, 
	60, 57, 76, 73, 
	203, 
	0, 18, 0, 2, 
	60, 88, 58, 70, 
	15, 57, 100, 
	14, 56, 3, 64, 0, 56, 
	7, 10, 74, 73, 
	194, 
	22, 7, 132, 
	1, 18, 36, 
	3, 26, 
	6, 8, 7, 
	3, 1, 
	117, 3, 26, 
	5, 80, 0, 
	8, 4, 
	30, 29, 
	5, 1, 10, 
	4, 7, 
	3, 46, 
	0, 1, 9, 
	44, 
	9, 7, 56, 
	3, 41, 0, 41, 
	1, 59, 
	11, 81, 24, 
	3, 51, 0, 26, 
	1, 117, 25, 58, 
	22, 53, 79, 
	3, 51, 3, 57, 8, 26, 0, 58, 0, 51, 
	1, 72, 73, 
	201, 
	0, 57, 0, 26, 
	59, 60, 
	18, 83, 24, 
	3, 58, 0, 51, 0, 58, 0, 57, 
	1, 72, 74, 
	1, 81, 0, 
	117, 118, 
	11, 1, 81, 
	2, 59, 9, 27, 
	3, 26, 117, 121, 
	10, 27, 81, 
	2, 59, 8, 27, 
	7, 122, 73, 
	203, 
	14, 60, 0, 60, 
	21, 10, 22, 74, 
	23, 73, 11, 
	4, 2, 13, 32, 0, 32, 0, 4, 0, 3, 
	56, 72, 58, 73, 
	203, 
	0, 3, 0, 15, 
	81, 83, 1, 135, 
	199, 
	15, 0, 
	3, 136, 137, 61, 
	199, 
	0, 3, 
	81, 88, 88, 88, 
	22, 34, 84, 
	14, 46, 8, 27, 2, 59, 3, 49, 0, 46, 
	1, 22, 74, 
	15, 27, 5, 
	8, 4, 8, 12, 8, 0, 
	7, 28, 7, 139, 
	5, 1, 69, 
	4, 2, 
	3, 133, 
	13, 75, 64, 
	4, 18, 8, 28, 0, 28, 
	1, 60, 
	6, 27, 11, 
	8, 7, 
	7, 120, 73, 
	204, 
	15, 6, 16, 0, 8, 29, 
	125, 
	200, 
	16, 6, 8, 29, 
	124, 
	196, 
	9, 29, 
	126, 
	15, 27, 86, 
	4, 2, 14, 61, 0, 61, 
	53, 7, 10, 76, 
	14, 64, 85, 
	2, 61, 8, 29, 0, 29, 
	1, 60, 127, 
	14, 64, 85, 
	2, 61, 9, 29, 0, 29, 
	1, 58, 127, 
	19, 89, 0, 
	0, 4, 0, 1, 0, 4, 0, 1, 
	83, 81, 82, 81, 
	15, 24, 11, 
	8, 4, 8, 12, 9, 0, 
	7, 40, 67, 139, 
	1, 64, 0, 
	30, 29, 
	11, 53, 0, 
	0, 9, 0, 26, 
	60, 60, 3, 26, 
	23, 73, 11, 
	4, 2, 14, 32, 0, 32, 0, 4, 0, 3, 
	73, 72, 60, 57, 
	207, 
	0, 3, 0, 15, 0, 31, 
	81, 83, 1, 60, 
	199, 
	15, 0, 
	3, 136, 137, 61, 
	196, 
	0, 3, 
	81, 
	23, 59, 95, 
	8, 30, 3, 44, 3, 67, 14, 66, 0, 66, 
	1, 74, 131, 22, 
	15, 27, 45, 
	4, 22, 14, 69, 0, 69, 
	7, 10, 22, 74, 
	15, 27, 5, 
	4, 22, 14, 69, 0, 69, 
	7, 10, 22, 74, 
	19, 24, 62, 
	2, 35, 0, 22, 0, 2, 0, 18, 
	39, 58, 58, 60, 
	10, 1, 45, 
	2, 27, 0, 22, 
	1, 54, 76, 
	0, 75, 0, 
	44, 
	1, 24, 0, 
	30, 29, 
	0, 57, 0, 
	44, 
	0, 80, 0, 
	1, 
	3, 27, 0, 
	7, 30, 31, 76, 
	1, 59, 0, 
	3, 26, 
	15, 1, 43, 
	1, 2, 0, 1, 0, 2, 
	24, 53, 73, 59, 
	197, 
	0, 2, 
	58, 56, 
	199, 
	10, 0, 
	3, 26, 10, 111, 
	215, 
	11, 0, 4, 19, 0, 2, 0, 48, 0, 45, 
	1, 54, 72, 76, 
	206, 
	11, 0, 2, 48, 0, 19, 
	1, 54, 76, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
215, 65, 76, 75,
198, 79, 76, 76,
197, 78, 84, 69,
195, 76, 73, 77,
195, 82, 65, 87,
68, 82, 73, 78,
70, 79, 76, 68,
81, 85, 73, 84,
71, 69, 84, 32,
212, 65, 75, 69,
199, 82, 65, 66,
208, 73, 67, 75,
200, 79, 76, 68,
211, 84, 65, 78,
76, 73, 83, 84,
200, 69, 65, 82,
68, 82, 79, 80,
204, 69, 65, 86,
208, 85, 84, 32,
210, 69, 76, 69,
199, 73, 86, 69,
212, 72, 82, 79,
70, 69, 69, 76,
212, 79, 85, 67,
208, 82, 79, 66,
76, 79, 79, 75,
197, 88, 65, 77,
204, 32, 32, 32,
196, 69, 83, 67,
46, 32, 32, 32,
71, 76, 85, 69,
70, 73, 78, 68,
82, 69, 77, 79,
213, 78, 83, 67,
87, 69, 65, 82,
196, 79, 78, 32,
208, 85, 84, 79,
74, 85, 77, 80,
204, 69, 65, 80,
73, 78, 86, 69,
201, 32, 32, 32,
68, 73, 71, 32,
87, 65, 75, 69,
66, 82, 69, 65,
82, 85, 66, 32,
72, 89, 80, 69,
83, 87, 73, 77,
83, 65, 89, 32,
77, 65, 75, 69,
205, 73, 88, 32,
68, 73, 86, 69,
84, 79, 32, 32,
201, 78, 32, 32,
83, 65, 86, 69,
87, 65, 75, 69,
79, 80, 69, 78,
213, 78, 76, 79,
85, 83, 69, 32,
215, 73, 84, 72,
67, 76, 69, 65,
215, 65, 83, 72,
210, 73, 78, 83,
77, 79, 86, 69,
208, 85, 83, 72,
208, 85, 76, 76,
204, 73, 70, 84,
208, 82, 69, 83,
83, 76, 69, 69,
197, 65, 84, 32,
83, 67, 79, 82,
72, 69, 76, 80,
83, 65, 73, 76,
196, 82, 73, 86,
83, 84, 79, 80,
211, 72, 85, 84,
69, 77, 80, 84,
83, 80, 73, 84,
89, 65, 87, 78,
83, 81, 85, 73,
82, 79, 76, 76,
215, 82, 65, 80,
85, 78, 82, 79,
213, 78, 87, 82,
32, 32, 32, 32,
82, 69, 65, 68,
83, 84, 65, 82,
195, 82, 65, 78,
87, 65, 73, 84,
208, 65, 85, 83,
32, 32, 32, 32,
67, 72, 65, 82,
210, 69, 67, 72,
203, 73, 76, 76,
204, 73, 71, 72,
194, 85, 82, 78,
212, 69, 65, 82,
210, 73, 80, 32,
211, 84, 65, 66,
211, 72, 79, 82,
198, 73, 88, 32,
210, 69, 80, 65,
46, 32, 32, 32,
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
71, 76, 65, 83,
76, 69, 78, 83,
76, 69, 68, 71,
72, 73, 76, 76,
83, 72, 73, 80,
194, 79, 65, 84,
67, 82, 65, 67,
195, 82, 69, 86,
80, 73, 82, 65,
67, 82, 65, 87,
70, 82, 65, 77,
65, 82, 79, 85,
68, 79, 85, 66,
83, 84, 65, 77,
66, 79, 88, 32,
83, 72, 69, 68,
212, 79, 79, 76,
80, 65, 73, 78,
208, 73, 67, 84,
210, 69, 77, 66,
194, 76, 65, 67,
66, 85, 78, 75,
194, 69, 68, 32,
66, 82, 69, 65,
83, 73, 71, 78,
77, 65, 83, 75,
198, 65, 67, 69,
72, 65, 77, 77,
68, 69, 67, 75,
71, 76, 85, 69,
65, 77, 66, 69,
66, 79, 79, 75,
83, 69, 65, 32,
207, 67, 69, 65,
215, 65, 84, 69,
208, 79, 79, 76,
80, 79, 82, 84,
79, 80, 69, 78,
68, 79, 67, 75,
194, 69, 65, 77,
68, 69, 69, 80,
67, 65, 66, 73,
66, 69, 65, 67,
65, 76, 71, 65,
73, 78, 86, 69,
67, 85, 82, 82,
71, 65, 77, 69,
68, 79, 79, 82,
83, 72, 79, 86,
82, 85, 77, 32,
67, 76, 79, 67,
193, 76, 65, 82,
84, 79, 82, 67,
82, 79, 67, 75,
69, 89, 69, 83,
83, 73, 76, 84,
76, 65, 68, 68,
69, 78, 71, 73,
87, 65, 84, 67,
84, 79, 80, 32,
212, 79, 80, 66,
211, 85, 77, 77,
72, 69, 76, 77,
80, 73, 76, 73,
73, 84, 32, 32,
207, 66, 74, 69,
200, 65, 78, 68,
89, 79, 72, 79,
83, 67, 82, 69,
77, 65, 80, 32,
66, 65, 67, 75,
69, 65, 82, 82,
79, 73, 76, 83,
210, 65, 73, 78,
70, 65, 78, 32,
66, 82, 79, 79,
77, 65, 84, 84,
66, 76, 65, 68,
66, 85, 84, 84,
67, 69, 73, 76,
195, 73, 69, 76,
76, 73, 71, 72,
198, 76, 79, 79,
66, 65, 84, 84,
68, 79, 76, 76,
73, 83, 76, 69,
79, 89, 83, 84,
80, 69, 65, 82,
83, 78, 65, 73,
70, 76, 79, 84,
202, 69, 84, 83,
80, 73, 78, 32,
82, 73, 78, 71,
67, 72, 69, 83,
71, 65, 83, 32,
198, 85, 69, 76,
87, 73, 82, 69,
210, 73, 77, 32,
	0,
};
const uint8_t automap[] = {
71, 76, 65, 83,
	1,
83, 73, 71, 78,
	3,
72, 65, 77, 77,
	6,
66, 79, 88, 32,
	8,
83, 84, 65, 77,
	10,
68, 79, 85, 66,
	11,
80, 65, 73, 78,
	12,
70, 82, 65, 77,
	13,
80, 65, 73, 78,
	14,
77, 65, 80, 32,
	15,
77, 65, 83, 75,
	17,
87, 73, 82, 69,
	19,
76, 69, 78, 83,
	20,
65, 76, 71, 65,
	21,
71, 76, 85, 69,
	22,
65, 77, 66, 69,
	23,
66, 79, 79, 75,
	24,
67, 76, 79, 67,
	31,
82, 79, 67, 75,
	34,
87, 65, 84, 67,
	37,
82, 85, 77, 32,
	41,
83, 73, 71, 78,
	42,
79, 89, 83, 84,
	44,
66, 76, 65, 68,
	46,
83, 67, 82, 69,
	49,
80, 65, 73, 78,
	50,
80, 65, 73, 78,
	51,
83, 73, 71, 78,
	54,
69, 65, 82, 82,
	55,
66, 79, 79, 75,
	56,
79, 73, 76, 83,
	57,
80, 65, 73, 78,
	58,
66, 82, 79, 79,
	60,
66, 79, 79, 75,
	62,
67, 72, 69, 83,
	64,
68, 79, 76, 76,
	65,
80, 69, 65, 82,
	66,
83, 78, 65, 73,
	67,
83, 78, 65, 73,
	68,
80, 73, 78, 32,
	69,
82, 73, 78, 71,
	70,
79, 89, 83, 84,
	71,
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
