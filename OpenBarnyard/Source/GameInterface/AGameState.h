#pragma once
#include "Input/AInputMapManager.h"
#include "AGameStateControllerEvent.h"
#include "Sound/ASound.h"
#include "HUD/AHudElement.h"

#include <Toshi/T2DList.h>
#include <Input/TInputInterface.h>
#include <GUI/T2GUIElement.h>

class MODAPI AGameState
    : public Toshi::TObject
    , public Toshi::T2DList<AGameState>::Node
{
public:
	using OVERLAY = TUINT;
	enum OVERLAY_ : OVERLAY
	{
		OVERLAY_1,
		OVERLAY_2,
		OVERLAY_3,
		OVERLAY_NUMOF,
	};

	friend class AGameStateController;

public:
	TDECLARE_CLASS( AGameState, Toshi::TObject );

	template <class Result, class ObjType, class... Args>
	using t_ExecuteForChildCb = Result ( ObjType::* )( Args... args );

	struct HUDParams
	{
		// $Barnyard: FUNCTION 0059c9d0
		HUDParams()
		    : m_uiFlags( 0 ), m_Unk1( 0 ), m_Unk2( 0 ), m_Unk3( 0 ), m_Unk4( 0 ), m_Unk5( 0 ), m_Unk6( 0 ), m_Unk7( 0 ), m_Unk8( 0 ), m_Unk9( 0 )
		{}

		// $Barnyard: FUNCTION 0059ca00
		HUDParams& operator=( const HUDParams& a_rcOther )
		{
			m_uiFlags = a_rcOther.m_uiFlags;
			m_Unk1    = a_rcOther.m_Unk1;
			m_Unk2    = a_rcOther.m_Unk2;
			m_Unk3    = a_rcOther.m_Unk3;
			m_Unk4    = a_rcOther.m_Unk4;
			m_Unk5    = a_rcOther.m_Unk5;
			m_Unk6    = a_rcOther.m_Unk6;
			m_Unk7    = a_rcOther.m_Unk7;
			m_Unk8    = a_rcOther.m_Unk8;
			m_Unk9    = a_rcOther.m_Unk9;
			return *this;
		}

		AHudElement::Mask m_uiFlags;
		TUINT             m_Unk1;
		TUINT             m_Unk2;
		TUINT             m_Unk3;
		TUINT             m_Unk4;
		TUINT             m_Unk5;
		TUINT             m_Unk6;
		TUINT             m_Unk7;
		TUINT             m_Unk8;
		TUINT             m_Unk9;

		// $Barnyard: FUNCTION 0059ca50
		void SetFlags( AHudElement::Mask a_uiFlags )
		{
			if ( a_uiFlags != m_uiFlags )
			{
				m_uiFlags = a_uiFlags;
			}
		}

		// Passes the parameters to the GUI system (AGameState::OnInsertion, OnResume)
		void Apply();

		// DOES_NOTHING (AGameState::OnRemoval)
		void Remove() {}
	};

public:
	AGameState();
	~AGameState();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent );
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents );
	virtual TBOOL Unknown1( void* a_pUnk1, void* a_pUnk2 );
	// NOTE: render hooks, names are invented (called by ARenderer::RenderMainScene via AGameStateController)
	virtual void     OnRender( TFLOAT a_fDeltaTime );        // +0x18, world pass, before glowing models
	virtual void     OnPostRender( TFLOAT a_fDeltaTime );    // +0x1c, world pass, after glowing models
	virtual void     OnRenderOverlay( TFLOAT a_fDeltaTime ); // +0x20, second pass without background/depth clear
	virtual void     OnLoaded();
	virtual TBOOL    ShouldPlacePlayerAtStartGoal();
	virtual ASoundId GetSound();
	virtual TBOOL    CanDismissInstructions();
	virtual void     OnInstructionsShown();
	virtual void     OnStart();
	virtual void     OnPause();
	virtual void     OnCollisionModelLoaded( void* a_pCollisionModel, void* a_pGroupId );
	virtual void     OnVISGroupChanged( void* a_pCurrentSection, void* a_pPreviousSection );
	virtual TFLOAT   GetFOV();
	virtual TBOOL    OnUpdate( TFLOAT a_fDeltaTime );
	virtual void     OnInsertion();
	virtual void     OnRemoval();
	virtual void     OnSuspend();
	virtual void     OnResume( AGameState* a_pPreviousState );
	virtual void     OnActivate();
	virtual void     OnDeactivate();

	TBOOL SendInputCommands( const Toshi::TInputInterface::InputEvent* a_pEvent );

	void SetInputMap( const Toshi::TPString8& a_MapName );

	void Activate();
	void Deactivate();
	void Suspend();
	void Remove();

	void Destroy();
	void Destroy( TBOOL a_bDeactivate );

	void    SetOverlay( OVERLAY a_eOverlay ) { m_eOverlayColorIndex = a_eOverlay; }
	OVERLAY GetOverlay() const { return m_eOverlayColorIndex; }

	TUINT32 GetHUDMask();

	AGameState* FindChildState( Toshi::TClass* a_pClass );

	// Saves/restores the FOV of the current camera into m_fFOV
	void SaveCameraFOV();
	void RestoreCameraFOV();

	// Makes a_pChildState a child of this state, inserting/activating it if this state already is
	void AddChildState( AGameState* a_pChildState );

	// Removes and deletes a_pOldChildState and adds a_pNewChildState instead of it
	void ReplaceChildState( AGameState* a_pOldChildState, AGameState* a_pNewChildState );

	AGameState* GetOwnerState() const { return m_pOwnerState; }

	HUDParams&           GetHUDParams() { return m_HUDParams; }
	AInputMap*           GetInputMap() const { return m_pInputMap; }
	Toshi::T2GUIElement& GetGUIElement() { return m_GUIElement; }

	// NOTE: retail has these as non-inlined template functions taking the member function
	// pointer, the offset and the arguments.
	// Instantiations:
	// <void> (OnInsertion, OnRemoval, OnSuspend, OnActivate, OnDeactivate): 004286a0
	// <void, TFLOAT> / <void, AGameState*> (OnUpdate, OnResume, folded):     004284a0
	// <void, TUINT32&> (GetHUDMaskImpl):                                      00428500
	// $Barnyard: FUNCTION 004286a0
	// $Barnyard: FUNCTION 004284a0
	// $Barnyard: FUNCTION 00428500
	template <class RetT, typename... Args>
	static TFORCEINLINE TNODISCARD auto Bind_CallForAllChild( AGameState* pRoot, t_ExecuteForChildCb<RetT, AGameState, Args...> a_fnCallback, TUINT32 a_uiOffset = 0 )
	{
		return [ pRoot, a_fnCallback, a_uiOffset ]( std::type_identity_t<Args>... args ) {
			// NOTE: the next state is taken before the call (like 004284a0) since a child can remove itself
			for ( auto it = pRoot->m_ChildStates.Begin(); it != pRoot->m_ChildStates.End(); )
			{
				auto pGameState = TREINTERPRETCAST(
				    AGameState*,
				    TREINTERPRETCAST( TUINTPTR, TSTATICCAST( AGameState, it ) ) + a_uiOffset
				);

				++it;
				( pGameState->*a_fnCallback )( args... );
			}
		};
	}

	// Instantiations (the next node is read before calling, like Bind_CallForAllChild):
	// <const InputEvent*> (ProcessInput, SendInputCommands):     004283a0
	// <void*, void*> (Unknown1):                                 00428420
	// <TClass*, AGameState*&> (FindChildState, FindChildStateImpl): 00428560
	// $Barnyard: FUNCTION 004283a0
	// $Barnyard: FUNCTION 00428420
	// $Barnyard: FUNCTION 00428560
	template <typename... Args>
	static TFORCEINLINE TNODISCARD auto Bind_CallForOneChild( AGameState* pRoot, t_ExecuteForChildCb<TBOOL, AGameState, Args...> a_fnCallback, TUINT32 a_uiOffset = 0 )
	{
		return [ pRoot, a_fnCallback, a_uiOffset ]( std::type_identity_t<Args>... args ) -> bool {
			for ( auto it = pRoot->m_ChildStates.Begin(); it != pRoot->m_ChildStates.End(); )
			{
				auto pGameState = TREINTERPRETCAST(
				    AGameState*,
				    TREINTERPRETCAST( TUINTPTR, TSTATICCAST( AGameState, it ) ) + a_uiOffset
				);

				++it;
				TBOOL bResult = ( pGameState->*a_fnCallback )( args... );

				if ( bResult )
				{
					return TTRUE;
				}
			}

			return TFALSE;
		};
	}

	TINT GetNumChildStates() const { return m_ChildStates.Size(); }

	// Returns the sound of the first child state that has one (-1 if none)
	// $Barnyard: FUNCTION 00428a90
	ASoundId GetChildStatesSound()
	{
		T2_FOREACH( m_ChildStates, it )
		{
			if ( it->GetSound() != -1 )
				return it->GetSound();
		}

		return ASOUNDID_INVALID;
	}

private:
	TBOOL FindChildStateImpl( Toshi::TClass* a_pClass, AGameState*& a_rOutput );
	void  GetHUDMaskImpl( TUINT32& a_rOutput );

protected:
	HUDParams                  m_HUDParams;
	OVERLAY                    m_eOverlayColorIndex;
	Toshi::T2DList<AGameState> m_ChildStates;
	AInputMap*                 m_pInputMap;
	AGameState*                m_pOwnerState;
	TBOOL                      m_bWasInserted;
	TBOOL                      m_bIsActivated;
	Toshi::T2GUIElement        m_GUIElement;
	TFLOAT                     m_fFOV;
};
