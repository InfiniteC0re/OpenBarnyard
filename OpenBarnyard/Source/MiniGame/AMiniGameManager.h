#pragma once
#include "AMiniGame.h"
#include "AMiniGameState.h"

#include <Toshi/TTask.h>
#include <Toshi/TPString8.h>
#include <ToshiTools/T2DynamicVector.h>

class AFunctionTable;

class AMiniGameManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AMiniGameManager>
{
public:
	TDECLARE_CLASS( AMiniGameManager, Toshi::TTask );

	static AFunctionTable ms_oFunctionTable;

public:
	AMiniGameManager();
	~AMiniGameManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x34

	// FSM natives: OnUpdate starts the minigame a_rcName (with the variant a_iVariant, 1-based) once the player is free
	void SetNextMinigame( const Toshi::TPString8& a_rcName );
	void SetNextMinigameVariant( const Toshi::TPString8& a_rcName, TINT a_iVariant );
	void SetNextMinigameNoFade( const Toshi::TPString8& a_rcName );

	AMiniGame& RegisterMiniGame(
	    const TCHAR*            a_szMiniGameName,
	    const TCHAR*            a_szClassStateName,
	    const TCHAR*            a_szIconName,
	    TINT                    a_eLocaleTitle,
	    TINT                    a_eLocaleDescription,
	    TINT                    a_eStartHudKind,
	    const Toshi::TPString8& a_strLoadingScreenName,
	    TBOOL                   a_bUnk2,
	    TINT                    a_iMinNumPlayers,
	    TINT                    a_iMaxNumPlayers,
	    TINT                    a_eInstancesType,
	    TINT                    a_iOrderId,
	    AMiniGame::LOADFLAGS    a_eLoadFlags
	);

	AMiniGame& RegisterHiddenMiniGame(
	    const TCHAR*            a_szMiniGameName,
	    const TCHAR*            a_szClassStateName,
	    const TCHAR*            a_szIconName,
	    TINT                    a_eLocaleTitle,
	    TINT                    a_eLocaleDescription,
	    TINT                    a_eStartHudKind,
	    const Toshi::TPString8& a_strLoadingScreenName,
	    TINT                    a_eInstancesType,
	    AMiniGame::LOADFLAGS    a_eLoadFlags
	);

	void MakeValidMiniGamePlayerSet( TINT a_iMiniGame );

	// Index of the minigame called a_rcName, -1 if there is none
	TINT FindMiniGame( const Toshi::TPString8& a_rcName );

	// Index of the minigame whose title is the locale string a_eLocaleTitle, -1 if there is none
	TINT FindMiniGameByTitle( TINT a_eLocaleTitle );

	AMiniGame* GetMiniGame( TINT a_iIndex );
	AMiniGame* GetVisibleMiniGame( TINT a_iIndex );
	TINT       GetIndexForAntic( TINT a_iIndex );
	TINT       GetNumVisibleMiniGames() const { return m_iNumVisibleMiniGames; }
	TINT       GetTotalNumMiniGames() const { return m_vecMiniGames.Size(); }

	TINT GetMiniGameVariant() const { return m_iMiniGameVariant; }
	void SetMiniGameVariant( TINT a_iVariant ) { m_iMiniGameVariant = a_iVariant; }

	// Name of the asset library of a_iMiniGame (the current variant's alternative state name if any, else the minigame name)
	const TCHAR* GetLibraryName( TINT a_iMiniGame );

	// $Barnyard: FUNCTION 004692a0
	const Toshi::TClass* GetStateClass( TINT a_iMiniGame ) { return m_vecMiniGames[ a_iMiniGame ].m_pGameStateClass; }

	// Returns name of the game state class of a_iMiniGame
	Toshi::TPString8& GetStateClassName( Toshi::TPString8& a_rOutName, TINT a_iMiniGame );

	// Returns localised loading screen picture of a_iMiniGame (empty if it has none)
	Toshi::TPString8& GetLoadingScreenName( Toshi::TPString8& a_rOutName, TINT a_iMiniGame );

#ifdef BARNYARD_LATEST
	// Wii: name of the XUI load screen scene of a_iMiniGame (empty if it has none)
	const Toshi::TPString8& GetXUILoadScreenName( TINT a_iMiniGame );
#endif

	// $Barnyard: FUNCTION 00469330
	AMiniGame* GetNextMiniGame() { return &m_vecMiniGames[ m_iNextMiniGameIndex ]; }

	TINT GetNextMiniGameIndex() const { return m_iNextMiniGameIndex; }

	// $Barnyard: FUNCTION 004698c0
	Toshi::TPString8 GetNextMiniGameName() const { return m_strNextMiniGameName; }
	void             SetNextMiniGameIndex( TINT a_iIndex ) { m_iNextMiniGameIndex = a_iIndex; }

	// OnUpdate returns to the frontend
	void RequestReturn() { m_bReturnRequested = TTRUE; }

	// Forgets the next minigame and the results for the sim (the story is saved before a minigame starts)
	void ClearNextMiniGame();

	// Asks OnUpdate to leave the minigame (quit from the start menu)
	void RequestQuit() { m_bQuitRequested = TTRUE; }

	AMiniGameState::S1& GetSessionResults() { return m_oSessionResults; }

	// NOTE: not in PC retail, used by AFrontEndPlayersState (recovered from the original source)
	// Clears the totals of the players over the multiplayer rounds as a new Antics session starts
	void ClearSessionResults() { m_oSessionResults = AMiniGameState::S1(); }

	// Stores the results of the minigame that has just finished
	void StoreResults( AMiniGameState::S1* a_pResults );

public:
	// Set while a minigame fade is pending (see AFadeActivity)
	inline static TBOOL ms_bHasFade; // 0x007810e0

private:
	// Copies a_rSource into a_rDest (only the fields AMiniGameManager cares about)
	static void CopyResults( AMiniGameState::S1& a_rDest, AMiniGameState::S1& a_rSource );

private:
	Toshi::T2DynamicVector<AMiniGame> m_vecMiniGames;         // 0x20
	Toshi::TPString8                  m_strNextMiniGameName;  // 0x34
	TINT                              m_iNextMiniGameIndex;   // 0x38
	TINT                              m_iUnk3c;               // 0x3c
	TBOOL                             m_bHasSimResults;       // 0x40, set by StoreResults when the minigame was played in story mode
	AMiniGameState::S1                m_oLastResults;         // 0x44
	TBOOL                             m_bQuitRequested;       // 0x15c, set by the start menu, OnUpdate leaves the minigame
	TBOOL                             m_bReturnRequested;     // 0x15d, OnUpdate returns to the frontend
	TINT                              m_iMiniGameVariant;     // 0x160
	AMiniGameState::S1                m_oSessionResults;      // 0x164, totals of the players over the multiplayer rounds
	TINT                              m_iNumVisibleMiniGames; // 0x27c
	Toshi::TTRB                       field612_0x280;         // 0x280
};

TSINGLETON_DECLARE_ALIAS( AMiniGameManager, MiniGameManager );
