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


//static BOOL g_bHiliteRGBAHack = FALSE;

#ifdef _DEBUG
char *cycleTypeStrs[] = {
	"1 Cycle",
	"2 Cycle",
	"Copy Mode",
	"Fill Mode"
};
#endif

//========================================================================

inline IColor GetIColor(uint8 flag, uint32 curCol)
{
	IColor newc;
	switch(flag&MUX_MASK)
	{
	case MUX_0:
		newc = 0;
		break;
	case MUX_1:
		newc = 0xFFFFFFFF;
		break;
	case MUX_PRIM:
		newc = gRDP.primitiveColor;
		break;
	case MUX_ENV:
		newc = gRDP.envColor;
		break;
	case MUX_COMBINED:
	case MUX_SHADE:
		newc = curCol;
		break;
	case MUX_K5:
		newc = 0xFFFFFFFF;
		break;
	case MUX_UNK:
		newc = curCol;
		if( options.enableHackForGames == HACK_FOR_CONKER )
			newc = 0xFFFFFFFF;
		break;
	default:
		newc = curCol;
		break;
	}

	if( flag&MUX_COMPLEMENT )
	{
		newc.Complement();
	}

	if( flag&MUX_ALPHAREPLICATE )
	{
		newc.AlphaReplicate();
	}

	return newc;
}

D3DCOLOR CalculateConstFactor(uint32 colorOp, uint32 alphaOp, uint32 curCol)
{
	N64CombinerType m;
	IColor color(curCol);
	IColor alpha(curCol);

	// For color channel
	*(uint32*)&m = colorOp;
	if( m.c != MUX_0 && m.a!=m.b)
	{
		if( m.a != MUX_0 )	color = GetIColor(m.a, curCol);
		if( m.b != MUX_0 )	color -= GetIColor(m.b, curCol);
		if( m.c != MUX_1 )	color *= GetIColor(m.c, curCol);
	}
	if( m.d != MUX_0 )	color += GetIColor(m.d, curCol);

	// For alpha channel
	*(uint32*)&m = alphaOp;
	if( m.c != MUX_0 && m.a!=m.b)
	{
		if( m.a != MUX_0 )	alpha = GetIColor(m.a, curCol);
		if( m.b != MUX_0 )	alpha -= GetIColor(m.b, curCol);
		if( m.c != MUX_1 )	alpha *= GetIColor(m.c, curCol);
	}
	if( m.d != MUX_0 )	alpha += GetIColor(m.d, curCol);

	return (D3DCOLOR)(((uint32)color&0x00FFFFFF)|((uint32)alpha&0xFF000000));
}


D3DCOLOR CColorCombiner::GetConstFactor(uint32 colorFlag, uint32	alphaFlag, uint32 defaultColor)
{
	// Allows a combine mode to select what TFACTOR should be
	uint32 color = defaultColor;
	uint32 alpha = defaultColor;

	switch (colorFlag&MUX_MASK)
	{
	case MUX_0:
		break;
	case MUX_FORCE_0:
		color = 0;
		break;
	case MUX_1:
		color = 0xFFFFFFFF;
		break;
	case MUX_PRIM:
		color = gRDP.primitiveColor;
		break;
	case MUX_ENV:
		color = gRDP.envColor;
		break;
	case MUX_LODFRAC:
		color = COLOR_RGBA(gRDP.LODFrac, gRDP.LODFrac, gRDP.LODFrac, gRDP.LODFrac);
		break;
	case MUX_PRIMLODFRAC:
		color = COLOR_RGBA(gRDP.primLODFrac, gRDP.primLODFrac, gRDP.primLODFrac, gRDP.primLODFrac);
		break;
	case MUX_PRIM_ALPHA:
		{
			IColor col(gRDP.primitiveColor);
			col.AlphaReplicate();
			color = (D3DCOLOR)col;
		}
		break;
	case MUX_ENV_ALPHA:
		{
			IColor col(gRDP.envColor);
			col.AlphaReplicate();
			color = (D3DCOLOR)col;
		}
		break;
	case MUX_K5:
		color = 0xFFFFFFFF;
		break;
	case MUX_UNK:
		color = defaultColor;
		if( options.enableHackForGames == HACK_FOR_CONKER )	color = 0xFFFFFFFF;
		break;
	default:
		color = defaultColor;
		break;
	}

	if( colorFlag & MUX_COMPLEMENT )
	{
		color = 0xFFFFFFFF - color;
	}
	if( colorFlag & MUX_ALPHAREPLICATE )
	{
		color = color>>24;
		color = color | (color<<8) | (color <<16) | (color<<24);
	}

	color &= 0x00FFFFFF;	// For color channel only, not the alpha channel


	switch (alphaFlag&MUX_MASK)
	{
	case MUX_0:
		break;
	case MUX_FORCE_0:
		alpha = 0;
		break;
	case MUX_1:
		alpha = 0xFFFFFFFF;
		break;
	case MUX_PRIM:
		alpha = gRDP.primitiveColor;
		break;
	case MUX_ENV:
		alpha = gRDP.envColor;
		break;
	case MUX_LODFRAC:
		alpha = COLOR_RGBA(gRDP.LODFrac, gRDP.LODFrac, gRDP.LODFrac, gRDP.LODFrac);
		break;
	case MUX_PRIMLODFRAC:
		alpha = COLOR_RGBA(gRDP.primLODFrac, gRDP.primLODFrac, gRDP.primLODFrac, gRDP.primLODFrac);
		break;
	case MUX_PRIM_ALPHA:
		{
			IColor col(gRDP.primitiveColor);
			col.AlphaReplicate();
			alpha = (D3DCOLOR)col;
		}
		break;
	case MUX_ENV_ALPHA:
		{
			IColor col(gRDP.envColor);
			col.AlphaReplicate();
			alpha = (D3DCOLOR)col;
		}
		break;
	default:
		alpha = defaultColor;
		break;
	}

	if( alphaFlag & MUX_COMPLEMENT )
	{
		alpha = 0xFFFFFFFF - alpha;
	}

	alpha &= 0xFF000000;

	return (color|alpha);
}

