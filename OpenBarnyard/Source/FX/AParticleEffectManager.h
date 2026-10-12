#pragma once
#include "AParticleSystem.h"

#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TDList.h>
#include <Math/TVector4.h>

class AModelInstance;

// A playing effect, optionally attached to a bone of a model or to the camera
class AParticleEffectHandle
    : public Toshi::TDList<AParticleEffectHandle>::TNode
{
public:
	friend class AParticleEffectManager;

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE            = 0,
		FLAGS_FOLLOW_POSITION = BITFLAG( 0 ), // Follows the position of the bone
		FLAGS_FOLLOW_ROTATION = BITFLAG( 1 ), // Follows the rotation of the bone
		FLAGS_FOLLOW_CAMERA   = BITFLAG( 2 ), // Stays in front of the camera
		FLAGS_FACE_CAMERA     = BITFLAG( 3 ), // Moves towards the camera a bit
		FLAGS_ATTACHED        = FLAGS_FOLLOW_POSITION | FLAGS_FOLLOW_ROTATION,
	};

	using t_OnFinished = void( __stdcall* )( AParticleEffectHandle* a_pHandle, void* a_pUserData );

public:
	AParticleEffectHandle();

	// Updates the transform of the effect
	TBOOL Update( TFLOAT a_fDeltaTime );

	// Makes the effect follow the bone a_iBone of a_pModel
	void AttachToBone( AModelInstance* a_pModel, TINT a_iBone, TBOOL a_bFollowPosition, TBOOL a_bFollowRotation );

	// Makes the effect stay in front of the camera
	void AttachToCamera();

	// Stops following the model
	void Detach()
	{
		m_eFlags &= ~FLAGS_ATTACHED;
		m_pAttachModel = TNULL;
		m_iAttachBone  = -1;
	}

	AParticleEffectInstance* GetInstance() const { return m_pInstance; }

	void SetOnFinishedCallback( t_OnFinished a_fnCallback, void* a_pUserData )
	{
		m_fnOnFinished        = a_fnCallback;
		m_pOnFinishedUserData = a_pUserData;
	}

public:
	AParticleEffectInstance* m_pInstance;           // 0x8
	t_OnFinished             m_fnOnFinished;        // 0xc
	void*                    m_pOnFinishedUserData; // 0x10
	AModelInstance*          m_pAttachModel;        // 0x14
	TINT                     m_iAttachBone;         // 0x18
	FLAGS                    m_eFlags;              // 0x1c
};

TSTATICASSERT( sizeof( AParticleEffectHandle ) == 0x20 );

// Plays the particle effects of the particle library. Effects are looked up by
// name once (FindEffect) and then played by index (PlayEffect). Every playing
// effect occupies one of the handles, which can follow a model's bone.
//
// Singleton at 0x007b5ea4, vtable 0x0073aae4.
class AParticleEffectManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AParticleEffectManager>
{
public:
	TDECLARE_CLASS( AParticleEffectManager, Toshi::TTask );

public:
	AParticleEffectManager();
	~AParticleEffectManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL OnCreate() OVERRIDE { return TTRUE; } // +0x18, RETURNS_TRUE (0042c2b0)
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Returns index of the effect (case insensitive) or -1 if there's no such effect.
	static TINT FindEffect( const TCHAR* a_szName );

	// Returns a copy of the effect a_iEffect that can be changed
	static AParticleEffect* CloneEffect( TINT a_iEffect );

	// Plays effect a_iEffect (see FindEffect) at a_rcPosition facing a_rcDirection.
	// Returns the handle of the effect or TNULL if it couldn't be played.
	AParticleEffectHandle* PlayEffect( TINT a_iEffect, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	// Plays a_pEffect at a_rcPosition facing a_rcDirection.
	AParticleEffectHandle* SpawnEffect( AParticleEffect* a_pEffect, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	// Kills every effect and unloads the particle library.
	static void UnloadParticleLibrary();

	// Kills every effect
	void KillAll();

	// Detaches every playing effect from the model it follows
	void DetachAllEffects();

	AParticleSystem* GetParticleSystem() const { return m_pParticleSystem; }

private:
	static void __stdcall OnEffectFinished( AParticleEffectInstance* a_pInstance, void* a_pUserData );

	static void __stdcall OnMaterialLibraryLoaded( AMaterialLibrary* a_pLibrary );
	static void __stdcall OnMaterialLibraryUnloaded( AMaterialLibrary* a_pLibrary );

private:
	// Whether effects are updated, set by OnActivate/OnDeactivate (0x007b5e70)
	inline static TBOOL ms_bActive = TFALSE;

private:
	AParticleSystem*                     m_pParticleSystem; // 0x20
	AParticleEffectPool*                 m_pEffectPool;     // 0x24
	TINT                                 m_iNumHandles;     // 0x28
	AParticleEffectHandle*               m_pHandles;        // 0x2c
	Toshi::TDList<AParticleEffectHandle> m_oFreeHandles;    // 0x30
	Toshi::TDList<AParticleEffectHandle> m_oUsedHandles;    // 0x38
};

TSTATICASSERT( sizeof( AParticleEffectManager ) == 0x40 );
