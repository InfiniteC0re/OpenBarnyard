#include "pch.h"
#include "AAnimalAttributes.h"
#include "AGeneralUnitComponentManager.h"
#include "ARandom.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"
#include "Helpers/AGameTimeManager.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject and CreateObjectInPlace of the TClass
// $Barnyard: FUNCTION 0058b1e0
// $Barnyard: FUNCTION 0058b240
TDEFINE_CLASS_INIT( AAnimalAttributes );

AFunctionTable AAnimalAttributes::ms_oFunctionTable( &TGetClass( AAnimalAttributes ), &AUnitComponent::ms_oFunctionTable );

// 0x007750e8
static const TCHAR* s_apAttributeNames[ AAnimalAttributes::ATTRIBUTE_NUMOF ] = {
	"happiness",
	"suggestion",
	"command",
	"schedule",
};

// 0x007284d8, the values DebugToggle resets the attributes to
static const TFLOAT s_afDefaultAttributes[ AAnimalAttributes::ATTRIBUTE_NUMOF ] = {
	1.0f,
	0.0f,
	0.0f,
	0.0f,
};

// $Barnyard: FUNCTION 0058b2f0
void AAnimalAttributes::InitialiseClass()
{
	ms_oFunctionTable.Register( TNULL );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, AAnimalAttributes>::Create( "GetHappiness", &AAnimalAttributes::GetHappiness ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimalAttributes, TFLOAT>::Create( "SetHappiness", &AAnimalAttributes::SetHappiness ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimalAttributes, TFLOAT>::Create( "ModifyHappiness", &AAnimalAttributes::ModifyHappiness ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, AAnimalAttributes>::Create( "GetSuggestion", &AAnimalAttributes::GetSuggestion ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimalAttributes, TFLOAT>::Create( "SetSuggestion", &AAnimalAttributes::SetSuggestion ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimalAttributes, TFLOAT>::Create( "ModifySuggestion", &AAnimalAttributes::ModifySuggestion ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, AAnimalAttributes>::Create( "GetCommand", &AAnimalAttributes::GetCommand ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimalAttributes, TFLOAT>::Create( "SetCommand", &AAnimalAttributes::SetCommand ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimalAttributes, TFLOAT>::Create( "ModifyCommand", &AAnimalAttributes::ModifyCommand ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<TFLOAT, AAnimalAttributes>::Create( "GetSchedule", &AAnimalAttributes::GetSchedule ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimalAttributes, TFLOAT>::Create( "SetSchedule", &AAnimalAttributes::SetSchedule ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AAnimalAttributes, TFLOAT>::Create( "ModifySchedule", &AAnimalAttributes::ModifySchedule ) );
	ms_oFunctionTable.AddFunction( AMethodPointer0Args<AAnimalAttributes>::Create( "DebugToggle", &AAnimalAttributes::DebugToggle ) );
}

// $Barnyard: FUNCTION 0058aa80
void AAnimalAttributes::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0058ad40
AAnimalAttributes::AAnimalAttributes()
{
	m_afAttributes[ ATTRIBUTE_HAPPINESS ]  = 1.0f;
	m_afAttributes[ ATTRIBUTE_SUGGESTION ] = 0.0f;
	m_afAttributes[ ATTRIBUTE_COMMAND ]    = 0.0f;
	m_afAttributes[ ATTRIBUTE_SCHEDULE ]   = 0.0f;

	// NOTE: TRandom::GetFloat is inlined here
	m_afAttributes[ ATTRIBUTE_HAPPINESS ] = ARandom::GetSingleton()->m_oRandom.GetFloat();
	TMath::Clip( m_afAttributes[ ATTRIBUTE_HAPPINESS ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058ae20
// $Barnyard: FUNCTION 0058ae00
AAnimalAttributes::~AAnimalAttributes()
{
}

// $Barnyard: FUNCTION 0058adf0
AFunctionTable* AAnimalAttributes::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// NOTE: folded with AUnitProfileComponent::GetManager (005d2300)
AUnitComponentManager* AAnimalAttributes::GetManager()
{
	return AGeneralUnitComponentManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 0058ae30
void AAnimalAttributes::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( AGameTimeManager::GetSingleton()->IsDaytime() )
		m_afAttributes[ ATTRIBUTE_SUGGESTION ] -= a_fDeltaTime * 0.008f;
	else
		m_afAttributes[ ATTRIBUTE_SUGGESTION ] -= a_fDeltaTime * 0.02f;

	TMath::Clip( m_afAttributes[ ATTRIBUTE_SUGGESTION ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058aad0
TFLOAT AAnimalAttributes::GetAttribute( ATTRIBUTE a_eAttribute )
{
	return m_afAttributes[ a_eAttribute ];
}

// $Barnyard: FUNCTION 0058aae0
void AAnimalAttributes::ChangeAttribute( ATTRIBUTE a_eAttribute, TFLOAT a_fValue )
{
	m_afAttributes[ a_eAttribute ] += a_fValue;
	TMath::Clip( m_afAttributes[ a_eAttribute ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058ab10
void AAnimalAttributes::SetAttribute( ATTRIBUTE a_eAttribute, TFLOAT a_fValue )
{
	m_afAttributes[ a_eAttribute ] = a_fValue;
	TMath::Clip( m_afAttributes[ a_eAttribute ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058ac00
TFLOAT AAnimalAttributes::GetHappiness()
{
	return m_afAttributes[ ATTRIBUTE_HAPPINESS ];
}

// $Barnyard: FUNCTION 0058ac10
void AAnimalAttributes::SetHappiness( TFLOAT a_fValue )
{
	m_afAttributes[ ATTRIBUTE_HAPPINESS ] = a_fValue;
	TMath::Clip( m_afAttributes[ ATTRIBUTE_HAPPINESS ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058ac30
void AAnimalAttributes::ModifyHappiness( TFLOAT a_fValue )
{
	m_afAttributes[ ATTRIBUTE_HAPPINESS ] += a_fValue;
	TMath::Clip( m_afAttributes[ ATTRIBUTE_HAPPINESS ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058ac50
TFLOAT AAnimalAttributes::GetSuggestion()
{
	return m_afAttributes[ ATTRIBUTE_SUGGESTION ];
}

// $Barnyard: FUNCTION 0058ac60
void AAnimalAttributes::SetSuggestion( TFLOAT a_fValue )
{
	m_afAttributes[ ATTRIBUTE_SUGGESTION ] = a_fValue;
	TMath::Clip( m_afAttributes[ ATTRIBUTE_SUGGESTION ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058ac80
void AAnimalAttributes::ModifySuggestion( TFLOAT a_fValue )
{
	m_afAttributes[ ATTRIBUTE_SUGGESTION ] += a_fValue;
	TMath::Clip( m_afAttributes[ ATTRIBUTE_SUGGESTION ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058aca0
TFLOAT AAnimalAttributes::GetCommand()
{
	return m_afAttributes[ ATTRIBUTE_COMMAND ];
}

// $Barnyard: FUNCTION 0058acb0
void AAnimalAttributes::SetCommand( TFLOAT a_fValue )
{
	m_afAttributes[ ATTRIBUTE_COMMAND ] = a_fValue;
	TMath::Clip( m_afAttributes[ ATTRIBUTE_COMMAND ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058acd0
void AAnimalAttributes::ModifyCommand( TFLOAT a_fValue )
{
	m_afAttributes[ ATTRIBUTE_COMMAND ] += a_fValue;
	TMath::Clip( m_afAttributes[ ATTRIBUTE_COMMAND ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058acf0
TFLOAT AAnimalAttributes::GetSchedule()
{
	return m_afAttributes[ ATTRIBUTE_SCHEDULE ];
}

// $Barnyard: FUNCTION 0058ad00
void AAnimalAttributes::SetSchedule( TFLOAT a_fValue )
{
	m_afAttributes[ ATTRIBUTE_SCHEDULE ] = a_fValue;
	TMath::Clip( m_afAttributes[ ATTRIBUTE_SCHEDULE ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058ad20
void AAnimalAttributes::ModifySchedule( TFLOAT a_fValue )
{
	m_afAttributes[ ATTRIBUTE_SCHEDULE ] += a_fValue;
	TMath::Clip( m_afAttributes[ ATTRIBUTE_SCHEDULE ], 0.0f, 1.0f );
}

// $Barnyard: FUNCTION 0058ab30
void AAnimalAttributes::DebugToggle()
{
	for ( TINT i = 0; i < ATTRIBUTE_NUMOF; i++ )
	{
		TFLOAT fValue = s_afDefaultAttributes[ i ];

		if ( i == ms_iDebugAttribute )
			fValue = 1.0f - fValue;

		m_afAttributes[ i ] = fValue;
		TMath::Clip( m_afAttributes[ i ], 0.0f, 1.0f );
	}

	// Skips the suggestion
	ms_iDebugAttribute++;

	if ( ms_iDebugAttribute == ATTRIBUTE_SUGGESTION )
		ms_iDebugAttribute = ATTRIBUTE_COMMAND;
	else if ( ms_iDebugAttribute >= ATTRIBUTE_NUMOF )
		ms_iDebugAttribute = ATTRIBUTE_HAPPINESS;
}

// $Barnyard: FUNCTION 0058aa90
AAnimalAttributes::ATTRIBUTE AAnimalAttributes::GetAttributeFromName( const TCHAR* a_szName )
{
	TINT i = 0;

	for ( ; i < ATTRIBUTE_NUMOF; i++ )
	{
		if ( TStringManager::String8CompareNoCase( a_szName, s_apAttributeNames[ i ] ) == 0 )
			break;
	}

	return i;
}
