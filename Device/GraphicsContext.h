/*
Copyright (C) 2003-2009 Rice1964

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

#ifndef _AFX_GFXCONTEXT_H_
#define _AFX_GFXCONTEXT_H_

#include "..\Utility\CritSect.h"

enum ClearFlag
{
	CLEAR_COLOR_BUFFER=0x01,
	CLEAR_DEPTH_BUFFER=0x02,
	CLEAR_COLOR_AND_DEPTH_BUFFER=0x03,
};

// This class basically provides an extra level of security for our
// multithreaded code. Threads can Grab the CGraphicsContext to prevent
// other threads from changing/releasing any of the pointers while it is
// running.

// It is based on CCritSect for Lock() and Unlock()

class CGraphicsContext : public CCritSect
{
	friend class CDeviceBuilder;
	
public:

	virtual ~CGraphicsContext();
	CGraphicsContext();

	bool Ready() { return m_bReady; }
	bool IsWindowed() {
		return m_bWindowed;
	}

	virtual bool Initialize(HWND hWnd, HWND hWndStatus, uint32 dwWidth, uint32 dwHeight, BOOL bWindowed );
	virtual void CleanUp();

	virtual void Clear(ClearFlag flags, uint32 color=0xFF000000, float depth=1.0f) = 0;
	virtual void UpdateFrame(bool swaponly=false) = 0;
	virtual bool ToggleFullscreen()=0;		// return false as the result is windowed

	static void InitWindowInfo();
	static void InitDeviceParameters();

public:
	static	HWND		m_hWnd;
	static	HWND		m_hWndStatus;
	static	HMENU		m_hMenu;
	static  int			m_FullScreenResolutions[40][2];
	static  int			m_numOfResolutions;

protected:
    static	uint32      m_dwWindowStyle;     // Saved window style for mode switches
    static	uint32      m_dwWindowExStyle;   // Saved window style for mode switches
	static	uint32      m_dwStatusWindowStyle;     // Saved window style for mode switches


	bool				m_bReady;
	bool				m_bActive;
	
	bool				m_bWindowed;
	RECT				m_rcWindowBounds;

	DEVMODEA			m_DMsaved;

public:
	static CGraphicsContext *g_pGraphicsContext;
	static CGraphicsContext * Get(void);
	static bool needCleanScene;
};

void OutputText(char *msg, RECT *prect, uint32 flag = DT_CENTER|DT_VCENTER );

#endif
