#include "pch.h"
#include "AParticleSystem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The random generators of the particle functions
static TRandom s_oInitRandom;           // 0x007cdc78
static TRandom s_oFloatRangeRandom;     // 0x007cda18
static TRandom s_oDwordRangeRandom;     // 0x007cdab0
static TRandom s_oColourRangeRandom;    // 0x007cd980
static TRandom s_oRandomForceRandom;    // 0x007cd898
static TRandom s_oRandomVelocityRandom; // 0x007cdd10

// Parameters of the functions (they follow AParticleFunctionData)
struct AParticleVectorParams
{
	AParticleFunctionData oHeader;       // 0x0
	TFLOAT                afValues[ 9 ]; // 0xc
};

struct AParticleDwordParams
{
	AParticleFunctionData oHeader;        // 0x0
	TUINT32               auiValues[ 2 ]; // 0xc
};

struct AParticleColourParams
{
	AParticleFunctionData oHeader;     // 0x0
	TUINT8                auiMin[ 4 ]; // 0xc
	TUINT8                auiMax[ 4 ]; // 0x10
};

//-----------------------------------------------------------------------------
// DOES_NOTHING callbacks shared by the functions
//-----------------------------------------------------------------------------

// DOES_NOTHING (0054df00)
static void __stdcall KeyframeOnStartDoesNothing( AParticleKeyframe* a_pKeyframe, AParticleKeyframe* a_pPrevious )
{
}

// DOES_NOTHING (00673660)
static void __stdcall KeyframeUpdateDoesNothing( AParticleKeyframe* a_pKeyframe, TFLOAT a_fDeltaTime, TFLOAT a_fProgress, AParticleTimelineState* a_pState )
{
}

// DOES_NOTHING (00673660)
static void __stdcall KeyframeApplyDoesNothing( AParticleKeyframe* a_pKeyframe, AParticleGenericEmitter* a_pEmitter, TFLOAT a_fProgress, TINT a_iParticle )
{
}

// DOES_NOTHING (00539d40)
static void __stdcall AffectorDoesNothing( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
}

//-----------------------------------------------------------------------------
// Initialisers
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00675800
static void __stdcall InitVectorFixed( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		rAttribute[ i ].Set( pParams->afValues[ 0 ], pParams->afValues[ 1 ], pParams->afValues[ 2 ], 1.0f );
}

// $Barnyard: FUNCTION 00675890
static void __stdcall InitVectorInBox( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fMinX  = pParams->afValues[ 0 ];
	TFLOAT fMinY  = pParams->afValues[ 1 ];
	TFLOAT fMinZ  = pParams->afValues[ 2 ];
	TFLOAT fSizeX = pParams->afValues[ 4 ] - fMinX;
	TFLOAT fSizeY = pParams->afValues[ 5 ] - fMinY;
	TFLOAT fSizeZ = pParams->afValues[ 6 ] - fMinZ;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVector = rAttribute[ i ];
		rVector.x         = s_oInitRandom.GetFloat() * fSizeX + fMinX;
		rVector.y         = s_oInitRandom.GetFloat() * fSizeY + fMinY;
		rVector.z         = s_oInitRandom.GetFloat() * fSizeZ + fMinZ;
		rVector.w         = 1.0f;
	}
}

