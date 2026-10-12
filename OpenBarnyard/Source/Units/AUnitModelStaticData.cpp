#include "pch.h"
#include "AUnitModelStaticData.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0041b840, CreateObject 0041bac0, CreateObjectInPlace 0041bb20
TDEFINE_CLASS( AUnitModelStaticData );

// $Barnyard: FUNCTION 0041b9d0
AUnitModelStaticData::AUnitModelStaticData()
{
}

// $Barnyard: FUNCTION 0041bb60
// $Barnyard: FUNCTION 0041ba20
AUnitModelStaticData::~AUnitModelStaticData()
{
	m_oModelNames.SetCapacity( 0 );
}

// $Barnyard: FUNCTION 0041bb80
TBOOL AUnitModelStaticData::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pModels = a_pProperties->GetProperty( "Models" );

	if ( !pModels )
		pModels = a_pProperties->GetProperty( "Model" );

	if ( pModels )
	{
		if ( pModels->GetType() == PBPropertyValue::Type::Array )
		{
			const PBPropertyValueArray* pArray     = pModels->GetArray();
			TINT                        iNumModels = pArray->GetSize();

			if ( iNumModels < 1 )
				return TFALSE;

			if ( m_oModelNames.iCapacity < iNumModels )
				m_oModelNames.SetCapacity( iNumModels );

			m_oModelNames.iSize = iNumModels;

			for ( TINT i = 0; i < iNumModels; i++ )
				m_oModelNames.pData[ i ] = pArray->GetValue( i )->GetTPString8();
		}
		else if ( pModels->GetType() == PBPropertyValue::Type::String )
		{
			if ( m_oModelNames.iCapacity < 1 )
				m_oModelNames.SetCapacity( 1 );

			m_oModelNames.iSize      = 1;
			m_oModelNames.pData[ 0 ] = pModels->GetTPString8();
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0041b880
void AUnitModelStaticData::ModelNameArray::SetCapacity( TINT a_iCapacity )
{
	if ( a_iCapacity != 0 )
	{
		TPString8* pNewData = new TPString8[ a_iCapacity ];
		TINT       iNumCopy = ( a_iCapacity < iSize ) ? a_iCapacity : iSize;

		for ( TINT i = 0; i < iNumCopy; i++ )
			pNewData[ i ] = pData[ i ];

		iCapacity = a_iCapacity;
		delete[] pData;
		pData = pNewData;
	}
	else
	{
		delete[] pData;
		pData     = TNULL;
		iCapacity = 0;
		iSize     = 0;
	}
}