//========================================================================

void CColorCombiner::InitCombinerMode(void)
{
#ifdef _DEBUG
	LOG_UCODE(cycleTypeStrs[gRDP.otherMode.cycle_type]);
	if( debuggerDropDecodedMux )
	{
		UpdateCombiner(m_pDecodedMux->m_dwMux0, m_pDecodedMux->m_dwMux1);
	}
#endif

	if( gRDP.otherMode.cycle_type  == CYCLE_TYPE_COPY )
	{
		InitCombinerCycleCopy();
		m_bCycleChanged = true;
	}
	else if ( gRDP.otherMode.cycle_type == CYCLE_TYPE_FILL )
	{
		InitCombinerCycleFill();
		m_bCycleChanged = true;
	}
	else
	{
		InitCombinerCycle12();
		m_bCycleChanged = false;
	}
}


bool bConkerHideShadow=false;
void CColorCombiner::UpdateCombiner(uint32 dwMux0, uint32 dwMux1)
{
#ifdef _DEBUG
	if( debuggerDropDecodedMux )
	{
		debuggerDropDecodedMux = false;
		m_pDecodedMux->m_dwMux0 = m_pDecodedMux->m_dwMux1 = 0;
		m_DecodedMuxList.clear();
	}
#endif

	DecodedMux &m_decodedMux = *m_pDecodedMux;
	if( m_decodedMux.m_dwMux0 != dwMux0 || m_decodedMux.m_dwMux1 != dwMux1 )
	{
		if( options.enableHackForGames == HACK_FOR_DR_MARIO )
		{
			// Hack for Dr. Mario
			if( dwMux1 == 0xfffcf239 && 
				((m_decodedMux.m_dwMux0 == dwMux0 && dwMux0 == 0x00ffffff && 
				m_decodedMux.m_dwMux1 != dwMux1 && m_decodedMux.m_dwMux1 == 0xfffcf279 ) || 
				(m_decodedMux.m_dwMux0 == 0x00ffb3ff && m_decodedMux.m_dwMux1 == 0xff64fe7f && dwMux0 == 0x00ffffff ) ))
			{
				//dwMux1 = 0xffcf23A;
				dwMux1 = 0xfffcf438;
			}
		}
		uint64 mux64 = (((uint64)dwMux1)<<32)+dwMux0;
		int index=m_DecodedMuxList.find(mux64);

		if( options.enableHackForGames == HACK_FOR_CONKER )
		{
			// Conker's shadow, to disable the shadow
			//Mux=0x00ffe9ff	Used in CONKER BFD
			//Color0: (0 - 0) * 0 + SHADE
			//Color1: (0 - 0) * 0 + SHADE
			//Alpha0: (1 - TEXEL0) * SHADE + 0
			//Alpha1: (1 - TEXEL0) * SHADE + 0				
			if( dwMux1 == 0xffd21f0f && dwMux0 == 0x00ffe9ff )
			{
				bConkerHideShadow = true;
			}
			else
			{
				bConkerHideShadow = false;
			}
		}

		if( index >= 0 )
		{
			m_decodedMux = m_DecodedMuxList[index];
		}
		else
		{
			m_decodedMux.Decode(dwMux0, dwMux1);

			m_decodedMux.Hack();

			m_decodedMux.Simplify();
			
			m_DecodedMuxList.add(m_decodedMux.m_u64Mux, *m_pDecodedMux);
#ifdef _DEBUG
			if( logCombiners ) 
			{
				TRACE0("Add a new mux");
				DisplayMuxString();
			}
#endif
		}

		m_bTex0Enabled = m_decodedMux.m_bTexel0IsUsed;
		m_bTex1Enabled = m_decodedMux.m_bTexel1IsUsed;

		gRSP.bProcessDiffuseColor = (m_decodedMux.m_dwShadeColorChannelFlag != MUX_0 || m_decodedMux.m_dwShadeAlphaChannelFlag != MUX_0);
		gRSP.bProcessSpecularColor = false;
	}
}


#ifdef _DEBUG
void CColorCombiner::DisplayMuxString(void)
{
	if( gRDP.otherMode.cycle_type == CYCLE_TYPE_COPY)
	{
		TRACE0("COPY Mode\n");
	}	
	else if( gRDP.otherMode.cycle_type == CYCLE_TYPE_FILL)
	{
		TRACE0("FILL Mode\n");
	}

	m_pDecodedMux->DisplayMuxString("Used");
}

void CColorCombiner::DisplaySimpleMuxString(void)
{
	m_pDecodedMux->DisplaySimpliedMuxString("Used");
}
#endif
