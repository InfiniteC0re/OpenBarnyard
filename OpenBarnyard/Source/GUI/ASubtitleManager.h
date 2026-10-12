#pragma once
#include "GUI/AGUI2TextBox.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2SList.h>
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// A subtitle of ASubtitleManager (vtable 0073b81c, size 0x94): shows the
// lines of a subtitle symbol of Data/Subs/<Language>/<Library>.trb at the
// bottom of the screen, depending on the time given to SetTime.
//-----------------------------------------------------------------------------
class ASubtitle
    : public Toshi::T2SList<ASubtitle>::Node
{
public:
	struct Line
	{
		TINT          iStart; // ms
		TINT          iEnd;   // ms
		const TWCHAR* wszText;
	};

	struct Data
	{
		TINT iNumLines;
		Line aLines[ 1 ];
	};

public:
	ASubtitle();
	virtual ~ASubtitle();

	// Hides the subtitle and forgets its data
	void Reset();

	// Starts showing a_pData
	void Start( const Data* a_pData );

	// Stretches the lines so that the subtitle lasts a_iDuration ms
	void SetDuration( TINT a_iDuration );

	// Shows the line of the time a_iTime (ms)
	void SetTime( TINT a_iTime );

	// Advances the time by a_fDeltaTime, returns TFALSE if no line is shown
	TBOOL Update( TFLOAT a_fDeltaTime );

private:
	// Adds the text box to the screen
	void Show();

private:
	const Data*   m_pData;      // 0x08
	TINT          m_iLine;      // 0x0c
	TINT          m_iTime;      // 0x10
	const TWCHAR* m_wszText;    // 0x14
	AGUI2TextBox  m_oText;      // 0x18
	TFLOAT        m_fTimeScale; // 0x90
};

//-----------------------------------------------------------------------------
// Keeps the subtitles of the current library (TClass 007cd3d4, singleton
// 007cd3d0). Ten subtitles can be played at the same time. The managed ones
// update themselves and stop once no line is shown, the other ones are updated
// and stopped by their owners (ASSeq).
//-----------------------------------------------------------------------------
class ASubtitleManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ASubtitleManager>
{
public:
	TDECLARE_CLASS( ASubtitleManager, Toshi::TTask );

	static constexpr TINT MAX_NUM_SUBTITLES = 10;

public:
	ASubtitleManager();
	~ASubtitleManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE { return TTRUE; } // +0x14, RETURNS_TRUE 0046cde0 (doesn't reset the children)
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	// Loads Data/Subs/<Language>/<a_szLibrary>.trb
	TBOOL LoadSubtitles( const TCHAR* a_szLibrary );
	void  UnloadSubtitles();

	// Starts the subtitle a_szName, lasting a_fDuration seconds if it's positive. Returns TNULL if it doesn't exist
	ASubtitle* PlaySubtitle( const TCHAR* a_szName, TFLOAT a_fDuration, TBOOL a_bManaged );

	void StopSubtitle( ASubtitle* a_pSubtitle );

private:
	Toshi::T2SList<ASubtitle> m_llManagedSubtitles; // 0x20
	Toshi::T2SList<ASubtitle> m_llSubtitles;        // 0x24
	Toshi::T2SList<ASubtitle> m_llFreeSubtitles;    // 0x28
	Toshi::TTRB*              m_pTRB;               // 0x2c
};
