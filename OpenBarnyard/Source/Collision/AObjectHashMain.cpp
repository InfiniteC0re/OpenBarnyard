#include "pch.h"
#include "AObjectHashMain.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AObjectHashMain::AObjectHashMain( TINT a_iNumNodes )
    : AObjectHash( a_iNumNodes )
{
}

// $Barnyard: FUNCTION 006165e0
// $Barnyard: FUNCTION 00616600
AObjectHashMain::~AObjectHashMain()
{
}

// $Barnyard: FUNCTION 0061f040
void AObjectHashMain::Initialise( TFLOAT a_fMinX, TFLOAT a_fMinY, TFLOAT a_fMaxX, TFLOAT a_fMaxY, TINT a_iNumCellsX, TINT a_iNumCellsY )
{
	m_fNumCellsX               = a_iNumCellsX;
	m_fNumCellsY               = a_iNumCellsY;
	m_fTotalX                  = a_fMaxX - a_fMinX;
	m_fTotalY                  = a_fMaxY - a_fMinY;
	m_fNegativeMinX            = -a_fMinX;
	m_fNegativeMinY            = -a_fMinY;
	m_fMaxXOverNumCells        = a_fMaxX / a_iNumCellsX;
	m_fMaxYOverNumCells        = a_fMaxY / a_iNumCellsY;
	m_fMaxXOverNumCellsOverOne = 1.0f / ( a_fMaxX / a_iNumCellsX );
	m_fMaxYOverNumCellsOverOne = 1.0f / ( a_fMaxY / a_iNumCellsY );
}
