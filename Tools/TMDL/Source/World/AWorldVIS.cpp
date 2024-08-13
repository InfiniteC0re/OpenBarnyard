#include "pch.h"
#include "AWorldVIS.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AWorldVis::AWorldVis()
{
	m_pWorld		 = TNULL;
	m_pModels		 = TNULL;
	m_iSomeNum1		 = 0;
	m_iSomeNum2		 = 0;
	m_fnSomeCallback = TNULL;
}

AWorldVis::~AWorldVis()
{
	Reset();

	if ( m_pModels )
	{
		delete[] m_pModels;
		m_pModels = TNULL;
	}
}

void AWorldVis::Create( World* a_pWorld )
{
	m_pWorld  = a_pWorld;
	m_pModels = new Model[ a_pWorld->m_iNumCells ];
	Reset();
}

void AWorldVis::Reset()
{
	for ( TINT i = 0; i < m_iSomeNum1; i++ )
	{
		m_pWorld->m_ppCells[ m_pModels[ i ].iIndex ]->m_Unknown = 0;
	}

	m_iSomeNum1 = 0;
	m_iSomeNum2 = 0;
}