// $Barnyard: FUNCTION 00675a10
static void __stdcall InitVectorOnSphere( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVector = rAttribute[ i ];
		rVector.x         = s_oInitRandom.GetFloat2();
		rVector.y         = s_oInitRandom.GetFloat2();
		rVector.z         = s_oInitRandom.GetFloat2();
		rVector.Normalise();
		rVector.Multiply( pParams->afValues[ 0 ] );
		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00675b60
static void __stdcall InitVectorInSphere( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVector = rAttribute[ i ];
		rVector.x         = s_oInitRandom.GetFloat2();
		rVector.y         = s_oInitRandom.GetFloat2();
		rVector.z         = s_oInitRandom.GetFloat2();
		rVector.Normalise();
		rVector.Multiply( TFLOAT( s_oInitRandom.RandRaw() >> 1 ) * pParams->afValues[ 0 ] * 4.6566129e-10f );
		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00676360
static void __stdcall InitVectorFromEmitter( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );
	AParticleGenericEmitter*  pEmitter   = AParticleEmitterSettings::ms_pInitEmitter;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		// Towards the centre of the emitter
		const TVector4& rcPosition = pEmitter->m_aPositions[ i ];
		TVector4&       rVector    = rAttribute[ i ];

		rVector.Set( -rcPosition.x, -rcPosition.y, -rcPosition.z, rcPosition.w );
		rVector.Normalise();
		rVector.Multiply( pParams->afValues[ 0 ] );
		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00676400
static void __stdcall InitVectorToEmitter( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );
	AParticleGenericEmitter*  pEmitter   = AParticleEmitterSettings::ms_pInitEmitter;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		// Away from the centre of the emitter
		TVector4& rVector = rAttribute[ i ];
		rVector           = pEmitter->m_aPositions[ i ];
		rVector.Normalise();
		rVector.Multiply( pParams->afValues[ 0 ] );
		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00676490
static void __stdcall InitVectorInCone( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fRadius   = pParams->afValues[ 0 ];
	TFLOAT fHeight   = pParams->afValues[ 1 ];
	TFLOAT fMinSpeed = pParams->afValues[ 2 ];
	TFLOAT fMaxSpeed = pParams->afValues[ 3 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT    fDistance = s_oInitRandom.GetFloat();
		TVector4& rVector   = rAttribute[ i ];

		rVector.y = 0.0f;
		rVector.z = 0.0f;
		rVector.x = fDistance * fRadius;
		rVector.w = 1.0f;

		rVector.AsVector3().RotateY( s_oInitRandom.GetFloat2() * TMath::PI );
		rVector.y = fHeight;
		rVector.Normalise();
		rVector.Multiply( TFLOAT( s_oInitRandom.RandRaw() >> 1 ) * ( fMaxSpeed - fMinSpeed ) * 4.6566129e-10f + fMinSpeed );
		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00676620
static void __stdcall InitVectorInAngle( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fMinAngle   = pParams->afValues[ 0 ];
	TFLOAT fMaxAngle   = pParams->afValues[ 1 ];
	TFLOAT fMinSpeed   = pParams->afValues[ 2 ];
	TFLOAT fSpeedRange = pParams->afValues[ 3 ] - fMinSpeed;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT    fAngle  = TFLOAT( s_oInitRandom.RandRaw() >> 1 ) * ( fMaxAngle - fMinAngle ) * 4.6566129e-10f + fMinAngle;
		TVector4& rVector = rAttribute[ i ];

		rVector.Set( 0.0f, -1.0f, 0.0f, 1.0f );
		rVector.AsVector3().RotateZ( fAngle );
		rVector.AsVector3().RotateY( s_oInitRandom.GetFloat2() * TMath::PI );
		rVector.Normalise();
		rVector.Multiply( TFLOAT( s_oInitRandom.RandRaw() >> 1 ) * fSpeedRange * 4.6566129e-10f + fMinSpeed );
		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00676080
static void __stdcall InitVectorOnBox( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fX = pParams->afValues[ 0 ];
	TFLOAT fY = pParams->afValues[ 1 ];
	TFLOAT fZ = pParams->afValues[ 2 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVector = rAttribute[ i ];

		// Random point on a random face
		switch ( s_oInitRandom.GetInt( 6 ) )
		{
			case 0:
				rVector.x = s_oInitRandom.GetFloat2() * fX;
				rVector.y = s_oInitRandom.GetFloat2() * fY;
				rVector.z = fZ;
				break;
			case 1:
				rVector.x = s_oInitRandom.GetFloat2() * fX;
				rVector.y = s_oInitRandom.GetFloat2() * fY;
				rVector.z = -fZ;
				break;
			case 2:
				rVector.x = s_oInitRandom.GetFloat2() * fX;
				rVector.y = fY;
				rVector.z = s_oInitRandom.GetFloat2() * fZ;
				break;
			case 3:
				rVector.x = s_oInitRandom.GetFloat2() * fX;
				rVector.y = -fY;
				rVector.z = s_oInitRandom.GetFloat2() * fZ;
				break;
			case 4:
				rVector.x = fX;
				rVector.y = s_oInitRandom.GetFloat2() * fY;
				rVector.z = s_oInitRandom.GetFloat2() * fZ;
				break;
			case 5:
				// NOTE: the original uses 0-1 instead of -1-1 for this face
				rVector.x = -fX;
				rVector.y = s_oInitRandom.GetFloat() * fY;
				rVector.z = s_oInitRandom.GetFloat() * fZ;
				break;
		}

		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00675cf0
static void __stdcall InitVectorInCircle( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );
	TFLOAT                    fRadius    = pParams->afValues[ 0 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVector = rAttribute[ i ];
		rVector.Set( fRadius, 0.0f, 0.0f, 1.0f );
		rVector.x = TFLOAT( s_oInitRandom.RandRaw() >> 1 ) * fRadius * 4.6566129e-10f;
		rVector.AsVector3().RotateY( s_oInitRandom.GetFloat2() * TMath::PI );
		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00675e40
static void __stdcall InitVectorOnCircle( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );
	TFLOAT                    fRadius    = pParams->afValues[ 0 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVector = rAttribute[ i ];
		rVector.Set( fRadius, 0.0f, 0.0f, 1.0f );
		rVector.AsVector3().RotateY( s_oInitRandom.GetFloat2() * TMath::PI );
		rVector.w = 1.0f;
	}
}

// $Barnyard: FUNCTION 00675f40
static void __stdcall InitVectorInDome( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleVectorAttribute& rAttribute = *TSTATICCAST( AParticleVectorAttribute, a_pAttribute );
	AParticleVectorParams*    pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );
	TFLOAT                    fRadius    = pParams->afValues[ 0 ];
	TFLOAT                    fHeight    = pParams->afValues[ 1 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT    fDistance = s_oInitRandom.GetFloat();
		TVector4& rVector   = rAttribute[ i ];

		rVector.y = 0.0f;
		rVector.z = 0.0f;
		rVector.w = 1.0f;
		rVector.x = fDistance * fRadius;
		rVector.AsVector3().RotateY( s_oInitRandom.GetFloat2() * TMath::PI );
		rVector.w = 1.0f;

		// Higher towards the centre
		rVector.y = ( 1.0f - ( 1.0f - fDistance ) * ( 1.0f - fDistance ) ) * fHeight + rVector.y;
	}
}

// $Barnyard: FUNCTION 00675430
static void __stdcall InitFloatFixed( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleFloatAttribute& rAttribute = *TSTATICCAST( AParticleFloatAttribute, a_pAttribute );
	AParticleVectorParams*   pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		rAttribute[ i ] = pParams->afValues[ 0 ];
}

// $Barnyard: FUNCTION 00675530
static void __stdcall InitFloatRange( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleFloatAttribute& rAttribute = *TSTATICCAST( AParticleFloatAttribute, a_pAttribute );
	AParticleVectorParams*   pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fMin   = pParams->afValues[ 0 ];
	TFLOAT fRange = pParams->afValues[ 1 ] - fMin;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		rAttribute[ i ] = TFLOAT( s_oFloatRangeRandom.RandRaw() >> 1 ) * fRange * 4.6566129e-10f + fMin;
}

// $Barnyard: FUNCTION 006755e0
static void __stdcall InitFloatFromSize( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleFloatAttribute& rAttribute = *TSTATICCAST( AParticleFloatAttribute, a_pAttribute );
	AParticleVectorParams*   pParams    = TREINTERPRETCAST( AParticleVectorParams*, a_pData );
	AParticleGenericEmitter* pEmitter   = AParticleEmitterSettings::ms_pInitEmitter;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		rAttribute[ i ] = pParams->afValues[ 0 ] * pEmitter->m_aSizes[ i ];
}

// $Barnyard: FUNCTION 00675750
static void __stdcall InitDwordFixed( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleDwordAttribute& rAttribute = *TSTATICCAST( AParticleDwordAttribute, a_pAttribute );
	AParticleDwordParams*    pParams    = TREINTERPRETCAST( AParticleDwordParams*, a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		rAttribute[ i ] = pParams->auiValues[ 0 ];
}

// $Barnyard: FUNCTION 006757a0
static void __stdcall InitDwordRange( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleDwordAttribute& rAttribute = *TSTATICCAST( AParticleDwordAttribute, a_pAttribute );
	AParticleDwordParams*    pParams    = TREINTERPRETCAST( AParticleDwordParams*, a_pData );

	TUINT32 uiMin   = pParams->auiValues[ 0 ];
	TUINT32 uiRange = pParams->auiValues[ 1 ] - uiMin;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
		rAttribute[ i ] = uiMin + s_oDwordRangeRandom.GetInt( uiRange );
}

// $Barnyard: FUNCTION 006752d0
static void __stdcall InitColourRange( void* a_pAttribute, TINT a_iStart, TINT a_iEnd, AParticleFunctionData* a_pData )
{
	AParticleDwordAttribute& rAttribute = *TSTATICCAST( AParticleDwordAttribute, a_pAttribute );
	AParticleColourParams*   pParams    = TREINTERPRETCAST( AParticleColourParams*, a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT fProgress = s_oColourRangeRandom.GetFloat();

		TUINT32 uiB = TUINT8( TINT( TMath::LERP( pParams->auiMin[ 0 ], pParams->auiMax[ 0 ], fProgress ) ) );
		TUINT32 uiG = TUINT8( TINT( TMath::LERP( pParams->auiMin[ 1 ], pParams->auiMax[ 1 ], fProgress ) ) );
		TUINT32 uiR = TUINT8( TINT( TMath::LERP( pParams->auiMin[ 2 ], pParams->auiMax[ 2 ], fProgress ) ) );
		TUINT32 uiA = TUINT8( TINT( TMath::LERP( pParams->auiMin[ 3 ], pParams->auiMax[ 3 ], fProgress ) ) );

		rAttribute[ i ] = uiB | ( uiG << 8 ) | ( uiR << 16 ) | ( uiA << 24 );
	}
}

// $Barnyard: FUNCTION 00675290
static void ConvertColourFixed( AParticleDwordParams* a_pParams )
{
	a_pParams->auiValues[ 0 ] = AParticleInitData::Convert( a_pParams->auiValues[ 0 ] );
}

// $Barnyard: FUNCTION 006752b0
static void ConvertColourRange( AParticleDwordParams* a_pParams )
{
	a_pParams->auiValues[ 0 ] = AParticleInitData::Convert( a_pParams->auiValues[ 0 ] );
	a_pParams->auiValues[ 1 ] = AParticleInitData::Convert( a_pParams->auiValues[ 1 ] );
}

// $Barnyard: FUNCTION 00675190
void AParticleInit::Resolve()
{
	switch ( pData->iType )
	{
		case TYPE_VECTOR_FIXED:
			fnInit = InitVectorFixed;
			break;
		case TYPE_VECTOR_IN_BOX:
			fnInit = InitVectorInBox;
			break;
		case TYPE_VECTOR_ON_SPHERE:
		case TYPE_VECTOR_ON_SPHERE_2:
			fnInit = InitVectorOnSphere;
			break;
		case TYPE_VECTOR_FROM_EMITTER:
			fnInit = InitVectorFromEmitter;
			break;
		case TYPE_VECTOR_TO_EMITTER:
			fnInit = InitVectorToEmitter;
			break;
		case TYPE_VECTOR_IN_CONE:
			fnInit = InitVectorInCone;
			break;
		case TYPE_VECTOR_IN_ANGLE:
			fnInit = InitVectorInAngle;
			break;
		case TYPE_VECTOR_IN_SPHERE:
			fnInit = InitVectorInSphere;
			break;
		case TYPE_VECTOR_ON_BOX:
			fnInit = InitVectorOnBox;
			break;
		case TYPE_VECTOR_IN_CIRCLE:
			fnInit = InitVectorInCircle;
			break;
		case TYPE_VECTOR_ON_CIRCLE:
			fnInit = InitVectorOnCircle;
			break;
		case TYPE_VECTOR_IN_DOME:
			fnInit = InitVectorInDome;
			break;
		case TYPE_FLOAT_FIXED:
			fnInit = InitFloatFixed;
			break;
		case TYPE_FLOAT_RANGE:
			fnInit = InitFloatRange;
			break;
		case TYPE_FLOAT_FROM_SIZE:
			fnInit = InitFloatFromSize;
			break;
		case TYPE_DWORD_FIXED:
			fnInit = InitDwordFixed;
			break;
		case TYPE_DWORD_RANGE:
			fnInit = InitDwordRange;
			break;
		case TYPE_COLOUR_FIXED:
			fnInit = InitDwordFixed;
			ConvertColourFixed( TREINTERPRETCAST( AParticleDwordParams*, pData ) );
			break;
		case TYPE_COLOUR_RANGE:
			fnInit = InitColourRange;
			ConvertColourRange( TREINTERPRETCAST( AParticleDwordParams*, pData ) );
			break;
	}
}

// $Barnyard: FUNCTION 006733a0
void AParticleEmitterSettings::InitParticles( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TBOOL a_bInitLife )
{
	if ( a_iStart == a_iEnd )
		return;

	ms_pInitEmitter = a_pEmitter;

	AParticleInit* pInit = apInits[ INIT_POSITION ];
	if ( pInit && HASANYFLAG( uiAttributes, AParticleGenericEmitter::FLAGS_POSITION ) )
		pInit->fnInit( &a_pEmitter->m_aPositions, a_iStart, a_iEnd, pInit->pData );

	pInit = apInits[ INIT_VELOCITY ];
	if ( pInit && HASANYFLAG( uiAttributes, AParticleGenericEmitter::FLAGS_VELOCITY ) )
		pInit->fnInit( &a_pEmitter->m_aVelocities, a_iStart, a_iEnd, pInit->pData );

	pInit = apInits[ INIT_LIFE ];
	if ( pInit && a_bInitLife == TTRUE && HASANYFLAG( uiAttributes, AParticleGenericEmitter::FLAGS_LIFE ) )
	{
		pInit->fnInit( &a_pEmitter->m_aAgeRates, a_iStart, a_iEnd, pInit->pData );

		for ( TINT i = a_iStart; i < a_iEnd; i++ )
			a_pEmitter->m_aAges[ i ] = 0.0f;
	}

	pInit = apInits[ INIT_SIZE ];
	if ( pInit && HASANYFLAG( uiAttributes, AParticleGenericEmitter::FLAGS_SIZE ) )
	{
		pInit->fnInit( &a_pEmitter->m_aSizes, a_iStart, a_iEnd, pInit->pData );

		if ( apTimelines[ TIMELINE_SIZE ] )
		{
			for ( TINT i = a_iStart; i < a_iEnd; i++ )
				a_pEmitter->m_aSizeKeys[ i ] = 0;
		}
	}

	pInit = apInits[ INIT_MASS ];
	if ( pInit && HASANYFLAG( uiAttributes, AParticleGenericEmitter::FLAGS_MASS ) )
		pInit->fnInit( &a_pEmitter->m_aMasses, a_iStart, a_iEnd, pInit->pData );

	pInit = apInits[ INIT_UV ];
	if ( pInit && HASANYFLAG( uiAttributes, AParticleGenericEmitter::FLAGS_UV ) )
		pInit->fnInit( &a_pEmitter->m_aUVFrames, a_iStart, a_iEnd, pInit->pData );

	pInit = apInits[ INIT_COLOUR ];
	if ( pInit && HASANYFLAG( uiAttributes, AParticleGenericEmitter::FLAGS_COLOUR ) )
	{
		pInit->fnInit( &a_pEmitter->m_aColours, a_iStart, a_iEnd, pInit->pData );

		if ( apTimelines[ TIMELINE_COLOUR ] )
		{
			for ( TINT i = a_iStart; i < a_iEnd; i++ )
				a_pEmitter->m_aColourKeys[ i ] = 0;
		}
	}

	if ( apTimelines[ TIMELINE_EVENTS ] )
	{
		for ( TINT i = a_iStart; i < a_iEnd; i++ )
			a_pEmitter->m_aEventKeys[ i ] = 0;
	}

	ms_pInitEmitter = TNULL;
}

//-----------------------------------------------------------------------------
// Affectors
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00673d40
static void __stdcall AffectorMove( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pEmitter->m_aPositions[ i ];

		TVector4 vecMove;
		vecMove.Multiply( a_pEmitter->m_aVelocities[ i ], a_fDeltaTime );

		rPosition.x = vecMove.x + rPosition.x;
		rPosition.y = vecMove.y + rPosition.y;
		rPosition.z = vecMove.z + rPosition.z;
	}
}

// $Barnyard: FUNCTION 006739d0
static void __stdcall AffectorDrag( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fDrag = a_fDeltaTime * pParams->afValues[ 0 ];
	TMath::Clip( fDrag, -1.0f, 1.0f );

	TVector4 vecDrag( fDrag, fDrag, fDrag, 0.0f );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVelocity = a_pEmitter->m_aVelocities[ i ];

		TVector4 vecLoss;
		vecLoss.Multiply( rVelocity, vecDrag );

		rVelocity.x = rVelocity.x - vecLoss.x;
		rVelocity.y = rVelocity.y - vecLoss.y;
		rVelocity.z = rVelocity.z - vecLoss.z;
	}
}

// $Barnyard: FUNCTION 006738d0
static void __stdcall AffectorDamping( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams   = TREINTERPRETCAST( AParticleVectorParams*, a_pData );
	TFLOAT                 fDamping  = 1.0f - a_fDeltaTime * pParams->afValues[ 0 ];
	TFLOAT                 fMinSpeed = pParams->afValues[ 1 ];

	if ( fMinSpeed <= 0.0f )
	{
		for ( TINT i = a_iStart; i < a_iEnd; i++ )
			a_pEmitter->m_aVelocities[ i ].Multiply( fDamping );
	}
	else
	{
		// Don't slow down below the min speed
		TFLOAT fMinSpeedSq = fMinSpeed * fMinSpeed;

		for ( TINT i = a_iStart; i < a_iEnd; i++ )
		{
			TVector4& rVelocity = a_pEmitter->m_aVelocities[ i ];
			TFLOAT    fSpeedSq  = rVelocity.MagnitudeSq();

			if ( fSpeedSq > fMinSpeedSq )
			{
				TFLOAT fSpeed    = TMath::Sqrt( fSpeedSq );
				TFLOAT fNewSpeed = fDamping * fSpeed;

				rVelocity.Multiply( ( ( fNewSpeed >= fMinSpeed ) ? fNewSpeed : fMinSpeed ) / fSpeed );
			}
		}
	}
}

// $Barnyard: FUNCTION 00673a90
static void __stdcall AffectorForce( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TVector4 vecForce( pParams->afValues[ 0 ], pParams->afValues[ 1 ], pParams->afValues[ 2 ], 0.0f );
	vecForce.Multiply( a_fDeltaTime * pParams->afValues[ 3 ] );

	TFLOAT fGravity = a_fDeltaTime * pParams->afValues[ 4 ];

	if ( vecForce.x > 0.01f || vecForce.y > 0.01f || vecForce.z > 0.01f ||
	     vecForce.x < -0.01f || vecForce.y < -0.01f || vecForce.z < -0.01f )
	{
		for ( TINT i = a_iStart; i < a_iEnd; i++ )
		{
			TVector4& rVelocity = a_pEmitter->m_aVelocities[ i ];

			TVector4 vecAcceleration;
			vecAcceleration.Divide( vecForce, a_pEmitter->m_aMasses[ i ] );
			vecAcceleration.y += fGravity;

			rVelocity.x = vecAcceleration.x + rVelocity.x;
			rVelocity.y = vecAcceleration.y + rVelocity.y;
			rVelocity.z = vecAcceleration.z + rVelocity.z;
		}
	}
	else
	{
		for ( TINT i = a_iStart; i < a_iEnd; i++ )
			a_pEmitter->m_aVelocities[ i ].y += fGravity;
	}
}

// $Barnyard: FUNCTION 00673ed0
static void __stdcall AffectorRandomForce( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fMinX   = pParams->afValues[ 0 ];
	TFLOAT fMinY   = pParams->afValues[ 1 ];
	TFLOAT fMinZ   = pParams->afValues[ 2 ];
	TFLOAT fRangeX = pParams->afValues[ 3 ] - fMinX;
	TFLOAT fRangeZ = pParams->afValues[ 5 ] - fMinZ;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVelocity = a_pEmitter->m_aVelocities[ i ];

		TVector4 vecForce;
		vecForce.x = TFLOAT( s_oRandomForceRandom.RandRaw() >> 1 ) * fRangeX * 4.6566129e-10f + fMinX;
		vecForce.y = TFLOAT( s_oRandomForceRandom.RandRaw() >> 1 ) * ( pParams->afValues[ 4 ] - fMinY ) * 4.6566129e-10f + fMinY;
		vecForce.z = TFLOAT( s_oRandomForceRandom.RandRaw() >> 1 ) * fRangeZ * 4.6566129e-10f + fMinZ;
		vecForce.Multiply( a_fDeltaTime / a_pEmitter->m_aMasses[ i ] );

		rVelocity.x = vecForce.x + rVelocity.x;
		rVelocity.y = vecForce.y + rVelocity.y;
		rVelocity.z = vecForce.z + rVelocity.z;
	}
}

// $Barnyard: FUNCTION 00674710
static void __stdcall AffectorRotateVelocity( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector3& rVelocity = a_pEmitter->m_aVelocities[ i ].AsVector3();
		rVelocity.RotateX( pParams->afValues[ 0 ] );
		rVelocity.RotateY( pParams->afValues[ 1 ] );
		rVelocity.RotateZ( pParams->afValues[ 2 ] );
	}
}

// $Barnyard: FUNCTION 00673dd0
static void __stdcall AffectorAttract( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fX = pParams->afValues[ 0 ];
	TFLOAT fY = pParams->afValues[ 1 ];
	TFLOAT fZ = pParams->afValues[ 2 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		const TVector4& rcPosition = a_pEmitter->m_aPositions[ i ];
		TVector4&       rVelocity  = a_pEmitter->m_aVelocities[ i ];

		TVector4 vecDirection( fX - rcPosition.x, fY - rcPosition.y, fZ - rcPosition.z, 0.0f );
		vecDirection.Normalise();
		vecDirection.Multiply( a_fDeltaTime * pParams->afValues[ 3 ] );

		TVector4 vecAcceleration;
		vecAcceleration.Divide( vecDirection, a_pEmitter->m_aMasses[ i ] );

		rVelocity.x = vecAcceleration.x + rVelocity.x;
		rVelocity.y = vecAcceleration.y + rVelocity.y;
		rVelocity.z = vecAcceleration.z + rVelocity.z;
	}
}

// $Barnyard: FUNCTION 006740a0
static void __stdcall AffectorRotateX( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fSin;
	TFLOAT fCos;
	TMath::SinCos( a_fDeltaTime * pParams->afValues[ 0 ], fSin, fCos );

	// NOTE: the original rotates around ( x, y ) of the emitter's position
	TFLOAT fCentreZ = a_pEmitter->m_mTransform.m_f42;
	TFLOAT fCentreY = a_pEmitter->m_mTransform.m_f41;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pEmitter->m_aPositions[ i ];

		TFLOAT fY   = rPosition.y - fCentreY;
		rPosition.y = fY;
		TFLOAT fZ   = rPosition.z - fCentreZ;
		rPosition.z = fZ;

		TFLOAT fNewZ = fCos * fZ + fY * fSin;
		rPosition.y  = fY * fCos - fZ * fSin + fCentreY;
		rPosition.z  = fNewZ + fCentreZ;
	}
}

// $Barnyard: FUNCTION 00674310
static void __stdcall AffectorRotateY( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fSin;
	TFLOAT fCos;
	TMath::SinCos( a_fDeltaTime * pParams->afValues[ 0 ], fSin, fCos );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pEmitter->m_aPositions[ i ];

		TFLOAT fNewZ = fCos * rPosition.z - fSin * rPosition.x;
		rPosition.x  = fCos * rPosition.x + fSin * rPosition.z;
		rPosition.z  = fNewZ;
	}
}

// $Barnyard: FUNCTION 006744b0
static void __stdcall AffectorRotateZ( AParticleGenericEmitter* a_pEmitter, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, AParticleFunctionData* a_pData )
{
	AParticleVectorParams* pParams = TREINTERPRETCAST( AParticleVectorParams*, a_pData );

	TFLOAT fSin;
	TFLOAT fCos;
	TMath::SinCos( a_fDeltaTime * pParams->afValues[ 0 ], fSin, fCos );

	TFLOAT fCentreY = a_pEmitter->m_mTransform.m_f42;
	TFLOAT fCentreX = a_pEmitter->m_mTransform.m_f41;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pEmitter->m_aPositions[ i ];

		TFLOAT fX   = rPosition.x - fCentreX;
		rPosition.x = fX;
		TFLOAT fY   = rPosition.y - fCentreY;
		rPosition.y = fY;

		TFLOAT fNewY = fCos * fY + fX * fSin;
		rPosition.x  = fX * fCos - fY * fSin + fCentreX;
		rPosition.y  = fNewY + fCentreY;
	}
}

