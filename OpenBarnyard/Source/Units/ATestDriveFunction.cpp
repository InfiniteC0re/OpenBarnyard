#include "pch.h"
#include "ATestDriveFunction.h"
#include "ARandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054ee70
// NOTE: GetClass 0054ee70, CreateObject 0054ee90, CreateObjectInPlace 0054ef10 (the constructor is inlined in both)
TDEFINE_CLASS( ATestDriveFunction );

// 0x0077421c
static const TCHAR* s_apActionNames[ ATestDriveFunction::ACTION_NUMOF ] = {
	"random",
	"fixed",
};

// $Barnyard: FUNCTION 0054ee80
// $Barnyard: FUNCTION 0054ef90
ATestDriveFunction::~ATestDriveFunction()
{
}

// $Barnyard: FUNCTION 0054f040
TBOOL ATestDriveFunction::Create( const PBProperties* a_pProperties, TINT* a_pDataSize )
{
	TPString8 strAction;

	if ( a_pProperties->GetOptionalPropertyValue( strAction, "action" ) )
	{
		for ( TINT i = 0; i < ACTION_NUMOF; i++ )
		{
			if ( strAction.GetString8().CompareNoCase( s_apActionNames[ i ] ) == 0 )
			{
				m_eAction = i;
				break;
			}
		}
	}

	a_pProperties->GetOptionalFloatOrIntValue( m_fValue, "value" );
	a_pProperties->GetOptionalFloatOrIntValue( m_fNullChance, "nullchance" );

	return TTRUE;
}

// $Barnyard: FUNCTION 0054efb0
void ATestDriveFunction::Evaluate( AUnit* a_pUnit, AInteractionActivityData* a_pData, TFLOAT* a_pValue )
{
	TRandom& rRandom = ARandom::GetSingleton()->m_oRandom;

	// NOTE: TRandom::GetFloat is inlined in the first test
	if ( rRandom.GetFloat() >= m_fNullChance )
	{
		if ( m_eAction == ACTION_RANDOM )
			*a_pValue = rRandom.GetFloat() * m_fValue;
		else if ( m_eAction == ACTION_FIXED )
			*a_pValue = m_fValue;
	}
	else
	{
		*a_pValue = 0.0f;
	}
}
