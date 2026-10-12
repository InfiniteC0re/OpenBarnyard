#include "pch.h"
#include "AModelSwapComponentData.h"
#include "AModelSwapComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 0058e2f0 / CreateObjectInPlace 0058e350
// NOTE: GetClass 0058dcc0, CreateObject 0058e2f0, CreateObjectInPlace 0058e350
TDEFINE_CLASS( AModelSwapComponentData );

// $Barnyard: FUNCTION 0058e200
AModelSwapComponentData::AModelSwapComponentData()
    : m_iGrowSize( 0 )
    , m_iNumModels( 0 )
    , m_iNumAllocModels( 0 )
    , m_pModels( TNULL )
{
}

// $Barnyard: FUNCTION 0058e390
// $Barnyard: FUNCTION 0058e250
AModelSwapComponentData::~AModelSwapComponentData()
{
	if ( m_pModels )
		delete[] m_pModels;

	m_pModels         = TNULL;
	m_iNumAllocModels = 0;
	m_iNumModels      = 0;
}

// $Barnyard: FUNCTION 0058e3b0
TBOOL AModelSwapComponentData::Create( const PBProperties* a_pProperties )
{
	const PBPropertyValue* pModels = a_pProperties->GetProperty( "Models" );

	if ( pModels && pModels->GetType() == PBPropertyValue::Type::Array )
	{
		const PBPropertyValueArray* pArray     = pModels->GetArray();
		TINT                        iNumModels = pArray->GetSize();

		if ( iNumModels < 1 )
			return TFALSE;

		if ( m_iNumAllocModels < iNumModels )
			ResizeModels( iNumModels );

		m_iNumModels = iNumModels;

		for ( TINT i = 0; i < iNumModels; i++ )
			m_pModels[ i ] = pArray->GetValue( i )->GetTPString8();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0058e170
void AModelSwapComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AModelSwapComponent* pComponent = new AModelSwapComponent();
	pComponent->CreateModels( a_pUnit, this );

	a_pUnit->AddComponent( pComponent );
}

// NOTE: TArray<TPString8>::Resize (0041b880), see the declaration
void AModelSwapComponentData::ResizeModels( TINT a_iSize )
{
	if ( a_iSize == 0 )
	{
		if ( m_pModels )
			delete[] m_pModels;

		m_pModels         = TNULL;
		m_iNumAllocModels = 0;
		m_iNumModels      = 0;
		return;
	}

	TPString8* pNewModels = new TPString8[ a_iSize ];
	TINT       iNumToCopy = TMath::Min( m_iNumModels, a_iSize );

	for ( TINT i = 0; i < iNumToCopy; i++ )
		pNewModels[ i ] = m_pModels[ i ];

	m_iNumAllocModels = a_iSize;

	if ( m_pModels )
		delete[] m_pModels;

	m_pModels = pNewModels;
}
