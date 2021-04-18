// C Source File
// Created 2021-1-17; 23:27:20

#include <tigcclib.h>
#include "font.h"

void Text3x5(char Px, char Py, const char *Txt, char *Buf)
{
	if (Px >= 40) Px = 0;
	if (Py >= 95) return;
	char SPx = Px;
	
	char *Ptr = Buf + Py*30 + (Px >> 1);
	while (*Txt)
	{
		if (*Txt == 0) break;
		
		char Chr = *Txt;
		if (Chr == '\n' || Chr == '\r')
		{
			Px = SPx;
			Py ++ ;
			
			Ptr = Buf + Py*30 + (Px >> 1);
			
			Txt ++ ;
			continue;
		}
		if (Chr > 0x20)
		{
			Chr = Chr - 0x20;
			
			if (Chr >= 0x5B) Chr -= 0x1A;
			else if (Chr >= 0x40) Chr -= 0x20;
		}
		else Chr = 0;
		
		char SftChr = (Chr & 1) << 2;
		char SftOut = (Px & 1) << 2;
		
		Chr >>= 1;
		char *FontData = GlacFontData + ((Chr << 2) + Chr);
		
		char i = 0;
		while (i < 5)
		{
			*Ptr |= (((*FontData++) << SftChr) & 0xF0) >> SftOut;
			Ptr += 30;
			
			i ++ ;
		}
		Ptr -= 150 - (Px & 1);
		
		Px ++ ;
		if (Px >= 40)
		{
			Px = 0;
			Py += 5;
			Ptr += 10;
		}
		if (Py >= 95) break;
			
		Txt ++ ;
	}
}