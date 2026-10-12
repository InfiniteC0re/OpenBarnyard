#include "pch.h"
#include "AUnitComponent.h"
#include "AUnit.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_INIT_NORUNTIME( AUnitComponent );

// DAT_0077e97c
AFunctionTable AUnitComponent::ms_oFunctionTable( &TGetClass( AUnitComponent ), TNULL );

// FSM fallback: resolves the component a native is called on, either directly or as the
// component of the unit that has the class of the table.
// $Barnyard: FUNCTION 0041b1e0
TObject* __stdcall AUnitComponentFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	if ( TObject* pComponent = a_pCaller->GetComponent() )
		return pComponent;

	if ( a_pCaller && a_pCaller->GetObject() )
		return TSTATICCAST( AUnit, a_pCaller->GetObject() )->FindComponentExact( a_pTable->GetFunctionClass() );

	return TNULL;
}

// $Barnyard: FUNCTION 0041b3f0
void AUnitComponent::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AUnitComponentFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer0ArgsRT<AUnit*, AUnitComponent>::Create( "GetOwner", &AUnitComponent::GetOwner ) );
}

// $Barnyard: FUNCTION 0041b220
void AUnitComponent::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0041b340
AUnitComponent::AUnitComponent()
    : m_bAIEnabled( TTRUE )
{
}

// $Barnyard: FUNCTION 0041b420
// $Barnyard: FUNCTION 0041b380
AUnitComponent::~AUnitComponent()
{
}

// $Barnyard: FUNCTION 0041b370
AFunctionTable* AUnitComponent::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0041b440
void AUnitComponent::SetOwner( AUnit* a_pUnit )
{
	if ( a_pUnit )
		m_oOwner = a_pUnit->GetSelfProxy();
	else
		m_oOwner = TNULL;
}

// $Barnyard: FUNCTION 0041b230
AUnit* AUnitComponent::GetOwner()
{
	if ( !m_oOwner.Get() )
		return TNULL;

	return m_oOwner->GetUnit();
}