// $Barnyard: FUNCTION 00673830
void AParticleAffector::Resolve()
{
	switch ( pData->iType )
	{
		case TYPE_MOVE:
			fnAffector = AffectorMove;
			break;
		case TYPE_DRAG:
			fnAffector = AffectorDrag;
			break;
		case TYPE_DAMPING:
			fnAffector = AffectorDamping;
			break;
		case TYPE_FORCE:
			fnAffector = AffectorForce;
			break;
		case TYPE_RANDOM_FORCE:
			fnAffector = AffectorRandomForce;
			break;
		case TYPE_ROTATE_VELOCITY:
			fnAffector = AffectorRotateVelocity;
			break;
		case TYPE_ATTRACT:
			fnAffector = AffectorAttract;
			break;
		case TYPE_ROTATE_X:
			fnAffector = AffectorRotateX;
			break;
		case TYPE_ROTATE_Y:
			fnAffector = AffectorRotateY;
			break;
		case TYPE_ROTATE_Z:
			fnAffector = AffectorRotateZ;
			break;
		case TYPE_NONE:
			fnAffector = AffectorDoesNothing;
			break;
	}
}

//-----------------------------------------------------------------------------
// Keyframes
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00676df0
static void __stdcall KeyframeSetPrevious( AParticleKeyframe* a_pKeyframe, AParticleKeyframe* a_pPrevious )
{
	a_pKeyframe->pPrevious = a_pPrevious;
}

