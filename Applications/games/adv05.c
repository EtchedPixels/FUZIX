#define NUM_OBJ 73
#define WORDSIZE 3
#define GAME_MAGIC 115
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
const uint8_t startcarried = 1;
const uint8_t maxcar = 7;
const uint8_t treasure = 0;
const uint8_t treasures = 0;
const uint8_t lastloc = 22;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6C\x79\x69\x6E\x67\x20\x69\x6E\x20\x61\x20\x6C\x61\x72\x67\x65\x20\x62\x72\x61\x73\x73\x20\x62\x65\x64",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x62\x65\x64\x72\x6F\x6F\x6D",
 { 11, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x41\x20\x6C\x65\x64\x67\x65\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x41\x6E\x20\x6F\x70\x65\x6E\x20\x77\x69\x6E\x64\x6F\x77",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x68\x61\x6E\x67\x69\x6E\x67\x20\x6F\x6E\x20\x74\x68\x65\x20\x65\x6E\x64\x20\x6F\x66\x20\x61\x20\x73\x68\x65\x65\x74\x2C\x20\x49\x20\x6D\x61\x64\x65\x20\x61\x20\x66\x6F\x6C\x64\x20\x69\x6E\x20\x74\x68\x65\x20\x73\x68\x65\x65\x74\x0A\x73\x6F\x20\x49\x20\x63\x61\x6E\x20\x6C\x65\x61\x76\x65\x20\x74\x68\x69\x6E\x67\x73\x20\x68\x65\x72\x65\x2E\x20\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x77\x69\x6E\x64\x6F\x77\x20\x62\x6F\x78\x20\x68\x65\x72\x65\x20\x6F\x6E\x20\x74\x68\x65\x0A\x73\x69\x64\x65\x20\x6F\x66\x20\x74\x68\x65\x20\x63\x61\x73\x74\x6C\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x66\x6C\x6F\x77\x65\x72\x20\x62\x6F\x78\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x41\x6E\x20\x6F\x70\x65\x6E\x20\x77\x69\x6E\x64\x6F\x77",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x43\x52\x59\x50\x54",
 { 0, 19, 0, 0, 0, 0 } }, 
		{ 	"\x63\x6C\x6F\x73\x65\x74",
 { 0, 0, 0, 21, 0, 0 } }, 
		{ 	"\x42\x61\x74\x68\x72\x6F\x6F\x6D",
 { 0, 11, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x74\x68\x65\x20\x63\x61\x73\x74\x6C\x65",
 { 0, 0, 17, 11, 0, 0 } }, 
		{ 	"\x44\x4F\x4F\x52\x4C\x45\x53\x53\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x68\x61\x6C\x6C\x20\x69\x6E\x73\x69\x64\x65\x20\x74\x68\x65\x20\x63\x61\x73\x74\x6C\x65",
 { 8, 2, 9, 12, 0, 0 } }, 
		{ 	"\x6B\x69\x74\x63\x68\x65\x6E",
 { 0, 0, 11, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x43\x4F\x46\x46\x49\x4E",
 { 0, 0, 0, 0, 6, 0 } }, 
		{ 	"\x70\x41\x6E\x74\x72\x79",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x67\x69\x61\x6E\x74\x20\x53\x4F\x4C\x41\x52\x20\x4F\x56\x45\x4E",
 { 0, 0, 0, 12, 0, 0 } }, 
		{ 	"\x44\x75\x6E\x67\x65\x6F\x6E",
 { 0, 0, 0, 0, 21, 0 } }, 
		{ 	"\x4D\x65\x61\x6E\x64\x65\x72\x69\x6E\x67\x20\x70\x61\x74\x68",
 { 0, 0, 0, 9, 0, 0 } }, 
		{ 	"\x50\x69\x74",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x44\x61\x72\x6B\x20\x70\x61\x73\x73\x61\x67\x65",
 { 6, 10, 0, 0, 0, 0 } }, 
		{ 	"\x64\x75\x6D\x62\x2D\x77\x61\x69\x74\x65\x72\x20\x62\x79\x20\x61\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x77\x6F\x72\x6B\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 16 } }, 
		{ 	"\x4C\x4F\x54\x20\x4F\x46\x20\x54\x52\x4F\x55\x42\x4C\x45\x21\x20\x28\x41\x6E\x64\x20\x73\x6F\x20\x41\x72\x65\x20\x79\x6F\x75\x21\x29",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	1,
	0,
	2,
	1,
	3,
	9,
	0,
	0,
	0,
	0,
	0,
	5,
	0,
	0,
	21,
	0,
	255,
	8,
	0,
	0,
	18,
	14,
	0,
	0,
	0,
	6,
	0,
	0,
	0,
	0,
	0,
	14,
	16,
	0,
	2,
	0,
	0,
	8,
	13,
	13,
	0,
	15,
	7,
	15,
	12,
	0,
	0,
	0,
	9,
	17,
	17,
	0,
	0,
	21,
	16,
	0,
	0,
	0,
	10,
	10,
	6,
	5,
	8,
	12,
	21,
	0,
	6,
	0,
	0,
	0,
	7,
	21,
	0,
};


const char *objtext[] = {
	"\x53\x68\x65\x65\x74\x73",
	"\x4F\x70\x65\x6E\x20\x77\x69\x6E\x64\x6F\x77",
	"\x43\x6C\x6F\x73\x65\x64\x20\x77\x69\x6E\x64\x6F\x77",
	"\x50\x69\x6C\x6C\x6F\x77",
	"\x46\x6C\x61\x67\x20\x70\x6F\x6C\x65\x20\x69\x6E\x20\x77\x61\x6C\x6C",
	"\x43\x6F\x61\x74\x2D\x6F\x66\x2D\x61\x72\x6D\x73",
	"\x53\x68\x65\x65\x74\x20\x67\x6F\x69\x6E\x67\x20\x69\x6E\x74\x6F\x20\x77\x69\x6E\x64\x6F\x77",
	"\x45\x6E\x64\x20\x6F\x66\x20\x73\x68\x65\x65\x74\x20\x74\x69\x65\x64\x20\x74\x6F\x20\x66\x6C\x41\x67\x70\x6F\x6C\x65",
	"\x31\x20\x6E\x6F\x64\x6F\x7A\x20\x74\x61\x62\x6C\x65\x74",
	"\x4C\x49\x54\x20\x74\x6F\x72\x63\x68",
	"\x4C\x6F\x6F\x73\x65\x20\x65\x6E\x64\x20\x6F\x66\x20\x73\x68\x65\x65\x74\x20\x67\x6F\x69\x6E\x67\x20\x6F\x76\x65\x72\x20\x6C\x65\x64\x67\x65",
	"\x45\x6E\x64\x20\x6F\x66\x20\x73\x68\x65\x65\x74\x20\x68\x41\x6E\x67\x69\x6E\x67\x20\x68\x65\x72\x65",
	"\x43\x6C\x6F\x73\x65\x64\x20\x26\x20\x55\x4E\x4C\x4F\x43\x4B\x45\x44\x20\x64\x6F\x6F\x72",
	"\x4F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
	"\x4C\x6F\x63\x6B\x65\x64\x20\x64\x6F\x6F\x72",
	"\x50\x61\x70\x65\x72\x20\x63\x6C\x69\x70",
	"\x54\x65\x6E\x74\x20\x53\x54\x41\x4B\x45",
	"\x4D\x69\x72\x72\x6F\x72",
	"\x42\x6F\x74\x74\x6C\x65\x20\x6F\x66\x20\x74\x79\x70\x65\x20\x56\x20\x62\x6C\x6F\x6F\x64",
	"\x45\x6D\x70\x74\x79\x20\x62\x6F\x74\x74\x6C\x65",
	"\x55\x6E\x6C\x69\x74\x20\x74\x6F\x72\x63\x68",
	"\x53\x75\x6C\x66\x75\x72\x20\x6D\x41\x74\x63\x68\x65\x73",
	"\x32\x20\x73\x6D\x61\x6C\x6C\x20\x68\x6F\x6C\x65\x73\x20\x69\x6E\x20\x6D\x79\x20\x6E\x65\x63\x6B",
	"\x33\x20\x6E\x6F\x2D\x64\x6F\x7A\x20\x74\x61\x62\x6C\x65\x74\x73",
	"\x32\x20\x6E\x6F\x64\x6F\x7A\x20\x74\x61\x62\x6C\x65\x74\x73",
	"\x50\x69\x6C\x65\x73\x20\x6F\x66\x20\x65\x78\x74\x69\x6E\x67\x75\x69\x73\x68\x65\x64\x20\x63\x69\x67\x41\x72\x65\x74\x74\x65\x73",
	"\x50\x61\x63\x6B\x20\x6F\x66\x20\x54\x72\x61\x6E\x73\x79\x6C\x76\x61\x6E\x69\x61\x6E\x20\x63\x69\x67\x61\x72\x65\x74\x74\x65\x73",
	"\x4C\x49\x54\x20\x63\x69\x67\x41\x72\x65\x74\x74\x65",
	"\x53\x74\x6F\x6E\x65\x20\x43\x4F\x46\x46\x49\x4E",
	"\x43\x6F\x66\x66\x69\x6E\x20\x69\x73\x20\x6F\x70\x65\x6E",
	"\x43\x6F\x66\x66\x69\x6E\x20\x69\x73\x20\x63\x6C\x6F\x73\x65\x64",
	"\x44\x75\x73\x74\x79\x20\x63\x6C\x6F\x76\x65\x20\x6F\x66\x20\x67\x61\x72\x6C\x69\x63",
	"\x44\x41\x52\x4B\x20\x70\x69\x74",
	"\x43\x69\x67\x61\x72\x65\x74\x74\x65",
	"\x42\x72\x61\x73\x73\x20\x62\x65\x64",
	"\x54\x68\x65\x20\x6F\x74\x68\x65\x72\x20\x65\x6E\x64\x20\x6F\x66\x20\x74\x68\x65\x20\x73\x68\x65\x65\x74",
	"\x53\x68\x65\x65\x74\x20\x74\x69\x65\x64\x20\x74\x6F\x20\x62\x65\x64",
	"\x50\x6F\x63\x6B\x65\x74\x20\x77\x61\x74\x63\x68",
	"\x43\x6F\x66\x66\x69\x6E\x20\x6C\x69\x64\x20\x69\x73\x20\x6F\x70\x65\x6E",
	"\x4C\x6F\x63\x6B\x61\x62\x6C\x65\x20\x73\x6C\x69\x64\x65\x20\x62\x6F\x6C\x74",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x73\x6C\x69\x64\x65\x20\x6C\x6F\x63\x6B",
	"\x4C\x61\x72\x67\x65\x20\x74\x65\x6D\x70\x65\x72\x65\x64\x20\x6E\x61\x69\x6C\x20\x66\x69\x6C\x65",
	"\x53\x6D\x61\x6C\x6C\x20\x56\x69\x61\x6C",
	"\x4C\x61\x72\x67\x65\x20\x64\x61\x72\x6B\x20\x6C\x65\x6E\x73\x20\x73\x65\x74\x20\x69\x6E\x20\x63\x65\x69\x6C\x69\x6E\x67",
	"\x4F\x76\x65\x6E",
	"\x50\x61\x63\x6B\x61\x67\x65",
	"\x45\x6D\x70\x74\x79\x20\x62\x6F\x78",
	"\x50\x6F\x73\x74\x63\x61\x72\x64",
	"\x42\x65\x6C\x6C\x20\x70\x75\x6C\x6C",
	"\x43\x61\x73\x74\x6C\x65\x20\x74\x6F\x77\x65\x72\x69\x6E\x67\x20\x61\x62\x6F\x76\x65\x20\x6D\x65",
	"\x46\x65\x6E\x63\x65\x20\x77\x69\x74\x68\x20\x61\x6E\x20\x6F\x70\x65\x6E\x20\x67\x61\x74\x65\x20\x26\x20\x61\x20\x63\x72\x6F\x77\x64\x20\x62\x65\x79\x6F\x6E\x64",
	"\x4E\x6F\x74\x65",
	"\x44\x52\x41\x43\x55\x4C\x41",
	"\x52\x75\x62\x62\x65\x72\x20\x6D\x61\x6C\x6C\x65\x74",
	"\x49\x72\x6F\x6E\x20\x72\x69\x6E\x67\x73\x20\x69\x6E\x20\x77\x41\x6C\x6C",
	"\x53\x68\x65\x65\x74\x20\x74\x69\x65\x64\x20\x74\x6F\x20\x72\x69\x6E\x67\x20\x67\x6F\x69\x6E\x67\x20\x69\x6E\x74\x6F\x20\x70\x69\x74",
	"\x44\x41\x52\x4B\x20\x66\x6F\x72\x65\x62\x6F\x64\x69\x6E\x67\x20\x70\x61\x73\x73\x61\x67\x65",
	"\x2E",
	"\x46\x75\x6C\x6C\x20\x73\x69\x7A\x65\x20\x70\x6F\x72\x74\x72\x61\x69\x74\x20\x6F\x66\x20\x44\x52\x41\x43\x55\x4C\x41",
	"\x57\x69\x6E\x64\x6F\x77",
	"\x56\x65\x6E\x74",
	"\x44\x61\x69\x73\x69\x65\x73",
	"\x54\x6F\x69\x6C\x65\x74",
	"\x44\x75\x6D\x62\x2D\x77\x61\x69\x74\x65\x72",
	"\x56\x65\x6E\x74",
	"\x4C\x65\x74\x74\x65\x72",
	"\x53\x69\x67\x6E\x20\x73\x61\x79\x73\x3A\x20\x22\x50\x4F\x53\x49\x54\x49\x56\x45\x4C\x59\x20\x4E\x4F\x20\x53\x4D\x4F\x4B\x49\x4E\x47\x20\x41\x4C\x4C\x4F\x57\x45\x44\x20\x48\x45\x52\x45\x21\x22\x20\x73\x69\x67\x6E\x65\x64\x20\x44\x72\x61\x63\x75\x6C\x61",
	"\x4D\x6F\x75\x6C\x64\x79\x20\x6F\x6C\x64\x20\x73\x6B\x65\x6C\x65\x74\x6F\x6E\x20\x77\x69\x74\x68\x20\x61\x20\x73\x74\x61\x6B\x65\x20\x69\x6E\x20\x74\x68\x65\x20\x72\x69\x62\x20\x63\x61\x67\x65",
	"\x2E",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x67\x6C\x61\x73\x73",
	"\x43\x65\x6E\x74\x75\x72\x79\x20\x77\x6F\x72\x74\x68\x20\x6F\x66\x20\x64\x75\x73\x74",
	"\x4D\x65\x6D\x6F\x20\x74\x61\x63\x6B\x65\x64\x20\x74\x6F\x20\x74\x68\x65\x20\x64\x6F\x6F\x72",
	"\x2E",
};
const char *msgptr[] = {
	"",
	"\x53\x6F\x72\x72\x79\x20\x49\x20\x63\x61\x6E\x27\x74\x20\x64\x6F\x20\x74\x68\x61\x74",
	"\x49\x20\x73\x65\x65\x20\x49\x20\x77\x61\x73\x20\x70\x75\x74\x20\x74\x6F\x20\x62\x65\x64\x2E\x20\x49\x74\x73\x20\x41\x46\x74\x65\x72\x6E\x6F\x6F\x6E\x20\x26\x20\x49\x20\x6F\x76\x65\x72\x73\x6C\x65\x70\x74\x21",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x44\x56\x45\x57\x4E\x54\x55\x52\x45\x3A\x20\x35\x20\x22\x54\x48\x45\x20\x43\x4F\x55\x4E\x54\x22\x2E\x20\x44\x65\x64\x69\x63\x61\x74\x65\x64\x20\x74\x6F\x20\x41\x6C\x76\x69\x6E\x20\x46\x69\x6C\x65\x73\x2E\x0A",
	"\x49\x27\x6D\x20\x75\x70\x20\x69\x6E\x20\x41\x20\x63\x41\x73\x74\x6C\x65\x2C\x20\x69\x6E\x20\x74\x68\x65\x20\x64\x69\x73\x74\x41\x6E\x63\x65\x20\x49\x20\x73\x65\x65\x20\x56\x4F\x4F\x44\x4F\x4F\x20\x43\x41\x53\x54\x4C\x45\x2E\x20\x54\x68\x65\x72\x65\x27\x73\x73\x74\x41\x6E\x64\x69\x6E\x67\x20\x72\x6F\x6F\x6D\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x74\x68\x65\x20\x77\x69\x6E\x64\x6F\x77",
	"\x53\x6F\x6D\x65\x20\x74\x69\x6D\x65\x20\x70\x61\x73\x73\x65\x73\x2E\x2E\x2E",
	"\x49\x74\x20\x62\x65\x61\x72\x73\x20\x74\x68\x65\x20\x46\x61\x6D\x69\x6C\x79\x20\x43\x72\x65\x73\x74\x20\x6F\x66\x20\x44\x52\x41\x43\x55\x4C\x41\x21",
	"\x54\x65\x6C\x6C\x20\x6D\x65\x20\x74\x6F\x20\x77\x68\x61\x74\x3F\x20\x69\x2E\x65\x2E\x20\x22\x54\x4F\x20\x54\x52\x45\x45\x22",
	"\x68\x69\x67\x68\x65\x72",
	"\x49\x20\x68\x65\x61\x72\x20\x74\x68\x65",
	"\x46\x6C\x61\x67\x70\x6F\x6C\x65\x20\x73\x70\x6C\x69\x6E\x74\x65\x72",
	"\x6F\x66\x20\x79\x6F\x75\x20\x61\x6C\x77\x61\x79\x73\x20\x61\x73\x6B\x69\x6E\x67\x20\x66\x6F\x72\x20\x68\x65\x6C\x70\x21",
	"\x49\x20\x66\x61\x6C\x6C\x20\x74\x6F\x20\x6D\x79\x20\x64\x65\x61\x74\x68",
	"\x48\x6F\x77\x3F",
	"\x49\x27\x76\x65\x20\x74\x75\x72\x6E\x65\x64\x20\x69\x6E\x74\x6F\x20\x41\x20\x56\x41\x4D\x50\x49\x52\x45\x21",
	"\x49\x20\x74\x68\x69\x6E\x6B\x20\x49\x20\x63\x61\x6E\x20\x6F\x6E\x6C\x79\x20\x74\x61\x6B\x65",
	"\x6D\x6F\x72\x65\x20\x64\x61\x79\x73\x20\x6F\x66\x20\x74\x68\x69\x73\x21",
	"\x49\x20\x61\x70\x70\x65\x61\x72\x20\x70\x61\x6C\x65\x20\x26\x20\x64\x72\x61\x69\x6E\x65\x64\x21",
	"\x4D\x79\x20\x6E\x65\x63\x6B\x20\x6C\x6F\x6F\x6B\x73\x20\x42\x49\x54\x54\x45\x4E\x21",
	"\x49\x74\x73\x20\x67\x65\x74\x74\x69\x6E\x67\x20\x44\x41\x52\x4B\x20\x6F\x75\x74\x73\x69\x64\x65\x21",
	"\x49\x27\x6D\x20\x67\x65\x74\x74\x69\x6E\x67\x20\x76\x65\x72\x79\x20\x74\x69\x72\x65\x64",
	"\x54\x68\x65\x20\x73\x75\x6E\x20\x68\x61\x73\x20\x73\x65\x74\x21",
	"\x54\x4F\x44\x41\x59\x20\x49\x20\x6C\x6F\x6F\x6B\x20\x68\x65\x61\x6C\x74\x68\x79\x2E\x2E\x2E",
	"\x41\x20\x62\x41\x74\x20\x66\x6C\x65\x77\x20\x62\x79\x20\x26\x20\x4C\x41\x55\x47\x48\x45\x44\x20\x41\x74\x20\x6D\x65\x21",
	"\x49\x27\x6D\x20\x72\x65\x61\x6C\x20\x50\x45\x50\x50\x59\x20\x6E\x6F\x77\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x41\x20\x43\x4F\x55\x47\x48\x49\x4E\x20\x28\x73\x69\x63\x29\x20\x69\x6E\x20\x74\x68\x65\x20\x72\x6F\x6F\x6D\x2E",
	"\x43\x4F\x55\x47\x48\x21",
	"\x54\x68\x65\x20\x63\x69\x67\x41\x72\x65\x74\x74\x65\x20\x77\x65\x6E\x74\x20\x6F\x75\x74\x20\x26\x20\x74\x68\x65\x20\x63\x6F\x66\x66\x69\x6E\x20\x56\x41\x4E\x49\x53\x48\x45\x44",
	"\x48\x65\x20\x73\x6D\x65\x6C\x6C\x65\x64\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x26\x20\x66\x6C\x65\x77\x20\x6F\x6E",
	"\x68\x65\x20\x73\x65\x74\x74\x6C\x65\x64\x20\x6F\x6E\x20\x6D\x79\x20\x4E\x45\x43\x4B\x21",
	"\x53\x68\x65\x65\x74\x20\x63\x61\x6D\x65\x20\x75\x6E\x74\x69\x65\x64",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x77\x61\x74\x63\x68\x20\x73\x61\x79\x73",
	"\x6D\x6F\x76\x65\x73\x20\x74\x69\x6C\x6C\x20\x73\x75\x6E\x73\x65\x74",
	"\x49\x20\x63\x6C\x6F\x73\x65\x64\x20\x74\x68\x65\x20\x6C\x69\x64\x20\x61\x6E\x64\x20\x49\x20\x73\x75\x66\x66\x6F\x63\x61\x74\x65\x64\x21",
	"\x49\x74\x73\x20\x4C\x4F\x43\x4B\x45\x44\x20\x66\x72\x6F\x6D\x20\x49\x4E\x53\x49\x44\x45\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x74\x68\x65\x72\x65\x2C\x20\x6D\x61\x79\x62\x65\x20\x49\x20\x73\x68\x6F\x75\x6C\x64",
	"\x67\x6F\x20\x74\x68\x65\x72\x65\x3F",
	"\x65\x6D\x70\x74\x79\x20\x69\x74\x3F",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C\x2E",
	"\x4F\x4B",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x41\x20\x74\x72\x65\x6D\x65\x6E\x64\x6F\x75\x73\x20\x41\x6D\x6F\x75\x6E\x74\x20\x6F\x66\x20\x48\x45\x41\x54\x20\x26\x20\x53\x55\x4E\x4C\x49\x47\x48\x54\x20\x63\x6F\x6D\x69\x6E\x67\x20\x6F\x75\x74\x2E",
	"\x44\x65\x41\x72\x20\x44\x52\x41\x43\x4B\x59\x2C\x20\x44\x6F\x6E\x27\x74\x20\x6F\x70\x65\x6E\x20\x74\x68\x69\x73\x20\x70\x72\x65\x73\x65\x6E\x74\x20\x74\x69\x6C\x6C\x20\x48\x41\x4C\x4C\x4F\x57\x45\x45\x4E\x2E\x0A\x20\x73\x69\x67\x6E\x65\x64\x20\x43\x4F\x55\x4E\x54\x20\x59\x4F\x52\x47\x41\x2E",
	"\x20\x0A\x41\x20\x62\x65\x6C\x6C\x20\x72\x69\x6E\x67\x73\x20\x73\x6F\x6D\x65\x77\x68\x65\x72\x65\x3A\x20\x22\x44\x49\x4E\x47\x2D\x44\x4F\x4E\x47\x22\x2E\x0A",
	"\x50\x65\x61\x73\x61\x6E\x74\x73\x20\x61\x74\x74\x61\x63\x6B\x20\x6D\x65\x2C\x20\x49\x20\x77\x61\x73\x20\x53\x55\x50\x50\x4F\x53\x45\x44\x20\x74\x6F\x20\x64\x65\x73\x74\x72\x6F\x79\x20\x74\x68\x65\x20\x56\x41\x4D\x50\x49\x52\x45\x21",
	"\x59\x4F\x55\x20\x48\x41\x56\x45\x20\x4C\x4F\x53\x54\x21",
	"\x49\x74\x73\x20\x66\x6F\x72\x20\x44\x52\x41\x43\x55\x4C\x41\x2C\x20\x69\x74\x73\x20\x61\x6E\x64\x20\x45\x41\x54\x49\x4E\x47\x20\x26\x20\x47\x48\x4F\x55\x4C\x49\x4E\x47\x20\x62\x69\x6C\x6C\x20\x66\x72\x6F\x6D\x20\x61\x0A\x6C\x6F\x63\x61\x6C\x20\x6D\x6F\x72\x74\x75\x61\x72\x79\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x6E\x6F\x74\x65\x20\x50\x41\x50\x45\x52\x20\x43\x4C\x49\x50\x50\x45\x44\x20\x74\x6F\x20\x74\x68\x65\x20\x70\x6F\x73\x74\x63\x61\x72\x64",
	"\x50\x6F\x73\x74\x6D\x61\x73\x74\x65\x72\x20\x73\x61\x79\x73\x20\x68\x65\x27\x6C\x6C\x20\x62\x65\x20\x64\x65\x6C\x69\x76\x65\x72\x69\x6E\x67\x20\x61\x20\x70\x61\x63\x6B\x61\x67\x65\x20\x74\x6F\x6D\x6F\x72\x72\x6F\x77\x2E",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x6B\x6E\x6F\x77\x20\x77\x68\x65\x72\x65\x20\x74\x6F\x20\x6C\x6F\x6F\x6B",
	"\x49\x20\x64\x72\x69\x76\x65\x20\x74\x68\x65\x20\x73\x74\x61\x6B\x65\x20\x74\x68\x72\x6F\x75\x67\x68\x20\x68\x69\x73\x20\x48\x45\x41\x52\x54\x2E\x20\x54\x68\x65\x20\x74\x6F\x77\x6E\x73\x70\x65\x6F\x70\x6C\x65\x0A\x63\x6F\x6D\x65\x20\x61\x6E\x64\x20\x63\x61\x72\x72\x79\x20\x6D\x65\x20\x6F\x66\x66\x20\x63\x68\x65\x65\x72\x69\x6E\x67\x21\x20\x28\x44\x6F\x6E\x27\x74\x20\x77\x6F\x72\x72\x79\x2C\x20\x49\x20\x74\x65\x6C\x6C\x20\x74\x68\x65\x6D\x20\x49\x0A\x6F\x77\x65\x20\x69\x74\x20\x61\x6C\x6C\x20\x74\x6F\x20\x79\x6F\x75\x21\x21\x21\x21\x29",
	"\x54\x72\x79\x3A\x20\x22\x43\x4C\x49\x4D\x42\x20\x53\x48\x45\x45\x54\x22",
	"\x49\x27\x6D\x20\x41\x20\x70\x72\x65\x74\x74\x79\x20\x67\x6F\x6F\x64\x20\x4C\x4F\x43\x4B\x20\x50\x49\x43\x4B\x21",
	"\x49\x27\x6D\x20\x6E\x6F\x74\x20\x61\x6E\x20\x61\x72\x73\x6F\x6E\x69\x73\x74\x21",
	"\x54\x68\x65\x20\x74\x6F\x72\x63\x68\x20\x62\x75\x72\x6E\x74\x20\x74\x68\x72\x75\x20\x74\x68\x65\x20\x73\x68\x65\x65\x74\x21",
	"\x41\x68\x20\x74\x68\x61\x74\x27\x73\x20\x6D\x75\x63\x68\x20\x62\x65\x74\x74\x65\x72\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x77\x72\x69\x74\x69\x6E\x67\x20\x6F\x6E\x20\x69\x74\x2E",
	"\x4D\x6F\x62\x20\x6C\x6F\x6F\x6B\x73\x20\x41\x4E\x47\x52\x59",
	"\x49\x27\x76\x65\x20\x61\x20\x68\x75\x6E\x63\x68\x20\x49\x27\x76\x65\x20\x62\x65\x65\x6E\x20\x72\x6F\x62\x62\x65\x64\x21",
	"\x4F\x64\x64\x2C\x20\x49\x20\x77\x61\x73\x6E\x27\x74\x20\x62\x69\x74\x74\x65\x6E\x20\x6C\x61\x73\x74\x20\x6E\x69\x74\x65\x21",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x73\x2E",
	"\x6C\x6F\x77\x65\x72",
	"\x49\x27\x6D\x20\x41\x74\x20\x74\x68\x65\x20\x74\x69\x70\x20\x6F\x66\x20\x74\x68\x65\x20\x70\x6F\x6C\x65",
	"\x49\x20\x6E\x6F\x74\x69\x63\x65\x20\x61\x20\x44\x41\x52\x4B\x20\x57\x49\x4E\x44\x4F\x57\x20\x55\x4E\x44\x45\x52\x20\x74\x68\x65\x20\x62\x65\x64\x72\x6F\x6F\x6D\x20\x77\x69\x6E\x64\x6F\x77\x20\x6C\x65\x64\x67\x65\x21",
	"\x61\x73\x20\x49\x20\x66\x61\x6C\x6C",
	"\x45\x58\x43\x45\x50\x54",
	"\x49\x74\x20\x77\x6F\x6E\x27\x74\x20\x67\x6F\x20\x61\x6E\x79",
	"\x74\x72\x79\x20\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x61\x74\x20\x69\x74",
	"\x54\x68\x65\x20\x70\x61\x70\x65\x72\x20\x63\x6C\x69\x70\x20\x69\x73\x20\x69\x6E\x20\x74\x68\x65\x20\x77\x61\x79\x21",
	"\x49\x20\x6B\x6E\x6F\x77\x20\x68\x6F\x77\x20\x74\x6F\x20\x52\x41\x49\x53\x45\x20\x61\x6E\x64\x20\x4C\x4F\x57\x45\x52\x20\x74\x68\x69\x73\x20\x74\x68\x69\x6E\x67\x21",
	"\x47\x45\x54\x20\x55\x50\x20\x79\x6F\x75\x20\x73\x6C\x65\x65\x70\x79\x20\x68\x65\x61\x64\x21",
	"\x54\x72\x79\x20\x65\x78\x61\x6D\x69\x6E\x69\x6E\x67\x20\x74\x68\x69\x6E\x67\x73\x2E",
	"\x49\x27\x76\x65\x20\x6E\x6F\x20\x6D\x61\x74\x63\x68\x65\x73\x21",
	"\x54\x68\x65\x20\x6D\x61\x74\x63\x68\x20\x66\x6C\x61\x72\x65\x73\x20\x75\x70\x20\x62\x72\x69\x65\x66\x6C\x79\x20\x2E\x2E\x2E",
	"\x49\x74\x73\x20\x61\x6C\x72\x65\x61\x64\x79\x20\x6F\x70\x65\x6E\x2E",
	"\x54\x72\x79\x3A\x20\x22\x43\x4C\x49\x4D\x42\x20\x50\x4F\x4C\x45\x22",
	"\x54\x72\x79\x3A\x20\x22\x53\x41\x56\x45\x20\x47\x41\x4D\x45\x22",
	"\x41\x20\x62\x61\x74\x20\x6D\x69\x67\x68\x74\x20\x6D\x61\x6B\x65\x20\x69\x74\x2C\x20\x62\x75\x74\x20\x6E\x6F\x74\x20\x6D\x65\x21",
	"\x46\x69\x72\x73\x74\x20\x49\x20\x6E\x65\x65\x64\x20\x61\x6E\x20\x75\x6E\x6C\x69\x74\x20\x63\x69\x67\x61\x72\x65\x74\x74\x65\x2E",
	"\x54\x65\x6C\x6C\x20\x6D\x65\x20\x77\x69\x74\x68\x20\x77\x68\x61\x74\x3F\x20\x4C\x69\x6B\x65\x3A\x20\x22\x57\x49\x54\x48\x20\x46\x49\x53\x54\x22",
	"\x49\x74\x20\x64\x6F\x65\x73\x6E\x27\x74\x20\x77\x6F\x72\x6B\x21",
	"\x4D\x61\x79\x62\x65\x20\x49\x20\x73\x68\x6F\x75\x6C\x64\x20\x42\x52\x45\x41\x4B\x20\x69\x74\x3F",
	"\x59\x75\x63\x6B\x21",
	"\x61\x6E\x64\x20\x74\x68\x65\x6E\x20\x67\x6F\x65\x73\x20\x6F\x75\x74\x21",
	"\x4D\x69\x72\x72\x6F\x72\x20\x73\x68\x61\x74\x74\x65\x72\x73\x21\x20\x54\x68\x61\x74\x27\x73\x20\x37\x20\x79\x65\x61\x72\x73\x20\x62\x61\x64\x20\x6C\x75\x63\x6B\x21",
	"\x49\x20\x73\x65\x74\x20\x74\x68\x65\x20\x6D\x69\x72\x72\x6F\x72\x20\x6F\x6E\x20\x74\x68\x65\x20\x70\x69\x6C\x6C\x6F\x77\x2E",
	"\x54\x72\x79\x3A\x20\x22\x47\x4F\x20\x52\x4F\x4F\x4D\x22",
	"\x57\x68\x61\x74\x20\x61\x20\x44\x52\x45\x41\x4D\x20\x49\x20\x6A\x75\x73\x74\x20\x68\x61\x64\x20\x21\x21\x21\x21",
	"\x44\x65\x61\x72\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x72\x3A\x0A\x20\x49\x20\x77\x61\x6E\x74\x65\x64\x20\x74\x6F\x20\x74\x61\x6B\x65\x20\x74\x68\x69\x73\x20\x74\x69\x6D\x65\x20\x28\x31\x20\x6D\x6F\x76\x65\x21\x29\x20\x74\x6F\x20\x74\x68\x61\x6E\x6B\x20\x41\x4C\x4C\x20\x6F\x66\x20\x79\x6F\x75\x20\x6F\x75\x74\x0A\x74\x68\x65\x72\x65\x20\x69\x6E\x20\x41\x44\x76\x65\x6E\x74\x75\x72\x65\x6C\x61\x6E\x64\x20\x66\x6F\x72\x20\x74\x68\x65\x20\x66\x61\x6E\x74\x61\x73\x74\x69\x63\x61\x6C\x6C\x79\x20\x77\x61\x72\x6D\x20\x72\x65\x63\x65\x70\x74\x69\x6F\x6E\x0A\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x68\x61\x73\x20\x72\x65\x63\x65\x69\x76\x65\x64\x21\x0A\x48\x61\x70\x70\x79\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x69\x6E\x67\x2C\x0A\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x0A\x28\x43\x68\x69\x65\x66\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x72\x29",
	"\x41\x20\x63\x68\x6F\x6F\x6F\x6F\x6F\x6F",
};


const uint8_t status[] = {
	161, 
	77, 73, 
	211, 
	9, 1, 0, 3, 0, 0, 0, 65, 
	3, 79, 81, 79, 
	211, 
	9, 1, 0, 3, 0, 0, 0, 3, 
	2, 81, 79, 81, 
	212, 
	15, 10, 16, 1, 9, 2, 7, 18, 7, 6, 
	19, 
	208, 
	15, 4, 9, 4, 16, 1, 9, 2, 
	20, 
	198, 
	19, 1, 
	56, 21, 64, 
	176, 
	2, 35, 4, 3, 0, 35, 0, 10, 
	72, 
	170, 
	8, 4, 0, 1, 
	81, 77, 73, 
	200, 
	15, 8, 15, 4, 
	20, 
	200, 
	15, 0, 0, 4, 
	60, 
	196, 
	0, 1, 
	81, 
	143, 33, 
	4, 4, 13, 7, 0, 22, 
	9, 10, 12, 54, 
	178, 
	1, 35, 4, 11, 0, 36, 0, 35, 
	59, 72, 30, 
	177, 
	15, 0, 9, 4, 0, 2, 7, 22, 
	58, 70, 
	145, 25, 
	15, 0, 1, 31, 9, 2, 7, 22, 
	23, 28, 
	151, 40, 
	15, 0, 6, 31, 9, 2, 7, 22, 0, 2, 
	23, 29, 58, 88, 
	142, 50, 
	4, 4, 3, 9, 0, 22, 
	103, 12, 54, 
	183, 
	8, 2, 0, 7, 0, 3, 0, 2, 7, 22, 
	73, 54, 58, 60, 
	214, 
	14, 14, 0, 0, 0, 13, 0, 21, 0, 12, 
	54, 62, 59, 
	209, 
	13, 16, 5, 16, 0, 16, 0, 9, 
	59, 58, 
	211, 
	0, 6, 0, 7, 0, 10, 0, 35, 
	59, 59, 59, 59, 
	215, 
	0, 1, 0, 2, 0, 2, 0, 65, 0, 55, 
	59, 62, 79, 59, 
	209, 
	13, 45, 5, 45, 0, 45, 0, 9, 
	59, 58, 
	213, 
	13, 26, 5, 26, 0, 26, 0, 13, 0, 9, 
	62, 58, 
	201, 
	4, 0, 0, 1, 
	54, 136, 
	206, 
	8, 9, 0, 9, 0, 1, 
	60, 54, 107, 
	199, 
	0, 1, 
	54, 57, 2, 53, 
	210, 
	3, 18, 0, 3, 0, 18, 0, 19, 
	60, 72, 108, 
	203, 
	0, 4, 0, 36, 
	60, 59, 53, 64, 
	171, 
	8, 3, 0, 3, 
	60, 81, 77, 73, 
	202, 
	15, 0, 0, 22, 
	18, 14, 54, 
	202, 
	0, 0, 0, 22, 
	18, 81, 74, 
	142, 50, 
	13, 27, 7, 13, 0, 2, 
	81, 77, 73, 
	215, 
	19, 0, 0, 27, 0, 28, 0, 29, 0, 30, 
	59, 59, 59, 59, 
	200, 
	4, 6, 19, 0, 
	27, 
	196, 
	0, 2, 
	81, 
	165, 
	4, 22, 
	44, 63, 
	149, 30, 
	15, 25, 14, 45, 0, 0, 7, 9, 14, 46, 
	73, 81, 
	214, 
	19, 2, 0, 45, 0, 9, 0, 65, 0, 9, 
	62, 62, 42, 
	192, 
	81, 
	168, 
	9, 1, 0, 1, 
	58, 
	169, 
	15, 1, 12, 9, 
	56, 64, 
	173, 
	4, 10, 16, 0, 8, 15, 
	57, 76, 
	149, 29, 
	15, 39, 14, 47, 0, 47, 0, 9, 7, 9, 
	62, 42, 
};
const uint8_t actions[] = {
	4, 31, 7, 
	3, 0, 
	7, 
	10, 1, 9, 
	4, 4, 0, 5, 
	54, 76, 39, 
	10, 1, 9, 
	4, 5, 0, 10, 
	54, 76, 39, 
	11, 8, 43, 
	3, 17, 13, 22, 
	73, 18, 17, 15, 
	195, 
	81, 78, 81, 16, 
	14, 43, 20, 
	3, 9, 0, 9, 0, 20, 
	72, 39, 64, 
	18, 45, 20, 
	3, 20, 3, 21, 0, 9, 0, 20, 
	72, 76, 39, 
	13, 50, 41, 
	3, 18, 0, 18, 0, 19, 
	72, 39, 
	8, 8, 43, 
	3, 17, 14, 22, 
	22, 
	4, 8, 9, 
	4, 2, 
	4, 
	14, 34, 9, 
	2, 1, 0, 1, 0, 2, 
	72, 76, 39, 
	19, 1, 9, 
	2, 1, 1, 35, 0, 3, 0, 6, 
	54, 53, 76, 39, 
	23, 42, 33, 
	3, 0, 2, 34, 0, 0, 0, 35, 0, 36, 
	59, 53, 53, 76, 
	23, 42, 12, 
	3, 0, 3, 4, 0, 0, 0, 7, 0, 10, 
	59, 53, 53, 76, 
	10, 28, 0, 
	2, 10, 0, 4, 
	54, 76, 39, 
	10, 28, 0, 
	2, 11, 0, 4, 
	54, 76, 39, 
	10, 40, 0, 
	0, 2, 0, 4, 
	58, 70, 60, 
	6, 8, 46, 
	3, 37, 
	31, 78, 32, 
	19, 1, 9, 
	4, 3, 1, 35, 0, 2, 0, 6, 
	54, 59, 76, 39, 
	8, 8, 44, 
	16, 0, 3, 44, 
	40, 
	9, 8, 44, 
	3, 44, 15, 0, 
	35, 36, 
	23, 42, 55, 
	2, 54, 0, 0, 0, 35, 0, 18, 0, 55, 
	59, 62, 53, 76, 
	15, 32, 0, 
	2, 55, 0, 35, 0, 55, 
	59, 59, 53, 76, 
	15, 32, 0, 
	2, 7, 0, 7, 0, 10, 
	59, 59, 53, 76, 
	6, 8, 37, 
	2, 63, 
	39, 35, 36, 
	23, 58, 53, 
	3, 23, 0, 4, 0, 23, 0, 24, 0, 1, 
	58, 72, 81, 73, 
	203, 
	0, 40, 0, 1, 
	79, 81, 39, 24, 
	23, 58, 53, 
	3, 24, 0, 4, 0, 24, 0, 8, 0, 1, 
	58, 72, 81, 73, 
	203, 
	0, 40, 0, 1, 
	79, 81, 39, 24, 
	5, 8, 21, 
	4, 4, 
	35, 36, 
	22, 25, 0, 
	3, 27, 4, 6, 0, 28, 0, 30, 14, 28, 
	25, 53, 53, 
	5, 8, 54, 
	2, 62, 
	35, 36, 
	5, 8, 16, 
	4, 20, 
	35, 36, 
	23, 45, 29, 
	3, 33, 3, 21, 0, 2, 0, 8, 0, 2, 
	81, 79, 81, 73, 
	201, 
	0, 33, 0, 27, 
	72, 39, 
	9, 10, 29, 
	3, 33, 0, 33, 
	52, 39, 
	4, 8, 48, 
	3, 47, 
	105, 
	4, 8, 59, 
	3, 65, 
	105, 
	9, 10, 29, 
	3, 27, 0, 27, 
	52, 39, 
	4, 8, 50, 
	3, 50, 
	106, 
	4, 8, 34, 
	2, 30, 
	38, 
	5, 8, 34, 
	2, 29, 
	35, 36, 
	9, 8, 47, 
	3, 42, 9, 6, 
	35, 37, 
	14, 37, 19, 
	2, 12, 0, 12, 0, 13, 
	72, 76, 39, 
	14, 34, 19, 
	2, 13, 0, 12, 0, 13, 
	72, 76, 39, 
	18, 29, 19, 
	2, 12, 3, 15, 0, 12, 0, 14, 
	72, 76, 39, 
	7, 8, 28, 
	2, 49, 
	39, 38, 114, 112, 
	18, 10, 56, 
	2, 14, 3, 15, 0, 14, 0, 12, 
	72, 76, 39, 
	8, 41, 17, 
	3, 47, 14, 51, 
	117, 
	10, 1, 33, 
	2, 34, 0, 1, 
	54, 76, 39, 
	9, 1, 44, 
	3, 44, 16, 0, 
	1, 40, 
	14, 1, 44, 
	3, 44, 15, 0, 0, 15, 
	54, 76, 39, 
	18, 61, 39, 
	3, 41, 2, 39, 0, 39, 0, 40, 
	72, 76, 39, 
	14, 37, 9, 
	2, 2, 0, 1, 0, 2, 
	72, 76, 39, 
	10, 1, 37, 
	2, 63, 0, 20, 
	54, 76, 39, 
	23, 22, 25, 
	3, 52, 3, 16, 3, 53, 0, 52, 0, 67, 
	72, 76, 49, 63, 
	18, 1, 16, 
	9, 8, 9, 7, 4, 20, 0, 12, 
	54, 76, 39, 
	18, 50, 47, 
	3, 42, 0, 23, 9, 6, 0, 6, 
	53, 76, 58, 
	19, 32, 0, 
	2, 36, 0, 36, 0, 6, 0, 35, 
	59, 59, 59, 73, 
	198, 
	0, 10, 
	59, 53, 76, 
	7, 52, 39, 
	3, 41, 
	39, 5, 77, 77, 
	9, 49, 0, 
	16, 5, 0, 5, 
	5, 83, 
	5, 56, 58, 
	2, 48, 
	39, 42, 
	19, 58, 53, 
	3, 8, 0, 4, 0, 8, 0, 1, 
	58, 59, 81, 73, 
	203, 
	0, 40, 0, 1, 
	79, 81, 39, 24, 
	5, 61, 39, 
	12, 41, 
	1, 107, 
	11, 1, 32, 
	2, 56, 0, 19, 
	54, 56, 76, 39, 
	1, 26, 0, 
	1, 48, 
	10, 1, 9, 
	4, 10, 0, 5, 
	54, 76, 39, 
	18, 10, 36, 
	3, 58, 0, 58, 0, 56, 0, 10, 
	52, 62, 39, 
	15, 28, 0, 
	2, 35, 4, 18, 0, 16, 
	54, 57, 76, 39, 
	11, 28, 0, 
	2, 32, 0, 18, 
	56, 54, 76, 39, 
	0, 1, 7, 
	50, 
	11, 1, 18, 
	2, 32, 0, 18, 
	56, 54, 76, 39, 
	5, 34, 0, 
	2, 38, 
	33, 63, 
	13, 34, 0, 
	2, 29, 0, 29, 0, 30, 
	72, 76, 
	13, 37, 34, 
	2, 28, 13, 39, 16, 0, 
	1, 34, 
	15, 37, 34, 
	2, 28, 0, 30, 0, 29, 
	59, 53, 76, 73, 
	204, 
	16, 0, 0, 52, 0, 13, 
	62, 
	9, 58, 30, 
	3, 31, 0, 31, 
	39, 59, 
	6, 1, 50, 
	2, 50, 
	43, 44, 63, 
	14, 1, 16, 
	4, 20, 8, 7, 0, 21, 
	54, 76, 39, 
	4, 41, 59, 
	3, 65, 
	41, 
	8, 41, 48, 
	3, 47, 13, 51, 
	45, 
	9, 41, 48, 
	3, 47, 14, 51, 
	45, 46, 
	4, 41, 17, 
	3, 51, 
	47, 
	19, 10, 23, 
	3, 47, 14, 15, 0, 15, 0, 51, 
	74, 74, 70, 39, 
	19, 7, 0, 
	3, 47, 14, 15, 0, 15, 0, 51, 
	74, 74, 70, 39, 
	9, 10, 17, 
	3, 51, 0, 51, 
	74, 39, 
	9, 10, 23, 
	3, 15, 0, 15, 
	74, 39, 
	19, 37, 21, 
	3, 45, 0, 46, 0, 26, 0, 18, 
	53, 53, 53, 73, 
	197, 
	0, 45, 
	59, 39, 
	4, 27, 0, 
	2, 14, 
	51, 
	11, 45, 22, 
	3, 21, 8, 15, 
	57, 76, 122, 73, 
	195, 
	88, 88, 88, 132, 
	193, 
	56, 76, 
	11, 48, 0, 
	4, 3, 0, 22, 
	12, 113, 112, 54, 
	14, 1, 16, 
	4, 20, 8, 8, 0, 14, 
	54, 76, 39, 
	10, 28, 0, 
	4, 4, 0, 3, 
	54, 76, 39, 
	1, 24, 63, 
	39, 71, 
	5, 10, 7, 
	3, 0, 
	52, 39, 
	6, 37, 9, 
	4, 4, 
	1, 35, 36, 
	5, 1, 54, 
	2, 62, 
	39, 104, 
	8, 42, 0, 
	3, 0, 7, 1, 
	1, 
	9, 10, 7, 
	3, 35, 0, 35, 
	52, 39, 
	4, 41, 78, 
	3, 71, 
	137, 
	10, 10, 5, 
	4, 1, 0, 2, 
	54, 76, 39, 
	4, 16, 0, 
	4, 20, 
	135, 
	4, 33, 0, 
	4, 0, 
	51, 
	4, 37, 59, 
	3, 65, 
	105, 
	9, 58, 41, 
	3, 18, 0, 22, 
	54, 14, 
	10, 1, 19, 
	2, 13, 0, 7, 
	54, 76, 39, 
	0, 56, 0, 
	109, 
	4, 25, 0, 
	3, 27, 
	26, 
	4, 43, 19, 
	2, 14, 
	13, 
	4, 27, 0, 
	4, 20, 
	118, 
	0, 6, 0, 
	63, 
	7, 28, 12, 
	2, 4, 
	70, 111, 73, 9, 
	199, 
	0, 22, 
	10, 113, 54, 112, 
	10, 18, 37, 
	4, 20, 8, 7, 
	1, 115, 110, 
	22, 18, 37, 
	4, 20, 8, 8, 0, 8, 0, 63, 0, 12, 
	60, 39, 62, 
	18, 18, 37, 
	4, 20, 0, 7, 0, 63, 0, 21, 
	58, 39, 62, 
	10, 37, 37, 
	4, 20, 8, 8, 
	1, 115, 8, 
	22, 37, 37, 
	8, 7, 4, 20, 0, 7, 0, 63, 0, 12, 
	60, 39, 62, 
	18, 37, 37, 
	4, 20, 0, 8, 0, 63, 0, 14, 
	58, 39, 62, 
	9, 10, 29, 
	3, 26, 0, 33, 
	52, 39, 
	5, 1, 60, 
	0, 2, 
	58, 70, 
	7, 8, 5, 
	4, 9, 
	39, 38, 114, 112, 
	7, 8, 28, 
	4, 9, 
	39, 38, 114, 112, 
	7, 8, 5, 
	2, 49, 
	39, 38, 114, 112, 
	0, 49, 0, 
	5, 
	1, 48, 0, 
	39, 109, 
	10, 1, 34, 
	2, 29, 0, 13, 
	54, 76, 39, 
	1, 61, 0, 
	129, 109, 
	1, 16, 0, 
	36, 13, 
	5, 37, 37, 
	2, 63, 
	35, 36, 
	0, 10, 14, 
	66, 
	0, 52, 0, 
	128, 
	0, 8, 70, 
	66, 
	4, 8, 43, 
	12, 17, 
	13, 
	4, 27, 0, 
	4, 1, 
	119, 
	10, 27, 0, 
	0, 3, 0, 1, 
	81, 82, 73, 
	202, 
	16, 4, 0, 3, 
	20, 11, 81, 
	201, 
	15, 4, 0, 3, 
	120, 81, 
	5, 10, 74, 
	2, 69, 
	131, 1, 
	4, 45, 29, 
	12, 33, 
	127, 
	7, 45, 22, 
	3, 21, 
	122, 88, 88, 132, 
	5, 45, 0, 
	6, 21, 
	13, 121, 
	5, 45, 0, 
	3, 21, 
	1, 102, 
	0, 37, 44, 
	123, 
	0, 34, 44, 
	1, 
	4, 1, 12, 
	4, 3, 
	124, 
	4, 60, 54, 
	2, 62, 
	39, 
	0, 24, 0, 
	125, 
	0, 31, 0, 
	1, 
	1, 1, 52, 
	1, 126, 
	5, 8, 19, 
	4, 21, 
	35, 36, 
	5, 8, 76, 
	3, 70, 
	138, 38, 
	14, 1, 9, 
	2, 1, 0, 3, 6, 35, 
	54, 76, 39, 
	0, 62, 0, 
	130, 
	23, 58, 24, 
	3, 52, 3, 16, 3, 53, 0, 52, 0, 67, 
	72, 76, 49, 63, 
	14, 1, 9, 
	4, 3, 0, 2, 6, 35, 
	54, 76, 39, 
	1, 65, 0, 
	39, 85, 
	0, 67, 33, 
	39, 
	0, 68, 0, 
	13, 
	0, 69, 0, 
	39, 
	0, 58, 0, 
	131, 
	1, 71, 0, 
	39, 38, 
	18, 18, 43, 
	1, 17, 5, 3, 0, 17, 0, 69, 
	59, 53, 133, 
	14, 18, 43, 
	1, 17, 2, 3, 0, 17, 
	39, 53, 134, 
	4, 8, 15, 
	3, 5, 
	6, 
	0, 41, 0, 
	116, 
	4, 8, 78, 
	3, 71, 
	105, 
	4, 8, 17, 
	3, 51, 
	105, 
	0, 30, 0, 
	66, 
	1, 8, 0, 
	76, 38, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84,
71, 79, 32,
210, 85, 78,
215, 65, 76,
197, 78, 84,
213, 83, 69,
81, 85, 73,
85, 78, 67,
76, 79, 79,
197, 88, 65,
71, 69, 84,
212, 65, 75,
208, 73, 67,
195, 65, 84,
210, 69, 77,
205, 79, 86,
79, 85, 84,
197, 88, 73,
80, 85, 84,
196, 82, 79,
204, 69, 65,
204, 79, 87,
75, 73, 76,
193, 84, 84,
83, 65, 86,
83, 77, 79,
70, 73, 78,
72, 69, 76,
67, 76, 73,
76, 79, 67,
73, 78, 86,
84, 73, 69,
85, 78, 84,
70, 76, 89,
67, 76, 79,
211, 72, 85,
46, 32, 32,
79, 80, 69,
204, 73, 70,
210, 65, 73,
83, 76, 69,
82, 69, 65,
84, 79, 32,
85, 78, 76,
197, 88, 84,
76, 73, 71,
194, 85, 82,
201, 71, 78,
74, 85, 77,
87, 65, 73,
69, 77, 80,
211, 80, 73,
67, 85, 84,
194, 82, 69,
198, 73, 76,
212, 82, 73,
80, 85, 76,
210, 73, 78,
69, 65, 84,
196, 82, 73,
70, 76, 85,
87, 73, 84,
74, 65, 77,
211, 77, 65,
194, 85, 83,
83, 65, 89,
217, 69, 76,
77, 65, 75,
83, 67, 79,
83, 77, 69,
211, 78, 73,
70, 69, 69,
212, 79, 85,
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
83, 72, 69,
197, 78, 68,
87, 73, 78,
194, 79, 88,
204, 69, 68,
80, 79, 76,
198, 76, 65,
73, 78, 86,
67, 79, 65,
82, 79, 79,
78, 79, 84,
80, 73, 84,
68, 79, 79,
84, 79, 82,
80, 65, 67,
77, 65, 84,
67, 76, 73,
83, 84, 65,
66, 65, 84,
214, 65, 77,
196, 82, 65,
67, 65, 83,
67, 73, 71,
71, 65, 82,
68, 65, 73,
80, 65, 83,
66, 69, 68,
67, 79, 70,
204, 73, 68,
80, 79, 82,
68, 85, 77,
77, 65, 76,
70, 73, 76,
206, 65, 73,
66, 79, 84,
194, 76, 79,
77, 73, 82,
79, 86, 69,
76, 69, 78,
87, 65, 84,
86, 73, 65,
80, 79, 83,
195, 65, 82,
71, 65, 84,
195, 82, 79,
86, 69, 78,
84, 65, 66,
84, 79, 73,
82, 73, 78,
76, 79, 67,
65, 82, 79,
66, 69, 76,
76, 69, 84,
83, 76, 69,
66, 79, 76,
72, 79, 76,
71, 65, 77,
80, 73, 76,
70, 73, 83,
200, 65, 78,
198, 79, 79,
198, 69, 69,
72, 69, 76,
78, 69, 67,
194, 73, 84,
75, 69, 89,
84, 82, 69,
71, 76, 65,
68, 82, 69,
68, 85, 83,
84, 65, 67,
77, 69, 77,
32, 32, 32,
	0,
};
const uint8_t automap[] = {
83, 72, 69,
	0,
80, 73, 76,
	3,
67, 79, 65,
	5,
84, 65, 66,
	8,
84, 79, 82,
	9,
67, 76, 73,
	15,
83, 84, 65,
	16,
77, 73, 82,
	17,
66, 79, 84,
	18,
66, 79, 84,
	19,
84, 79, 82,
	20,
77, 65, 84,
	21,
84, 65, 66,
	23,
84, 65, 66,
	24,
80, 65, 67,
	26,
67, 73, 71,
	27,
71, 65, 82,
	31,
67, 73, 71,
	33,
83, 72, 69,
	35,
87, 65, 84,
	37,
70, 73, 76,
	41,
86, 73, 65,
	42,
80, 65, 67,
	45,
87, 73, 78,
	46,
80, 79, 83,
	47,
78, 79, 84,
	51,
66, 65, 84,
	52,
77, 65, 76,
	53,
80, 79, 82,
	58,
68, 65, 73,
	61,
76, 69, 84,
	65,
75, 69, 89,
	68,
68, 85, 83,
	70,
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
