#include "pch.h"
#include "AModelAttachComponentData.h"
#include "AModelAttachComponent.h"
#include "AUnit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0058d0e0, CreateObject 0058dad0 / CreateObjectInPlace 0058db30
// NOTE: GetClass 0058d0e0, CreateObject 0058dad0, CreateObjectInPlace 0058db30
TDEFINE_CLASS( AModelAttachComponentData );

// $Barnyard: FUNCTION 0058d990
AModelAttachComponentData::AModelAttachComponentData()
    : m_iGrowSize( 0 )
    , m_iNumModels( 0 )
    , m_iNumAllocModels( 0 )
    , m_pModels( TNULL )
{
}

// $Barnyard: FUNCTION 0058d9e0
// $Barnyard: FUNCTION 0058db70
AModelAttachComponentData::~AModelAttachComponentData()
{
	if ( m_pModels )
		delete[] m_pModels;

	m_pModels         = TNULL;
	m_iNumAllocModels = 0;
	m_iNumModels      = 0;
}

// $Barnyard: FUNCTION 0058db90
TBOOL AModelAttachComponentData::Create( const PBProperties* a_pProperties )
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

	return AUnitStaticData::Create( a_pProperties );
}

// $Barnyard: FUNCTION 0058d760
void AModelAttachComponentData::OnUnitCreate( AUnit* a_pUnit )
{
	AModelAttachComponent* pComponent = new AModelAttachComponent();
	pComponent->CreateModels( a_pUnit, this );

	a_pUnit->AddComponent( pComponent );
}

// NOTE: TArray<TPString8>::Resize (0041b880), see the declaration
void AModelAttachComponentData::ResizeModels( TINT a_iSize )
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
