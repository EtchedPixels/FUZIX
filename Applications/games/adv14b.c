#define NUM_OBJ 61
#define WORDSIZE 4
#define GAME_MAGIC 341
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
const uint8_t startlamp = 35;
const uint8_t lightfill = 35;
const uint8_t startcarried = 0;
const uint8_t maxcar = 5;
const uint8_t treasure = 0;
const uint8_t treasures = 0;
const uint8_t lastloc = 35;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"\x53\x74\x6F\x72\x65\x72\x6F\x6F\x6D\x2C\x20\x63\x61\x6E\x27\x74\x20\x67\x65\x74\x20\x68\x65\x72\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x59\x6F\x79\x6F\x64\x79\x6E\x65\x20\x6F\x66\x66\x69\x63\x65",
 { 0, 0, 0, 3, 0, 0 } }, 
		{ 	"\x42\x61\x73\x65\x6D\x65\x6E\x74\x20\x4C\x61\x62\x6F\x72\x61\x74\x6F\x72\x79",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x74\x68\x65\x20\x63\x6F\x72\x6E\x65\x72\x20\x6F\x66\x20\x4D\x61\x69\x6E\x20\x26\x20\x48\x69\x63\x6B\x6F\x72\x79",
 { 4, 5, 6, 7, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x74\x68\x65\x20\x63\x6F\x72\x6E\x65\x72\x20\x6F\x66\x20\x4D\x61\x69\x6E\x20\x26\x20\x4D\x6F\x75\x6E\x74\x61\x69\x6E\x20\x56\x69\x65\x77",
 { 18, 3, 28, 28, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x61\x20\x68\x69\x67\x68\x77\x61\x79\x20\x61\x63\x63\x65\x73\x73\x20\x72\x61\x6D\x70",
 { 3, 12, 11, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x48\x69\x63\x6B\x6F\x72\x79\x20\x41\x76\x65",
 { 0, 0, 10, 3, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x48\x69\x63\x6B\x6F\x72\x79\x20\x41\x76\x65",
 { 0, 0, 3, 15, 0, 0 } }, 
		{ 	"\x47\x61\x73\x20\x53\x74\x61\x74\x69\x6F\x6E",
 { 0, 0, 3, 0, 0, 0 } }, 
		{ 	"\x43\x61\x73\x68\x69\x65\x72\x73\x20\x42\x6F\x6F\x74\x68",
 { 8, 0, 0, 0, 0, 0 } }, 
		{ 	"\x43\x75\x6C\x2D\x64\x65\x2D\x73\x61\x63\x20\x61\x74\x20\x45\x61\x73\x74\x20\x65\x6E\x64\x20\x6F\x66\x20\x48\x69\x63\x6B\x6F\x72\x79",
 { 0, 0, 0, 6, 0, 0 } }, 
		{ 	"\x66\x69\x65\x6C\x64\x20\x6F\x66\x20\x74\x61\x6C\x6C\x20\x67\x72\x61\x73\x73",
 { 0, 0, 0, 5, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x66\x72\x65\x65\x77\x61\x79",
 { 0, 0, 12, 12, 0, 0 } }, 
		{ 	"\x73\x68\x65\x64\x20\x66\x75\x6C\x6C\x20\x6F\x66\x20\x72\x61\x64\x69\x6F\x20\x65\x71\x75\x69\x70\x6D\x65\x6E\x74",
 { 7, 0, 0, 0, 0, 0 } }, 
		{ 	"\x4D\x69\x6E\x65\x20\x53\x68\x61\x66\x74",
 { 0, 24, 0, 0, 0, 0 } }, 
		{ 	"\x43\x75\x6C\x2D\x64\x65\x2D\x73\x61\x63\x20\x61\x74\x20\x57\x65\x73\x74\x20\x65\x6E\x64\x20\x6F\x66\x20\x48\x69\x63\x6B\x6F\x72\x79",
 { 0, 0, 7, 0, 0, 0 } }, 
		{ 	"\x6C\x69\x76\x69\x6E\x67\x20\x72\x6F\x6F\x6D\x20\x6F\x66\x20\x61\x20\x6C\x61\x72\x67\x65\x20\x68\x6F\x75\x73\x65",
 { 0, 0, 15, 0, 0, 0 } }, 
		{ 	"\x68\x61\x72\x64\x77\x61\x72\x65\x20\x73\x74\x6F\x72\x65",
 { 0, 0, 4, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x74\x68\x65\x20\x65\x61\x73\x74\x20\x65\x6E\x64\x20\x6F\x66\x20\x61\x20\x70\x61\x72\x6B\x69\x6E\x67\x20\x6C\x6F\x74",
 { 22, 4, 0, 19, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x74\x68\x65\x20\x77\x65\x73\x74\x20\x65\x6E\x64\x20\x6F\x66\x20\x61\x20\x70\x61\x72\x6B\x69\x6E\x67\x20\x6C\x6F\x74",
 { 20, 0, 18, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x62\x65\x61\x63\x68",
 { 21, 19, 0, 0, 0, 0 } }, 
		{ 	"\x63\x6F\x6C\x64\x20\x73\x68\x61\x6C\x6C\x6F\x77\x20\x6C\x61\x6B\x65",
 { 0, 20, 22, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x74\x68\x65\x20\x66\x6F\x6F\x74\x20\x6F\x66\x20\x61\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E",
 { 0, 18, 0, 21, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E\x20\x6C\x65\x64\x67\x65",
 { 0, 0, 0, 0, 0, 22 } }, 
		{ 	"\x63\x61\x76\x65\x72\x6E\x20\x69\x6E\x73\x69\x64\x65\x20\x74\x68\x65\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E",
 { 14, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6C\x69\x66\x65\x67\x75\x61\x72\x64\x20\x73\x68\x61\x63\x6B",
 { 0, 0, 20, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x67\x61\x72\x61\x67\x65\x20\x6F\x66\x20\x61\x20\x6C\x61\x72\x67\x65\x20\x68\x6F\x75\x73\x65",
 { 0, 10, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x65\x68\x69\x6E\x64\x20\x74\x68\x65\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E\x20\x6F\x6E\x20\x4D\x6F\x75\x6E\x74\x61\x69\x6E\x20\x56\x69\x65\x77",
 { 0, 0, 4, 4, 0, 0 } }, 
		{ 	"\x63\x6F\x63\x6B\x70\x69\x74\x20\x6F\x66\x20\x74\x68\x65\x20\x6A\x65\x74\x20\x63\x61\x72",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x20\x20\x20\x20",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x68\x65\x72\x65\x61\x66\x74\x65\x72\x2E",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	2,
	3,
	3,
	4,
	23,
	7,
	10,
	26,
	24,
	0,
	15,
	8,
	8,
	8,
	9,
	9,
	1,
	1,
	0,
	0,
	0,
	0,
	0,
	17,
	0,
	17,
	17,
	19,
	0,
	20,
	25,
	0,
	24,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	29,
	13,
	0,
	0,
	34,
	2,
	20,
	0,
	50,
	17,
	14,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
};


const char *objtext[] = {
	"\x53\x74\x61\x69\x72\x63\x61\x73\x65",
	"\x59\x6F\x79\x6F\x64\x79\x6E\x65\x20\x62\x75\x69\x6C\x64\x69\x6E\x67",
	"\x47\x61\x73\x20\x53\x74\x61\x74\x69\x6F\x6E",
	"\x48\x61\x72\x64\x77\x61\x72\x65\x20\x73\x74\x6F\x72\x65",
	"\x51\x75\x61\x72\x74\x7A",
	"\x53\x68\x65\x64\x20\x77\x69\x74\x68\x20\x61\x6E\x74\x65\x6E\x6E\x61",
	"\x4C\x61\x72\x67\x65\x20\x68\x6F\x75\x73\x65",
	"\x53\x6D\x61\x6C\x6C\x20\x74\x6F\x6F\x6C\x62\x6F\x78",
	"\x4D\x69\x6E\x65\x20\x73\x68\x61\x66\x74",
	"\x46\x6C\x61\x73\x68\x6C\x69\x67\x68\x74\x20\x28\x6F\x6E\x29",
	"\x4C\x61\x72\x67\x65\x20\x68\x6F\x75\x73\x65",
	"\x47\x61\x73\x20\x50\x75\x6D\x70\x73",
	"\x43\x61\x73\x68\x69\x65\x72\x73\x20\x42\x6F\x6F\x74\x68",
	"\x46\x69\x6C\x6C\x65\x72\x20\x70\x69\x70\x65\x20\x74\x6F\x20\x75\x6E\x64\x65\x72\x67\x72\x6F\x75\x6E\x64\x20\x67\x61\x73\x20\x74\x61\x6E\x6B",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x43\x6F\x6E\x73\x6F\x6C\x65",
	"\x54\x72\x61\x73\x68\x20\x50\x69\x6C\x65",
	"\x53\x74\x61\x69\x72\x63\x61\x73\x65\x20\x67\x6F\x69\x6E\x67\x20\x64\x6F\x77\x6E",
	"\x54\x61\x62\x6C\x65",
	"\x46\x6C\x6F\x6F\x72\x20\x73\x61\x66\x65",
	"\x4F\x6C\x64\x20\x63\x61\x72\x20\x62\x61\x74\x74\x65\x72\x79",
	"\x4F\x70\x65\x6E\x20\x74\x6F\x6F\x6C\x62\x6F\x78",
	"\x4B\x65\x79",
	"\x52\x6F\x6C\x6C\x20\x6F\x66\x20\x64\x75\x63\x74\x20\x74\x61\x70\x65",
	"\x43\x6C\x69\x6D\x62\x65\x72\x73\x20\x50\x69\x63\x6B",
	"\x55\x6E\x6C\x6F\x63\x6B\x65\x64\x20\x66\x69\x6C\x6C\x65\x72\x20\x70\x69\x70\x65",
	"\x48\x79\x64\x72\x6F\x6D\x65\x74\x65\x72",
	"\x4D\x61\x6E\x75\x61\x6C\x20\x68\x61\x6E\x64\x20\x70\x75\x6D\x70",
	"\x4A\x65\x74\x20\x43\x61\x72",
	"\x4B\x65\x79",
	"\x4C\x69\x66\x65\x67\x75\x61\x72\x64\x20\x73\x68\x61\x63\x6B",
	"\x53\x6D\x61\x6C\x6C\x20\x48\x61\x6D\x20\x72\x61\x64\x69\x6F",
	"\x46\x6C\x61\x73\x68\x6C\x69\x67\x68\x74\x20\x28\x6F\x66\x66\x29",
	"\x42\x6F\x6D\x62\x20\x68\x6F\x75\x73\x69\x6E\x67",
	"\x45\x6E\x76\x65\x6C\x6F\x70\x65",
	"\x6C\x61\x72\x67\x65\x20\x68\x6F\x6C\x65",
	"\x41\x6E\x6F\x74\x68\x65\x72\x20\x6C\x61\x72\x67\x65\x20\x68\x6F\x6C\x65",
	"\x32\x30\x20\x66\x74\x20\x6F\x66\x20\x70\x68\x6F\x6E\x65\x20\x6C\x69\x6E\x65",
	"\x50\x68\x6F\x6E\x65\x20\x6C\x69\x6E\x65\x20\x64\x61\x6E\x67\x6C\x69\x6E\x67\x20\x66\x72\x6F\x6D\x20\x61\x62\x6F\x76\x65",
	"\x51\x75\x61\x72\x74\x7A",
	"\x53\x68\x65\x65\x74\x20\x6F\x66\x20\x70\x61\x70\x65\x72",
	"\x46\x75\x65\x6C\x20\x68\x6F\x73\x65",
	"\x46\x6F\x72\x6D\x75\x6C\x61",
	"\x44\x61\x73\x68\x62\x6F\x61\x72\x64",
	"\x41\x6E\x74\x65\x6E\x6E\x61\x20\x6C\x65\x61\x64",
	"\x42\x61\x74\x74\x65\x72\x79\x20\x61\x74\x74\x61\x63\x68\x65\x64\x20\x74\x6F\x20\x72\x61\x64\x69\x6F",
	"\x45\x6D\x70\x74\x79\x20\x63\x68\x65\x6D\x69\x63\x61\x6C\x20\x4A\x75\x67",
	"\x4A\x75\x67\x20\x6F\x66\x20\x67\x61\x73\x6F\x6C\x69\x6E\x65",
	"\x4D\x69\x78\x69\x6E\x67\x20\x76\x61\x74",
	"\x53\x61\x6E\x64",
	"\x4A\x61\x72\x20\x6F\x66\x20\x67\x6C\x6F\x77\x69\x6E\x67\x20\x46\x6C\x75\x69\x64",
	"\x4A\x75\x67\x20\x6F\x66\x20\x6A\x65\x74\x20\x66\x75\x65\x6C",
	"\x4A\x75\x6D\x70\x65\x72\x20\x43\x61\x62\x6C\x65\x73",
	"\x48\x65\x61\x76\x79\x20\x73\x74\x6F\x6E\x65\x20\x64\x6F\x6F\x72",
	"\x20",
	"",
	"",
	"",
	"",
	"",
	"",
	"",
};
const char *msgptr[] = {
	"\x20\x20\x20\x20\x20\x20",
	"\x4F\x2E\x4B\x2E",
	"\x49\x20\x73\x65\x65",
	"\x6E\x6F\x74\x68\x69\x6E\x67",
	"\x64\x61\x72\x6B\x6E\x65\x73\x73",
	"\x69\x74\x27\x73\x20\x70\x61\x64\x6C\x6F\x63\x6B\x65\x64\x2E",
	"\x74\x68\x65\x69\x72\x20\x74\x75\x72\x6E\x65\x64\x20\x6F\x66\x66\x2E",
	"\x61\x20\x73\x77\x69\x74\x63\x68\x20\x74\x6F\x20\x74\x68\x65\x20\x67\x61\x73\x20\x70\x75\x6D\x70\x73\x2E",
	"\x73\x70\x65\x63\x69\x61\x6C",
	"\x49\x20\x67\x6F\x74",
	"\x69\x74\x27\x73\x20\x61\x20\x6B\x65\x79\x20\x73\x61\x66\x65\x2E",
	"\x74\x68\x65\x72\x65\x27\x73\x20\x6E\x6F",
	"\x77\x61\x74\x65\x72\x20\x69\x6E\x20\x74\x68\x65\x20\x62\x61\x74\x74\x65\x72\x79\x2E",
	"\x49\x20\x63\x61\x6E\x27\x74\x2C",
	"\x42\x61\x74\x74\x65\x72\x79\x20\x74\x65\x73\x74\x73\x3A",
	"\x57\x65\x61\x6B",
	"\x44\x65\x61\x64",
	"\x6D\x6F\x76\x65\x73",
	"\x74\x68\x65\x20\x68\x6F\x73\x65\x20\x69\x73",
	"\x41\x6E\x20\x61\x74\x6F\x6D\x69\x63\x20\x65\x78\x70\x6C\x6F\x73\x69\x6F\x6E\x20\x65\x72\x75\x70\x74\x73\x2E",
	"\x68\x61\x70\x70\x65\x6E\x73",
	"\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67",
	"\x53\x74\x69\x63\x6B\x65\x72\x20\x73\x61\x79\x73\x3A\x20\x53\x61\x6D\x27\x73\x20\x53\x65\x72\x76\x69\x63\x65\x20\x53\x74\x61\x74\x69\x6F\x6E\x2E",
	"\x74\x68\x65\x20\x6B\x65\x79\x20\x64\x6F\x65\x73\x6E\x27\x74\x20\x66\x69\x74\x2E",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x6E\x6F\x20\x62\x61\x74\x74\x65\x72\x79",
	"\x62\x61\x74\x74\x65\x72\x79\x73\x20\x61\x72\x65\x20\x64\x65\x61\x64",
	"\x42\x6F\x6D\x62\x20\x61\x63\x6B\x6E\x6F\x77\x6C\x65\x64\x67\x65\x73\x20\x63\x6F\x64\x65",
	"\x54\x68\x65\x20\x65\x61\x72\x74\x68\x20\x69\x73\x20\x73\x61\x66\x65\x20\x61\x67\x61\x69\x6E\x2E",
	"\x6D\x61\x72\x6B\x65\x64\x20\x27\x52\x46\x49\x20\x53\x68\x69\x65\x6C\x64\x27\x20\x69\x73",
	"\x61\x20\x73\x77\x69\x74\x63\x68",
	"\x6C\x69\x67\x68\x74",
	"\x6F\x66\x66",
	"\x6F\x6E",
	"\x4E\x6F\x74\x20\x74\x6F\x20\x74\x68\x61\x74\x2E",
	"\x61\x6E\x64\x20\x61\x6E\x20\x65\x6E\x76\x65\x6C\x6F\x70\x65",
	"\x77\x6F\x72\x6B",
	"\x53\x63\x6F\x74\x74\x20\x77\x6F\x6E\x27\x74\x20\x6C\x65\x74\x20\x6D\x65\x2E",
	"\x61\x20\x6C\x65\x64\x67\x65\x20\x61\x62\x6F\x75\x74\x20\x31\x35\x27\x20\x75\x70\x20\x77\x69\x74\x68\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x70\x65\x61\x6B\x2E",
	"\x61\x6E\x20\x75\x6E\x64\x65\x72\x67\x72\x6F\x75\x6E\x64\x20\x70\x68\x6F\x6E\x65\x20\x6C\x69\x6E\x65\x2E",
	"\x74\x68\x65\x20\x6C\x69\x6E\x65\x20\x69\x73\x20\x73\x65\x76\x65\x72\x65\x64\x2E",
	"\x4E\x6F\x74\x20\x77\x69\x74\x68\x20\x74\x68\x65\x73\x65\x20\x68\x61\x6E\x64\x73\x2E",
	"\x77\x68\x69\x6C\x65\x20\x63\x61\x72\x72\x79\x69\x6E\x67\x20\x61\x6E\x79\x74\x68\x69\x6E\x67\x2E",
	"\x69\x74\x27\x73\x20\x73\x74\x69\x6C\x6C\x20\x61\x74\x74\x61\x63\x68\x65\x64\x2E",
	"\x54\x6F\x20\x77\x68\x61\x74\x3F",
	"\x49\x74\x27\x73\x20\x69\x6D\x62\x65\x64\x64\x65\x64\x20\x69\x6E\x20\x72\x6F\x63\x6B",
	"\x44\x69\x73\x61\x72\x6D\x20\x43\x6F\x64\x65\x3A\x20\x31\x3D\x57\x61\x72\x66\x69\x6E\x20\x32\x3D\x59\x6F\x79\x6F\x64\x79\x6E\x65",
	"\x54\x68\x65\x20\x63\x6F\x63\x6B\x70\x69\x74\x20\x69\x73",
	"\x6F\x70\x65\x6E",
	"\x63\x6C\x6F\x73\x65\x64",
	"\x61\x20\x66\x75\x65\x6C\x20\x74\x61\x6E\x6B\x20\x61\x6E\x64\x20\x61\x20\x6A\x65\x74\x20\x65\x6E\x67\x69\x6E\x65",
	"\x48\x6F\x77\x3F",
	"\x73\x74\x61\x72\x74\x65\x72\x20\x62\x75\x74\x74\x6F\x6E\x20\x26\x20\x67\x6C\x6F\x76\x65\x20\x63\x6F\x6D\x70\x61\x72\x74\x6D\x65\x6E\x74\x2E",
	"\x46\x75\x65\x6C\x20\x67\x61\x75\x67\x65\x20\x72\x65\x61\x64\x73",
	"\x66\x75\x6C\x6C",
	"\x65\x6D\x70\x74\x79",
	"\x46\x75\x65\x6C\x20\x69\x73\x20\x6D\x61\x64\x65\x20\x66\x72\x6F\x6D",
	"\x67\x61\x73\x6F\x6C\x69\x6E\x65\x2C\x20\x73\x61\x6E\x64\x2C\x20\x71\x75\x61\x72\x74\x7A\x2C\x20\x63\x61\x74\x61\x6C\x79\x73\x74\x2E",
	"\x61\x20\x74\x72\x61\x6E\x73\x6D\x69\x74\x20\x73\x77\x69\x74\x63\x68",
	"\x49\x20\x68\x65\x61\x72\x20\x74\x68\x65\x20\x62\x6F\x6D\x62\x20\x77\x69\x6C\x6C\x20\x65\x78\x70\x6C\x6F\x64\x65\x20\x69\x6E",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x6E\x6F\x20\x61\x6E\x74\x65\x6E\x6E\x61",
	"\x49\x6E\x20\x32\x20\x77\x6F\x72\x64\x73\x2C\x20\x63\x6F\x6E\x6E\x65\x63\x74\x20\x74\x6F\x20\x77\x68\x61\x74\x3F\x2C\x20\x69\x2E\x65\x2E\x20\x43\x4F\x4E\x4E\x45\x43\x54\x20\x50\x49\x50\x45",
	"\x49\x74\x27\x73\x20\x6E\x6F\x74\x20\x77\x6F\x72\x6B\x69\x6E\x67\x2E",
	"\x54\x6F\x20\x74\x72\x61\x6E\x73\x6D\x69\x74\x20\x73\x61\x79\x20\x22\x54\x52\x41\x4E\x53\x4D\x49\x54\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x22",
	"\x36",
	"\x31\x30",
	"\x66\x74\x20\x6C\x6F\x6E\x67",
	"\x69\x74\x20\x77\x6F\x6E\x27\x74\x20\x73\x74\x61\x79",
	"\x74\x61\x70\x65\x20\x68\x6F\x6C\x64\x73\x20\x69\x74",
	"\x61\x74\x74\x61\x63\x68\x65\x64\x20\x74\x6F\x20\x70\x75\x6D\x70\x2E",
	"\x49\x74\x20\x64\x6F\x65\x73\x6E\x27\x74",
	"\x72\x65\x61\x63\x68\x20\x74\x68\x65\x20\x67\x61\x73\x6F\x6C\x69\x6E\x65\x2E",
	"\x49\x74\x20\x64\x6F\x65\x73",
	"\x49\x20\x6C\x6F\x73\x65\x20\x69\x74\x20\x69\x6E\x20\x74\x68\x65\x20\x74\x61\x6E\x6B\x2E",
	"\x63\x6F\x6E\x6E\x65\x63\x74\x65\x64\x20\x62\x79\x20\x61\x20\x68\x6F\x73\x65\x2E",
	"\x43\x61\x72\x20\x63\x61\x74\x63\x68\x65\x73\x20\x66\x69\x72\x65\x20\x61\x6E\x64\x20\x62\x6C\x6F\x77\x73\x20\x75\x70\x2E",
	"\x49\x27\x6D\x20\x66\x72\x69\x65\x64\x20\x42\x61\x6E\x7A\x61\x69\x2E",
	"\x45\x6E\x67\x69\x6E\x65\x20\x69\x73\x20\x69\x64\x6C\x69\x6E\x67\x2E",
	"\x70\x6F\x77\x65\x72\x20\x74\x65\x72\x6D\x69\x6E\x61\x6C\x73\x2C",
	"\x49\x20\x64\x72\x69\x76\x65\x20\x74\x6F\x20\x4D\x61\x69\x6E\x20\x53\x74\x20\x66\x6F\x72\x20\x61\x74\x74\x65\x6D\x70\x74\x20\x74\x6F\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E",
	"\x49\x20\x67\x6F\x20\x74\x68\x72\x75\x20\x64\x6F\x6F\x72\x2E\x20\x53\x6C\x61\x6D\x21\x21\x21",
	"\x63\x6F\x6E\x74\x61\x69\x6E\x65\x72",
	"\x49\x20\x64\x72\x69\x76\x65\x20\x69\x6E\x74\x6F\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E\x2E",
	"\x75\x6E\x64\x65\x72\x20\x73\x74\x61\x69\x72\x73\x2C\x20\x49\x20\x67\x65\x74\x20\x69\x74\x2E",
	"\x22\x42\x75\x63\x6B\x61\x72\x6F\x6F\x20\x42\x61\x6E\x7A\x61\x69\x22",
	"\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x62\x79\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x20\x26\x20\x50\x68\x69\x6C\x6C\x69\x70\x20\x43\x61\x73\x65",
	"\x49\x20\x63\x61\x6E\x27\x74\x20\x64\x6F\x20\x74\x68\x61\x74\x2E\x2E\x2E\x79\x65\x74",
	"\x72\x65\x73\x69\x64\x75\x65\x20\x69\x6E\x73\x69\x64\x65\x20\x69\x74\x2E",
	"\x69\x74\x27\x73\x20\x63\x6C\x65\x61\x6E\x2E",
	"\x4E\x6F\x20\x66\x6F\x72\x6D\x75\x6C\x61\x21",
	"\x69\x73\x20\x6D\x69\x73\x73\x69\x6E\x67\x2E",
	"",
	"",
	"",
	"",
	"",
	"",
};


const uint8_t status[] = {
	183, 
	7, 2, 7, 24, 8, 15, 9, 6, 7, 14, 
	67, 0, 0, 73, 
	203, 
	8, 0, 0, 27, 
	60, 68, 57, 64, 
	178, 
	8, 16, 9, 28, 0, 31, 0, 28, 
	52, 58, 64, 
	178, 
	4, 24, 6, 9, 9, 27, 0, 27, 
	56, 58, 64, 
	168, 
	1, 30, 0, 14, 
	60, 
	183, 
	2, 43, 2, 44, 8, 30, 8, 14, 0, 1, 
	108, 81, 78, 73, 
	197, 
	0, 1, 
	81, 17, 
	172, 
	2, 30, 1, 44, 0, 44, 
	53, 
	172, 
	2, 44, 1, 30, 0, 44, 
	74, 
	179, 
	9, 2, 64, 238, 0, 4, 0, 1, 
	79, 58, 81, 73, 
	198, 
	0, 2, 
	58, 133, 134, 
	164, 
	0, 1, 
	81, 
	171, 
	19, 0, 0, 35, 
	54, 64, 19, 125, 
	164, 
	0, 1, 
	81, 
	170, 
	0, 1, 0, 1, 
	81, 77, 81, 
	172, 
	7, 8, 1, 26, 0, 12, 
	60, 
	164, 
	4, 35, 
	63, 
	173, 
	1, 51, 0, 9, 0, 10, 
	60, 60, 
	168, 
	1, 27, 0, 10, 
	60, 
	180, 
	8, 9, 8, 10, 8, 11, 8, 31, 0, 30, 
	58, 
	169, 
	4, 14, 9, 9, 
	56, 64, 
	182, 
	9, 3, 4, 12, 0, 3, 0, 2, 0, 1, 
	58, 79, 81, 
	170, 
	4, 28, 0, 6, 
	60, 57, 64, 
};
const uint8_t actions[] = {
	7, 50, 0, 
	6, 45, 
	13, 11, 104, 130, 
	4, 49, 18, 
	4, 0, 
	51, 
	4, 51, 0, 
	4, 0, 
	51, 
	9, 66, 59, 
	4, 14, 0, 28, 
	129, 54, 
	23, 52, 57, 
	1, 26, 8, 13, 0, 13, 0, 12, 0, 40, 
	60, 60, 74, 1, 
	23, 50, 71, 
	8, 12, 2, 24, 1, 45, 0, 46, 0, 45, 
	1, 72, 67, 73, 
	201, 
	8, 0, 0, 20, 
	68, 58, 
	18, 43, 57, 
	2, 24, 1, 26, 8, 13, 0, 12, 
	121, 120, 58, 
	14, 43, 57, 
	2, 24, 1, 40, 0, 40, 
	1, 122, 59, 
	13, 43, 57, 
	2, 24, 1, 26, 9, 13, 
	119, 120, 
	23, 38, 57, 
	1, 40, 1, 26, 1, 22, 0, 13, 0, 40, 
	58, 117, 118, 59, 
	13, 38, 57, 
	1, 40, 1, 26, 6, 22, 
	116, 118, 
	4, 38, 18, 
	1, 26, 
	110, 
	11, 5, 18, 
	3, 26, 8, 13, 
	2, 18, 114, 115, 
	11, 5, 18, 
	3, 26, 9, 13, 
	2, 18, 113, 115, 
	19, 41, 70, 
	4, 13, 8, 14, 8, 29, 8, 30, 
	67, 0, 0, 73, 
	203, 
	8, 0, 9, 1, 
	3, 8, 20, 73, 
	203, 
	8, 0, 9, 1, 
	68, 0, 0, 73, 
	202, 
	8, 0, 0, 17, 
	26, 68, 58, 
	23, 41, 10, 
	4, 13, 8, 14, 8, 17, 8, 29, 8, 30, 
	1, 26, 27, 63, 
	9, 1, 7, 
	2, 2, 0, 8, 
	54, 1, 
	9, 1, 8, 
	2, 12, 0, 9, 
	54, 1, 
	9, 1, 10, 
	2, 1, 0, 1, 
	54, 1, 
	5, 5, 12, 
	2, 16, 
	2, 4, 
	11, 1, 12, 
	2, 16, 0, 2, 
	54, 1, 56, 64, 
	5, 5, 13, 
	2, 13, 
	2, 5, 
	9, 1, 15, 
	2, 10, 0, 16, 
	54, 1, 
	9, 1, 15, 
	2, 6, 0, 26, 
	54, 1, 
	9, 1, 16, 
	2, 3, 0, 17, 
	54, 1, 
	5, 5, 18, 
	2, 11, 
	2, 6, 
	5, 5, 19, 
	2, 14, 
	2, 7, 
	5, 66, 21, 
	2, 14, 
	3, 20, 
	13, 66, 25, 
	2, 17, 0, 18, 0, 1, 
	62, 1, 
	5, 64, 21, 
	2, 14, 
	3, 20, 
	13, 73, 25, 
	2, 17, 0, 18, 0, 1, 
	62, 1, 
	19, 5, 23, 
	2, 15, 17, 19, 0, 19, 0, 9, 
	62, 1, 2, 21, 
	23, 19, 29, 
	3, 7, 17, 22, 0, 7, 0, 20, 0, 21, 
	72, 74, 0, 73, 
	203, 
	0, 22, 0, 31, 
	74, 74, 9, 21, 
	5, 5, 6, 
	4, 1, 
	2, 4, 
	17, 19, 13, 
	2, 13, 1, 21, 0, 24, 0, 13, 
	72, 1, 
	5, 5, 35, 
	2, 18, 
	2, 10, 
	18, 10, 26, 
	2, 44, 0, 44, 0, 29, 0, 19, 
	59, 60, 74, 
	8, 10, 26, 
	2, 19, 0, 19, 
	52, 
	18, 18, 26, 
	1, 44, 0, 44, 0, 19, 0, 14, 
	59, 53, 60, 
	8, 18, 26, 
	1, 19, 0, 19, 
	53, 
	9, 1, 42, 
	4, 20, 0, 25, 
	1, 54, 
	13, 46, 26, 
	3, 19, 4, 21, 0, 31, 
	58, 1, 
	10, 5, 26, 
	3, 19, 9, 31, 
	2, 11, 12, 
	9, 5, 26, 
	3, 19, 8, 31, 
	2, 12, 
	4, 5, 29, 
	3, 7, 
	22, 
	13, 23, 26, 
	3, 19, 1, 25, 8, 30, 
	14, 15, 
	13, 23, 26, 
	3, 19, 1, 25, 9, 30, 
	14, 16, 
	13, 23, 26, 
	3, 22, 1, 25, 8, 30, 
	14, 15, 
	13, 23, 26, 
	3, 44, 1, 25, 9, 30, 
	14, 16, 
	15, 5, 62, 
	4, 29, 9, 18, 0, 41, 
	9, 21, 74, 73, 
	201, 
	0, 18, 0, 49, 
	58, 74, 
	9, 25, 44, 
	8, 16, 3, 31, 
	13, 25, 
	14, 25, 44, 
	3, 31, 0, 9, 0, 31, 
	72, 64, 1, 
	14, 25, 45, 
	3, 9, 0, 9, 0, 31, 
	72, 64, 1, 
	15, 5, 48, 
	4, 24, 1, 9, 9, 1, 
	2, 30, 28, 73, 
	194, 
	32, 2, 29, 
	196, 
	17, 33, 
	34, 
	15, 66, 21, 
	4, 24, 1, 9, 0, 1, 
	58, 2, 21, 20, 
	15, 5, 48, 
	4, 24, 1, 9, 8, 1, 
	30, 28, 31, 73, 
	196, 
	17, 33, 
	34, 
	17, 10, 34, 
	4, 24, 17, 33, 1, 9, 0, 33, 
	1, 52, 
	14, 19, 34, 
	1, 33, 6, 28, 0, 28, 
	9, 21, 74, 
	5, 5, 52, 
	4, 22, 
	2, 37, 
	19, 84, 0, 
	4, 11, 1, 23, 9, 26, 0, 26, 
	58, 1, 0, 73, 
	200, 
	0, 34, 0, 11, 
	62, 
	21, 84, 0, 
	8, 26, 1, 23, 4, 11, 0, 35, 0, 11, 
	62, 1, 
	9, 5, 54, 
	2, 34, 17, 36, 
	2, 38, 
	22, 27, 55, 
	2, 34, 1, 23, 9, 25, 0, 25, 17, 36, 
	2, 39, 58, 
	22, 27, 55, 
	2, 35, 8, 25, 1, 23, 0, 24, 17, 36, 
	58, 2, 39, 
	19, 66, 55, 
	2, 35, 8, 24, 17, 36, 0, 36, 
	52, 1, 9, 21, 
	17, 10, 55, 
	2, 35, 8, 24, 17, 36, 0, 36, 
	52, 1, 
	17, 64, 55, 
	1, 36, 4, 22, 0, 36, 0, 37, 
	59, 53, 
	9, 34, 55, 
	2, 37, 10, 0, 
	13, 41, 
	13, 34, 55, 
	2, 37, 11, 0, 0, 23, 
	54, 1, 
	4, 36, 55, 
	2, 37, 
	43, 
	18, 37, 51, 
	2, 37, 3, 23, 0, 23, 0, 23, 
	1, 58, 59, 
	13, 10, 53, 
	4, 23, 6, 23, 2, 4, 
	13, 44, 
	22, 10, 53, 
	1, 23, 4, 23, 2, 4, 0, 4, 0, 38, 
	1, 59, 52, 
	15, 19, 35, 
	1, 28, 2, 18, 0, 39, 
	52, 1, 9, 21, 
	5, 5, 50, 
	1, 39, 
	2, 45, 
	7, 5, 39, 
	2, 27, 
	2, 46, 47, 73, 
	194, 
	2, 127, 49, 
	196, 
	9, 21, 
	123, 
	196, 
	8, 11, 
	126, 
	22, 10, 57, 
	2, 27, 9, 21, 9, 11, 0, 40, 0, 21, 
	52, 58, 1, 
	9, 10, 57, 
	2, 40, 0, 40, 
	52, 1, 
	18, 32, 57, 
	1, 40, 2, 27, 0, 21, 0, 40, 
	1, 60, 59, 
	9, 1, 59, 
	2, 27, 0, 29, 
	54, 1, 
	13, 35, 59, 
	4, 29, 9, 22, 0, 22, 
	1, 58, 
	7, 5, 64, 
	4, 29, 
	2, 51, 46, 73, 
	197, 
	9, 22, 
	47, 102, 
	197, 
	8, 22, 
	48, 102, 
	196, 
	8, 19, 
	103, 
	196, 
	8, 8, 
	103, 
	200, 
	9, 19, 9, 8, 
	104, 
	15, 19, 62, 
	4, 29, 9, 18, 0, 41, 
	9, 21, 74, 73, 
	201, 
	0, 18, 0, 49, 
	58, 74, 
	5, 5, 65, 
	1, 41, 
	105, 106, 
	7, 5, 43, 
	3, 30, 
	2, 107, 0, 73, 
	199, 
	9, 29, 
	24, 0, 0, 73, 
	199, 
	9, 14, 
	109, 0, 0, 73, 
	211, 
	8, 14, 8, 29, 8, 30, 8, 31, 
	67, 0, 0, 73, 
	199, 
	9, 0, 
	111, 0, 0, 73, 
	192, 
	68, 
	4, 38, 43, 
	3, 30, 
	110, 
	22, 38, 66, 
	4, 13, 9, 14, 3, 30, 0, 14, 0, 30, 
	58, 53, 1, 
	4, 38, 75, 
	3, 51, 
	43, 
	19, 38, 26, 
	3, 30, 3, 19, 9, 29, 0, 29, 
	58, 1, 0, 73, 
	200, 
	0, 19, 0, 44, 
	72, 
	16, 66, 21, 
	4, 13, 2, 44, 8, 14, 8, 30, 
	112, 
	23, 55, 74, 
	4, 2, 3, 9, 3, 38, 3, 49, 3, 46, 
	0, 0, 0, 73, 
	215, 
	3, 48, 3, 41, 0, 38, 0, 49, 0, 48, 
	59, 59, 59, 67, 
	196, 
	9, 0, 
	135, 
	211, 
	8, 0, 0, 46, 0, 34, 0, 50, 
	62, 52, 1, 68, 
	22, 46, 39, 
	2, 27, 1, 50, 0, 50, 0, 45, 0, 19, 
	72, 58, 1, 
	10, 66, 79, 
	4, 29, 8, 8, 
	124, 125, 63, 
	10, 66, 79, 
	4, 29, 9, 19, 
	3, 8, 20, 
	10, 66, 79, 
	4, 29, 8, 21, 
	124, 125, 63, 
	10, 66, 79, 
	4, 29, 8, 20, 
	124, 125, 63, 
	14, 66, 79, 
	4, 29, 9, 22, 0, 11, 
	1, 126, 58, 
	18, 66, 79, 
	4, 29, 8, 4, 0, 4, 0, 5, 
	60, 58, 128, 
	23, 66, 79, 
	4, 29, 8, 5, 0, 5, 0, 6, 0, 19, 
	60, 58, 60, 73, 
	194, 
	131, 56, 64, 
	10, 10, 57, 
	2, 27, 8, 11, 
	124, 125, 63, 
	18, 19, 59, 
	4, 29, 8, 22, 0, 22, 0, 11, 
	60, 60, 1, 
	13, 59, 57, 
	1, 40, 4, 21, 0, 20, 
	60, 1, 
	6, 10, 5, 
	8, 22, 
	13, 46, 48, 
	22, 10, 5, 
	4, 29, 8, 4, 0, 19, 0, 27, 0, 19, 
	54, 62, 1, 
	22, 10, 5, 
	4, 29, 8, 5, 0, 3, 0, 27, 0, 3, 
	54, 62, 1, 
	22, 10, 5, 
	4, 29, 8, 6, 0, 24, 0, 27, 0, 24, 
	54, 62, 1, 
	9, 19, 59, 
	4, 14, 0, 28, 
	129, 54, 
	18, 37, 72, 
	2, 27, 3, 51, 0, 9, 0, 51, 
	1, 58, 53, 
	23, 37, 26, 
	2, 27, 3, 51, 0, 10, 0, 51, 0, 19, 
	58, 53, 53, 1, 
	7, 5, 80, 
	4, 2, 
	1, 2, 3, 8, 
	4, 46, 73, 
	1, 45, 
	50, 
	22, 46, 39, 
	2, 27, 1, 46, 0, 8, 0, 46, 0, 45, 
	58, 72, 1, 
	4, 34, 52, 
	4, 22, 
	50, 
	1, 80, 0, 
	1, 85, 
	9, 1, 39, 
	2, 27, 0, 29, 
	54, 1, 
	6, 19, 62, 
	4, 29, 
	3, 8, 20, 
	6, 10, 89, 
	8, 22, 
	13, 46, 48, 
	22, 10, 89, 
	4, 29, 8, 4, 0, 19, 0, 27, 0, 19, 
	54, 62, 1, 
	22, 10, 89, 
	4, 29, 8, 5, 0, 3, 0, 27, 0, 3, 
	54, 62, 1, 
	22, 10, 89, 
	4, 29, 8, 6, 0, 24, 0, 27, 0, 24, 
	54, 62, 1, 
	8, 1, 58, 
	4, 7, 0, 13, 
	54, 
	17, 10, 55, 
	4, 23, 9, 23, 0, 36, 0, 37, 
	52, 59, 
	17, 66, 55, 
	4, 23, 9, 23, 0, 36, 0, 37, 
	52, 59, 
	19, 10, 55, 
	4, 23, 8, 23, 0, 37, 0, 36, 
	55, 74, 67, 73, 
	206, 
	8, 0, 0, 23, 0, 23, 
	68, 74, 60, 
	23, 66, 55, 
	4, 23, 8, 23, 0, 37, 0, 36, 0, 23, 
	55, 74, 74, 73, 
	199, 
	0, 23, 
	60, 1, 9, 21, 
	12, 35, 29, 
	3, 20, 0, 20, 0, 7, 
	72, 
	12, 19, 29, 
	3, 7, 0, 7, 0, 20, 
	72, 
	23, 5, 12, 
	4, 2, 6, 45, 14, 45, 3, 9, 0, 45, 
	74, 2, 21, 132, 
	8, 1, 12, 
	4, 2, 0, 1, 
	54, 
	9, 25, 46, 
	8, 16, 3, 31, 
	13, 25, 
	14, 25, 46, 
	3, 31, 0, 9, 0, 31, 
	72, 64, 1, 
	14, 15, 46, 
	3, 9, 0, 9, 0, 31, 
	72, 64, 1, 
	12, 10, 53, 
	2, 38, 7, 23, 0, 38, 
	52, 
	8, 10, 34, 
	2, 33, 0, 33, 
	52, 
	8, 1, 59, 
	4, 14, 0, 28, 
	54, 
	7, 21, 0, 
	0, 35, 
	54, 64, 19, 125, 
	2, 5, 93, 
	1, 2, 133, 
	9, 5, 57, 
	1, 40, 8, 20, 
	2, 136, 
	9, 5, 57, 
	1, 40, 9, 20, 
	2, 137, 
	8, 55, 74, 
	6, 41, 4, 2, 
	138, 
	9, 55, 74, 
	4, 2, 1, 9, 
	21, 139, 
	18, 43, 57, 
	1, 40, 2, 27, 0, 21, 0, 40, 
	1, 60, 59, 
	4, 42, 26, 
	1, 19, 
	50, 
	8, 10, 55, 
	2, 36, 0, 36, 
	52, 
	6, 66, 21, 
	1, 30, 
	1, 3, 20, 
	4, 41, 0, 
	4, 0, 
	51, 
	4, 66, 39, 
	2, 27, 
	40, 
	1, 71, 0, 
	119, 35, 
	4, 57, 0, 
	4, 21, 
	1, 
	0, 37, 0, 
	33, 
	9, 66, 55, 
	2, 34, 9, 24, 
	13, 42, 
	9, 10, 55, 
	2, 34, 9, 24, 
	13, 42, 
	8, 27, 55, 
	6, 23, 2, 34, 
	40, 
	4, 84, 0, 
	6, 23, 
	40, 
	9, 19, 13, 
	1, 28, 6, 21, 
	13, 23, 
	10, 5, 0, 
	8, 15, 6, 9, 
	13, 11, 30, 
	9, 19, 35, 
	1, 21, 6, 28, 
	13, 23, 
	3, 5, 0, 
	1, 2, 3, 8, 
	0, 10, 27, 
	66, 
	0, 62, 0, 
	66, 
	0, 22, 0, 
	63, 
	0, 17, 28, 
	71, 
	0, 31, 0, 
	36, 
	7, 84, 0, 
	4, 20, 
	1, 3, 8, 20, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
215, 65, 76, 75,
210, 85, 78, 32,
197, 78, 84, 69,
76, 79, 79, 75,
197, 88, 65, 77,
204, 32, 32, 32,
196, 69, 83, 67,
210, 69, 65, 68,
71, 69, 84, 32,
212, 65, 75, 69,
199, 82, 65, 66,
208, 73, 67, 75,
210, 69, 77, 79,
85, 78, 76, 73,
32, 32, 32, 32,
83, 65, 86, 69,
68, 82, 79, 80,
79, 80, 69, 78,
213, 78, 76, 79,
83, 76, 69, 69,
81, 85, 73, 84,
67, 72, 69, 67,
212, 69, 83, 84,
76, 73, 71, 72,
198, 76, 65, 83,
67, 85, 84, 32,
211, 76, 73, 67,
200, 73, 84, 32,
195, 72, 79, 80,
72, 69, 76, 80,
82, 69, 80, 76,
210, 69, 67, 79,
67, 76, 73, 77,
67, 76, 79, 83,
84, 73, 69, 32,
84, 79, 32, 32,
67, 79, 78, 78,
212, 65, 80, 69,
193, 84, 84, 65,
84, 82, 65, 78,
67, 72, 65, 82,
80, 85, 84, 32,
201, 78, 83, 69,
32, 32, 32, 32,
70, 85, 69, 76,
210, 69, 70, 85,
198, 73, 76, 76,
83, 84, 65, 82,
80, 85, 77, 80,
70, 73, 78, 68,
85, 78, 67, 79,
196, 73, 83, 67,
32, 32, 32, 32,
77, 65, 75, 69,
205, 73, 88, 32,
83, 87, 73, 77,
215, 65, 68, 69,
87, 65, 83, 72,
210, 73, 78, 83,
195, 76, 69, 65,
73, 78, 86, 69,
67, 76, 73, 77,
84, 72, 82, 79,
195, 65, 83, 84,
80, 85, 83, 72,
208, 85, 76, 76,
208, 82, 69, 83,
198, 76, 73, 80,
204, 73, 70, 84,
85, 83, 69, 32,
215, 73, 84, 72,
83, 76, 73, 68,
205, 79, 86, 69,
76, 65, 85, 71,
83, 73, 78, 71,
83, 77, 69, 76,
76, 73, 83, 84,
83, 72, 65, 75,
83, 65, 89, 32,
217, 69, 76, 76,
211, 67, 82, 69,
212, 65, 76, 75,
68, 73, 71, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
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
83, 84, 65, 84,
66, 79, 79, 84,
195, 65, 83, 72,
89, 79, 89, 79,
194, 85, 73, 76,
83, 84, 65, 73,
80, 73, 80, 69,
208, 65, 68, 76,
72, 79, 85, 83,
72, 65, 82, 68,
211, 84, 79, 82,
80, 85, 77, 80,
67, 79, 78, 83,
83, 84, 82, 69,
83, 87, 73, 84,
67, 79, 82, 78,
80, 73, 76, 69,
212, 82, 65, 83,
84, 65, 66, 76,
66, 65, 84, 84,
73, 78, 86, 69,
71, 65, 77, 69,
84, 79, 79, 76,
194, 79, 88, 32,
75, 69, 89, 32,
68, 85, 67, 84,
212, 65, 80, 69,
69, 78, 86, 69,
83, 65, 70, 69,
72, 89, 68, 82,
82, 79, 79, 77,
80, 85, 77, 80,
67, 65, 82, 32,
202, 69, 84, 32,
212, 65, 78, 75,
83, 72, 65, 67,
82, 65, 68, 73,
79, 78, 32, 32,
79, 70, 70, 32,
70, 76, 65, 83,
204, 73, 71, 72,
66, 79, 77, 66,
195, 65, 78, 73,
80, 65, 80, 69,
80, 73, 67, 75,
77, 79, 85, 78,
81, 85, 65, 82,
72, 79, 76, 69,
76, 73, 78, 69,
208, 72, 79, 78,
72, 79, 83, 69,
83, 72, 69, 68,
67, 79, 67, 75,
215, 73, 78, 68,
196, 79, 79, 82,
71, 76, 79, 86,
195, 79, 77, 80,
68, 65, 83, 72,
70, 79, 82, 77,
65, 78, 84, 69,
204, 69, 65, 68,
84, 85, 78, 69,
196, 73, 65, 76,
87, 65, 82, 70,
71, 65, 83, 32,
84, 69, 82, 77,
74, 85, 71, 32,
70, 85, 69, 76,
67, 65, 66, 76,
83, 65, 78, 68,
74, 65, 82, 32,
198, 76, 85, 73,
66, 85, 84, 84,
86, 65, 84, 32,
77, 73, 78, 69,
211, 72, 65, 70,
32, 32, 32, 32,
211, 84, 79, 78,
79, 70, 70, 73,
84, 65, 78, 75,
70, 69, 69, 84,
72, 65, 78, 68,
79, 85, 84, 32,
66, 65, 83, 69,
76, 65, 66, 79,
69, 78, 71, 73,
77, 69, 32, 32,
205, 89, 83, 69,
71, 82, 65, 83,
70, 73, 69, 76,
72, 73, 71, 72,
76, 79, 84, 32,
76, 69, 68, 71,
76, 65, 75, 69,
66, 69, 65, 67,
67, 79, 68, 69,
205, 69, 83, 83,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
	0,
};
const uint8_t automap[] = {
84, 79, 79, 76,
	7,
70, 76, 65, 83,
	9,
66, 65, 84, 84,
	19,
84, 79, 79, 76,
	20,
75, 69, 89, 32,
	21,
68, 85, 67, 84,
	22,
80, 73, 67, 75,
	23,
72, 89, 68, 82,
	25,
80, 85, 77, 80,
	26,
75, 69, 89, 32,
	28,
82, 65, 68, 73,
	30,
70, 76, 65, 83,
	31,
69, 78, 86, 69,
	33,
76, 73, 78, 69,
	36,
81, 85, 65, 82,
	38,
80, 65, 80, 69,
	39,
72, 79, 83, 69,
	40,
70, 79, 82, 77,
	41,
66, 65, 84, 84,
	44,
74, 85, 71, 32,
	45,
74, 85, 71, 32,
	46,
83, 65, 78, 68,
	48,
74, 65, 82, 32,
	49,
74, 85, 71, 32,
	50,
67, 65, 66, 76,
	51,
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
