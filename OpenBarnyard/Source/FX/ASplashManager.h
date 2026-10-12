#pragma once
#include "Animation/AModel.h"

#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

// Pool of the short-lived milk and water splash models (bike landing in a puddle, milk squirt hitting the ground...).
// Created by AFXManager. Singleton at 0x007b5f44.
class ASplashManager
    : public Toshi::TSingleton<ASplashManager>
{
public:
	enum SPLASHTYPE : TINT
	{
		SPLASHTYPE_MILK,
		SPLASHTYPE_WATER,
		SPLASHTYPE_NUMOF
	};

	static constexpr TINT MAX_NUM_SPLASHES = 16; // per type

	struct Splash
	{
		Splash();
		~Splash();

		// Releases and destroys the model instance.
		void Destroy();

		// Shows the splash at a_rcPosition (facing a_rcDirection, randomly rotated around it) with the size of a_fScale.
		void Spawn( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TFLOAT a_fScale );

		// Grows and fades the splash out, hides it once its time is over.
		void Update( TFLOAT a_fDeltaTime );

		// Instantiates the model of a_eType.
		void Create( SPLASHTYPE a_eType );

		AModelInstanceRef m_pModelInstance; // 0x0
		TFLOAT            m_fScale;         // 0x4
		TFLOAT            m_fTime;          // 0x8, time left, the splash is free when it's not positive
		SPLASHTYPE        m_eType;          // 0xc
	};

public:
	ASplashManager();
	virtual ~ASplashManager();

	void Update( TFLOAT a_fDeltaTime );

	// Returns a splash of a_eType that isn't used or TNULL if all of them are used.
	Splash* GetFreeSplash( SPLASHTYPE a_eType );

	// Shows a free splash of a_eType (if there's any).
	void Spawn( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection, TFLOAT a_fScale, SPLASHTYPE a_eType );

private:
	Splash m_aSplashes[ SPLASHTYPE_NUMOF ][ MAX_NUM_SPLASHES ]; // 0x4
};

TSTATICASSERT( sizeof( ASplashManager::Splash ) == 0x10 );
TSTATICASSERT( sizeof( ASplashManager ) == 516 );
