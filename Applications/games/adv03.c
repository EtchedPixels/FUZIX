#define NUM_OBJ 54
#define WORDSIZE 3
#define GAME_MAGIC 306
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
const uint8_t startcarried = 1;
const uint8_t maxcar = 7;
const uint8_t treasure = 1;
const uint8_t treasures = 0;
const uint8_t lastloc = 23;
const uint8_t startloc = 2;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x62\x72\x69\x65\x66\x69\x6E\x67\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 3, 0, 0 } }, 
		{ 	"\x6C\x6F\x6E\x67\x20\x73\x6C\x6F\x70\x69\x6E\x67\x20\x67\x72\x65\x79\x20\x63\x6F\x72\x72\x69\x64\x6F\x72",
 { 8, 4, 2, 12, 6, 7 } }, 
		{ 	"\x67\x72\x65\x79\x20\x72\x6F\x6F\x6D",
 { 3, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x73\x69\x74\x74\x69\x6E\x67\x20\x69\x6E\x20\x61\x20\x67\x72\x65\x79\x20\x63\x68\x61\x69\x72\x0A\x74\x68\x65\x72\x65\x27\x73\x20\x61\x20\x62\x6F\x78\x20\x70\x6F\x69\x6E\x74\x69\x6E\x67\x20\x61\x74\x20\x6D\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x74\x77\x69\x73\x74\x69\x6E\x67\x20\x77\x68\x69\x74\x65\x20\x68\x61\x6C\x6C\x77\x61\x79",
 { 9, 0, 8, 0, 0, 3 } }, 
		{ 	"\x74\x77\x69\x73\x74\x69\x6E\x67\x20\x79\x65\x6C\x6C\x6F\x77\x20\x68\x61\x6C\x6C\x77\x61\x79",
 { 10, 0, 0, 0, 3, 0 } }, 
		{ 	"\x74\x77\x69\x73\x74\x69\x6E\x67\x20\x62\x6C\x75\x65\x20\x68\x61\x6C\x6C\x77\x61\x79",
 { 11, 3, 0, 6, 0, 0 } }, 
		{ 	"\x77\x68\x69\x74\x65\x20\x72\x6F\x6F\x6D",
 { 0, 6, 0, 0, 0, 0 } }, 
		{ 	"\x79\x65\x6C\x6C\x6F\x77\x20\x72\x6F\x6F\x6D",
 { 0, 7, 0, 0, 0, 0 } }, 
		{ 	"\x62\x6C\x75\x65\x20\x72\x6F\x6F\x6D",
 { 0, 8, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x61\x69\x6E\x74\x65\x6E\x61\x6E\x63\x65\x20\x72\x6F\x6F\x6D\x20\x31",
 { 0, 0, 3, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x77\x68\x69\x74\x65\x20\x76\x69\x73\x69\x74\x6F\x72\x73\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x79\x65\x6C\x6C\x6F\x77\x20\x63\x6F\x72\x72\x69\x64\x6F\x72",
 { 0, 0, 0, 17, 0, 0 } }, 
		{ 	"\x62\x6C\x75\x65\x20\x61\x6E\x74\x65\x72\x6F\x6F\x6D",
 { 0, 0, 0, 23, 22, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x6C\x65\x64\x67\x65\x20\x6F\x75\x74\x73\x69\x64\x65\x20\x6F\x66\x20\x61\x20\x77\x69\x6E\x64\x6F\x77\x0A\x68\x69\x67\x68\x20\x61\x62\x6F\x76\x65\x20\x74\x68\x65\x20\x72\x65\x61\x63\x74\x6F\x72\x20\x63\x6F\x72\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x61\x69\x6E\x74\x65\x6E\x61\x6E\x63\x65\x20\x72\x6F\x6F\x6D\x20\x32",
 { 0, 0, 14, 0, 18, 0 } }, 
		{ 	"\x70\x72\x6F\x6A\x65\x63\x74\x69\x6F\x6E\x69\x73\x74\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 17 } }, 
		{ 	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x72\x6F\x6F\x6D\x20\x73\x75\x72\x72\x6F\x6E\x64\x69\x6E\x67\x0A\x74\x68\x65\x20\x72\x65\x61\x63\x74\x6F\x72\x20\x63\x6F\x72\x65",
 { 0, 0, 20, 0, 0, 21 } }, 
		{ 	"\x62\x72\x65\x61\x6B\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 19, 0, 0 } }, 
		{ 	"\x72\x65\x61\x63\x74\x6F\x72\x20\x63\x6F\x72\x65",
 { 0, 0, 0, 0, 19, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x76\x69\x65\x77\x69\x6E\x67\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 15 } }, 
		{ 	"\x73\x74\x6F\x72\x61\x67\x65\x20\x72\x6F\x6F\x6D",
 { 0, 0, 15, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	0,
	1,
	2,
	4,
	4,
	5,
	1,
	0,
	255,
	0,
	0,
	16,
	9,
	13,
	13,
	17,
	0,
	13,
	16,
	18,
	0,
	19,
	12,
	0,
	23,
	17,
	23,
	21,
	21,
	0,
	19,
	10,
	19,
	11,
	15,
	0,
	0,
	22,
	0,
	0,
	0,
	0,
	0,
	19,
	14,
	15,
	5,
	19,
	0,
	0,
	0,
	0,
	0,
};


const char *objtext[] = {
	"\x54\x6F\x72\x6E\x20\x75\x70\x20\x6D\x61\x70",
	"\x50\x69\x63\x74\x75\x72\x65\x20\x6F\x66\x20\x6D\x65\x20\x73\x74\x61\x6D\x70\x65\x64\x20\x2D\x73\x65\x63\x75\x72\x69\x74\x79\x2D",
	"\x42\x6F\x6D\x62\x20\x64\x65\x74\x65\x63\x74\x6F\x72\x20\x66\x6C\x61\x73\x68\x69\x6E\x67\x20\x79\x65\x6C\x6C\x6F\x77\x20\x28\x62\x6F\x6D\x62\x20\x69\x73\x20\x6E\x6F\x77\x20\x61\x72\x6D\x65\x64\x29",
	"\x4C\x61\x72\x67\x65\x20\x74\x61\x70\x65\x20\x72\x65\x63\x6F\x72\x64\x65\x72",
	"\x42\x6F\x78\x20\x77\x69\x74\x68\x20\x61\x70\x70\x61\x72\x61\x74\x75\x73\x20\x70\x6F\x69\x6E\x74\x69\x6E\x67\x20\x61\x74\x20\x63\x68\x61\x69\x72",
	"\x43\x68\x61\x69\x72\x20\x62\x6F\x6C\x74\x65\x64\x20\x74\x6F\x20\x66\x6C\x6F\x6F\x72",
	"\x52\x6F\x77\x20\x6F\x66\x20\x34\x20\x62\x75\x74\x74\x6F\x6E\x73\x20\x2D\x72\x65\x64\x20\x77\x68\x69\x74\x65\x20\x62\x6C\x75\x65\x20\x79\x65\x6C\x6C\x6F\x77\x2D",
	"\x50\x69\x63\x74\x75\x72\x65\x20\x6F\x66\x20\x6D\x65\x20\x73\x74\x61\x6D\x70\x65\x64\x3A\x20\x2D\x76\x69\x73\x69\x74\x6F\x72\x2D",
	"\x50\x69\x63\x74\x75\x72\x65\x20\x6F\x66\x20\x6D\x65\x20\x73\x74\x61\x6D\x70\x65\x64\x20\x2D\x6D\x61\x69\x6E\x74\x65\x6E\x61\x6E\x63\x65\x2D",
	"\x53\x75\x72\x67\x69\x63\x61\x6C\x6C\x79\x20\x69\x6D\x70\x6C\x61\x6E\x74\x65\x64\x20\x62\x6F\x6D\x62\x20\x64\x65\x74\x65\x63\x74\x6F\x72\x20\x67\x6C\x6F\x77\x73\x20\x67\x72\x65\x65\x6E\x20\x28\x62\x6F\x6D\x62\x27\x73\x20\x2D\x73\x61\x66\x65\x2D\x29",
	"\x42\x6F\x6D\x62\x20\x64\x65\x74\x65\x63\x74\x6F\x72\x20\x67\x6C\x6F\x77\x69\x6E\x67\x20\x72\x65\x64\x20\x28\x66\x69\x6E\x61\x6C\x20\x63\x6F\x75\x6E\x74\x64\x6F\x77\x6E\x20\x61\x63\x74\x69\x76\x65\x29",
	"\x42\x6C\x75\x65\x20\x6B\x65\x79",
	"\x59\x65\x6C\x6C\x6F\x77\x20\x6B\x65\x79",
	"\x43\x6C\x6F\x73\x65\x64\x20\x77\x68\x69\x74\x65\x20\x64\x6F\x6F\x72\x20\x77\x69\x74\x68\x20\x74\x76\x20\x63\x61\x6D\x65\x72\x61\x20\x6D\x6F\x75\x6E\x74\x65\x64\x20\x6F\x76\x65\x72\x20\x69\x74",
	"\x50\x6C\x61\x74\x65\x20\x67\x6C\x61\x73\x73\x20\x77\x69\x6E\x64\x6F\x77\x20\x77\x69\x74\x68\x20\x65\x6D\x62\x65\x64\x65\x64\x20\x72\x65\x64\x20\x77\x69\x72\x65\x73",
	"\x50\x61\x6E\x65\x6C\x20\x6F\x66\x20\x62\x75\x74\x74\x6F\x6E\x73\x20\x2D\x77\x68\x69\x74\x65\x20\x67\x72\x65\x65\x6E\x2D",
	"\x4F\x6C\x64\x20\x66\x61\x73\x68\x69\x6F\x6E\x65\x64\x20\x79\x61\x72\x6E\x20\x6D\x6F\x70",
	"\x45\x6D\x70\x74\x79\x20\x77\x69\x6E\x64\x6F\x77\x20\x66\x72\x61\x6D\x65",
	"\x54\x76\x20\x63\x61\x6D\x65\x72\x61\x20\x6D\x6F\x75\x6E\x74\x65\x64\x20\x6F\x76\x65\x72\x20\x77\x69\x6E\x64\x6F\x77",
	"\x42\x72\x6F\x6B\x65\x6E\x20\x67\x6C\x61\x73\x73",
	"\x45\x6D\x70\x74\x79\x20\x6D\x6F\x76\x69\x65\x20\x70\x72\x6F\x6A\x65\x63\x74\x6F\x72",
	"\x4D\x6F\x76\x69\x65\x20\x70\x72\x6F\x6A\x65\x63\x74\x6F\x72\x20\x77\x69\x74\x68\x20\x66\x69\x6C\x6D\x20\x63\x61\x72\x74\x72\x69\x64\x67\x65",
	"\x4D\x6F\x76\x69\x65\x20\x66\x69\x6C\x6D\x20\x63\x61\x72\x74\x72\x69\x64\x67\x65",
	"\x45\x6D\x70\x74\x79\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x70\x61\x69\x6C",
	"\x57\x61\x74\x65\x72\x20\x66\x69\x6C\x6C\x65\x64\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x70\x61\x69\x6C",
	"\x56\x61\x74\x20\x6F\x66\x20\x68\x65\x61\x76\x79\x20\x77\x61\x74\x65\x72",
	"\x57\x69\x72\x65\x20\x63\x75\x74\x74\x65\x72\x73",
	"\x41\x6E\x74\x69\x2D\x72\x61\x64\x69\x61\x74\x69\x6F\x6E\x20\x73\x75\x69\x74",
	"\x56\x65\x72\x79\x20\x6C\x61\x72\x67\x65\x20\x74\x69\x6D\x65\x20\x62\x6F\x6D\x62",
	"\x52\x65\x64\x20\x77\x69\x72\x65\x20\x67\x6F\x69\x6E\x67\x20\x66\x72\x6F\x6D\x20\x62\x6F\x6D\x62\x20\x69\x6E\x74\x6F\x20\x77\x61\x6C\x6C",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x6C\x75\x6D\x70\x20\x6F\x66\x20\x67\x6C\x6F\x77\x69\x6E\x67\x20\x70\x6C\x61\x73\x74\x69\x63",
	"\x53\x69\x67\x6E\x20\x22\x4E\x6F\x20\x62\x65\x76\x65\x72\x61\x67\x65\x73\x2C\x20\x50\x6C\x65\x61\x73\x65\x20\x75\x73\x65\x20\x42\x72\x65\x61\x6B\x20\x52\x6F\x6F\x6D\x2E\x22",
	"\x59\x65\x6C\x6C\x6F\x77\x20\x64\x6F\x6F\x72\x20\x77\x69\x74\x68\x20\x74\x76\x20\x63\x61\x6D\x65\x72\x61\x20\x6F\x76\x65\x72\x20\x69\x74",
	"\x4D\x65\x74\x61\x6C\x20\x64\x6F\x6F\x72\x20\x6A\x61\x6D\x6D\x65\x64\x20\x70\x61\x72\x74\x69\x61\x6C\x6C\x79\x20\x6F\x70\x65\x6E\x20\x62\x79\x20\x72\x65\x6D\x61\x69\x6E\x73\x20\x6F\x66\x20\x61\x20\x74\x61\x70\x65\x20\x72\x65\x63\x6F\x72\x64\x65\x72",
	"\x42\x6C\x75\x65\x20\x64\x6F\x6F\x72\x20\x77\x69\x74\x68\x20\x74\x76\x20\x63\x61\x6D\x65\x72\x61\x20\x6F\x76\x65\x72\x20\x69\x74",
	"\x50\x6C\x61\x69\x6E\x20\x6D\x65\x74\x61\x6C\x20\x64\x6F\x6F\x72\x20\x77\x69\x74\x68\x20\x73\x69\x67\x6E\x20\x2D\x63\x6F\x6E\x74\x72\x6F\x6C\x20\x72\x6F\x6F\x6D\x2D",
	"\x54\x68\x65\x20\x64\x6F\x6F\x72\x20\x69\x73\x20\x70\x61\x72\x74\x69\x61\x6C\x6C\x79\x20\x6F\x70\x65\x6E",
	"\x45\x6D\x70\x74\x79\x20\x70\x69\x6C\x6C\x20\x63\x61\x73\x65",
	"\x36\x20\x69\x6E\x63\x68\x20\x77\x69\x6E\x64\x6F\x77",
	"\x45\x6D\x70\x74\x79\x20\x6D\x61\x6E\x69\x6C\x61\x20\x65\x6E\x76\x65\x6C\x6F\x70\x65",
	"\x50\x69\x65\x63\x65\x20\x6F\x66\x20\x79\x61\x72\x6E",
	"\x50\x69\x63\x74\x75\x72\x65\x20\x6F\x66\x20\x73\x61\x62\x6F\x74\x65\x75\x72\x20\x73\x74\x61\x6D\x70\x65\x64\x20\x2D\x77\x69\x6E\x64\x6F\x77\x20\x6D\x61\x69\x6E\x74\x61\x6E\x63\x65\x2D",
	"\x44\x65\x61\x64\x20\x73\x61\x62\x6F\x74\x65\x75\x72",
	"\x4C\x6F\x6F\x73\x65\x20\x72\x65\x64\x20\x77\x69\x72\x65\x20\x67\x6F\x69\x6E\x67\x20\x69\x6E\x74\x6F\x20\x77\x61\x6C\x6C",
	"\x53\x74\x65\x70\x73\x20\x6C\x65\x61\x64\x69\x6E\x67\x20\x64\x6F\x77\x6E\x20\x69\x6E\x74\x6F\x20\x74\x68\x65\x20\x72\x65\x61\x63\x74\x6F\x72\x20\x63\x6F\x72\x65",
	"\x59\x65\x6C\x6C\x6F\x77\x20\x62\x75\x74\x74\x6F\x6E",
	"\x42\x6C\x75\x65\x20\x62\x75\x74\x74\x6F\x6E",
	"\x4B\x65\x79\x68\x6F\x6C\x65\x73\x20\x75\x6E\x64\x65\x72\x20\x62\x75\x74\x74\x6F\x6E\x73",
	"\x45\x78\x70\x6F\x73\x65\x64\x20\x64\x69\x61\x6C\x73\x20\x61\x6E\x64\x20\x67\x61\x75\x67\x65\x73\x20\x65\x76\x65\x72\x79\x77\x68\x65\x72\x65",
	"\x41\x20\x6C\x65\x61\x66\x6C\x65\x74",
	"",
	"",
	"",
	"",
};
const char *msgptr[] = {
	"",
	"\x47\x6F\x6F\x64\x20\x6D\x6F\x72\x6E\x69\x6E\x67\x20\x4D\x72\x2E\x20\x50\x68\x65\x6C\x70\x73\x2E\x20\x59\x6F\x75\x72\x20\x4D\x69\x73\x73\x69\x6F\x6E\x20\x28\x73\x68\x6F\x75\x6C\x64\x20\x79\x6F\x75\x20\x64\x65\x63\x69\x64\x65\x20\x74\x6F\x0A\x61\x63\x63\x65\x70\x74\x20\x69\x74\x29\x20\x69\x73\x20\x74\x6F\x20\x70\x72\x65\x76\x65\x6E\x74\x20\x74\x68\x69\x73\x20\x61\x75\x74\x6F\x6D\x61\x74\x65\x64\x20\x6E\x75\x63\x6C\x65\x61\x72\x20\x72\x65\x61\x63\x74\x6F\x72\x0A\x66\x72\x6F\x6D\x20\x62\x65\x69\x6E\x67\x20\x64\x65\x73\x74\x72\x6F\x79\x65\x64\x20\x62\x79\x20\x61\x20\x73\x61\x62\x6F\x74\x65\x75\x72\x27\x73\x20\x54\x49\x4D\x45\x20\x42\x4F\x4D\x42\x21\x0A",
	"\x4E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x65\x64",
	"\x4D\x79\x20\x62\x6F\x6D\x62\x20\x64\x65\x74\x65\x63\x74\x6F\x72",
	"\x77\x61\x69\x6C\x73\x20\x41\x4C\x41\x52\x4D\x49\x4E\x47\x4C\x59\x21",
	"\x43\x4C\x49\x43\x4B\x21",
	"\x49\x20\x63\x61\x6E\x27\x74\x20\x64\x6F\x20\x74\x68\x61\x74",
	"\x49\x74\x20\x61\x70\x70\x65\x61\x72\x73\x20\x6C\x6F\x63\x6B\x65\x64",
	"\x62\x75\x74\x74\x6F\x6E\x20\x69\x73\x20\x6E\x6F\x77\x20\x75\x6E\x6C\x6F\x63\x6B\x65\x64",
	"\x42\x6C\x75\x65",
	"\x59\x65\x6C\x6C\x6F\x77",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x42\x72\x69\x67\x68\x74\x20\x66\x6C\x61\x73\x68\x20\x26\x20\x49\x20\x68\x65\x61\x72\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x66\x61\x6C\x6C\x20\x74\x6F\x20\x74\x68\x65\x20\x66\x6C\x6F\x6F\x72\x2E\x0A\x49\x20\x63\x61\x6E\x27\x74\x20\x73\x65\x65\x20\x77\x68\x61\x74\x20\x69\x74\x20\x69\x73\x20\x66\x72\x6F\x6D\x20\x68\x65\x72\x65\x20\x74\x68\x6F\x75\x67\x68\x2E\x0A",
	"\x2A\x20\x20\x20\x42\x20\x4F\x20\x4F\x20\x4F\x20\x4F\x20\x4F\x20\x4F\x20\x4F\x20\x4F\x20\x4F\x20\x4D\x20\x21\x20\x2A\x20\x2A\x20\x2A\x20\x2A\x20\x2A\x20\x2A",
	"\x54\x68\x65\x20\x73\x61\x62\x6F\x74\x65\x75\x72\x20\x28\x77\x68\x6F\x20\x61\x6C\x73\x6F\x20\x72\x65\x77\x69\x72\x65\x64\x20\x74\x68\x65\x20\x73\x65\x63\x75\x72\x69\x74\x79\x20\x73\x79\x73\x74\x65\x6D\x29\x20\x69\x73\x20\x61\x20\x68\x65\x61\x72\x74\x0A\x70\x61\x74\x69\x65\x6E\x74\x2E\x20\x48\x65\x20\x70\x6C\x61\x6E\x73\x20\x74\x6F\x20\x53\x55\x49\x43\x49\x44\x45\x20\x77\x69\x74\x68\x20\x74\x68\x65\x20\x72\x65\x61\x63\x74\x6F\x72\x21\x20\x48\x65\x20\x69\x73\x20\x73\x74\x69\x6C\x6C\x20\x6C\x6F\x6F\x73\x65\x69\x6E\x20\x74\x68\x65\x20\x62\x75\x69\x6C\x64\x69\x6E\x67\x2E\x0A\x59\x6F\x75\x27\x6C\x6C\x20\x66\x69\x6E\x64\x20\x53\x65\x63\x75\x72\x69\x74\x79\x20\x6B\x65\x79\x73\x20\x26\x20\x61\x20\x6D\x61\x70\x20\x69\x6E\x20\x74\x68\x65\x20\x6D\x61\x6E\x69\x6C\x61\x20\x65\x6E\x76\x65\x6C\x6F\x70\x65\x20\x6C\x79\x69\x6E\x67\x0A\x6E\x65\x78\x74\x20\x74\x6F\x20\x74\x68\x65\x20\x74\x61\x70\x65\x20\x70\x6C\x61\x79\x65\x72\x2E",
	"\x70\x6F\x6C\x69\x74\x65\x6C\x79\x20\x62\x65\x65\x70\x73\x2E\x2E\x2E\x0A",
	"\x61\x6E\x67\x72\x69\x6C\x79\x20\x62\x75\x7A\x7A\x65\x73\x2E\x2E\x2E\x0A",
	"\x53\x74\x72\x61\x6E\x67\x65\x2E\x2E\x2E",
	"\x4D\x6F\x73\x74\x20\x64\x6F\x6F\x72\x73\x20\x26\x20\x77\x69\x6E\x64\x6F\x77\x73\x20\x61\x72\x65\x20\x75\x6E\x64\x65\x72\x20\x61\x75\x74\x6F\x6D\x61\x74\x65\x64\x20\x73\x65\x63\x75\x72\x69\x74\x79\x20\x63\x6F\x6E\x74\x72\x6F\x6C\x2E",
	"\x4D\x65\x74\x61\x6C\x6C\x69\x63\x20\x76\x6F\x69\x63\x65\x20\x73\x61\x79\x73\x3A",
	"\x22\x53\x68\x6F\x77\x20\x61\x75\x74\x68\x6F\x72\x69\x7A\x61\x74\x69\x6F\x6E\x20\x70\x6C\x65\x61\x73\x65\x22\x0A",
	"\x22\x41\x75\x74\x68\x6F\x72\x69\x7A\x61\x74\x69\x6F\x6E\x20\x49\x4E\x53\x55\x46\x46\x49\x43\x49\x45\x4E\x54\x20\x66\x6F\x72\x20\x61\x63\x63\x65\x73\x73\x22\x0A",
	"\x54\x68\x65\x20\x64\x6F\x6F\x72\x20\x6F\x70\x65\x6E\x73\x20\x6A\x75\x73\x74\x20\x6C\x6F\x6E\x67\x20\x65\x6E\x6F\x75\x67\x68\x20\x66\x6F\x72\x20\x6D\x65\x20\x74\x6F\x20\x73\x63\x75\x72\x72\x79\x20\x74\x68\x72\x6F\x75\x67\x68\x2E",
	"\x49\x27\x6D\x20\x6F\x6E\x20\x74\x68\x65\x20\x32\x6E\x64\x20\x66\x6C\x6F\x6F\x72\x2E\x20\x42\x65\x6C\x6F\x77\x20\x49\x20\x73\x65\x65\x20\x74\x68\x65\x20\x63\x6F\x6E\x74\x72\x6F\x6C\x20\x72\x6F\x6F\x6D\x20\x73\x75\x72\x72\x6F\x6E\x64\x69\x6E\x67\x0A\x74\x68\x65\x20\x72\x65\x61\x63\x74\x6F\x72\x20\x63\x6F\x72\x65\x2E\x20\x54\x68\x65\x72\x65\x27\x73\x20\x61\x20\x77\x69\x64\x65\x20\x6C\x65\x64\x67\x65\x20\x6A\x75\x73\x74\x20\x75\x6E\x64\x65\x72\x20\x74\x68\x65\x20\x77\x69\x6E\x64\x6F\x77\x2E",
	"\x49\x6E\x20\x74\x68\x69\x73\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x74\x68\x65\x72\x65\x27\x73\x20\x4E\x4F\x20\x73\x63\x6F\x72\x65\x2E\x20\x45\x69\x74\x68\x65\x72\x20\x79\x6F\x75\x20\x6D\x61\x6B\x65\x20\x69\x74\x20\x6F\x72\x20\x2E\x2E\x2E\x0A",
	"\x54\x65\x6C\x6C\x20\x6D\x65\x20\x77\x69\x74\x68\x20\x77\x68\x61\x74\x3F\x20\x45\x78\x61\x6D\x70\x6C\x65\x3A\x20\x22\x57\x49\x54\x48\x20\x46\x49\x53\x54\x22",
	"\x42\x65\x20\x73\x75\x72\x65\x20\x49\x27\x6D\x20\x63\x61\x72\x72\x79\x69\x6E\x67\x20\x69\x74\x21",
	"\x52\x65\x63\x6F\x72\x64\x65\x72\x20\x67\x6F\x65\x73\x20\x66\x6C\x79\x69\x6E\x67\x20\x74\x68\x72\x75\x20\x74\x68\x65\x20\x67\x6C\x61\x73\x73\x20\x6C\x61\x6E\x64\x69\x6E\x67\x20\x69\x6E\x20\x74\x68\x65\x20\x63\x6F\x6E\x74\x72\x6F\x6C\x20\x72\x6F\x6F\x6D\x2E\x42\x6F\x79\x20\x77\x68\x61\x74\x20\x61\x20\x4D\x45\x53\x53\x21\x0A",
	"\x43\x6C\x69\x63\x6B\x21\x0A\x52\x6F\x6F\x6D\x20\x6C\x69\x67\x68\x74\x73\x20\x64\x69\x6D\x20\x61\x6E\x64\x20\x61\x20\x73\x63\x72\x65\x65\x6E\x20\x64\x72\x6F\x70\x73\x20\x66\x72\x6F\x6D\x20\x74\x68\x65\x20\x63\x65\x69\x6C\x69\x6E\x67\x2E\x0A\x59\x6F\x75\x20\x68\x65\x61\x72\x20\x61\x20\x68\x69\x64\x64\x65\x6E\x20\x70\x72\x6F\x6A\x65\x63\x74\x6F\x72\x20\x73\x74\x61\x72\x74\x2E",
	"\x54\x68\x65\x20\x73\x63\x72\x65\x65\x6E\x20\x69\x6C\x6C\x75\x6D\x69\x6E\x61\x74\x65\x73\x20\x66\x6F\x72\x20\x61\x77\x68\x69\x6C\x65\x2C\x20\x62\x75\x74\x20\x6E\x6F\x20\x6D\x6F\x76\x69\x65\x21\x3F",
	"\x54\x68\x65\x20\x70\x72\x6F\x6A\x65\x63\x74\x6F\x72\x20\x73\x74\x6F\x70\x73\x2C\x20\x74\x68\x65\x20\x73\x63\x72\x65\x65\x6E\x20\x72\x69\x73\x65\x73\x2C\x20\x61\x6E\x64\x20\x74\x68\x65\x20\x6C\x69\x67\x68\x74\x73\x20\x72\x65\x74\x75\x72\x6E\x2E",
	"\x41\x20\x6D\x6F\x76\x69\x65\x20\x72\x75\x6E\x73\x20\x74\x65\x6C\x6C\x69\x6E\x67\x20\x61\x62\x6F\x75\x74\x20\x74\x68\x65\x20\x72\x65\x61\x63\x74\x6F\x72\x20\x61\x6E\x64\x20\x69\x74\x73\x20\x63\x6F\x6E\x73\x74\x72\x75\x63\x74\x69\x6F\x6E\x0A\x69\x6E\x74\x65\x72\x65\x73\x74\x69\x6E\x67\x20\x68\x69\x67\x68\x6C\x69\x67\x68\x74\x73\x3A\x0A\x20\x31\x29\x20\x50\x6C\x61\x73\x74\x69\x63\x20\x44\x45\x46\x4F\x52\x4D\x53\x20\x73\x74\x72\x61\x6E\x67\x65\x6C\x79\x20\x69\x6E\x20\x72\x61\x64\x69\x61\x74\x69\x6F\x6E\x0A\x20\x32\x29\x20\x45\x76\x65\x6E\x20\x73\x68\x6F\x72\x74\x20\x65\x78\x70\x6F\x73\x75\x72\x65\x20\x74\x6F\x20\x48\x49\x47\x48\x20\x72\x61\x64\x69\x61\x74\x69\x6F\x6E\x20\x69\x73\x20\x4C\x45\x54\x48\x41\x4C\x2C\x20\x73\x6F\x20\x73\x75\x69\x74\x20\x75\x70\x0A",
	"\x54\x68\x65\x20\x72\x61\x64\x69\x61\x74\x69\x6F\x6E\x20\x64\x69\x64\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x74\x6F\x20\x6D\x79\x20\x70\x61\x69\x6C\x21",
	"\x48\x6F\x6C\x64\x69\x6E\x67\x20\x61\x20\x52\x61\x64\x69\x61\x74\x69\x6F\x6E\x20\x53\x75\x69\x74",
	"\x64\x6F\x65\x73\x6E\x27\x74\x20\x70\x72\x6F\x76\x69\x64\x65\x20\x6D\x75\x63\x68\x20\x70\x72\x6F\x74\x65\x63\x74\x69\x6F\x6E\x20\x66\x6F\x72\x20\x6D\x65\x0A\x20\x61\x73\x20\x49\x20\x66\x61\x6C\x6C\x20\x64\x6F\x77\x6E\x20\x72\x65\x74\x63\x68\x69\x6E\x67\x20\x49\x20\x68\x65\x61\x72\x2F\x46\x45\x45\x4C\x20\x54\x48\x45\x20\x42\x4F\x4D\x42\x20\x65\x78\x70\x6C\x6F\x64\x65\x21\x0A",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x6E\x75\x6D\x62\x65\x72\x20\x33\x20\x22\x4D\x49\x53\x53\x49\x4F\x4E\x20\x49\x4D\x50\x4F\x53\x53\x49\x42\x4C\x45\x22\x0A\x20\x62\x79\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x2C\x20\x44\x65\x64\x69\x63\x61\x74\x65\x64\x20\x74\x6F\x20\x4D\x61\x65\x67\x65\x6E\x20\x41\x64\x61\x6D\x73\x2E\x0A",
	"\x41\x20\x62\x75\x73\x69\x6E\x65\x73\x73\x20\x73\x75\x69\x74",
	"\x49\x27\x6D\x20\x57\x45\x41\x52\x49\x4E\x47\x20\x61\x6E\x20\x61\x6E\x74\x69\x2D\x72\x61\x64\x69\x61\x74\x69\x6F\x6E\x20\x73\x75\x69\x74\x2E",
	"\x49\x74\x20\x77\x6F\x6E\x27\x74\x20\x62\x75\x64\x67\x65\x21",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x77\x6F\x6E\x27\x74\x20\x66\x69\x74\x20\x74\x68\x72\x75\x20\x74\x68\x65\x20\x64\x6F\x6F\x72\x2E",
	"\x49\x20\x74\x75\x72\x6E\x20\x74\x68\x65\x20\x6B\x6E\x6F\x62\x20\x61\x6E\x64\x20\x70\x75\x73\x68",
	"\x67\x65\x6E\x74\x6C\x79\x20\x6F\x6E\x20\x74\x68\x65\x20\x64\x6F\x6F\x72",
	"\x68\x61\x72\x64\x20\x6F\x6E\x20\x74\x68\x65\x20\x64\x6F\x6F\x72",
	"\x69\x74\x20\x6F\x70\x65\x6E\x73\x20\x73\x6C\x69\x67\x68\x74\x6C\x79",
	"\x69\x74\x73\x20\x63\x6C\x6F\x73\x65\x64",
	"\x74\x68\x65\x20\x77\x61\x74\x65\x72\x20\x73\x70\x69\x6C\x6C\x73\x20\x6F\x6E\x20\x74\x68\x65\x20\x62\x6F\x6D\x62\x20\x61\x6E\x64",
	"\x74\x68\x65\x20\x63\x6F\x6E\x74\x72\x6F\x6C\x20\x70\x61\x6E\x65\x6C\x20\x77\x68\x69\x63\x68\x20\x69\x6D\x6D\x65\x64\x69\x61\x74\x65\x6C\x79\x20\x73\x68\x6F\x72\x74\x73\x20\x6F\x75\x74\x20\x74\x72\x69\x67\x67\x65\x72\x69\x6E\x67\x0A\x74\x68\x65\x20\x62\x6F\x6D\x62",
	"\x64\x65\x66\x75\x73\x65\x73\x20\x69\x74\x21\x20\x46\x41\x4E\x54\x41\x53\x54\x49\x43\x2C\x20\x59\x6F\x75\x20\x63\x6F\x6D\x70\x6C\x65\x74\x65\x64\x20\x61\x6E\x20\x49\x4D\x50\x4F\x53\x53\x49\x42\x4C\x45\x20\x6D\x69\x73\x73\x69\x6F\x6E\x21",
	"\x74\x68\x65\x20\x77\x61\x74\x65\x72\x20\x73\x6F\x61\x6B\x73\x20\x69\x6E\x74\x6F\x20\x74\x68\x65\x20\x66\x6C\x6F\x6F\x72",
	"\x73\x61\x79\x20\x61\x67\x61\x69\x6E\x20\x26\x20\x75\x73\x65\x20\x61\x20\x63\x6F\x6C\x6F\x72\x21",
	"\x49\x27\x6D\x20\x6C\x6F\x6F\x6B\x69\x6E\x67\x20\x69\x6E\x74\x6F\x20\x74\x68\x65\x20\x63\x6F\x6E\x74\x72\x6F\x6C\x20\x72\x6F\x6F\x6D\x2E\x20\x49\x20\x6E\x6F\x74\x69\x63\x65\x20\x74\x68\x61\x74\x20\x74\x68\x65\x20\x64\x6F\x6F\x72\x20\x69\x73\x0A\x20\x62\x6C\x6F\x63\x6B\x65\x64\x20\x62\x79\x20\x73\x6F\x6D\x65\x20\x64\x65\x62\x72\x69\x73\x2E",
	"\x49\x6E\x20\x74\x68\x65\x20\x64\x69\x73\x74\x61\x6E\x63\x65\x20\x79\x6F\x75\x20\x68\x65\x61\x72\x20\x61\x20\x64\x75\x6C\x6C\x20\x74\x68\x75\x64\x3B\x20\x61\x73\x20\x69\x66\x20\x73\x6F\x6D\x65\x6F\x6E\x65\x20\x66\x65\x6C\x6C\x20\x6F\x72\x0A\x20\x64\x72\x6F\x70\x70\x65\x64\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x68\x65\x61\x76\x79\x2E",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x66\x65\x6C\x6C\x20\x74\x6F\x20\x74\x68\x65\x20\x66\x6C\x6F\x6F\x72\x2E",
	"\x69\x74\x73\x20\x75\x6E\x72\x65\x61\x64\x61\x62\x6C\x65",
	"\x69\x74\x73\x20\x73\x74\x69\x6C\x6C\x20\x63\x6F\x6E\x6E\x65\x63\x74\x65\x64",
	"\x4F\x68\x20\x42\x6F\x79\x2E\x2E\x2E\x49\x20\x74\x68\x69\x6E\x6B\x20\x49\x20\x64\x69\x64\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x2E\x2E\x2E",
	"\x49\x74\x20\x6D\x61\x64\x65\x20\x61\x6E\x20\x6F\x64\x64\x20\x73\x6F\x75\x6E\x64\x2E",
	"\x22\x54\x56\x20\x64\x65\x61\x63\x74\x69\x76\x61\x74\x65\x64\x22\x0A",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x6B\x6E\x6F\x77\x20\x77\x68\x65\x72\x65\x20\x74\x6F\x20\x6C\x6F\x6F\x6B",
	"\x54\x65\x6C\x6C\x20\x6D\x65\x20\x68\x6F\x77\x3F",
	"\x54\x72\x79\x20\x73\x74\x61\x72\x74\x69\x6E\x67\x20\x69\x74\x2E",
	"\x4D\x61\x79\x62\x65\x20\x49\x20\x73\x68\x6F\x75\x6C\x64\x20\x46\x52\x49\x53\x4B\x20\x68\x69\x6D\x3F",
	"\x49\x74\x73\x20\x61\x20\x73\x65\x61\x6D\x6C\x65\x73\x73\x20\x62\x6F\x78\x20\x77\x69\x74\x68\x20\x31\x20\x73\x6D\x61\x6C\x6C\x20\x68\x6F\x6C\x65\x20\x66\x6F\x72\x20\x74\x68\x65\x20\x72\x65\x64\x20\x77\x69\x72\x65\x2E",
	"\x54\x56\x20\x63\x61\x6D\x65\x72\x61\x20\x69\x73\x20\x73\x6C\x6F\x77\x20\x73\x63\x61\x6E\x6E\x69\x6E\x67\x20\x74\x68\x65\x20\x77\x69\x6E\x64\x6F\x77\x20\x61\x72\x65\x61\x2E",
	"\x54\x56\x20\x63\x61\x6D\x65\x72\x61\x20\x69\x73\x20\x70\x6F\x77\x65\x72\x65\x64\x20\x64\x6F\x77\x6E\x2E\x0A",
	"\x49\x74\x20\x73\x65\x65\x6D\x73\x20\x73\x61\x66\x65\x2E\x20\x53\x68\x61\x6C\x6C\x20\x49\x20\x73\x69\x74\x20\x64\x6F\x77\x6E\x3F",
	"\x4F\x6E\x6C\x79\x20\x68\x65\x6C\x70\x20\x49\x20\x63\x61\x6E\x20\x74\x68\x69\x6E\x6B\x20\x6F\x66\x20\x69\x73\x20\x74\x6F\x3A\x20\x22\x45\x78\x61\x6D\x69\x6E\x65\x20\x65\x76\x65\x72\x79\x74\x68\x69\x6E\x67\x20\x63\x6C\x6F\x73\x65\x6C\x79\x21\x22\x0A",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C\x2E",
	"\x48\x55\x48\x3F",
	"\x41\x20\x6D\x69\x6E\x75\x74\x65\x20\x61\x67\x6F\x20\x73\x6F\x6D\x65\x6F\x6E\x65\x20\x72\x61\x6E\x20\x6F\x75\x74\x20\x6F\x66\x20\x74\x68\x69\x73\x20\x72\x6F\x6F\x6D\x21\x20\x42\x79\x20\x74\x68\x65\x20\x77\x61\x79\x20\x49\x20\x73\x65\x65\x6D\x0A\x74\x6F\x20\x62\x65\x20\x63\x61\x72\x72\x79\x69\x6E\x67\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x21\x0A",
	"\x53\x6F\x6D\x65\x6F\x6E\x65\x20\x63\x61\x6D\x65\x20\x69\x6E\x20\x74\x68\x65\x20\x72\x6F\x6F\x6D\x2C\x20\x68\x65\x20\x73\x61\x77\x20\x6D\x65\x20\x61\x6E\x64\x20\x72\x61\x6E\x20\x6F\x75\x74\x21",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x20\x6F\x6E\x65\x20\x68\x65\x72\x65\x2E",
	"\x22\x4F\x77\x6E\x65\x72\x20\x6F\x66\x20\x62\x61\x64\x67\x65\x20\x69\x73\x20\x6E\x6F\x74\x20\x70\x72\x65\x73\x65\x6E\x74\x21\x22",
	"\x48\x69\x21\x20\x4C\x6F\x6F\x6B\x20\x66\x6F\x72\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x6E\x75\x6D\x62\x65\x72\x20\x34\x3A\x20\x22\x56\x4F\x4F\x44\x4F\x4F\x20\x43\x41\x53\x54\x4C\x45\x22\x20\x61\x74\x20\x79\x6F\x75\x72\x0A\x66\x61\x76\x6F\x72\x69\x74\x65\x20\x63\x6F\x6D\x70\x75\x74\x65\x72\x20\x73\x74\x6F\x72\x65\x21\x20\x28\x4E\x6F\x77\x20\x62\x61\x63\x6B\x20\x74\x6F\x20\x6F\x75\x72\x20\x63\x75\x72\x72\x65\x6E\x74\x20\x70\x72\x6F\x67\x72\x61\x6D\x2E\x29",
	"\x46\x69\x6E\x61\x6C\x20\x63\x6F\x75\x6E\x74\x64\x6F\x77\x6E\x20\x73\x74\x61\x72\x74\x73\x20\x69\x6E",
	"\x74\x75\x72\x6E\x73\x21",
	"\x53\x6F\x6D\x65\x20\x74\x69\x6D\x65\x20\x70\x61\x73\x73\x65\x73\x2E\x2E\x2E",
	"\x54\x68\x65\x72\x65\x20\x69\x73\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x77\x72\x69\x74\x74\x65\x6E\x20\x74\x68\x65\x72\x65",
	"\x4F\x57\x21\x20\x54\x68\x61\x74\x20\x48\x55\x52\x54\x21\x20\x41\x6E\x79\x77\x61\x79",
	"\x22\x41\x43\x43\x45\x50\x54\x45\x44\x22",
	"",
	"",
	"",
};


const uint8_t status[] = {
	136, 15, 
	14, 29, 0, 9, 
	59, 
	179, 
	14, 9, 6, 10, 0, 2, 0, 10, 
	59, 3, 4, 74, 
	144, 30, 
	12, 3, 14, 42, 0, 3, 17, 3, 
	59, 
	160, 
	77, 
	135, 9, 
	1, 10, 
	70, 66, 12, 63, 
	174, 
	15, 40, 16, 0, 6, 10, 
	123, 78, 124, 
	175, 
	9, 12, 0, 12, 32, 44, 
	34, 118, 58, 79, 
	149, 30, 
	14, 42, 7, 10, 0, 42, 0, 10, 47, 19, 
	62, 50, 
	149, 30, 
	14, 42, 7, 4, 0, 42, 0, 4, 47, 19, 
	62, 50, 
	149, 30, 
	14, 42, 7, 3, 0, 42, 0, 3, 47, 19, 
	62, 50, 
	133, 25, 
	4, 10, 
	18, 19, 
	132, 15, 
	14, 42, 
	119, 
	133, 25, 
	4, 9, 
	18, 19, 
	133, 25, 
	4, 11, 
	18, 19, 
	136, 80, 
	4, 13, 8, 13, 
	112, 
	136, 40, 
	4, 13, 9, 13, 
	113, 
	172, 
	8, 1, 1, 9, 0, 1, 
	60, 
	172, 
	8, 2, 1, 2, 0, 2, 
	60, 
	183, 
	8, 1, 0, 9, 13, 9, 0, 2, 0, 1, 
	3, 72, 14, 60, 
	183, 
	8, 2, 13, 9, 0, 9, 0, 2, 0, 2, 
	3, 72, 15, 60, 
	146, 80, 
	4, 21, 1, 24, 0, 24, 0, 30, 
	72, 31, 47, 
	138, 80, 
	4, 21, 1, 27, 
	32, 33, 63, 
	138, 80, 
	4, 21, 13, 27, 
	35, 33, 63, 
	168, 
	15, 0, 0, 9, 
	59, 
};
const uint8_t actions[] = {
	18, 34, 14, 
	4, 13, 12, 42, 3, 41, 8, 13, 
	18, 20, 121, 
	5, 24, 52, 
	3, 49, 
	70, 122, 
	13, 54, 42, 
	3, 16, 14, 11, 0, 11, 
	53, 51, 
	6, 43, 20, 
	7, 15, 
	6, 7, 17, 
	19, 25, 10, 
	6, 9, 2, 6, 0, 1, 0, 1, 
	58, 54, 73, 5, 
	210, 
	2, 1, 0, 1, 0, 4, 0, 5, 
	62, 54, 11, 
	210, 
	2, 8, 0, 8, 0, 4, 0, 5, 
	62, 54, 11, 
	210, 
	2, 7, 0, 7, 0, 4, 0, 5, 
	62, 54, 11, 
	202, 
	4, 1, 0, 5, 
	54, 2, 16, 
	9, 13, 0, 
	3, 16, 13, 11, 
	117, 2, 
	10, 32, 0, 
	4, 16, 0, 9, 
	104, 12, 63, 
	8, 40, 42, 
	3, 16, 14, 11, 
	105, 
	13, 25, 9, 
	2, 6, 0, 2, 6, 2, 
	58, 5, 
	0, 37, 19, 
	48, 
	9, 25, 11, 
	9, 4, 2, 6, 
	6, 7, 
	9, 25, 12, 
	9, 3, 2, 6, 
	6, 7, 
	17, 25, 11, 
	2, 6, 14, 1, 0, 1, 0, 1, 
	62, 5, 
	0, 25, 19, 
	48, 
	17, 25, 12, 
	2, 6, 14, 8, 0, 8, 0, 1, 
	62, 5, 
	13, 25, 9, 
	2, 6, 0, 9, 1, 2, 
	5, 59, 
	4, 25, 0, 
	2, 6, 
	5, 
	10, 57, 5, 
	4, 5, 0, 4, 
	54, 70, 64, 
	8, 10, 7, 
	3, 3, 0, 3, 
	52, 
	10, 10, 5, 
	4, 5, 0, 4, 
	54, 70, 64, 
	5, 5, 0, 
	13, 27, 
	70, 66, 
	6, 5, 0, 
	14, 27, 
	70, 66, 36, 
	4, 40, 7, 
	3, 3, 
	109, 
	0, 49, 13, 
	71, 
	1, 50, 0, 
	12, 63, 
	5, 40, 39, 
	3, 42, 
	116, 110, 
	11, 25, 46, 
	2, 15, 13, 20, 
	27, 16, 28, 29, 
	11, 25, 46, 
	2, 15, 14, 20, 
	70, 27, 30, 29, 
	6, 57, 7, 
	3, 3, 
	70, 1, 13, 
	18, 37, 0, 
	2, 6, 3, 11, 0, 4, 9, 4, 
	58, 9, 8, 
	18, 37, 0, 
	2, 6, 3, 12, 0, 3, 9, 3, 
	58, 10, 8, 
	5, 10, 47, 
	13, 27, 
	70, 66, 
	6, 10, 47, 
	14, 27, 
	70, 66, 36, 
	10, 25, 10, 
	4, 13, 0, 9, 
	5, 54, 21, 
	11, 25, 12, 
	4, 14, 0, 10, 
	70, 5, 21, 54, 
	19, 1, 21, 
	2, 17, 8, 13, 0, 9, 0, 16, 
	59, 54, 70, 64, 
	14, 1, 21, 
	2, 17, 9, 13, 0, 16, 
	54, 70, 64, 
	10, 1, 21, 
	4, 16, 0, 13, 
	54, 70, 64, 
	11, 25, 11, 
	4, 15, 0, 11, 
	70, 5, 21, 54, 
	23, 29, 0, 
	3, 22, 3, 20, 0, 20, 0, 21, 0, 22, 
	72, 59, 70, 64, 
	19, 37, 0, 
	3, 21, 0, 22, 0, 21, 0, 20, 
	52, 72, 70, 64, 
	9, 45, 29, 
	3, 28, 0, 8, 
	24, 58, 
	19, 15, 0, 
	8, 8, 0, 8, 0, 9, 3, 28, 
	60, 105, 59, 104, 
	12, 10, 29, 
	2, 28, 14, 29, 0, 28, 
	52, 
	0, 22, 0, 
	120, 
	10, 10, 29, 
	2, 28, 14, 43, 
	0, 6, 103, 
	15, 18, 29, 
	1, 28, 0, 9, 0, 28, 
	105, 59, 104, 53, 
	15, 6, 0, 
	4, 19, 3, 24, 3, 28, 
	70, 44, 45, 63, 
	14, 6, 0, 
	4, 20, 3, 24, 3, 28, 
	44, 46, 63, 
	23, 45, 31, 
	2, 29, 3, 26, 0, 43, 0, 29, 0, 2, 
	72, 58, 70, 64, 
	14, 18, 42, 
	1, 16, 14, 11, 0, 16, 
	16, 105, 53, 
	13, 6, 0, 
	3, 24, 0, 24, 0, 23, 
	72, 47, 
	15, 34, 14, 
	4, 9, 1, 7, 0, 13, 
	18, 128, 21, 54, 
	15, 34, 14, 
	4, 9, 1, 8, 0, 13, 
	18, 128, 21, 54, 
	15, 34, 14, 
	4, 9, 1, 1, 0, 13, 
	18, 128, 21, 54, 
	12, 18, 42, 
	1, 16, 13, 11, 0, 16, 
	53, 
	15, 34, 14, 
	4, 10, 1, 1, 0, 14, 
	18, 128, 21, 54, 
	15, 34, 14, 
	4, 10, 1, 8, 0, 14, 
	18, 128, 21, 54, 
	15, 34, 14, 
	4, 11, 1, 1, 0, 15, 
	18, 128, 21, 54, 
	6, 34, 14, 
	4, 10, 
	70, 18, 20, 
	6, 34, 14, 
	4, 9, 
	70, 18, 20, 
	6, 34, 14, 
	4, 11, 
	70, 18, 20, 
	9, 13, 0, 
	3, 16, 14, 11, 
	104, 105, 
	18, 45, 21, 
	2, 14, 0, 7, 0, 10, 3, 3, 
	24, 58, 58, 
	23, 15, 7, 
	2, 14, 3, 3, 8, 7, 0, 3, 0, 1, 
	73, 62, 70, 26, 
	215, 
	0, 7, 0, 2, 0, 17, 0, 14, 0, 13, 
	60, 58, 72, 58, 
	1, 10, 33, 
	6, 37, 
	4, 40, 21, 
	4, 13, 
	22, 
	1, 51, 0, 
	70, 23, 
	10, 36, 0, 
	2, 5, 0, 5, 
	70, 54, 64, 
	2, 43, 21, 
	6, 7, 17, 
	14, 45, 42, 
	3, 16, 0, 10, 0, 11, 
	58, 58, 24, 
	23, 15, 32, 
	3, 26, 8, 11, 0, 11, 0, 10, 0, 11, 
	53, 51, 60, 60, 
	1, 37, 21, 
	2, 17, 
	18, 34, 14, 
	3, 42, 0, 13, 4, 13, 3, 41, 
	60, 18, 106, 
	4, 1, 33, 
	4, 4, 
	114, 
	4, 40, 33, 
	4, 4, 
	114, 
	4, 40, 29, 
	3, 28, 
	111, 
	0, 40, 30, 
	66, 
	8, 18, 48, 
	14, 27, 0, 27, 
	53, 
	8, 18, 48, 
	1, 27, 0, 27, 
	53, 
	5, 40, 21, 
	4, 22, 
	70, 49, 
	16, 10, 28, 
	3, 23, 2, 25, 0, 23, 0, 24, 
	72, 
	8, 52, 48, 
	3, 27, 0, 27, 
	59, 
	5, 10, 7, 
	2, 33, 
	6, 37, 
	5, 43, 20, 
	2, 33, 
	6, 37, 
	8, 1, 20, 
	2, 33, 1, 28, 
	38, 
	10, 1, 20, 
	2, 33, 0, 15, 
	70, 54, 64, 
	7, 43, 20, 
	2, 35, 
	16, 39, 40, 37, 
	7, 25, 20, 
	2, 35, 
	16, 39, 40, 37, 
	15, 25, 51, 
	2, 35, 5, 36, 0, 36, 
	39, 41, 42, 53, 
	11, 25, 51, 
	2, 35, 2, 36, 
	16, 39, 41, 37, 
	14, 1, 20, 
	2, 35, 2, 36, 0, 19, 
	54, 70, 64, 
	9, 1, 20, 
	2, 35, 5, 36, 
	6, 43, 
	14, 31, 20, 
	2, 35, 5, 36, 0, 36, 
	127, 42, 53, 
	0, 42, 20, 
	37, 
	8, 33, 5, 
	3, 27, 0, 27, 
	59, 
	9, 34, 14, 
	4, 13, 8, 13, 
	18, 20, 
	1, 37, 20, 
	2, 17, 
	23, 54, 39, 
	3, 42, 14, 39, 0, 39, 0, 40, 0, 41, 
	53, 53, 53, 73, 
	207, 
	0, 0, 0, 37, 0, 49, 
	53, 53, 51, 53, 
	200, 
	14, 3, 0, 3, 
	53, 
	6, 53, 0, 
	4, 9, 
	70, 18, 19, 
	0, 9, 0, 
	115, 
	0, 35, 0, 
	37, 
	0, 25, 0, 
	2, 
	6, 53, 0, 
	4, 10, 
	70, 18, 19, 
	6, 53, 0, 
	4, 11, 
	70, 18, 19, 
	0, 53, 0, 
	2, 
	4, 15, 0, 
	9, 10, 
	117, 
	10, 15, 53, 
	8, 10, 0, 10, 
	60, 127, 2, 
	0, 54, 0, 
	2, 
	1, 31, 0, 
	127, 2, 
	1, 34, 0, 
	16, 2, 
	1, 24, 0, 
	6, 102, 
	1, 38, 0, 
	6, 107, 
	5, 45, 0, 
	0, 10, 
	24, 58, 
	11, 15, 0, 
	8, 10, 0, 10, 
	60, 16, 2, 25, 
	0, 57, 0, 
	108, 
	5, 28, 0, 
	0, 30, 
	83, 125, 
	0, 16, 0, 
	108, 
	4, 40, 52, 
	3, 49, 
	126, 
	8, 40, 37, 
	4, 13, 8, 13, 
	112, 
	8, 40, 37, 
	4, 13, 9, 13, 
	113, 
	1, 40, 0, 
	116, 76, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84,
71, 79, 32,
197, 78, 84,
215, 65, 76,
210, 85, 78,
73, 78, 86,
80, 79, 85,
211, 80, 73,
197, 77, 80,
72, 69, 76,
71, 69, 84,
212, 65, 75,
205, 79, 86,
67, 76, 69,
205, 79, 80,
87, 73, 84,
85, 78, 65,
196, 73, 83,
68, 82, 79,
208, 85, 84,
204, 69, 65,
210, 69, 77,
67, 72, 65,
198, 79, 76,
82, 69, 65,
80, 82, 69,
212, 79, 85,
208, 85, 83,
87, 65, 73,
76, 79, 65,
201, 78, 83,
75, 73, 67,
74, 85, 77,
83, 85, 73,
83, 72, 79,
80, 85, 76,
83, 73, 84,
85, 78, 76,
70, 73, 78,
204, 79, 67,
76, 79, 79,
197, 88, 65,
67, 76, 79,
79, 80, 69,
46, 32, 32,
66, 82, 69,
211, 77, 65,
195, 85, 84,
213, 78, 66,
83, 65, 86,
81, 85, 73,
83, 67, 79,
87, 69, 65,
75, 78, 79,
70, 82, 73,
211, 69, 65,
211, 72, 65,
65, 67, 84,
211, 84, 65,
208, 76, 65,
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
84, 65, 80,
210, 69, 67,
82, 69, 68,
87, 72, 73,
66, 76, 85,
89, 69, 76,
71, 65, 77,
80, 73, 67,
208, 65, 83,
194, 65, 68,
193, 85, 84,
65, 82, 79,
66, 85, 84,
68, 79, 79,
87, 73, 78,
199, 76, 65,
89, 65, 82,
70, 73, 76,
195, 65, 82,
80, 82, 79,
67, 65, 83,
87, 65, 84,
66, 79, 77,
68, 69, 84,
87, 73, 82,
67, 85, 84,
67, 72, 65,
75, 69, 89,
69, 78, 86,
205, 65, 78,
67, 65, 77,
194, 79, 88,
83, 65, 66,
200, 73, 77,
212, 72, 69,
77, 79, 80,
77, 65, 80,
80, 65, 73,
208, 76, 65,
71, 82, 69,
73, 78, 86,
83, 85, 73,
193, 78, 84,
210, 65, 68,
72, 65, 82,
76, 69, 65,
72, 65, 78,
198, 73, 83,
198, 79, 79,
198, 69, 69,
70, 76, 79,
215, 65, 76,
86, 65, 84,
68, 73, 65,
199, 65, 85,
68, 69, 66,
32, 32, 32,
32, 32, 32,
	0,
};
const uint8_t automap[] = {
77, 65, 80,
	0,
80, 73, 67,
	1,
84, 65, 80,
	3,
80, 73, 67,
	7,
80, 73, 67,
	8,
75, 69, 89,
	11,
75, 69, 89,
	12,
77, 79, 80,
	16,
87, 73, 78,
	19,
70, 73, 76,
	22,
80, 65, 73,
	23,
80, 65, 73,
	24,
67, 85, 84,
	26,
83, 85, 73,
	27,
80, 65, 73,
	30,
67, 65, 83,
	37,
69, 78, 86,
	39,
89, 65, 82,
	40,
80, 73, 67,
	41,
83, 65, 66,
	42,
76, 69, 65,
	49,
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
