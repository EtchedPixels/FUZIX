/**
 * plato_key.h
 * PLATO specific key mappings
 * used by key.h
 * this define table is ordered as in s0ascers document.
 */

#ifndef PLATO_KEY_H
#define PLATO_KEY_H

#define PKEY_a 0x41
#define PKEY_A 0x61
#define PKEY_b 0x42
#define PKEY_B 0x62
#define PKEY_c 0x43
#define PKEY_C 0x63
#define PKEY_d 0x44
#define PKEY_D 0x64
#define PKEY_e 0x45
#define PKEY_E 0x65
#define PKEY_f 0x46
#define PKEY_F 0x66
#define PKEY_g 0x47
#define PKEY_G 0x67
#define PKEY_h 0x48
#define PKEY_H 0x68
#define PKEY_i 0x49
#define PKEY_I 0x69
#define PKEY_j 0x4a
#define PKEY_J 0x6a
#define PKEY_k 0x4b
#define PKEY_K 0x6b
#define PKEY_l 0x4c
#define PKEY_L 0x6C
#define PKEY_m 0x4d
#define PKEY_M 0x6d
#define PKEY_n 0x4e
#define PKEY_N 0x6e
#define PKEY_o 0x4f
#define PKEY_O 0x6f
#define PKEY_p 0x50
#define PKEY_P 0x70
#define PKEY_q 0x51
#define PKEY_Q 0x71
#define PKEY_r 0x52
#define PKEY_R 0x72
#define PKEY_s 0x53
#define PKEY_S 0x73
#define PKEY_t 0x54
#define PKEY_T 0x74
#define PKEY_u 0x55
#define PKEY_U 0x75
#define PKEY_v 0x56
#define PKEY_V 0x76
#define PKEY_w 0x57
#define PKEY_W 0x77
#define PKEY_x 0x58
#define PKEY_X 0x78
#define PKEY_y 0x59
#define PKEY_Y 0x79
#define PKEY_z 0x5a
#define PKEY_Z 0x7A
#define PKEY_0 0x00
#define PKEY_LESS_THAN 0x20
#define PKEY_1 0x01
#define PKEY_GREATER_THAN 0x21
#define PKEY_2 0x02
#define PKEY_BRACKET_LEFT 0x22
#define PKEY_3 0x03
#define PKEY_BRACKET_RIGHT 0x23
#define PKEY_4 0x04
#define PKEY_DOLLAR 0x24
#define PKEY_5 0x05
#define PKEY_PERCENT 0x25
#define PKEY_6 0x06
#define PKEY_UNDERSCORE 0x26
#define PKEY_7 0x07
#define PKEY_APOSTROPHE 0x27
#define PKEY_8 0x08
#define PKEY_ASTERISK 0x28
#define PKEY_9 0x09
#define PKEY_PARENTHESIS_LEFT 0x29
#define PKEY_EQUALS 0x5B
#define PKEY_PARENTHESIS_RIGHT 0x7B
#define PKEY_PLUS 0x0E
#define PKEY_SIGMA 0x2E
#define PKEY_ASSIGN 0x0D
#define PKEY_SHIFT 0x2D
#define PKEY_MINUS 0x0F
#define PKEY_DELTA 0x2F
#define PKEY_DIVIDE 0x0B
#define PKEY_INTERSECT 0x2B
#define PKEY_MULTIPLY 0x0A
#define PKEY_UNION 0x2A
#define PKEY_SEMICOLON 0x5c
#define PKEY_COLON 0x7c
#define PKEY_PERIOD 0x5e
#define PKEY_EXCLAMATION 0x7e
#define PKEY_COMMA 0x5f
#define PKEY_QUOTE 0x7f
#define PKEY_SLASH 0x5d
#define PKEY_QUESTION_MARK 0x7d
#define PKEY_SUPER 0x10
#define PKEY_SUPER1 0x30
#define PKEY_SUB 0x11
#define PKEY_SUB1 0x31
#define PKEY_ANS 0x12
#define PKEY_TERM 0x32
#define PKEY_COPY 0x1B
#define PKEY_COPY1 0x3B
#define PKEY_TAB 0x0c
#define PKEY_CR 0x2c
#define PKEY_ERASE 0x13
#define PKEY_ERASE1 0x33
#define PKEY_MICRO 0x14
#define PKEY_FONT 0x34
#define PKEY_HELP 0x15
#define PKEY_HELP1 0x35
#define PKEY_SQUARE 0x1C
#define PKEY_ACCESS 0x3C
#define PKEY_NEXT 0x16
#define PKEY_NEXT1 0x36
#define PKEY_EDIT 0x17
#define PKEY_EDIT1 0x37
#define PKEY_BACK 0x18
#define PKEY_BACK1 0x38
#define PKEY_LAB 0x1D
#define PKEY_LAB1 0x3D
#define PKEY_DATA 0x19
#define PKEY_DATA1 0x39
#define PKEY_STOP 0x1a
#define PKEY_STOP1 0x3a
#define PKEY_SPACE 0x40
#define PKEY_BACKSPACE 0x60
#define PKEY_PRINT 0x1F
#define PKEY_PRINT1 0x3F
#define PKEY_NOKEY 0xFF /* no key mapping */

