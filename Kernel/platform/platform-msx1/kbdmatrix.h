#ifndef __KBDMATRIX_DOT_H__
#define __KBDMATRIX_DOT_H__
/*
 * International key matrix
 */
const uint8_t keyboard_int[11][8] = {
	{'0','1','2', '3','4','5','6','7'},
	{'8','9','-','=','\\','[',']',';'},
	{ 39, '`', ',', '.','/',' ','a','b'},
	{'c','d','e', 'f','g','h','i','j'},
	{'k','l','m', 'n','o','p','q','r'},
	{'s','t','u', 'v','w','x','y','z'},
	{0/*SHIFT*/,0/*CTRL*/,0/*GRPH*/,0/*CAPS*/,0/*CODE*/ , KEY_F1 , KEY_F2 , KEY_F3 },
	{KEY_F4 , KEY_F5,  KEY_ESC , '\t', KEY_STOP ,KEY_BS , 0 , 13},
	{32 , KEY_HOME,  KEY_INSERT , KEY_DEL, KEY_LEFT , KEY_UP , KEY_DOWN , KEY_RIGHT},
	{'*','+','/','0','1' ,'2','3','4'},
	{'5','6','7','8','9' ,'-',',','.'}
};

const uint8_t shiftkeyboard_int[11][8] = {
	{')','!','@', '#','$','%','^','&'},
	{'*','(','_','+','|','{','}',':'},
	{'"','~','<','>','?',' ','A','B'},
	{'C','D','E', 'F','G','H','I','J'},
	{'K','L','M', 'N','O','P','Q','R'},
	{'S','T','U', 'V','W','X','Y','Z'},
	{0/*SHIFT*/,0/*CTRL*/,0/*GRPH*/,0/*CAPS*/,0/*CODE*/, KEY_F1 , KEY_F2 , KEY_F3 },
	{KEY_F4 , KEY_F5,  KEY_ESC , '\t', KEY_STOP ,KEY_BS , 0/*SELECT*/ , 13},
	{32 ,KEY_HOME,  KEY_INSERT , KEY_DEL, KEY_LEFT , KEY_UP , KEY_DOWN , KEY_RIGHT},
	{'*','+','/','0','1' ,'2','3','4'},
	{'5','6','7','8','9' ,'-',',','.'}
};

/*
 * Japan overlay
 */
const uint8_t keyboard_jp[3][8] = {
	{'0','1','2', '3','4','5','6','7'},
	{'8','9','-','^',KEY_YEN,'@','[',';'},
	{':',']', ',', '.','/',' ','a','b'}};

const uint8_t shiftkeyboard_jp[3][8] = {
	{' ','!','"', '#','$','%','&',39},
	{'(',')','=','~','|','`','{','+'},
	{'*','}','<','>','?','_','A','B'}};
/*
 * UK overlay
 */
const uint8_t shiftkeyboard_uk[1][8] = {
	{39,'`', ',', '.','/',KEY_POUND,'A','B'}};  /* row 2 */

/*
 * Spanish overlay
 */
const uint8_t keyboard_es[2][8] = {
	{'8','9','-','=','\\','[',']', 'N'/* Ñ */}, /* row 1 */
	{39, ':', ',', '.','/',' ','a','b'}};

const uint8_t shiftkeyboard_es[2][8] = {
	{'*','(','_','+','|','{','}', 'n' /* ñ */},  /* row 1 */
	{'"',':','<','>','?',' ','A','B'}};


#endif
