#define NUM_OBJ 56
#define WORDSIZE 4
#define GAME_MAGIC 119
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
const uint8_t startlamp = 255;
const uint8_t lightfill = 255;
const uint8_t startcarried = 0;
const uint8_t maxcar = 6;
const uint8_t treasure = 22;
const uint8_t treasures = 5;
const uint8_t lastloc = 35;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"\x2E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6F\x6E\x65\x20\x6D\x61\x6E\x20\x73\x63\x6F\x75\x74\x73\x68\x69\x70",
 { 0, 0, 0, 0, 0, 7 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x61\x69\x72\x6C\x6F\x63\x6B",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x63\x61\x76\x65\x72\x6E",
 { 0, 0, 0, 0, 8, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x70\x6C\x61\x6E\x65\x74\x6F\x69\x64",
 { 5, 8, 5, 8, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x70\x6C\x61\x6E\x65\x74\x6F\x69\x64",
 { 8, 5, 8, 4, 0, 0 } }, 
		{ 	"\x73\x74\x72\x61\x6E\x67\x65\x20\x68\x65\x78\x61\x67\x6F\x6E\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x74\x6F\x72\x61\x67\x65\x20\x68\x6F\x6C\x64",
 { 0, 0, 0, 0, 1, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x70\x6C\x61\x6E\x65\x74\x6F\x69\x64",
 { 4, 5, 5, 4, 0, 0 } }, 
		{ 	"\x4C\x61\x72\x67\x65\x20\x67\x72\x61\x73\x73\x79\x20\x70\x6C\x61\x69\x6E\x20\x61\x74\x20\x65\x64\x67\x65\x20\x6F\x66\x20\x61\x20\x6A\x75\x6E\x67\x6C\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x68\x61\x6E\x65\x20\x73\x6E\x6F\x77\x20\x73\x74\x6F\x72\x6D",
 { 19, 19, 19, 20, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x64\x65\x72\x65\x6C\x69\x63\x74\x20\x73\x70\x61\x63\x65\x63\x72\x61\x66\x74",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x74\x68\x65\x20\x61\x69\x72\x6C\x6F\x63\x6B\x20\x6F\x6E\x20\x61\x20\x6C\x65\x64\x67\x65\x2E\x0A\x54\x68\x65\x20\x67\x72\x6F\x75\x6E\x64\x20\x69\x73\x20\x39\x30\x20\x6D\x65\x74\x65\x72\x73\x20\x62\x65\x6C\x6F\x77",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x41\x6C\x69\x65\x6E\x20\x41\x72\x74\x20\x4D\x75\x73\x65\x75\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x64\x65\x73\x65\x72\x74\x65\x64\x20\x4A\x6F\x76\x69\x61\x6E\x20\x6D\x69\x6E\x69\x6E\x67\x20\x63\x6F\x6C\x6F\x6E\x79",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x53\x74\x72\x61\x6E\x67\x65\x20\x6A\x75\x6E\x67\x6C\x65",
 { 15, 15, 21, 9, 0, 0 } }, 
		{ 	"\x2E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x42\x4C\x41\x43\x4B\x20\x45\x4D\x50\x54\x49\x4E\x45\x53\x53",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x68\x61\x6E\x65\x20\x73\x6E\x6F\x77\x20\x73\x74\x6F\x72\x6D",
 { 19, 20, 10, 20, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x68\x61\x6E\x65\x20\x73\x6E\x6F\x77\x20\x73\x74\x6F\x72\x6D",
 { 10, 18, 20, 18, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x68\x61\x6E\x65\x20\x73\x6E\x6F\x77\x20\x73\x74\x6F\x72\x6D",
 { 20, 20, 20, 20, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x72\x75\x69\x6E\x73\x20\x6F\x66\x20\x61\x6E\x20\x69\x6E\x74\x65\x72\x67\x61\x6C\x61\x74\x69\x63\x20\x5A\x4F\x4F",
 { 0, 15, 0, 0, 0, 0 } }, 
		{ 	"\x73\x74\x6F\x72\x61\x67\x65\x20\x68\x6F\x6C\x64\x20\x6F\x66\x20\x74\x68\x65\x20\x6D\x6F\x74\x68\x65\x72\x20\x73\x68\x69\x70",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x61\x69\x6E\x74\x65\x6E\x61\x6E\x63\x65\x20\x63\x72\x61\x77\x6C\x20\x77\x61\x79",
 { 0, 0, 0, 0, 7, 0 } }, 
		{ 	"\x68\x6F\x6C\x6C\x6F\x77\x20\x69\x63\x65\x20\x6D\x6F\x75\x6E\x64",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x6F\x74\x20\x6F\x66\x20\x54\x52\x4F\x55\x42\x4C\x45\x21",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	0,
	0,
	7,
	13,
	1,
	7,
	0,
	17,
	0,
	7,
	0,
	1,
	0,
	6,
	8,
	0,
	2,
	2,
	0,
	0,
	2,
	11,
	0,
	3,
	6,
	0,
	6,
	0,
	18,
	21,
	0,
	11,
	22,
	14,
	23,
	23,
	0,
	0,
	0,
	0,
	14,
	0,
	24,
	0,
	0,
	0,
	13,
	0,
	15,
	13,
	6,
	0,
	7,
	0,
	0,
};


const char *objtext[] = {
	"\x2E",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x66\x6C\x69\x63\x6B\x65\x72\x69\x6E\x67\x20\x63\x75\x72\x74\x61\x69\x6E\x20\x6F\x66\x20\x6C\x69\x67\x68\x74",
	"\x53\x63\x6F\x75\x74\x73\x68\x69\x70",
	"\x4D\x61\x69\x6E\x74\x65\x6E\x61\x6E\x63\x65\x20\x61\x63\x63\x65\x73\x73\x20\x68\x61\x74\x63\x68",
	"\x41\x6C\x69\x65\x6E\x20\x73\x69\x67\x6E",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x63\x6F\x6E\x73\x6F\x6C\x65",
	"\x53\x70\x61\x63\x65\x20\x73\x75\x69\x74",
	"\x77\x68\x69\x63\x68\x20\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67",
	"\x42\x6C\x61\x63\x6B\x20\x48\x6F\x6C\x65",
	"\x2E",
	"\x50\x68\x61\x73\x65\x72",
	"\x50\x68\x61\x73\x65\x72",
	"\x43\x6C\x6F\x73\x65\x64\x20\x64\x6F\x6F\x72",
	"\x4F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x66\x6C\x69\x63\x6B\x65\x72\x69\x6E\x67\x20\x63\x75\x72\x74\x61\x69\x6E\x20\x6F\x66\x20\x6C\x69\x67\x68\x74",
	"\x43\x61\x76\x65",
	"\x45\x6D\x70\x74\x79\x20\x66\x6C\x61\x73\x6B",
	"\x52\x65\x64\x20\x62\x75\x74\x74\x6F\x6E\x20\x62\x79\x20\x64\x6F\x6F\x72",
	"\x43\x6C\x6F\x73\x65\x64\x20\x6F\x75\x74\x65\x72\x20\x64\x6F\x6F\x72",
	"\x4F\x50\x65\x6E\x20\x6F\x75\x74\x65\x72\x20\x64\x6F\x6F\x72",
	"\x43\x6C\x6F\x73\x65\x64\x20\x69\x6E\x6E\x65\x72\x20\x64\x6F\x6F\x72",
	"\x4F\x70\x65\x6E\x20\x69\x6E\x6E\x65\x72\x20\x64\x6F\x6F\x72",
	"\x41\x6C\x69\x65\x6E\x20\x6D\x61\x63\x68\x69\x6E\x65",
	"\x48\x6F\x73\x65\x20\x63\x6F\x6E\x6E\x65\x63\x74\x73\x20\x6D\x79\x20\x73\x75\x69\x74\x20\x74\x6F\x20\x6D\x61\x63\x68\x69\x6E\x65",
	"\x4C\x61\x72\x67\x65\x20\x62\x6F\x75\x6C\x64\x65\x72",
	"\x53\x6D\x61\x6C\x6C\x20\x70\x69\x65\x63\x65\x20\x6F\x66\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x66\x6C\x75\x73\x68\x20\x69\x6E\x20\x74\x68\x65\x20\x77\x61\x6C\x6C",
	"\x2E",
	"\x52\x6F\x64\x20\x6A\x75\x74\x74\x69\x6E\x67\x20\x73\x74\x72\x61\x69\x67\x68\x74\x20\x6F\x75\x74\x20\x6F\x66\x20\x74\x68\x65\x20\x77\x61\x6C\x6C",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x70\x69\x65\x63\x65\x20\x6F\x66\x20\x72\x6F\x64",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x6C\x69\x67\x68\x74\x20\x66\x61\x72\x20\x74\x6F\x20\x74\x68\x65\x20\x4E\x4F\x52\x54\x48",
	"\x52\x69\x67\x69\x6C\x69\x61\x6E\x20\x44\x69\x61\x2D\x49\x63\x65\x20\x48\x6F\x75\x6E\x64",
	"\x53\x74\x75\x6E\x6E\x65\x64\x20\x44\x69\x61\x2D\x49\x63\x65\x20\x48\x6F\x75\x6E\x64",
	"\x56\x69\x65\x77\x70\x6F\x72\x74",
	"\x53\x69\x67\x6E\x3A\x20\x22\x4C\x65\x61\x76\x65\x20\x54\x72\x65\x61\x73\x75\x72\x65\x73\x20\x68\x65\x72\x65\x20\x73\x61\x79\x3A\x20\x53\x43\x4F\x52\x45\x22",
	"\x2A\x20\x41\x4E\x43\x49\x45\x4E\x54\x20\x46\x4C\x41\x53\x4B\x20\x53\x41\x55\x52\x49\x41\x4E\x20\x42\x52\x41\x4E\x44\x59\x20\x2A",
	"\x45\x6D\x70\x74\x79\x20\x63\x72\x79\x73\x74\x61\x6C\x20\x68\x6F\x6C\x64\x65\x72",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x70\x69\x65\x63\x65\x73\x20\x6F\x66\x20\x50\x6F\x77\x65\x72\x20\x43\x72\x79\x73\x74\x61\x6C",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x72\x6F\x64\x20\x69\x6E\x20\x70\x6F\x77\x65\x72\x20\x68\x6F\x6C\x64\x65\x72",
	"\x4C\x61\x72\x67\x65\x20\x69\x63\x65\x20\x6D\x6F\x75\x6E\x64",
	"\x41\x6E\x63\x69\x65\x6E\x74\x20\x69\x63\x65\x20\x70\x69\x63\x6B",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x69\x63\x65\x20\x70\x69\x63\x6B",
	"\x53\x68\x6F\x72\x74\x20\x74\x77\x69\x73\x74\x65\x64\x20\x70\x69\x65\x63\x65\x20\x6F\x66\x20\x6D\x65\x74\x61\x6C",
	"\x45\x6E\x74\x72\x61\x6E\x63\x65\x20\x74\x6F\x20\x61\x20\x63\x72\x61\x77\x6C\x77\x61\x79",
	"\x2A\x20\x52\x49\x47\x49\x4C\x49\x41\x4E\x20\x49\x43\x45\x20\x44\x49\x41\x4D\x4F\x4E\x44\x20\x2A",
	"\x2A\x20\x53\x54\x52\x41\x4E\x47\x45\x20\x41\x4C\x49\x45\x4E\x20\x42\x45\x4C\x54\x20\x2A",
	"\x77\x68\x69\x63\x68\x20\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67",
	"\x26\x20\x69\x74\x73\x20\x61\x63\x74\x69\x76\x61\x74\x65\x64",
	"\x2A\x20\x52\x41\x52\x45\x20\x41\x4C\x49\x45\x4E\x20\x50\x41\x49\x4E\x54\x49\x4E\x47\x20\x2A",
	"\x52\x6F\x63\x6B\x20\x64\x75\x73\x74",
	"\x43\x65\x6E\x74\x75\x72\x69\x6F\x6E\x20\x53\x6C\x69\x6D\x65\x20\x54\x72\x65\x65\x73",
	"\x2A\x20\x41\x4C\x49\x45\x4E\x20\x53\x43\x55\x4C\x50\x54\x55\x52\x45\x20\x2A",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x67\x6F\x67\x67\x6C\x65\x73",
	"\x77\x68\x69\x63\x68\x20\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67",
	"\x53\x68\x6F\x76\x65\x6C",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x73\x70\x6C\x69\x6E\x74\x65\x72\x65\x64\x20\x73\x68\x6F\x76\x65\x6C",
	"\x45\x56\x45\x52\x59\x54\x48\x49\x4E\x47\x20\x48\x41\x53\x20\x41\x20\x42\x4C\x55\x45\x49\x53\x48\x20\x54\x49\x4E\x54",
};
const char *msgptr[] = {
	"",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x3A\x20\x36\x20\x22\x53\x54\x52\x41\x4E\x47\x45\x20\x4F\x44\x59\x53\x53\x45\x59\x22\x20\x62\x79\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x2E\x0A\x44\x65\x64\x69\x63\x61\x74\x65\x64\x20\x74\x6F\x20\x74\x68\x65\x20\x4E\x6F\x76\x61\x6B\x73\x2E\x0A",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x73",
	"\x4F\x4B",
	"\x50\x72\x65\x73\x73\x75\x72\x65\x20\x67\x61\x75\x67\x65\x20\x73\x61\x79\x73",
	"\x6D\x6F\x76\x65\x73\x20\x74\x69\x6C\x6C\x20\x73\x75\x69\x74\x20\x61\x69\x72\x20\x65\x78\x70\x65\x6E\x64\x65\x64\x2E\x20\x41\x69\x72\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x73\x75\x69\x74\x20\x69\x73",
	"\x54\x68\x65\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x68\x61\x73\x20\x61\x20\x70\x72\x65\x73\x73\x75\x72\x65\x20\x67\x61\x75\x67\x65\x20\x26\x20\x61\x20\x63\x6F\x6E\x6E\x65\x63\x74\x65\x72\x20\x66\x6F\x72\x20\x61\x20\x68\x6F\x73\x65\x2E",
	"\x62\x72\x65\x61\x74\x68\x61\x62\x6C\x65\x2E",
	"\x6E\x6F\x74",
	"\x53\x74\x61\x6C\x65\x20\x61\x69\x72\x20\x69\x73",
	"\x49\x74\x20\x69\x73",
	"\x4D\x79\x20\x61\x69\x72\x20\x72\x61\x6E\x20\x6F\x75\x74\x21",
	"\x53\x6F\x6D\x65\x20\x47\x41\x53\x20\x63\x6F\x6D\x65\x73\x20\x6F\x75\x74\x20\x6F\x66\x20\x74\x68\x65\x20\x68\x6F\x73\x65\x20\x66\x6F\x72\x20\x61\x77\x68\x69\x6C\x65\x20\x74\x68\x65\x6E\x20\x73\x74\x6F\x70\x73\x2E",
	"\x0A\x50\x68\x61\x73\x65\x72\x20\x69\x73\x20\x73\x65\x74\x20\x6F\x6E\x3A",
	"\x44\x45\x53\x54\x52\x4F\x59\x21",
	"\x73\x74\x75\x6E\x2E",
	"\x22\x54\x4F\x20\x53\x54\x55\x4E\x22\x20\x6F\x72\x20\x22\x54\x4F\x20\x44\x45\x53\x54\x52\x4F\x59\x22\x20\x3F",
	"\x28\x75\x73\x65\x20\x32\x20\x77\x6F\x72\x64\x73\x29",
	"\x0A\x4D\x79\x20\x73\x75\x69\x74\x20\x70\x6F\x70\x70\x65\x64\x20\x6F\x70\x65\x6E\x21",
	"\x57\x61\x72\x6E\x69\x6E\x67\x20\x6C\x69\x67\x68\x74\x20\x73\x61\x79\x73\x3A\x20\x22\x50\x4F\x57\x45\x52\x20\x43\x52\x59\x53\x54\x41\x4C\x20\x44\x41\x4D\x41\x47\x45\x44\x22",
	"\x42\x6C\x75\x65\x20\x62\x75\x74\x74\x6F\x6E\x20\x6D\x61\x72\x6B\x65\x64\x20\x22\x42\x4C\x41\x53\x54\x20\x4F\x46\x46\x22\x20\x26\x20\x61\x20\x75\x6E\x6D\x61\x72\x6B\x65\x64\x20\x72\x65\x64\x20\x62\x75\x74\x74\x6F\x6E",
	"\x43\x6F\x6E\x73\x6F\x6C\x65\x20\x72\x65\x70\x6C\x69\x65\x73\x3A\x20\x22\x49\x4E\x53\x55\x46\x46\x49\x43\x49\x45\x4E\x54\x20\x50\x4F\x57\x45\x52\x22",
	"\x41\x6C\x69\x65\x6E\x20\x73\x63\x72\x69\x70\x74\x20\x63\x6F\x76\x65\x72\x73\x20\x69\x74",
	"\x49\x20\x66\x69\x72\x65\x20\x74\x68\x65\x20\x50\x68\x61\x73\x65\x72\x2C",
	"\x49\x20\x64\x65\x73\x74\x72\x6F\x79\x65\x64\x20\x69\x74\x21",
	"\x41\x69\x72\x20\x69\x73\x20\x67\x65\x74\x74\x69\x6E\x67\x20\x73\x74\x61\x6C\x65\x21\x20\x52\x65\x61\x64\x20\x67\x61\x75\x67\x65\x21",
	"\x77\x6F\x6E\x27\x74\x20\x62\x75\x64\x67\x65\x21",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C\x2E",
	"\x49\x74\x73\x20\x73\x6F\x6D\x65\x20\x73\x6F\x72\x74\x20\x6F\x66\x20\x66\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64\x2E",
	"\x49\x20\x72\x75\x69\x6E\x65\x64\x20\x74\x68\x65\x20\x73\x68\x69\x70\x21",
	"\x45\x69\x74\x68\x65\x72\x20\x49\x20\x6D\x69\x73\x73\x65\x64\x20\x6F\x72\x20\x62\x65\x61\x6D\x20\x77\x61\x73\x20\x61\x62\x73\x6F\x72\x62\x65\x64\x20\x77\x69\x74\x68\x6F\x75\x74\x20\x76\x69\x73\x69\x62\x6C\x65\x20\x65\x66\x66\x65\x63\x74\x21",
	"\x49\x74\x73\x20\x65\x6D\x70\x74\x79\x2E",
	"\x43\x68\x61\x72\x67\x65\x20\x6C\x65\x66\x74\x20\x72\x65\x67\x69\x73\x74\x65\x72\x73\x3A",
	"\x49\x74\x20\x73\x65\x65\x6D\x73\x20\x74\x6F\x20\x62\x65\x20\x61\x73\x20\x66\x61\x72",
	"\x6F\x75\x74",
	"\x69\x6E",
	"\x61\x73\x20\x69\x74\x20\x77\x69\x6C\x6C\x20\x67\x6F\x2E",
	"\x4F\x64\x64\x20\x69\x74\x20\x6F\x6E\x6C\x79\x20\x72\x65\x71\x75\x69\x72\x65\x64\x20\x76\x65\x72\x79\x20\x6C\x69\x74\x74\x6C\x65\x20\x66\x6F\x72\x63\x65",
	"\x66\x6F\x72\x20\x69\x74\x20\x74\x6F\x20\x62\x72\x65\x61\x6B\x20\x6F\x66\x66\x20\x69\x6E\x20\x6D\x79\x20\x68\x61\x6E\x64\x20\x77\x69\x74\x68\x20\x61\x0A\x43\x52\x59\x53\x54\x41\x4C\x4C\x49\x4E\x45\x20\x73\x6E\x61\x70\x21",
	"\x48\x4F\x57\x3F",
	"\x54\x68\x65\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x47\x4C\x4F\x57\x45\x44\x20\x62\x72\x69\x65\x66\x6C\x79",
	"\x74\x69\x6D\x65\x73\x2E",
	"\x66\x6F\x72\x20\x69\x74\x20\x74\x6F\x20\x73\x68\x61\x74\x74\x65\x72\x21",
	"\x49\x20\x66\x65\x65\x6C\x20\x73\x74\x72\x61\x6E\x67\x65\x6C\x79\x20\x64\x69\x73\x6F\x72\x69\x65\x6E\x74\x65\x64\x20\x66\x6F\x72\x20\x61\x20\x6D\x6F\x6D\x65\x6E\x74\x21",
	"\x74\x6F\x20\x73\x6C\x69\x64\x65",
	"\x49\x63\x65\x20\x48\x6F\x75\x6E\x64\x20\x61\x74\x74\x61\x63\x6B\x73\x20\x6D\x65\x20\x75\x6E\x65\x78\x70\x65\x63\x74\x69\x64\x6C\x79\x21",
	"\x49\x74\x20\x77\x6F\x6B\x65\x20\x75\x70\x21",
	"\x49\x20\x66\x6F\x75\x6E\x64\x20\x6E\x6F\x74\x68\x69\x6E\x67",
	"\x57\x65\x27\x72\x65\x20\x64\x72\x69\x66\x74\x69\x6E\x67\x20\x69\x6E\x20\x73\x70\x61\x63\x65\x21",
	"\x49\x20\x73\x65\x65\x3A\x20\x61\x20\x62\x6C\x61\x63\x6B\x20\x62\x75\x74\x74\x6F\x6E\x2C\x20\x61\x20\x77\x68\x69\x74\x65\x20\x62\x75\x74\x74\x6F\x6E\x2C\x20\x61\x20\x68\x6F\x73\x65\x2E",
	"\x48\x6F\x73\x65\x20\x72\x69\x70\x73\x20\x6F\x75\x74",
	"\x54\x69\x67\x68\x74\x20\x66\x69\x74\x21",
	"\x4C\x69\x66\x74\x20\x6F\x66\x66\x21\x21\x20\x41\x66\x74\x65\x72\x20\x61\x20\x62\x72\x69\x65\x66\x20\x66\x6C\x69\x67\x68\x74\x20\x77\x65\x20\x61\x72\x72\x69\x76\x65\x2E\x2E\x2E",
	"\x54\x68\x65\x20\x49\x63\x65\x20\x48\x6F\x75\x6E\x64\x20\x62\x75\x72\x72\x6F\x77\x73\x20\x6F\x66\x66\x2E\x2E\x2E",
	"\x49\x63\x65\x20\x66\x69\x6C\x6C\x73\x20\x62\x61\x63\x6B\x20\x69\x6E\x20\x61\x72\x6F\x75\x6E\x64\x20\x6D\x65\x20\x61\x73\x20\x49\x20\x64\x69\x67\x20\x6D\x79\x20\x77\x61\x79\x20\x69\x6E\x2E\x20\x54\x68\x65\x0A\x70\x69\x63\x6B\x20\x62\x72\x6F\x6B\x65\x20\x62\x75\x74\x20\x49\x20\x6D\x61\x64\x65\x20\x69\x74\x21",
	"\x49\x63\x65\x20\x69\x73\x20\x76\x65\x72\x79\x20\x73\x6D\x6F\x6F\x74\x68\x20\x26\x20\x68\x61\x72\x64\x21",
	"\x49\x20\x73\x6C\x69\x64\x65\x20\x62\x61\x63\x6B\x20\x64\x6F\x77\x6E\x2E",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x6B\x6E\x6F\x77\x20\x77\x68\x65\x72\x65\x20\x74\x6F\x20\x6C\x6F\x6F\x6B",
	"\x54\x72\x79\x3A\x20\x22\x53\x48\x4F\x4F\x54\x20\x53\x4F\x4D\x45\x54\x48\x49\x4E\x47\x22",
	"\x53\x61\x79\x20\x61\x67\x61\x69\x6E\x20\x26\x20\x75\x73\x65\x20\x61\x20\x63\x6F\x6C\x6F\x72",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x77\x72\x69\x74\x69\x6E\x67\x20\x74\x68\x65\x72\x65",
	"\x49\x20\x63\x61\x6E\x27\x74\x20\x64\x6F\x20\x74\x68\x61\x74",
	"\x57\x68\x6F\x6F\x6F\x73\x68\x21",
	"\x54\x68\x65\x20\x67\x72\x61\x76\x69\x74\x79\x20\x68\x65\x72\x65\x20\x69\x73\x20\x76\x65\x72\x79",
	"\x73\x74\x72\x6F\x6E\x67\x2E",
	"\x77\x65\x61\x6B\x2E",
	"\x49\x74\x73\x20\x73\x74\x75\x63\x6B\x2E",
	"\x49\x27\x6D\x20\x53\x51\x55\x41\x53\x48\x45\x44\x21",
	"\x54\x68\x65\x20\x70\x6F\x77\x65\x72\x20\x63\x72\x79\x73\x74\x61\x6C\x20\x77\x61\x73\x20\x69\x6E\x20\x74\x68\x65\x20\x73\x68\x61\x70\x65\x20\x6F\x66\x20\x61\x20\x74\x68\x69\x6E\x20\x72\x6F\x64\x2E",
	"\x49\x20\x66\x65\x65\x6C\x20\x4C\x49\x47\x48\x54\x20\x48\x45\x41\x44\x45\x44\x21",
	"\x49\x74\x20\x66\x6C\x6F\x61\x74\x73\x21",
	"\x41\x20\x67\x65\x6E\x74\x6C\x65\x20\x74\x6F\x75\x63\x68\x20\x77\x69\x6C\x6C\x20\x77\x6F\x72\x6B\x20\x77\x6F\x6E\x64\x65\x72\x73\x21",
	"\x49\x74\x20\x73\x68\x6F\x75\x6C\x64\x20\x66\x69\x74\x2E\x20\x49\x74\x20\x68\x61\x73\x20\x61\x20\x6C\x61\x72\x67\x65\x20\x62\x75\x63\x6B\x6C\x65\x2E",
	"\x50\x61\x69\x6E\x74\x69\x6E\x67\x20\x69\x73\x20\x6F\x66\x20\x61\x6E\x20\x41\x6C\x69\x65\x6E\x20\x74\x77\x69\x73\x74\x69\x6E\x67\x20\x61\x20\x62\x75\x63\x6B\x6C\x65\x20\x6F\x6E\x20\x61\x20\x62\x65\x6C\x74\x2E",
	"\x57\x69\x72\x69\x6E\x67\x20\x62\x65\x68\x69\x6E\x64\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x73\x68\x6F\x72\x74\x73\x20\x26\x20\x49\x27\x6D\x20\x65\x6C\x65\x63\x74\x72\x6F\x63\x75\x74\x65\x64\x21",
	"\x49\x20\x73\x65\x65\x20\x61\x20\x70\x69\x63\x74\x75\x72\x65\x20\x6F\x66\x20\x61\x6E\x20\x41\x4C\x49\x45\x4E\x20\x54\x45\x4E\x54\x41\x43\x4C\x45\x2E",
	"\x57\x6F\x6E\x64\x65\x72\x66\x75\x6C\x2E\x20\x49\x74\x20\x41\x54\x45\x20\x6D\x65\x21",
	"\x49\x74\x20\x69\x73\x20\x6F\x66\x20\x61\x6E\x20\x61\x6C\x69\x65\x6E\x20\x62\x65\x69\x6E\x67\x2E",
	"\x49\x74\x73\x20\x77\x65\x61\x72\x69\x6E\x67\x20\x6F\x6E\x6C\x79\x20\x61\x20\x73\x74\x72\x61\x6E\x67\x65\x20\x62\x65\x6C\x74\x21",
	"\x57\x61\x74\x63\x68\x20\x69\x74\x21\x20\x49\x74\x73\x20\x6B\x6E\x6F\x77\x6E\x20\x74\x6F\x20\x73\x70\x69\x74\x20\x6D\x6F\x6C\x74\x65\x6E\x20\x44\x49\x41\x4D\x4F\x4E\x44\x53\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x62\x6F\x78\x20\x6F\x6E\x20\x69\x74\x20\x77\x69\x74\x68\x20\x61\x20\x79\x65\x6C\x6C\x6F\x77\x20\x62\x75\x74\x74\x6F\x6E\x2E",
	"\x49\x20\x6A\x75\x73\x74\x20\x73\x65\x65\x20\x73\x68\x61\x64\x65\x73\x20\x6F\x66\x20\x62\x6C\x61\x63\x6B\x2C\x20\x69\x74\x20\x6D\x61\x6B\x65\x73\x20\x6D\x79\x20\x65\x79\x65\x73\x20\x73\x77\x69\x6D\x21",
	"\x49\x20\x74\x68\x69\x6E\x6B\x20\x69\x74\x73\x20\x62\x72\x6F\x6B\x65\x6E\x21",
	"\x49\x20\x66\x6F\x75\x6E\x64\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x2E",
	"\x4F\x75\x63\x68\x21",
	"\x69\x74\x73\x20\x73\x74\x75\x6E\x6E\x65\x64\x21",
	"\x54\x68\x65\x20\x6D\x65\x74\x61\x6C\x20\x68\x65\x6C\x70\x65\x64\x21",
	"\x47\x65\x74\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x37\x20\x22\x4D\x59\x53\x54\x45\x52\x59\x20\x46\x55\x4E\x20\x48\x4F\x55\x53\x45\x22\x20\x66\x72\x6F\x6D\x20\x79\x6F\x75\x72\x20\x66\x61\x76\x6F\x72\x69\x74\x65\x20\x44\x65\x61\x6C\x65\x72\x21\x0A\x48\x61\x76\x65\x20\x79\x6F\x75\x20\x67\x6F\x74\x74\x65\x6E\x20\x79\x6F\x75\x72\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x54\x2D\x53\x48\x49\x52\x54\x20\x79\x65\x74\x3F\x0A",
	"\x54\x69\x64\x61\x6C\x20\x66\x6F\x72\x63\x65\x20\x72\x69\x70\x73\x20\x6D\x65\x20\x61\x70\x61\x72\x74\x21",
	"\x49\x27\x6D\x20\x6E\x6F\x74\x20\x77\x65\x61\x72\x69\x6E\x67\x20\x69\x74\x21",
	"\x4D\x79\x20\x68\x61\x6E\x64\x20\x77\x65\x6E\x74\x20\x54\x48\x52\x4F\x55\x47\x48\x20\x69\x74\x21",
	"\x44\x6F\x6E\x27\x74\x20\x68\x61\x76\x65\x20\x69\x74\x21",
	"",
	"",
	"",
};


const uint8_t status[] = {
	171, 
	4, 14, 6, 45, 
	113, 114, 117, 63, 
	168, 
	2, 18, 0, 2, 
	60, 
	171, 
	4, 14, 6, 46, 
	113, 114, 117, 63, 
	179, 
	9, 1, 0, 1, 0, 5, 0, 6, 
	58, 73, 58, 58, 
	207, 
	0, 2, 0, 28, 0, 2, 
	81, 79, 81, 1, 
	211, 
	0, 1, 0, 110, 0, 1, 0, 6, 
	81, 79, 81, 54, 
	207, 
	0, 3, 0, 2, 0, 3, 
	81, 79, 81, 80, 
	211, 
	0, 1, 0, 5, 0, 20, 0, 5, 
	54, 81, 79, 81, 
	170, 
	3, 7, 0, 1, 
	81, 77, 73, 
	196, 
	15, 27, 
	25, 
	197, 
	15, 0, 
	11, 61, 
	196, 
	0, 1, 
	81, 
	176, 
	1, 44, 13, 46, 6, 46, 0, 46, 
	52, 
	176, 
	2, 44, 13, 46, 5, 46, 0, 46, 
	53, 
	133, 30, 
	1, 30, 
	45, 61, 
	133, 15, 
	2, 30, 
	45, 61, 
	170, 
	13, 31, 0, 4, 
	81, 77, 73, 
	204, 
	15, 0, 0, 31, 0, 30, 
	72, 
	196, 
	3, 30, 
	46, 
	196, 
	0, 4, 
	81, 
	174, 
	8, 4, 0, 4, 0, 2, 
	73, 60, 81, 
	205, 
	9, 3, 0, 4, 0, 2, 
	83, 81, 
	205, 
	8, 3, 0, 7, 0, 2, 
	83, 81, 
	171, 
	8, 2, 6, 7, 
	10, 8, 7, 63, 
	165, 
	0, 2, 
	81, 73, 
	208, 
	15, 0, 14, 11, 0, 11, 0, 10, 
	72, 
	196, 
	0, 2, 
	81, 
	183, 
	2, 35, 2, 28, 0, 28, 0, 37, 0, 35, 
	72, 59, 51, 76, 
	164, 
	2, 30, 
	73, 
	210, 
	4, 18, 0, 38, 0, 20, 0, 30, 
	62, 59, 103, 
	210, 
	4, 19, 0, 38, 0, 20, 0, 30, 
	62, 59, 103, 
	210, 
	4, 20, 0, 38, 0, 20, 0, 30, 
	62, 59, 103, 
	210, 
	4, 10, 0, 38, 0, 20, 0, 30, 
	62, 59, 103, 
	204, 
	14, 30, 0, 30, 0, 24, 
	62, 
	164, 
	4, 35, 
	63, 
	170, 
	8, 9, 0, 5, 
	81, 77, 73, 
	205, 
	15, 1, 0, 9, 0, 10, 
	60, 58, 
	206, 
	1, 52, 1, 51, 0, 55, 
	57, 53, 76, 
	196, 
	0, 5, 
	81, 
	173, 
	1, 52, 9, 9, 9, 15, 
	56, 76, 
};
const uint8_t actions[] = {
	4, 37, 74, 
	3, 6, 
	110, 
	23, 46, 0, 
	3, 6, 2, 9, 0, 1, 0, 75, 0, 1, 
	3, 81, 79, 81, 
	5, 55, 22, 
	3, 10, 
	16, 17, 
	9, 9, 73, 
	3, 10, 0, 3, 
	60, 3, 
	9, 9, 72, 
	3, 10, 0, 3, 
	58, 3, 
	5, 71, 0, 
	4, 24, 
	111, 105, 
	13, 32, 8, 
	3, 31, 0, 31, 0, 30, 
	3, 72, 
	13, 25, 27, 
	4, 1, 0, 12, 0, 13, 
	72, 3, 
	23, 25, 27, 
	2, 17, 0, 18, 0, 19, 0, 20, 0, 21, 
	3, 72, 72, 73, 
	205, 
	9, 7, 2, 19, 0, 2, 
	58, 112, 
	204, 
	9, 7, 2, 18, 0, 2, 
	60, 
	9, 25, 37, 
	3, 22, 5, 23, 
	3, 12, 
	14, 25, 79, 
	3, 22, 0, 8, 9, 8, 
	3, 2, 58, 
	15, 25, 37, 
	3, 22, 3, 23, 9, 8, 
	3, 12, 10, 73, 
	194, 
	8, 7, 61, 
	23, 25, 37, 
	3, 22, 3, 23, 0, 1, 0, 25, 8, 8, 
	81, 3, 73, 82, 
	199, 
	16, 110, 
	12, 18, 11, 61, 
	197, 
	0, 1, 
	12, 81, 
	13, 10, 26, 
	2, 22, 0, 23, 14, 23, 
	3, 53, 
	9, 18, 26, 
	2, 23, 0, 23, 
	3, 59, 
	14, 1, 31, 
	2, 19, 9, 7, 0, 12, 
	3, 54, 76, 
	10, 1, 31, 
	2, 21, 0, 1, 
	3, 54, 76, 
	10, 1, 31, 
	2, 13, 0, 2, 
	3, 54, 76, 
	4, 37, 30, 
	2, 5, 
	20, 
	11, 25, 28, 
	2, 5, 14, 37, 
	3, 21, 19, 2, 
	19, 51, 71, 
	1, 10, 8, 3, 2, 24, 0, 4, 
	23, 73, 58, 88, 
	207, 
	0, 24, 0, 1, 0, 48, 
	72, 24, 53, 76, 
	14, 25, 79, 
	3, 22, 8, 8, 0, 8, 
	3, 2, 60, 
	14, 58, 20, 
	3, 6, 0, 7, 0, 6, 
	74, 74, 3, 
	14, 57, 5, 
	3, 6, 0, 7, 0, 6, 
	74, 74, 3, 
	14, 18, 20, 
	3, 6, 0, 6, 0, 7, 
	53, 59, 3, 
	4, 37, 32, 
	2, 1, 
	28, 
	15, 51, 32, 
	1, 10, 0, 4, 2, 1, 
	28, 23, 30, 58, 
	23, 51, 34, 
	1, 10, 8, 3, 2, 2, 0, 4, 0, 2, 
	23, 58, 24, 59, 
	10, 1, 40, 
	2, 15, 0, 3, 
	3, 54, 76, 
	19, 51, 0, 
	1, 10, 8, 3, 4, 1, 0, 35, 
	23, 54, 29, 63, 
	19, 51, 0, 
	1, 10, 8, 3, 4, 7, 0, 35, 
	23, 54, 29, 63, 
	19, 51, 0, 
	1, 10, 8, 3, 4, 2, 0, 35, 
	23, 54, 29, 63, 
	15, 37, 22, 
	3, 10, 0, 2, 9, 15, 
	73, 81, 32, 78, 
	202, 
	8, 3, 0, 2, 
	13, 14, 81, 
	202, 
	9, 3, 0, 2, 
	13, 15, 81, 
	9, 37, 22, 
	3, 11, 9, 15, 
	32, 31, 
	5, 1, 60, 
	2, 8, 
	138, 61, 
	11, 17, 39, 
	2, 27, 9, 5, 
	2, 33, 34, 36, 
	11, 25, 39, 
	2, 27, 8, 5, 
	2, 33, 35, 36, 
	15, 62, 39, 
	2, 27, 0, 27, 0, 28, 
	37, 38, 59, 74, 
	4, 10, 39, 
	2, 27, 
	39, 
	9, 10, 39, 
	3, 28, 0, 28, 
	3, 52, 
	23, 51, 39, 
	1, 10, 8, 3, 2, 27, 0, 27, 0, 4, 
	23, 24, 59, 58, 
	23, 51, 41, 
	1, 10, 8, 3, 2, 25, 0, 25, 0, 4, 
	23, 24, 59, 58, 
	17, 51, 41, 
	1, 10, 9, 3, 2, 25, 0, 4, 
	40, 58, 
	7, 25, 41, 
	2, 25, 
	37, 42, 124, 61, 
	23, 51, 41, 
	1, 10, 8, 3, 3, 29, 0, 29, 0, 4, 
	23, 24, 59, 58, 
	15, 33, 41, 
	2, 25, 9, 5, 0, 3, 
	40, 73, 81, 79, 
	196, 
	0, 3, 
	81, 
	15, 25, 39, 
	2, 27, 9, 5, 0, 3, 
	73, 37, 44, 81, 
	202, 
	0, 1, 0, 5, 
	35, 82, 58, 
	199, 
	0, 3, 
	40, 78, 41, 81, 
	15, 17, 39, 
	2, 27, 8, 5, 0, 5, 
	37, 44, 34, 60, 
	5, 24, 78, 
	3, 4, 
	3, 137, 
	15, 33, 41, 
	2, 25, 8, 5, 0, 3, 
	73, 81, 80, 3, 
	194, 
	43, 88, 88, 
	205, 
	19, 1, 0, 3, 0, 6, 
	54, 58, 
	205, 
	19, 2, 0, 9, 0, 6, 
	54, 60, 
	205, 
	19, 3, 0, 10, 0, 6, 
	54, 58, 
	205, 
	19, 4, 0, 11, 0, 6, 
	54, 58, 
	205, 
	19, 5, 0, 17, 0, 6, 
	54, 58, 
	204, 
	19, 6, 0, 13, 0, 6, 
	54, 
	204, 
	19, 7, 0, 14, 0, 6, 
	54, 
	205, 
	16, 7, 0, 17, 0, 6, 
	54, 58, 
	198, 
	0, 3, 
	80, 76, 81, 
	0, 51, 22, 
	108, 
	5, 33, 32, 
	2, 1, 
	3, 140, 
	5, 33, 32, 
	2, 14, 
	3, 140, 
	22, 71, 0, 
	14, 40, 3, 53, 4, 9, 14, 39, 0, 39, 
	53, 3, 133, 
	19, 1, 32, 
	4, 6, 0, 1, 8, 6, 0, 2, 
	80, 53, 76, 58, 
	15, 1, 32, 
	2, 1, 14, 23, 0, 2, 
	80, 76, 60, 3, 
	19, 1, 32, 
	4, 6, 0, 1, 9, 6, 0, 2, 
	80, 53, 76, 60, 
	23, 51, 8, 
	1, 10, 3, 30, 8, 3, 0, 4, 0, 30, 
	58, 23, 24, 59, 
	19, 51, 8, 
	1, 10, 0, 4, 3, 30, 9, 3, 
	58, 23, 73, 135, 
	215, 
	0, 30, 0, 31, 0, 4, 0, 17, 0, 4, 
	72, 81, 79, 81, 
	10, 1, 32, 
	2, 1, 13, 23, 
	50, 18, 61, 
	10, 69, 0, 
	2, 2, 0, 2, 
	3, 54, 76, 
	9, 37, 10, 
	2, 32, 9, 15, 
	3, 48, 
	0, 60, 0, 
	65, 
	9, 37, 24, 
	3, 22, 9, 15, 
	3, 49, 
	17, 37, 59, 
	3, 47, 8, 9, 1, 52, 9, 15, 
	3, 123, 
	17, 25, 28, 
	2, 5, 13, 37, 9, 7, 0, 7, 
	102, 58, 
	17, 25, 28, 
	2, 5, 13, 37, 8, 7, 0, 7, 
	102, 60, 
	19, 1, 31, 
	2, 19, 8, 7, 0, 22, 0, 2, 
	3, 54, 53, 76, 
	10, 1, 34, 
	2, 2, 0, 2, 
	3, 54, 76, 
	23, 51, 44, 
	1, 10, 8, 3, 2, 38, 0, 38, 0, 4, 
	23, 24, 59, 58, 
	23, 71, 0, 
	2, 38, 0, 24, 1, 39, 0, 39, 0, 40, 
	104, 54, 72, 76, 
	23, 51, 44, 
	4, 24, 8, 3, 1, 10, 0, 18, 0, 4, 
	54, 58, 24, 103, 
	10, 1, 44, 
	2, 38, 6, 39, 
	39, 105, 106, 
	9, 71, 0, 
	2, 38, 6, 39, 
	39, 105, 
	4, 74, 44, 
	2, 38, 
	39, 
	4, 68, 31, 
	4, 1, 
	39, 
	7, 69, 0, 
	4, 12, 
	3, 113, 115, 73, 
	194, 
	88, 88, 88, 
	202, 
	0, 4, 0, 2, 
	54, 53, 76, 
	0, 69, 0, 
	3, 
	10, 1, 9, 
	4, 9, 0, 15, 
	3, 54, 76, 
	13, 10, 48, 
	2, 3, 12, 41, 14, 42, 
	111, 116, 
	23, 10, 48, 
	2, 3, 3, 41, 0, 3, 0, 42, 14, 42, 
	52, 53, 3, 136, 
	9, 1, 55, 
	2, 42, 0, 23, 
	54, 76, 
	14, 45, 14, 
	3, 34, 0, 34, 0, 16, 
	72, 3, 112, 
	13, 10, 48, 
	3, 3, 0, 3, 13, 42, 
	52, 3, 
	15, 58, 58, 
	3, 44, 0, 45, 0, 44, 
	74, 3, 73, 74, 
	201, 
	13, 46, 0, 46, 
	74, 119, 
	15, 18, 58, 
	3, 44, 0, 45, 0, 44, 
	59, 3, 73, 53, 
	201, 
	13, 46, 0, 46, 
	53, 120, 
	18, 62, 57, 
	3, 44, 14, 46, 0, 46, 0, 44, 
	75, 3, 120, 
	13, 62, 57, 
	3, 44, 13, 46, 0, 46, 
	3, 59, 
	8, 37, 58, 
	3, 44, 9, 15, 
	122, 
	9, 37, 59, 
	3, 47, 9, 15, 
	3, 131, 
	0, 15, 24, 
	39, 
	5, 1, 71, 
	2, 24, 
	47, 106, 
	5, 37, 71, 
	2, 24, 
	3, 22, 
	9, 68, 20, 
	1, 7, 0, 7, 
	3, 59, 
	9, 65, 20, 
	1, 6, 0, 7, 
	3, 74, 
	4, 1, 63, 
	4, 12, 
	39, 
	8, 37, 12, 
	3, 36, 9, 15, 
	118, 
	4, 8, 0, 
	4, 6, 
	121, 
	13, 68, 48, 
	2, 3, 12, 41, 14, 42, 
	111, 116, 
	19, 68, 48, 
	2, 3, 3, 41, 0, 3, 0, 42, 
	52, 53, 3, 136, 
	10, 1, 34, 
	4, 12, 0, 2, 
	3, 54, 76, 
	4, 10, 41, 
	2, 25, 
	39, 
	8, 37, 41, 
	2, 25, 9, 15, 
	125, 
	1, 33, 0, 
	3, 2, 
	13, 30, 0, 
	2, 22, 14, 23, 0, 23, 
	53, 3, 
	5, 1, 67, 
	2, 49, 
	126, 61, 
	10, 37, 69, 
	3, 50, 9, 15, 
	73, 3, 127, 
	196, 
	14, 44, 
	128, 
	13, 10, 58, 
	3, 50, 14, 44, 0, 44, 
	3, 52, 
	9, 10, 58, 
	3, 44, 0, 44, 
	3, 52, 
	8, 37, 8, 
	3, 30, 9, 15, 
	129, 
	22, 51, 58, 
	3, 50, 1, 10, 8, 3, 0, 50, 0, 4, 
	59, 58, 73, 
	198, 
	0, 44, 
	59, 23, 24, 
	15, 58, 75, 
	3, 51, 0, 51, 0, 52, 
	74, 74, 3, 73, 
	8, 37, 75, 
	3, 51, 9, 15, 
	130, 
	15, 18, 75, 
	1, 51, 0, 51, 0, 52, 
	53, 59, 57, 73, 
	198, 
	0, 55, 
	3, 59, 76, 
	17, 25, 77, 
	3, 51, 9, 10, 9, 9, 0, 9, 
	3, 58, 
	13, 37, 59, 
	3, 47, 6, 52, 9, 15, 
	3, 131, 
	17, 25, 77, 
	3, 51, 9, 10, 8, 9, 0, 9, 
	3, 60, 
	10, 25, 77, 
	3, 51, 8, 10, 
	3, 2, 132, 
	4, 37, 20, 
	3, 6, 
	6, 
	11, 24, 74, 
	3, 6, 0, 1, 
	4, 81, 78, 73, 
	192, 
	5, 
	196, 
	8, 2, 
	8, 
	197, 
	0, 1, 
	7, 81, 
	19, 71, 0, 
	2, 38, 3, 53, 0, 53, 0, 54, 
	3, 37, 42, 72, 
	1, 41, 0, 
	3, 85, 
	0, 25, 29, 
	109, 
	4, 15, 22, 
	3, 10, 
	108, 
	7, 51, 0, 
	3, 11, 
	3, 23, 2, 31, 
	11, 51, 0, 
	1, 10, 0, 4, 
	58, 23, 2, 30, 
	0, 54, 0, 
	26, 
	1, 37, 0, 
	27, 76, 
	0, 61, 0, 
	66, 
	4, 71, 0, 
	3, 53, 
	47, 
	1, 53, 0, 
	47, 107, 
	0, 24, 0, 
	111, 
	1, 8, 0, 
	3, 39, 
	0, 7, 0, 
	63, 
	0, 6, 51, 
	71, 
	1, 29, 0, 
	134, 2, 
	0, 65, 0, 
	39, 
	15, 1, 39, 
	2, 27, 0, 27, 0, 28, 
	37, 38, 59, 74, 
	14, 77, 75, 
	1, 52, 0, 52, 0, 55, 
	3, 59, 59, 
	9, 48, 20, 
	1, 7, 0, 7, 
	3, 59, 
	15, 48, 75, 
	1, 52, 0, 52, 0, 55, 
	57, 59, 59, 76, 
	1, 48, 0, 
	111, 139, 
	5, 62, 57, 
	14, 44, 
	111, 141, 
	13, 79, 14, 
	3, 34, 0, 34, 0, 16, 
	72, 3, 
	0, 10, 7, 
	66, 
	10, 1, 48, 
	2, 42, 0, 23, 
	2, 54, 76, 
	4, 75, 12, 
	4, 0, 
	51, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
215, 65, 76, 75,
210, 85, 78, 32,
197, 78, 84, 69,
195, 76, 73, 77,
83, 65, 86, 69,
81, 85, 73, 84,
72, 69, 76, 80,
84, 79, 32, 32,
71, 69, 84, 32,
212, 65, 75, 69,
199, 82, 65, 66,
174, 32, 32, 32,
213, 78, 83, 84,
70, 73, 82, 69,
193, 67, 84, 73,
80, 85, 76, 76,
68, 82, 79, 80,
208, 85, 84, 32,
196, 73, 83, 67,
204, 69, 65, 86,
212, 72, 82, 79,
46, 32, 32, 32,
82, 69, 65, 68,
80, 85, 83, 72,
208, 82, 69, 83,
194, 65, 78, 71,
200, 73, 84, 32,
75, 73, 67, 75,
67, 79, 78, 78,
200, 79, 79, 75,
87, 65, 75, 69,
84, 79, 85, 67,
210, 85, 66, 32,
198, 69, 69, 76,
208, 69, 84, 32,
76, 79, 79, 75,
196, 69, 83, 67,
197, 88, 65, 77,
211, 69, 69, 32,
83, 65, 89, 32,
217, 69, 76, 76,
200, 79, 76, 76,
211, 67, 82, 69,
68, 82, 73, 78,
67, 72, 65, 82,
210, 69, 67, 72,
82, 69, 77, 79,
213, 78, 87, 69,
65, 84, 32, 32,
83, 72, 79, 79,
194, 76, 65, 83,
70, 73, 78, 68,
77, 79, 86, 69,
83, 69, 84, 32,
210, 69, 83, 69,
83, 85, 73, 84,
87, 69, 65, 82,
198, 65, 83, 84,
83, 67, 79, 82,
73, 78, 86, 69,
66, 69, 78, 68,
212, 87, 73, 83,
194, 82, 69, 65,
67, 76, 79, 83,
211, 72, 85, 84,
211, 76, 65, 77,
79, 80, 69, 78,
74, 85, 77, 80,
198, 65, 76, 76,
68, 73, 71, 32,
208, 73, 67, 75,
195, 82, 65, 32,
77, 69, 76, 84,
70, 73, 88, 32,
210, 69, 80, 65,
76, 73, 70, 84,
210, 65, 73, 83,
69, 77, 80, 84,
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
72, 79, 85, 78,
74, 85, 78, 71,
86, 73, 69, 87,
72, 69, 76, 76,
67, 82, 89, 83,
72, 69, 76, 80,
70, 76, 65, 83,
194, 82, 65, 78,
79, 85, 84, 32,
73, 78, 32, 32,
90, 79, 79, 32,
83, 72, 79, 86,
83, 85, 73, 84,
200, 69, 76, 77,
80, 72, 65, 83,
199, 85, 78, 32,
77, 65, 67, 72,
77, 73, 78, 69,
72, 79, 83, 69,
82, 69, 68, 32,
66, 76, 85, 69,
66, 85, 84, 84,
67, 79, 78, 83,
68, 79, 79, 82,
67, 85, 82, 84,
204, 73, 71, 72,
83, 72, 73, 80,
211, 67, 79, 85,
193, 73, 82, 76,
66, 76, 65, 67,
83, 78, 79, 87,
82, 79, 68, 32,
67, 65, 86, 69,
80, 76, 65, 83,
198, 73, 78, 71,
80, 73, 67, 75,
77, 79, 85, 78,
201, 67, 69, 32,
215, 65, 76, 76,
72, 79, 76, 68,
72, 65, 84, 67,
84, 79, 79, 76,
67, 79, 78, 78,
71, 65, 77, 69,
76, 69, 68, 71,
65, 82, 79, 85,
77, 69, 84, 65,
67, 82, 65, 87,
68, 73, 65, 77,
66, 85, 67, 75,
66, 69, 76, 84,
80, 65, 73, 78,
72, 79, 76, 69,
83, 67, 82, 73,
65, 76, 73, 69,
71, 82, 79, 85,
208, 76, 65, 78,
78, 79, 86, 65,
68, 85, 83, 84,
84, 82, 69, 69,
211, 76, 73, 77,
83, 67, 85, 76,
32, 32, 32, 32,
66, 79, 85, 76,
68, 69, 83, 84,
83, 84, 85, 78,
71, 65, 85, 71,
71, 79, 71, 71,
66, 79, 88, 32,
89, 69, 76, 76,
83, 73, 71, 78,
87, 72, 73, 84,
	0,
};
const uint8_t automap[] = {
72, 65, 84, 67,
	3,
83, 73, 71, 78,
	4,
83, 85, 73, 84,
	6,
80, 72, 65, 83,
	10,
80, 72, 65, 83,
	11,
70, 76, 65, 83,
	16,
82, 79, 68, 32,
	28,
72, 79, 85, 78,
	30,
72, 79, 85, 78,
	31,
70, 76, 65, 83,
	34,
67, 82, 89, 83,
	36,
80, 73, 67, 75,
	39,
80, 73, 67, 75,
	40,
77, 69, 84, 65,
	41,
68, 73, 65, 77,
	43,
66, 69, 76, 84,
	44,
80, 65, 73, 78,
	47,
68, 85, 83, 84,
	48,
83, 67, 85, 76,
	50,
71, 79, 71, 71,
	51,
83, 72, 79, 86,
	53,
83, 72, 79, 86,
	54,
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
