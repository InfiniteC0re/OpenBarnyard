#pragma once
#include "PrimeDebug.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Purpose: the minimum, maximum, average and deviation of a value (a frame
// time), taken over intervals (Prime's crStats), and the frame stats the
// game feeds once a frame: the frame time and the CPU time, averaged over two
// seconds. Print adds their lines to the debug text ("TOTAL FPS ...",
// green at 60 frames a second, yellow below, red below 30).
// NOTE: Prime also keeps the GPU time (gcKernel); TOSHI's renderer doesn't
// measure it.
//-----------------------------------------------------------------------------
class PStats
{
public:
	PStats( TFLOAT a_fInterval );

	TBOOL Add( TFLOAT a_fValue, TFLOAT a_fTime );

	TFLOAT GetLastMin() const { return m_fLastMin; }
	TFLOAT GetLastMax() const { return m_fLastMax; }
	TFLOAT GetAverage() const { return m_fAverage; }
	TFLOAT GetDeviation() const { return m_fDeviation; }
	TFLOAT GetRate() const { return m_fRate; }

private:
	TFLOAT m_fMin;        // 0x00, of the current interval
	TFLOAT m_fMax;        // 0x04
	TFLOAT m_fSum;        // 0x08
	TFLOAT m_fSumSquares; // 0x0C
	TFLOAT m_fLastMin;    // 0x10, of the last interval
	TFLOAT m_fLastMax;    // 0x14
	TFLOAT m_fAverage;    // 0x18
	TFLOAT m_fDeviation;  // 0x1C
	TFLOAT m_fRate;       // 0x20, 1 / the average
	TFLOAT m_fInterval;   // 0x24
	TFLOAT m_fTime;       // 0x28
	TINT   m_iCount;      // 0x2C

public:
	static void Startup();
	static void Shutdown();

	// The frame's CPU and total times, in seconds
	static void Update( TFLOAT a_fCpuTime, TFLOAT a_fFrameTime );
	static void Print();
	static void SetUnknown( TUINT a_uiValue );

	static TUINT32       GetColour( TFLOAT a_fRate );
	static const PStats& GetFrameStats() { return ms_oFrameStats; }
	static const PStats& GetCpuStats() { return ms_oCpuStats; }

private:
	static PStats ms_oCpuStats;
	static PStats ms_oFrameStats;
	static TUINT  ms_uiUnknown;
};

#endif // PRIMEDEBUG_DEV
