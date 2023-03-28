#define NUM_OBJ 80
#define WORDSIZE 4
#define GAME_MAGIC 184
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
const uint8_t startlamp = 100;
const uint8_t lightfill = 100;
const uint8_t startcarried = 0;
const uint8_t maxcar = 7;
const uint8_t treasure = 0;
const uint8_t treasures = 0;
const uint8_t lastloc = 37;
const uint8_t startloc = 4;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x52\x6F\x79\x61\x6C\x20\x70\x61\x6C\x61\x63\x65",
 { 0, 0, 2, 0, 0, 0 } }, 
		{ 	"\x50\x65\x72\x73\x69\x61\x6E\x20\x63\x69\x74\x79",
 { 0, 3, 0, 0, 0, 0 } }, 
		{ 	"\x50\x65\x72\x73\x69\x61\x6E\x20\x63\x69\x74\x79",
 { 2, 0, 4, 5, 0, 0 } }, 
		{ 	"\x50\x65\x72\x73\x69\x61\x6E\x20\x63\x69\x74\x79",
 { 0, 0, 0, 3, 0, 0 } }, 
		{ 	"\x2A\x49\x20\x61\x6D\x20\x61\x74\x20\x61\x20\x50\x65\x72\x73\x69\x61\x6E\x20\x62\x61\x79",
 { 0, 0, 3, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x20\x61\x6D\x20\x6F\x6E\x20\x61\x20\x6C\x61\x72\x67\x65\x20\x73\x68\x69\x70",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x61\x6E\x64\x79\x20\x62\x65\x61\x63\x68",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x64\x65\x6E\x73\x65\x20\x6A\x75\x6E\x67\x6C\x65",
 { 10, 9, 9, 7, 0, 0 } }, 
		{ 	"\x64\x65\x6E\x73\x65\x20\x6A\x75\x6E\x67\x6C\x65",
 { 10, 9, 10, 10, 0, 0 } }, 
		{ 	"\x64\x61\x6D\x70\x20\x63\x61\x76\x65",
 { 0, 0, 0, 10, 0, 0 } }, 
		{ 	"\x63\x72\x6F\x77\x27\x73\x2D\x6E\x65\x73\x74",
 { 0, 0, 0, 0, 0, 6 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x69\x73\x6C\x61\x6E\x64",
 { 14, 0, 0, 18, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x69\x73\x6C\x61\x6E\x64",
 { 0, 13, 0, 0, 0, 0 } }, 
		{ 	"\x67\x72\x61\x73\x73\x20\x68\x75\x74",
 { 0, 0, 14, 0, 0, 0 } }, 
		{ 	"\x64\x61\x72\x6B\x20\x63\x61\x76\x65\x72\x6E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E",
 { 0, 0, 0, 0, 0, 14 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x69\x73\x6C\x61\x6E\x64",
 { 0, 0, 13, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x72\x6F\x63\x6B\x79\x20\x73\x74\x72\x61\x6E\x64",
 { 0, 32, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x6C\x6F\x6E\x67\x20\x73\x74\x61\x69\x72\x63\x61\x73\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x61\x63\x72\x65\x64\x20\x74\x65\x6D\x70\x6C\x65",
 { 0, 0, 0, 22, 0, 0 } }, 
		{ 	"\x73\x61\x63\x72\x65\x64\x20\x74\x65\x6D\x70\x6C\x65",
 { 25, 0, 21, 0, 0, 0 } }, 
		{ 	"\x68\x69\x64\x64\x65\x6E\x20\x63\x68\x61\x6D\x62\x65\x72",
 { 22, 0, 0, 24, 0, 0 } }, 
		{ 	"\x68\x69\x64\x64\x65\x6E\x20\x63\x68\x61\x6D\x62\x65\x72",
 { 0, 0, 23, 0, 0, 0 } }, 
		{ 	"\x73\x61\x63\x72\x65\x64\x20\x74\x65\x6D\x70\x6C\x65",
 { 0, 22, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x75\x73\x74\x79\x20\x68\x61\x6C\x6C\x77\x61\x79",
 { 23, 0, 30, 0, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x63\x61\x62\x69\x6E",
 { 0, 0, 0, 0, 6, 0 } }, 
		{ 	"\x63\x6F\x7A\x79\x20\x63\x6F\x74",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x64\x61\x6D\x70\x20\x67\x72\x6F\x74\x74\x6F",
 { 0, 0, 0, 27, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x67\x72\x61\x73\x73\x79\x20\x70\x6C\x61\x69\x6E",
 { 19, 0, 0, 0, 0, 0 } }, 
		{ 	"\x64\x65\x65\x70\x20\x70\x69\x74",
 { 0, 34, 0, 0, 0, 0 } }, 
		{ 	"\x64\x65\x65\x70\x20\x70\x69\x74",
 { 33, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x65\x6E\x6F\x72\x6D\x6F\x75\x73\x20\x63\x61\x76\x65",
 { 34, 0, 0, 36, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x65\x6E\x6F\x72\x6D\x6F\x75\x73\x20\x63\x61\x76\x65",
 { 0, 0, 35, 0, 0, 0 } }, 
		{ 	"\x4C\x4F\x54\x20\x4F\x46\x20\x54\x52\x4F\x55\x42\x4C\x45\x21",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	1,
	1,
	2,
	5,
	6,
	6,
	0,
	13,
	0,
	15,
	11,
	0,
	7,
	7,
	0,
	0,
	10,
	19,
	20,
	0,
	0,
	24,
	0,
	0,
	19,
	0,
	14,
	4,
	3,
	5,
	18,
	4,
	0,
	3,
	3,
	0,
	0,
	0,
	14,
	23,
	25,
	28,
	29,
	30,
	34,
	34,
	36,
	5,
	0,
	0,
	0,
	0,
	8,
	0,
	0,
	0,
	0,
	3,
	22,
	0,
	0,
	0,
	17,
	0,
	0,
	0,
	6,
	36,
	0,
	30,
	0,
	0,
	0,
	0,
	33,
	0,
	6,
	0,
	21,
};


const char *objtext[] = {
	"",
	"\x50\x61\x6C\x61\x63\x65\x20\x67\x75\x61\x72\x64\x73",
	"\x41\x67\x65\x64\x20\x4B\x69\x6E\x67",
	"\x50\x61\x6C\x61\x63\x65",
	"\x4C\x61\x72\x67\x65\x20\x73\x68\x69\x70",
	"\x4D\x61\x73\x74",
	"\x53\x61\x69\x6C",
	"\x41\x6E\x63\x68\x6F\x72",
	"\x53\x6B\x65\x6C\x65\x74\x6F\x6E",
	"\x4C\x69\x74\x20\x74\x6F\x72\x63\x68",
	"\x4F\x6C\x64\x20\x62\x6F\x78",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x66\x6F\x75\x6E\x74\x61\x69\x6E",
	"\x43\x72\x65\x76\x69\x63\x65",
	"\x4D\x61\x6E",
	"\x4A\x75\x6E\x67\x6C\x65",
	"\x48\x69\x64\x64\x65\x6E\x20\x70\x61\x73\x73\x61\x67\x65",
	"\x47\x6F\x6C\x64\x20\x63\x68\x61\x6C\x69\x63\x65",
	"\x43\x61\x76\x65",
	"\x4C\x6F\x6E\x67\x20\x73\x74\x61\x69\x72\x63\x61\x73\x65",
	"\x46\x6C\x69\x6E\x74\x20\x26\x20\x73\x74\x65\x65\x6C",
	"\x47\x6F\x6C\x64\x20\x6D\x61\x73\x6B",
	"\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67\x20\x69\x74",
	"\x53\x6D\x61\x6C\x6C\x20\x73\x74\x6F\x6E\x65",
	"\x53\x6D\x61\x6C\x6C\x20\x73\x74\x6F\x6E\x65",
	"\x53\x6D\x61\x6C\x6C\x20\x73\x74\x6F\x6E\x65",
	"\x53\x74\x6F\x6E\x65\x20\x47\x6F\x64\x64\x65\x73\x73",
	"\x57\x68\x69\x74\x65\x20\x67\x6C\x6F\x62\x65",
	"\x47\x72\x61\x73\x73\x20\x68\x75\x74",
	"\x4D\x65\x72\x63\x68\x61\x6E\x74",
	"\x4D\x65\x72\x63\x68\x61\x6E\x74",
	"\x4D\x65\x72\x63\x68\x61\x6E\x74",
	"\x53\x68\x6F\x76\x65\x6C",
	"\x53\x61\x6E\x64\x61\x6C\x73",
	"\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67\x20\x74\x68\x65\x6D",
	"\x43\x6F\x6D\x70\x61\x73\x73",
	"\x54\x65\x6C\x65\x73\x63\x6F\x70\x65",
	"\x44\x65\x61\x64\x20\x6D\x61\x6E",
	"\x4B\x65\x79",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x72\x75\x62\x62\x6C\x65",
	"\x4D\x6F\x75\x6E\x74\x61\x69\x6E",
	"\x53\x74\x6F\x6E\x65\x20\x62\x6C\x6F\x63\x6B",
	"\x4F\x72\x6E\x61\x74\x65\x20\x63\x68\x65\x73\x74",
	"\x43\x6F\x74",
	"\x4E\x6F\x74\x65",
	"\x44\x65\x65\x70\x20\x70\x69\x74",
	"\x43\x79\x63\x6C\x6F\x70\x73",
	"\x43\x61\x76\x65",
	"\x4D\x61\x67\x6E\x69\x66\x69\x63\x65\x6E\x74\x20\x66\x6F\x75\x6E\x74\x61\x69\x6E",
	"\x4F\x63\x65\x61\x6E",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x72\x75\x62\x62\x6C\x65",
	"",
	"\x4F\x70\x65\x6E\x20\x63\x68\x65\x73\x74",
	"\x59\x6F\x75\x74\x68\x66\x75\x6C\x20\x4B\x69\x6E\x67",
	"\x42\x61\x67\x20\x6F\x66\x20\x67\x6F\x6C\x64",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x67\x6C\x6F\x62\x65",
	"\x53\x63\x6F\x72\x70\x69\x6F\x6E\x73",
	"\x54\x6F\x72\x63\x68",
	"\x43\x72\x6F\x77\x64\x20\x6F\x66\x20\x6D\x6F\x75\x72\x6E\x65\x72\x73",
	"\x53\x74\x6F\x6E\x65\x20\x74\x61\x62\x6C\x65\x74",
	"\x41\x6C\x74\x61\x72",
	"\x53\x63\x6F\x72\x70\x69\x6F\x6E\x20\x73\x74\x69\x6E\x67\x73",
	"\x46\x65\x73\x74\x65\x72\x69\x6E\x67\x20\x73\x63\x6F\x72\x70\x69\x6F\x6E\x20\x73\x74\x69\x6E\x67\x73",
	"\x41\x6E\x69\x6D\x61\x74\x65\x64\x20\x73\x74\x61\x74\x75\x65",
	"\x53\x77\x6F\x72\x64",
	"\x52\x6F\x63\x6B\x79\x20\x73\x74\x72\x61\x6E\x64",
	"\x53\x61\x6E\x64\x79\x20\x62\x65\x61\x63\x68",
	"\x53\x6D\x61\x6C\x6C\x20\x69\x73\x6C\x61\x6E\x64",
	"\x53\x68\x69\x70\x20\x69\x73\x20\x61\x6E\x63\x68\x6F\x72\x65\x64",
	"\x53\x74\x6F\x6E\x65\x20\x74\x61\x62\x6C\x65\x74",
	"\x42\x6C\x69\x6E\x64\x20\x63\x79\x63\x6C\x6F\x70\x73",
	"\x53\x74\x61\x6C\x61\x67\x6D\x69\x74\x65",
	"\x52\x6F\x70\x65",
	"\x52\x6F\x70\x65\x20\x74\x69\x65\x64\x20\x74\x6F\x20\x73\x74\x61\x6C\x61\x67\x6D\x69\x74\x65",
	"\x4F\x74\x68\x65\x72\x20\x65\x6E\x64\x20\x6F\x66\x20\x74\x68\x65\x20\x72\x6F\x70\x65",
	"\x52\x6F\x70\x65\x20\x6C\x65\x61\x64\x69\x6E\x67\x20\x64\x6F\x77\x6E\x20\x69\x6E\x74\x6F\x20\x70\x69\x74",
	"\x52\x6F\x70\x65\x20\x6C\x65\x61\x64\x69\x6E\x67\x20\x6F\x75\x74\x20\x6F\x66\x20\x74\x68\x65\x20\x70\x69\x74",
	"\x44\x61\x72\x6B\x20\x68\x61\x6C\x6C\x77\x61\x79",
	"\x43\x61\x62\x69\x6E",
	"\x50\x65\x72\x73\x69\x61\x6E\x20\x63\x69\x74\x79",
	"\x4C\x6F\x6E\x67\x20\x73\x74\x61\x69\x72\x63\x61\x73\x65",
};
const char *msgptr[] = {
	"",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x31\x32\x3A\x20\x22\x54\x48\x45\x20\x47\x4F\x4C\x44\x45\x4E\x20\x56\x4F\x59\x41\x47\x45\x22\x0A\x62\x79\x20\x57\x69\x6C\x6C\x69\x61\x6D\x20\x44\x65\x6D\x61\x73\x20\x26\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x0A\x44\x65\x64\x69\x63\x61\x74\x65\x64\x3A\x20\x42\x72\x69\x74\x69\x73\x68\x20\x62\x61\x6E\x64\x20\x22\x51\x55\x45\x45\x4E\x22",
	"\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21",
	"\x57\x68\x65\x72\x65\x3F",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x72\x61\x74\x74\x6C\x65\x73",
	"\x53\x74\x61\x74\x75\x65\x20\x74\x6F\x70\x70\x6C\x65\x73\x20\x64\x6F\x77\x6E\x20\x74\x68\x65\x20\x73\x74\x61\x69\x72\x73",
	"\x49\x20\x66\x6F\x75\x6E\x64",
	"\x6E\x6F\x74\x68\x69\x6E\x67\x2E",
	"\x41\x70\x70\x65\x61\x72\x73\x20\x74\x6F\x20\x62\x65\x20\x62\x72\x6F\x6B\x65\x6E\x2C\x20\x77\x69\x74\x68\x20\x73\x74\x72\x61\x6E\x67\x65\x20\x6D\x61\x72\x6B\x69\x6E\x67\x73",
	"\x54\x68\x65\x20\x73\x74\x6F\x6E\x65\x73\x20\x6D\x79\x73\x74\x65\x72\x69\x6F\x75\x73\x6C\x79\x20\x75\x6E\x69\x74\x65\x21",
	"\x43\x6F\x6E\x74\x61\x69\x6E\x73\x20\x6D\x65\x64\x69\x63\x69\x6E\x65",
	"\x45\x76\x65\x72\x79\x74\x68\x69\x6E\x67\x20\x6C\x6F\x6F\x6B\x73\x20\x67\x72\x65\x79",
	"\x4F\x4B",
	"\x4F\x55\x43\x48\x21",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C",
	"\x48\x65\x27\x73\x20\x62\x65\x65\x6E\x20\x73\x74\x75\x6E\x67\x20\x62\x79\x20\x73\x63\x6F\x72\x70\x69\x6F\x6E\x73",
	"\x4C\x41\x4E\x44\x20\x48\x4F\x21",
	"\x49\x20\x68\x65\x61\x72",
	"\x4F\x6E\x65\x20\x67\x72\x65\x61\x74\x20\x62\x69\x67\x20\x45\x59\x45\x20\x68\x61\x73\x20\x61\x20\x66\x6F\x63\x75\x73\x20\x69\x6E\x20\x6D\x79\x20\x64\x69\x72\x65\x63\x74\x69\x6F\x6E\x21",
	"\x50\x69\x63\x74\x75\x72\x65\x20\x6F\x66\x20\x61\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E\x20\x61\x6E\x64\x20\x74\x68\x65\x20\x77\x6F\x72\x64\x20\x22\x53\x55\x4E\x22",
	"\x49\x20\x63\x61\x6E\x27\x74\x20\x74\x65\x6C\x6C\x20\x77\x68\x69\x63\x68\x20\x77\x61\x79\x20\x74\x68\x61\x74\x20\x69\x73\x21",
	"\x4D\x65\x72\x63\x68\x61\x6E\x74\x20\x63\x61\x6C\x6C\x73\x20\x6D\x65\x20\x61\x20\x74\x68\x69\x65\x66\x20\x61\x6E\x64\x20\x73\x6C\x69\x74\x73\x20\x6D\x79\x20\x74\x68\x72\x6F\x61\x74\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x74\x72\x65\x6D\x65\x6E\x64\x6F\x75\x73\x20\x66\x6C\x61\x73\x68\x20\x6F\x66\x20\x6C\x69\x67\x68\x74\x21",
	"\x49\x27\x6D\x20\x62\x6C\x69\x6E\x64\x21",
	"\x48\x65\x20\x67\x69\x76\x65\x73\x20\x61\x20\x67\x72\x65\x61\x74\x20\x62\x69\x67\x20\x43\x52\x59",
	"\x48\x69\x67\x68\x20\x61\x74\x6F\x70\x20\x6D\x61\x73\x74\x20\x69\x73\x20\x61\x20\x63\x72\x6F\x77\x27\x73\x2D\x6E\x65\x73\x74",
	"\x4B\x49\x4E\x47\x3A\x20\x49\x20\x68\x61\x76\x65\x20\x62\x65\x65\x6E\x20\x67\x69\x76\x65\x6E\x20\x6F\x6E\x6C\x79\x20\x33\x20\x64\x61\x79\x73\x20\x74\x6F\x20\x6C\x69\x76\x65\x2E\x20\x59\x6F\x75\x20\x6D\x75\x73\x74\x20\x73\x65\x65\x6B\x20\x66\x6F\x72\x0A\x20\x20\x20\x20\x20\x20\x61\x20\x77\x61\x79\x20\x74\x6F\x20\x72\x65\x73\x74\x6F\x72\x65\x20\x6D\x79\x20\x76\x69\x74\x61\x6C\x69\x74\x79\x2E\x20\x54\x68\x65\x20\x67\x6F\x6C\x64\x20\x69\x73\x20\x79\x6F\x75\x72\x73\x2E\x20\x47\x6F\x20\x6E\x6F\x77\x21",
	"\x4F\x48\x20\x4E\x4F\x21\x20\x49\x20\x63\x61\x6E\x27\x74\x20\x73\x77\x69\x6D\x21",
	"\x43\x79\x63\x6C\x6F\x70\x73",
	"\x49\x74\x27\x73\x20\x6C\x6F\x63\x6B\x65\x64",
	"\x50\x69\x63\x74\x75\x72\x65\x20\x6F\x66\x20\x61\x20\x63\x61\x76\x65",
	"\x67\x72\x61\x62\x73\x20\x6D\x65\x20\x61\x6E\x64\x20\x65\x61\x74\x73\x20\x6D\x65\x20\x61\x6C\x69\x76\x65\x21",
	"\x53\x74\x61\x74\x75\x65\x20\x63\x6F\x6D\x65\x73\x20\x74\x6F\x20\x6C\x69\x66\x65\x21",
	"\x49\x74\x20\x68\x6F\x6C\x64\x73\x20\x61\x20\x73\x77\x6F\x72\x64",
	"\x53\x6F\x72\x72\x79\x2C\x20\x69\x74\x20\x77\x6F\x6E\x27\x74\x20\x42\x55\x44\x47\x45\x21",
	"\x54\x65\x6C\x6C\x20\x6D\x65\x20\x68\x6F\x77\x3F",
	"\x4D\x41\x4E\x3A\x20\x49\x27\x6D\x20\x61\x66\x72\x61\x69\x64\x20\x69\x74\x27\x73\x20\x74\x6F\x6F\x20\x6C\x61\x74\x65\x20\x66\x6F\x72\x20\x6D\x65\x2E\x20\x41\x70\x70\x65\x61\x6C\x20\x74\x6F\x20\x74\x68\x65\x0A\x20\x20\x20\x20\x20\x47\x6F\x64\x73\x20\x66\x6F\x72\x20\x68\x65\x6C\x70\x21",
	"\x54\x68\x65\x20\x67\x72\x6F\x75\x6E\x64\x20\x73\x68\x61\x6B\x65\x73\x20\x76\x69\x6F\x6C\x65\x6E\x74\x6C\x79\x21",
	"\x49\x27\x76\x65\x20\x64\x65\x73\x65\x63\x72\x61\x74\x65\x64\x20\x74\x68\x65\x20\x68\x6F\x6C\x79\x20\x66\x6F\x75\x6E\x74\x61\x69\x6E\x2E\x0A\x49\x27\x6D\x20\x73\x74\x72\x75\x63\x6B\x20\x64\x6F\x77\x6E\x20\x62\x79\x20\x61\x20\x54\x48\x55\x4E\x44\x45\x52\x42\x4F\x4C\x54\x21",
	"\x4B\x69\x6E\x67\x20\x67\x72\x61\x62\x73\x20\x69\x74\x2C",
	"\x61\x6E\x64\x20\x44\x49\x45\x53\x21\x20\x47\x75\x61\x72\x64\x73\x20\x68\x61\x76\x65\x20\x6D\x65\x20\x62\x65\x68\x65\x61\x64\x65\x64",
	"\x49\x20\x68\x61\x76\x65\x20\x66\x61\x69\x6C\x65\x64\x20\x6D\x79\x20\x6D\x69\x73\x73\x69\x6F\x6E",
	"\x49\x74\x27\x73\x20\x67\x65\x74\x74\x69\x6E\x67\x20\x64\x61\x72\x6B\x21",
	"\x4D\x6F\x72\x6E\x69\x6E\x67\x21\x20\x49\x20\x68\x61\x76\x65",
	"\x47\x6F\x6F\x64\x6E\x69\x67\x68\x74",
	"\x32\x20\x64\x61\x79\x73",
	"\x31\x20\x64\x61\x79",
	"\x6C\x65\x66\x74\x20\x74\x6F\x20\x63\x6F\x6D\x70\x6C\x65\x74\x65\x20\x6D\x79\x20\x6D\x69\x73\x73\x69\x6F\x6E",
	"\x54\x68\x65\x20\x73\x75\x6E\x20\x68\x61\x73\x20\x73\x65\x74",
	"\x4E\x69\x67\x68\x74\x20\x61\x69\x72\x20\x69\x73\x20\x43\x4F\x4C\x44\x21\x0A\x49\x20\x63\x61\x74\x63\x68\x20\x70\x6E\x65\x75\x6D\x6F\x6E\x69\x61\x20\x61\x6E\x64\x20\x64\x69\x65",
	"\x66\x61\x69\x6C\x65\x64\x20\x6D\x79\x20\x6D\x69\x73\x73\x69\x6F\x6E",
	"\x49\x27\x6D\x20\x69\x6E\x20\x67\x72\x65\x61\x74\x20\x50\x41\x49\x4E\x21",
	"\x4D\x79\x20\x62\x6F\x64\x79\x20\x68\x61\x73\x20\x62\x65\x63\x6F\x6D\x65\x20\x69\x6E\x66\x65\x63\x74\x65\x64",
	"\x53\x74\x61\x74\x75\x65\x20\x66\x6F\x6C\x6C\x6F\x77\x73\x20\x6D\x65",
	"\x53\x74\x61\x74\x75\x65\x20\x73\x77\x69\x6E\x67\x73\x20\x69\x74\x73\x20\x73\x77\x6F\x72\x64",
	"\x49\x74\x20\x6D\x69\x73\x73\x65\x64\x20\x6D\x65\x21",
	"\x49\x27\x76\x65\x20\x62\x65\x65\x6E\x20\x63\x75\x74\x20\x69\x6E\x20\x74\x77\x6F\x21",
	"\x49\x20\x66\x65\x6E\x64\x20\x6F\x66\x66\x20\x69\x74\x73\x20\x61\x74\x74\x61\x63\x6B\x2E",
	"\x49\x27\x6D\x20\x6C\x79\x6E\x63\x68\x65\x64\x20\x62\x79\x20\x61\x6E\x67\x72\x79\x20\x6D\x6F\x75\x72\x6E\x65\x72\x73\x2E\x0A\x54\x68\x65\x69\x72\x20\x4B\x69\x6E\x67\x20\x69\x73\x20\x64\x65\x61\x64\x2E\x20\x28\x41\x4E\x44\x20\x53\x4F\x20\x41\x4D\x20\x49\x29\x21\x21",
	"\x4D\x61\x6E\x20\x69\x73\x20\x64\x65\x61\x64\x21",
	"\x47\x65\x74\x20\x79\x6F\x75\x72\x20\x63\x6F\x70\x79\x20\x6F\x66\x20\x22\x46\x52\x4F\x47\x22\x20\x66\x72\x6F\x6D\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x49\x6E\x74\x65\x72\x6E\x61\x74\x69\x6F\x6E\x61\x6C\x20\x74\x6F\x64\x61\x79\x21",
	"\x53\x68\x69\x70\x20\x64\x72\x69\x66\x74\x73\x20\x61\x77\x61\x79",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x73",
	"\x49\x74\x27\x73\x20\x66\x75\x6C\x6C\x20\x6F\x66",
	"\x77\x61\x74\x65\x72",
	"\x61\x20\x73\x74\x72\x61\x6E\x67\x65\x20\x6C\x69\x71\x75\x69\x64",
	"\x61\x6E\x64\x20\x74\x72\x61\x6E\x73\x66\x6F\x72\x6D\x73\x20\x69\x6E\x74\x6F\x20\x61\x20\x79\x6F\x75\x6E\x67\x20\x6D\x61\x6E\x2E\x20\x43\x4F\x4E\x47\x52\x41\x54\x55\x4C\x41\x54\x49\x4F\x4E\x53\x21\x21\x0A\x4D\x69\x73\x73\x69\x6F\x6E\x20\x61\x63\x63\x6F\x6D\x70\x6C\x69\x73\x68\x65\x64\x2E",
	"\x64\x72\x69\x6E\x6B\x73\x20\x74\x68\x65\x20\x6C\x69\x71\x75\x69\x64\x2C",
	"\x61\x6E\x64\x20\x79\x65\x6C\x6C\x73\x20\x22\x49\x74\x27\x73\x20\x45\x4D\x50\x54\x59\x21\x22\x2E",
	"\x77\x6F\x6E\x27\x74\x20\x6C\x65\x74\x20\x6D\x65",
	"\x43\x79\x63\x6C\x6F\x70\x73\x20\x73\x74\x61\x67\x67\x65\x72\x73\x20\x61\x6E\x64\x20\x73\x74\x65\x70\x73\x20\x6F\x6E\x20\x6D\x65\x21",
	"\x49\x27\x6C\x6C\x20\x6F\x6E\x6C\x79\x20\x64\x65\x66\x65\x6E\x64\x20\x6D\x79\x73\x65\x6C\x66",
	"\x4D\x65\x72\x63\x68\x61\x6E\x74",
	"\x50\x6C\x65\x61\x73\x65\x20\x62\x65\x20\x6D\x6F\x72\x65\x20\x73\x70\x65\x63\x69\x66\x69\x63",
	"\x49\x20\x66\x65\x65\x6C\x20\x6D\x75\x63\x68\x20\x62\x65\x74\x74\x65\x72",
	"\x45\x6D\x70\x74\x79\x3F",
	"\x53\x6F\x72\x72\x79",
	"\x48\x65\x20\x68\x61\x73\x20\x6D\x65\x20\x65\x78\x65\x63\x75\x74\x65\x64",
	"\x53\x75\x72\x70\x72\x69\x73\x65\x64\x20\x6D\x65\x72\x63\x68\x61\x6E\x74\x20\x67\x72\x61\x62\x73\x20\x69\x74\x20\x61\x6E\x64\x20\x72\x75\x6E\x73\x20\x6C\x69\x6B\x65\x20\x61\x20\x74\x68\x69\x65\x66\x21",
	"\x53\x74\x61\x74\x75\x65\x20\x61\x74\x74\x61\x63\x6B\x73\x20\x77\x68\x69\x6C\x65\x20\x49\x20\x73\x6C\x65\x65\x70\x21",
	"\x52\x6F\x70\x65\x20\x73\x6C\x69\x64\x65\x73\x20\x69\x6E\x74\x6F\x20\x70\x69\x74",
	"\x50\x4C\x4F\x4F\x53\x48",
	"\x59\x55\x43\x4B\x21",
};


const uint8_t status[] = {
	175, 
	9, 31, 0, 3, 0, 19, 
	1, 79, 58, 73, 
	211, 
	0, 0, 0, 215, 0, 31, 0, 17, 
	81, 79, 58, 58, 
	160, 
	77, 
	168, 
	2, 4, 0, 48, 
	53, 
	164, 
	19, 0, 
	48, 
	168, 
	12, 9, 15, 0, 
	56, 
	171, 
	8, 14, 0, 16, 
	58, 56, 64, 23, 
	172, 
	4, 22, 9, 14, 0, 8, 
	60, 
	176, 
	3, 9, 8, 16, 0, 9, 0, 56, 
	72, 
	183, 
	8, 29, 13, 62, 4, 29, 0, 29, 0, 62, 
	129, 60, 61, 55, 
	168, 
	3, 9, 9, 14, 
	57, 
	165, 
	8, 29, 
	88, 73, 
	199, 
	7, 29, 
	49, 61, 41, 63, 
	166, 
	8, 29, 
	88, 57, 73, 
	207, 
	0, 215, 0, 0, 0, 1, 
	79, 81, 83, 43, 
	175, 
	8, 29, 19, 2, 0, 29, 
	45, 47, 60, 81, 
	175, 
	8, 29, 19, 1, 0, 29, 
	46, 47, 60, 81, 
	182, 
	8, 29, 15, 0, 0, 0, 0, 57, 0, 2, 
	50, 81, 62, 
	150, 50, 
	6, 33, 6, 61, 6, 60, 3, 55, 0, 60, 
	13, 74, 73, 
	211, 
	0, 3, 0, 10, 0, 3, 0, 23, 
	81, 79, 81, 58, 
	170, 
	8, 23, 0, 3, 
	81, 77, 73, 
	210, 
	19, 5, 0, 60, 0, 61, 0, 3, 
	51, 72, 73, 
	198, 
	15, 0, 
	102, 61, 73, 
	200, 
	8, 23, 0, 3, 
	81, 
	173, 
	5, 62, 13, 62, 0, 62, 
	103, 53, 
	139, 45, 
	2, 62, 0, 25, 
	104, 88, 88, 58, 
	145, 40, 
	14, 62, 2, 25, 0, 25, 0, 62, 
	32, 72, 
	141, 70, 
	8, 25, 1, 63, 0, 25, 
	107, 60, 
	139, 39, 
	8, 25, 0, 25, 
	106, 57, 61, 60, 
	169, 
	8, 25, 0, 25, 
	105, 60, 
	143, 60, 
	8, 18, 2, 45, 0, 26, 
	24, 88, 88, 58, 
	133, 45, 
	8, 26, 
	31, 61, 
	168, 
	8, 26, 0, 26, 
	60, 
	168, 
	8, 29, 0, 29, 
	60, 
	167, 
	2, 57, 
	88, 88, 108, 73, 
	193, 
	61, 63, 
	183, 
	4, 6, 0, 64, 0, 65, 0, 66, 0, 78, 
	59, 59, 59, 59, 
	183, 
	14, 58, 14, 68, 9, 9, 0, 30, 0, 9, 
	37, 58, 58, 73, 
	210, 
	0, 38, 0, 40, 0, 76, 0, 23, 
	72, 62, 131, 
	137, 45, 
	9, 6, 2, 69, 
	120, 61, 
	168, 
	13, 69, 0, 6, 
	60, 
	183, 
	2, 22, 2, 23, 2, 24, 0, 22, 0, 23, 
	9, 59, 59, 73, 
	201, 
	0, 24, 0, 68, 
	59, 53, 
	169, 
	12, 9, 8, 8, 
	56, 64, 
	168, 
	15, 10, 16, 0, 
	42, 
	169, 
	4, 16, 12, 9, 
	56, 64, 
	173, 
	4, 14, 16, 0, 9, 14, 
	57, 64, 
	141, 60, 
	9, 19, 2, 4, 0, 4, 
	111, 55, 
	176, 
	8, 0, 4, 6, 8, 22, 0, 64, 
	53, 
	169, 
	12, 9, 8, 15, 
	56, 64, 
	167, 
	4, 37, 
	57, 64, 41, 63, 
	176, 
	8, 0, 4, 12, 8, 22, 0, 64, 
	53, 
	176, 
	8, 1, 4, 6, 8, 20, 0, 66, 
	53, 
	176, 
	8, 1, 4, 12, 8, 20, 0, 66, 
	53, 
	176, 
	8, 5, 4, 6, 8, 28, 0, 65, 
	53, 
	176, 
	8, 5, 4, 12, 8, 28, 0, 65, 
	53, 
	169, 
	9, 15, 9, 14, 
	57, 64, 
	178, 
	4, 10, 16, 0, 9, 14, 0, 8, 
	57, 64, 60, 
	172, 
	4, 6, 8, 17, 0, 78, 
	53, 
	172, 
	4, 12, 8, 17, 0, 78, 
	53, 
	173, 
	4, 1, 17, 53, 0, 53, 
	26, 53, 
	168, 
	4, 34, 0, 18, 
	58, 
	169, 
	4, 22, 16, 0, 
	57, 64, 
	176, 
	4, 34, 1, 73, 0, 73, 0, 33, 
	62, 
	176, 
	4, 27, 1, 73, 0, 73, 0, 30, 
	62, 
};
const uint8_t actions[] = {
	5, 1, 45, 
	4, 30, 
	12, 35, 
	6, 1, 47, 
	2, 11, 
	12, 38, 61, 
	23, 1, 63, 
	2, 64, 4, 6, 0, 19, 0, 55, 0, 4, 
	12, 54, 53, 53, 
	23, 1, 64, 
	2, 65, 4, 6, 0, 7, 0, 55, 0, 4, 
	12, 54, 53, 53, 
	23, 1, 65, 
	2, 66, 4, 6, 0, 13, 0, 55, 0, 4, 
	12, 54, 53, 53, 
	23, 1, 67, 
	4, 6, 2, 64, 0, 19, 0, 55, 0, 4, 
	12, 54, 53, 53, 
	23, 1, 67, 
	4, 6, 2, 65, 0, 7, 0, 55, 0, 4, 
	12, 54, 53, 53, 
	23, 1, 67, 
	4, 6, 2, 66, 0, 13, 0, 55, 0, 4, 
	12, 54, 53, 53, 
	9, 1, 72, 
	2, 74, 0, 33, 
	12, 54, 
	9, 1, 72, 
	2, 75, 0, 30, 
	12, 54, 
	15, 1, 21, 
	2, 15, 0, 23, 0, 8, 
	12, 54, 56, 58, 
	9, 1, 22, 
	2, 76, 0, 27, 
	12, 54, 
	0, 8, 68, 
	71, 
	18, 9, 0, 
	4, 32, 14, 37, 1, 31, 0, 37, 
	6, 2, 53, 
	18, 9, 0, 
	4, 9, 14, 71, 1, 31, 0, 71, 
	6, 2, 53, 
	0, 10, 60, 
	66, 
	9, 1, 9, 
	4, 2, 0, 1, 
	12, 54, 
	19, 10, 16, 
	3, 10, 13, 60, 0, 60, 0, 23, 
	12, 59, 124, 60, 
	19, 10, 16, 
	3, 10, 13, 61, 0, 61, 0, 23, 
	12, 59, 124, 60, 
	5, 10, 16, 
	3, 10, 
	12, 112, 
	1, 10, 18, 
	12, 35, 
	22, 9, 0, 
	4, 10, 14, 56, 14, 9, 1, 31, 0, 56, 
	6, 2, 53, 
	18, 9, 0, 
	4, 10, 14, 24, 1, 31, 0, 24, 
	6, 2, 53, 
	18, 1, 11, 
	2, 4, 8, 2, 0, 6, 0, 48, 
	12, 54, 53, 
	5, 9, 0, 
	1, 31, 
	6, 7, 
	9, 10, 25, 
	2, 22, 0, 22, 
	52, 12, 
	9, 10, 25, 
	2, 23, 0, 23, 
	52, 12, 
	9, 10, 25, 
	2, 24, 0, 24, 
	52, 12, 
	15, 10, 24, 
	2, 58, 9, 6, 0, 58, 
	52, 12, 21, 61, 
	9, 10, 24, 
	2, 58, 0, 58, 
	52, 12, 
	9, 10, 24, 
	2, 68, 0, 68, 
	52, 12, 
	15, 10, 54, 
	2, 35, 9, 7, 0, 35, 
	52, 12, 21, 61, 
	9, 10, 54, 
	2, 35, 0, 35, 
	52, 12, 
	15, 10, 55, 
	2, 34, 9, 3, 0, 34, 
	52, 12, 21, 61, 
	9, 10, 55, 
	2, 34, 0, 34, 
	52, 12, 
	11, 10, 56, 
	2, 32, 9, 4, 
	52, 12, 21, 61, 
	9, 10, 56, 
	2, 32, 0, 32, 
	52, 12, 
	9, 22, 47, 
	2, 11, 9, 30, 
	113, 115, 
	6, 74, 0, 
	0, 6, 
	12, 88, 83, 
	9, 10, 72, 
	2, 71, 0, 71, 
	52, 12, 
	14, 10, 72, 
	2, 74, 0, 74, 0, 73, 
	12, 59, 74, 
	15, 17, 16, 
	2, 13, 0, 13, 0, 36, 
	86, 36, 72, 109, 
	6, 1, 47, 
	4, 36, 
	12, 38, 61, 
	15, 17, 59, 
	4, 1, 1, 16, 8, 10, 
	39, 117, 88, 73, 
	195, 
	88, 112, 127, 61, 
	15, 17, 59, 
	4, 1, 1, 16, 8, 11, 
	39, 117, 88, 73, 
	194, 
	88, 40, 61, 
	15, 17, 59, 
	4, 1, 1, 16, 8, 12, 
	39, 117, 88, 73, 
	203, 
	0, 2, 0, 52, 
	88, 72, 116, 63, 
	10, 17, 59, 
	4, 1, 1, 16, 
	39, 88, 73, 
	195, 
	88, 118, 127, 61, 
	15, 61, 0, 
	4, 22, 5, 15, 0, 15, 
	12, 17, 2, 53, 
	23, 17, 73, 
	3, 53, 4, 5, 0, 53, 0, 2, 0, 30, 
	55, 128, 58, 59, 
	23, 17, 73, 
	3, 53, 4, 4, 0, 53, 0, 4, 0, 28, 
	55, 128, 58, 59, 
	19, 17, 73, 
	3, 53, 4, 3, 0, 53, 0, 6, 
	55, 128, 58, 73, 
	206, 
	0, 3, 0, 7, 0, 29, 
	58, 58, 59, 
	6, 49, 0, 
	0, 29, 
	44, 58, 88, 
	5, 60, 45, 
	4, 30, 
	12, 61, 
	18, 1, 67, 
	4, 6, 2, 78, 0, 5, 0, 4, 
	12, 54, 53, 
	17, 1, 37, 
	2, 78, 4, 6, 0, 5, 0, 4, 
	54, 53, 
	9, 1, 46, 
	4, 6, 0, 28, 
	12, 54, 
	11, 1, 11, 
	2, 4, 9, 2, 
	12, 88, 122, 119, 
	9, 1, 14, 
	4, 6, 0, 12, 
	12, 54, 
	6, 1, 18, 
	2, 48, 
	12, 27, 61, 
	9, 1, 44, 
	4, 7, 0, 9, 
	12, 54, 
	19, 1, 23, 
	4, 10, 0, 11, 0, 8, 0, 11, 
	12, 54, 58, 53, 
	9, 1, 32, 
	4, 14, 0, 15, 
	12, 54, 
	9, 1, 33, 
	4, 14, 0, 17, 
	12, 54, 
	23, 1, 34, 
	2, 12, 4, 14, 0, 16, 0, 12, 0, 11, 
	12, 54, 53, 53, 
	9, 10, 5, 
	4, 29, 0, 28, 
	12, 54, 
	8, 16, 59, 
	1, 16, 8, 10, 
	126, 
	8, 16, 59, 
	1, 16, 8, 11, 
	126, 
	8, 16, 59, 
	1, 16, 8, 12, 
	126, 
	21, 16, 59, 
	2, 11, 1, 16, 9, 10, 9, 12, 0, 11, 
	12, 58, 
	21, 16, 59, 
	2, 48, 1, 16, 9, 11, 9, 12, 0, 10, 
	12, 58, 
	21, 16, 59, 
	2, 47, 1, 16, 9, 10, 9, 11, 0, 12, 
	12, 58, 
	9, 18, 16, 
	1, 10, 0, 10, 
	12, 53, 
	9, 52, 18, 
	1, 16, 8, 11, 
	12, 61, 
	9, 18, 25, 
	1, 22, 0, 22, 
	12, 53, 
	9, 18, 25, 
	1, 23, 0, 23, 
	12, 53, 
	9, 18, 25, 
	1, 24, 0, 24, 
	12, 53, 
	4, 18, 24, 
	1, 58, 
	3, 
	4, 18, 24, 
	1, 68, 
	3, 
	4, 18, 72, 
	1, 73, 
	3, 
	4, 22, 14, 
	4, 6, 
	25, 
	4, 22, 24, 
	3, 68, 
	19, 
	4, 22, 24, 
	3, 58, 
	30, 
	4, 22, 25, 
	3, 24, 
	8, 
	4, 22, 25, 
	3, 23, 
	8, 
	4, 22, 25, 
	3, 22, 
	8, 
	4, 22, 28, 
	2, 13, 
	15, 
	4, 22, 35, 
	3, 10, 
	10, 
	4, 22, 38, 
	2, 25, 
	33, 
	4, 22, 38, 
	2, 62, 
	33, 
	14, 22, 40, 
	4, 22, 14, 16, 0, 16, 
	6, 2, 53, 
	14, 22, 41, 
	3, 51, 14, 20, 0, 20, 
	6, 2, 53, 
	4, 22, 43, 
	2, 45, 
	18, 
	4, 22, 48, 
	3, 43, 
	123, 
	17, 22, 54, 
	1, 35, 4, 12, 8, 22, 14, 64, 
	16, 58, 
	21, 22, 54, 
	1, 35, 4, 12, 8, 28, 14, 65, 0, 5, 
	16, 58, 
	21, 22, 54, 
	1, 35, 4, 12, 8, 20, 14, 66, 0, 1, 
	16, 58, 
	14, 22, 53, 
	3, 49, 14, 23, 0, 23, 
	6, 2, 53, 
	9, 22, 59, 
	3, 16, 8, 10, 
	113, 114, 
	9, 22, 59, 
	3, 16, 8, 11, 
	113, 115, 
	9, 22, 59, 
	3, 16, 8, 12, 
	113, 115, 
	11, 1, 23, 
	4, 34, 13, 45, 
	12, 88, 28, 119, 
	9, 1, 23, 
	4, 34, 0, 35, 
	12, 54, 
	8, 10, 62, 
	2, 25, 5, 63, 
	34, 
	8, 10, 62, 
	2, 62, 5, 63, 
	34, 
	9, 10, 62, 
	2, 63, 0, 63, 
	52, 12, 
	8, 27, 0, 
	4, 6, 2, 67, 
	126, 
	23, 27, 4, 
	4, 6, 8, 17, 3, 34, 0, 17, 0, 20, 
	12, 88, 60, 58, 
	23, 27, 3, 
	4, 6, 8, 17, 3, 34, 0, 17, 0, 27, 
	12, 88, 60, 58, 
	23, 27, 2, 
	4, 6, 8, 17, 3, 34, 0, 17, 0, 22, 
	12, 88, 60, 58, 
	23, 27, 3, 
	4, 6, 8, 20, 3, 34, 0, 20, 0, 17, 
	12, 88, 60, 58, 
	23, 27, 2, 
	4, 6, 8, 20, 3, 34, 0, 20, 0, 21, 
	12, 88, 60, 58, 
	23, 27, 1, 
	4, 6, 8, 21, 3, 34, 0, 21, 0, 20, 
	12, 88, 60, 58, 
	23, 27, 3, 
	4, 6, 8, 21, 3, 34, 0, 21, 0, 22, 
	12, 88, 60, 58, 
	23, 27, 1, 
	4, 6, 8, 22, 3, 34, 0, 22, 0, 17, 
	12, 88, 60, 58, 
	23, 27, 4, 
	4, 6, 8, 22, 3, 34, 0, 22, 0, 21, 
	12, 88, 60, 58, 
	23, 27, 3, 
	4, 6, 8, 27, 3, 34, 0, 27, 0, 28, 
	12, 88, 60, 58, 
	23, 27, 4, 
	4, 6, 8, 27, 3, 34, 0, 27, 0, 17, 
	12, 88, 60, 58, 
	23, 27, 4, 
	4, 6, 8, 28, 3, 34, 0, 28, 0, 27, 
	12, 88, 60, 58, 
	8, 27, 0, 
	4, 6, 12, 34, 
	20, 
	4, 29, 38, 
	2, 25, 
	4, 
	22, 29, 38, 
	2, 62, 4, 20, 0, 62, 0, 49, 0, 19, 
	59, 5, 62, 
	0, 29, 38, 
	112, 
	0, 29, 0, 
	35, 
	9, 66, 70, 
	1, 73, 0, 73, 
	12, 53, 
	19, 18, 57, 
	1, 26, 0, 26, 0, 26, 0, 54, 
	53, 72, 22, 73, 
	202, 
	6, 21, 0, 14, 
	88, 58, 73, 
	205, 
	2, 45, 0, 45, 0, 69, 
	88, 72, 
	17, 33, 41, 
	3, 41, 1, 37, 0, 41, 0, 51, 
	12, 72, 
	21, 35, 52, 
	3, 56, 3, 19, 9, 16, 0, 56, 0, 9, 
	12, 72, 
	0, 38, 0, 
	126, 
	5, 33, 41, 
	3, 41, 
	126, 29, 
	1, 41, 0, 
	126, 121, 
	16, 43, 59, 
	1, 16, 9, 10, 9, 11, 9, 12, 
	112, 
	4, 44, 18, 
	1, 16, 
	125, 
	19, 22, 47, 
	2, 11, 8, 30, 0, 30, 0, 26, 
	6, 2, 60, 53, 
	18, 46, 72, 
	2, 72, 2, 74, 0, 72, 0, 74, 
	59, 59, 130, 
	19, 46, 72, 
	2, 72, 0, 72, 0, 73, 0, 71, 
	55, 55, 53, 12, 
	13, 47, 52, 
	3, 9, 0, 9, 0, 56, 
	12, 72, 
	4, 48, 48, 
	3, 43, 
	110, 
	13, 50, 56, 
	1, 32, 6, 33, 0, 33, 
	12, 74, 
	14, 50, 58, 
	1, 20, 6, 21, 0, 21, 
	12, 74, 11, 
	9, 10, 72, 
	2, 73, 0, 73, 
	12, 74, 
	9, 51, 56, 
	1, 33, 0, 33, 
	12, 55, 
	9, 51, 58, 
	1, 21, 0, 21, 
	12, 55, 
	19, 43, 59, 
	1, 16, 0, 10, 0, 11, 0, 12, 
	12, 60, 60, 60, 
	23, 66, 70, 
	1, 58, 0, 58, 0, 58, 0, 30, 0, 38, 
	12, 53, 62, 53, 
	22, 18, 66, 
	4, 6, 2, 7, 0, 67, 0, 7, 0, 19, 
	12, 72, 58, 
	15, 54, 61, 
	4, 14, 3, 68, 0, 12, 
	12, 85, 37, 53, 
	2, 54, 0, 
	12, 85, 112, 
	1, 58, 0, 
	41, 63, 
	0, 59, 0, 
	66, 
	9, 72, 5, 
	4, 20, 0, 21, 
	12, 54, 
	9, 72, 6, 
	4, 20, 0, 19, 
	12, 54, 
	22, 69, 66, 
	4, 6, 2, 67, 0, 67, 0, 7, 0, 19, 
	12, 72, 60, 
	5, 62, 72, 
	1, 71, 
	12, 3, 
	22, 63, 69, 
	4, 30, 3, 71, 0, 72, 0, 73, 0, 71, 
	12, 53, 72, 
	9, 18, 72, 
	1, 71, 0, 71, 
	12, 53, 
	9, 66, 70, 
	1, 68, 0, 68, 
	12, 53, 
	14, 68, 47, 
	1, 58, 4, 11, 0, 58, 
	12, 59, 131, 
	14, 68, 47, 
	1, 68, 4, 16, 0, 68, 
	12, 59, 131, 
	17, 64, 11, 
	2, 4, 1, 53, 9, 2, 0, 2, 
	12, 58, 
	17, 64, 56, 
	2, 32, 1, 53, 9, 4, 0, 4, 
	12, 58, 
	17, 64, 55, 
	2, 34, 1, 53, 9, 3, 0, 3, 
	12, 58, 
	17, 64, 54, 
	2, 35, 1, 53, 9, 7, 0, 7, 
	12, 58, 
	17, 64, 24, 
	2, 58, 1, 53, 9, 6, 0, 6, 
	12, 58, 
	23, 1, 34, 
	2, 12, 4, 16, 0, 14, 0, 12, 0, 8, 
	12, 54, 53, 60, 
	8, 1, 36, 
	4, 19, 0, 20, 
	54, 
	8, 1, 36, 
	4, 21, 0, 20, 
	54, 
	4, 1, 36, 
	4, 20, 
	123, 
	0, 53, 0, 
	126, 
	14, 68, 45, 
	1, 73, 0, 73, 0, 74, 
	12, 59, 53, 
	1, 61, 0, 
	12, 112, 
	18, 68, 47, 
	1, 58, 4, 16, 0, 58, 0, 8, 
	12, 62, 131, 
	18, 68, 47, 
	1, 68, 4, 11, 0, 68, 0, 8, 
	12, 62, 131, 
	14, 52, 18, 
	1, 16, 8, 10, 0, 10, 
	12, 60, 132, 
	13, 52, 18, 
	1, 16, 8, 12, 0, 12, 
	12, 60, 
	5, 22, 47, 
	4, 36, 
	113, 115, 
	1, 22, 0, 
	76, 14, 
	14, 18, 58, 
	1, 20, 0, 20, 0, 21, 
	12, 53, 55, 
	5, 27, 0, 
	4, 6, 
	12, 88, 
	14, 18, 56, 
	1, 32, 0, 32, 0, 33, 
	12, 53, 55, 
	4, 18, 56, 
	1, 32, 
	51, 
	5, 10, 39, 
	2, 55, 
	132, 126, 
	9, 1, 49, 
	4, 28, 0, 29, 
	12, 54, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
208, 82, 79, 67,
210, 73, 68, 69,
210, 85, 78, 32,
195, 76, 73, 77,
197, 78, 84, 69,
197, 88, 73, 84,
83, 65, 86, 69,
68, 73, 71, 32,
84, 65, 75, 69,
199, 69, 84, 32,
195, 65, 82, 82,
199, 82, 65, 66,
200, 79, 76, 68,
211, 84, 69, 65,
70, 73, 76, 76,
71, 73, 86, 69,
68, 82, 79, 80,
204, 79, 87, 69,
208, 85, 84, 32,
212, 72, 82, 79,
69, 88, 65, 77,
211, 84, 85, 68,
204, 79, 79, 75,
211, 69, 69, 32,
215, 65, 84, 67,
83, 65, 73, 76,
206, 65, 86, 73,
80, 85, 83, 72,
211, 72, 79, 86,
205, 79, 86, 69,
211, 72, 65, 75,
79, 80, 69, 78,
213, 78, 76, 79,
76, 73, 71, 72,
201, 71, 78, 73,
194, 85, 82, 78,
66, 82, 69, 65,
196, 69, 83, 84,
211, 77, 65, 83,
65, 84, 84, 65,
203, 73, 76, 76,
69, 77, 80, 84,
80, 79, 85, 82,
211, 80, 73, 76,
85, 78, 84, 73,
85, 78, 76, 73,
82, 69, 65, 68,
83, 76, 69, 69,
87, 69, 65, 82,
82, 69, 77, 79,
68, 82, 73, 78,
72, 69, 76, 80,
83, 65, 89, 32,
211, 67, 82, 69,
217, 69, 76, 76,
200, 79, 76, 76,
81, 85, 73, 84,
73, 78, 86, 69,
74, 85, 77, 80,
80, 82, 65, 89,
84, 73, 69, 32,
84, 79, 32, 32,
66, 85, 89, 32,
208, 85, 82, 67,
79, 78, 32, 32,
193, 84, 32, 32,
73, 78, 32, 32,
82, 65, 73, 83,
204, 73, 70, 84,
215, 69, 73, 71,
83, 84, 69, 80,
215, 65, 76, 75,
87, 65, 73, 84,
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
75, 73, 78, 71,
71, 85, 65, 82,
80, 65, 76, 65,
77, 69, 82, 67,
83, 72, 73, 80,
194, 79, 65, 84,
193, 66, 79, 65,
77, 65, 83, 84,
83, 65, 73, 76,
77, 69, 68, 73,
65, 82, 79, 85,
79, 67, 69, 65,
215, 65, 84, 69,
204, 73, 81, 85,
80, 65, 83, 83,
72, 65, 76, 76,
67, 65, 86, 69,
84, 65, 66, 76,
83, 84, 79, 78,
83, 75, 69, 76,
194, 79, 78, 69,
77, 65, 78, 32,
70, 76, 73, 78,
211, 84, 69, 69,
83, 72, 79, 86,
72, 85, 84, 32,
77, 79, 85, 78,
67, 82, 69, 86,
66, 79, 88, 32,
83, 84, 65, 73,
67, 73, 84, 89,
83, 84, 65, 84,
83, 67, 79, 82,
65, 76, 84, 65,
67, 72, 69, 83,
75, 69, 89, 32,
67, 89, 67, 76,
74, 85, 78, 71,
80, 73, 84, 32,
67, 65, 66, 73,
70, 79, 85, 78,
78, 79, 84, 69,
67, 79, 84, 32,
77, 65, 82, 75,
66, 76, 79, 67,
84, 79, 82, 67,
82, 85, 66, 66,
84, 69, 76, 69,
67, 79, 77, 80,
83, 65, 78, 68,
71, 76, 79, 66,
77, 65, 83, 75,
67, 72, 65, 76,
73, 78, 86, 69,
83, 85, 78, 32,
83, 87, 79, 82,
83, 84, 82, 65,
66, 69, 65, 67,
73, 83, 76, 65,
65, 78, 67, 72,
65, 83, 72, 79,
71, 65, 77, 69,
83, 84, 65, 76,
71, 82, 79, 85,
198, 76, 79, 79,
82, 79, 80, 69,
66, 65, 71, 32,
199, 79, 76, 68,
	0,
};
const uint8_t automap[] = {
83, 75, 69, 76,
	8,
84, 79, 82, 67,
	9,
66, 79, 88, 32,
	10,
67, 72, 65, 76,
	16,
70, 76, 73, 78,
	19,
77, 65, 83, 75,
	20,
71, 76, 79, 66,
	26,
83, 72, 79, 86,
	31,
83, 65, 78, 68,
	32,
67, 79, 77, 80,
	34,
84, 69, 76, 69,
	35,
75, 69, 89, 32,
	37,
82, 85, 66, 66,
	38,
67, 72, 69, 83,
	41,
78, 79, 84, 69,
	43,
82, 85, 66, 66,
	49,
67, 72, 69, 83,
	51,
66, 65, 71, 32,
	53,
84, 79, 82, 67,
	56,
83, 87, 79, 82,
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
