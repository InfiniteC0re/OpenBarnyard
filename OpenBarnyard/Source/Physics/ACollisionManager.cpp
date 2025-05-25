#include "pch.h"
#include "ACollisionManager.h"

#include <ToshiTools/T2DynamicVector.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ACollisionManager );

#define MAX_SOME_COUNT1 8
#define MAX_NODES 20

static T2DynamicVector<int>* s_pDynVector;
static TINT                  s_iSomeCount1;
static TINT                  s_aSomeArray1[ MAX_SOME_COUNT1 ];
static TINT                  s_iSomeCount2;
static TINT                  s_aSomeArray2[ MAX_SOME_COUNT1 ];

static TINT                     s_iUnk5;
static TINT                     s_TotalNumNodes;
static ACollisionManager::Node* s_pNodes;
static TBOOL                    s_bFlag1;
static TINT                     s_iUnk4;
static TBOOL                    s_bFlag2;
static TBOOL                    s_bFlag3;

// $Barnyard: FUNCTION 006166b0
ACollisionManager::ACollisionManager()
{
	field2_0x28    = 0;
	field403_0x1bc = 0.0f;
	field404_0x1c0 = 1.0f / 60.0f;
	field405_0x1c4 = true;
}

// $Barnyard: FUNCTION 00616740
// $Barnyard: FUNCTION 00616190
ACollisionManager::~ACollisionManager()
{

}

// $Barnyard: FUNCTION 00616140
TBOOL ACollisionManager::Reset()
{
	return BaseClass::Reset();
}

// $Barnyard: FUNCTION 00616080
TBOOL ACollisionManager::OnCreate()
{
	CreateStatic();
	field403_0x1bc = 0.0f;
	field404_0x1c0 = 1.0f / 60.0f;

	return BaseClass::OnCreate();
}

// $Barnyard: FUNCTION 00616760
TBOOL ACollisionManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	TIMPLEMENT();

	return BaseClass::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 006160b0
void ACollisionManager::OnDestroy()
{
	DestroyStatic();

	BaseClass::OnDestroy();
}

// $Barnyard: FUNCTION 006160c0
void ACollisionManager::OnActivate()
{
	field403_0x1bc = 0.0f;

	BaseClass::OnActivate();
}

void ACollisionManager::OnDeactivate()
{
	field403_0x1bc = 0.0f;

	BaseClass::OnDeactivate();
}

// $Barnyard: FUNCTION 006161b0
AObjectHashMain* ACollisionManager::CreateObjectHashMain( HashType a_eHashType )
{
	TINT iNumCellsY;
	TINT iNumCellsX;
	TFLOAT fMaxY;
	TFLOAT fMaxX;
	TFLOAT fMinY;
	TFLOAT fMinX;

	if ( a_eHashType == HashType_UNK4 )
	{
		AObjectHashMain::CreateSingleton( 6500 );

		iNumCellsY = 24;
		iNumCellsX = 24;
		fMaxY = 1950.0f;
		fMaxX = 1150.0f;
		fMinY = -1000.0f;
		fMinX = -1250.0f;
	}
	else
	{
		AObjectHashMain::CreateSingleton( 6500 );

		iNumCellsY = 18;
		iNumCellsX = 18;
		fMaxY = 200.0f;
		fMaxX = 400.0f;
		fMinY = -200.0f;
		fMinX = -400.0f;
	}

	AObjectHashMain::GetSingleton()->Initialise( fMinX, fMinY, fMaxX, fMaxY, iNumCellsX, iNumCellsY );

	return AObjectHashMain::GetSingleton();
}

// $Barnyard: FUNCTION 0065a2a0
void ACollisionManager::CreateStatic()
{
	TASSERT( s_pDynVector == TNULL );

	s_pDynVector = new T2DynamicVector<int>( GetGlobalAllocator(), 64, 64 );

	// Reset the first array
	TASSERT( s_iSomeCount1 < MAX_SOME_COUNT1 );
	for ( TINT i = 0; s_iSomeCount1; i++ )
		s_aSomeArray1[ i ] = 0;

	s_iSomeCount1 = 0;

	// Reset the second array
	TASSERT( s_iSomeCount2 < MAX_SOME_COUNT1 );
	for ( TINT i = 0; s_iSomeCount2; i++ )
		s_aSomeArray2[ i ] = 0;

	s_iSomeCount2 = 0;

	// Create objects that contain some coordinates
	s_iSomeCount2   = 0;
	s_iUnk5         = 0;
	s_TotalNumNodes = MAX_NODES;
	s_pNodes        = new Node[ MAX_NODES ];

	s_bFlag1 = TFALSE;
	s_bFlag2 = TFALSE;
	s_bFlag3 = TFALSE;
	s_iUnk4  = 1;
}

// $Barnyard: FUNCTION 0065d900
void ACollisionManager::DestroyStatic()
{
	s_iUnk5 = 0;

	TASSERT( s_pNodes != TNULL );
	delete[] s_pNodes;
	s_pNodes = TNULL;

	// Reset the first array
	TASSERT( s_iSomeCount1 < MAX_SOME_COUNT1 );
	for ( TINT i = 0; s_iSomeCount1; i++ )
		s_aSomeArray1[ i ] = 0;

	s_iSomeCount1 = 0;

	// Reset the second array
	TASSERT( s_iSomeCount2 < MAX_SOME_COUNT1 );
	for ( TINT i = 0; s_iSomeCount2; i++ )
		s_aSomeArray2[ i ] = 0;

	s_iSomeCount2 = 0;

	TASSERT( s_pDynVector != TNULL );
	delete s_pDynVector;
}