// $Barnyard: FUNCTION 00676970
static void __stdcall KeyframeApplyColour( AParticleKeyframe* a_pKeyframe, AParticleGenericEmitter* a_pEmitter, TFLOAT a_fProgress, TINT a_iParticle )
{
	AParticleKeyframe* pCurrent = TREINTERPRETCAST( AParticleKeyframe*, a_pEmitter->m_aColourKeys[ a_iParticle ] );

	if ( pCurrent != a_pKeyframe )
	{
		// The particle reached the colour of the previous key
		if ( pCurrent && pCurrent->pData )
			a_pEmitter->m_aColours[ a_iParticle ] = TREINTERPRETCAST( AParticleColourKeyframeData*, pCurrent->pData )->uiColour;

		a_pEmitter->m_aColourKeys[ a_iParticle ] = TREINTERPRETCAST( TUINT32, a_pKeyframe );
	}
}

// $Barnyard: FUNCTION 00676d30
static void __stdcall KeyframeApplyMass( AParticleKeyframe* a_pKeyframe, AParticleGenericEmitter* a_pEmitter, TFLOAT a_fProgress, TINT a_iParticle )
{
	TFLOAT fMass;

	if ( a_pKeyframe->pPrevious )
		fMass = TMath::LERP( a_pKeyframe->pPrevious->pData->fValue, a_pKeyframe->pData->fValue, a_fProgress );
	else
		fMass = TMath::LERP( a_pEmitter->m_aMasses[ a_iParticle ], a_pKeyframe->pData->fValue, a_fProgress );

	if ( fMass == 0.0f )
		fMass = 0.01f;

	a_pEmitter->m_aMasses[ a_iParticle ] = fMass;
}

