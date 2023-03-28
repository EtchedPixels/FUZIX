#define NUM_OBJ 101
#define WORDSIZE 3
#define GAME_MAGIC 125
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
const uint8_t startcarried = 2;
const uint8_t maxcar = 8;
const uint8_t treasure = 15;
const uint8_t treasures = 13;
const uint8_t lastloc = 27;
const uint8_t startloc = 3;


const struct location locdata[] = {
		{ 	"",
 { 0, 0, 0, 0, 0, 0 } }, 
		{ 	"\x64\x65\x73\x65\x72\x74",
 { 0, 4, 0, 2, 0, 0 } }, 
		{ 	"\x64\x65\x73\x65\x72\x74",
 { 0, 3, 1, 0, 0, 0 } }, 
		{ 	"\x64\x65\x73\x65\x72\x74",
 { 2, 0, 4, 0, 0, 0 } }, 
		{ 	"\x64\x65\x73\x65\x72\x74\x20\x6E\x65\x78\x74\x20\x74\x6F\x20\x61\x20\x70\x79\x72\x61\x6D\x69\x64",
 { 1, 0, 0, 3, 0, 0 } }, 
		{ 	"\x68\x6F\x6C\x65",
 { 0, 0, 0, 0, 4, 0 } }, 
		{ 	"\x72\x6F\x63\x6B\x79\x20\x65\x6E\x74\x72\x61\x6E\x63\x65\x2D\x77\x61\x79",
 { 12, 16, 0, 0, 0, 0 } }, 
		{ 	"\x73\x61\x72\x63\x6F\x70\x68\x61\x67\x75\x73",
 { 0, 0, 0, 6, 0, 8 } }, 
		{ 	"\x62\x75\x72\x69\x61\x6C\x20\x72\x6F\x6F\x6D",
 { 9, 11, 0, 0, 7, 0 } }, 
		{ 	"\x6C\x6F\x6E\x67\x2C\x20\x6E\x61\x72\x72\x6F\x77\x20\x70\x61\x73\x73\x61\x67\x65\x77\x61\x79",
 { 0, 8, 0, 0, 0, 0 } }, 
		{ 	"\x68\x69\x64\x64\x65\x6E\x20\x61\x6C\x63\x6F\x76\x65",
 { 0, 0, 0, 8, 0, 0 } }, 
		{ 	"\x74\x61\x6C\x6C\x20\x72\x6F\x6F\x6D",
 { 8, 0, 0, 0, 0, 0 } }, 
		{ 	"\x64\x69\x6E\x69\x6E\x67\x20\x68\x61\x6C\x6C",
 { 0, 6, 13, 0, 0, 0 } }, 
		{ 	"\x68\x61\x6C\x6C\x77\x61\x79",
 { 0, 0, 0, 12, 0, 0 } }, 
		{ 	"\x72\x6F\x75\x6E\x64\x20\x61\x6C\x74\x61\x72\x20\x72\x6F\x6F\x6D",
 { 13, 17, 15, 0, 0, 0 } }, 
		{ 	"\x68\x69\x65\x72\x6F\x67\x6C\x79\x70\x68\x69\x63\x73\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 14, 0, 0 } }, 
		{ 	"\x73\x69\x74\x74\x69\x6E\x67\x20\x72\x6F\x6F\x6D",
 { 6, 0, 0, 0, 0, 0 } }, 
		{ 	"\x73\x6C\x6F\x70\x69\x6E\x67\x20\x63\x72\x61\x77\x6C\x77\x61\x79",
 { 14, 0, 0, 0, 0, 0 } }, 
		{ 	"\x52\x65\x76\x6F\x6C\x76\x69\x6E\x67\x20\x63\x61\x76\x65\x72\x6E",
 { 19, 20, 0, 0, 0, 11 } }, 
		{ 	"\x70\x72\x69\x73\x6F\x6E\x20\x63\x65\x6C\x6C",
 { 0, 20, 0, 18, 0, 0 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x61\x20\x6E\x61\x72\x72\x6F\x77\x20\x6C\x65\x64\x67\x65",
 { 0, 19, 0, 18, 0, 0 } }, 
		{ 	"\x74\x68\x72\x6F\x6E\x65\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 20 } }, 
		{ 	"\x74\x72\x65\x61\x73\x75\x72\x65\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 0, 0, 21 } }, 
		{ 	"\x2A\x49\x27\x6D\x20\x6F\x6E\x20\x74\x6F\x70\x20\x6F\x66\x20\x61\x20\x70\x79\x72\x61\x6D\x69\x64",
 { 0, 0, 0, 22, 0, 0 } }, 
		{ 	"\x50\x6F\x6F\x6C\x20\x6F\x66\x20\x77\x61\x74\x65\x72",
 { 0, 0, 3, 0, 0, 0 } }, 
		{ 	"\x2E",
 { 9, 9, 26, 9, 9, 9 } }, 
		{ 	"\x64\x72\x65\x73\x73\x69\x6E\x67\x20\x72\x6F\x6F\x6D",
 { 0, 0, 0, 25, 0, 0 } }, 
		{ 	"\x6C\x6F\x74\x20\x6F\x66\x20\x54\x52\x4F\x55\x42\x4C\x45\x21",
 { 0, 0, 0, 0, 0, 0 } }, 
};
const uint8_t objinit[] = {
	0,
	4,
	0,
	15,
	255,
	0,
	0,
	5,
	255,
	0,
	3,
	0,
	0,
	0,
	3,
	4,
	0,
	0,
	24,
	0,
	6,
	6,
	0,
	7,
	0,
	6,
	0,
	0,
	6,
	8,
	8,
	8,
	0,
	0,
	26,
	0,
	0,
	0,
	9,
	0,
	0,
	16,
	16,
	0,
	0,
	9,
	0,
	10,
	10,
	10,
	0,
	0,
	13,
	11,
	11,
	11,
	16,
	0,
	0,
	0,
	12,
	0,
	0,
	13,
	0,
	13,
	14,
	23,
	0,
	0,
	0,
	17,
	0,
	0,
	0,
	0,
	19,
	19,
	19,
	0,
	0,
	0,
	0,
	20,
	20,
	0,
	21,
	21,
	21,
	21,
	0,
	20,
	0,
	0,
	0,
	0,
	0,
	0,
	0,
	22,
	22,
};


const char *objtext[] = {
	"",
	"\x53\x74\x6F\x6E\x65",
	"\x44\x6F\x6F\x72\x20\x77\x69\x74\x68\x20\x6C\x61\x72\x67\x65\x20\x6B\x65\x79\x68\x6F\x6C\x65",
	"\x44\x72\x69\x65\x64\x20\x43\x61\x6D\x65\x6C\x20\x4A\x65\x72\x6B\x79",
	"\x45\x6D\x70\x74\x79\x20\x63\x61\x6E\x74\x65\x65\x6E",
	"\x46\x75\x6C\x6C\x20\x63\x61\x6E\x74\x65\x65\x6E",
	"\x48\x6F\x6C\x65",
	"\x54\x69\x6E\x79\x20\x6C\x6F\x63\x6B\x65\x64\x20\x64\x6F\x6F\x72",
	"\x55\x6E\x6C\x69\x74\x20\x66\x6C\x61\x73\x68\x6C\x69\x74\x65",
	"\x4C\x69\x74\x20\x66\x6C\x61\x73\x68\x6C\x69\x74\x65",
	"\x57\x6F\x6F\x64\x65\x6E\x20\x70\x6F\x6C\x65\x20\x73\x74\x69\x63\x6B\x69\x6E\x67\x20\x66\x72\x6F\x6D\x20\x73\x61\x6E\x64",
	"\x53\x68\x6F\x76\x65\x6C",
	"\x54\x69\x6E\x79\x20\x6B\x65\x79",
	"\x4C\x75\x6D\x70\x20\x6F\x66\x20\x63\x6F\x61\x6C",
	"\x50\x6F\x6F\x6C\x20\x6F\x66\x20\x6C\x69\x71\x75\x69\x64",
	"\x53\x69\x67\x6E",
	"\x44\x6F\x6F\x72\x20\x77\x69\x74\x68\x20\x6C\x61\x72\x67\x65\x20\x6B\x65\x79\x68\x6F\x6C\x65",
	"\x4F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
	"\x4C\x61\x72\x67\x65\x20\x6B\x65\x79",
	"\x49\x27\x6D\x20\x77\x65\x61\x72\x69\x6E\x67\x20\x61\x6E\x20\x69\x72\x6F\x6E\x20\x67\x6C\x6F\x76\x65",
	"\x43\x6C\x6F\x73\x65\x64\x20\x73\x61\x72\x63\x6F\x70\x68\x61\x67\x75\x73",
	"\x4D\x6F\x75\x6C\x64\x79\x20\x62\x61\x6E\x64\x61\x67\x65\x73",
	"\x4F\x70\x65\x6E\x20\x73\x61\x72\x63\x6F\x70\x68\x61\x67\x75\x73",
	"\x53\x74\x61\x69\x72\x73",
	"\x42\x6F\x6E\x65\x73",
	"\x4F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
	"\x54\x69\x6E\x79\x20\x6F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
	"\x53\x6D\x61\x6C\x6C\x20\x4E\x6F\x6D\x61\x64",
	"\x50\x69\x73\x74\x6F\x6C",
	"\x46\x65\x61\x72\x73\x6F\x6D\x65\x20\x6D\x75\x6D\x6D\x79",
	"\x2A\x20\x41\x4E\x54\x49\x51\x55\x45\x20\x54\x41\x50\x45\x53\x54\x52\x59\x20\x2A",
	"\x42\x75\x72\x6E\x69\x6E\x67\x20\x74\x61\x6E\x6E\x61\x20\x6C\x65\x61\x76\x65\x73",
	"\x45\x6E\x72\x61\x67\x65\x64\x20\x6D\x75\x6D\x6D\x79",
	"\x53\x6C\x65\x65\x70\x69\x6E\x67\x20\x6D\x75\x6D\x6D\x79",
	"\x2A\x20\x47\x4F\x4C\x44\x20\x53\x43\x41\x52\x41\x42\x20\x2A",
	"\x2E",
	"\x57\x65\x74\x20\x74\x61\x6E\x6E\x61\x20\x6C\x65\x61\x76\x65\x73",
	"\x45\x6E\x74\x72\x61\x6E\x63\x65\x20\x74\x6F\x20\x61\x6E\x20\x61\x6C\x63\x6F\x76\x65",
	"\x42\x72\x69\x63\x6B\x65\x64\x20\x75\x70\x20\x64\x6F\x6F\x72\x77\x61\x79",
	"\x4F\x70\x65\x6E\x20\x64\x6F\x6F\x72",
	"\x2A\x20\x47\x4F\x4C\x44\x20\x43\x4F\x49\x4E\x20\x2A",
	"\x41\x73\x68\x65\x73",
	"\x42\x61\x73\x6B\x65\x74",
	"\x50\x61\x73\x73\x61\x67\x65\x77\x61\x79\x20\x62\x65\x68\x69\x6E\x64\x20\x66\x69\x72\x65\x70\x6C\x61\x63\x65",
	"\x4F\x70\x65\x6E\x20\x74\x72\x65\x61\x73\x75\x72\x65\x20\x63\x6F\x66\x66\x65\x72",
	"\x52\x6F\x70\x65",
	"\x20",
	"\x43\x68\x6F\x70\x70\x69\x6E\x67\x20\x62\x6C\x6F\x63\x6B",
	"\x53\x6B\x75\x6C\x6C",
	"\x42\x6F\x78",
	"\x2A\x20\x47\x4F\x4C\x44\x20\x54\x45\x45\x54\x48\x20\x2A",
	"\x49\x72\x6F\x6E\x20\x67\x6C\x6F\x76\x65",
	"\x41\x72\x63\x68\x77\x61\x79",
	"\x4D\x65\x74\x61\x6C\x20\x62\x61\x72\x20\x70\x72\x6F\x74\x72\x75\x64\x69\x6E\x67\x20\x66\x72\x6F\x6D\x20\x63\x65\x69\x6C\x69\x6E\x67",
	"\x44\x65\x63\x61\x70\x69\x74\x61\x74\x65\x64\x20\x73\x6B\x65\x6C\x65\x74\x6F\x6E",
	"\x53\x61\x77",
	"\x46\x69\x72\x65\x70\x6C\x61\x63\x65",
	"\x20",
	"\x4C\x61\x64\x64\x65\x72",
	"\x47\x6C\x6F\x77\x69\x6E\x67\x20\x73\x6B\x65\x6C\x65\x74\x6F\x6E",
	"\x54\x61\x62\x6C\x65",
	"\x20\x20",
	"\x20",
	"\x47\x69\x61\x6E\x74\x20\x4F\x79\x73\x74\x65\x72",
	"\x2A\x20\x42\x4C\x41\x43\x4B\x20\x50\x45\x41\x52\x4C\x20\x2A",
	"\x46\x6C\x75\x74\x65",
	"\x42\x6C\x6F\x6F\x64\x2D\x73\x74\x61\x69\x6E\x65\x64\x20\x61\x6C\x74\x61\x72",
	"\x2A\x20\x50\x4C\x41\x54\x49\x4E\x55\x4D\x20\x42\x41\x52\x20\x2A",
	"\x2A\x20\x44\x49\x41\x4D\x4F\x4E\x44\x20\x4E\x45\x43\x4B\x4C\x41\x43\x45\x20\x2A",
	"\x53\x61\x74\x69\x73\x66\x69\x65\x64\x20\x72\x61\x74\x73",
	"\x20",
	"\x53\x74\x61\x72\x76\x69\x6E\x67\x20\x72\x61\x74\x73",
	"\x20",
	"\x48\x69\x73\x73\x69\x6E\x67\x20\x63\x6F\x62\x72\x61",
	"\x2A\x20\x47\x4F\x4C\x44\x20\x4E\x45\x43\x4B\x4C\x41\x43\x45\x20\x2A",
	"\x50\x61\x73\x73\x61\x67\x65\x77\x61\x79",
	"\x44\x65\x61\x64\x20\x65\x78\x70\x6C\x6F\x72\x65\x72\x20\x63\x68\x61\x69\x6E\x65\x64\x20\x74\x6F\x20\x77\x61\x6C\x6C",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x52\x75\x62\x62\x69\x73\x68",
	"\x43\x6C\x6F\x73\x65\x64\x20\x70\x6F\x72\x74\x61\x6C",
	"\x4F\x70\x65\x6E\x20\x70\x6F\x72\x74\x61\x6C",
	"\x50\x75\x72\x70\x6C\x65\x20\x57\x6F\x72\x6D",
	"\x2A\x20\x47\x4F\x4C\x44\x20\x50\x49\x4E\x20\x2A",
	"\x2A\x20\x4A\x41\x44\x45\x20\x43\x41\x52\x56\x49\x4E\x47\x20\x2A",
	"\x2A\x20\x53\x41\x50\x50\x48\x49\x52\x45\x20\x2A",
	"\x48\x6F\x6C\x65\x20\x69\x6E\x20\x74\x68\x65\x20\x63\x65\x69\x6C\x69\x6E\x67",
	"\x52\x6F\x70\x65\x20\x68\x61\x6E\x67\x69\x6E\x67\x20\x66\x72\x6F\x6D\x20\x63\x65\x69\x6C\x69\x6E\x67",
	"\x49\x72\x6F\x6E\x20\x73\x74\x61\x74\x75\x65\x20\x6F\x66\x20\x50\x68\x61\x72\x6F\x61\x68\x20\x73\x65\x61\x74\x65\x64\x20\x6F\x6E\x20\x74\x68\x72\x6F\x6E\x65",
	"\x43\x68\x61\x69\x6E\x20\x68\x61\x6E\x67\x69\x6E\x67\x20\x66\x72\x6F\x6D\x20\x63\x65\x69\x6C\x69\x6E\x67",
	"\x43\x68\x65\x73\x74",
	"\x57\x61\x6C\x6C\x20\x4D\x75\x72\x61\x6C",
	"\x20\x2A\x20\x52\x55\x42\x59\x20\x2A",
	"\x50\x6F\x6F\x6C\x20\x6F\x66\x20\x6C\x69\x71\x75\x69\x64\x20\x28\x66\x61\x72\x20\x62\x65\x6C\x6F\x77\x20\x6C\x65\x64\x67\x65\x29",
	"\x53\x70\x69\x72\x61\x6C\x20\x53\x74\x61\x69\x72\x63\x61\x73\x65",
	"\x2A\x20\x50\x4C\x41\x54\x49\x4E\x55\x4D\x20\x43\x52\x4F\x57\x4E\x20\x2A",
	"\x53\x74\x61\x6E\x64\x69\x6E\x67\x20\x69\x72\x6F\x6E\x20\x73\x74\x61\x74\x75\x65",
	"\x49\x72\x6F\x6E\x20\x53\x74\x61\x74\x75\x65\x2C\x20\x73\x6C\x6F\x77\x6C\x79\x20\x61\x64\x76\x61\x6E\x63\x69\x6E\x67\x2E\x20\x2E\x20\x2E",
	"\x50\x69\x6C\x65\x20\x6F\x66\x20\x4D\x65\x6C\x74\x65\x64\x20\x69\x72\x6F\x6E",
	"\x2A\x20\x45\x4D\x45\x52\x41\x4C\x44\x20\x42\x52\x41\x43\x45\x4C\x45\x54\x20\x2A",
	"\x4F\x70\x65\x6E\x20\x77\x69\x6E\x64\x6F\x77",
	"\x54\x72\x65\x61\x73\x75\x72\x65\x20\x63\x6F\x66\x66\x65\x72",
	"\x42\x61\x72\x72\x65\x64\x20\x77\x69\x6E\x64\x6F\x77",
};
const char *msgptr[] = {
	"",
	"\x49\x20\x73\x65\x65\x20\x73\x6F\x6D\x65\x74\x68\x69\x6E\x67\x2E",
	"\x42\x61\x72\x65\x2D\x68\x61\x6E\x64\x65\x64\x3F",
	"\x2E",
	"\x49\x20\x63\x61\x6E\x27\x74\x2E",
	"\x4E\x6F\x20\x63\x6F\x6E\x74\x61\x69\x6E\x65\x72\x2E",
	"\x43\x61\x6E\x74\x65\x65\x6E\x20\x69\x73\x20\x66\x75\x6C\x6C\x2E",
	"\x57\x65\x6C\x63\x6F\x6D\x65\x20\x74\x6F\x20\x41\x64\x76\x65\x6E\x74\x75\x72\x65\x3A\x20\x38\x20\x22\x50\x59\x52\x41\x4D\x49\x44\x20\x4F\x46\x20\x44\x4F\x4F\x4D\x22",
	"\x42\x79\x20\x41\x6C\x76\x69\x6E\x20\x46\x69\x6C\x65\x73\x20\x26\x20\x53\x63\x6F\x74\x74\x20\x41\x64\x61\x6D\x73\x20\x44\x65\x64\x69\x63\x61\x74\x65\x64\x20\x74\x6F\x20\x52\x61\x79\x20\x48\x61\x72\x73\x68\x61\x77\x21",
	"\x53\x69\x67\x6E\x3A\x20\x48\x65\x20\x77\x68\x6F\x20\x64\x65\x66\x69\x6C\x65\x73\x20\x74\x68\x65\x20\x74\x6F\x6D\x62\x73\x20\x6F\x66\x20\x45\x67\x79\x70\x74\x20\x73\x68\x61\x6C\x6C\x20\x73\x75\x72\x65\x6C\x79\x20\x70\x65\x72\x69\x73\x68\x21",
	"\x54\x68\x65\x20\x73\x6F\x75\x6E\x64\x20\x6F\x66\x20\x6D\x61\x63\x68\x69\x6E\x65\x72\x79\x2E",
	"\x49\x20\x68\x65\x61\x72\x20\x73\x74\x72\x61\x6E\x67\x65\x20\x6E\x6F\x69\x73\x65\x73\x21",
	"\x41\x52\x52\x47\x48\x21\x20\x50\x6F\x69\x73\x6F\x6E\x20\x6E\x65\x65\x64\x6C\x65\x20\x69\x6E\x20\x74\x68\x65\x20\x6C\x6F\x63\x6B\x2E",
	"\x54\x68\x65\x20\x72\x69\x67\x68\x74\x20\x6B\x65\x79\x20\x63\x6F\x75\x6C\x64\x20\x68\x65\x6C\x70\x20\x28\x6D\x61\x79\x62\x65\x29\x2E",
	"\x4C\x61\x72\x67\x65\x20\x73\x74\x6F\x6E\x65\x20\x66\x61\x6C\x6C\x73\x20\x6F\x6E\x20\x6D\x65\x21",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x77\x6F\x6E\x27\x74\x20\x66\x69\x74\x2E",
	"\x4E\x6F\x69\x73\x65\x20\x73\x63\x61\x72\x65\x73\x20\x74\x68\x65\x6D\x20\x6F\x66\x66",
	"\x54\x68\x61\x6E\x6B\x73\x2E",
	"\x43\x72\x61\x73\x68\x21\x21\x21",
	"\x49\x20\x68\x65\x61\x72\x20\x61\x20\x68\x6F\x6C\x6C\x6F\x77\x20\x6C\x61\x75\x67\x68\x2E\x20\x2E\x20\x2E",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x73\x65\x65\x20\x61\x6E\x79\x20\x72\x65\x61\x73\x6F\x6E\x20\x74\x6F\x20\x74\x68\x72\x6F\x77\x20\x61\x20",
	"\x41\x20\x73\x6D\x61\x6C\x6C\x20\x64\x65\x73\x65\x72\x74\x20\x6E\x6F\x6D\x61\x64\x20\x61\x70\x70\x65\x61\x72\x73\x2E\x2E\x2E",
	"\x54\x68\x65\x20\x6E\x6F\x6D\x61\x64\x20\x73\x74\x75\x63\x6B\x20\x6F\x75\x74\x20\x68\x69\x73\x20\x74\x6F\x6E\x67\x75\x65\x20\x61\x74\x20\x6D\x65\x2E",
	"\x57\x68\x61\x74\x21\x20\x54\x68\x65\x20\x6E\x6F\x6D\x61\x64\x20\x6A\x75\x6D\x70\x65\x64\x20\x6D\x65\x20\x77\x68\x69\x6C\x65\x20\x6D\x79\x20\x62\x61\x63\x6B\x20\x77\x61\x73\x20\x74\x75\x72\x6E\x65\x64\x21",
	"\x4E\x6F\x6D\x61\x64\x20\x76\x61\x6E\x69\x73\x68\x65\x73\x20\x69\x6E\x20\x61\x20\x70\x75\x66\x66\x20\x6F\x66\x20\x79\x65\x6C\x6C\x6F\x77\x20\x73\x6D\x6F\x6B\x65\x2E",
	"\x53\x6F\x72\x72\x79\x2C\x20\x77\x6F\x6E\x27\x74\x20\x77\x6F\x72\x6B\x2E",
	"\x50\x69\x73\x74\x6F\x6C\x20\x68\x61\x73",
	"\x62\x75\x6C\x6C\x65\x74\x73",
	"\x4E\x6F\x20\x62\x75\x6C\x6C\x65\x74\x73\x2E",
	"\x47\x6F\x74\x20\x68\x69\x6D\x21\x21",
	"\x54\x6F\x6F\x20\x68\x6F\x74\x2E",
	"\x4D\x75\x6D\x6D\x79\x20\x6D\x6F\x76\x65\x73\x20\x74\x6F\x77\x61\x72\x64\x20\x6D\x65\x2E",
	"\x4D\x75\x6D\x6D\x79\x20\x68\x61\x73\x20\x6D\x65\x20\x62\x79\x20\x74\x68\x65\x20\x74\x68\x72\x6F\x61\x74\x21",
	"\x4D\x75\x6D\x6D\x79",
	"\x77\x6F\x6E\x27\x74\x20\x6C\x65\x74\x20\x6D\x65",
	"\x59\x6F\x75\x20\x62\x65\x20\x77\x65\x69\x72\x64\x2E\x20\x43\x75\x74\x20\x74\x68\x61\x74\x20\x6F\x75\x74\x2E",
	"\x44\x6F\x65\x73\x6E\x27\x74\x20\x62\x6F\x74\x68\x65\x72\x20\x68\x69\x6D\x2E",
	"\x43\x6F\x62\x72\x61\x20\x63\x72\x61\x77\x6C\x73\x20\x6F\x75\x74\x2C\x20\x70\x72\x65\x73\x73\x65\x73\x20\x61\x20\x6C\x6F\x6F\x73\x65\x20\x62\x72\x69\x63\x6B\x20\x61\x6E\x64\x20\x74\x68\x65\x6E\x20\x64\x69\x73\x61\x70\x70\x65\x61\x72\x73\x20\x69\x6E\x74\x6F\x61\x20\x70\x61\x73\x73\x61\x67\x65\x20\x77\x68\x69\x63\x68\x20\x61\x70\x70\x65\x61\x72\x65\x64\x2E",
	"\x53\x6F\x6D\x65\x74\x68\x69\x6E\x67\x20\x66\x61\x6C\x6C\x73\x20\x6F\x75\x74\x2E",
	"\x2E",
	"\x2E",
	"\x50\x75\x72\x70\x6C\x65\x20\x77\x6F\x72\x6D\x20\x64\x65\x76\x6F\x75\x72\x73\x20\x6D\x65\x2E",
	"\x43\x68\x61\x69\x6E\x20\x74\x6F\x6F\x20\x74\x6F\x75\x67\x68\x2E",
	"\x49\x74\x73\x20\x62\x6C\x61\x63\x6B\x2C\x20\x64\x69\x72\x74\x79\x20\x26\x20\x64\x75\x73\x74\x79",
	"\x54\x68\x61\x74\x20\x66\x65\x65\x6C\x73\x20\x67\x6F\x6F\x64\x2E",
	"\x52\x61\x74\x73",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x2E\x20\x2E\x20\x2E\x20\x4E\x6F\x74\x68\x69\x6E\x67\x20\x68\x61\x70\x70\x65\x6E\x73\x2E",
	"\x49\x74\x73\x20\x6C\x6F\x63\x6B\x65\x64\x2E",
	"\x53\x6B\x65\x6C\x65\x74\x6F\x6E\x20\x70\x6C\x61\x63\x65\x73\x20\x73\x6B\x75\x6C\x6C\x20\x6F\x6E\x20\x68\x69\x73\x20\x73\x68\x6F\x75\x6C\x64\x65\x72\x73\x2C\x20\x67\x72\x61\x73\x70\x73",
	"\x6D\x65\x74\x61\x6C\x20\x62\x61\x72\x2C\x20\x70\x75\x6C\x6C\x73\x20\x64\x6F\x77\x6E\x20\x6C\x61\x64\x64\x65\x72",
	"\x49\x20\x63\x61\x6E\x27\x74\x20\x72\x65\x61\x63\x68\x20\x69\x74\x2E",
	"\x49\x27\x6D\x20\x6E\x6F\x74\x20\x61\x6E\x20\x61\x72\x73\x6F\x6E\x69\x73\x74\x2E",
	"\x49\x74\x20\x73\x61\x79\x73\x3A\x20\x22\x4C\x45\x41\x56\x45\x20\x2A\x54\x52\x45\x41\x53\x55\x52\x45\x53\x2A\x20\x48\x45\x52\x45\x21\x22",
	"\x4F\x79\x73\x74\x65\x72\x20\x6D\x61\x6B\x65\x73\x20\x61\x20\x73\x6C\x6F\x62\x62\x65\x72\x69\x6E\x67\x20\x6E\x6F\x69\x73\x65",
	"\x4F\x79\x73\x74\x65\x72\x20\x77\x6F\x6E\x27\x74\x20\x6C\x65\x74\x20\x6D\x65",
	"\x49\x74\x20\x61\x74\x74\x61\x63\x6B\x73\x21",
	"\x49\x20\x6A\x75\x73\x74\x20\x63\x61\x75\x67\x68\x74\x20\x64\x65\x6E\x67\x75\x65\x20\x66\x65\x76\x65\x72\x2E\x20\x56\x65\x72\x79\x20\x62\x61\x64\x2E",
	"\x43\x6F\x6E\x66\x75\x73\x69\x6E\x67\x2E\x20\x50\x61\x72\x74\x20\x61\x70\x70\x65\x61\x72\x73\x20\x6D\x69\x73\x73\x69\x6E\x67\x2E",
	"\x49\x27\x6D\x20\x74\x68\x65\x72\x65\x2E",
	"\x50\x6F\x75\x72\x20\x77\x61\x74\x65\x72\x3F",
	"\x49\x74\x27\x73\x20\x73\x74\x75\x63\x6B\x2E",
	"\x2E",
	"\x43\x6F\x62\x72\x61",
	"\x53\x74\x72\x61\x6E\x67\x65\x20\x6D\x61\x72\x6B\x69\x6E\x67\x73\x20\x61\x72\x65\x20\x70\x72\x65\x73\x65\x6E\x74\x2E",
	"\x61\x74\x74\x61\x63\x6B\x73\x21",
	"\x4D\x69\x73\x73\x65\x64\x2E\x2E\x2E",
	"\x4C\x69\x71\x75\x69\x64\x20\x69\x73\x20\x61\x63\x69\x64\x2E",
	"\x43\x61\x6E\x74\x65\x65\x6E\x20\x64\x69\x73\x73\x6F\x6C\x76\x65\x73\x2E",
	"\x49\x72\x6F\x6E\x20\x53\x74\x61\x74\x75\x65",
	"\x4D\x75\x72\x61\x6C\x3A\x20\x53\x65\x65\x6B\x20\x79\x65\x20\x77\x65\x6C\x6C\x20\x74\x68\x65\x20\x48\x45\x41\x52\x54\x20\x6F\x66\x20\x49\x72\x6F\x6E\x2E",
	"\x49\x72\x6F\x6E\x20\x53\x74\x61\x74\x75\x65\x20\x74\x65\x61\x72\x73\x20\x6D\x65\x20\x61\x70\x61\x72\x74\x21",
	"\x52\x75\x62\x79\x20\x66\x61\x6C\x6C\x73\x20\x69\x6E\x74\x6F\x20\x70\x6F\x6F\x6C\x20\x6F\x66\x20\x61\x63\x69\x64\x2C\x20\x62\x75\x72\x6E\x73\x20\x75\x70\x2E",
	"\x49\x20\x73\x65\x65\x20\x6E\x6F\x74\x68\x69\x6E\x67\x20\x73\x70\x65\x63\x69\x61\x6C\x2E",
	"\x41\x20\x74\x69\x6E\x79\x20\x76\x6F\x69\x63\x65\x20\x73\x61\x79\x73\x3A\x20\x22\x53\x45\x41\x52\x43\x48\x20\x41\x4E\x44\x20\x59\x45\x20\x53\x48\x41\x4C\x4C\x20\x46\x49\x4E\x44\x21\x22",
	"\x49\x20\x63\x61\x6E\x27\x74\x20\x6C\x69\x66\x74\x20\x69\x74\x2E\x20\x54\x68\x69\x73\x20\x69\x73\x20\x61\x20\x62\x69\x67\x20\x6F\x79\x73\x74\x65\x72",
	"\x49\x20\x64\x6F\x6E\x27\x74\x20\x73\x65\x65\x20\x69\x74",
	"\x4E\x6F\x6D\x61\x64\x3A\x20\x22\x4C\x6F\x6F\x6B\x20\x75\x70\x20\x64\x65\x63\x61\x70\x69\x74\x61\x74\x65\x64\x21\x22",
	"\x4E\x6F\x6D\x61\x64\x3A\x20\x22\x52\x55\x4E\x20\x59\x4F\x55\x20\x46\x4F\x4F\x4C\x21\x22",
	"\x4E\x6F\x6D\x61\x64\x3A\x20\x22\x4F\x79\x73\x74\x65\x72\x20\x74\x68\x69\x72\x73\x74\x79\x3F\x22",
	"\x47\x6C\x6F\x76\x65\x20\x66\x61\x6C\x6C\x73\x20\x6F\x66\x66\x20\x6D\x79\x20\x73\x77\x65\x61\x74\x79\x20\x68\x61\x6E\x64",
	"\x41\x68\x2C\x20\x27\x74\x69\x73\x20\x6D\x75\x73\x69\x63\x20\x74\x6F\x20\x6D\x79\x20\x65\x61\x72\x73\x2E",
	"\x4F\x4B",
	"\x49\x6E\x74\x6F\x20\x74\x68\x65\x20\x77\x6F\x72\x6D\x27\x73\x20\x6D\x6F\x75\x74\x68\x21",
	"\x54\x52\x59\x3A\x20\x22\x54\x41\x4B\x45\x20\x49\x4E\x56\x45\x4E\x54\x4F\x52\x59\x22",
	"\x4D\x69\x72\x72\x6F\x72\x73\x20\x45\x56\x45\x52\x59\x57\x48\x45\x52\x45\x21\x20\x4C\x69\x67\x68\x74\x20\x62\x6C\x69\x6E\x64\x73\x20\x6D\x65\x2C\x20\x73\x6F\x20\x49\x20\x73\x68\x75\x74\x20\x69\x74\x20\x4F\x46\x46\x21",
	"\x50\x68\x61\x72\x6F\x61\x68\x27\x73\x20\x68\x65\x61\x72\x74\x20\x69\x73\x20\x72\x65\x64\x20\x6C\x69\x6B\x65\x20\x79\x6F\x75\x72\x73\x2C\x20\x79\x65\x74\x20\x65\x76\x69\x6C\x20\x68\x61\x73\x20\x64\x61\x72\x6B\x65\x6E\x65\x64\x20\x69\x74\x21",
	"\x57\x4F\x57\x2C\x20\x6D\x61\x6E\x2C\x20\x63\x72\x61\x7A\x79\x2E\x2E\x2E",
	"\x31\x30\x30\x30\x20\x79\x65\x61\x72\x73\x20\x62\x61\x64\x20\x6C\x75\x63\x6B\x20\x73\x74\x61\x72\x74\x73\x20\x77\x69\x74\x68\x20\x61\x20\x63\x61\x76\x65\x20\x69\x6E\x21",
	"\x49\x20\x66\x65\x65\x6C\x20\x61\x20\x63\x6F\x69\x6E\x20\x6F\x6E\x20\x74\x68\x65\x20\x66\x6C\x6F\x6F\x72",
};


const uint8_t status[] = {
	170, 
	9, 3, 0, 3, 
	58, 7, 8, 
	134, 50, 
	1, 71, 
	45, 114, 61, 
	177, 
	8, 16, 13, 9, 0, 9, 0, 8, 
	72, 76, 
	137, 2, 
	14, 27, 0, 27, 
	53, 21, 
	173, 
	9, 14, 0, 14, 0, 4, 
	58, 79, 
	174, 
	8, 12, 1, 28, 16, 1, 
	26, 78, 27, 
	168, 
	8, 12, 0, 12, 
	60, 
	143, 70, 
	8, 11, 0, 11, 0, 27, 
	29, 24, 60, 55, 
	169, 
	8, 11, 0, 11, 
	60, 115, 
	141, 10, 
	2, 29, 0, 29, 0, 32, 
	72, 31, 
	133, 20, 
	2, 32, 
	32, 61, 
	142, 10, 
	1, 19, 0, 19, 0, 51, 
	59, 53, 129, 
	149, 25, 
	2, 27, 4, 11, 0, 20, 14, 59, 8, 20, 
	126, 60, 
	179, 
	2, 54, 2, 48, 0, 59, 0, 58, 
	48, 49, 53, 53, 
	182, 
	2, 54, 2, 48, 0, 54, 0, 48, 0, 53, 
	55, 55, 55, 
	172, 
	13, 27, 5, 27, 0, 27, 
	53, 
	183, 
	2, 63, 2, 3, 0, 3, 0, 10, 0, 64, 
	55, 103, 58, 53, 
	136, 20, 
	2, 27, 4, 19, 
	127, 
	149, 25, 
	2, 27, 2, 63, 14, 64, 8, 20, 0, 20, 
	128, 60, 
	181, 
	8, 17, 14, 96, 0, 96, 0, 21, 0, 86, 
	62, 59, 
	181, 
	2, 3, 2, 71, 0, 3, 0, 71, 0, 69, 
	55, 72, 
	134, 30, 
	2, 73, 
	112, 114, 61, 
	134, 10, 
	2, 71, 
	45, 114, 61, 
	133, 40, 
	2, 80, 
	41, 61, 
	133, 20, 
	2, 95, 
	120, 61, 
	169, 
	4, 23, 8, 15, 
	57, 76, 
	178, 
	4, 25, 3, 9, 0, 9, 0, 8, 
	72, 134, 76, 
	165, 
	4, 27, 
	65, 63, 
	173, 
	8, 1, 0, 1, 0, 1, 
	60, 83, 
	137, 1, 
	2, 27, 6, 28, 
	23, 61, 
};
const uint8_t actions[] = {
	5, 30, 27, 
	4, 15, 
	131, 113, 
	6, 52, 0, 
	4, 20, 
	131, 116, 61, 
	1, 52, 0, 
	131, 46, 
	8, 41, 68, 
	4, 11, 1, 55, 
	50, 
	5, 30, 46, 
	3, 1, 
	131, 113, 
	5, 51, 0, 
	4, 24, 
	131, 44, 
	6, 51, 0, 
	4, 20, 
	131, 116, 61, 
	9, 40, 46, 
	1, 1, 0, 1, 
	53, 131, 
	21, 30, 83, 
	2, 56, 14, 13, 14, 90, 0, 13, 9, 17, 
	1, 53, 
	17, 27, 0, 
	1, 11, 14, 12, 4, 1, 0, 12, 
	1, 53, 
	4, 27, 0, 
	6, 11, 
	2, 
	12, 27, 0, 
	1, 11, 13, 12, 4, 1, 
	122, 
	13, 30, 57, 
	2, 76, 14, 81, 0, 81, 
	38, 53, 
	5, 10, 16, 
	0, 12, 
	66, 58, 
	5, 46, 0, 
	0, 12, 
	66, 58, 
	4, 27, 0, 
	4, 2, 
	122, 
	8, 27, 0, 
	4, 2, 13, 13, 
	122, 
	22, 10, 46, 
	4, 4, 2, 1, 14, 16, 0, 16, 0, 1, 
	53, 10, 52, 
	17, 10, 7, 
	4, 24, 1, 4, 0, 4, 0, 5, 
	55, 52, 
	8, 10, 7, 
	2, 14, 6, 4, 
	5, 
	10, 10, 7, 
	2, 14, 1, 5, 
	0, 0, 6, 
	9, 1, 12, 
	4, 3, 0, 24, 
	54, 76, 
	17, 10, 7, 
	4, 3, 1, 4, 0, 4, 0, 5, 
	55, 52, 
	14, 1, 10, 
	2, 17, 4, 4, 0, 6, 
	54, 56, 76, 
	8, 1, 10, 
	2, 16, 4, 4, 
	13, 
	16, 27, 0, 
	1, 11, 14, 6, 4, 4, 0, 6, 
	53, 
	8, 27, 0, 
	1, 11, 2, 6, 
	122, 
	9, 1, 53, 
	2, 6, 0, 5, 
	54, 76, 
	20, 14, 10, 
	2, 16, 1, 18, 8, 6, 0, 17, 0, 16, 
	72, 
	8, 14, 10, 
	2, 16, 6, 18, 
	13, 
	4, 12, 9, 
	2, 15, 
	9, 
	0, 17, 0, 
	63, 
	13, 14, 10, 
	2, 16, 1, 18, 9, 6, 
	14, 61, 
	23, 14, 10, 
	4, 5, 1, 12, 0, 6, 0, 7, 0, 26, 
	11, 10, 58, 72, 
	8, 14, 10, 
	4, 5, 6, 12, 
	13, 
	8, 1, 10, 
	4, 5, 8, 6, 
	15, 
	13, 44, 7, 
	1, 5, 0, 5, 0, 4, 
	17, 72, 
	12, 14, 14, 
	2, 20, 0, 22, 0, 20, 
	72, 
	10, 1, 14, 
	2, 22, 0, 7, 
	54, 76, 56, 
	14, 13, 13, 
	2, 54, 0, 24, 0, 54, 
	18, 19, 72, 
	14, 22, 13, 
	2, 54, 0, 24, 0, 54, 
	18, 19, 72, 
	14, 1, 10, 
	4, 6, 2, 25, 0, 4, 
	54, 57, 76, 
	9, 1, 23, 
	2, 29, 2, 37, 
	33, 34, 
	0, 25, 0, 
	25, 
	4, 24, 0, 
	6, 28, 
	2, 
	4, 24, 0, 
	19, 0, 
	28, 
	9, 24, 17, 
	0, 11, 0, 1, 
	58, 58, 
	22, 6, 7, 
	1, 5, 2, 31, 0, 36, 0, 31, 0, 2, 
	72, 58, 73, 
	205, 
	8, 2, 0, 5, 0, 4, 
	72, 73, 
	214, 
	8, 2, 2, 29, 0, 33, 0, 29, 0, 2, 
	72, 60, 73, 
	213, 
	8, 2, 2, 32, 0, 32, 0, 33, 0, 2, 
	72, 60, 
	4, 10, 20, 
	5, 30, 
	4, 
	5, 10, 20, 
	2, 29, 
	33, 34, 
	6, 10, 22, 
	2, 29, 
	33, 34, 35, 
	4, 10, 21, 
	2, 31, 
	30, 
	18, 24, 22, 
	2, 29, 0, 29, 0, 32, 0, 1, 
	36, 72, 58, 
	9, 24, 22, 
	2, 32, 0, 1, 
	36, 58, 
	4, 13, 58, 
	1, 90, 
	4, 
	13, 30, 24, 
	3, 48, 14, 50, 0, 50, 
	53, 38, 
	22, 30, 37, 
	3, 49, 14, 51, 0, 51, 14, 19, 13, 24, 
	1, 38, 53, 
	6, 79, 17, 
	2, 27, 
	34, 45, 22, 
	9, 24, 22, 
	2, 54, 0, 1, 
	36, 58, 
	13, 10, 48, 
	2, 10, 0, 10, 0, 11, 
	55, 52, 
	4, 35, 68, 
	2, 53, 
	50, 
	4, 1, 68, 
	2, 53, 
	50, 
	9, 1, 36, 
	2, 58, 0, 18, 
	54, 76, 
	8, 1, 43, 
	4, 13, 9, 10, 
	104, 
	17, 1, 43, 
	4, 13, 8, 10, 6, 63, 0, 14, 
	54, 76, 
	13, 10, 76, 
	2, 60, 0, 60, 13, 68, 
	131, 52, 
	9, 10, 46, 
	2, 1, 0, 1, 
	52, 131, 
	17, 41, 76, 
	3, 55, 3, 60, 0, 68, 14, 68, 
	1, 53, 
	17, 28, 77, 
	1, 8, 9, 16, 0, 8, 0, 9, 
	72, 76, 
	0, 16, 0, 
	65, 
	13, 14, 77, 
	1, 9, 0, 8, 0, 9, 
	72, 76, 
	13, 30, 55, 
	4, 16, 14, 74, 0, 74, 
	1, 53, 
	17, 30, 32, 
	14, 73, 4, 16, 14, 43, 0, 73, 
	53, 1, 
	6, 10, 54, 
	2, 73, 
	112, 114, 61, 
	23, 32, 42, 
	4, 16, 14, 43, 1, 65, 0, 43, 0, 73, 
	37, 53, 55, 73, 
	200, 
	0, 75, 0, 17, 
	62, 
	13, 10, 32, 
	2, 42, 14, 43, 0, 73, 
	38, 53, 
	13, 1, 74, 
	4, 16, 2, 43, 0, 17, 
	54, 76, 
	8, 1, 55, 
	4, 16, 14, 74, 
	1, 
	13, 1, 74, 
	4, 17, 2, 75, 0, 16, 
	54, 76, 
	4, 32, 42, 
	1, 65, 
	130, 
	14, 10, 76, 
	2, 60, 0, 60, 14, 68, 
	52, 131, 11, 
	13, 30, 58, 
	2, 77, 14, 82, 0, 82, 
	1, 53, 
	9, 30, 58, 
	2, 77, 13, 82, 
	106, 61, 
	4, 1, 59, 
	2, 78, 
	13, 
	5, 1, 59, 
	2, 79, 
	132, 61, 
	17, 14, 59, 
	2, 78, 0, 80, 0, 79, 0, 78, 
	53, 72, 
	9, 24, 63, 
	2, 80, 0, 1, 
	36, 58, 
	0, 60, 0, 
	109, 
	17, 40, 29, 
	4, 20, 1, 45, 0, 45, 0, 85, 
	55, 53, 
	13, 1, 29, 
	4, 20, 2, 85, 0, 21, 
	54, 76, 
	13, 1, 53, 
	4, 20, 2, 85, 0, 21, 
	54, 76, 
	8, 1, 53, 
	4, 20, 14, 85, 
	50, 
	4, 10, 36, 
	2, 58, 
	110, 
	9, 26, 0, 
	4, 21, 0, 56, 
	40, 55, 
	5, 1, 12, 
	4, 20, 
	116, 61, 
	8, 10, 7, 
	4, 20, 1, 5, 
	6, 
	14, 10, 7, 
	4, 20, 1, 4, 0, 4, 
	116, 117, 55, 
	8, 10, 7, 
	4, 20, 6, 4, 
	5, 
	14, 1, 38, 
	2, 86, 0, 94, 0, 86, 
	19, 72, 10, 
	19, 1, 38, 
	2, 94, 0, 94, 0, 95, 0, 92, 
	10, 19, 72, 53, 
	14, 35, 38, 
	2, 86, 0, 94, 0, 86, 
	19, 72, 10, 
	19, 35, 38, 
	2, 94, 0, 94, 0, 95, 0, 92, 
	10, 19, 72, 53, 
	5, 1, 62, 
	2, 95, 
	118, 34, 
	13, 1, 62, 
	2, 92, 14, 95, 0, 22, 
	54, 76, 
	20, 14, 66, 
	4, 21, 14, 86, 14, 94, 14, 95, 14, 93, 
	73, 
	197, 
	0, 93, 
	1, 53, 
	5, 14, 66, 
	4, 21, 
	118, 34, 
	23, 40, 58, 
	1, 90, 4, 20, 0, 90, 0, 95, 0, 94, 
	55, 55, 55, 73, 
	198, 
	0, 17, 
	58, 131, 121, 
	4, 12, 67, 
	4, 21, 
	119, 
	4, 10, 9, 
	2, 15, 
	110, 
	5, 44, 7, 
	4, 20, 
	116, 61, 
	22, 10, 58, 
	2, 90, 2, 94, 0, 90, 0, 95, 0, 94, 
	52, 72, 10, 
	22, 10, 58, 
	2, 90, 2, 86, 0, 90, 0, 95, 0, 86, 
	52, 72, 10, 
	13, 1, 68, 
	4, 22, 2, 98, 0, 23, 
	54, 76, 
	16, 41, 68, 
	4, 22, 1, 55, 0, 100, 0, 98, 
	72, 
	16, 14, 68, 
	4, 22, 1, 55, 0, 100, 0, 98, 
	72, 
	20, 14, 71, 
	2, 99, 1, 19, 0, 99, 0, 44, 1, 12, 
	72, 
	13, 14, 71, 
	2, 99, 6, 19, 1, 12, 
	12, 61, 
	12, 42, 34, 
	1, 51, 0, 51, 0, 19, 
	72, 
	12, 58, 34, 
	1, 19, 0, 51, 0, 19, 
	72, 
	9, 1, 10, 
	2, 39, 0, 25, 
	54, 131, 
	8, 12, 46, 
	3, 1, 7, 15, 
	107, 
	0, 1, 80, 
	4, 
	0, 34, 0, 
	35, 
	13, 6, 7, 
	1, 5, 0, 5, 0, 4, 
	131, 72, 
	4, 27, 0, 
	1, 11, 
	122, 
	0, 45, 75, 
	71, 
	21, 35, 38, 
	4, 21, 14, 86, 14, 95, 14, 94, 0, 92, 
	53, 1, 
	21, 1, 38, 
	4, 21, 14, 86, 14, 95, 14, 94, 0, 92, 
	53, 1, 
	13, 30, 71, 
	2, 44, 14, 97, 0, 97, 
	53, 1, 
	5, 13, 64, 
	4, 25, 
	137, 61, 
	13, 1, 23, 
	2, 37, 5, 29, 0, 10, 
	54, 76, 
	13, 6, 7, 
	1, 5, 0, 4, 0, 5, 
	72, 10, 
	5, 9, 0, 
	0, 20, 
	58, 123, 
	4, 10, 40, 
	2, 63, 
	124, 
	4, 10, 40, 
	5, 63, 
	125, 
	9, 40, 58, 
	1, 90, 0, 90, 
	131, 53, 
	13, 75, 0, 
	14, 40, 4, 25, 0, 40, 
	138, 53, 
	9, 1, 62, 
	4, 7, 0, 8, 
	54, 76, 
	4, 1, 79, 
	4, 5, 
	4, 
	4, 1, 10, 
	4, 5, 
	4, 
	9, 10, 58, 
	2, 90, 0, 90, 
	52, 131, 
	22, 10, 20, 
	5, 29, 5, 32, 14, 37, 0, 37, 0, 30, 
	53, 74, 131, 
	5, 10, 20, 
	2, 32, 
	33, 34, 
	9, 1, 79, 
	2, 37, 2, 29, 
	33, 34, 
	13, 1, 79, 
	2, 37, 5, 29, 0, 10, 
	54, 76, 
	4, 30, 9, 
	2, 15, 
	9, 
	4, 30, 67, 
	4, 21, 
	119, 
	23, 24, 52, 
	2, 71, 4, 17, 0, 71, 0, 14, 0, 1, 
	62, 115, 58, 16, 
	23, 24, 52, 
	2, 71, 2, 71, 0, 71, 0, 17, 0, 1, 
	62, 115, 58, 16, 
	4, 50, 21, 
	2, 31, 
	136, 
	9, 40, 29, 
	1, 45, 0, 45, 
	53, 131, 
	13, 38, 14, 
	2, 22, 0, 22, 0, 20, 
	72, 131, 
	14, 30, 37, 
	3, 49, 14, 24, 0, 24, 
	1, 38, 53, 
	0, 10, 81, 
	135, 
	4, 49, 44, 
	1, 28, 
	133, 
	4, 30, 72, 
	3, 13, 
	43, 
	9, 40, 46, 
	1, 1, 0, 1, 
	53, 46, 
	4, 10, 68, 
	4, 11, 
	50, 
	18, 79, 10, 
	2, 38, 0, 38, 0, 39, 1, 19, 
	72, 131, 18, 
	1, 40, 0, 
	20, 85, 
	1, 30, 0, 
	122, 76, 
	9, 10, 32, 
	2, 42, 0, 42, 
	52, 131, 
	10, 59, 45, 
	1, 3, 0, 3, 
	131, 17, 55, 
	6, 24, 0, 
	0, 1, 
	131, 58, 115, 
	0, 53, 0, 
	46, 
	4, 61, 17, 
	2, 27, 
	22, 
	4, 14, 40, 
	2, 63, 
	104, 
	8, 12, 27, 
	4, 15, 12, 1, 
	107, 
	9, 12, 27, 
	4, 15, 3, 1, 
	131, 102, 
	9, 12, 46, 
	3, 1, 4, 15, 
	131, 102, 
	22, 75, 52, 
	2, 71, 3, 3, 0, 71, 0, 69, 0, 3, 
	131, 72, 55, 
	5, 44, 7, 
	4, 24, 
	131, 17, 
	4, 1, 74, 
	4, 9, 
	108, 
	23, 75, 40, 
	2, 63, 1, 3, 0, 3, 0, 10, 0, 64, 
	55, 58, 53, 103, 
	5, 10, 55, 
	2, 41, 
	4, 30, 
	17, 10, 20, 
	5, 29, 5, 32, 13, 37, 0, 30, 
	131, 74, 
	8, 41, 38, 
	1, 55, 2, 76, 
	42, 
	8, 41, 38, 
	1, 55, 2, 86, 
	42, 
	4, 70, 57, 
	2, 76, 
	42, 
	0, 61, 0, 
	46, 
	6, 35, 38, 
	2, 95, 
	131, 19, 127, 
	0, 6, 51, 
	109, 
	0, 38, 0, 
	4, 
	9, 10, 21, 
	2, 36, 0, 36, 
	52, 131, 
	9, 10, 68, 
	2, 67, 0, 67, 
	52, 131, 
	8, 14, 71, 
	2, 99, 6, 12, 
	47, 
	17, 72, 72, 
	3, 13, 0, 13, 0, 90, 3, 5, 
	72, 131, 
	17, 72, 72, 
	3, 13, 0, 13, 0, 90, 4, 24, 
	72, 131, 
	0, 13, 0, 
	4, 
	4, 79, 0, 
	6, 19, 
	2, 
	4, 22, 0, 
	2, 27, 
	2, 
	0, 75, 0, 
	35, 
	255,
};


const uint8_t verbs[] = {
65, 85, 84,
71, 79, 32,
197, 78, 84,
215, 65, 76,
210, 85, 78,
195, 76, 73,
80, 79, 85,
211, 80, 73,
197, 77, 80,
72, 69, 76,
71, 69, 84,
212, 65, 75,
82, 69, 65,
66, 82, 69,
85, 78, 76,
207, 80, 69,
83, 67, 79,
81, 85, 73,
68, 82, 79,
208, 85, 84,
204, 69, 65,
80, 69, 84,
65, 84, 84,
203, 73, 76,
83, 72, 79,
89, 69, 83,
66, 85, 82,
68, 73, 71,
76, 73, 71,
198, 76, 65,
76, 79, 79,
211, 69, 65,
80, 76, 65,
66, 65, 76,
82, 85, 66,
80, 85, 76,
199, 82, 65,
80, 76, 65,
67, 76, 79,
211, 72, 85,
84, 72, 82,
83, 65, 87,
87, 69, 65,
71, 76, 79,
68, 82, 73,
83, 65, 86,
73, 78, 86,
68, 82, 65,
46, 32, 32,
76, 79, 65,
83, 77, 79,
83, 87, 73,
74, 85, 77,
83, 72, 65,
174, 32, 32,
211, 77, 69,
208, 85, 83,
215, 65, 86,
82, 69, 77,
69, 65, 84,
87, 69, 84,
65, 83, 75,
212, 65, 76,
217, 69, 76,
211, 65, 89,
211, 67, 82,
203, 73, 67,
212, 73, 67,
46, 32, 32,
72, 85, 71,
85, 78, 67,
32, 32, 32,
67, 76, 69,
215, 65, 83,
32, 32, 32,
70, 69, 69,
212, 79, 85,
199, 82, 79,
32, 32, 32,
80, 85, 78,
200, 73, 84,
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
87, 65, 84,
204, 73, 81,
83, 73, 71,
68, 79, 79,
194, 82, 73,
80, 79, 79,
83, 75, 69,
83, 65, 82,
66, 65, 78,
73, 78, 86,
78, 79, 77,
66, 82, 65,
66, 79, 78,
84, 65, 80,
76, 69, 65,
77, 85, 77,
65, 76, 67,
83, 75, 85,
67, 82, 79,
70, 76, 79,
72, 73, 69,
84, 72, 82,
82, 79, 80,
80, 76, 65,
66, 76, 79,
66, 65, 83,
84, 69, 69,
71, 76, 79,
83, 65, 87,
76, 65, 68,
66, 79, 88,
67, 72, 65,
65, 82, 79,
79, 89, 83,
80, 69, 65,
70, 76, 85,
65, 82, 67,
80, 73, 83,
74, 69, 82,
83, 84, 79,
205, 65, 82,
80, 79, 76,
83, 72, 79,
78, 69, 67,
67, 65, 78,
82, 65, 84,
72, 79, 76,
67, 79, 66,
65, 83, 72,
75, 69, 89,
69, 88, 80,
82, 85, 66,
80, 79, 82,
80, 73, 78,
67, 65, 82,
83, 84, 65,
87, 79, 82,
77, 73, 82,
83, 65, 80,
67, 72, 69,
77, 85, 82,
66, 65, 82,
215, 73, 78,
46, 32, 32,
67, 79, 70,
67, 79, 65,
67, 79, 73,
80, 65, 83,
71, 65, 77,
84, 65, 66,
70, 76, 65,
83, 84, 65,
69, 78, 84,
80, 89, 82,
72, 69, 65,
83, 67, 65,
70, 73, 82,
65, 76, 84,
	0,
};
const uint8_t automap[] = {
83, 84, 79,
	1,
74, 69, 82,
	3,
67, 65, 78,
	4,
67, 65, 78,
	5,
70, 76, 65,
	8,
70, 76, 65,
	9,
83, 72, 79,
	11,
75, 69, 89,
	12,
67, 79, 65,
	13,
75, 69, 89,
	18,
66, 65, 78,
	21,
66, 79, 78,
	24,
80, 73, 83,
	28,
84, 65, 80,
	30,
83, 67, 65,
	34,
76, 69, 65,
	36,
67, 79, 73,
	40,
66, 65, 83,
	42,
82, 79, 80,
	45,
66, 76, 79,
	47,
83, 75, 85,
	48,
66, 79, 88,
	49,
84, 69, 69,
	50,
71, 76, 79,
	51,
83, 65, 87,
	55,
84, 65, 66,
	60,
80, 69, 65,
	64,
70, 76, 85,
	65,
66, 65, 82,
	67,
78, 69, 67,
	68,
82, 65, 84,
	69,
82, 65, 84,
	71,
78, 69, 67,
	74,
80, 73, 78,
	81,
67, 65, 82,
	82,
83, 65, 80,
	83,
82, 85, 66,
	90,
67, 82, 79,
	93,
66, 82, 65,
	97,
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