/* The following keys require an ACCESS key combination */
#define PKEY_ALPHA 0x80
#define PKEY_BETA 0x81
#define PKEY_CEDILLA 0x82
#define PKEY_LOWERCASE_DELTA 0x83
#define PKEY_ACUTE_ACCENT 0x84
#define PKEY_LOWERCASE_AE 0x85
#define PKEY_LOWERCASE_OE 0x86
#define PKEY_LOWERCASE_A_WITH_RING 0x87
#define PKEY_LOWERCASE_A_WITH_DIAERESIS 0x88
#define PKEY_LAMBDA 0x89
#define PKEY_MU 0x8A
#define PKEY_TILDE 0x8B
#define PKEY_DEGREE 0x8C
#define PKEY_PI 0x8D
#define PKEY_GRAVE 0x8E
#define PKEY_RHO 0x8F
#define PKEY_LOWERCASE_SIGMA 0x90
#define PKEY_THETA 0x91
#define PKEY_DIARESIS 0x92
#define PKEY_HACEK 0x93
#define PKEY_CAPITAL_PI 0x94
#define PKEY_CIRCUMFLEX 0x95
#define PKEY_LEFT_EMBED 0x96
#define PKEY_RIGHT_EMBED 0x97
#define PKEY_AT 0x98
#define PKEY_ARROW 0x99
#define PKEY_AMPERSAND 0x9A
#define PKEY_INTERPUNCT 0x9B
#define PKEY_LOWER_TILDE 0x9C
#define PKEY_DELIMITER 0x9D
#define PKEY_BACKSLASH 0x9E
#define PKEY_NOT_EQUAL 0x9F
#define PKEY_LOWERCASE_O_WITH_DIARESIS 0xA0
#define PKEY_LEFT_ARROW 0xA1
#define PKEY_DOWN_ARROW 0xA2
#define PKEY_RIGHT_ARROW 0xA3
#define PKEY_UP_ARROW 0xA4
#define PKEY_COPYRIGHT 0xA5
#define PKEY_DIAMOND 0xA6
#define PKEY_UPPERCASE_AE 0xA7
#define PKEY_UPPERCASE_OE 0xA8
#define PKEY_BAR 0xA9
#define PKEY_UPPERCASE_A_WITH_RING 0xAA
#define PKEY_UPPERCASE_A_WITH_DIAERESIS 0xAB
#define PKEY_ACCESS_SQUARE 0xAC
#define PKEY_UPPERCASE_O_WITH_DIARESIS 0xAD
#define PKEY_LESS_THAN_OR_EQUAL 0xAE
#define PKEY_GREATER_THAN_OR_EQUAL 0xAF
#define PKEY_LEFT_CURLY_BRACE 0xB0
#define PKEY_RIGHT_CURLY_BRACE 0xB1
#define PKEY_POUND 0xB2
#define PKEY_BIG_CROSS 0xB3
#define PKEY_EQUIVALENT 0xB4

/* ACCESS Key combinations. */
static uint8_t ACCESS_KEYS[] = {
  PKEY_a, /* 0x80 a ɑ alpha */ 
  PKEY_B, /* 0x81 b ß beta */
  PKEY_c, /* 0x82 c cedilla */
  PKEY_d, /* 0x83 d δ delta */
  PKEY_e, /* 0x84 e ' acute accent */
  PKEY_g, /* 0x85 g æ ae */
  PKEY_h, /* 0x86 h oe oe */
  PKEY_j, /* 0x87 j å a with ring */
  PKEY_k, /* 0x88 k ä a with diaeresis */
  PKEY_l, /* 0x89 l ƛ lambda */
  PKEY_m, /* 0x8A m μ mu */
  PKEY_n, /* 0x8B n ~ tilde */
  PKEY_o, /* 0x8C o ° degree */
  PKEY_p, /* 0x8D p π pi */
  0x51, /* 0x8E q ` grave */
  PKEY_r, /* 0x8F r ρ rho */
  PKEY_s, /* 0x90 s σ sigma */
  PKEY_t, /* 0x91 t θ theta */
  PKEY_u, /* 0x92 u ¨ diaeresis */
  PKEY_v, /* 0x93 v hacek (upside down circumflex) */
  PKEY_w, /* 0x94 w ϖ capital pi */
  PKEY_x, /* 0x95 x ^ circumflex */
  PKEY_0, /* 0x96 0 l-embed */
  PKEY_1, /* 0x97 1 r-embed */
  PKEY_5, /* 0x98 5 @ */
  PKEY_6, /* 0x99 6 arrow */
  PKEY_PLUS, /* 0x9a + & */
  0x26, /* 0x9b & interpunct */
  PKEY_COLON, /* 0x9c : ~ lower tilde */
  0x5f, /* 0x9d , delimiter */
  PKEY_SLASH, /* 0x9e / \ */
  PKEY_EQUALS, /* 0x9f = not equal */
  PKEY_y, /* 0xA0 y ö */
  0x61, /* 0xA1 A left arrow */
  0x78, /* 0xA2 X down arrow */
  0x64, /* 0xA3 D right arrow */
  0x77, /* 0xA4 W up arrow */
  0x63, /* 0xA5 C © */
  0x66, /* 0xA6 F ♦ */
  0x67, /* 0xA7 G Æ */
  0x68, /* 0xA8 H OE */
  0x69, /* 0xA9 I | */
  0x6A, /* 0xAA J Å */
  0x6B, /* 0xAB K Ä */
  0x6F, /* 0xAC O SQUARE */
  0x79, /* 0xAD Y Ö */
  0x20, /* 0xAE < ≤ */
  0x21, /* 0xAF > ≥ */
  0x5B, /* 0xB0 [ { */
  PKEY_SLASH, /* 0xB1 ] } */
  0x24, /* 0xB2 $ # */
  0x9a, /* 0xB3 & big cross */
  0x7B  /* 0xB4 EQUIVALENT */
};

#endif /* PLATO_KEY_H */

