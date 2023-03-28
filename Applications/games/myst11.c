#define NUM_OBJ 58
#define WORDSIZE 4
#define GAME_MAGIC 551
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
const uint8_t startlamp = 250;
const uint8_t lightfill = 250;
const uint8_t startcarried = 3;
const uint8_t maxcar = 6;
const uint8_t treasure = 0;
const uint8_t treasures = 0;
const uint8_t lastloc = 41;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"\x20",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x4C\x65\x69\x73\x75\x72\x65\x20\x4C\x6F\x75\x6E\x67\x65",
 { 2, 0, 0, 5, 3, 0 } }, 
		{ 	"\x57\x61\x73\x68\x20\x52\x6F\x6F\x6D",
 { 0, 1, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x47\x72\x61\x6E\x64\x20\x48\x61\x6C\x6C",
 { 34, 0, 0, 35, 0, 1 } }, 
		{ 	"\x44\x61\x72\x6B\x20\x50\x61\x73\x73\x61\x67\x65",
 { 4, 4, 0, 9, 0, 0 } }, 
		{ 	"\x48\x61\x6C\x6C\x20\x6F\x66\x20\x4D\x69\x72\x72\x6F\x72\x73",
 { 6, 5, 6, 8, 0, 0 } }, 
		{ 	"\x48\x61\x6C\x6C\x20\x6F\x66\x20\x4D\x69\x72\x72\x6F\x72\x73",
 { 7, 5, 5, 8, 0, 0 } }, 
		{ 	"\x48\x61\x6C\x6C\x20\x6F\x66\x20\x4D\x69\x72\x72\x6F\x72\x73",
 { 6, 7, 7, 9, 0, 0 } }, 
		{ 	"\x48\x61\x6C\x6C\x20\x6F\x66\x20\x4D\x69\x72\x72\x6F\x72\x73",
 { 6, 9, 10, 8, 0, 0 } }, 
		{ 	"\x48\x61\x6C\x6C\x20\x6F\x66\x20\x4D\x69\x72\x72\x6F\x72\x73",
 { 8, 10, 4, 7, 0, 0 } }, 
		{ 	"\x48\x61\x6C\x6C\x20\x6F\x66\x20\x4D\x69\x72\x72\x6F\x72\x73",
 { 9, 10, 10, 11, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x6E\x20\x41\x69\x72\x6C\x6F\x63\x6B",
 { 0, 0, 10, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x41\x71\x75\x61\x72\x69\x75\x6D\x20\x54\x61\x6E\x6B",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x6E\x20\x41\x69\x72\x6C\x6F\x63\x6B",
 { 0, 0, 0, 23, 0, 0 } }, 
		{ 	"\x4A\x65\x77\x65\x6C\x6C\x72\x79\x20\x45\x78\x68\x69\x62\x69\x74\x69\x6F\x6E",
 { 0, 0, 16, 0, 0, 0 } }, 
		{ 	"\x4D\x6F\x72\x67\x75\x65",
 { 0, 0, 0, 36, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x45\x78\x65\x63\x75\x74\x69\x6F\x6E\x20\x43\x68\x61\x6D\x62\x65\x72",
 { 0, 39, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x79\x20\x61\x20\x50\x61\x73\x73\x61\x67\x65",
 { 0, 0, 0, 15, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x45\x67\x79\x70\x74\x69\x61\x6E\x20\x54\x65\x6D\x70\x6C\x65",
 { 0, 0, 0, 17, 0, 0 } }, 
		{ 	"\x44\x55\x53\x54\x42\x49\x4E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x62\x6F\x76\x65\x20\x61\x20\x64\x65\x65\x70\x20\x50\x69\x74",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x45\x6E\x63\x68\x61\x6E\x74\x65\x64\x20\x57\x6F\x6F\x64\x6C\x61\x6E\x64",
 { 0, 0, 0, 4, 22, 0 } }, 
		{ 	"\x54\x72\x65\x65\x20\x48\x6F\x75\x73\x65\x20\x77\x69\x74\x68\x20\x61\x20\x68\x6F\x6C\x65\x20\x69\x6E\x20\x72\x6F\x6F\x66",
 { 0, 0, 0, 0, 3, 21 } }, 
		{ 	"\x4D\x61\x69\x6E\x74\x65\x6E\x61\x6E\x63\x65\x20\x57\x6F\x72\x6B\x73\x68\x6F\x70",
 { 0, 0, 13, 0, 0, 24 } }, 
		{ 	"\x53\x74\x6F\x72\x65\x20\x52\x6F\x6F\x6D",
 { 32, 0, 0, 0, 23, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x6F\x6C\x64\x20\x53\x65\x77\x65\x72",
 { 28, 25, 26, 25, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x6F\x6C\x64\x20\x53\x65\x77\x65\x72",
 { 27, 25, 27, 25, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x6F\x6C\x64\x20\x53\x65\x77\x65\x72",
 { 28, 26, 26, 28, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x6F\x6C\x64\x20\x53\x65\x77\x65\x72",
 { 27, 25, 27, 29, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x6F\x6C\x64\x20\x53\x65\x77\x65\x72",
 { 26, 30, 28, 30, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x6F\x6C\x64\x20\x53\x65\x77\x65\x72",
 { 29, 2, 25, 29, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x62\x65\x73\x69\x64\x65\x20\x61\x20\x52\x75\x73\x74\x69\x63\x20\x57\x65\x6C\x6C",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x4D\x6F\x64\x65\x6C\x6C\x65\x72\x27\x73\x20\x53\x74\x75\x64\x69\x6F",
 { 0, 24, 0, 0, 0, 0 } }, 
		{ 	"\x43\x72\x6F\x77\x64\x65\x64\x20\x4C\x65\x69\x73\x75\x72\x65\x20\x4C\x6F\x75\x6E\x67\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x27\x41\x73\x63\x65\x6E\x64\x20\x45\x76\x65\x72\x65\x73\x74\x27\x20\x44\x69\x73\x70\x6C\x61\x79",
 { 0, 3, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x61\x6E\x20\x27\x55\x6E\x64\x65\x72\x73\x65\x61\x20\x57\x6F\x72\x6C\x64\x27\x20\x44\x69\x73\x70\x6C\x61\x79",
 { 0, 0, 3, 0, 0, 0 } }, 
		{ 	"\x44\x61\x6D\x70\x20\x57\x65\x6C\x6C\x20\x53\x68\x61\x66\x74",
 { 0, 0, 15, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x72\x6F\x70\x65\x64\x20\x74\x6F\x20\x61\x20\x53\x74\x6F\x6E\x65\x20\x41\x6C\x74\x61\x72\x2E\x2E\x2E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x73\x74\x61\x6E\x64\x69\x6E\x67\x20\x62\x79\x20\x74\x68\x65\x20\x41\x6C\x74\x61\x72",
 { 0, 0, 0, 0, 39, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x62\x6F\x76\x65\x20\x61\x20\x64\x65\x65\x70\x20\x77\x69\x64\x65\x20\x50\x69\x74",
 { 16, 0, 0, 0, 36, 38 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x69\x6E\x20\x41\x6C\x61\x64\x64\x69\x6E\x27\x73\x20\x43\x61\x76\x65",
 { 0, 0, 0, 0, 0, 21 } }, 
		{ 	"\x2A\x4F\x68\x21\x20\x4F\x68\x21\x2E\x2E\x2E\x49\x27\x6D\x20\x69\x6E\x20\x74\x68\x65\x20\x4D\x65\x6C\x74\x69\x6E\x67\x20\x50\x6F\x74\x20\x21",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	0,
	0,
	0,
	0,
	0,
	36,
	0,
	0,
	0,
	12,
	27,
	0,
	0,
	0,
	1,
	22,
	0,
	0,
	0,
	0,
	255,
	0,
	255,
	0,
	37,
	0,
	18,
	0,
	38,
	255,
	16,
	1,
	1,
	1,
	2,
	21,
	0,
	3,
	34,
	35,
	21,
	0,
	19,
	29,
	0,
	15,
	0,
	17,
	0,
	0,
	15,
	32,
	0,
	0,
	4,
	24,
	0,
};


const char *objtext[] = {
	"\x20",
	"\x43\x6F\x69\x6E",
	"\x53\x6D\x61\x6C\x6C\x20\x4B\x65\x79",
	"\x43\x6C\x69\x6D\x62\x65\x72\x27\x73\x20\x52\x6F\x70\x65",
	"\x41\x70\x70\x6C\x65",
	"\x50\x69\x65\x63\x65\x20\x6F\x66\x20\x43\x68\x65\x65\x73\x65",
	"\x44\x61\x72\x6B\x20\x54\x75\x6E\x6E\x65\x6C",
	"\x50\x69\x70\x65\x72\x27\x73\x20\x46\x6C\x75\x74\x65",
	"\x41\x6E\x20\x41\x71\x75\x61\x6C\x75\x6E\x67",
	"\x54\x6F\x72\x63\x68\x20\x28\x6C\x69\x74\x29",
	"\x4A\x61\x77\x73\x20\x49\x56",
	"\x53\x65\x77\x65\x72\x20\x52\x61\x74\x73",
	"\x47\x6F\x6C\x64\x65\x6E\x20\x43\x61\x73\x6B\x65\x74",
	"\x54\x61\x6E\x6E\x61\x20\x4C\x65\x61\x76\x65\x73",
	"\x4F\x6C\x64\x20\x4C\x61\x6D\x70",
	"\x57\x6F\x6F\x64\x65\x6E\x20\x42\x65\x61\x6D",
	"\x43\x75\x70\x62\x6F\x61\x72\x64\x20\x77\x69\x74\x68\x20\x6B\x65\x79\x20\x69\x6E\x20\x6C\x6F\x63\x6B",
	"\x50\x69\x73\x74\x6F\x6C",
	"\x33\x20\x53\x69\x6C\x76\x65\x72\x20\x42\x75\x6C\x6C\x65\x74\x73",
	"\x54\x61\x6C\x69\x73\x6D\x61\x6E",
	"\x43\x72\x6F\x77\x62\x61\x72",
	"\x42\x6F\x78\x20\x6F\x66\x20\x4D\x61\x74\x63\x68\x65\x73",
	"\x45\x67\x79\x70\x74\x69\x61\x6E\x20\x54\x61\x6C\x69\x73\x6D\x61\x6E\x20\x28\x77\x6F\x72\x6E\x29",
	"\x50\x69\x65\x63\x65\x20\x6F\x66\x20\x50\x61\x70\x65\x72",
	"\x53\x6D\x61\x6C\x6C\x20\x42\x61\x72\x72\x65\x6C",
	"\x52\x61\x7A\x6F\x72\x20\x53\x68\x61\x72\x70\x20\x50\x65\x6E\x64\x75\x6C\x75\x6D\x20\x61\x62\x6F\x76\x65\x20\x6D\x65",
	"\x47\x6F\x6C\x64\x20\x4B\x65\x79",
	"\x53\x61\x72\x63\x6F\x70\x68\x61\x67\x75\x73",
	"\x53\x69\x6E\x69\x73\x74\x65\x72\x20\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x4D\x75\x6D\x6D\x79",
	"\x50\x65\x6E\x64\x75\x6C\x75\x6D\x20\x62\x75\x72\x69\x65\x64\x20\x69\x6E\x20\x41\x6C\x74\x61\x72",
	"\x43\x6F\x69\x6E",
	"\x54\x72\x61\x70\x64\x6F\x6F\x72\x2E\x20\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x64\x6F\x6F\x72\x20\x62\x65\x79\x6F\x6E\x64",
	"\x50\x75\x62\x6C\x69\x63\x20\x54\x65\x6C\x65\x70\x68\x6F\x6E\x65",
	"\x4F\x6C\x64\x2D\x46\x61\x73\x68\x69\x6F\x6E\x65\x64\x20\x53\x6C\x6F\x74\x20\x4D\x61\x63\x68\x69\x6E\x65",
	"\x53\x65\x61\x74\x73\x20\x61\x72\x6F\x75\x6E\x64\x20\x74\x68\x65\x20\x72\x6F\x6F\x6D",
	"\x57\x61\x73\x68\x62\x61\x73\x69\x6E\x73\x20\x26\x20\x54\x6F\x69\x6C\x65\x74\x73\x2E\x20\x47\x72\x69\x64\x20\x69\x6E\x20\x66\x6C\x6F\x6F\x72",
	"\x54\x68\x65\x20\x50\x69\x65\x64\x20\x50\x69\x70\x65\x72\x20\x26\x20\x47\x75\x79\x20\x46\x61\x77\x6B\x65\x73",
	"\x53\x6D\x61\x6C\x6C\x20\x54\x6F\x72\x63\x68",
	"\x44\x69\x73\x70\x6C\x61\x79\x73\x20\x74\x6F\x20\x4E\x6F\x72\x74\x68\x20\x26\x20\x57\x65\x73\x74",
	"\x45\x64\x6D\x75\x6E\x64\x20\x48\x69\x6C\x6C\x61\x72\x79\x20\x26\x20\x54\x65\x6E\x73\x69\x6E\x67",
	"\x4A\x61\x63\x71\x75\x65\x73\x20\x43\x6F\x75\x73\x74\x65\x61\x75\x20\x69\x6E\x20\x53\x63\x75\x62\x61\x20\x47\x65\x61\x72",
	"\x53\x69\x67\x6E\x20\x27\x4E\x6F\x20\x57\x61\x69\x74\x69\x6E\x67\x27",
	"\x41\x71\x75\x61\x6C\x75\x6E\x67\x20\x28\x77\x6F\x72\x6E\x29",
	"\x52\x6F\x70\x65\x20\x68\x61\x6E\x67\x69\x6E\x67\x20\x64\x6F\x77\x6E\x20\x74\x68\x65\x20\x57\x65\x6C\x6C",
	"\x48\x75\x6D\x61\x6E\x65\x20\x52\x61\x74\x20\x54\x72\x61\x70",
	"\x4A\x61\x63\x6B\x65\x74",
	"\x44\x65\x63\x6F\x6D\x70\x6F\x73\x69\x6E\x67\x20\x5A\x6F\x6D\x62\x69\x65",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x52\x6F\x74\x74\x69\x6E\x67\x20\x46\x6C\x65\x73\x68",
	"\x4D\x61\x73\x6F\x6E\x72\x79\x20\x62\x6C\x6F\x63\x6B\x69\x6E\x67\x20\x74\x68\x65\x20\x50\x61\x73\x73\x61\x67\x65",
	"\x44\x75\x73\x74\x20\x26\x20\x44\x65\x62\x72\x69\x73\x20\x65\x76\x65\x72\x79\x77\x68\x65\x72\x65",
	"\x50\x61\x73\x73\x61\x67\x65\x20\x6E\x6F\x77\x20\x63\x6C\x65\x61\x72\x65\x64",
	"\x41\x6E\x20\x6F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x4A\x75\x6E\x6B",
	"\x47\x6F\x6C\x64\x65\x6E\x20\x4D\x61\x73\x6B",
	"\x43\x75\x70\x62\x6F\x61\x72\x64",
	"\x4E\x61\x72\x72\x6F\x77\x20\x63\x72\x61\x63\x6B\x20\x69\x6E\x20\x77\x61\x6C\x6C",
	"\x4C\x6F\x6F\x73\x65\x20\x74\x72\x61\x70\x64\x6F\x6F\x72\x20\x69\x6E\x20\x72\x6F\x6F\x66",
	"\x54\x72\x61\x70\x64\x6F\x6F\x72\x20\x73\x65\x63\x75\x72\x65\x64\x20\x62\x79\x20\x62\x65\x61\x6D",
};
const char *msgptr[] = {
	"\x4F",
	"\x49\x20\x6D\x75\x73\x74\x20\x68\x61\x76\x65\x20\x64\x6F\x7A\x65\x64\x20\x6F\x66\x66\x2E\x20\x49\x27\x6D\x20\x6C\x6F\x63\x6B\x65\x64\x20\x69\x6E\x21",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x43\x6F\x69\x6E\x20\x68\x65\x72\x65",
	"\x49\x74\x20\x67\x69\x76\x65\x73\x20\x50\x72\x69\x7A\x65\x73",
	"\x49\x27\x76\x65\x20\x77\x6F\x6E\x20\x61\x20\x66\x6C\x61\x73\x68\x6C\x69\x67\x68\x74",
	"\x50\x68\x6F\x74\x6F\x2D\x65\x6C\x65\x63\x74\x72\x69\x63\x20\x63\x65\x6C\x6C\x73\x20\x74\x72\x69\x67\x67\x65\x72\x20\x44\x69\x73\x70\x6C\x61\x79",
	"\x49\x27\x76\x65\x20\x6C\x6F\x73\x74\x21",
	"\x43\x52\x49\x50\x45\x53\x21\x20\x54\x68\x65\x20\x53\x68\x61\x72\x6B\x20\x62\x69\x74\x20\x6D\x65\x20\x69\x6E\x20\x48\x61\x6C\x66",
	"\x4F\x2E\x4B\x2E",
	"\x49\x74\x27\x73\x20\x45\x6D\x70\x74\x79\x21",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C",
	"\x49\x20\x63\x61\x6E\x20\x73\x65\x65\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21",
	"\x49\x74\x27\x73\x20\x4F\x70\x65\x6E",
	"\x49\x74\x27\x73\x20\x4C\x6F\x63\x6B\x65\x64",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x4D\x59\x53\x54\x45\x52\x49\x4F\x55\x53\x20\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x20\x23\x31\x31",
	"\x44\x65\x64\x69\x63\x61\x74\x65\x64\x20\x74\x6F\x20\x4D\x61\x72\x67\x61\x72\x65\x74",
	"\x48\x61\x76\x65\x20\x61\x20\x43\x6F\x66\x66\x65\x65\x2E\x2E\x2E\x61\x6E\x64\x20\x54\x48\x49\x4E\x4B",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x73",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x69\x73\x20\x48\x61\x70\x70\x65\x6E\x69\x6E\x67\x21",
	"\x49\x20\x72\x65\x6D\x65\x6D\x62\x65\x72\x20\x63\x6F\x6D\x69\x6E\x67\x20\x74\x6F\x20\x74\x68\x65\x20\x57\x61\x78\x77\x6F\x72\x6B\x73\x2E\x2E",
	"\x49\x74\x27\x73\x20\x53\x50\x4F\x4F\x4B\x59\x21",
	"\x53\x68\x61\x72\x6B\x20\x73\x77\x69\x6D\x73\x20\x63\x6C\x6F\x73\x65\x72\x2E\x2E\x2E",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x68\x61\x76\x65\x20\x61\x6E\x79",
	"\x49\x20\x63\x61\x6E\x27\x74\x20\x73\x71\x75\x65\x65\x7A\x65\x20\x74\x68\x72\x6F\x75\x67\x68\x2E\x2E\x2E\x79\x65\x74",
	"\x52\x61\x74\x73\x20\x65\x61\x74\x20\x74\x68\x65\x20\x63\x68\x65\x65\x73\x65",
	"\x52\x61\x74\x73\x20\x66\x6F\x6C\x6C\x6F\x77\x20\x6D\x65\x21",
	"\x54\x68\x65\x79\x20\x6C\x6F\x6F\x6B\x20\x68\x75\x6E\x67\x72\x79\x21",
	"\x52\x61\x74\x73\x20\x61\x72\x65\x6E\x27\x74\x20\x68\x75\x6E\x67\x72\x79\x20\x6E\x6F\x77",
	"\x41\x20\x76\x6F\x69\x63\x65\x20\x73\x61\x79\x73\x20\x3A",
	"\x27\x53\x61\x79\x20\x74\x68\x65\x20\x59\x65\x61\x72\x20\x69\x74\x20\x68\x61\x70\x70\x65\x6E\x65\x64\x27",
	"\x54\x6F\x6F\x74\x20\x74\x65\x20\x54\x6F\x6F\x74\x20\x74\x65\x20\x54\x6F\x6F\x74\x21",
	"\x57\x68\x65\x6E\x20\x49\x20\x77\x61\x73\x20\x33\x31\x20\x6D\x79\x20\x73\x6F\x6E\x20\x77\x61\x73\x20\x38",
	"\x20\x43\x6F\x72\x72\x65\x63\x74\x21",
	"\x2E\x2E\x2E\x2E\x2E\x54\x68\x61\x74\x27\x73\x20\x42\x65\x74\x74\x65\x72\x21",
	"\x49\x74\x27\x73\x20\x74\x6F\x6F\x20\x52\x69\x73\x6B\x79\x21",
	"\x54\x6F\x72\x71\x75\x65\x6D\x61\x64\x61\x27\x73\x20\x67\x6F\x74\x20\x6D\x65\x21\x2E\x2E\x2E\x48\x65\x20\x4B\x2E\x4F\x2E\x73\x20\x6D\x65\x21",
	"\x54\x68\x61\x74\x20\x77\x61\x73\x20\x53\x54\x55\x50\x49\x44\x21\x20\x49\x27\x76\x65\x20\x64\x72\x6F\x77\x6E\x65\x64",
	"\x49\x74\x27\x73\x20\x74\x6F\x6F\x20\x73\x6D\x61\x6C\x6C",
	"\x52\x61\x74\x73\x20\x67\x6E\x61\x77\x20\x72\x6F\x70\x65\x73\x2E\x20\x46\x72\x65\x65\x20\x6D\x65\x20\x6A\x75\x73\x74\x20\x69\x6E\x20\x74\x69\x6D\x65\x21",
	"\x48\x75\x6E\x67\x72\x79\x20\x52\x61\x74\x73\x20\x77\x69\x6C\x6C\x20\x63\x68\x65\x77\x20\x61\x6E\x79\x74\x68\x69\x6E\x67\x2E\x2E",
	"\x49\x20\x63\x61\x6E\x20\x68\x65\x61\x72\x20\x52\x61\x74\x73\x20\x73\x63\x72\x61\x62\x62\x6C\x69\x6E\x67\x20\x61\x62\x6F\x75\x74",
	"\x41\x61\x61\x72\x72\x67\x67\x68\x68\x2E\x2E\x54\x68\x65\x20\x50\x65\x6E\x64\x75\x6C\x75\x6D\x20\x72\x65\x61\x63\x68\x65\x64\x20\x6D\x65\x21",
	"\x50\x65\x6E\x64\x75\x6C\x75\x6D\x20\x73\x77\x69\x6E\x67\x73\x20\x6C\x6F\x77\x65\x72\x20\x61\x6E\x64\x20\x6C\x6F\x77\x65\x72\x2E\x2E\x2E",
	"\x54\x72\x61\x70\x64\x6F\x6F\x72\x20\x69\x73\x20\x6C\x6F\x6F\x73\x65\x21",
	"\x48\x65\x27\x73\x20\x68\x69\x64\x69\x6E\x67\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21",
	"\x49\x74\x27\x73\x20\x47\x75\x6E\x70\x6F\x77\x64\x65\x72\x20\x61\x6E\x64\x20\x69\x74\x27\x73\x20\x46\x75\x73\x65\x64\x21",
	"\x23\x21\x2A\x2E\x2E\x73\x73\x2E\x7A\x7A\x7A\x2E\x2E\x49\x74\x27\x73\x20\x6C\x69\x74",
	"\x54\x69\x6D\x65\x20\x70\x61\x73\x73\x65\x73\x2E\x2E\x2E\x2E",
	"\x20\x69\x73\x20\x77\x72\x6F\x6E\x67",
	"\x42\x20\x4F\x20\x4F\x20\x4D\x20\x21\x2E\x2E\x20\x49\x20\x77\x61\x73\x20\x74\x6F\x6F\x20\x73\x6C\x6F\x77\x21",
	"\x49\x74\x20\x68\x61\x6E\x67\x73\x20\x62\x65\x68\x69\x6E\x64\x20\x74\x68\x65\x20\x64\x6F\x6F\x72",
	"\x49\x74\x27\x73\x20\x4C\x6F\x61\x64\x65\x64",
	"\x46\x6F\x72\x20\x41\x6C\x6C\x20\x74\x68\x65\x20\x4D\x59\x53\x54\x45\x52\x49\x4F\x55\x53\x20\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x53",
	"\x2A\x20\x42\x20\x41\x20\x4E\x20\x47\x20\x2A\x20\x21",
	"\x49\x27\x76\x65\x20\x4B\x69\x6C\x6C\x65\x64\x20\x69\x74\x21",
	"\x55\x75\x67\x67\x68\x68\x21\x20\x49\x74\x20\x62\x69\x74\x20\x6D\x79\x20\x54\x68\x72\x6F\x61\x74\x20\x4F\x75\x74\x21",
	"\x49\x74\x20\x6D\x6F\x76\x65\x73\x20\x63\x6C\x6F\x73\x65\x72\x2E\x2E\x2E",
	"\x49\x74\x20\x73\x61\x79\x73\x20\x27\x53\x65\x65\x20\x79\x6F\x75\x72\x20\x4C\x6F\x63\x61\x6C\x20\x44\x65\x61\x6C\x65\x72",
	"\x47\x65\x74\x20\x6F\x75\x74\x20\x46\x41\x53\x54\x21",
	"\x4D\x61\x74\x63\x68\x20\x68\x61\x73\x20\x67\x6F\x6E\x65\x20\x6F\x75\x74",
	"\x4D\x61\x74\x63\x68\x20\x62\x75\x72\x6E\x73\x20\x64\x69\x6D\x6C\x79",
	"\x55\x6D\x6D\x6D\x6D\x2E\x2E\x2E\x56\x65\x72\x79\x20\x54\x61\x73\x74\x79\x21",
	"\x59\x6F\x75\x20\x61\x72\x65\x20\x41\x42\x4F\x4D\x49\x4E\x41\x42\x4C\x45\x21",
	"\x49\x20\x6E\x65\x65\x64\x20\x61\x20\x6C\x65\x76\x65\x72",
	"\x54\x72\x61\x70\x64\x6F\x6F\x72\x20\x69\x73\x20\x73\x65\x63\x75\x72\x65\x64",
	"\x42\x20\x4F\x20\x4F\x20\x4D\x20\x54\x68\x65\x72\x65\x27\x73\x20\x61\x6E\x20\x45\x78\x70\x6C\x6F\x73\x69\x6F\x6E\x21",
	"\x57\x68\x61\x74\x20\x61\x20\x4D\x65\x73\x73\x21",
	"\x49\x27\x76\x65\x20\x66\x61\x6C\x6C\x65\x6E\x20\x64\x6F\x77\x6E\x20\x61\x6E\x64\x20\x62\x72\x6F\x6B\x65\x6E\x20\x6D\x79\x20\x6E\x65\x63\x6B\x21",
	"\x4D\x75\x6D\x6D\x79\x20\x52\x65\x76\x69\x76\x65\x73\x20\x61\x6E\x64\x20\x73\x65\x65\x73\x20\x54\x61\x6C\x69\x73\x6D\x61\x6E",
	"\x47\x69\x76\x65\x73\x20\x6D\x65\x20\x61\x20\x47\x6F\x6C\x64\x65\x6E\x20\x4D\x61\x73\x6B",
	"\x2E\x2E\x2E\x49\x27\x76\x65\x20\x62\x65\x65\x6E\x20\x61\x73\x6C\x65\x65\x70\x2E\x20\x49\x74\x20\x77\x61\x73\x20\x61\x6C\x6C\x20\x61\x20\x44\x72\x65\x61\x6D\x21",
	"\x54\x68\x61\x6E\x6B\x20\x47\x6F\x6F\x64\x6E\x65\x73\x73\x20\x49\x27\x76\x65\x20\x53\x75\x72\x76\x69\x76\x65\x64\x21",
	"\x5A\x6F\x6D\x62\x69\x65\x20\x62\x61\x72\x73\x20\x74\x68\x65\x20\x77\x61\x79\x21",
	"\x49\x74\x27\x73\x20\x62\x65\x65\x6E\x20\x56\x61\x6E\x64\x61\x6C\x69\x7A\x65\x64\x21",
	"\x55\x67\x68\x21\x2E\x2E\x49\x74\x27\x73\x20\x57\x41\x58\x21",
	"\x4D\x75\x6D\x6D\x79\x20\x52\x65\x76\x69\x76\x65\x73\x21\x2E\x2E\x53\x61\x79\x73\x20\x27\x55\x6E\x62\x65\x6C\x69\x65\x76\x65\x72\x21\x27",
	"\x61\x6E\x64\x20\x74\x68\x65\x6E\x20\x53\x74\x72\x61\x6E\x67\x6C\x65\x73\x20\x6D\x65\x21\x2E\x2E\x4F\x6F\x6F\x6F\x6F\x67\x67\x68\x21",
	"\x49\x27\x6D\x20\x67\x6F\x69\x6E\x67\x20\x69\x6E\x20\x63\x69\x72\x63\x6C\x65\x73\x21",
	"\x59\x6F\x75\x72\x20\x50\x69\x65\x74\x79\x20\x69\x73\x20\x4E\x6F\x74\x65\x77\x6F\x72\x74\x68\x79\x21",
	"\x49\x74\x27\x73\x20\x4C\x69\x74",
	"\x49\x74\x27\x73\x20\x6E\x6F\x74\x20\x4C\x69\x74",
	"\x54\x68\x61\x74\x27\x73\x20\x55\x6E\x6E\x65\x63\x65\x73\x73\x61\x72\x79\x21\x20\x59\x6F\x75\x20\x52\x61\x70\x73\x63\x61\x6C\x6C\x69\x6F\x6E\x21",
	"\x27\x20\x57\x61\x78\x77\x6F\x72\x6B\x73\x20\x27\x20\x28\x43\x29\x20\x31\x39\x38\x33",
	"\x62\x79\x20\x42\x72\x69\x61\x6E\x20\x48\x6F\x77\x61\x72\x74\x68\x20\x26\x20\x43\x6C\x69\x66\x66\x20\x4F\x67\x64\x65\x6E",
	"\x49\x20\x61\x6D\x20\x6E\x6F\x77\x20\x64\x6F\x75\x62\x6C\x65\x20\x68\x69\x73\x20\x61\x67\x65",
	"\x27\x53\x61\x79\x27\x20\x48\x6F\x77\x20\x6F\x6C\x64\x20\x49\x20\x61\x6D",
	"\x6F\x72\x20\x43\x6F\x6E\x74\x61\x63\x74\x20\x20\x44\x49\x47\x49\x54\x41\x4C\x20\x46\x41\x4E\x54\x41\x53\x49\x41\x27",
	"\x53\x6F\x6D\x65\x6F\x6E\x65\x20\x63\x72\x65\x70\x74\x20\x75\x70\x20\x62\x65\x68\x69\x6E\x64\x20\x6D\x65\x21\x2E\x2E",
	"\x42\x75\x74\x20\x57\x68\x61\x74\x20\x61\x6E\x20\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x20\x69\x74\x20\x77\x61\x73\x21",
	"\x42\x65\x61\x6D\x20\x69\x73\x20\x74\x6F\x6F\x20\x6C\x6F\x6E\x67\x21",
	"\x55\x73\x65\x20\x74\x68\x65\x20\x50\x69\x73\x74\x6F\x6C\x21",
	"\x56\x69\x6F\x6C\x65\x6E\x63\x65\x20\x61\x63\x68\x69\x65\x76\x65\x73\x20\x4E\x6F\x74\x68\x69\x6E\x67\x21",
};


const uint8_t status[] = {
	167, 
	9, 1, 
	14, 132, 133, 73, 
	195, 
	0, 15, 86, 73, 
	199, 
	0, 1, 
	58, 19, 1, 20, 
	182, 
	1, 7, 12, 11, 8, 4, 0, 11, 0, 4, 
	53, 60, 25, 
	179, 
	4, 34, 14, 3, 8, 6, 0, 6, 
	5, 28, 29, 60, 
	165, 
	4, 2, 
	57, 64, 
	165, 
	4, 31, 
	56, 64, 
	165, 
	4, 3, 
	57, 64, 
	179, 
	4, 35, 8, 7, 14, 8, 14, 42, 
	5, 28, 31, 73, 
	198, 
	0, 7, 
	60, 134, 135, 
	168, 
	7, 34, 0, 6, 
	58, 
	176, 
	7, 35, 14, 8, 10, 55, 0, 7, 
	58, 
	171, 
	4, 12, 6, 42, 
	36, 88, 61, 63, 
	165, 
	4, 25, 
	56, 64, 
	164, 
	4, 41, 
	63, 
	180, 
	4, 20, 8, 4, 14, 19, 0, 11, 0, 37, 
	62, 
	183, 
	4, 20, 0, 9, 0, 1, 0, 4, 0, 1, 
	58, 81, 79, 81, 
	171, 
	4, 20, 14, 19, 
	88, 35, 86, 73, 
	199, 
	0, 37, 
	88, 88, 88, 54, 
	170, 
	4, 37, 0, 1, 
	81, 77, 73, 
	198, 
	19, 3, 
	40, 86, 86, 
	198, 
	19, 0, 
	41, 61, 63, 
	197, 
	0, 1, 
	81, 42, 
	170, 
	8, 8, 0, 2, 
	81, 77, 73, 
	202, 
	19, 0, 3, 24, 
	49, 61, 63, 
	214, 
	19, 0, 5, 24, 0, 24, 0, 49, 0, 2, 
	72, 115, 81, 
	200, 
	19, 0, 0, 8, 
	60, 
	200, 
	16, 0, 0, 2, 
	81, 
	168, 
	2, 49, 7, 17, 
	116, 
	180, 
	2, 49, 4, 17, 13, 48, 0, 48, 0, 50, 
	72, 
	145, 40, 
	2, 46, 8, 12, 0, 46, 0, 47, 
	72, 104, 
	179, 
	5, 46, 0, 4, 0, 4, 0, 4, 
	0, 81, 79, 81, 
	170, 
	2, 46, 0, 4, 
	81, 77, 73, 
	198, 
	19, 0, 
	105, 61, 63, 
	202, 
	0, 4, 0, 12, 
	81, 106, 60, 
	171, 
	8, 14, 0, 5, 
	81, 77, 0, 73, 
	196, 
	16, 0, 
	110, 
	201, 
	15, 0, 0, 14, 
	109, 60, 
	196, 
	0, 5, 
	81, 
	178, 
	7, 12, 0, 6, 0, 3, 0, 6, 
	81, 79, 81, 
	171, 
	4, 12, 0, 6, 
	81, 77, 0, 73, 
	203, 
	4, 12, 19, 0, 
	7, 88, 61, 63, 
	198, 
	0, 6, 
	81, 127, 21, 
};
const uint8_t actions[] = {
	19, 28, 12, 
	4, 1, 1, 1, 14, 37, 0, 37, 
	18, 4, 53, 73, 
	205, 
	0, 1, 0, 19, 0, 37, 
	62, 74, 
	4, 3, 0, 
	4, 37, 
	39, 
	8, 7, 8, 
	4, 1, 13, 1, 
	123, 
	22, 28, 12, 
	4, 1, 1, 30, 13, 37, 0, 30, 0, 19, 
	62, 17, 6, 
	22, 28, 12, 
	4, 1, 1, 1, 13, 37, 0, 1, 0, 19, 
	17, 6, 62, 
	22, 28, 12, 
	4, 1, 1, 30, 14, 37, 0, 37, 0, 30, 
	18, 4, 72, 
	13, 7, 8, 
	4, 1, 14, 1, 0, 1, 
	53, 2, 
	13, 28, 12, 
	4, 1, 6, 30, 6, 1, 
	22, 85, 
	4, 7, 67, 
	4, 1, 
	3, 
	22, 53, 0, 
	4, 12, 1, 42, 8, 13, 0, 11, 0, 13, 
	54, 64, 60, 
	5, 31, 0, 
	4, 2, 
	8, 33, 
	0, 2, 0, 
	66, 
	14, 18, 13, 
	1, 9, 0, 9, 0, 37, 
	59, 53, 64, 
	15, 18, 33, 
	2, 28, 1, 13, 6, 22, 
	8, 125, 126, 73, 
	194, 
	88, 61, 63, 
	8, 23, 81, 
	1, 2, 4, 2, 
	12, 
	13, 1, 81, 
	4, 2, 1, 2, 0, 25, 
	54, 8, 
	8, 1, 81, 
	4, 2, 6, 2, 
	13, 
	13, 7, 7, 
	4, 1, 14, 2, 0, 2, 
	53, 11, 
	11, 10, 83, 
	2, 2, 0, 2, 
	52, 0, 0, 8, 
	8, 7, 81, 
	4, 2, 1, 2, 
	12, 
	9, 7, 81, 
	4, 2, 6, 2, 
	0, 13, 
	14, 76, 82, 
	1, 7, 2, 11, 0, 4, 
	58, 8, 30, 
	8, 1, 63, 
	4, 4, 10, 50, 
	23, 
	13, 1, 63, 
	4, 4, 11, 50, 0, 21, 
	54, 64, 
	19, 37, 25, 
	2, 11, 9, 5, 1, 5, 7, 37, 
	0, 8, 24, 73, 
	205, 
	0, 5, 0, 19, 0, 5, 
	62, 58, 
	8, 7, 25, 
	2, 11, 9, 5, 
	26, 
	8, 7, 25, 
	2, 11, 8, 5, 
	27, 
	13, 7, 85, 
	4, 21, 14, 7, 0, 7, 
	53, 11, 
	19, 25, 13, 
	1, 37, 0, 9, 0, 37, 0, 9, 
	72, 74, 8, 64, 
	19, 66, 13, 
	1, 9, 0, 9, 0, 37, 0, 37, 
	72, 74, 64, 8, 
	9, 76, 82, 
	1, 7, 7, 36, 
	8, 30, 
	23, 57, 39, 
	4, 34, 14, 3, 0, 3, 0, 31, 0, 6, 
	62, 84, 32, 58, 
	23, 57, 24, 
	4, 35, 14, 8, 14, 42, 0, 8, 0, 10, 
	62, 84, 32, 73, 
	196, 
	0, 7, 
	58, 
	11, 77, 0, 
	4, 21, 0, 31, 
	47, 88, 88, 54, 
	0, 78, 0, 
	8, 
	0, 79, 0, 
	8, 
	6, 1, 65, 
	4, 2, 
	8, 88, 33, 
	18, 13, 17, 
	1, 8, 0, 8, 0, 42, 0, 42, 
	72, 74, 8, 
	4, 1, 69, 
	1, 15, 
	139, 
	14, 1, 69, 
	4, 13, 0, 12, 0, 13, 
	54, 64, 58, 
	14, 1, 69, 
	4, 11, 0, 12, 0, 11, 
	54, 64, 58, 
	22, 53, 0, 
	4, 12, 1, 42, 8, 11, 0, 13, 0, 11, 
	54, 64, 60, 
	8, 23, 81, 
	4, 2, 6, 2, 
	13, 
	22, 68, 19, 
	4, 31, 1, 3, 0, 3, 0, 43, 0, 43, 
	8, 72, 53, 
	22, 15, 17, 
	1, 42, 7, 12, 0, 8, 0, 42, 0, 8, 
	72, 53, 8, 
	11, 15, 17, 
	1, 42, 4, 12, 
	36, 88, 61, 63, 
	8, 1, 70, 
	4, 31, 12, 43, 
	34, 
	13, 1, 70, 
	4, 31, 2, 43, 0, 36, 
	54, 64, 
	18, 10, 19, 
	2, 43, 0, 43, 0, 3, 0, 3, 
	72, 0, 52, 
	8, 10, 19, 
	2, 3, 0, 3, 
	52, 
	8, 7, 70, 
	4, 36, 17, 11, 
	40, 
	14, 76, 82, 
	17, 11, 1, 7, 0, 4, 
	8, 30, 58, 
	5, 76, 82, 
	1, 7, 
	8, 30, 
	22, 37, 25, 
	4, 37, 9, 5, 1, 5, 0, 5, 0, 19, 
	62, 28, 73, 
	207, 
	0, 38, 0, 11, 0, 19, 
	38, 88, 54, 62, 
	13, 7, 73, 
	4, 38, 14, 19, 0, 19, 
	11, 53, 
	15, 7, 27, 
	3, 44, 14, 5, 0, 5, 
	8, 88, 11, 53, 
	0, 43, 0, 
	128, 
	13, 1, 48, 
	4, 36, 9, 9, 0, 20, 
	54, 137, 
	12, 1, 48, 
	4, 36, 8, 9, 0, 39, 
	54, 
	18, 13, 66, 
	1, 19, 0, 19, 0, 22, 0, 22, 
	72, 74, 8, 
	18, 15, 66, 
	1, 22, 0, 22, 0, 19, 0, 19, 
	72, 53, 8, 
	13, 1, 19, 
	4, 31, 2, 43, 0, 36, 
	54, 64, 
	12, 7, 37, 
	4, 21, 14, 24, 14, 49, 
	44, 
	18, 33, 37, 
	4, 21, 14, 24, 14, 49, 0, 24, 
	8, 11, 53, 
	4, 7, 74, 
	1, 24, 
	45, 
	23, 25, 23, 
	1, 24, 8, 14, 0, 2, 0, 3, 0, 2, 
	81, 79, 81, 73, 
	198, 
	0, 8, 
	46, 108, 58, 
	5, 76, 82, 
	6, 7, 
	22, 85, 
	9, 57, 0, 
	4, 34, 9, 6, 
	84, 48, 
	9, 57, 0, 
	4, 35, 9, 7, 
	84, 48, 
	4, 57, 39, 
	8, 6, 
	85, 
	4, 57, 24, 
	8, 7, 
	85, 
	14, 7, 65, 
	4, 2, 14, 45, 0, 45, 
	11, 50, 53, 
	14, 7, 21, 
	1, 45, 14, 17, 0, 17, 
	8, 11, 53, 
	17, 7, 21, 
	1, 45, 13, 17, 14, 18, 0, 18, 
	11, 53, 
	8, 7, 51, 
	1, 17, 9, 10, 
	9, 
	23, 82, 51, 
	1, 17, 1, 18, 9, 10, 0, 18, 0, 19, 
	62, 8, 51, 73, 
	211, 
	0, 3, 0, 4, 0, 3, 0, 10, 
	81, 79, 81, 58, 
	8, 7, 51, 
	1, 17, 8, 10, 
	51, 
	14, 80, 0, 
	1, 17, 8, 10, 0, 3, 
	81, 77, 73, 
	201, 
	19, 1, 0, 10, 
	60, 73, 
	201, 
	19, 0, 0, 3, 
	81, 9, 
	200, 
	2, 46, 0, 12, 
	58, 
	197, 
	0, 3, 
	81, 103, 
	15, 64, 35, 
	4, 40, 1, 14, 0, 1, 
	87, 18, 88, 64, 
	18, 64, 35, 
	7, 40, 1, 14, 0, 1, 0, 40, 
	87, 18, 54, 
	4, 13, 21, 
	1, 45, 
	37, 
	6, 7, 76, 
	1, 23, 
	107, 102, 136, 
	6, 56, 76, 
	1, 23, 
	107, 102, 136, 
	23, 25, 84, 
	1, 21, 9, 14, 0, 5, 0, 2, 0, 5, 
	81, 79, 81, 73, 
	197, 
	0, 14, 
	58, 46, 
	13, 34, 29, 
	1, 5, 0, 5, 0, 19, 
	111, 62, 
	6, 34, 57, 
	1, 4, 
	8, 88, 124, 
	5, 34, 57, 
	6, 4, 
	22, 85, 
	5, 34, 29, 
	6, 5, 
	22, 85, 
	0, 34, 0, 
	112, 
	4, 1, 60, 
	2, 46, 
	122, 
	9, 1, 60, 
	2, 47, 0, 17, 
	54, 64, 
	13, 7, 11, 
	3, 52, 14, 20, 0, 20, 
	53, 11, 
	14, 7, 11, 
	3, 52, 14, 14, 0, 14, 
	0, 53, 11, 
	13, 1, 28, 
	4, 17, 2, 50, 0, 18, 
	54, 64, 
	17, 23, 41, 
	1, 12, 1, 26, 14, 13, 0, 13, 
	53, 11, 
	19, 23, 43, 
	4, 18, 14, 28, 1, 20, 0, 28, 
	12, 11, 53, 20, 
	13, 23, 43, 
	4, 18, 6, 20, 14, 28, 
	13, 113, 
	23, 68, 45, 
	4, 24, 1, 15, 0, 15, 0, 56, 0, 57, 
	8, 55, 72, 114, 
	23, 68, 27, 
	4, 24, 1, 15, 0, 15, 0, 56, 0, 57, 
	8, 55, 72, 114, 
	13, 1, 60, 
	4, 16, 13, 57, 0, 14, 
	54, 64, 
	11, 1, 60, 
	4, 16, 14, 57, 
	43, 117, 61, 63, 
	23, 18, 33, 
	2, 28, 1, 22, 1, 13, 0, 53, 0, 13, 
	118, 119, 74, 55, 
	9, 18, 33, 
	2, 28, 13, 53, 
	8, 17, 
	11, 13, 46, 
	1, 53, 0, 33, 
	54, 64, 86, 73, 
	195, 
	120, 138, 121, 63, 
	22, 18, 29, 
	4, 37, 9, 5, 1, 5, 0, 5, 0, 19, 
	62, 18, 73, 
	207, 
	0, 38, 0, 11, 0, 19, 
	38, 88, 54, 62, 
	12, 18, 29, 
	1, 5, 7, 37, 0, 5, 
	53, 
	18, 23, 88, 
	4, 22, 14, 4, 1, 26, 0, 4, 
	8, 53, 11, 
	18, 23, 88, 
	4, 22, 14, 4, 14, 26, 0, 4, 
	8, 53, 11, 
	22, 10, 83, 
	4, 22, 14, 26, 0, 26, 0, 54, 0, 16, 
	74, 72, 8, 
	10, 10, 83, 
	2, 26, 0, 26, 
	52, 0, 8, 
	13, 7, 56, 
	4, 14, 14, 12, 0, 12, 
	11, 53, 
	8, 23, 41, 
	1, 12, 6, 26, 
	13, 
	12, 23, 41, 
	1, 12, 1, 26, 13, 13, 
	12, 
	8, 23, 88, 
	4, 22, 13, 4, 
	12, 
	14, 18, 17, 
	1, 42, 0, 42, 0, 8, 
	8, 55, 53, 
	8, 1, 62, 
	4, 22, 0, 3, 
	54, 
	8, 18, 13, 
	1, 37, 0, 37, 
	53, 
	4, 7, 13, 
	1, 9, 
	129, 
	4, 7, 13, 
	1, 37, 
	130, 
	4, 7, 32, 
	2, 28, 
	20, 
	9, 18, 33, 
	1, 13, 0, 13, 
	53, 8, 
	4, 57, 0, 
	4, 34, 
	85, 
	4, 57, 0, 
	4, 35, 
	85, 
	0, 57, 0, 
	85, 
	3, 77, 0, 
	47, 88, 88, 88, 
	0, 4, 0, 
	71, 
	1, 3, 0, 
	0, 16, 
	0, 5, 0, 
	63, 
	0, 7, 0, 
	10, 
	0, 30, 8, 
	123, 
	1, 83, 0, 
	0, 131, 
	4, 46, 0, 
	2, 46, 
	140, 
	0, 46, 0, 
	141, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
73, 78, 86, 69,
72, 69, 76, 80,
83, 65, 86, 69,
81, 85, 73, 84,
83, 67, 79, 82,
76, 79, 79, 75,
197, 88, 65, 77,
211, 69, 65, 82,
71, 69, 84, 32,
212, 65, 75, 69,
199, 82, 65, 66,
87, 69, 65, 82,
196, 79, 78, 32,
82, 69, 77, 79,
196, 79, 70, 70,
32, 32, 32, 32,
68, 82, 79, 80,
208, 85, 84, 32,
204, 69, 65, 86,
199, 73, 86, 69,
76, 79, 67, 75,
79, 80, 69, 78,
213, 78, 76, 79,
76, 73, 71, 72,
211, 84, 82, 73,
201, 71, 78, 73,
73, 78, 83, 69,
77, 79, 85, 76,
85, 83, 69, 32,
87, 65, 83, 72,
210, 73, 78, 83,
77, 79, 86, 69,
69, 65, 84, 32,
212, 65, 83, 84,
196, 82, 73, 78,
70, 69, 69, 68,
32, 32, 32, 32,
32, 32, 32, 32,
83, 77, 65, 83,
194, 82, 69, 65,
211, 72, 65, 84,
80, 82, 65, 89,
67, 76, 73, 77,
32, 32, 32, 32,
75, 73, 76, 76,
193, 84, 84, 65,
32, 32, 32, 32,
67, 85, 84, 32,
195, 72, 79, 80,
211, 76, 65, 83,
200, 65, 67, 75,
83, 87, 73, 77,
84, 85, 82, 78,
32, 32, 32, 32,
82, 69, 65, 68,
83, 65, 89, 32,
76, 69, 86, 69,
208, 82, 73, 83,
32, 32, 32, 32,
66, 82, 69, 65,
32, 32, 32, 32,
32, 32, 32, 32,
82, 85, 66, 32,
208, 79, 76, 73,
85, 78, 76, 73,
196, 79, 85, 83,
84, 73, 69, 32,
198, 73, 88, 32,
193, 84, 84, 65,
198, 65, 83, 84,
84, 79, 32, 32,
72, 79, 84, 32,
67, 79, 76, 68,
70, 73, 76, 76,
80, 76, 65, 89,
87, 65, 73, 84,
83, 73, 84, 32,
83, 84, 65, 78,
70, 73, 82, 69,
211, 72, 79, 79,
76, 79, 65, 68,
66, 85, 71, 71,
194, 79, 71, 32,
194, 65, 83, 84,
211, 72, 73, 84,
208, 73, 83, 83,
194, 65, 76, 76,
198, 85, 67, 75,
195, 85, 78, 84,
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
83, 69, 65, 84,
80, 72, 79, 78,
212, 69, 76, 69,
90, 79, 77, 66,
74, 85, 78, 75,
67, 79, 73, 78,
84, 79, 82, 67,
198, 76, 65, 83,
52, 50, 32, 32,
77, 73, 82, 82,
65, 81, 85, 65,
211, 67, 85, 66,
82, 79, 80, 69,
68, 69, 66, 82,
74, 65, 67, 75,
195, 79, 65, 84,
70, 85, 83, 69,
52, 54, 32, 32,
82, 65, 84, 32,
210, 65, 84, 83,
84, 82, 65, 80,
80, 65, 83, 83,
67, 72, 69, 69,
67, 82, 79, 87,
80, 69, 78, 68,
77, 85, 77, 77,
84, 65, 78, 78,
204, 69, 65, 86,
76, 65, 77, 80,
87, 65, 84, 69,
71, 85, 89, 32,
198, 65, 87, 75,
49, 57, 53, 51,
181, 51, 32, 32,
67, 65, 83, 75,
83, 73, 71, 78,
83, 65, 82, 67,
76, 69, 86, 69,
66, 69, 65, 77,
77, 65, 83, 75,
79, 70, 70, 32,
84, 85, 78, 78,
68, 65, 82, 75,
53, 50, 32, 32,
80, 73, 83, 84,
199, 85, 78, 32,
66, 85, 76, 76,
211, 72, 69, 76,
53, 49, 32, 32,
74, 69, 87, 69,
65, 80, 80, 76,
77, 79, 68, 69,
53, 52, 32, 32,
68, 79, 79, 82,
52, 56, 32, 32,
72, 79, 76, 69,
67, 82, 65, 67,
67, 65, 86, 69,
84, 79, 73, 76,
84, 65, 76, 73,
83, 76, 79, 84,
205, 65, 67, 72,
65, 73, 82, 76,
87, 69, 76, 76,
211, 72, 65, 70,
68, 85, 78, 71,
65, 76, 84, 65,
66, 65, 82, 82,
71, 85, 78, 80,
80, 65, 80, 69,
52, 52, 32, 32,
83, 72, 65, 82,
202, 65, 87, 83,
83, 69, 87, 69,
71, 82, 73, 68,
70, 76, 85, 84,
75, 69, 89, 32,
77, 65, 84, 67,
80, 73, 80, 69,
208, 73, 69, 68,
87, 79, 82, 75,
67, 85, 80, 66,
71, 79, 76, 68,
87, 65, 83, 72,
72, 65, 78, 68,
70, 65, 67, 69,
68, 73, 83, 80,
76, 69, 73, 83,
204, 79, 85, 78,
77, 79, 82, 71,
87, 79, 79, 68,
84, 82, 69, 69,
72, 65, 76, 76,
70, 76, 69, 83,
210, 79, 84, 84,
72, 73, 76, 65,
212, 69, 78, 83,
74, 65, 67, 81,
195, 79, 85, 83,
	0,
};
const uint8_t automap[] = {
67, 79, 73, 78,
	1,
75, 69, 89, 32,
	2,
82, 79, 80, 69,
	3,
65, 80, 80, 76,
	4,
67, 72, 69, 69,
	5,
70, 76, 85, 84,
	7,
65, 81, 85, 65,
	8,
67, 65, 83, 75,
	12,
84, 65, 78, 78,
	13,
76, 65, 77, 80,
	14,
66, 69, 65, 77,
	15,
80, 73, 83, 84,
	17,
66, 85, 76, 76,
	18,
84, 65, 76, 73,
	19,
67, 82, 79, 87,
	20,
77, 65, 84, 67,
	21,
80, 65, 80, 69,
	23,
66, 65, 82, 82,
	24,
75, 69, 89, 32,
	26,
67, 79, 73, 78,
	30,
84, 79, 82, 67,
	37,
84, 82, 65, 80,
	44,
74, 65, 67, 75,
	45,
74, 85, 78, 75,
	52,
77, 65, 83, 75,
	53,
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
