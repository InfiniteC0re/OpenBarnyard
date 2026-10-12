#pragma once
#include <Toshi/TDList.h>
#include <Toshi/TSingleton.h>
#include <Math/TVector2.h>

class AUnit;
class AModelInstance;
class AParticleEffectHandle;

// Blob shadows: every unit or model instance that wants one gets a node with a
// "shadow" (or "shadow_car") particle effect. Update() projects the effect onto
// the terrain under its owner while the owner is visible and within 30 units
// of the camera, and kills it otherwise.
//
// TClass 0x0079b2f4 (TObject, version 0.1, size 0x2340), vtable 0x00736e60,
// singleton 0x0079b2f0. Only created through its TClass (CreateObject 0060dba0).
class AShadowManager
    : public Toshi::TObject
    , public Toshi::TSingleton<AShadowManager>
{
public:
	TDECLARE_CLASS( AShadowManager, Toshi::TObject );

	static constexpr TINT MAX_NUM_SHADOWS = 250;

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE             = 0,
		FLAGS_ENABLED          = BITFLAG( 0 ), // SetShadowEnabled, shadow is shown while the owner is in range
		FLAGS_UNIT             = BITFLAG( 1 ), // pOwner is an AUnit (AddUnitShadow)
		FLAGS_MODEL            = BITFLAG( 2 ), // pOwner is an AModelInstance (AddModelShadow)
		FLAGS_STATIC           = BITFLAG( 3 ), // SetShadowStatic, the shadow isn't moved after its owner unless forced
		FLAGS_UNK4             = BITFLAG( 4 ), // same as FLAGS_STATIC; never set by AShadowManager
		FLAGS_UNIFORM_SCALE    = BITFLAG( 5 ), // scaled by vecScale.x on all axes
		FLAGS_NONUNIFORM_SCALE = BITFLAG( 6 ), // X axis scaled by vecScale.x, Z axis by vecScale.y
		FLAGS_FORCE_UPDATE     = BITFLAG( 7 ), // reproject on the next Update()
	};

	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_DEFAULT, // "shadow"
		TYPE_CAR,     // "shadow_car"
	};

	// 0x24 bytes
	struct Shadow : public Toshi::TDList<Shadow>::TNode
	{
		Toshi::TObject*        pOwner;        // 0x08, AUnit (FLAGS_UNIT) or AModelInstance (FLAGS_MODEL)
		AParticleEffectHandle* pEffect;       // 0x0c, cleared by OnShadowEffectFinished
		Toshi::TVector2        vecScale;      // 0x10
		TFLOAT                 fHeightOffset; // 0x18
		TYPE                   eType;         // 0x1c
		TINT8                  iBone;         // 0x20, -1 to follow the model origin
		FLAGS                  eFlags;        // 0x21
	};

public:
	AShadowManager();
	~AShadowManager();

	// Kills every shadow effect and returns all nodes to the free list.
	void Reset();

	void Update();

	void AddUnitShadow( AUnit* a_pUnit, const Toshi::TVector2& a_rcScale, TINT8 a_iBone, TFLOAT a_fHeightOffset, FLAGS a_eFlags, TYPE a_eType );
	void AddUnitShadow( AUnit* a_pUnit, TFLOAT a_fScale, TINT8 a_iBone, TFLOAT a_fHeightOffset, FLAGS a_eFlags, TYPE a_eType );
	void AddModelShadow( AModelInstance* a_pModelInstance, const Toshi::TVector2& a_rcScale, TINT8 a_iBone, TFLOAT a_fHeightOffset, FLAGS a_eFlags, TYPE a_eType );
	void AddModelShadow( AModelInstance* a_pModelInstance, TFLOAT a_fScale, TINT8 a_iBone, TFLOAT a_fHeightOffset, FLAGS a_eFlags, TYPE a_eType );

	// All of these do nothing for TNULL or for an owner without a shadow.
	void RemoveShadow( Toshi::TObject* a_pOwner );
	void SetShadowEnabled( Toshi::TObject* a_pOwner, TBOOL a_bEnabled );
	void SetShadowStatic( Toshi::TObject* a_pOwner, TBOOL a_bStatic );
	void SetShadowScale( Toshi::TObject* a_pOwner, const Toshi::TVector2& a_rcScale );

	// Called by the terrain section change callback (00423720); reprojects every shadow.
	void OnTerrainSectionChanged( TINT a_iUnk1, TINT a_iUnk2 );

private:
	void RemoveShadowImp( Toshi::TObject* a_pOwner );
	void SetShadowEnabledImp( Toshi::TObject* a_pOwner, TBOOL a_bEnabled );
	void SetShadowFlagImp( Toshi::TObject* a_pOwner, FLAGS a_eFlag, TBOOL a_bSet );
	void SetShadowScaleImp( Toshi::TObject* a_pOwner, const Toshi::TVector2& a_rcScale );

	// AParticleEffectHandle finished callback, a_pUserData is the Shadow.
	static void __stdcall OnShadowEffectFinished( AParticleEffectHandle* a_pEffect, void* a_pUserData );

private:
	Shadow                m_aShadows[ MAX_NUM_SHADOWS ]; // 0x0004
	Toshi::TDList<Shadow> m_oUsedShadows;                // 0x232c
	Toshi::TDList<Shadow> m_oFreeShadows;                // 0x2334
	TINT                  m_iEffectIndex;                // 0x233c, AParticleEffectManager::FindEffect result of the last spawn
};

TSTATICASSERT( sizeof( AShadowManager::Shadow ) == 0x24 );
TSTATICASSERT( sizeof( AShadowManager ) == 0x2340 );