// $Barnyard: FUNCTION 00676e00
static void __stdcall KeyframeApplySize( AParticleKeyframe* a_pKeyframe, AParticleGenericEmitter* a_pEmitter, TFLOAT a_fProgress, TINT a_iParticle )
{
	AParticleKeyframe* pCurrent = TREINTERPRETCAST( AParticleKeyframe*, a_pEmitter->m_aSizeKeys[ a_iParticle ] );

	if ( pCurrent != a_pKeyframe )
	{
		// The particle reached the size of the previous key
		if ( pCurrent )
		{
			if ( pCurrent->IsType( AParticleKeyframe::TYPE_SIZE_SCALE ) )
				a_pEmitter->m_aSizes[ a_iParticle ] = pCurrent->pData->fValue * a_pEmitter->m_aSizes[ a_iParticle ];
			else
				a_pEmitter->m_aSizes[ a_iParticle ] = pCurrent->pData->fValue;
		}

		a_pEmitter->m_aSizeKeys[ a_iParticle ] = TREINTERPRETCAST( TUINT32, a_pKeyframe );
	}
}

// $Barnyard: FUNCTION 00677410
static void __stdcall KeyframeApplyUV( AParticleKeyframe* a_pKeyframe, AParticleGenericEmitter* a_pEmitter, TFLOAT a_fProgress, TINT a_iParticle )
{
	if ( a_pKeyframe->pPrevious )
		a_pEmitter->m_aUVFrames[ a_iParticle ] = TUINT32( a_pKeyframe->pPrevious->pData->fValue );
}

