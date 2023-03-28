#define NUM_OBJ 50
#define WORDSIZE 4
#define GAME_MAGIC 784
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
const uint8_t treasure = 7;
const uint8_t treasures = 4;
const uint8_t lastloc = 40;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"\x53\x54\x4F\x52\x45\x52\x4F\x4F\x4D\x2E\x20\x20\x43\x41\x4E\x27\x54\x20\x47\x45\x54\x20\x48\x45\x52\x45",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x54\x68\x72\x6F\x6E\x65\x2D\x52\x6F\x6F\x6D",
 { 0, 0, 2, 6, 0, 0 } }, 
		{ 	"\x43\x6F\x72\x72\x69\x64\x6F\x72",
 { 3, 2, 2, 8, 0, 0 } }, 
		{ 	"\x43\x6F\x72\x72\x69\x64\x6F\x72",
 { 0, 1, 8, 4, 0, 0 } }, 
		{ 	"\x43\x6F\x72\x72\x69\x64\x6F\x72",
 { 3, 1, 3, 5, 5, 0 } }, 
		{ 	"\x50\x72\x69\x6E\x63\x65\x73\x73\x27\x20\x42\x65\x64\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 4 } }, 
		{ 	"\x44\x72\x65\x73\x73\x69\x6E\x67\x20\x52\x6F\x6F\x6D",
 { 0, 0, 1, 0, 0, 0 } }, 
		{ 	"\x52\x6F\x79\x61\x6C\x20\x54\x72\x65\x61\x73\x75\x72\x65\x20\x43\x68\x61\x6D\x62\x65\x72",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x50\x61\x6C\x61\x63\x65\x20\x43\x6F\x72\x72\x69\x64\x6F\x72",
 { 2, 0, 3, 2, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x73\x69\x64\x65\x20\x74\x68\x65\x20\x43\x68\x65\x73\x74",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x44\x61\x72\x6B\x20\x46\x6F\x72\x65\x73\x74",
 { 10, 10, 13, 10, 0, 0 } }, 
		{ 	"\x74\x61\x6C\x6C\x20\x54\x72\x65\x65\x74\x6F\x70",
 { 0, 0, 0, 0, 0, 10 } }, 
		{ 	"\x74\x61\x6C\x6C\x20\x54\x72\x65\x65\x74\x6F\x70",
 { 0, 0, 0, 0, 0, 13 } }, 
		{ 	"\x44\x61\x72\x6B\x20\x46\x6F\x72\x65\x73\x74",
 { 13, 14, 13, 10, 0, 0 } }, 
		{ 	"\x63\x61\x76\x65\x20\x65\x6E\x74\x72\x61\x6E\x63\x65",
 { 13, 15, 0, 0, 0, 0 } }, 
		{ 	"\x53\x74\x69\x6E\x6B\x69\x6E\x67\x20\x43\x61\x76\x65\x72\x6E",
 { 14, 0, 0, 35, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x52\x6F\x63\x6B\x79\x20\x4C\x65\x64\x67\x65",
 { 0, 0, 37, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E\x20\x70\x61\x74\x68",
 { 0, 18, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x20\x52\x61\x67\x69\x6E\x67\x20\x54\x6F\x72\x72\x65\x6E\x74",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x6D\x6F\x75\x6E\x74\x61\x69\x6E\x20\x74\x72\x61\x69\x6C",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x46\x61\x72\x6D\x2D\x59\x61\x72\x64",
 { 19, 0, 0, 0, 0, 0 } }, 
		{ 	"\x4C\x61\x72\x67\x65\x20\x48\x65\x6E\x20\x48\x6F\x75\x73\x65",
 { 20, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x6F\x70\x65\x6E\x20\x57\x6F\x6F\x64\x6C\x61\x6E\x64",
 { 21, 39, 23, 38, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x57\x6F\x6F\x64\x73",
 { 22, 0, 0, 0, 0, 0 } }, 
		{ 	"\x4E\x61\x72\x72\x6F\x77\x20\x74\x75\x6E\x6E\x65\x6C\x20\x62\x79\x20\x61\x20\x53\x68\x61\x66\x74",
 { 0, 0, 0, 23, 0, 27 } }, 
		{ 	"\x6E\x61\x72\x72\x6F\x77\x20\x74\x75\x6E\x6E\x65\x6C",
 { 26, 28, 25, 25, 0, 0 } }, 
		{ 	"\x6E\x61\x72\x72\x6F\x77\x20\x74\x75\x6E\x6E\x65\x6C",
 { 27, 25, 27, 26, 0, 0 } }, 
		{ 	"\x6E\x61\x72\x72\x6F\x77\x20\x74\x75\x6E\x6E\x65\x6C",
 { 26, 37, 26, 26, 0, 0 } }, 
		{ 	"\x6E\x61\x72\x72\x6F\x77\x20\x74\x75\x6E\x6E\x65\x6C",
 { 27, 34, 28, 25, 0, 0 } }, 
		{ 	"\x56\x61\x75\x6C\x74\x65\x64\x20\x63\x68\x61\x6D\x62\x65\x72\x20\x62\x79\x20\x61\x20\x57\x65\x6C\x6C\x2D\x53\x68\x61\x66\x74",
 { 0, 0, 30, 0, 0, 27 } }, 
		{ 	"\x4C\x69\x62\x72\x61\x72\x79",
 { 31, 0, 0, 29, 0, 0 } }, 
		{ 	"\x53\x74\x75\x64\x79\x20\x77\x69\x74\x68\x20\x61\x20\x64\x6F\x6F\x72\x20\x69\x6E\x20\x6E\x6F\x72\x74\x68\x20\x57\x61\x6C\x6C",
 { 0, 30, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x74\x27\x73\x20\x74\x68\x65\x20\x57\x69\x7A\x61\x72\x64\x27\x73\x20\x4C\x61\x69\x72",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x44\x55\x53\x54\x42\x49\x4E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6E\x61\x72\x72\x6F\x77\x20\x74\x75\x6E\x6E\x65\x6C",
 { 26, 37, 25, 28, 0, 0 } }, 
		{ 	"\x53\x6D\x65\x6C\x6C\x79\x20\x63\x61\x76\x65",
 { 36, 0, 15, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x74\x20\x61\x20\x44\x65\x61\x64\x2D\x45\x6E\x64",
 { 0, 35, 0, 0, 0, 0 } }, 
		{ 	"\x6E\x61\x72\x72\x6F\x77\x20\x74\x75\x6E\x6E\x65\x6C",
 { 27, 34, 37, 16, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x6F\x70\x65\x6E\x20\x57\x6F\x6F\x64\x6C\x61\x6E\x64",
 { 39, 38, 22, 39, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x6F\x70\x65\x6E\x20\x57\x6F\x6F\x64\x6C\x61\x6E\x64",
 { 22, 38, 38, 38, 0, 0 } }, 
		{ 	"\x2A\x41\x6C\x6C\x20\x6C\x69\x76\x65\x64\x20\x68\x61\x70\x70\x69\x6C\x79\x2E\x2E\x79\x6F\x75\x27\x72\x65\x20\x6A\x6F\x6B\x69\x6E\x67\x20\x21",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	0,
	7,
	0,
	0,
	0,
	0,
	0,
	1,
	0,
	2,
	6,
	4,
	5,
	17,
	18,
	18,
	18,
	20,
	21,
	22,
	22,
	23,
	23,
	10,
	30,
	0,
	0,
	0,
	0,
	8,
	0,
	15,
	0,
	12,
	0,
	0,
	19,
	0,
	0,
	19,
	20,
	0,
	0,
	0,
	32,
	31,
	0,
	7,
	0,
};


const char *objtext[] = {
	"\x20",
	"\x41\x20\x52\x75\x62\x79\x20\x52\x6F\x64",
	"\x53\x49\x47\x4E\x20\x2A\x20\x54\x72\x65\x61\x73\x75\x72\x65\x20\x69\x73\x20\x53\x61\x66\x65\x20\x48\x65\x72\x65\x20\x2A",
	"\x2A\x47\x6F\x6C\x64\x65\x6E\x20\x53\x63\x65\x70\x74\x72\x65\x2A",
	"\x43\x6F\x6E\x74\x65\x6E\x74\x65\x64\x20\x43\x68\x69\x63\x6B\x65\x6E\x73\x20\x69\x6E\x20\x4E\x65\x73\x74\x20\x42\x6F\x78\x65\x73",
	"\x49\x72\x6F\x6E\x20\x4B\x65\x79",
	"\x54\x72\x61\x70\x64\x6F\x6F\x72\x20\x69\x6E\x20\x66\x6C\x6F\x6F\x72",
	"\x49\x72\x6F\x6E\x2D\x62\x6F\x75\x6E\x64\x20\x57\x61\x6E\x64",
	"\x54\x68\x65\x20\x4B\x69\x6E\x67\x20\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x64\x69\x73\x74\x72\x61\x75\x67\x68\x74",
	"\x53\x70\x65\x63\x74\x61\x63\x6C\x65\x73\x20\x28\x77\x6F\x72\x6E\x29",
	"\x4F\x69\x6C\x2D\x50\x61\x69\x6E\x74\x69\x6E\x67",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x53\x70\x65\x63\x74\x61\x63\x6C\x65\x73",
	"\x53\x74\x61\x69\x72\x63\x61\x73\x65",
	"\x53\x68\x65\x65\x70\x73\x6B\x69\x6E\x20\x52\x75\x67",
	"\x43\x68\x69\x63\x6B\x65\x6E",
	"\x48\x75\x6E\x67\x72\x79\x2D\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x46\x6F\x78",
	"\x42\x61\x67\x20\x6F\x66\x20\x43\x6F\x72\x6E",
	"\x52\x69\x63\x6B\x65\x74\x79\x20\x42\x72\x69\x64\x67\x65",
	"\x41\x20\x46\x61\x72\x6D\x65\x72\x20\x28\x63\x6F\x75\x6E\x74\x69\x6E\x67\x20\x68\x69\x73\x20\x43\x68\x69\x63\x6B\x65\x6E\x73\x29",
	"\x46\x6C\x6F\x63\x6B\x20\x6F\x66\x20\x68\x75\x6E\x67\x72\x79\x20\x43\x68\x69\x63\x6B\x65\x6E\x73",
	"\x48\x75\x6E\x74\x65\x72\x73",
	"\x42\x61\x79\x69\x6E\x67\x20\x68\x6F\x75\x6E\x64\x73",
	"\x56\x69\x78\x65\x6E",
	"\x46\x6F\x78\x63\x75\x62\x73",
	"\x53\x68\x6F\x76\x65\x6C",
	"\x41\x6E\x20\x41\x6E\x63\x69\x65\x6E\x74\x20\x42\x6F\x6F\x6B",
	"\x53\x74\x72\x69\x70\x20\x6F\x66\x20\x50\x61\x72\x63\x68\x6D\x65\x6E\x74",
	"\x2A\x4A\x65\x77\x65\x6C\x6C\x65\x64\x20\x4F\x72\x62\x2A",
	"\x41\x6E\x20\x6F\x72\x6E\x61\x74\x65\x20\x45\x6C\x76\x69\x73\x68\x20\x53\x77\x6F\x72\x64",
	"\x41\x20\x66\x69\x6E\x65\x20\x63\x68\x61\x69\x6E",
	"\x41\x20\x4C\x6F\x63\x6B\x65\x64\x20\x43\x68\x65\x73\x74",
	"\x41\x20\x4C\x69\x6E\x65\x6E\x20\x43\x68\x65\x73\x74",
	"\x41\x20\x52\x65\x70\x75\x6C\x73\x69\x76\x65\x20\x53\x6C\x69\x6D\x79\x20\x47\x6F\x62\x6C\x69\x6E",
	"\x41\x20\x6E\x61\x72\x72\x6F\x77\x20\x74\x75\x6E\x6E\x65\x6C",
	"\x52\x61\x76\x65\x6E\x27\x73\x20\x4E\x65\x73\x74",
	"\x41\x20\x73\x6D\x61\x6C\x6C\x20\x64\x6F\x6F\x72\x20\x28\x6E\x6F\x77\x20\x76\x69\x73\x69\x62\x6C\x65\x29",
	"\x2A\x54\x68\x65\x20\x42\x65\x61\x75\x74\x69\x66\x75\x6C\x20\x50\x72\x69\x6E\x63\x65\x73\x73\x2A",
	"\x41\x20\x52\x69\x63\x6B\x65\x74\x79\x20\x42\x72\x69\x64\x67\x65",
	"\x41\x20\x53\x74\x65\x65\x70\x20\x70\x61\x74\x68",
	"\x41\x20\x48\x61\x70\x70\x79\x20\x46\x61\x72\x6D\x65\x72",
	"\x41\x20\x6C\x61\x72\x67\x65\x20\x72\x6F\x63\x6B\x20\x62\x6C\x6F\x63\x6B\x69\x6E\x67\x20\x74\x68\x65\x20\x70\x61\x74\x68",
	"\x41\x20\x6C\x61\x72\x67\x65\x20\x68\x65\x6E\x20\x68\x6F\x75\x73\x65",
	"\x54\x65\x72\x72\x69\x66\x69\x65\x64\x20\x43\x68\x69\x63\x6B\x65\x6E\x73\x20\x28\x74\x68\x65\x79\x20\x73\x65\x65\x20\x74\x68\x65\x20\x46\x6F\x78\x29",
	"\x4F\x70\x65\x6E\x20\x54\x72\x61\x70\x64\x6F\x6F\x72",
	"\x2A\x52\x6F\x79\x61\x6C\x20\x43\x72\x6F\x77\x6E\x2A",
	"\x45\x76\x69\x6C\x20\x41\x6B\x79\x72\x7A\x20\x28\x48\x75\x72\x74\x69\x6E\x67\x20\x74\x68\x65\x20\x50\x72\x69\x6E\x63\x65\x73\x73\x29",
	"\x41\x20\x4D\x65\x6E\x61\x63\x69\x6E\x67\x20\x53\x74\x6F\x6E\x65\x20\x54\x72\x6F\x6C\x6C",
	"\x53\x63\x6F\x72\x63\x68\x65\x64\x20\x53\x74\x61\x74\x75\x65\x20\x6F\x66\x20\x61\x20\x54\x72\x6F\x6C\x6C",
	"\x52\x65\x67\x61\x6C\x69\x61\x20\x73\x63\x61\x74\x74\x65\x72\x65\x64\x20\x61\x6C\x6C\x20\x61\x72\x6F\x75\x6E\x64",
	"\x54\x68\x65\x20\x47\x72\x61\x74\x65\x66\x75\x6C\x20\x4B\x69\x6E\x67",
};
const char *msgptr[] = {
	"\x28\x63\x29\x20\x31\x39\x38\x33\x20\x42\x2E\x48\x6F\x77\x61\x72\x74\x68\x20\x26\x20\x43\x2E\x4A\x2E\x20\x4F\x67\x64\x65\x6E",
	"\x54\x68\x65\x20\x4B\x69\x6E\x67\x20\x73\x61\x79\x73\x3A",
	"\x27\x53\x61\x76\x65\x20\x74\x68\x65\x20\x50\x72\x69\x6E\x63\x65\x73\x73\x20\x26\x20\x66\x69\x6E\x64\x20\x74\x68\x65\x20\x4A\x65\x77\x65\x6C\x73",
	"\x49\x20\x68\x69\x64\x20\x74\x68\x65\x20\x70\x61\x72\x63\x68\x6D\x65\x6E\x74\x20\x69\x6E\x20\x74\x68\x65\x20\x62\x6F\x6F\x6B\x21",
	"\x59\x6F\x75\x72\x20\x72\x65\x77\x61\x72\x64\x20\x77\x69\x6C\x6C\x20\x62\x65\x20\x20\x50\x72\x69\x63\x65\x6C\x65\x73\x73\x21\x22",
	"\x4F\x2E\x4B\x2E",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x6D\x69\x73\x73\x69\x6E\x67",
	"\x54\x68\x65\x20\x73\x69\x64\x65\x73\x20\x61\x72\x65\x20\x6C\x6F\x6F\x73\x65",
	"\x49\x74\x20\x68\x61\x6E\x67\x73\x20\x62\x79\x20\x61\x20\x66\x69\x6E\x65\x20\x63\x68\x61\x69\x6E",
	"\x49\x74\x27\x73\x20\x45\x6D\x70\x74\x79",
	"\x41\x20\x76\x6F\x69\x63\x65",
	"\x73\x61\x79\x73\x3A",
	"\x22\x52\x41\x56\x45\x4E\x53\x20\x47\x75\x61\x72\x64\x20\x74\x68\x65\x20\x54\x72\x65\x61\x73\x75\x72\x65\x73\x22",
	"\x49\x74\x27\x73\x20\x74\x6F\x6F\x20\x73\x6D\x61\x6C\x6C\x21",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C",
	"\x49\x20\x63\x61\x6E\x20\x73\x65\x65\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21",
	"\x53\x6C\x61\x76\x65\x72\x69\x6E\x67\x20\x54\x72\x6F\x6C\x6C\x20\x70\x72\x6F\x63\x65\x65\x64\x73\x20\x74\x6F\x20\x65\x61\x74\x20\x4D\x45\x21",
	"\x54\x72\x6F\x6C\x6C\x20\x67\x72\x69\x6D\x61\x63\x65\x73\x2E\x2E\x61\x6E\x64\x20\x62\x61\x72\x73\x20\x74\x68\x65\x20\x77\x61\x79",
	"\x49\x74\x27\x73\x20\x4F\x70\x65\x6E",
	"\x49\x74\x27\x73\x20\x4C\x6F\x63\x6B\x65\x64",
	"\x55\x75\x75\x67\x67\x68\x2E\x2E\x49\x20\x63\x61\x6E\x20\x6A\x75\x73\x74\x20\x73\x71\x75\x65\x65\x7A\x65\x20\x62\x79\x20\x74\x68\x65\x20\x54\x72\x6F\x6C\x6C",
	"\x41\x20\x54\x61\x70\x65\x73\x74\x72\x79\x20\x68\x61\x6E\x67\x73\x20\x6F\x6E\x20\x74\x68\x65\x20\x77\x61\x6C\x6C",
	"\x43\x68\x61\x69\x6E\x20\x63\x6C\x69\x70\x73\x20\x6E\x65\x61\x74\x6C\x79\x20\x6F\x6E\x20\x73\x70\x65\x63\x73\x21",
	"\x4C\x69\x67\x68\x74\x20\x69\x73\x20\x42\x6C\x69\x6E\x64\x69\x6E\x67\x21",
	"\x54\x68\x65\x79\x20\x6D\x75\x73\x74\x20\x62\x65\x20\x73\x70\x65\x63\x69\x61\x6C\x20\x73\x70\x65\x63\x73\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x73\x69\x63\x6B\x65\x6E\x69\x6E\x67\x20\x63\x72\x75\x6E\x63\x68\x20\x6F\x66\x20\x67\x6C\x61\x73\x73\x21",
	"\x59\x6F\x75\x27\x72\x65\x20\x61\x20\x42\x6C\x6F\x6F\x64\x74\x68\x69\x72\x73\x74\x79\x20\x44\x65\x76\x69\x6C\x20\x61\x72\x65\x6E\x27\x74\x20\x79\x6F\x75\x21",
	"\x49\x20\x63\x61\x6E\x20\x73\x65\x65\x20\x65\x76\x65\x72\x79\x74\x68\x69\x6E\x67\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x6C\x6F\x75\x64\x20\x22\x43\x6C\x69\x63\x6B\x22\x20\x6E\x65\x61\x72\x62\x79\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x6E\x65\x73\x74\x20\x69\x6E\x20\x74\x68\x65\x20\x6E\x65\x78\x74\x20\x74\x72\x65\x65\x21",
	"\x44\x65\x6C\x69\x63\x69\x6F\x75\x73\x21\x20\x54\x68\x61\x6E\x6B\x73",
	"\x49\x20\x4B\x69\x6C\x6C\x65\x64\x20\x69\x74\x21",
	"\x55\x67\x68\x21\x20\x59\x6F\x75\x27\x72\x65\x20\x4E\x61\x75\x73\x65\x61\x74\x69\x6E\x67\x21",
	"\x54\x68\x65\x20\x53\x77\x6F\x72\x64\x20\x67\x6C\x6F\x77\x73\x20\x46\x69\x65\x72\x63\x65\x6C\x79\x21",
	"\x52\x75\x62\x79\x20\x52\x6F\x64\x20\x76\x61\x6E\x69\x73\x68\x65\x73\x20\x69\x6E\x74\x6F\x20\x53\x77\x6F\x72\x64\x20\x68\x69\x6C\x74\x21",
	"\x54\x68\x65\x20\x53\x77\x6F\x72\x64\x20\x6C\x61\x63\x6B\x73\x20\x74\x68\x65\x20\x50\x6F\x77\x65\x72\x21",
	"\x53\x77\x6F\x72\x64\x20\x45\x72\x75\x70\x74\x73\x20\x69\x6E\x74\x6F\x20\x42\x72\x69\x6C\x6C\x69\x61\x6E\x74\x20\x57\x68\x69\x74\x65\x20\x46\x69\x72\x65\x21",
	"\x49\x20\x63\x61\x6E\x20\x68\x65\x61\x72\x20\x57\x61\x76\x65\x73\x21",
	"\x46\x6F\x78\x20\x65\x61\x74\x73\x20\x43\x68\x69\x63\x6B\x65\x6E\x21",
	"\x43\x68\x69\x63\x6B\x65\x6E\x20\x65\x61\x74\x73\x20\x43\x6F\x72\x6E\x21",
	"\x48\x65\x20\x6C\x65\x61\x64\x73\x20\x6D\x65\x20\x69\x6E\x74\x6F\x20\x61\x20\x63\x6F\x6E\x63\x65\x61\x6C\x65\x64\x20\x63\x61\x76\x65",
	"\x53\x6E\x61\x72\x6C\x69\x6E\x67\x20\x48\x6F\x75\x6E\x64\x73\x20\x70\x72\x65\x76\x65\x6E\x74\x20\x6D\x65\x21",
	"\x48\x65\x20\x67\x69\x76\x65\x73\x20\x6D\x65\x20\x61\x6E\x20\x49\x72\x6F\x6E\x62\x6F\x75\x6E\x64\x20\x53\x74\x61\x66\x66",
	"\x49\x20\x63\x61\x6E\x20\x6F\x6E\x6C\x79\x20\x74\x61\x6B\x65\x20\x6F\x6E\x65\x20\x61\x74\x20\x61\x20\x74\x69\x6D\x65\x21",
	"\x48\x75\x6E\x74\x65\x72\x20\x73\x61\x79\x73\x20\x27\x57\x65\x27\x76\x65\x20\x6C\x6F\x73\x74\x20\x74\x68\x65\x20\x73\x63\x65\x6E\x74\x21\x27",
	"\x46\x6F\x78\x20\x66\x6C\x65\x65\x73\x20\x77\x69\x74\x68\x20\x48\x75\x6E\x74\x20\x69\x6E\x20\x66\x75\x6C\x6C\x20\x43\x72\x79\x2E\x2E",
	"\x46\x6F\x78\x20\x74\x72\x6F\x74\x73\x20\x61\x77\x61\x79\x2C\x20\x73\x69\x74\x73\x20\x61\x6E\x64\x20\x6C\x6F\x6F\x6B\x73\x20\x61\x74\x20\x6D\x65\x21",
	"\x4C\x61\x75\x67\x68\x73\x20\x68\x6F\x72\x72\x69\x62\x6C\x79\x2E\x2E\x54\x68\x65\x6E\x20\x74\x65\x61\x72\x73\x20\x6D\x65\x20\x61\x70\x61\x72\x74\x21",
	"\x74\x68\x65\x6E\x20\x64\x69\x73\x61\x70\x70\x65\x61\x72\x73\x20\x69\x6E\x74\x6F\x20\x68\x69\x73\x20\x44\x65\x6E",
	"\x49\x27\x6D\x20\x66\x6C\x6F\x61\x74\x69\x6E\x67\x20\x75\x70\x20\x74\x68\x65\x20\x53\x68\x61\x66\x74\x21",
	"\x4D\x75\x73\x74\x20\x62\x65\x20\x74\x68\x65\x20\x57\x69\x7A\x61\x72\x64\x27\x73\x20\x57\x61\x6E\x64\x21",
	"\x41\x20\x73\x74\x72\x69\x70\x20\x6F\x66\x20\x70\x61\x70\x65\x72\x20\x66\x6C\x69\x65\x73\x20\x69\x6E\x74\x6F\x20\x6D\x79\x20\x68\x61\x6E\x64\x21",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x4D\x59\x53\x54\x45\x52\x49\x4F\x55\x53\x20\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x20\x4E\x6F\x2E\x20\x38",
	"\x62\x79\x20\x42\x72\x69\x61\x6E\x20\x48\x6F\x77\x61\x72\x74\x68\x20\x26\x20\x43\x6C\x69\x66\x66\x20\x4F\x67\x64\x65\x6E\x20\x28\x43\x29\x20\x31\x39\x38\x33",
	"\x54\x61\x6B\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x61\x74\x20\x66\x61\x63\x65\x20\x76\x61\x6C\x75\x65\x21",
	"\x49\x74\x27\x73\x20\x61\x20\x70\x6F\x72\x74\x72\x61\x69\x74\x20\x6F\x66\x20\x74\x68\x65\x20\x50\x72\x69\x6E\x63\x65\x73\x73",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x66\x69\x6E\x65\x20\x68\x61\x6E\x67\x69\x6E\x67\x2D\x63\x68\x61\x69\x6E",
	"\x59\x6F\x75\x20\x68\x61\x76\x65\x6E\x27\x74\x20\x6D\x75\x63\x68\x20\x74\x69\x6D\x65\x20\x6C\x65\x66\x74\x21",
	"\x54\x68\x65\x20\x62\x61\x63\x6B\x20\x6F\x66\x20\x74\x68\x65\x20\x43\x68\x65\x73\x74\x20\x53\x6E\x61\x70\x73\x20\x6F\x70\x65\x6E\x21",
	"\x49\x27\x6D\x20\x72\x6F\x6C\x6C\x69\x6E\x67\x20\x64\x6F\x77\x6E\x20\x61\x20\x70\x61\x73\x73\x61\x67\x65\x21",
	"\x54\x68\x65\x20\x53\x70\x65\x63\x74\x61\x63\x6C\x65\x73\x20\x68\x61\x76\x65\x20\x66\x61\x6C\x6C\x65\x6E\x20\x6F\x66\x66\x21",
	"\x4F\x68\x68\x68\x68\x21\x20\x4D\x79\x20\x48\x65\x61\x64\x21\x2E\x2E\x2E\x57\x68\x65\x72\x65\x20\x61\x6D\x20\x49\x3F",
	"\x54\x68\x65\x20\x6C\x6F\x61\x74\x68\x73\x6F\x6D\x65\x20\x43\x72\x65\x61\x74\x75\x72\x65",
	"\x74\x65\x61\x72\x73\x20\x6D\x79\x20\x74\x68\x72\x6F\x61\x74\x20\x6F\x75\x74\x21\x21",
	"\x64\x65\x63\x61\x79\x73\x20\x69\x6E\x74\x6F\x20\x61\x20\x50\x75\x74\x72\x69\x64\x20\x47\x72\x65\x65\x6E\x20\x53\x6C\x69\x6D\x65\x21",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x48\x61\x70\x70\x65\x6E\x73",
	"\x49\x74\x20\x77\x61\x73\x20\x61\x20\x67\x6F\x6F\x64\x20\x49\x64\x65\x61\x2C\x20\x62\x75\x74\x2E\x2E\x2E",
	"\x49\x74\x27\x73\x20\x62\x65\x65\x6E\x20\x61\x6E\x74\x69\x63\x69\x70\x61\x74\x65\x64\x21",
	"\x43\x68\x69\x63\x6B\x65\x6E\x20\x66\x6C\x61\x70\x73\x20\x61\x77\x61\x79",
	"\x61\x6E\x64\x20\x72\x75\x73\x68\x65\x73\x20\x69\x6E\x74\x6F\x20\x74\x68\x65\x20\x48\x65\x6E\x2D\x43\x61\x62\x69\x6E\x21",
	"\x49\x20\x63\x61\x6E\x27\x74\x21\x20\x49\x74\x27\x73\x20\x6A\x61\x6D\x6D\x65\x64\x20\x73\x6F\x6C\x69\x64\x21",
	"\x49\x20\x6D\x75\x73\x74\x20\x68\x61\x76\x65\x20\x6C\x6F\x6F\x73\x65\x6E\x65\x64\x20\x69\x74\x21",
	"\x49\x74\x20\x63\x72\x61\x73\x68\x65\x73\x20\x6F\x66\x66\x20\x74\x68\x65\x20\x6C\x65\x64\x67\x65\x21",
	"\x46\x61\x72\x6D\x65\x72\x20\x74\x68\x61\x6E\x6B\x73\x20\x6D\x65",
	"\x49\x72\x61\x74\x65\x20\x46\x61\x72\x6D\x65\x72\x20\x79\x65\x6C\x6C\x73\x20\x27\x43\x68\x69\x63\x6B\x65\x6E\x20\x74\x68\x69\x65\x66\x20\x45\x68\x3F\x27",
	"\x54\x68\x65\x6E\x20\x68\x65\x20\x6B\x6E\x6F\x63\x6B\x73\x20\x6D\x65\x20\x6F\x75\x74\x20\x43\x4F\x4C\x44\x21",
	"\x49\x20\x68\x61\x76\x65\x20\x6E\x6F\x20\x77\x65\x61\x70\x6F\x6E\x73\x21",
	"\x49\x27\x76\x65\x20\x67\x6F\x74\x20\x61\x20\x22\x46\x6F\x78\x20\x69\x6E\x20\x53\x68\x65\x65\x70\x27\x73\x20\x63\x6C\x6F\x74\x68\x69\x6E\x67\x21\x22",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x68\x61\x76\x65\x20\x61\x20\x6B\x65\x79",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x68\x61\x76\x65\x20\x61\x20\x57\x61\x6E\x64\x21",
	"\x54\x68\x6F\x75\x20\x61\x72\x74\x20\x61\x20\x46\x6F\x75\x6C\x20\x4D\x6F\x75\x74\x68\x65\x64\x20\x4B\x6E\x61\x76\x65\x21",
	"\x56\x69\x78\x65\x6E\x20\x67\x75\x65\x73\x73\x65\x73\x20\x6D\x79\x20\x69\x6E\x74\x65\x6E\x74\x69\x6F\x6E",
	"\x61\x6E\x64\x20\x73\x63\x75\x72\x72\x69\x65\x73\x20\x61\x77\x61\x79\x20\x77\x69\x74\x68\x20\x68\x65\x72\x20\x43\x75\x62\x73\x21",
	"\x54\x68\x65\x20\x46\x6F\x78\x20\x6C\x6F\x6F\x6B\x73\x20\x61\x74\x20\x6D\x65\x20\x73\x63\x6F\x72\x6E\x66\x75\x6C\x6C\x79\x21\x2E\x2E\x2E\x2E",
	"\x48\x75\x6E\x74\x65\x72\x73\x20\x61\x74\x74\x61\x63\x6B\x20\x6D\x65\x20\x61\x6E\x67\x72\x69\x6C\x79\x21",
	"\x22\x2E\x2E\x2E\x20\x74\x6F\x20\x61\x76\x6F\x69\x64\x20\x63\x65\x72\x74\x61\x69\x6E\x20\x44\x65\x61\x74\x68\x20\x79\x6F\x75\x20\x6D\x75\x73\x74",
	"\x50\x61\x73\x73\x20\x74\x68\x65\x20\x52\x75\x6E\x65\x73\x20\x74\x6F\x20\x74\x68\x65\x20\x45\x76\x69\x6C\x20\x4F\x6E\x65",
	"\x62\x65\x66\x6F\x72\x65\x20\x79\x6F\x75\x72\x20\x51\x75\x65\x73\x74\x20\x69\x73\x20\x65\x6E\x64\x65\x64\x21\x22",
	"\x49\x74\x20\x69\x73\x20\x75\x6E\x64\x65\x63\x69\x70\x68\x65\x72\x61\x62\x6C\x65",
	"\x54\x68\x65\x20\x57\x69\x7A\x61\x72\x64\x20\x67\x72\x61\x62\x73\x20\x74\x68\x65\x20\x62\x6F\x6F\x6B\x20\x28\x61\x6E\x64\x20\x52\x75\x6E\x65\x73\x29",
	"\x43\x75\x72\x73\x65\x73\x20\x6D\x65\x2C\x20\x74\x68\x65\x6E\x20\x66\x6C\x65\x65\x73\x20\x66\x72\x6F\x6D\x20\x61\x20\x48\x75\x67\x65\x20\x44\x65\x6D\x6F\x6E",
	"\x28\x48\x69\x64\x65\x20\x27\x65\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x42\x6F\x6F\x6B\x21\x29",
	"\x50\x61\x72\x63\x68\x6D\x65\x6E\x74\x20\x77\x6F\x6E\x27\x74\x20\x6C\x65\x61\x76\x65\x20\x6D\x79\x20\x68\x61\x6E\x64\x21",
	"\x49\x74\x20\x68\x61\x73\x20\x73\x74\x72\x61\x6E\x67\x65\x20\x52\x75\x6E\x65\x73\x21\x20\x69\x74\x20\x66\x65\x65\x6C\x73\x20\x41\x4C\x49\x56\x45\x21",
	"\x54\x68\x65\x20\x57\x69\x7A\x61\x72\x64\x27\x73\x20\x50\x6F\x77\x65\x72\x20\x69\x73\x20\x6E\x6F\x20\x6D\x6F\x72\x65\x21",
	"\x54\x68\x65\x20\x50\x72\x69\x6E\x63\x65\x73\x73\x20\x69\x73\x20\x46\x72\x65\x65\x21",
	"\x57\x69\x7A\x61\x72\x64\x20\x73\x61\x79\x73\x20\x27\x49\x6E\x20\x33\x20\x6D\x6F\x76\x65\x73\x20\x79\x6F\x75\x20\x64\x69\x65\x21\x27",
	"\x54\x69\x6D\x65\x27\x73\x20\x75\x70\x20\x4D\x6F\x6E\x73\x74\x72\x6F\x75\x73\x20\x44\x65\x6D\x6F\x6E\x20\x61\x70\x70\x65\x61\x72\x73",
	"\x59\x6F\x75\x20\x53\x75\x63\x63\x65\x65\x64\x65\x64\x20\x4D\x41\x53\x54\x45\x52\x20\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x52\x21",
	"\x27\x51\x55\x45\x53\x54\x4F\x52\x27\x20\x69\x73\x20\x79\x6F\x75\x72\x20\x72\x65\x77\x61\x72\x64\x20\x52\x65\x6D\x65\x6D\x62\x65\x72\x20\x69\x74\x21",
};


const uint8_t status[] = {
	166, 
	9, 1, 
	102, 103, 73, 
	199, 
	0, 1, 
	58, 1, 2, 4, 
	182, 
	8, 3, 17, 30, 0, 30, 0, 31, 0, 8, 
	55, 62, 28, 
	165, 
	4, 25, 
	56, 64, 
	165, 
	4, 26, 
	56, 64, 
	168, 
	4, 22, 13, 20, 
	44, 
	178, 
	6, 14, 6, 15, 6, 16, 0, 5, 
	81, 79, 73, 
	196, 
	0, 5, 
	81, 
	182, 
	7, 32, 7, 7, 0, 7, 0, 4, 0, 7, 
	81, 79, 81, 
	175, 
	2, 45, 4, 32, 0, 7, 
	81, 77, 0, 73, 
	199, 
	19, 0, 
	147, 47, 61, 63, 
	197, 
	0, 7, 
	81, 107, 
	180, 
	18, 16, 18, 14, 18, 15, 4, 19, 0, 7, 
	58, 
	177, 
	4, 26, 1, 14, 0, 14, 0, 17, 
	62, 73, 
	206, 
	0, 5, 0, 1, 0, 5, 
	81, 83, 81, 
	182, 
	4, 26, 8, 8, 0, 8, 0, 14, 0, 17, 
	60, 62, 111, 
	172, 
	4, 14, 1, 28, 13, 32, 
	33, 
	172, 
	4, 35, 1, 28, 13, 32, 
	33, 
	165, 
	4, 16, 
	57, 64, 
	165, 
	4, 37, 
	56, 64, 
	165, 
	4, 29, 
	56, 64, 
	165, 
	4, 15, 
	56, 64, 
	178, 
	6, 9, 0, 1, 0, 3, 0, 1, 
	81, 79, 81, 
	174, 
	1, 9, 9, 4, 0, 1, 
	81, 77, 73, 
	207, 
	19, 0, 0, 9, 0, 11, 
	55, 53, 110, 73, 
	197, 
	0, 1, 
	81, 64, 
	133, 50, 
	4, 11, 
	15, 29, 
	165, 
	4, 40, 
	0, 63, 
	135, 30, 
	2, 32, 
	88, 112, 113, 73, 
	195, 
	88, 61, 88, 63, 
	168, 
	9, 15, 1, 9, 
	23, 
	165, 
	4, 14, 
	57, 64, 
	164, 
	4, 24, 
	37, 
	168, 
	4, 23, 2, 15, 
	46, 
	181, 
	7, 23, 0, 22, 0, 23, 0, 23, 0, 23, 
	62, 62, 
	169, 
	4, 27, 0, 15, 
	56, 64, 
	183, 
	4, 7, 2, 36, 2, 27, 2, 3, 2, 44, 
	0, 0, 0, 73, 
	203, 
	0, 49, 0, 49, 
	74, 53, 64, 86, 
	195, 
	1, 148, 149, 63, 
	181, 
	8, 11, 7, 23, 0, 15, 0, 23, 0, 11, 
	62, 60, 
};
const uint8_t actions[] = {
	19, 10, 33, 
	4, 23, 2, 15, 0, 15, 0, 11, 
	55, 58, 133, 48, 
	13, 29, 72, 
	2, 46, 1, 28, 9, 13, 
	35, 6, 
	18, 18, 14, 
	1, 9, 0, 9, 0, 9, 0, 11, 
	53, 72, 5, 
	9, 18, 14, 
	1, 11, 0, 11, 
	53, 5, 
	23, 10, 10, 
	9, 2, 3, 10, 0, 29, 0, 29, 0, 2, 
	53, 52, 58, 5, 
	13, 10, 8, 
	2, 10, 8, 3, 0, 10, 
	52, 105, 
	9, 27, 12, 
	4, 8, 2, 31, 
	0, 18, 
	4, 23, 12, 
	2, 31, 
	9, 
	8, 23, 8, 
	1, 10, 8, 2, 
	105, 
	9, 23, 8, 
	1, 10, 9, 2, 
	105, 8, 
	0, 4, 0, 
	71, 
	11, 1, 12, 
	2, 31, 0, 9, 
	54, 108, 109, 73, 
	199, 
	0, 10, 
	88, 88, 54, 64, 
	17, 10, 14, 
	9, 15, 2, 11, 9, 4, 0, 11, 
	52, 7, 
	14, 15, 14, 
	1, 9, 0, 11, 0, 9, 
	72, 5, 64, 
	19, 29, 16, 
	2, 32, 1, 28, 0, 32, 0, 5, 
	72, 31, 112, 114, 
	4, 29, 16, 
	6, 28, 
	126, 
	19, 33, 31, 
	1, 16, 0, 5, 0, 1, 0, 5, 
	81, 83, 81, 73, 
	197, 
	0, 16, 
	55, 30, 
	0, 5, 0, 
	66, 
	23, 13, 14, 
	1, 11, 8, 15, 0, 9, 0, 11, 0, 9, 
	72, 74, 27, 24, 
	22, 13, 14, 
	1, 11, 9, 15, 0, 9, 0, 11, 0, 9, 
	72, 74, 24, 
	22, 41, 10, 
	1, 9, 1, 29, 0, 29, 0, 33, 0, 4, 
	62, 58, 22, 
	23, 41, 10, 
	1, 11, 1, 29, 0, 29, 0, 33, 0, 4, 
	62, 58, 0, 22, 
	17, 10, 14, 
	2, 11, 9, 15, 8, 4, 0, 11, 
	52, 106, 
	13, 10, 7, 
	2, 28, 9, 13, 0, 28, 
	52, 6, 
	13, 10, 7, 
	2, 28, 8, 13, 0, 28, 
	52, 33, 
	8, 23, 7, 
	1, 28, 9, 13, 
	6, 
	8, 23, 7, 
	1, 28, 8, 13, 
	33, 
	23, 10, 8, 
	2, 10, 9, 2, 0, 10, 9, 3, 0, 3, 
	52, 105, 8, 58, 
	22, 10, 8, 
	2, 10, 8, 2, 9, 3, 0, 10, 0, 3, 
	52, 58, 105, 
	13, 10, 14, 
	8, 15, 2, 11, 0, 11, 
	55, 25, 
	13, 10, 10, 
	2, 29, 8, 2, 0, 29, 
	52, 5, 
	0, 33, 0, 
	32, 
	23, 62, 0, 
	4, 36, 14, 3, 1, 24, 0, 3, 0, 36, 
	62, 10, 11, 12, 
	17, 62, 0, 
	4, 36, 13, 3, 1, 24, 0, 33, 
	15, 53, 
	13, 62, 0, 
	1, 24, 13, 33, 13, 44, 
	5, 14, 
	9, 1, 41, 
	2, 33, 0, 25, 
	54, 64, 
	14, 23, 37, 
	4, 12, 14, 27, 0, 27, 
	15, 53, 12, 
	7, 23, 18, 
	4, 1, 
	1, 2, 0, 4, 
	13, 43, 13, 
	7, 7, 7, 18, 7, 19, 
	85, 73, 
	201, 
	0, 1, 0, 7, 
	87, 54, 
	10, 43, 13, 
	4, 7, 0, 1, 
	87, 85, 64, 
	18, 62, 0, 
	4, 34, 1, 24, 14, 44, 0, 44, 
	53, 15, 12, 
	0, 3, 0, 
	63, 
	4, 1, 37, 
	4, 12, 
	13, 
	8, 38, 0, 
	4, 16, 0, 17, 
	54, 
	23, 10, 43, 
	2, 14, 0, 5, 0, 1, 0, 5, 0, 14, 
	81, 82, 81, 52, 
	23, 10, 33, 
	2, 15, 0, 5, 0, 1, 0, 5, 0, 15, 
	81, 82, 81, 52, 
	23, 10, 31, 
	2, 16, 0, 5, 0, 1, 0, 5, 0, 16, 
	81, 82, 81, 52, 
	23, 18, 43, 
	1, 14, 7, 20, 0, 5, 0, 1, 0, 5, 
	81, 83, 81, 73, 
	196, 
	0, 14, 
	53, 
	21, 18, 33, 
	1, 15, 7, 21, 7, 22, 7, 7, 0, 15, 
	53, 73, 
	211, 
	0, 8, 0, 5, 0, 1, 0, 5, 
	60, 81, 83, 81, 
	23, 18, 31, 
	1, 16, 0, 5, 0, 1, 0, 5, 0, 16, 
	81, 83, 81, 53, 
	23, 1, 34, 
	4, 18, 2, 14, 2, 15, 2, 16, 0, 16, 
	55, 39, 38, 73, 
	202, 
	0, 14, 0, 19, 
	55, 54, 64, 
	23, 1, 34, 
	4, 18, 2, 14, 2, 15, 0, 14, 0, 19, 
	55, 54, 64, 38, 
	23, 1, 34, 
	4, 18, 2, 14, 2, 16, 0, 16, 0, 19, 
	55, 54, 64, 39, 
	9, 1, 34, 
	4, 18, 0, 5, 
	81, 73, 
	204, 
	15, 1, 0, 19, 0, 19, 
	54, 
	196, 
	16, 1, 
	43, 
	196, 
	0, 5, 
	81, 
	11, 43, 13, 
	1, 14, 7, 20, 
	85, 115, 116, 117, 
	12, 1, 35, 
	4, 19, 2, 38, 0, 20, 
	54, 
	7, 43, 13, 
	4, 18, 
	85, 115, 116, 117, 
	7, 43, 13, 
	4, 19, 
	85, 115, 116, 117, 
	23, 1, 34, 
	4, 19, 2, 14, 2, 15, 2, 16, 0, 16, 
	55, 39, 38, 73, 
	202, 
	0, 14, 0, 18, 
	55, 54, 64, 
	23, 1, 34, 
	4, 19, 2, 14, 2, 15, 0, 14, 0, 18, 
	55, 54, 64, 38, 
	23, 1, 34, 
	4, 19, 2, 14, 2, 16, 0, 16, 0, 18, 
	55, 54, 64, 39, 
	9, 1, 34, 
	4, 19, 0, 5, 
	81, 73, 
	201, 
	15, 1, 0, 18, 
	54, 64, 
	196, 
	16, 1, 
	43, 
	196, 
	0, 5, 
	81, 
	8, 57, 63, 
	4, 19, 9, 7, 
	120, 
	19, 57, 63, 
	4, 19, 8, 7, 0, 38, 0, 40, 
	72, 121, 122, 15, 
	19, 18, 43, 
	1, 14, 4, 20, 0, 14, 0, 7, 
	72, 118, 119, 73, 
	211, 
	0, 5, 0, 1, 0, 5, 0, 8, 
	81, 83, 81, 60, 
	207, 
	0, 18, 0, 39, 0, 7, 
	72, 74, 123, 42, 
	19, 35, 55, 
	1, 7, 7, 24, 7, 29, 6, 26, 
	10, 11, 2, 137, 
	21, 1, 64, 
	4, 20, 13, 7, 9, 8, 1, 15, 0, 43, 
	55, 73, 
	215, 
	0, 6, 0, 4, 0, 19, 0, 42, 0, 21, 
	55, 55, 55, 62, 
	201, 
	0, 21, 0, 9, 
	54, 58, 
	20, 1, 64, 
	4, 20, 13, 7, 8, 8, 8, 9, 9, 10, 
	73, 
	210, 
	0, 42, 0, 19, 0, 21, 0, 9, 
	72, 54, 60, 
	20, 1, 64, 
	4, 20, 13, 7, 9, 8, 8, 9, 9, 10, 
	73, 
	210, 
	0, 42, 0, 19, 0, 21, 0, 9, 
	72, 54, 60, 
	20, 1, 64, 
	4, 20, 13, 7, 9, 8, 8, 9, 8, 10, 
	73, 
	214, 
	0, 42, 0, 4, 0, 9, 0, 6, 0, 21, 
	72, 60, 62, 
	196, 
	0, 21, 
	54, 
	20, 1, 64, 
	4, 20, 13, 7, 8, 8, 8, 9, 8, 10, 
	73, 
	214, 
	0, 42, 0, 4, 0, 9, 0, 6, 0, 21, 
	72, 60, 62, 
	196, 
	0, 21, 
	54, 
	20, 1, 64, 
	4, 20, 13, 7, 9, 8, 9, 9, 6, 15, 
	73, 
	196, 
	0, 21, 
	54, 
	22, 18, 26, 
	8, 8, 7, 21, 7, 7, 0, 8, 0, 13, 
	60, 53, 5, 
	12, 18, 26, 
	9, 8, 7, 21, 0, 13, 
	53, 
	16, 1, 64, 
	4, 20, 13, 7, 8, 8, 0, 21, 
	54, 
	11, 1, 64, 
	4, 20, 14, 7, 
	124, 125, 88, 73, 
	203, 
	0, 8, 0, 26, 
	58, 88, 54, 64, 
	5, 36, 0, 
	10, 0, 
	5, 115, 
	0, 6, 0, 
	65, 
	17, 63, 33, 
	1, 15, 1, 13, 7, 21, 0, 8, 
	58, 127, 
	23, 56, 43, 
	2, 19, 1, 16, 0, 19, 0, 4, 0, 6, 
	72, 53, 15, 73, 
	205, 
	0, 16, 0, 33, 0, 10, 
	62, 58, 
	206, 
	0, 5, 0, 1, 0, 5, 
	81, 83, 81, 
	19, 27, 66, 
	2, 6, 1, 5, 0, 6, 0, 43, 
	72, 5, 0, 18, 
	10, 27, 66, 
	2, 6, 6, 5, 
	0, 19, 128, 
	11, 1, 66, 
	2, 43, 0, 22, 
	109, 0, 88, 54, 
	14, 35, 55, 
	4, 24, 1, 7, 0, 29, 
	54, 49, 50, 
	23, 18, 33, 
	4, 22, 1, 15, 0, 15, 0, 20, 0, 21, 
	55, 55, 55, 73, 
	193, 
	64, 45, 
	19, 55, 33, 
	4, 23, 2, 15, 0, 15, 0, 24, 
	53, 54, 40, 48, 
	15, 35, 55, 
	1, 26, 1, 7, 7, 29, 
	10, 11, 136, 141, 
	4, 35, 55, 
	6, 7, 
	129, 
	9, 1, 68, 
	4, 29, 0, 27, 
	54, 64, 
	15, 10, 61, 
	2, 22, 0, 22, 0, 23, 
	55, 55, 131, 132, 
	15, 10, 47, 
	2, 22, 0, 22, 0, 23, 
	55, 55, 131, 132, 
	15, 29, 61, 
	2, 22, 0, 22, 0, 23, 
	55, 55, 131, 132, 
	15, 29, 47, 
	2, 22, 0, 22, 0, 23, 
	55, 55, 131, 132, 
	15, 10, 47, 
	2, 22, 0, 22, 0, 23, 
	55, 55, 131, 132, 
	8, 10, 46, 
	4, 22, 13, 20, 
	41, 
	8, 29, 46, 
	4, 22, 13, 21, 
	41, 
	8, 10, 71, 
	4, 22, 13, 21, 
	134, 
	8, 29, 71, 
	4, 22, 13, 21, 
	134, 
	19, 10, 70, 
	2, 25, 14, 26, 0, 25, 0, 26, 
	52, 74, 5, 51, 
	13, 10, 70, 
	2, 25, 13, 26, 0, 25, 
	52, 5, 
	6, 40, 70, 
	1, 25, 
	135, 136, 137, 
	4, 40, 48, 
	1, 26, 
	138, 
	23, 18, 70, 
	2, 45, 1, 25, 8, 12, 0, 25, 0, 26, 
	55, 55, 139, 73, 
	207, 
	0, 45, 0, 36, 0, 35, 
	72, 53, 140, 144, 
	192, 
	145, 
	18, 23, 37, 
	4, 21, 2, 4, 14, 1, 0, 1, 
	53, 5, 15, 
	0, 49, 0, 
	130, 
	4, 23, 24, 
	4, 4, 
	21, 
	15, 35, 55, 
	4, 29, 1, 7, 0, 24, 
	50, 88, 54, 64, 
	14, 63, 48, 
	1, 25, 1, 26, 0, 12, 
	58, 5, 3, 
	18, 18, 70, 
	8, 12, 5, 45, 0, 12, 0, 25, 
	60, 53, 5, 
	9, 18, 70, 
	9, 12, 0, 25, 
	53, 5, 
	0, 18, 48, 
	142, 
	8, 17, 28, 
	4, 10, 0, 11, 
	54, 
	8, 17, 28, 
	4, 13, 0, 12, 
	54, 
	23, 41, 29, 
	1, 28, 1, 1, 0, 13, 0, 1, 0, 33, 
	58, 62, 34, 33, 
	23, 29, 72, 
	2, 46, 1, 28, 8, 13, 0, 46, 0, 47, 
	72, 36, 23, 31, 
	4, 1, 66, 
	2, 46, 
	17, 
	15, 1, 66, 
	2, 47, 14, 36, 0, 32, 
	20, 88, 54, 146, 
	15, 1, 66, 
	2, 47, 13, 36, 0, 32, 
	20, 88, 54, 64, 
	13, 23, 22, 
	4, 4, 14, 28, 0, 28, 
	53, 15, 
	9, 1, 66, 
	2, 35, 0, 31, 
	54, 64, 
	4, 23, 48, 
	1, 26, 
	143, 
	3, 23, 83, 
	64, 86, 66, 14, 
	7, 56, 72, 
	2, 46, 
	86, 16, 61, 63, 
	1, 43, 75, 
	85, 130, 
	0, 29, 0, 
	26, 
	0, 43, 0, 
	85, 
	0, 21, 75, 
	130, 
	1, 23, 0, 
	5, 14, 
	0, 7, 0, 
	104, 
	9, 18, 33, 
	4, 7, 1, 15, 
	116, 117, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
204, 69, 65, 86,
81, 85, 73, 84,
83, 65, 86, 69,
73, 78, 86, 69,
83, 67, 79, 82,
72, 69, 76, 80,
200, 73, 78, 84,
195, 76, 85, 69,
71, 69, 84, 32,
212, 65, 75, 69,
87, 65, 73, 84,
87, 69, 65, 82,
196, 79, 78, 32,
82, 69, 77, 79,
196, 79, 70, 70,
67, 76, 73, 77,
68, 82, 79, 80,
199, 73, 86, 69,
208, 85, 84, 32,
89, 79, 85, 32,
32, 32, 32, 32,
76, 79, 79, 75,
197, 88, 65, 77,
198, 82, 73, 83,
211, 69, 65, 82,
79, 80, 69, 78,
213, 78, 76, 79,
75, 73, 76, 76,
193, 84, 84, 65,
66, 82, 69, 65,
211, 77, 65, 83,
69, 65, 84, 32,
196, 82, 73, 78,
87, 65, 86, 69,
82, 85, 66, 32,
208, 79, 76, 73,
74, 85, 77, 80,
204, 69, 65, 80,
82, 69, 65, 68,
70, 73, 84, 32,
198, 73, 88, 32,
83, 65, 89, 32,
211, 80, 69, 65,
82, 73, 68, 69,
208, 69, 68, 65,
83, 77, 79, 75,
32, 32, 32, 32,
70, 85, 67, 75,
208, 73, 83, 83,
194, 79, 76, 76,
194, 65, 83, 84,
211, 72, 73, 84,
195, 85, 78, 84,
70, 79, 76, 76,
70, 69, 69, 68,
77, 79, 86, 69,
208, 85, 83, 72,
80, 82, 65, 89,
82, 65, 80, 69,
71, 73, 86, 69,
68, 73, 71, 32,
87, 82, 65, 80,
195, 79, 86, 69,
195, 79, 78, 67,
200, 73, 68, 69,
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
83, 87, 79, 82,
80, 65, 73, 78,
208, 79, 82, 84,
67, 72, 65, 73,
75, 69, 89, 32,
67, 72, 69, 83,
82, 65, 86, 69,
83, 80, 69, 67,
199, 76, 65, 83,
71, 79, 66, 76,
211, 76, 73, 77,
75, 73, 78, 71,
80, 82, 73, 78,
65, 75, 89, 82,
215, 73, 90, 65,
84, 65, 80, 69,
79, 70, 70, 32,
83, 84, 65, 73,
67, 79, 82, 82,
82, 85, 71, 32,
211, 72, 69, 69,
84, 82, 69, 69,
82, 85, 66, 89,
210, 79, 68, 32,
67, 79, 82, 78,
194, 65, 71, 32,
70, 79, 88, 32,
66, 82, 73, 68,
80, 65, 84, 72,
76, 69, 68, 71,
78, 69, 83, 84,
83, 67, 69, 80,
83, 72, 79, 86,
76, 73, 66, 82,
84, 85, 78, 78,
80, 65, 83, 83,
67, 72, 73, 67,
82, 69, 71, 65,
71, 85, 65, 82,
72, 85, 78, 84,
67, 85, 66, 32,
80, 65, 82, 67,
208, 65, 80, 69,
211, 84, 82, 73,
210, 85, 78, 69,
70, 65, 82, 77,
76, 69, 65, 68,
71, 79, 76, 68,
87, 65, 78, 68,
211, 84, 65, 70,
74, 69, 87, 69,
211, 67, 69, 80,
79, 82, 66, 32,
67, 82, 79, 87,
86, 73, 88, 69,
71, 65, 77, 69,
82, 79, 67, 75,
72, 69, 78, 32,
200, 79, 85, 83,
68, 79, 79, 82,
212, 82, 65, 80,
87, 69, 76, 76,
211, 72, 65, 70,
66, 79, 79, 75,
72, 79, 85, 78,
84, 82, 79, 76,
211, 84, 79, 78,
72, 69, 76, 76,
66, 79, 76, 76,
194, 65, 83, 84,
208, 73, 83, 83,
211, 72, 73, 84,
198, 85, 67, 75,
194, 65, 76, 76,
195, 85, 78, 84,
203, 78, 65, 67,
65, 82, 79, 85,
210, 79, 85, 78,
193, 66, 79, 85,
	0,
};
const uint8_t automap[] = {
82, 85, 66, 89,
	1,
83, 67, 69, 80,
	3,
75, 69, 89, 32,
	5,
87, 65, 78, 68,
	7,
80, 65, 73, 78,
	10,
83, 80, 69, 67,
	11,
82, 85, 71, 32,
	13,
67, 72, 73, 67,
	14,
70, 79, 88, 32,
	15,
67, 79, 82, 78,
	16,
83, 72, 79, 86,
	24,
66, 79, 79, 75,
	25,
80, 65, 82, 67,
	26,
79, 82, 66, 32,
	27,
83, 87, 79, 82,
	28,
67, 72, 65, 73,
	29,
80, 82, 73, 78,
	36,
67, 82, 79, 87,
	44,
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
