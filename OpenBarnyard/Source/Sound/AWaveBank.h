#pragma once
#include "AWave.h"

#include <Toshi/TPString8.h>

TDECLARE_POINTER_HANDLE( AWaveBankFileHandle );

class PBProperties;

class AWaveBank
{
public:
	using LOADFLAGS = TUINT32;
	enum LOADFLAGS_ : LOADFLAGS
	{
		LOADFLAGS_NONE     = 0,
		LOADFLAGS_LOCALISE = BITFLAG( 0 ),
	};

	enum LOADRESULT
	{
		LOADRESULT_ERR = -1,
		LOADRESULT_OK = 0,
	};

	enum UNLOADRESULT
	{
		UNLOADRESULT_OK = 0
	};

	static constexpr const TCHAR* SOUNDS_BASE_DIRECTORY = "Data/Sound/";

public:
	AWaveBank( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strPath, TBOOL a_bSetFrequency );

	virtual TBOOL IsReady() const = 0;
	virtual LOADRESULT Load( LOADFLAGS a_uiFlags, TINT a_iBufferSize ) = 0;
	virtual UNLOADRESULT Unload() = 0;
	virtual AWaveSampleHandle GetWaveSample( TUINT a_uiNumSample ) const;
	virtual void Unknown( void* a_Unknown ); // empty in the original
	virtual ~AWaveBank();

	void ParseWavesData( const PBProperties* a_pBankProperties, TUINT a_uiForcedFlags );

protected:
	TINT m_iNumWaves;
	TINT m_iNumLoads;
	Toshi::TPString8 m_strBank;
	Toshi::TPString8 m_strPath;
	Toshi::TPString8 m_strExtension;
	TBOOL m_bSetFrequency;
	AWave* m_pWaves;
	AWaveBankFileHandle m_FileHandle;
};