// $Barnyard: FUNCTION 00676cd0
static void __stdcall KeyframeApplyReverseVelocity( AParticleKeyframe* a_pKeyframe, AParticleGenericEmitter* a_pEmitter, TFLOAT a_fProgress, TINT a_iParticle )
{
	TVector4& rVelocity = a_pEmitter->m_aVelocities[ a_iParticle ];
	rVelocity.x         = -rVelocity.x;
	rVelocity.y         = -rVelocity.y;
	rVelocity.z         = -rVelocity.z;
}

// $Barnyard: FUNCTION 00676b60
static void __stdcall KeyframeApplyRandomVelocity( AParticleKeyframe* a_pKeyframe, AParticleGenericEmitter* a_pEmitter, TFLOAT a_fProgress, TINT a_iParticle )
{
	a_pEmitter->m_aVelocities[ a_iParticle ].x = s_oRandomVelocityRandom.GetFloat2();
	a_pEmitter->m_aVelocities[ a_iParticle ].y = s_oRandomVelocityRandom.GetFloat2();
	a_pEmitter->m_aVelocities[ a_iParticle ].z = s_oRandomVelocityRandom.GetFloat2();
}

// $Barnyard: FUNCTION 00673670
static void __stdcall KeyframeUpdateEmitter( AParticleKeyframe* a_pKeyframe, TFLOAT a_fDeltaTime, TFLOAT a_fProgress, AParticleTimelineState* a_pState )
{
	AParticleEmitterKeyframeData*     pData     = TREINTERPRETCAST( AParticleEmitterKeyframeData*, a_pKeyframe->pData );
	AParticleEffectInstance::Emitter* pEmitter  = TREINTERPRETCAST( AParticleEffectInstance::Emitter*, a_pState );
	TFLOAT                            fPrevious = 0.0f;

	if ( !pEmitter->pEmitter )
	{
		if ( pData->fNumParticles > 0.0f )
		{
			// Spawn the emitter
			AParticleSystem*         pSystem         = AParticleSystem::GetSingleton();
			AParticleGenericEmitter* pGenericEmitter = TSTATICCAST( AParticleGenericEmitter, pSystem->CreateEmitter( AParticleSystem::EMITTERTYPE_GENERIC ) );

			if ( !pGenericEmitter )
			{
				// Free the emitters of a stealable effect
				pSystem->GetEffectPool()->StealInstance();
			}
			else
			{
				AParticleEffectEmitter* pEffectEmitter = AParticleEffectInstance::ms_pCurrentEffectEmitter;
				pGenericEmitter->Setup( pData->pSettings );

				AParticleGenericEmitter::FLAGS eFlags = pGenericEmitter->GetFlags() | AParticleGenericEmitter::FLAGS_SPAWN_NEW | AParticleGenericEmitter::FLAGS_KEEP_ALIVE;

				if ( HASANYFLAG( pEffectEmitter->uiFlags, AParticleEffectEmitter::FLAGS_FOLLOW_POS ) )
					eFlags |= AParticleGenericEmitter::FLAGS_FOLLOW_POS;
				else if ( HASANYFLAG( pEffectEmitter->uiFlags, AParticleEffectEmitter::FLAGS_FOLLOW_TRANS ) )
					eFlags |= AParticleGenericEmitter::FLAGS_FOLLOW_TRANS;

				pGenericEmitter->SetFlags( eFlags );
				pGenericEmitter->SetTransform( AParticleEffectInstance::ms_mEmitterTransform );
				pGenericEmitter->m_pOnFinishedData = AParticleEffectInstance::ms_pCurrentInstance;
				pGenericEmitter->m_fnOnFinished    = AParticleEffectInstance::OnEmitterFinished;
				pGenericEmitter->Start();

				pEmitter->pEmitter = pGenericEmitter;
			}
		}

		if ( !pEmitter->pEmitter )
			return;
	}

	// Blend the number of particles from the previous key
	if ( pData->pPrevious )
		fPrevious = TREINTERPRETCAST( AParticleEmitterKeyframeData*, a_pKeyframe->pPrevious->pData )->fNumParticles;

	TMath::Clip( a_fProgress, 0.0f, 1.0f );

	TINT iNumParticles = TINT( pData->pSettings->iMaxParticles * TMath::LERP( fPrevious, pData->fNumParticles, a_fProgress ) );

	if ( pData->fNumParticles > 0.0f && TINT16( iNumParticles ) < 1 )
		iNumParticles = 1;

	pEmitter->pEmitter->SetTransform( AParticleEffectInstance::ms_mEmitterTransform );
	pEmitter->pEmitter->SetMaxParticles( TUINT16( iNumParticles ) );
}

