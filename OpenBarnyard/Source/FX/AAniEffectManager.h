#pragma once
#include "AAniEffect.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2Vector.h>
#include <Toshi/T2ObjectPool.h>

// Owns the AAniEffect objects (allocated from its own pool). Finished effects are moved
// to a dead list and deleted later.
//
// Singleton at 0x007b4d4c, vtable 0x00739484.
class AAniEffectManager
    : public Toshi::TSingleton<AAniEffectManager>
{
public:
	static constexpr TINT MAX_NUM_EFFECTS = 250;

public:
	AAniEffectManager();
	virtual ~AAniEffectManager();

	// Removes every effect (and the effects chained after it) of a_pUnit. Returns number of removed effects.
	TINT RemoveUnitEffects( AUnit* a_pUnit );

	// Removes a_pEffect and the effects chained after it. Returns number of removed effects.
	TINT RemoveEffect( AAniEffect* a_pEffect );

	// Queues a_pEffect for deletion
	void KillEffect( AAniEffect* a_pEffect );

	// Removes a_pEffect from the list of effects without deleting it
	void UnregisterEffect( AAniEffect* a_pEffect );

	// Returns the effect a_pEffect is chained after
	AAniEffect* FindPreviousEffect( AAniEffect* a_pEffect );

	void DeleteDeadEffects();
	void DeleteAllEffects();

	// Updates the active effects
	void Update( TFLOAT a_fDeltaTime );

	// Number of effects including the ones waiting for deletion
	TINT GetNumEffects() const { return m_vecEffects.Size() + m_vecDeadEffects.Size(); } // 0062fad0

	// Destroys the units (and removes their effects) of up to a_iNumEffects effects whose unit
	// can be disposed and isn't visible. Returns number of removed effects.
	TINT DisposeInvisibleUnits( TINT a_iNumEffects );

	// DisposeInvisibleUnits, then destroys the units of disposable effects even if visible
	// until a_iNumEffects effects are removed. Returns number of removed effects.
	TINT DisposeUnits( TINT a_iNumEffects );

	// Returns TTRUE if there's room for a_iNumEffects more effects, removing effects if needed
	TBOOL ReserveEffects( TINT a_iNumEffects );

	// Creates an effect of type T, TNULL if there's no room for it.
	// NOTE: the original has one copy of this function per effect class.
	template <class T>
	T* CreateEffect();

private:
	Toshi::T2Vector<AAniEffect*, MAX_NUM_EFFECTS> m_vecEffects;     // 0x4
	Toshi::T2Vector<AAniEffect*, MAX_NUM_EFFECTS> m_vecDeadEffects; // 0x3f0
	Toshi::T2DerivedDynamicObjectPool<AAniEffect> m_oEffectPool;    // 0x7dc
};

TSTATICASSERT( sizeof( AAniEffectManager ) == 0x7f4 );

// $Barnyard: FUNCTION 0052cf30
// $Barnyard: FUNCTION 00528d80
// $Barnyard: FUNCTION 0052cfa0
// $Barnyard: FUNCTION 00535040
// $Barnyard: FUNCTION 0046be60
// $Barnyard: FUNCTION 0046bdf0
// $Barnyard: FUNCTION 00625ba0
// $Barnyard: FUNCTION 005b88d0
// NOTE: AWaitAniEffect, AFadeAniEffect, ADieAniEffect, ATrackBoneAniEffect, AInventoryAniEffect, ASwirlTrackHudAniEffect, AHudTrackBoneAniEffect, ARotateAniEffect
template <class T>
inline T* AAniEffectManager::CreateEffect()
{
	if ( GetNumEffects() > MAX_NUM_EFFECTS - 1 )
		DisposeUnits( 1 );

	if ( GetNumEffects() == MAX_NUM_EFFECTS )
		return TNULL;

	// NOTE: retail allocates the size of T's TClass from the pool and calls TClass::CreateObjectInPlace
	T* pEffect = m_oEffectPool.NewDerivedObject<T>();
	m_vecEffects.PushBack( pEffect );
	pEffect->OnCreate();

	return pEffect;
}
