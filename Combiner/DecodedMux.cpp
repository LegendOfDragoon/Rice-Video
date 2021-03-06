/*
Copyright (C) 2002-2009 Rice1964

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "..\stdafx.h"

static const uint8 sc_Mux32[32] = 
{
	MUX_COMBINED, MUX_TEXEL0,   MUX_TEXEL1, MUX_PRIM,
	MUX_SHADE,    MUX_ENV,      MUX_1,      MUX_COMBINED|MUX_ALPHAREPLICATE,
	MUX_TEXEL0|MUX_ALPHAREPLICATE, MUX_TEXEL1|MUX_ALPHAREPLICATE, MUX_PRIM|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE,
	MUX_ENV|MUX_ALPHAREPLICATE, MUX_LODFRAC, MUX_PRIMLODFRAC, MUX_K5,		// Actually k5
	MUX_UNK, MUX_UNK, MUX_UNK, MUX_UNK,
	MUX_UNK, MUX_UNK, MUX_UNK, MUX_UNK,
	MUX_UNK, MUX_UNK, MUX_UNK, MUX_UNK,
	MUX_UNK, MUX_UNK, MUX_UNK, MUX_0
};

static const uint8 sc_Mux16[16] = 
{
	MUX_COMBINED, MUX_TEXEL0,   MUX_TEXEL1, MUX_PRIM,
	MUX_SHADE,    MUX_ENV,      MUX_1,      MUX_COMBALPHA,
	MUX_TEXEL0|MUX_ALPHAREPLICATE, MUX_TEXEL1|MUX_ALPHAREPLICATE, MUX_PRIM|MUX_ALPHAREPLICATE, MUX_SHADE|MUX_ALPHAREPLICATE,
	MUX_ENV|MUX_ALPHAREPLICATE, MUX_LODFRAC, MUX_PRIMLODFRAC, MUX_0	
};
static const uint8 sc_Mux8[8] = 
{
	MUX_COMBINED, MUX_TEXEL0,   MUX_TEXEL1, MUX_PRIM,
	MUX_SHADE,    MUX_ENV,      MUX_1,      MUX_0
};

void DecodedMux::Decode(uint32 dwMux0, uint32 dwMux1)
{
	m_dwMux0 = dwMux0;
	m_dwMux1 = dwMux1;

	aRGB0  = uint8((dwMux0>>20)&0x0F);	// c1 c1		// a0
	bRGB0  = uint8((dwMux1>>28)&0x0F);	// c1 c2		// b0
	cRGB0  = uint8((dwMux0>>15)&0x1F);	// c1 c3		// c0
	dRGB0  = uint8((dwMux1>>15)&0x07);	// c1 c4		// d0
	
	aA0    = uint8((dwMux0>>12)&0x07);	// c1 a1		// Aa0
	bA0    = uint8((dwMux1>>12)&0x07);	// c1 a2		// Ab0
	cA0    = uint8((dwMux0>>9 )&0x07);	// c1 a3		// Ac0
	dA0    = uint8((dwMux1>>9 )&0x07);	// c1 a4		// Ad0
	
	aRGB1  = uint8((dwMux0>>5 )&0x0F);	// c2 c1		// a1
	bRGB1  = uint8((dwMux1>>24)&0x0F);	// c2 c2		// b1
	cRGB1  = uint8((dwMux0    )&0x1F);	// c2 c3		// c1
	dRGB1  = uint8((dwMux1>>6 )&0x07);	// c2 c4		// d1
	
	aA1    = uint8((dwMux1>>21)&0x07);	// c2 a1		// Aa1
	bA1    = uint8((dwMux1>>3 )&0x07);	// c2 a2		// Ab1
	cA1    = uint8((dwMux1>>18)&0x07);	// c2 a3		// Ac1
	dA1    = uint8((dwMux1    )&0x07);	// c2 a4		// Ad1

	//This fuction will translate the decode mux info further, so we can use
	//the decode data better in D3D.
	//Will translate A,B,C,D to unified presentation
	aRGB0  = sc_Mux16[aRGB0];
	bRGB0  = sc_Mux16[bRGB0];
	cRGB0  = sc_Mux32[cRGB0];
	dRGB0  = sc_Mux8[dRGB0];
	
	aA0    = sc_Mux8[aA0];
	bA0    = sc_Mux8[bA0];
	cA0    = sc_Mux8[cA0];
	dA0    = sc_Mux8[dA0];
	
	aRGB1  = sc_Mux16[aRGB1];
	bRGB1  = sc_Mux16[bRGB1];
	cRGB1  = sc_Mux32[cRGB1];
	dRGB1  = sc_Mux8[dRGB1];
	
	aA1    = sc_Mux8[aA1];
	bA1    = sc_Mux8[bA1];
	cA1    = sc_Mux8[cA1];
	dA1    = sc_Mux8[dA1];

	m_bTexel0IsUsed = isUsed(MUX_TEXEL0);
	m_bTexel1IsUsed = isUsed(MUX_TEXEL1);
}

bool DecodedMux::isUsed(uint8 val, uint8 mask)
{
	uint8* pmux = m_bytes;
	bool isUsed = false;
	for( int i=0; i<16; i++ )
	{
		if( (pmux[i]&mask) == (val&mask) )
		{
			isUsed = true;
			break;
		}
	}
	return isUsed;
}


bool DecodedMux::isUsedInCycle(uint8 val, int cycle, CombineChannel channel, uint8 mask)
{
	cycle *=2;
	if( channel == ALPHA_CHANNEL ) cycle++;

	uint8* pmux = m_bytes;
	for( int i=0; i<4; i++ )
	{
		if( (pmux[i+cycle*4]&mask) == (val&mask) )
		{
			return true;
		}
	}

	return false;
}

bool DecodedMux::isUsedInCycle(uint8 val, int cycle, uint8 mask)
{
	return isUsedInCycle(val, cycle/2, cycle%2?ALPHA_CHANNEL:COLOR_CHANNEL, mask);
}

void DecodedMux::Simplify(void)
{
	CheckCombineInCycle1();

	if( g_curRomInfo.bTexture1Hack )
	{
		ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,2);
		ReplaceVal(MUX_TEXEL1,MUX_TEXEL0,3);
	}

	m_bTexel0IsUsed = isUsed(MUX_TEXEL0);
	m_bTexel1IsUsed = isUsed(MUX_TEXEL1);
}

void DecodedMux::ReplaceVal(uint8 val1, uint8 val2, int cycle, uint8 mask)
{
	int start = 0;
	int end = 16;

	if( cycle >= 0 )
	{
		start = cycle*4;
		end = start+4;
	}

	uint8* pmux = m_bytes;
	for( int i=start; i<end; i++ )
	{
		if( (pmux[i]&mask) == (val1&mask) )
		{
			pmux[i] &= (~mask);
			pmux[i] |= val2;
		}
	}
}

#ifdef _DEBUG
void DecodedMux::DisplayMuxString(const char *prompt)
{
	DebuggerAppendMsg("//Mux=0x%08x%08x\t%s in %s\n", m_dwMux0, m_dwMux1, prompt, g_curRomInfo.szGameName);
//	Display(false);
	TRACE0("\n");
}

void DecodedMux::DisplaySimpliedMuxString(const char *prompt)
{
	DebuggerAppendMsg("//Simplied Mux=0x%08x%08x\t%s in %s\n", m_dwMux0, m_dwMux1, prompt, g_curRomInfo.szGameName);
//	Display(true);

	TRACE0("\n");
}

void DecodedMux::DisplayConstantsWithShade(uint32 flag,CombineChannel channel)
{
	DebuggerAppendMsg("Shade = %08X in %s channel",flag,channel==COLOR_CHANNEL?"color":"alpha");
}
#endif

void DecodedMux::CheckCombineInCycle1(void)
{
	if( isUsedInCycle(MUX_COMBINED,0,COLOR_CHANNEL) )
	{
		ReplaceVal(MUX_COMBINED, MUX_SHADE, 0);
	}

	if( isUsedInCycle(MUX_COMBALPHA,0,COLOR_CHANNEL) )
	{
		ReplaceVal(MUX_COMBALPHA, MUX_SHADE|MUX_ALPHAREPLICATE, 0);
	}

	if( isUsedInCycle(MUX_COMBINED,0,ALPHA_CHANNEL) )
	{
		if( cA0 == MUX_COMBINED && cRGB0 == MUX_LODFRAC && bRGB0 == dRGB0 && bA0 == dA0 )
		{
			cA0 = MUX_LODFRAC;
		}
		else
		{
			ReplaceVal(MUX_COMBINED, MUX_SHADE, 1);
		}
	}
	if( isUsedInCycle(MUX_COMBALPHA,0,ALPHA_CHANNEL) )
	{
		ReplaceVal(MUX_COMBALPHA, MUX_SHADE, 1);
	}
}

void DecodedMux::Hack(void)
{
	if( options.enableHackForGames == HACK_FOR_TONYHAWK )
	{
		if( gRSP.curTile == 1 )
		{
			ReplaceVal(MUX_TEXEL1, MUX_TEXEL0);
		}
	}
	else if( options.enableHackForGames == HACK_FOR_ZELDA || options.enableHackForGames == HACK_FOR_ZELDA_MM)
	{
		if( m_dwMux1 == 0xfffd9238 && m_dwMux0 == 0x00ffadff )
		{
			ReplaceVal(MUX_TEXEL1, MUX_TEXEL0);
		}
		else if( m_dwMux1 == 0xff5bfff8 && m_dwMux0 == 0x00121603 )
		{
			// The Zelda road trace
			ReplaceVal(MUX_TEXEL1, MUX_0);
		}
	}
	else if( options.enableHackForGames == HACK_FOR_MARIO_TENNIS )
	{
		if( m_dwMux1 == 0xffebdbc0 && m_dwMux0 == 0x00ffb9ff )
		{
			// Player shadow
			//m_decodedMux.dRGB0 = MUX_TEXEL0;
			//m_decodedMux.dRGB1 = MUX_COMBINED;
			cA1 = MUX_TEXEL0;
		}
	}
	else if( options.enableHackForGames == HACK_FOR_MARIO_GOLF )
	{
		// Hack for Mario Golf
		if( m_dwMux1 == 0xf1ffca7e || m_dwMux0 == 0x00115407 )
		{
			// The grass
			ReplaceVal(MUX_TEXEL0, MUX_TEXEL1);
		}
	}
	else if( options.enableHackForGames == HACK_FOR_TOPGEARRALLY )
	{
		//Mux=0x00317e025ffef3fa	Used in TOP GEAR RALLY
		//Color0: (PRIM - ENV) * TEXEL1 + ENV
		//Color1: (COMBINED - 0) * TEXEL1 + 0
		//Alpha0: (0 - 0) * 0 + TEXEL0
		//Alpha1: (0 - 0) * 0 + TEXEL1
		if( m_dwMux1 == 0x5ffef3fa || m_dwMux0 == 0x00317e02 )
		{
			// The grass
			//ReplaceVal(MUX_TEXEL0, MUX_TEXEL1);
			dA1 = MUX_COMBINED;
			//aA1 = MUX_COMBINED;
			//cA1 = MUX_TEXEL1;
			//dA1 = MUX_0;
			cRGB1 = MUX_TEXEL0;
		}
	}
}
