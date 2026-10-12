#include "pch.h"
#include "ADriveFunction.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0054dec0, retail registers RETURNS_ZERO as both create functions and an alignment of 1
TDEFINE_CLASS_NORUNTIME( ADriveFunction );

// $Barnyard: FUNCTION 0054ded0
ADriveFunction::ADriveFunction()
{
}

// $Barnyard: FUNCTION 0054def0
// $Barnyard: FUNCTION 0054df10
ADriveFunction::~ADriveFunction()
{
}

// $Barnyard: FUNCTION 0054da80
ADriveFunction* ADriveFunction::CreateFromProperties( const PBProperties* a_pProperties, TINT* a_pDataSize )
{
	ADriveFunction* pFunction = TNULL;
	TPString8       strType;

	if ( a_pProperties->GetOptionalPropertyValue( strType, "type" ) )
	{
		const TClass* pClass = TClass::Find( strType.GetString(), &TGetClass( ADriveFunction ) );

		if ( pClass )
		{
			pFunction = TSTATICCAST( ADriveFunction, pClass->CreateObject() );

			if ( !pFunction->Create( a_pProperties, a_pDataSize ) )
			{
				delete pFunction;
				pFunction = TNULL;
			}
		}
	}

	return pFunction;
}
