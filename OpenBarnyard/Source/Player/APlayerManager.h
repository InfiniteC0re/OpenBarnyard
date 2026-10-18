#pragma once
#include "APlayerSet.h"

#include <Toshi/TObject.h>
#include <Toshi/TEvent.h>
#include "FSM/AFunctionTable.h"

class MODAPI APlayerManager
    : public Toshi::TObject
    , public Toshi::TSingleton<APlayerManager>
{
public:
	TDECLARE_CLASS( APlayerManager, Toshi::TObject );

	// Script natives of the class, for the FSM
	// $Barnyard: FUNCTION 0061fc50
	virtual AFunctionTable* GetFunctionTable() { return &ms_oFunctionTable; } // +0x0c

	static AFunctionTable ms_oFunctionTable;

public:
	using PLAYERSET = TINT;
	enum PLAYERSET_ : PLAYERSET
	{
		PLAYERSET_SIM,
		PLAYERSET_MINIGAME,
		PLAYERSET_NUMOF
	};

public:
	APlayerManager();
	~APlayerManager();

	// $Barnyard: FUNCTION 0061fad0
	APlayerSet* GetPlayerSet( TINT a_eType )
	{
		TASSERT( a_eType < TARRAYSIZE( m_aPlayerSets ) );
		return &m_aPlayerSets[ a_eType ];
	}

	APlayerSet* GetMiniGamePlayerSet() { return GetPlayerSet( PLAYERSET_MINIGAME ); }

	void ResetPlayerIterator() { m_iCurrentPlayerIt = 0; }

	APlayerSet* GetCurrentPlayerSet() const { return m_pPlayerSets; }

	// $Barnyard: FUNCTION 0061fc60
	void SetCurrentPlayerSet( TINT a_eType ) { m_pPlayerSets = &m_aPlayerSets[ a_eType ]; }

	// Unit of the a_iIndex-th human player of the current player set
	class AUnit* GetPlayer( TINT a_iIndex );

	// Unit of the a_iSlot slot of the current player set
	class AUnit* GetPlayerUnit( TINT a_iSlot );

	// Returns index of the player set called a_rcName ("sim" or "minigame") or -1
	static PLAYERSET GetPlayerSetIndex( const Toshi::TPString8& a_rcName );

	// Moves the iterator to the next human player (wraps to 0) and returns it.
	TINT GetNextPlayer();
	TINT GetCurrentPlayerIterator() const { return m_iCurrentPlayerIt; }

private:
	static TBOOL __stdcall OnUnitCreated( APlayerManager* a_pCaller, void* a_pOwner, class AUnit** a_ppUnit );
	static TBOOL __stdcall OnUnitsRemoved( APlayerManager* a_pCaller, void* a_pOwner, TUINT* a_pType );

private:
	APlayerSet*             m_pPlayerSets;           // 0x04
	APlayerSet              m_aPlayerSets[ 2 ];      // 0x08
	Toshi::TGenericListener m_oUnitCreatedListener;  // 0xd0
	Toshi::TGenericListener m_oUnitsRemovedListener; // 0xe4

	// ...

	TINT m_iCurrentPlayerIt;
};
