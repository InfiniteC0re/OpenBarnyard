#pragma once
#include <Toshi/TObject.h>

class AUnit;

// Base class of the scripted unit effects (ADieAniEffect, AFadeAniEffect, ATrackBoneAniEffect...)
// owned by AAniEffectManager. Effects can be chained: when an effect finishes, the next one
// becomes active.
//
// TClass 0x007b4d1c (size 0xc, alignment 4), vtable 0x00739400.
class AAniEffect : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AAniEffect, Toshi::TObject );

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE   = 0,
		FLAGS_ACTIVE = BITFLAG( 0 ),
		FLAGS_DEAD   = BITFLAG( 1 ), // queued for deletion by AAniEffectManager
	};

public:
	AAniEffect();
	~AAniEffect();

	virtual AAniEffect* GetNextEffect() { return m_pNextEffect; }       // +0x0c, shared 006d9d10 (returns the field at +0x4)
	virtual AUnit*      GetUnit() { return TNULL; }                     // +0x10, RETURNS_ZERO
	virtual TBOOL       Update( TFLOAT a_fDeltaTime ) { return TTRUE; } // +0x14, RETURNS_TRUE 00413920. Returns TFALSE once the effect has finished

	// The unit of the effect can be destroyed by AAniEffectManager to make room for new effects
	virtual TBOOL IsUnitDisposable() { return TFALSE; } // +0x18, RETURNS_FALSE

	// Called by AAniEffectManager::CreateEffect once the effect is registered
	// NOTE: Every subclass overrides it with a jump to the base (0042c2b0).
	virtual TBOOL OnCreate() { return TTRUE; } // +0x1c, RETURNS_TRUE 0046cde0

	virtual void Unk_0x20() {} // +0x20, DOES_NOTHING
	virtual void Unk_0x24() {} // +0x24, DOES_NOTHING

	void SetActive( TBOOL a_bActive );

	// Makes a_pNextEffect wait until this effect finishes
	void SetNextEffect( AAniEffect* a_pNextEffect );

	// Activates the next effect and queues this one for deletion
	void Finish();

	TBOOL IsActive() const { return m_eFlags & FLAGS_ACTIVE; }
	TBOOL IsDead() const { return m_eFlags & FLAGS_DEAD; }

private:
	friend class AAniEffectManager;

	AAniEffect* m_pNextEffect; // 0x4
	FLAGS       m_eFlags;      // 0x8
};

TSTATICASSERT( sizeof( AAniEffect ) == 0xc );
