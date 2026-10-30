#pragma once
#include "PrimeDebug.h"

#ifdef PRIMEDEBUG_DEV
#  include "Util/PList.h"

//-----------------------------------------------------------------------------
// Purpose: a value summed per frame and in total (Prime's crProfileCounter,
// crProfileCounterObject): times, counts... Counters that got a value this
// frame are on the active list until EndFrame, which keeps the frame's
// maximum. NextFrame counts the frames (Prime's kernel frame).
//-----------------------------------------------------------------------------
class PProfileCounter
{
public:
	typedef void ( *Callback )( void* a_pUserData, PProfileCounter* a_pCounter );

public:
	PProfileCounter( const TCHAR* a_szName, void* a_pUserData = TNULL );
	~PProfileCounter();

	void   Reset();
	void   Add( TFLOAT a_fValue );
	TFLOAT GetFrameTotal();
	TINT   GetFrameCount();
	TFLOAT GetTotal();
	TINT   GetTotalCount();
	TFLOAT GetFrameMax();
	TINT   GetFrameMaxCount();
	TFLOAT GetAverage();
	TFLOAT GetTotalPerFrame();
	TFLOAT GetCountPerFrame();
	TINT   GetNumFrames();

	const TCHAR* GetName() const { return m_szName; }
	void*        GetUserData() const { return m_pUserData; }

	static void  EndFrame();
	static void  ForEach( Callback a_fnCallback, void* a_pUserData );
	static void  NextFrame() { ms_uiFrame++; }
	static TUINT GetFrame() { return ms_uiFrame; }

private:
	void Create( const TCHAR* a_szName, void* a_pUserData );

private:
	PListNode    m_oNode;          // 0x00
	const TCHAR* m_szName;         // 0x08
	void*        m_pUserData;      // 0x0C
	TUINT        m_uiStartFrame;   // 0x10
	TFLOAT       m_fFrameTotal;    // 0x14
	TINT         m_iFrameCount;    // 0x18
	TFLOAT       m_fTotal;         // 0x1C
	TINT         m_iTotalCount;    // 0x20
	TFLOAT       m_fFrameMax;      // 0x24
	TINT         m_iFrameMaxCount; // 0x28

	static PListNode ms_oCounters;
	static PListNode ms_oActiveCounters;
	static TUINT     ms_uiFrame;
};

//-----------------------------------------------------------------------------
// Purpose: warnings kept once each (by text), with a count and the frame of
// the last one (Prime's crWarning)
//-----------------------------------------------------------------------------
class PWarning
{
public:
	static constexpr TINT MAX_WARNINGS = 256;
	static constexpr TINT TEXT_SIZE    = 256;

	typedef void ( *Callback )( void* a_pUserData, const TCHAR* a_szText, TINT a_iCount, TUINT a_uiLastFrame );

public:
	static void Add( const TCHAR* a_szFormat, ... );
	static void ForEach( Callback a_fnCallback, void* a_pUserData );
	static TINT GetNumWarnings() { return ms_iNumWarnings; }

private:
	struct Warning
	{
		TUINT32 uiHash;      // 0x000, 0: free
		TINT    iCount;      // 0x004
		TUINT   uiLastFrame; // 0x008
		TCHAR   szText[ TEXT_SIZE ];
	};

	static TINT    ms_iNumWarnings;
	static Warning ms_aWarnings[ MAX_WARNINGS ];
};

#endif // PRIMEDEBUG_DEV
