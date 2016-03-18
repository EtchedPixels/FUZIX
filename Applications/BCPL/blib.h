/* Copyright (c) 2004 Robert Nordier.  All rights reserved. */

/* $Id: blib.h,v 1.3 2004/12/11 11:27:20 rn Exp $ */

#ifndef BLIB_H_
#define BLIB_H_

uint16_t getbyte(uint16_t, uint16_t);
void putbyte(uint16_t, uint16_t, uint16_t);
void initio(void);
int16_t findinput(uint16_t);
int16_t findoutput(uint16_t);
void selectinput(int16_t);
void selectoutput(int16_t);
int16_t input(void);
int16_t output(void);
int16_t rdch(void);
void wrch(int16_t);
void endread(void);
void endwrite(void);
void mapstore(void);

#endif
