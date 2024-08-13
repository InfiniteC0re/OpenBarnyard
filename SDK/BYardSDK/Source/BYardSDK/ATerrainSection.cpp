#include "pch.h"
#include "ATerrainSection.h"
#include "ATerrainInterface.h"
#include "AAssetStreaming.h"

void ATerrainSection::LoadCollision()
{
}

void ATerrainSection::LoadModels( ATerrainLODType a_eLODType )
{
}

void ATerrainSection::LoadMatlib( ATerrainLODType a_eLODType )
{
}

void ATerrainSection::UnloadMatlib( ATerrainLODType a_eLODType )
{
}

void ATerrainSection::DestroyLOD( ATerrainLODType a_eLODType )
{
}

void ATerrainSection::RemoveFromStreamingQueue()
{
}

TBOOL ATerrainSection::IsMatLibLoaded( ATerrainLODType a_eLODType ) const
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_eLODType == ATerrainLODType_High )
	{
		return m_pMatLibHigh != TNULL;
	}
	else
	{
		return m_pMatLibLow != TNULL;
	}
}

void ATerrainSection::SetLODQueued( ATerrainLODType a_eLODType, TBOOL a_bQueued )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_bQueued )
	{
		m_eFlags |= ( 16 << a_eLODType );
	}
	else
	{
		m_eFlags &= ~( 16 << a_eLODType );
	}
}

void ATerrainSection::SetLODLoaded( ATerrainLODType a_eLODType, TBOOL a_bLoaded )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_bLoaded )
	{
		m_eFlags |= ( 1 << ( a_eLODType ) );
	}
	else
	{
		m_eFlags &= ~( 1 << ( a_eLODType ) );
	}
}

void ATerrainSection::SetLODLoading( ATerrainLODType a_eLODType, TBOOL a_bLoading )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_bLoading )
	{
		m_eFlags |= ( 1 << ( a_eLODType + ATerrainLODType_NUMOF ) );
	}
	else
	{
		m_eFlags &= ~( 1 << ( a_eLODType + ATerrainLODType_NUMOF ) );
	}
}

void ATerrainSection::SetLODEmpty( ATerrainLODType a_eLODType, TBOOL a_bEmpty )
{
	TASSERT( a_eLODType == ATerrainLODType_High || a_eLODType == ATerrainLODType_Low );

	if ( a_bEmpty )
	{
		m_eFlags |= ( 64 << a_eLODType );
	}
	else
	{
		m_eFlags &= ~( 64 << a_eLODType );
	}
}

ATerrainSection::ModelNode::~ModelNode()
{
}
