#pragma once
#include "GameInterface/SaveLoad.h"

// Windows part of SaveLoad: the save data is the file Save\savedata.by next to the executable.
class MODAPI SaveLoadSKU
    : public SaveLoad
{
public:
	TDECLARE_CLASS( SaveLoadSKU, SaveLoad );

	using MODE = TINT;
	enum MODE_ : MODE
	{
		MODE_LOAD = 1,
		MODE_2    = 2,
		MODE_3    = 3,
		MODE_SAVE = 4,
		MODE_NONE = 5,
	};

	using RESULT = TINT;
	enum RESULT_ : RESULT
	{
		RESULT_NONE    = 0,
		RESULT_SUCCESS = 1,
		RESULT_FAILED  = 2,
	};

	// One step of the saving / loading, Update returns the next step (TNULL when done)
	class Step
	{
	public:
		// $Barnyard: FUNCTION 00652f90
		Step()
		    : m_pOwner( ms_pInstance )
		{}

		// $Barnyard: FUNCTION 00652f80
		// $Barnyard: FUNCTION 00652a50
		// $Barnyard: FUNCTION 00652bc0
		virtual ~Step() {}

		virtual Step* Update( TFLOAT a_fDeltaTime ) = 0;

	protected:
		SaveLoadSKU* m_pOwner; // 0x04
	};

	// Checks the save directory and file exist
	class CheckFileStep : public Step
	{
	public:
		// $Barnyard: FUNCTION 00652a70
		CheckFileStep() {}

		virtual Step* Update( TFLOAT a_fDeltaTime ) OVERRIDE;
	};

	// Shows a message for a moment
	class MessageStep : public Step
	{
	public:
		MessageStep( TINT a_iTextId );

		virtual Step* Update( TFLOAT a_fDeltaTime ) OVERRIDE;

	private:
		TFLOAT m_fTime; // 0x08
	};

	// Reads the save file
	class LoadStep : public Step
	{
	public:
		// $Barnyard: FUNCTION 00652c80
		LoadStep() {}

		virtual Step* Update( TFLOAT a_fDeltaTime ) OVERRIDE;
	};

	// Writes the save file
	class SaveStep : public Step
	{
	public:
		// $Barnyard: FUNCTION 00652e00
		SaveStep() {}

		virtual Step* Update( TFLOAT a_fDeltaTime ) OVERRIDE;
	};

public:
	SaveLoadSKU( APlayerProgress* a_pProgress, TBOOL a_bBackground, AGameState* a_pNextState );
	~SaveLoadSKU();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x4c
	virtual void  OnInsertion() OVERRIDE;                   // +0x50
	virtual void  OnRemoval() OVERRIDE;                     // +0x54

	//-----------------------------------------------------------------------------
	// SaveLoad
	//-----------------------------------------------------------------------------
	virtual void Cancel() OVERRIDE;                                                               // +0x68
	virtual void StartLoad() OVERRIDE;                                                            // +0x6c
	virtual void StartSave( TBOOL a_bUnused, TBOOL a_bDontSave, TBOOL a_bTakeSnapshot ) OVERRIDE; // +0x74
	virtual void StartMode2() OVERRIDE;                                                           // +0x78
	virtual void StartMode3() OVERRIDE;                                                           // +0x7c
	virtual TINT GetResult() OVERRIDE;                                                            // +0x88

	// Shows a_wszText in the dialog (only while saving), TNULL hides it
	void SetMessage( const TWCHAR* a_wszText );

	const TCHAR* GetSavePath() const { return m_szSavePath; }

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: makes every SaveLoadSKU use a_szPath instead of Save\savedata.by (TNULL goes back to it)
	static void Dev_SetSavePath( const TCHAR* a_szPath );

	// Development: the path set by Dev_SetSavePath, an empty string if there's none
	static const TCHAR* Dev_GetSavePath() { return ms_szDevSavePath; }
#endif

private:
	inline static SaveLoadSKU* ms_pInstance = TNULL; // 0x007b5edc

#ifdef BARNYARD_COMMUNITY_PATCH
	inline static TCHAR ms_szDevSavePath[ 260 ] = { '\0' };
#endif

private:
	MODE        m_eMode;             // 0xc4
	Step*       m_pStep;             // 0xc8
	AGameState* m_pNextState;        // 0xcc
	TCHAR       m_szSaveDir[ 259 ];  // 0xd0, <exe directory>Save
	TCHAR       m_szSavePath[ 261 ]; // 0x1d3, <exe directory>Save\savedata.by
	RESULT      m_eResult;           // 0x2d8
	TBOOL       m_bDontSave;         // 0x2dc
};
