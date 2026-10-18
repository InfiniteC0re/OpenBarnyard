#include "ToshiPCH.h"
#include "TGUIDeserialisable.h"
#include "TGUIManager.h"
#include "TGUITextureSection.h"
#include "TGUIStyle.h"
#include "TGUIPropertySpecSimple.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PPropertyName.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: abstract, CreateObject returns TNULL in Nicktoons
TDEFINE_CLASS_INIT_NORUNTIME( TGUIDeserialisable );

TPString8 TGUIDeserialisable::ms_strType;

// NOTE: Nicktoons creates the string with the TClass object (static initialiser 00386c18)
// $NicktoonsPS2: FUNCTION 00386c18
void TGUIDeserialisable::InitialiseClass()
{
	ms_strType = TPString8( "type" );
}

void TGUIDeserialisable::DeinitialiseClass()
{
	ms_strType = TPString8();
}

// $NicktoonsPS2: FUNCTION 00385610
// $JPOG: FUNCTION TGuiInterface.dll+00009c70
TGUIDeserialisable::TGUIDeserialisable()
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00008570
TGUIDeserialisable::TGUIDeserialisable( const TGUIDeserialisable& a_rcOther )
{
}

// $NicktoonsPS2: FUNCTION 00385648
// $JPOG: FUNCTION TGuiInterface.dll+00009c60
TGUIDeserialisable::~TGUIDeserialisable()
{
}

