/**
 * PLATOTerm64 - A PLATO Terminal for the Commodore 64
 * Based on Steve Peltz's PAD
 * 
 * Author: Thomas Cherryhomes <thom.cherryhomes at gmail dot com>
 * This file written by Steve Peltz. Copyright notice preserved.
 * and this code has been used with permission, and can be considered
 * public domain.
 *
 * protocol.h - Protocol decoder functions 
 */

/* Copyright (c) 1990 by Steve Peltz */

#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

#define ACCESS	0x3c

typedef	unsigned char	padByte;
typedef	enum {padF, padT} padBool;
typedef	unsigned char	padChar;
typedef	short	padWord;
typedef	padWord	charData[8];
typedef	struct {
	padByte	red,
		green,
		blue;
	} padRGB;

typedef struct {
	padWord	x,
		y;
	} padPt;

typedef enum {ModeWrite, ModeErase, ModeRewrite, ModeInverse} DispMode;
typedef enum {M0, M1, M2, M3} CharMem;

typedef enum {	mBlock, mPoint, mLine, mAlpha, mLoadCoord, mLoadChar, mSSF,
		mExternal, mLoadMem, mMode5, mMode6, mMode7, mLoadAddr,
		mLoadEcho, mFore, mBack, mPaint } Mode;
 
typedef enum {	tByte, tWord, tCoord, tColor, tPaint } DataType;

extern	padPt		PLATOSize;
extern	CharMem		CurMem;
extern	padBool		TTY,
			FlowControl,
			ModeBold,
			Rotate,
			Reverse;
extern	DispMode	CurMode;

void InitPAD(void);
void InitTTY(void);
void InitPLATO(void);
void InitPLATOx(void);
void Key(padWord theKey);
void Touch(padPt* where);
void Ext(padWord theKey);
void Echo(padWord theKey);
void SetCommand(Mode theMode, DataType theType);
void SetMode(Mode theMode, DataType theType);
void FixXY(int16_t DX, int16_t DY);
void Superx(void);
void Subx(void);
void Marginx(void);
void BSx(void);
void HTx(void);
void LFx(void);
void VTx(void);
void FFx(void);
void CRx(void);
void LoadCoordx(padPt* SetCoord);
void Blockx(void);
void Pointx(void);
void Linex(void);
void Alphax(void);
void LoadEchox(void);
void LoadAddrx(void);
void LoadCharx(void);
void LoadMemx(void);
void SSFx(void);
void Externalx(void);
void GoMode(void);
void GoWord(void);
void GoCoord(void);
void GoColor(void);
void GoPaint(void);
void DataChar(void);
void ShowPLATO(padByte* buff, uint16_t count);
void SetFast(void);

#endif
