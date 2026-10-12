#pragma once
#include "ASSeqAttachmentManager.h"
#include "ASSeqFade.h"
#include "FSM/AFunctionTable.h"
#include "GUI/AGUI2TextBox.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <ToshiTools/T2DynamicVector.h>

class ASSeq;

// Plays the scripted sequences (SSeq, cutscenes). Created by AGameSystemManager.
//
// TClass 0x007b4b60 (size 0x26c, runtime creatable), vtable 0x007392e0, singleton 0x007b4b5c.
class ASSeqController
    : public Toshi::TTask
    , public Toshi::TSingleton<ASSeqController>
{
public:
	TDECLARE_CLASS( ASSeqController, Toshi::TTask );

	// Script natives of the class, for the FSM
	// $Barnyard: FUNCTION 0062a470
	virtual AFunctionTable* GetFunctionTable() { return &ms_oFunctionTable; } // +0x38

	// Flags of the actors in m_ppActors
	using ACTORFLAGS = TUINT32;
	enum ACTORFLAGS_ : ACTORFLAGS
	{
		ACTORFLAGS_NONE   = 0,
		ACTORFLAGS_UPDATE = BITFLAG( 1 ), // UpdateActor is called every frame while a sequence is playing
	};

public:
	ASSeqController();
	~ASSeqController();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() override;                         // +0x14
	virtual TBOOL OnCreate() override;                      // +0x18
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x1c
	virtual void  OnDestroy() override;                     // +0x24
	virtual void  OnActivate() override;                    // +0x30
	virtual void  OnDeactivate() override;                  // +0x34

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Loads Data/ScriptedSequence/<a_szFileName> unless it's already loaded
	TBOOL LoadScript( const TCHAR* a_szFileName, const Toshi::TVector4& a_rcPosition );

	// Starts playing the loaded sequence a_szName (from a_szFileName if not TNULL)
	TBOOL StartScript( const TCHAR* a_szName, const TCHAR* a_szFileName, TFLOAT a_fFadeTime, TFLOAT a_fSyncSpeed );

	// Stops the playing sequences called a_szName (from a_szFileName if not TNULL)
	void StopScript( const TCHAR* a_szName, const TCHAR* a_szFileName );

	// Stops every playing sequence
	void StopAllScripts();

	// Finds the sequence a_szName (from a_szFileName if not TNULL), loaded or playing
	ASSeq* FindScript( const TCHAR* a_szName, const TCHAR* a_szFileName );

	// Name of the first playing sequence, TNULL if none
	const TCHAR* GetCurrentScriptName();

	// Allocates the table of a_iNumActors actors
	void CreateActors( TINT a_iNumActors );
	void DestroyActors();

	// Sets the actor with the id a_iActor, a_bUpdate: UpdateActor is called every frame while it plays
	void SetActor( TINT a_iActor, ASSeqActorInterface* a_pActor, TBOOL a_bUpdate );

	// $Barnyard: FUNCTION 00629c00
	ASSeqActorInterface* GetActor( TINT a_iActor ) const { return m_ppActors[ a_iActor ]; }

	void ClearActorFlags();
	void SetActorFlag( TINT a_iActor, TINT a_iFlag );

	// Sets ACTORFLAGS_UPDATE if the actor has to be updated every frame
	void UpdateActorFlags( TINT a_iActor, TBOOL a_bUnused );

	// The chapter title shown by the story sequences
	void   SetChapterTitle( const TCHAR* a_szScriptName );
	void   ShowChapterTitle( TBOOL a_bShow );
	void   SetChapterTitleAlpha( TFLOAT a_fAlpha );
	TFLOAT GetChapterTitleAlpha() const { return m_oTextBox.GetAlpha(); }

	// Script functions
	TBOOL StartSSeqState( Toshi::TPString8 a_strScript, Toshi::TPString8 a_strUnk1, Toshi::TPString8 a_strUnk2, TBOOL a_bUnk1, TBOOL a_bUnk2 );
	TBOOL IsSeqPlaying();

	ASSeqAttachmentManager& GetAttachmentManager() { return m_oAttachmentManager; }
	ASSeqFade*              GetFade() const { return m_pFade; }
	TBOOL                   IsStoppingAllScripts() const { return m_bStopAllScripts; }

	// Finds the idle (loaded, not playing) sequence a_szName
	ASSeq* FindLoadedScriptByName( const TCHAR* a_szName ) { return FindScript( m_vecLoadedScripts, a_szName, TNULL ); }

private:
	// Finds the loaded sequence from a_szFileName
	ASSeq* FindLoadedScript( const TCHAR* a_szFileName );

	static ASSeq* FindScript( Toshi::T2DynamicVector<ASSeq*>& a_rScripts, const TCHAR* a_szName, const TCHAR* a_szFileName );

public:
	static AFunctionTable ms_oFunctionTable;

private:
	Toshi::T2DynamicVector<ASSeq*>               m_vecLoadedScripts;   // 0x20, idle sequences
	Toshi::T2DynamicVector<ASSeq*>               m_vecPlayingScripts;  // 0x34
	TINT                                         m_iNumActors;         // 0x48
	ASSeqActorInterface**                        m_ppActors;           // 0x4c, TMalloc'd
	ACTORFLAGS*                                  m_pActorFlags;        // 0x50, TMalloc'd
	TBOOL                                        m_bStopAllScripts;    // 0x54, StopAllScripts is called on the next update
	Toshi::T2DynamicVector<ASSeqActorInterface*> m_vecUpdatedActors;   // 0x58, actors updated every frame
	ASSeqFade*                                   m_pFade;              // 0x6c
	ASSeqAttachmentManager                       m_oAttachmentManager; // 0x70
	AGUI2TextBox                                 m_oTextBox;           // 0x17c, chapter title
	AGUI2TextBox                                 m_oSubTextBox;        // 0x1f4, child of m_oTextBox
};
