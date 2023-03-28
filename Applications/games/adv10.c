#define NUM_OBJ 59
#define WORDSIZE 4
#define GAME_MAGIC 123
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
const uint8_t maxcar = 6;
const uint8_t treasure = 0;
const uint8_t treasures = 0;
const uint8_t lastloc = 34;
const uint8_t startloc = 1;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x42\x65\x61\x63\x68\x20\x62\x79\x20\x6F\x63\x65\x61\x6E",
 { 0, 7, 4, 2, 0, 5 } }, 
		{ 	"\x42\x65\x61\x63\x68\x20\x62\x79\x20\x6F\x63\x65\x61\x6E\x2C\x20\x61\x20\x63\x6C\x69\x66\x66\x20\x74\x6F\x77\x65\x72\x73\x20\x6F\x76\x65\x72\x0A\x77\x65\x73\x74\x20\x65\x6E\x64",
 { 0, 0, 1, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x68\x65\x20\x65\x64\x67\x65\x20\x6F\x66\x20\x61\x20\x68\x6F\x74\x20\x72\x6F\x63\x6B\x79\x20\x63\x6C\x69\x66\x66\x0A\x6F\x75\x74\x73\x69\x64\x65\x20\x74\x68\x65\x20\x56\x6F\x6C\x63\x61\x6E\x6F",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x42\x65\x61\x63\x68\x20\x62\x79\x20\x6F\x63\x65\x61\x6E",
 { 0, 0, 0, 1, 0, 0 } }, 
		{ 	"\x73\x68\x61\x6C\x6C\x6F\x77\x20\x74\x69\x64\x65\x70\x6F\x6F\x6C",
 { 0, 1, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x68\x65\x20\x72\x69\x6D\x20\x6F\x66\x20\x61\x6E\x20\x65\x78\x74\x69\x6E\x63\x74\x20\x76\x6F\x6C\x63\x61\x6E\x6F",
 { 0, 0, 0, 0, 0, 7 } }, 
		{ 	"\x6C\x61\x72\x67\x65\x20\x70\x6C\x61\x69\x6E",
 { 1, 0, 0, 0, 0, 0 } }, 
		{ 	"\x76\x6F\x6C\x63\x61\x6E\x6F",
 { 0, 0, 0, 0, 6, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 22, 17, 20, 21, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x6F\x70\x20\x6F\x66\x20\x74\x68\x65\x20\x68\x65\x61\x64\x2C\x20\x6F\x66\x66\x73\x68\x6F\x72\x65\x20\x49\x20\x73\x65\x65\x20\x61\x6E\x20\x61\x74\x6F\x6C\x6C",
 { 0, 0, 0, 0, 0, 1 } }, 
		{ 	"\x64\x65\x65\x70\x20\x63\x61\x76\x65",
 { 0, 0, 8, 0, 0, 0 } }, 
		{ 	"\x6C\x61\x6B\x65\x2E\x20\x54\x68\x65\x72\x65\x27\x73\x20\x73\x68\x6F\x72\x65\x20\x74\x6F\x20\x4E\x6F\x72\x74\x68\x2C\x0A\x53\x6F\x75\x74\x68\x20\x26\x20\x45\x61\x73\x74",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x61\x62\x6F\x76\x65\x20\x74\x68\x65\x20\x6C\x61\x6B\x65\x20\x62\x6F\x74\x74\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x72\x61\x66\x74",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x61\x74\x6F\x6C\x6C",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x62\x65\x61\x63\x68\x20\x62\x79\x20\x6F\x63\x65\x61\x6E",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x61\x6E\x63\x69\x65\x6E\x74\x20\x63\x61\x76\x65",
 { 0, 16, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x77\x65\x73\x74\x20\x6F\x66\x20\x6C\x61\x6B\x65\x20\x6F\x6E\x20\x61\x20\x73\x6D\x61\x6C\x6C\x20\x73\x65\x63\x6C\x75\x64\x65\x64\x20\x6C\x65\x64\x67\x65\x0A\x6F\x6E\x20\x76\x6F\x6C\x63\x61\x6E\x6F\x27\x73\x20\x77\x61\x6C\x6C",
 { 0, 0, 12, 0, 0, 0 } }, 
		{ 	"\x74\x6F\x70\x20\x6F\x66\x20\x61\x20\x70\x61\x6C\x6D\x20\x74\x72\x65\x65",
 { 0, 0, 0, 0, 0, 4 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 9, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 9, 0, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x6C\x69\x6E\x65\x64\x20\x74\x75\x6E\x6E\x65\x6C",
 { 0, 9, 23, 29, 0, 0 } }, 
		{ 	"\x64\x69\x6D\x6C\x79\x20\x6C\x69\x74\x20\x72\x6F\x63\x6B\x20\x72\x6F\x6F\x6D",
 { 0, 24, 0, 22, 0, 0 } }, 
		{ 	"\x72\x6F\x63\x6B\x20\x72\x6F\x6F\x6D",
 { 23, 0, 0, 0, 0, 0 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 25, 27, 26, 27, 0, 0 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 25, 26, 25, 27, 0, 0 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 28, 26, 27, 25, 0, 0 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 28, 26, 25, 33, 0, 0 } }, 
		{ 	"\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 22, 0, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x6D\x61\x6C\x6C\x20\x6D\x65\x74\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x68\x65\x20\x65\x64\x67\x65\x20\x6F\x66\x20\x61\x20\x73\x61\x6E\x64\x79\x20\x63\x6C\x69\x66\x66\x20\x6F\x75\x74\x73\x69\x64\x65\x0A\x74\x68\x65\x20\x76\x6F\x6C\x63\x61\x6E\x6F",
 { 0, 0, 18, 0, 0, 0 } }, 
		{ 	"\x6D\x61\x7A\x65\x20\x6F\x66\x20\x63\x61\x76\x65\x73",
 { 28, 27, 26, 25, 0, 17 } }, 
		{ 	"\x6C\x6F\x74\x20\x6F\x66\x20\x74\x72\x6F\x75\x62\x6C\x65",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	1,
	0,
	4,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	11,
	8,
	0,
	11,
	0,
	3,
	5,
	255,
	11,
	1,
	1,
	0,
	0,
	0,
	0,
	7,
	0,
	0,
	11,
	0,
	8,
	0,
	0,
	0,
	0,
	0,
	16,
	17,
	0,
	0,
	19,
	17,
	0,
	0,
	20,
	21,
	11,
	28,
	28,
	29,
	29,
	18,
	17,
	0,
	0,
	24,
	0,
};


const char *objtext[] = {
	"",
	"\x53\x61\x6E\x64",
	"\x2E",
	"\x50\x61\x6C\x6D\x20\x74\x72\x65\x65\x73",
	"\x53\x75\x72\x6C\x79\x20\x70\x69\x72\x61\x74\x65\x20\x77\x61\x69\x74\x69\x6E\x67\x20\x66\x6F\x72\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67",
	"\x50\x69\x72\x61\x74\x65\x20\x73\x68\x69\x70",
	"\x42\x6F\x74\x74\x6C\x65\x20\x6F\x66\x20\x52\x75\x6D",
	"\x45\x6D\x70\x74\x79\x20\x62\x6F\x74\x74\x6C\x65",
	"\x42\x6F\x74\x74\x6C\x65\x20\x6F\x66\x20\x73\x65\x61\x77\x61\x74\x65\x72",
	"\x47\x6C\x6F\x77\x69\x6E\x67\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x62\x6C\x6F\x63\x6B",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x73\x61\x6C\x74",
	"\x50\x75\x64\x64\x6C\x65",
	"\x46\x69\x73\x68\x20\x62\x6F\x6E\x65\x73",
	"\x4C\x61\x6B\x65",
	"\x42\x6F\x74\x74\x6C\x65\x20\x66\x72\x65\x73\x68\x77\x61\x74\x65\x72",
	"\x4E\x61\x74\x75\x72\x61\x6C\x20\x73\x74\x6F\x6E\x65\x20\x62\x61\x73\x69\x6E",
	"\x77\x69\x74\x68\x20\x72\x75\x6D\x20\x69\x6E\x20\x69\x74",
	"\x43\x72\x65\x76\x69\x63\x65",
	"\x57\x61\x74\x65\x72",
	"\x57\x61\x74\x63\x68",
	"\x42\x65\x61\x72",
	"\x4C\x61\x72\x67\x65\x20\x73\x74\x6F\x6E\x65\x20\x68\x65\x61\x64",
	"\x45\x64\x67\x65\x20\x6F\x66\x20\x69\x6D\x70\x65\x6E\x65\x74\x72\x61\x62\x6C\x65\x20\x6A\x75\x6E\x67\x6C\x65",
	"\x50\x69\x65\x63\x65\x73\x20\x6F\x66\x20\x70\x6C\x69\x61\x62\x6C\x65\x20\x76\x69\x6E\x65\x73",
	"\x4C\x61\x72\x67\x65\x20\x6B\x6E\x69\x66\x65",
	"\x50\x61\x6C\x6D\x20\x6C\x6F\x67",
	"\x53\x6D\x61\x6C\x6C\x20\x70\x6C\x61\x73\x74\x69\x63\x20\x62\x6C\x6F\x63\x6B",
	"\x45\x78\x74\x69\x6E\x63\x74\x20\x76\x6F\x6C\x63\x61\x6E\x6F",
	"\x50\x61\x6C\x6D\x20\x6C\x6F\x67",
	"\x52\x61\x66\x74\x20\x6F\x66\x20\x76\x69\x6E\x65\x73\x20\x26\x20\x6C\x6F\x67\x73",
	"\x43\x72\x65\x76\x69\x63\x65",
	"\x57\x49\x4E\x44\x2D\x2D\x2D\x2D\x2D\x3E",
	"\x43\x61\x76\x65",
	"\x53\x77\x65\x61\x74",
	"\x4F\x63\x65\x61\x6E",
	"\x42\x65\x61\x63\x68",
	"\x54\x69\x64\x65\x70\x6F\x6F\x6C",
	"\x41\x74\x6F\x6C\x6C",
	"\x43\x61\x76\x65\x20\x69\x6E\x20\x63\x6C\x69\x66\x66",
	"\x50\x72\x69\x6D\x69\x74\x76\x65\x20\x63\x61\x76\x65\x20\x64\x72\x61\x77\x69\x6E\x67\x73",
	"\x4E\x6F\x74\x65",
	"\x42\x61\x6E\x64\x61\x6E\x6E\x61",
	"\x43\x6F\x63\x6F\x6E\x75\x74\x73",
	"\x53\x74\x61\x6C\x61\x63\x74\x69\x74\x65\x20\x69\x6E\x20\x72\x6F\x6F\x66",
	"\x4E\x65\x61\x6E\x64\x65\x72\x74\x68\x61\x6C",
	"\x43\x72\x65\x76\x69\x63\x65\x20\x69\x6E\x20\x72\x6F\x63\x6B",
	"\x53\x65\x61\x6C\x65\x64\x20\x64\x69\x73\x70\x6C\x61\x79\x20\x63\x61\x73\x65",
	"\x53\x65\x61\x6C\x65\x64\x20\x64\x69\x73\x70\x6C\x61\x79\x20\x63\x61\x73\x65",
	"\x44\x61\x72\x6B\x20\x6F\x70\x65\x6E\x69\x6E\x67",
	"\x4F\x70\x65\x6E\x69\x6E\x67",
	"\x50\x69\x6C\x65\x20\x62\x61\x74\x20\x67\x75\x61\x6E\x6F",
	"\x46\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64",
	"\x43\x6F\x6E\x74\x72\x6F\x6C\x20\x63\x6F\x6E\x73\x6F\x6C\x65",
	"\x4E\x61\x72\x72\x6F\x77\x20\x63\x72\x65\x76\x69\x63\x65",
	"\x44\x61\x72\x6B\x20\x6F\x70\x65\x6E\x69\x6E\x67\x20\x69\x6E\x20\x72\x6F\x6F\x66",
	"\x32\x20\x63\x65\x6E\x74\x69\x6D\x65\x74\x65\x72\x20\x70\x69\x65\x63\x65\x20\x6F\x66\x20\x73\x68\x69\x6E\x79\x20\x77\x69\x72\x65",
	"\x4F\x70\x65\x6E\x20\x64\x69\x73\x70\x6C\x61\x79\x20\x63\x61\x73\x65",
	"\x41\x6C\x69\x65\x6E\x20\x6D\x61\x63\x68\x69\x6E\x65\x72\x79",
	"\x43\x6F\x63\x6F\x6E\x75\x74\x20\x6D\x65\x61\x74",
};
const char *msgptr[] = {
	"",
	"\x4F\x4B",
	"\x53\x6F\x61\x6B\x73\x20\x69\x6E\x74\x6F\x20\x67\x72\x6F\x75\x6E\x64\x2E",
	"\x45\x6D\x70\x74\x79\x3F",
	"\x47\x65\x74\x20\x6C\x69\x71\x75\x69\x64",
	"\x47\x65\x74\x74\x69\x6E\x67\x20\x64\x61\x72\x6B",
	"\x53\x75\x6E\x73\x65\x74",
	"\x49\x20\x68\x65\x61\x72\x20\x63\x61\x6E\x6E\x6F\x6E\x20\x6F\x66\x66\x73\x68\x6F\x72\x65",
	"\x4D\x79\x20\x62\x6F\x6E\x65\x73\x20\x61\x63\x68\x65",
	"\x48\x75\x72\x72\x69\x63\x61\x6E\x65\x20\x41\x6C\x65\x78\x69\x73\x20\x68\x69\x74\x73\x20\x69\x73\x6C\x61\x6E\x64",
	"\x53\x74\x6F\x72\x6D\x20\x70\x61\x73\x73\x65\x64",
	"\x73\x74\x6F\x72\x6D\x20\x6C\x69\x66\x74\x73\x20\x6D\x65\x20\x6F\x75\x74\x20\x74\x6F\x20\x73\x65\x61",
	"\x49\x27\x76\x65\x20\x62\x65\x65\x6E\x20\x68\x65\x72\x65\x20",
	"\x6D\x6F\x76\x65\x73",
	"\x42\x65\x61\x72\x20\x73\x6D\x65\x6C\x6C\x73\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x6F\x6E\x20\x6D\x65\x2C\x20\x6C\x69\x63\x6B\x73\x20\x6D\x65\x2C",
	"\x49\x27\x6D\x20\x61\x74\x74\x61\x63\x6B\x65\x64\x20\x62\x79\x20\x77\x69\x6C\x64\x20\x61\x6E\x69\x6D\x61\x6C",
	"\x62\x65\x61\x72\x20\x65\x61\x74\x73",
	"\x73\x61\x6C\x74",
	"\x6D\x65",
	"\x53\x6F\x72\x72\x79",
	"\x4E\x6F\x74\x20\x74\x69\x6C\x6C",
	"\x49\x20\x63\x61\x6E",
	"\x67\x6F\x20\x74\x68\x65\x72\x65",
	"\x49\x20\x73\x65\x65",
	"\x61\x20\x62\x65\x61\x72",
	"\x61\x73\x68\x20\x69\x73\x20\x6C\x6F\x6F\x73\x65\x2C\x20\x49\x20\x73\x6C\x69\x64\x65\x20\x64\x6F\x77\x6E",
	"\x49\x20\x73\x68\x61\x77\x64\x61\x20\x70\x69\x63\x6B\x65\x64\x20\x61\x6E\x6F\x74\x68\x65\x72\x20\x70\x72\x6F\x66\x66\x65\x73\x69\x6F\x6E\x21",
	"\x54\x72\x79",
	"\x46\x61\x72\x20\x62\x65\x6C\x6F\x77",
	"\x61\x20\x63\x61\x76\x65",
	"\x61\x74\x20\x73\x65\x61\x20\x6C\x65\x76\x65\x6C",
	"\x6E\x6F\x74\x68\x69\x6E\x67",
	"\x73\x70\x65\x63\x69\x61\x6C",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x20\x31\x30\x3A\x20\x22\x53\x41\x56\x41\x47\x45\x20\x49\x53\x4C\x41\x4E\x44\x2C\x20\x50\x61\x72\x74\x20\x49\x22\x0A\x62\x79\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x2C\x20\x64\x65\x64\x69\x63\x61\x74\x65\x64\x3A\x20\x44\x65\x6E\x6E\x69\x73\x20\x42\x72\x65\x6E\x74\x2E\x0A",
	"\x73\x68\x61\x72\x6B\x73",
	"\x74\x72\x65\x65\x73\x2C\x20\x77\x69\x6C\x64\x20\x61\x6E\x69\x6D\x61\x6C\x73\x2C",
	"\x48\x6F\x77\x3F",
	"\x20\x64\x6F\x65\x73\x6E\x27\x74\x20\x77\x6F\x72\x6B",
	"\x49\x20\x64\x72\x6F\x77\x6E",
	"\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67",
	"\x74\x6F\x6F\x20\x68\x65\x61\x76\x79\x20\x6F\x72\x20\x61\x77\x6B\x77\x61\x72\x64",
	"\x49\x20\x62\x72\x65\x61\x74\x68\x65\x64\x21",
	"\x76\x69\x6E\x65\x73",
	"\x43\x52\x41\x53\x48\x21",
	"\x49\x20\x66\x65\x65\x6C",
	"\x41\x72\x67\x68\x21",
	"\x54\x68\x65\x79\x27\x72\x65\x20\x61\x74\x74\x61\x63\x68\x65\x64\x20\x74\x6F",
	"\x68\x61\x72\x64\x20\x77\x6F\x72\x6B",
	"\x6D\x61\x6B\x65\x73\x20\x6D\x65\x20\x6E\x65\x72\x76\x6F\x75\x73",
	"\x72\x65\x73\x65\x6D\x62\x6C\x65\x73",
	"\x57\x69\x65\x72\x64\x21",
	"\x48\x61\x72\x64\x20\x74\x6F\x20\x73\x74\x65\x65\x72\x20\x74\x68\x69\x73\x20\x74\x68\x69\x6E\x67\x21",
	"\x49\x74\x73\x20\x63\x6F\x6D\x69\x6E\x67\x20\x61\x70\x61\x72\x74",
	"\x3F",
	"\x6C\x65\x64\x67\x65\x20\x69\x6E\x20\x74\x68\x65\x20\x77\x61\x6C\x6C\x20\x6F\x66\x20\x74\x68\x65\x20\x76\x6F\x6C\x63\x61\x6E\x6F",
	"\x6C\x6F\x6F\x6B\x20\x73\x69\x63\x6B\x6C\x79",
	"\x4E\x6F\x74\x20\x73\x61\x66\x65",
	"\x49\x27\x6D\x20\x74\x6F\x6F\x20\x62\x69\x67",
	"\x4D\x6F\x72\x6E\x69\x6E\x67\x21",
	"\x0A\x43\x6F\x6E\x67\x72\x61\x74\x73\x21",
	"\x62\x65\x61\x72\x20\x70\x61\x77\x73\x20\x67\x72\x6F\x75\x6E\x64\x20\x26\x20\x77\x68\x69\x6E\x65\x73",
	"\x55\x46\x4F\x20\x6C\x61\x6E\x64\x69\x6E\x67\x20\x61\x6D\x6F\x6E\x67\x20\x64\x69\x6E\x6F\x73\x61\x75\x72\x73\x20\x26\x20\x70\x69\x63\x74\x75\x72\x65\x20\x6F\x66\x20\x6D\x65\x21",
	"\x61\x73\x20\x70\x69\x72\x61\x74\x65\x20\x74\x61\x6B\x65\x73\x20\x69\x74\x20\x68\x69\x73\x20\x62\x61\x6E\x64\x61\x6E\x6E\x61\x20\x66\x61\x6C\x6C\x73\x20\x6F\x66\x66\x0A\x49\x20\x73\x65\x65\x20\x68\x65\x20\x68\x61\x73\x20\x61\x6E\x74\x65\x6E\x6E\x61\x73\x0A\x48\x65\x20\x64\x72\x6F\x70\x73\x20\x61\x20\x6E\x6F\x74\x65\x2C\x20\x6C\x65\x61\x76\x69\x6E\x67\x20\x63\x68\x6F\x72\x74\x6C\x69\x6E\x67",
	"\x4E\x6F\x74\x65\x20\x73\x61\x79\x73\x3A\x0A\x22\x57\x68\x61\x74\x20\x77\x61\x73\x2C\x20\x6D\x75\x73\x74\x20\x62\x65\x2C\x0A\x73\x6F\x20\x73\x65\x6E\x64\x20\x74\x68\x69\x73\x20\x6E\x6F\x74\x65\x20\x77\x65\x0A\x74\x6F\x20\x74\x65\x6C\x6C\x20\x79\x6F\x75\x20\x61\x20\x77\x6F\x72\x64\x27\x73\x20\x66\x72\x65\x65\x21\x22",
	"\x73\x61\x6C\x74\x79",
	"\x49\x27\x6D\x20\x61\x20\x62\x6F\x74\x74\x6C\x65\x20\x62\x61\x62\x79",
	"\x74\x6F\x20\x6D\x6F\x76\x65\x20\x69\x6E\x20\x68\x75\x72\x72\x69\x63\x61\x6E\x65",
	"\x49\x27\x6D\x20\x74\x72\x65\x61\x64\x69\x6E\x67\x20\x77\x61\x74\x65\x72",
	"\x63\x61\x76\x65\x6D\x61\x6E\x20\x73\x61\x79\x73",
	"\x49\x20\x68\x69\x74\x20\x73\x74\x6F\x6E\x65\x20\x77\x69\x74\x68",
	"\x69\x74",
	"\x77\x6F\x6E\x27\x74\x20\x62\x75\x64\x67\x65",
	"\x63\x61\x76\x65\x6D\x61\x6E",
	"\x69\x74\x73\x20\x68\x69\x6E\x67\x65\x64\x20\x61\x74\x20\x74\x6F\x70\x21",
	"\x54\x59\x72\x61\x6E\x6F\x73\x61\x75\x72\x75\x73",
	"\x72\x65\x70\x6C\x69\x63\x61\x20\x66\x75\x6C\x6C\x20\x73\x69\x7A\x65\x64\x21\x20\x54\x68\x65\x72\x65\x27\x73\x20\x62\x75\x74\x74\x6F\x6E\x20\x61\x74\x20\x62\x6F\x74\x74\x6F\x6D",
	"\x67\x6C\x6F\x77\x73",
	"\x62\x75\x74\x74\x6F\x6E\x2C\x20\x64\x69\x61\x6C\x73\x2C\x20\x6C\x65\x76\x65\x72\x73",
	"\x62\x65\x61\x63\x68\x20\x33\x20\x6D\x65\x74\x65\x72\x73\x20\x62\x65\x6C\x6F\x77",
	"\x44\x6F\x6E\x27\x74\x20\x62\x65\x20\x22\x59\x45\x53\x22\x20\x6D\x61\x6E\x2E\x20\x44\x6F\x20\x69\x74\x20\x79\x6F\x75\x72\x73\x65\x6C\x66\x21",
	"\x61\x20\x68\x6F\x6C\x65",
	"\x59\x55\x43\x4B\x21",
	"\x4D\x65\x74\x61\x6C\x6C\x69\x63\x20\x76\x6F\x69\x63\x65\x20\x77\x68\x69\x73\x70\x65\x72\x73\x20\x69\x6E\x20\x6D\x79\x20\x6D\x69\x6E\x64\x3A",
	"\x22\x56\x4F\x43\x41\x4C\x49\x5A\x45\x20\x50\x41\x53\x53\x57\x4F\x52\x44\x20\x50\x4C\x45\x41\x53\x45\x22",
	"\x22\x52\x45\x41\x44\x59\x22",
	"\x70\x75\x73\x68\x65\x73\x20\x62\x75\x74\x74\x6F\x6E",
	"\x54\x68\x65\x72\x65\x27\x73\x20\x62\x6C\x69\x6E\x64\x69\x6E\x67\x20\x66\x6C\x61\x73\x68\x20\x66\x72\x6F\x6D\x20\x46\x6F\x72\x63\x65\x20\x46\x69\x65\x6C\x64",
	"\x53\x6D\x69\x6C\x65\x73\x20\x26\x20\x70\x6F\x69\x6E\x74\x73\x20\x61\x74\x20\x6D\x65\x20\x74\x68\x65\x6E\x20\x74\x68\x65\x20\x66\x6F\x72\x63\x65\x20\x66\x69\x65\x6C\x64",
	"\x63\x61\x73\x65\x20\x6F\x70\x65\x6E\x73\x2E\x20\x49\x74\x73\x20\x61\x6C\x69\x76\x65\x21",
	"\x49\x27\x6D\x20\x64\x69\x6E\x6F\x73\x61\x75\x72\x20\x73\x6E\x61\x63\x6B\x21",
	"\x6E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x73",
	"\x69\x74\x73\x20\x61\x6E\x20\x61\x6C\x69\x65\x6E\x20\x73\x63\x72\x69\x70\x74\x21",
	"\x50\x69\x65\x63\x65\x20\x77\x69\x72\x65\x20\x6D\x69\x73\x73\x69\x6E\x67\x20\x66\x72\x6F\x6D\x20\x69\x74\x21",
	"\x74\x72\x69\x70\x70\x65\x64\x20\x26\x20\x66\x65\x6C\x6C\x21",
	"\x49\x66\x20\x79\x6F\x75\x20\x6C\x69\x6B\x65\x20\x74\x6F\x20\x6B\x69\x6C\x6C\x20\x6D\x6F\x6E\x73\x74\x65\x72\x73\x20\x70\x6C\x61\x79\x20\x22\x4D\x41\x43\x45\x53\x20\x26\x20\x4D\x41\x47\x49\x43\x22\x21",
	"\x49\x20\x64\x72\x6F\x70\x70\x65\x64\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67",
	"",
	"",
	"",
	"\x47\x6C\x6F\x77\x69\x6E\x67\x20\x73\x69\x67\x6E\x20\x61\x70\x70\x65\x61\x72\x73\x3A\x20\x22\x53\x41\x56\x45\x20\x54\x48\x49\x53\x20\x50\x41\x53\x53\x57\x4F\x52\x44\x20\x46\x4F\x52\x20\x41\x44\x56\x45\x4E\x54\x55\x52\x45\x20\x31\x31\x3A\x22",
};


const uint8_t status[] = {
	164, 
	0, 1, 
	82, 
	141, 10, 
	13, 11, 0, 11, 9, 2, 
	59, 73, 
	209, 
	8, 1, 0, 10, 0, 3, 0, 1, 
	62, 60, 
	180, 
	8, 4, 5, 31, 9, 17, 9, 18, 7, 13, 
	73, 
	196, 
	0, 31, 
	53, 
	179, 
	9, 3, 0, 3, 0, 250, 0, 1, 
	58, 73, 79, 81, 
	211, 
	0, 70, 0, 2, 32, 44, 0, 3, 
	79, 81, 79, 81, 
	194, 
	79, 77, 33, 
	203, 
	0, 30, 0, 1, 
	54, 80, 54, 76, 
	168, 
	7, 12, 0, 14, 
	60, 
	166, 
	0, 1, 
	81, 77, 73, 
	208, 
	15, 30, 9, 2, 9, 17, 9, 18, 
	5, 
	215, 
	15, 0, 0, 2, 9, 2, 9, 17, 9, 18, 
	58, 6, 56, 76, 
	202, 
	0, 1, 0, 3, 
	81, 81, 77, 
	214, 
	15, 0, 0, 5, 0, 15, 9, 6, 0, 6, 
	62, 7, 58, 
	202, 
	0, 3, 0, 2, 
	81, 81, 77, 
	212, 
	15, 15, 16, 0, 9, 4, 7, 13, 9, 19, 
	8, 
	214, 
	9, 4, 15, 0, 0, 4, 0, 85, 9, 19, 
	9, 58, 79, 
	214, 
	8, 4, 15, 50, 0, 4, 0, 19, 8, 5, 
	60, 58, 10, 
	211, 
	8, 4, 15, 0, 2, 31, 7, 3, 
	106, 116, 11, 61, 
	196, 
	0, 2, 
	81, 
	168, 
	1, 25, 0, 14, 
	60, 
	168, 
	1, 28, 0, 14, 
	60, 
	176, 
	13, 5, 14, 4, 0, 4, 0, 15, 
	62, 
	179, 
	2, 10, 2, 20, 0, 10, 0, 5, 
	59, 58, 16, 17, 
	143, 30, 
	2, 20, 9, 5, 1, 33, 
	14, 16, 18, 61, 
	143, 40, 
	2, 52, 8, 22, 2, 44, 
	122, 135, 137, 136, 
	141, 40, 
	4, 8, 3, 10, 0, 20, 
	53, 14, 
	140, 10, 
	0, 20, 0, 8, 9, 4, 
	62, 
	144, 40, 
	8, 18, 7, 17, 0, 20, 5, 20, 
	53, 
	140, 35, 
	0, 20, 0, 11, 8, 4, 
	62, 
	170, 
	8, 7, 0, 4, 
	81, 77, 73, 
	196, 
	15, 2, 
	45, 
	201, 
	15, 0, 0, 7, 
	60, 41, 
	196, 
	0, 4, 
	81, 
	174, 
	1, 28, 1, 25, 0, 28, 
	19, 40, 53, 
	149, 25, 
	8, 4, 0, 25, 0, 4, 14, 25, 14, 29, 
	62, 43, 
	177, 
	4, 30, 0, 23, 8, 22, 13, 44, 
	58, 122, 
	177, 
	4, 31, 0, 23, 8, 22, 13, 44, 
	58, 122, 
	167, 
	8, 23, 
	135, 136, 149, 73, 
	203, 
	12, 41, 32, 218, 
	79, 78, 109, 63, 
	203, 
	3, 41, 0, 123, 
	79, 78, 109, 63, 
	170, 
	8, 10, 0, 5, 
	81, 77, 73, 
	196, 
	15, 10, 
	102, 
	199, 
	15, 0, 
	43, 34, 15, 61, 
	196, 
	0, 5, 
	81, 
	142, 30, 
	2, 20, 0, 33, 9, 5, 
	24, 48, 52, 
	142, 15, 
	8, 10, 0, 10, 0, 37, 
	60, 53, 76, 
	142, 40, 
	8, 10, 0, 10, 0, 36, 
	60, 53, 76, 
	142, 30, 
	8, 10, 0, 10, 0, 35, 
	60, 53, 76, 
	174, 
	8, 10, 0, 10, 0, 34, 
	60, 53, 76, 
	149, 2, 
	8, 4, 0, 28, 0, 4, 14, 28, 14, 29, 
	62, 43, 
	183, 
	2, 6, 2, 4, 0, 40, 0, 6, 0, 41, 
	72, 73, 53, 112, 
	201, 
	0, 4, 0, 5, 
	59, 59, 
	147, 5, 
	8, 4, 6, 28, 2, 31, 6, 25, 
	106, 116, 11, 61, 
	176, 
	4, 12, 10, 0, 6, 25, 6, 28, 
	73, 
	203, 
	8, 14, 0, 13, 
	39, 40, 54, 76, 
	203, 
	9, 14, 0, 14, 
	58, 117, 39, 40, 
	172, 
	9, 4, 13, 31, 0, 31, 
	59, 
	165, 
	4, 24, 
	56, 76, 
	165, 
	4, 23, 
	57, 76, 
	165, 
	4, 9, 
	57, 76, 
	178, 
	4, 17, 0, 17, 0, 18, 9, 2, 
	60, 60, 57, 
	168, 
	4, 8, 0, 18, 
	60, 
	179, 
	4, 17, 0, 17, 0, 18, 8, 2, 
	60, 60, 56, 76, 
	181, 
	13, 44, 5, 44, 0, 44, 7, 30, 7, 31, 
	53, 76, 
	143, 40, 
	2, 52, 2, 44, 9, 22, 
	122, 135, 73, 132, 
	195, 
	133, 118, 45, 140, 
	169, 
	8, 25, 14, 47, 
	139, 61, 
	176, 
	8, 25, 14, 46, 14, 44, 0, 44, 
	53, 
	173, 
	8, 24, 0, 25, 0, 24, 
	58, 60, 
	169, 
	4, 33, 12, 9, 
	143, 61, 
	172, 
	4, 12, 11, 0, 0, 14, 
	60, 
	176, 
	4, 12, 2, 19, 0, 19, 0, 13, 
	62, 
	176, 
	4, 12, 2, 24, 0, 24, 0, 13, 
	62, 
	176, 
	4, 12, 2, 6, 0, 6, 0, 13, 
	62, 
	172, 
	4, 12, 3, 10, 0, 10, 
	59, 
	175, 
	13, 9, 0, 7, 0, 26, 
	81, 77, 59, 73, 
	205, 
	15, 0, 0, 9, 0, 26, 
	72, 76, 
	196, 
	0, 7, 
	81, 
	181, 
	14, 29, 8, 19, 14, 25, 0, 25, 0, 4, 
	62, 43, 
	181, 
	14, 29, 8, 19, 14, 28, 0, 28, 0, 4, 
	62, 43, 
	171, 
	4, 6, 8, 12, 
	88, 88, 88, 73, 
	199, 
	0, 8, 
	19, 25, 54, 76, 
	211, 
	1, 28, 0, 28, 0, 7, 8, 28, 
	47, 145, 62, 76, 
	211, 
	1, 25, 0, 25, 0, 7, 8, 28, 
	47, 145, 62, 76, 
	168, 
	4, 7, 0, 28, 
	60, 
	168, 
	4, 8, 0, 28, 
	58, 
	176, 
	4, 12, 2, 14, 0, 14, 0, 13, 
	62, 
	169, 
	4, 13, 9, 7, 
	38, 61, 
	176, 
	4, 8, 8, 4, 0, 31, 5, 31, 
	53, 
	175, 
	4, 34, 0, 20, 0, 44, 
	59, 59, 76, 63, 
};
const uint8_t actions[] = {
	7, 30, 22, 
	4, 3, 
	28, 23, 29, 30, 
	9, 1, 36, 
	8, 4, 2, 29, 
	11, 61, 
	7, 30, 6, 
	4, 3, 
	28, 23, 29, 30, 
	10, 1, 19, 
	3, 21, 0, 10, 
	1, 54, 76, 
	2, 30, 25, 
	1, 23, 34, 
	7, 30, 26, 
	2, 22, 
	1, 23, 35, 73, 
	196, 
	14, 23, 
	42, 
	17, 38, 27, 
	2, 22, 14, 23, 0, 23, 3, 24, 
	1, 53, 
	0, 44, 0, 
	66, 
	0, 10, 28, 
	66, 
	14, 1, 34, 
	4, 12, 0, 18, 9, 14, 
	1, 54, 76, 
	14, 46, 4, 
	4, 12, 9, 14, 0, 18, 
	1, 54, 76, 
	18, 46, 6, 
	4, 12, 0, 13, 6, 28, 6, 25, 
	1, 54, 76, 
	15, 10, 31, 
	7, 13, 0, 7, 0, 4, 
	1, 58, 81, 73, 
	201, 
	0, 7, 0, 4, 
	79, 81, 
	11, 54, 58, 
	2, 20, 9, 5, 
	1, 14, 15, 61, 
	8, 54, 58, 
	2, 20, 8, 5, 
	1, 
	6, 10, 33, 
	4, 8, 
	19, 25, 26, 
	6, 46, 6, 
	4, 12, 
	103, 19, 66, 
	21, 51, 41, 
	3, 42, 3, 24, 8, 27, 0, 42, 0, 58, 
	72, 1, 
	10, 46, 5, 
	4, 13, 0, 12, 
	1, 54, 76, 
	15, 46, 3, 
	4, 13, 0, 26, 14, 26, 
	1, 23, 39, 53, 
	15, 46, 1, 
	4, 13, 0, 24, 14, 24, 
	1, 23, 39, 53, 
	9, 81, 68, 
	3, 42, 0, 27, 
	36, 58, 
	4, 46, 0, 
	4, 13, 
	1, 
	15, 1, 50, 
	2, 27, 0, 6, 0, 33, 
	47, 54, 76, 52, 
	19, 1, 25, 
	7, 8, 7, 11, 7, 12, 7, 13, 
	1, 15, 34, 61, 
	17, 49, 36, 
	3, 23, 3, 28, 3, 25, 3, 24, 
	1, 73, 
	211, 
	0, 29, 0, 23, 0, 28, 0, 25, 
	53, 59, 59, 59, 
	209, 
	4, 5, 0, 36, 0, 14, 0, 11, 
	62, 58, 
	206, 
	0, 5, 0, 25, 0, 5, 
	81, 79, 81, 
	15, 72, 51, 
	2, 52, 0, 22, 9, 22, 
	85, 132, 134, 58, 
	0, 42, 0, 
	63, 
	15, 57, 0, 
	0, 1, 0, 50, 0, 1, 
	81, 83, 81, 73, 
	211, 
	0, 3, 0, 50, 0, 3, 0, 2, 
	81, 83, 81, 81, 
	206, 
	0, 50, 0, 2, 0, 50, 
	83, 81, 82, 
	18, 51, 29, 
	2, 1, 8, 20, 0, 20, 0, 21, 
	1, 60, 58, 
	15, 1, 38, 
	2, 30, 0, 3, 0, 18, 
	1, 54, 76, 60, 
	15, 1, 39, 
	2, 32, 0, 11, 0, 18, 
	1, 54, 76, 58, 
	14, 1, 36, 
	2, 29, 0, 14, 8, 11, 
	1, 54, 76, 
	9, 10, 27, 
	3, 23, 0, 23, 
	1, 52, 
	6, 10, 27, 
	2, 22, 
	19, 46, 35, 
	7, 30, 19, 
	3, 21, 
	1, 49, 18, 50, 
	14, 79, 0, 
	8, 11, 4, 14, 0, 10, 
	51, 58, 73, 
	211, 
	0, 37, 0, 34, 0, 35, 0, 36, 
	59, 59, 59, 59, 
	202, 
	0, 5, 0, 5, 
	81, 77, 81, 
	0, 79, 0, 
	36, 
	15, 1, 61, 
	2, 35, 0, 16, 0, 29, 
	1, 54, 53, 76, 
	15, 1, 60, 
	2, 37, 0, 15, 0, 29, 
	1, 54, 53, 76, 
	15, 1, 52, 
	2, 36, 0, 5, 0, 29, 
	1, 54, 53, 76, 
	10, 1, 39, 
	2, 38, 0, 17, 
	1, 54, 76, 
	6, 30, 4, 
	4, 12, 
	1, 23, 104, 
	11, 51, 29, 
	4, 32, 0, 20, 
	1, 60, 31, 32, 
	19, 63, 0, 
	4, 18, 0, 2, 0, 1, 0, 170, 
	60, 81, 79, 73, 
	199, 
	0, 1, 
	81, 88, 88, 88, 
	198, 
	8, 4, 
	106, 11, 61, 
	194, 
	57, 108, 76, 
	5, 1, 32, 
	2, 15, 
	19, 107, 
	6, 63, 0, 
	7, 18, 
	106, 15, 61, 
	5, 61, 0, 
	4, 3, 
	1, 61, 
	2, 1, 11, 
	19, 20, 109, 
	9, 45, 37, 
	2, 2, 0, 2, 
	1, 59, 
	23, 51, 29, 
	8, 20, 2, 50, 14, 55, 0, 55, 0, 20, 
	53, 60, 1, 131, 
	10, 61, 0, 
	4, 32, 0, 2, 
	1, 54, 76, 
	0, 18, 9, 
	3, 
	7, 30, 62, 
	2, 39, 
	1, 23, 111, 50, 
	5, 30, 66, 
	3, 40, 
	1, 113, 
	4, 68, 9, 
	3, 6, 
	1, 
	6, 68, 55, 
	3, 8, 
	1, 45, 114, 
	6, 68, 59, 
	3, 33, 
	1, 45, 114, 
	4, 68, 55, 
	3, 14, 
	1, 
	1, 78, 0, 
	1, 71, 
	10, 1, 16, 
	2, 3, 0, 19, 
	1, 54, 76, 
	5, 41, 57, 
	9, 4, 
	1, 71, 
	4, 46, 0, 
	4, 5, 
	1, 
	19, 46, 0, 
	4, 12, 10, 0, 6, 28, 6, 25, 
	39, 40, 38, 61, 
	8, 30, 58, 
	2, 20, 9, 5, 
	105, 
	9, 71, 55, 
	4, 12, 0, 14, 
	60, 1, 
	10, 46, 0, 
	4, 12, 0, 8, 
	1, 54, 76, 
	7, 72, 0, 
	2, 44, 
	1, 85, 118, 45, 
	4, 10, 42, 
	2, 43, 
	36, 
	15, 51, 68, 
	2, 43, 14, 45, 1, 42, 
	119, 120, 120, 73, 
	199, 
	0, 45, 
	13, 23, 39, 53, 
	15, 1, 38, 
	2, 45, 0, 9, 0, 17, 
	1, 54, 76, 58, 
	11, 30, 69, 
	2, 46, 14, 44, 
	1, 23, 122, 125, 
	6, 30, 42, 
	2, 43, 
	50, 23, 123, 
	7, 30, 69, 
	2, 47, 
	1, 23, 124, 125, 
	1, 30, 44, 
	120, 126, 
	6, 30, 74, 
	2, 52, 
	1, 23, 127, 
	23, 30, 37, 
	8, 21, 9, 9, 0, 9, 0, 6, 2, 1, 
	23, 58, 53, 39, 
	19, 51, 68, 
	2, 43, 13, 45, 1, 42, 0, 45, 
	119, 120, 13, 59, 
	22, 51, 20, 
	9, 26, 2, 57, 3, 55, 0, 55, 0, 26, 
	1, 59, 58, 
	15, 51, 29, 
	8, 20, 2, 50, 0, 20, 
	60, 23, 31, 32, 
	0, 81, 0, 
	36, 
	1, 59, 0, 
	19, 144, 
	2, 54, 0, 
	44, 31, 32, 
	11, 51, 0, 
	0, 20, 0, 27, 
	60, 84, 37, 60, 
	11, 1, 36, 
	2, 29, 9, 11, 
	19, 120, 121, 63, 
	5, 58, 0, 
	0, 20, 
	36, 58, 
	2, 43, 0, 
	45, 44, 26, 
	9, 68, 68, 
	3, 58, 0, 58, 
	1, 59, 
	5, 64, 0, 
	0, 7, 
	36, 60, 
	1, 72, 0, 
	1, 85, 
	7, 66, 0, 
	2, 20, 
	1, 85, 15, 61, 
	0, 61, 0, 
	1, 
	11, 1, 72, 
	2, 48, 0, 25, 
	1, 54, 56, 76, 
	11, 1, 72, 
	2, 49, 0, 11, 
	1, 54, 76, 73, 
	197, 
	9, 2, 
	57, 76, 
	18, 1, 38, 
	2, 53, 0, 32, 6, 28, 6, 25, 
	1, 54, 76, 
	6, 30, 22, 
	4, 32, 
	1, 23, 128, 
	6, 30, 6, 
	4, 32, 
	1, 23, 128, 
	0, 1, 22, 
	19, 
	0, 1, 61, 
	36, 
	0, 48, 0, 
	129, 
	5, 10, 73, 
	2, 50, 
	19, 131, 
	10, 25, 54, 
	2, 52, 9, 22, 
	1, 132, 133, 
	22, 25, 54, 
	2, 46, 0, 46, 0, 56, 0, 24, 8, 26, 
	72, 73, 58, 
	196, 
	14, 44, 
	138, 
	18, 25, 54, 
	2, 47, 0, 47, 0, 56, 0, 24, 
	72, 138, 58, 
	22, 25, 54, 
	2, 56, 4, 20, 0, 46, 0, 56, 0, 24, 
	1, 72, 60, 
	22, 25, 54, 
	2, 56, 4, 21, 0, 47, 0, 56, 0, 24, 
	72, 60, 1, 
	9, 25, 54, 
	2, 46, 9, 26, 
	1, 140, 
	1, 68, 0, 
	19, 115, 
	5, 25, 80, 
	2, 52, 
	36, 121, 
	10, 30, 7, 
	2, 1, 8, 21, 
	1, 23, 130, 
	15, 1, 50, 
	4, 6, 0, 8, 0, 12, 
	25, 54, 58, 76, 
	0, 47, 0, 
	3, 
	4, 35, 8, 
	4, 3, 
	73, 
	215, 
	3, 8, 0, 8, 0, 7, 0, 11, 0, 1, 
	1, 72, 53, 58, 
	210, 
	3, 6, 0, 6, 0, 7, 0, 11, 
	1, 72, 53, 
	210, 
	3, 14, 0, 14, 0, 7, 0, 11, 
	1, 72, 53, 
	1, 40, 0, 
	19, 37, 
	11, 1, 75, 
	2, 51, 0, 51, 
	80, 53, 73, 1, 
	215, 
	1, 26, 0, 9, 0, 7, 0, 85, 0, 7, 
	74, 81, 79, 81, 
	9, 1, 37, 
	2, 1, 8, 21, 
	19, 107, 
	6, 30, 79, 
	2, 52, 
	1, 23, 141, 
	8, 30, 81, 
	9, 26, 2, 57, 
	142, 
	8, 76, 81, 
	9, 26, 2, 57, 
	36, 
	11, 25, 54, 
	8, 22, 2, 52, 
	1, 136, 73, 80, 
	202, 
	4, 30, 0, 31, 
	54, 80, 76, 
	202, 
	4, 31, 0, 30, 
	54, 80, 76, 
	23, 38, 27, 
	2, 29, 0, 28, 0, 25, 3, 24, 7, 14, 
	1, 53, 53, 73, 
	196, 
	0, 29, 
	59, 
	9, 38, 68, 
	3, 42, 0, 27, 
	36, 58, 
	11, 1, 49, 
	2, 13, 0, 12, 
	1, 73, 54, 76, 
	205, 
	1, 19, 0, 19, 0, 13, 
	62, 145, 
	200, 
	1, 33, 0, 33, 
	59, 
	22, 35, 8, 
	3, 6, 2, 15, 0, 6, 0, 7, 0, 16, 
	1, 72, 53, 
	15, 35, 8, 
	3, 8, 0, 8, 0, 7, 
	1, 72, 2, 73, 
	196, 
	2, 20, 
	110, 
	14, 35, 8, 
	3, 14, 0, 14, 0, 7, 
	1, 72, 2, 
	14, 35, 8, 
	3, 6, 0, 6, 0, 7, 
	1, 72, 2, 
	17, 10, 55, 
	4, 12, 3, 7, 0, 7, 0, 14, 
	1, 72, 
	0, 18, 55, 
	3, 
	15, 1, 38, 
	2, 17, 0, 11, 0, 18, 
	1, 54, 76, 58, 
	1, 46, 0, 
	22, 103, 
	0, 38, 0, 
	36, 
	0, 25, 0, 
	36, 
	22, 10, 9, 
	3, 7, 2, 16, 0, 16, 0, 7, 0, 6, 
	1, 59, 72, 
	17, 10, 55, 
	3, 7, 2, 13, 0, 14, 0, 7, 
	1, 72, 
	17, 10, 55, 
	3, 7, 2, 18, 0, 8, 0, 7, 
	1, 72, 
	5, 45, 8, 
	3, 7, 
	27, 4, 
	6, 41, 57, 
	8, 4, 
	19, 20, 10, 
	6, 30, 21, 
	3, 19, 
	12, 78, 13, 
	7, 30, 50, 
	4, 6, 
	21, 22, 73, 28, 
	197, 
	18, 20, 
	23, 24, 
	193, 
	23, 29, 
	2, 30, 0, 
	23, 31, 32, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84, 79,
71, 79, 32, 32,
215, 65, 76, 75,
210, 85, 78, 32,
195, 76, 73, 77,
197, 78, 84, 69,
195, 82, 65, 87,
32, 32, 32, 32,
32, 32, 32, 32,
32, 32, 32, 32,
71, 69, 84, 32,
212, 65, 75, 69,
199, 82, 65, 66,
208, 73, 67, 75,
200, 79, 76, 68,
195, 65, 84, 67,
46, 32, 32, 32,
46, 32, 32, 32,
68, 82, 79, 80,
204, 69, 65, 86,
208, 85, 84, 32,
210, 69, 76, 69,
199, 73, 86, 69,
32, 32, 32, 32,
32, 32, 32, 32,
77, 79, 86, 69,
208, 82, 69, 83,
208, 85, 83, 72,
212, 85, 82, 78,
208, 85, 76, 76,
76, 79, 79, 75,
197, 88, 65, 77,
196, 69, 83, 67,
210, 69, 65, 68,
211, 69, 69, 32,
69, 77, 80, 84,
46, 32, 32, 32,
46, 32, 32, 32,
67, 85, 84, 32,
32, 32, 32, 32,
83, 67, 79, 82,
83, 65, 86, 69,
81, 85, 73, 84,
72, 69, 76, 80,
73, 78, 86, 69,
70, 73, 76, 76,
83, 87, 73, 77,
83, 80, 73, 76,
89, 69, 83, 32,
66, 85, 73, 76,
205, 65, 75, 69,
87, 73, 84, 72,
213, 83, 69, 32,
83, 87, 73, 77,
80, 69, 84, 32,
212, 79, 85, 67,
198, 69, 69, 76,
87, 65, 73, 84,
68, 73, 71, 32,
75, 73, 76, 76,
196, 69, 83, 84,
74, 85, 77, 80,
46, 32, 32, 32,
83, 76, 69, 69,
66, 82, 69, 65,
211, 77, 65, 83,
89, 69, 76, 76,
211, 67, 82, 69,
68, 82, 73, 78,
197, 65, 84, 32,
212, 65, 83, 84,
84, 82, 69, 65,
65, 83, 75, 32,
211, 65, 89, 32,
212, 69, 76, 76,
217, 69, 76, 76,
70, 73, 88, 32,
210, 69, 80, 65,
89, 79, 72, 79,
80, 65, 68, 68,
211, 65, 73, 76,
79, 80, 69, 78,
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
83, 65, 78, 68,
66, 79, 84, 84,
82, 85, 77, 32,
83, 65, 76, 84,
83, 72, 73, 80,
80, 73, 82, 65,
83, 84, 79, 82,
200, 85, 82, 82,
65, 82, 71, 72,
84, 82, 69, 69,
208, 65, 76, 77,
76, 79, 71, 32,
72, 69, 65, 68,
87, 73, 82, 69,
87, 65, 84, 67,
67, 76, 73, 70,
197, 68, 71, 69,
66, 65, 84, 32,
79, 67, 69, 65,
74, 85, 78, 71,
86, 73, 78, 69,
73, 78, 86, 69,
72, 65, 78, 68,
70, 69, 69, 84,
66, 82, 69, 65,
66, 65, 83, 73,
79, 85, 84, 32,
76, 69, 68, 71,
66, 79, 78, 69,
82, 65, 70, 84,
72, 79, 76, 69,
67, 82, 69, 86,
67, 65, 86, 69,
65, 83, 72, 32,
75, 78, 73, 70,
83, 84, 65, 76,
78, 69, 65, 78,
77, 69, 84, 65,
84, 85, 78, 78,
80, 73, 82, 65,
65, 78, 73, 77,
46, 32, 32, 32,
76, 65, 75, 69,
86, 79, 76, 67,
70, 82, 69, 69,
84, 73, 68, 69,
46, 32, 32, 32,
66, 85, 84, 84,
87, 65, 84, 69,
72, 79, 76, 32,
71, 65, 77, 69,
66, 69, 65, 82,
83, 87, 69, 65,
65, 84, 79, 76,
66, 69, 65, 67,
68, 82, 65, 87,
65, 82, 79, 85,
83, 72, 65, 82,
80, 76, 65, 73,
78, 79, 84, 69,
66, 65, 78, 68,
67, 79, 67, 79,
67, 65, 83, 69,
196, 73, 83, 80,
66, 76, 79, 67,
79, 80, 69, 78,
71, 85, 65, 78,
67, 79, 78, 83,
70, 79, 82, 67,
198, 73, 69, 76,
82, 79, 79, 70,
72, 69, 76, 76,
68, 73, 65, 76,
76, 69, 86, 69,
77, 65, 67, 72,
71, 82, 79, 85,
82, 79, 67, 75,
80, 85, 68, 68,
	0,
};
const uint8_t automap[] = {
66, 79, 84, 84,
	6,
66, 79, 84, 84,
	7,
66, 79, 84, 84,
	8,
66, 76, 79, 67,
	9,
83, 65, 76, 84,
	10,
66, 79, 78, 69,
	12,
66, 79, 84, 84,
	14,
87, 65, 84, 67,
	19,
86, 73, 78, 69,
	23,
75, 78, 73, 70,
	24,
76, 79, 71, 32,
	25,
66, 76, 79, 67,
	26,
76, 79, 71, 32,
	28,
78, 79, 84, 69,
	40,
66, 65, 78, 68,
	41,
67, 79, 67, 79,
	42,
87, 73, 82, 69,
	55,
67, 79, 67, 79,
	58,
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
