#pragma once
#include "ACameraHelper.h"
#include "Animation/AModel.h"

#include <Render/TAnimation.h>

//-----------------------------------------------------------------------------
// Purpose: Camera of the speech sequences (ASpeechSeqActivity), placed relative to the
// speakers with one of nine camera rig models ("point_pip_group", ...)
// (ACameraManager::CAMERAHELPER_SPEECH).
//
// TClass 0x007825a8 (size 0x1c4), vtable 0x0071d1f8.
//-----------------------------------------------------------------------------
class ASpeechCameraHelper
    : public ACameraHelper
{
public:
	TDECLARE_CLASS( ASpeechCameraHelper, ACameraHelper );

	static constexpr TINT NUM_MODELS   = 9;
	static constexpr TINT MAX_SPEAKERS = 4;

public:
	ASpeechCameraHelper();
	~ASpeechCameraHelper();

	//-----------------------------------------------------------------------------
	// ACameraHelper
	//-----------------------------------------------------------------------------
	virtual void  UpdateCurrentCamera() override;                                   // +0x0c
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;                         // +0x10
	virtual TBOOL Unk2( void* ) override { return TFALSE; }                         // +0x14, RETURNS_FALSE 006ce000
	virtual TBOOL ProcessInputCommand( void* a_pCommand, void* a_pEvent ) override; // +0x18
	virtual void  OnCreate() override {}                                            // +0x1c, DOES_NOTHING 005acd80
	virtual void  OnDestroy() override {}                                           // +0x20, DOES_NOTHING 005acd80
	virtual void  OnAttach() override;                                              // +0x24
	virtual void  ResetTarget() override;                                           // +0x38
	virtual void  OnDetach() override;                                              // +0x3c

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Shows the unit that was hidden while the camera was inside it and forgets it
	virtual void ShowHiddenUnit(); // +0x44

	// Shows the unit with the id a_iUnitId from the camera a_iCamera of the camera model,
	// a_bSwitchAngle switches to the other angle (unless the unit is already shown)
	void SetSpeaker( TINT a_iAngle, TINT a_iCamera, TINT a_iUnitId, TBOOL a_bSwitchAngle );

	// Shows the speaker a_iUnitId (one of the units added with AddSpeaker) from the camera a_iCamera,
	// the angle is the index of the speaker
	void ShowSpeaker( TINT a_iCamera, TINT a_iUnitId );

	// Selects the camera rig model called a_szName ("model" of the speech sequence)
	void SetModel( const TCHAR* a_szName );

	// Selects the camera rig model a_uiModel (TNULL if it's not one of the NUM_MODELS)
	void SetModel( TUINT a_uiModel );

	// Forgets the speaker the camera shows
	void ResetCamera();

	// Hides the HUD of the current game state (RestoreHUD restores it)
	void HideHUD();
	void RestoreHUD();

	// Releases the speakers (TNULL'ing the slots without moving them back) and shows the hidden unit
	void ReleaseSpeakers();

	// Forgets the speakers and their saved positions
	void ClearSpeakers();

	// Takes a_pUnit as the next speaker, remembering its position and direction to restore them later
	TBOOL AddSpeaker( AUnit* a_pUnit );

	// Returns the position/direction of the speaker spot a_iSpeaker ("char_XX" bone) of the camera rig
	TBOOL GetSpeakerPosition( TINT a_iSpeaker, Toshi::TVector4& a_rOutPosition ) const;
	TBOOL GetSpeakerDirection( TINT a_iSpeaker, Toshi::TVector4& a_rOutDirection ) const;

	// Places the camera rig at a_rcPosition
	void SetPosition( const Toshi::TVector4& a_rcPosition );

	// Turns the camera rig to look along a_rcDirection
	void SetDirection( const Toshi::TVector4& a_rcDirection );

	// Turns the camera rig to the yaw a_fYaw (only for the first two models)
	void SetYaw( TFLOAT a_fYaw );

	// Places the camera rig with a_rcTransform (a locator) instead of the position and yaw
	void SetTransform( const Toshi::TMatrix44& a_rcTransform );

	// Stops the sequence, moves the speakers back to where they were (a_bFaceCentre turns them to the centre of the group)
	void Stop( TBOOL a_bFaceCentre );

	TINT GetSpeakerId() const { return m_iSpeakerId; }
	TINT GetModelIndex() const { return m_iCurrentModel; }

private:
	// Loads the camera rig models
	void LoadModels();

	// Destroys the camera rig models
	void DestroyModels();

	// Selects the first camera rig model
	void ResetModel();

	// Updates the speaker spots of the camera rig
	void UpdateSpeakers();

	// Hides the unit the camera would be inside of
	void HideUnitInCamera();

	// Returns the index of the camera rig model called a_szName, -1 if there's none
	static TINT FindModel( const TCHAR* a_szName );

private:
	TINT                      m_iCurrentModel;                       // 0x10
	AModelInstance*           m_apModels[ NUM_MODELS ];              // 0x14
	AModelInstance*           m_pCurrentModel;                       // 0x38
	Toshi::TSkeletonInstance* m_pSkeletonInstance;                   // 0x3c
	Toshi::TMatrix44          m_oTransform;                          // 0x40, see SetTransform
	TBOOL                     m_bHasTransform;                       // 0x80
	TINT                      m_iCameraBone;                         // 0x84, bone "cam_cXX_aXX" of the camera model
	TINT                      m_iTargetBone;                         // 0x88, bone "targ_cXX_aXX" of the camera model
	Toshi::TVector4           m_vPosition;                           // 0x8c, see SetPosition
	TFLOAT                    m_fYaw;                                // 0x9c, see SetYaw
	TINT                      m_iSpeakerId;                          // 0xa0, AUnit id of the unit the camera shows
	TINT                      m_iAngle;                              // 0xa4
	Toshi::TRef<AUnitProxy>   m_aSpeakers[ MAX_SPEAKERS ];           // 0xa8
	Toshi::TVector4           m_avSavedPositions[ MAX_SPEAKERS ];    // 0xb8, positions of the speakers before the sequence
	Toshi::TVector4           m_avSavedDirections[ MAX_SPEAKERS ];   // 0xf8, directions of the speakers before the sequence
	Toshi::TVector4           m_avSpeakerPositions[ MAX_SPEAKERS ];  // 0x138
	Toshi::TVector4           m_avSpeakerDirections[ MAX_SPEAKERS ]; // 0x178
	TUINT                     m_uiSpeakerMask;                       // 0x1b8, the speaker spots the camera rig has
	TUINT                     m_uiSavedHUDFlags;                     // 0x1bc, see HideHUD
	Toshi::TRef<AUnitProxy>   m_pHiddenUnit;                         // 0x1c0
};

TSTATICASSERT( sizeof( ASpeechCameraHelper ) == 0x1c4 );