// $NicktoonsPS2: FUNCTION 00385958
// $JPOG: FUNCTION TGuiInterface.dll+00008c50
TBOOL TGUIDeserialisable::Deserialise( const PProperties* a_pProperties )
{
	PropertySet* pSet = GetPropertyInfoSet();

	for ( TINT i = 0; i < pSet->Size(); i++ )
	{
		const TGUIPropertySpec* pSpec = ( *pSet )[ i ];

		if ( pSpec->AllowsAutoInstall() != TTRUE )
			continue;

		const PPropertyValue* pValue = a_pProperties->GetProperty( pSpec->GetName() );

		if ( !pValue )
			continue;

		if ( pSpec->IsEnumerated() )
		{
			if ( !pValue->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
			{
				TGUIManager::Error( TString8( "InstallProperties: Property '" ) + pSpec->GetName().GetString() + "' of incorrect type " + PPropertyValue::ToString( pSpec->GetType() ) );
				return TFALSE;
			}

			TINT iValue = pSpec->GetEnumValue( pValue->GetTPString8() );

			if ( iValue < 0 )
			{
				TGUIManager::Error( TString8( "InstallProperties: Property '" ) + pSpec->GetName().GetString() + "' has unknown enumeration value " + pValue->GetTPString8().GetString() );
				return TFALSE;
			}

			SetProperty( pSpec->GetName(), PPropertyValue( iValue ) );
		}
		else
		{
			if ( !pValue->CanBeType( pSpec->GetType() ) )
			{
				TGUIManager::Error( TString8( "InstallProperties: Property '" ) + pSpec->GetName().GetString() + "' of incorrect type; expected " + PPropertyValue::ToString( pSpec->GetType() ) );
				return TFALSE;
			}

			SetProperty( pSpec->GetName(), *pValue );
		}
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00385d60
// $JPOG: FUNCTION TGuiInterface.dll+00009010
TBOOL TGUIDeserialisable::Serialise( PProperties* a_pProperties )
{
	a_pProperties->PutProperty( ms_strType, PPropertyValue( TPString8( GetClass()->GetName() ) ) );

	PropertySet* pOrderedSet = GetPropertyInfoOrderedSet();

	for ( TINT i = 0; i < pOrderedSet->Size(); i++ )
	{
		const TGUIPropertySpec* pSpec = ( *pOrderedSet )[ i ];

		if ( !pSpec->AllowsAutoInstall() )
			continue;

		PPropertyValue oValue = pSpec->get( this );

		if ( pSpec->IsEnumerated() )
		{
			a_pProperties->PutProperty( pSpec->GetName(), PPropertyValue( pSpec->GetEnumName( oValue.GetInteger() ) ) );
		}
		else if ( pSpec->GetType() == &TGetClass( TGUITextureSection ) )
		{
			// Texture sections are written as their names
			TGUITextureSection* pTextureSection = TSTATICCAST( TGUITextureSection, oValue.GetTObject() );
			a_pProperties->PutProperty( pSpec->GetName(), PPropertyValue( pTextureSection->GetName() ) );
		}
		else if ( pSpec->GetType() == &TGetClass( TGUIStyle ) )
		{
			// Styles are written as their names
			TGUIStyle* pStyle = TSTATICCAST( TGUIStyle, oValue.GetTObject() );
			a_pProperties->PutProperty( pSpec->GetName(), PPropertyValue( pStyle->GetName() ) );
		}
		else if ( oValue.IsDefined() && !oValue.IsEmpty() )
		{
			a_pProperties->PutProperty( pSpec->GetName(), oValue );
		}
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 003866c8
// $JPOG: FUNCTION TGuiInterface.dll+00009bf0
TINT TGUIDeserialisable::GetPropertyCount() const
{
	return GetPropertyInfoSet()->Size();
}

// Binary search of the ordered set, returns the first spec with the name
// $NicktoonsPS2: FUNCTION 00385710
// $JPOG: FUNCTION TGuiInterface.dll+00009670
const TGUIPropertySpec* TGUIDeserialisable::FindPropertyInfo( const TPString8& a_rcName ) const
{
	TGUIPropertySpecSimple oKey( a_rcName );

	PropertySet*                 pOrderedSet = GetPropertyInfoOrderedSet();
	TGUIPropertySpec::Comparator oComparator;

	TINT iLow  = 0;
	TINT iHigh = pOrderedSet->Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = oComparator( ( *pOrderedSet )[ iMiddle ], &oKey );

		if ( iCompare > 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iCompare < 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			TINT iIndex = iMiddle - 1;

			while ( iIndex >= iLow && oComparator( ( *pOrderedSet )[ iIndex ], &oKey ) == 0 )
				iIndex--;

			return ( *pOrderedSet )[ iIndex + 1 ];
		}
	}

	return TNULL;
}

// $NicktoonsPS2: FUNCTION 003858d0
// $JPOG: FUNCTION TGuiInterface.dll+00009bd0
const TGUIPropertySpec* TGUIDeserialisable::GetPropertyInfo( TINT a_iIndex ) const
{
	return ( *GetPropertyInfoSet() )[ a_iIndex ];
}

// $NicktoonsPS2: FUNCTION 00385910
// $JPOG: FUNCTION TGuiInterface.dll+000096e0
TBOOL TGUIDeserialisable::ValidatePropertyInfo( const TGUIPropertySpec* a_pSpec ) const
{
	if ( !a_pSpec )
		return TFALSE;

	return GetClass()->IsA( a_pSpec->GetSourceClass() );
}

// $JPOG: FUNCTION TGuiInterface.dll+00009500
TBOOL TGUIDeserialisable::SetProperty( const TGUIPropertySpec* a_pSpec, const PPropertyValue& a_rcValue )
{
	if ( !ValidatePropertyInfo( a_pSpec ) )
		return TFALSE;

	a_pSpec->set( this, a_rcValue );
	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00385690
// $JPOG: FUNCTION TGuiInterface.dll+00009540
TBOOL TGUIDeserialisable::SetProperty( const TPString8& a_rcName, const PPropertyValue& a_rcValue )
{
	const TGUIPropertySpec* pSpec = FindPropertyInfo( a_rcName );

	if ( !pSpec )
		return TFALSE;

	pSpec->set( this, a_rcValue );
	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00009c40
TBOOL TGUIDeserialisable::SetProperty( const PPropertyName& a_rcName, const PPropertyValue& a_rcValue )
{
	return SetProperty( a_rcName.GetName(), a_rcValue );
}

// $JPOG: FUNCTION TGuiInterface.dll+00009580
PPropertyValue TGUIDeserialisable::GetProperty( const TGUIPropertySpec* a_pSpec, TBOOL& a_rbFound ) const
{
	if ( ValidatePropertyInfo( a_pSpec ) )
	{
		a_rbFound = TTRUE;
		return a_pSpec->get( this );
	}

	a_rbFound = TFALSE;
	return PPropertyValue();
}

// $JPOG: FUNCTION TGuiInterface.dll+000095e0
PPropertyValue TGUIDeserialisable::GetProperty( const TPString8& a_rcName, TBOOL& a_rbFound ) const
{
	const TGUIPropertySpec* pSpec = FindPropertyInfo( a_rcName );

	if ( pSpec )
	{
		a_rbFound = TTRUE;
		return pSpec->get( this );
	}

	a_rbFound = TFALSE;
	return PPropertyValue();
}

// $JPOG: FUNCTION TGuiInterface.dll+00009c20
PPropertyValue TGUIDeserialisable::GetProperty( const PPropertyName& a_rcName, TBOOL& a_rbFound ) const
{
	return GetProperty( a_rcName.GetName(), a_rbFound );
}

// $JPOG: FUNCTION TGuiInterface.dll+00009630
TBOOL TGUIDeserialisable::ValidateProperty( const TPString8& a_rcName, const PPropertyValue& a_rcValue ) const
{
	const TGUIPropertySpec* pSpec = FindPropertyInfo( a_rcName );
	return pSpec && pSpec->validate( this, a_rcValue );
}

// $JPOG: FUNCTION TGuiInterface.dll+00009c00
TBOOL TGUIDeserialisable::ValidateProperty( const PPropertyName& a_rcName, const PPropertyValue& a_rcValue ) const
{
	return ValidateProperty( a_rcName.GetName(), a_rcValue );
}

// $NicktoonsPS2: FUNCTION 00386350
// $JPOG: FUNCTION TGuiInterface.dll+00009820
const PPropertyValue* TGUIDeserialisable::GetRequiredProperty( const PProperties* a_pProperties, const PPropertyName& a_rcName, const TClass* a_pType ) const
{
	const PPropertyValue* pValue = a_pProperties->GetProperty( a_rcName );

	if ( !pValue )
	{
		TGUIManager::Error( GetClass()->GetName(), TString8( "Required property '" ) + a_rcName + "' not specified" );
		return TNULL;
	}

	if ( !pValue->CanBeType( a_pType ) )
	{
		TGUIManager::Error( GetClass()->GetName(), TString8( "Incorrect type of '" ) + a_rcName + "' property" );
		return TNULL;
	}

	return pValue;
}

// $NicktoonsPS2: FUNCTION 003862e0
// $JPOG: FUNCTION TGuiInterface.dll+00009720
const PPropertyValue* TGUIDeserialisable::GetRequiredProperty( const PProperties* a_pProperties, const TPString8& a_rcName, const TClass* a_pType ) const
{
	return GetRequiredProperty( a_pProperties, PPropertyName( a_rcName ), a_pType );
}

// $JPOG: FUNCTION TGuiInterface.dll+000097a0
const PPropertyValue* TGUIDeserialisable::GetRequiredProperty( const PProperties* a_pProperties, const TPString8& a_rcName, const TPString8& a_rcSubName, const TClass* a_pType ) const
{
	return GetRequiredProperty( a_pProperties, PPropertyName( a_rcName, a_rcSubName ), a_pType );
}

// $NicktoonsPS2: FUNCTION 003865b0
// $JPOG: FUNCTION TGuiInterface.dll+00009a90
const PPropertyValue* TGUIDeserialisable::GetOptionalProperty( const PProperties* a_pProperties, const PPropertyName& a_rcName, const TClass* a_pType ) const
{
	const PPropertyValue* pValue = a_pProperties->GetProperty( a_rcName );

	if ( !pValue )
		return TNULL;

	if ( !pValue->CanBeType( a_pType ) )
	{
		TGUIManager::Error( GetClass()->GetName(), TString8( "Incorrect type of '" ) + a_rcName + "' property" );
		return TNULL;
	}

	return pValue;
}

// $NicktoonsPS2: FUNCTION 003864d0
// $JPOG: FUNCTION TGuiInterface.dll+00009990
const PPropertyValue* TGUIDeserialisable::GetOptionalProperty( const PProperties* a_pProperties, const TPString8& a_rcName, const TClass* a_pType ) const
{
	return GetOptionalProperty( a_pProperties, PPropertyName( a_rcName ), a_pType );
}

// $NicktoonsPS2: FUNCTION 00386540
// $JPOG: FUNCTION TGuiInterface.dll+00009a10
const PPropertyValue* TGUIDeserialisable::GetOptionalProperty( const PProperties* a_pProperties, const TPString8& a_rcName, const TPString8& a_rcSubName, const TClass* a_pType ) const
{
	return GetOptionalProperty( a_pProperties, PPropertyName( a_rcName, a_rcSubName ), a_pType );
}

// NOTE: JPOG takes the interface and calls SetInterface on the new object
// $NicktoonsPS2: FUNCTION 00386038
// $JPOG: FUNCTION TGuiInterface.dll+00009290
TGUIDeserialisable* TGUIDeserialisable::Create( const PProperties* a_pProperties )
{
	static const TClass* s_pDeserialisableClass = TClass::Find( "TGUIDeserialisable" );

	const PPropertyValue* pType = a_pProperties->GetProperty( ms_strType );

	if ( !pType )
	{
		TGUIManager::Error( "Deserialise: Object has no 'type' property specified" );
		return TNULL;
	}

	if ( !pType->IsType( PPropertyValue::TYPE_TPSTRING8 ) )
	{
		TGUIManager::Error( "Deserialise: Incorrect type for 'type' property" );
		return TNULL;
	}

	TPString8     strType = pType->GetTPString8();
	const TClass* pClass  = TClass::Find( strType.GetString() );

	if ( !pClass )
	{
		TGUIManager::Error( TString8( "Deserialise: Object has unknown type " ) + strType.GetString() );
		return TNULL;
	}

	if ( !pClass->IsA( s_pDeserialisableClass ) )
	{
		TGUIManager::Error( "Deserialise: Type specified for object is not allowed" );
		return TNULL;
	}

	TObject* pObject = pClass->CreateObject();

	if ( !pObject )
	{
		TGUIManager::Error( "Deserialise: Could not create object" );
		return TNULL;
	}

	return TSTATICCAST( TGUIDeserialisable, pObject );
}

// Clones the specs into the set (declaration order) and the ordered set (sorted by name, equal names keep their order)
// $NicktoonsPS2: FUNCTION 003866f0
void TGUIDeserialisable::CreatePropertySets( PropertySet*& a_rpSet, PropertySet*& a_rpOrderedSet, const PropertySet& a_rcSpecs )
{
	a_rpSet        = new PropertySet( 4, 0 );
	a_rpOrderedSet = new PropertySet( 4, 0 );

	for ( TINT i = 0; i < a_rcSpecs.Size(); i++ )
		AddPropertySpec( a_rpSet, a_rpOrderedSet, a_rcSpecs[ i ]->Clone() );
}

void TGUIDeserialisable::AddPropertySpec( PropertySet* a_pSet, PropertySet* a_pOrderedSet, TGUIPropertySpec* a_pSpec )
{
	a_pSet->Push( a_pSpec );

	// Insert after the specs with the same name
	TGUIPropertySpec::Comparator oComparator;

	TINT iLow  = 0;
	TINT iHigh = a_pOrderedSet->Size() - 1;

	while ( iLow <= iHigh )
	{
		TINT iMiddle  = ( iLow + iHigh ) / 2;
		TINT iCompare = oComparator( ( *a_pOrderedSet )[ iMiddle ], a_pSpec );

		if ( iCompare > 0 )
		{
			iHigh = iMiddle - 1;
		}
		else if ( iCompare < 0 )
		{
			iLow = iMiddle + 1;
		}
		else
		{
			iLow = iMiddle + 1;

			while ( iLow <= iHigh && oComparator( ( *a_pOrderedSet )[ iLow ], a_pSpec ) == 0 )
				iLow++;

			break;
		}
	}

	a_pOrderedSet->InsertBefore( PropertySet::Iterator( iLow, *a_pOrderedSet ), a_pSpec );
}

// NOTE: the specs are owned by the set (the ordered set has the same pointers)
void TGUIDeserialisable::DestroyPropertySets( PropertySet*& a_rpSet, PropertySet*& a_rpOrderedSet )
{
	if ( a_rpSet )
	{
		for ( TINT i = 0; i < a_rpSet->Size(); i++ )
		{
			if ( ( *a_rpSet )[ i ] )
				delete ( *a_rpSet )[ i ];
		}

		delete a_rpSet;
	}

	if ( a_rpOrderedSet )
		delete a_rpOrderedSet;

	a_rpSet        = TNULL;
	a_rpOrderedSet = TNULL;
}

TOSHI_NAMESPACE_END