// $Barnyard: FUNCTION 00676b30
static void __stdcall ResolveColourKeyframe( AParticleKeyframe* a_pKeyframe )
{
	a_pKeyframe->fnOnStart = KeyframeSetPrevious;
	a_pKeyframe->fnUpdate  = KeyframeUpdateDoesNothing;
	a_pKeyframe->fnApply   = KeyframeApplyColour;

	AParticleColourKeyframeData* pData = TREINTERPRETCAST( AParticleColourKeyframeData*, a_pKeyframe->pData );
	pData->uiColour                    = AParticleInitData::Convert( pData->uiColour );
}

// $Barnyard: FUNCTION 00676dd0
static void __stdcall ResolveMassKeyframe( AParticleKeyframe* a_pKeyframe )
{
	a_pKeyframe->fnOnStart = KeyframeOnStartDoesNothing;
	a_pKeyframe->fnUpdate  = KeyframeUpdateDoesNothing;
	a_pKeyframe->fnApply   = KeyframeApplyMass;
}

// $Barnyard: FUNCTION 00676fa0
static void __stdcall ResolveSizeKeyframe( AParticleKeyframe* a_pKeyframe )
{
	a_pKeyframe->fnOnStart = KeyframeSetPrevious;
	a_pKeyframe->fnUpdate  = KeyframeUpdateDoesNothing;
	a_pKeyframe->fnApply   = KeyframeApplySize;
}

// $Barnyard: FUNCTION 00677460
static void __stdcall ResolveUVKeyframe( AParticleKeyframe* a_pKeyframe )
{
	a_pKeyframe->fnOnStart = KeyframeOnStartDoesNothing;
	a_pKeyframe->fnUpdate  = KeyframeUpdateDoesNothing;
	a_pKeyframe->fnApply   = KeyframeApplyUV;
}

// $Barnyard: FUNCTION 006737e0
static void __stdcall ResolveEmitterKeyframe( AParticleKeyframe* a_pKeyframe )
{
	a_pKeyframe->fnOnStart = KeyframeOnStartDoesNothing;
	a_pKeyframe->fnUpdate  = KeyframeUpdateEmitter;
	a_pKeyframe->fnApply   = KeyframeApplyDoesNothing;
}

// $Barnyard: FUNCTION 00676d10
static void __stdcall ResolveReverseVelocityKeyframe( AParticleKeyframe* a_pKeyframe )
{
	a_pKeyframe->fnOnStart = KeyframeOnStartDoesNothing;
	a_pKeyframe->fnUpdate  = KeyframeUpdateDoesNothing;
	a_pKeyframe->fnApply   = KeyframeApplyReverseVelocity;
}

// $Barnyard: FUNCTION 00676cb0
static void __stdcall ResolveRandomVelocityKeyframe( AParticleKeyframe* a_pKeyframe )
{
	a_pKeyframe->fnOnStart = KeyframeOnStartDoesNothing;
	a_pKeyframe->fnUpdate  = KeyframeUpdateDoesNothing;
	a_pKeyframe->fnApply   = KeyframeApplyRandomVelocity;
}

// $Barnyard: FUNCTION 006768e0
void AParticleKeyframe::Resolve()
{
	fnOnStart = KeyframeOnStartDoesNothing;
	fnUpdate  = KeyframeUpdateDoesNothing;
	fnApply   = KeyframeApplyDoesNothing;
	pPrevious = TNULL;

	switch ( pData->iType )
	{
		case TYPE_COLOUR:
			ResolveColourKeyframe( this );
			break;
		case TYPE_MASS:
			ResolveMassKeyframe( this );
			break;
		case TYPE_SIZE:
		case TYPE_SIZE_SCALE:
			ResolveSizeKeyframe( this );
			break;
		case TYPE_UV:
			ResolveUVKeyframe( this );
			break;
		case TYPE_EMITTER:
			ResolveEmitterKeyframe( this );
			break;
		case TYPE_REVERSE_VELOCITY:
			ResolveReverseVelocityKeyframe( this );
			break;
		case TYPE_RANDOMISE_VELOCITY:
			ResolveRandomVelocityKeyframe( this );
			break;
	}
}

// $Barnyard: FUNCTION 006769f0
TUINT32 AParticleKeyframe::BlendColour( AParticleGenericEmitter* a_pEmitter, TINT a_iParticle )
{
	TFLOAT fProgress = a_pEmitter->m_aAges[ a_iParticle ];

	if ( pPrevious )
		fProgress = ( fProgress - pPrevious->pData->fTime ) / ( pData->fTime - pPrevious->pData->fTime );
	else
		fProgress = fProgress / pData->fTime;

	TMath::Clip( fProgress, 0.0f, 1.0f );
	fProgress = GetProgress( fProgress );

	TINT    iProgress = TINT( lrintf( fProgress * 255.0f ) );
	TUINT32 uiFrom    = a_pEmitter->m_aColours[ a_iParticle ];
	TUINT32 uiTo      = TREINTERPRETCAST( AParticleColourKeyframeData*, pData )->uiColour;

	TINT iFromB = uiFrom & 0xff;
	TINT iFromG = ( uiFrom >> 8 ) & 0xff;
	TINT iFromR = ( uiFrom >> 16 ) & 0xff;
	TINT iFromA = uiFrom >> 24;

	TINT iR = ( ( ( TINT( ( uiTo >> 16 ) & 0xff ) - iFromR ) * iProgress ) >> 8 ) + iFromR;
	TINT iG = ( ( ( TINT( ( uiTo >> 8 ) & 0xff ) - iFromG ) * iProgress ) >> 8 ) + iFromG;
	TINT iA = ( ( ( TINT( uiTo >> 24 ) - iFromA ) * iProgress ) >> 8 ) + iFromA;
	TINT iB = ( ( ( TINT( uiTo & 0xff ) - iFromB ) * iProgress ) >> 8 ) + iFromB;

	return ( ( iR << 16 ) & 0xff0000 ) | ( ( iG << 8 ) & 0xff00 ) | ( ( iA << 24 ) & 0xff000000 ) | ( iB & 0xff );
}

// $Barnyard: FUNCTION 00676eb0
TFLOAT AParticleKeyframe::BlendSize( AParticleGenericEmitter* a_pEmitter, TINT a_iParticle )
{
	TFLOAT fProgress = a_pEmitter->m_aAges[ a_iParticle ];

	if ( pPrevious )
		fProgress = ( fProgress - pPrevious->pData->fTime ) / ( pData->fTime - pPrevious->pData->fTime );
	else
		fProgress = fProgress / pData->fTime;

	TMath::Clip( fProgress, 0.0f, 1.0f );
	fProgress = GetProgress( fProgress );

	TFLOAT fFrom = a_pEmitter->m_aSizes[ a_iParticle ];
	TFLOAT fTo   = pData->fValue;

	if ( IsType( TYPE_SIZE_SCALE ) )
		fTo = fTo * fFrom;

	TFLOAT fSize = TMath::LERP( fFrom, fTo, fProgress );

	if ( fSize == 0.0f )
		return 0.01f;

	return fSize;
}
