#include "ToshiPCH.h"
#include "TGUIComponent.h"
#include "TGUIManager.h"
#include "TGUIStyle.h"
#include "TGUIFunction.h"

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
TDEFINE_CLASS_INIT_NORUNTIME( TGUIComponent );

TPString8 TGUIComponent::str_states;
TPString8 TGUIComponent::str_state;
TPString8 TGUIComponent::str_element;
TPString8 TGUIComponent::str_param;
TPString8 TGUIComponent::str_type;
TPString8 TGUIComponent::str_marginleft;
TPString8 TGUIComponent::str_marginright;
TPString8 TGUIComponent::str_margintop;
TPString8 TGUIComponent::str_marginbottom;
TPString8 TGUIComponent::str_function;
TPString8 TGUIComponent::str_optimise_size;

TPString8          TGUIComponent::ms_saFunctionName[ Function_NUMOF ];
const TCHAR* const TGUIComponent::ms_csaFunctionNameStr[ Function_NUMOF ] = { "alpha", "size" };

// NOTE: Nicktoons creates the strings with the TClass object (static initialiser 0037b208), "optimise_size" is
// one of the pooled strings of the GUI there. The names of the elements are created here too (TGUIElement has no TClass).
// $NicktoonsPS2: FUNCTION 0037b208
// $JPOG: FUNCTION TGuiInterface.dll+00024ca0
void TGUIComponent::InitialiseClass()
{
	for ( TINT i = 0; i < Function_NUMOF; i++ )
		ms_saFunctionName[ i ] = TPString8( ms_csaFunctionNameStr[ i ] );

	str_states        = TPString8( "states" );
	str_state         = TPString8( "state" );
	str_element       = TPString8( "element" );
	str_param         = TPString8( "param" );
	str_type          = TPString8( "type" );
	str_marginleft    = TPString8( "marginleft" );
	str_marginright   = TPString8( "marginright" );
	str_margintop     = TPString8( "margintop" );
	str_marginbottom  = TPString8( "marginbottom" );
	str_function      = TPString8( "function" );
	str_optimise_size = TPString8( "optimise_size" );

	TGUIElement::InitialiseStatic();
}

// $JPOG: FUNCTION TGuiInterface.dll+00026170
void TGUIComponent::DeinitialiseClass()
{
	TGUIElement::DeinitialiseStatic();

	str_optimise_size = TPString8();
	str_function      = TPString8();
	str_marginbottom  = TPString8();
	str_margintop     = TPString8();
	str_marginright   = TPString8();
	str_marginleft    = TPString8();
	str_type          = TPString8();
	str_param         = TPString8();
	str_element       = TPString8();
	str_state         = TPString8();
	str_states        = TPString8();

	for ( TINT i = 0; i < Function_NUMOF; i++ )
		ms_saFunctionName[ i ] = TPString8();
}

// $NicktoonsPS2: FUNCTION 0037a3a0
// $JPOG: FUNCTION TGuiInterface.dll+000251e0
TGUIComponent::TGUIComponent()
    : m_iMarginLeft( 0 )
    , m_iMarginRight( 0 )
    , m_iMarginTop( 0 )
    , m_iMarginBottom( 0 )
    , m_bOptimiseSize( TFALSE )
{
	for ( TINT i = 0; i < Function_NUMOF; i++ )
		m_apFunctions[ i ] = TNULL;
}

// $NicktoonsPS2: FUNCTION 0037a420
// $JPOG: FUNCTION TGuiInterface.dll+00025b90
TGUIComponent::TGUIComponent( const TGUIComponent& a_rcOther )
    : m_iMarginLeft( a_rcOther.m_iMarginLeft )
    , m_iMarginRight( a_rcOther.m_iMarginRight )
    , m_iMarginTop( a_rcOther.m_iMarginTop )
    , m_iMarginBottom( a_rcOther.m_iMarginBottom )
    , m_bOptimiseSize( a_rcOther.m_bOptimiseSize )
{
	for ( TINT i = 0; i < Function_NUMOF; i++ )
		m_apFunctions[ i ] = a_rcOther.m_apFunctions[ i ] ? a_rcOther.m_apFunctions[ i ]->Clone() : TNULL;
}

// $NicktoonsPS2: FUNCTION 0037a4f0
// $JPOG: FUNCTION TGuiInterface.dll+00025220
TGUIComponent::~TGUIComponent()
{
	for ( TINT i = 0; i < Function_NUMOF; i++ )
	{
		if ( m_apFunctions[ i ] )
			delete m_apFunctions[ i ];
	}
}

// $NicktoonsPS2: FUNCTION 0037a5a0
// $JPOG: FUNCTION TGuiInterface.dll+00025270
TBOOL TGUIComponent::Deserialise( const PProperties* a_pProperties, TGUIStyle* a_pStyle )
{
	const PPropertyValue* pValue = a_pProperties->GetOptionalProperty( str_marginleft, PPropertyValue::TYPE_INT );
	if ( pValue ) m_iMarginLeft = TINT8( pValue->GetInteger() );

	pValue = a_pProperties->GetOptionalProperty( str_marginright, PPropertyValue::TYPE_INT );
	if ( pValue ) m_iMarginRight = TINT8( pValue->GetInteger() );

	pValue = a_pProperties->GetOptionalProperty( str_margintop, PPropertyValue::TYPE_INT );
	if ( pValue ) m_iMarginTop = TINT8( pValue->GetInteger() );

	pValue = a_pProperties->GetOptionalProperty( str_marginbottom, PPropertyValue::TYPE_INT );
	if ( pValue ) m_iMarginBottom = TINT8( pValue->GetInteger() );

	pValue = a_pProperties->GetOptionalProperty( str_optimise_size, PPropertyValue::TYPE_BOOL );
	if ( pValue ) m_bOptimiseSize = pValue->GetBoolean();

	// The elements
	TINT         iNumElements = GetComponentInfo()->GetElementCount();
	TGUIElement* pElement     = GetElements();

	for ( TINT i = 0; i < iNumElements; i++, pElement++ )
	{
		const PPropertyValue* pElementValue = a_pProperties->GetOptionalProperty( str_element, GetComponentInfo()->GetElementName( i ), PPropertyValue::TYPE_PROPS );

		if ( !pElementValue )
			continue;

		const PProperties* pElementProperties = pElementValue->GetProperties();

		// NOTE: an element that fails to deserialise doesn't fail the component
		if ( !pElement->Deserialise( pElementProperties ) )
			continue;

		const PPropertyValue* pParam = pElementProperties->GetOptionalProperty( str_param, PPropertyValue::TYPE_TPSTRING8 );

		if ( !pParam )
			continue;

		// The parameter is declared by the first element that uses it, the others must have the same type
		TINT iParameter = a_pStyle->FindParameter( pParam->GetTPString8() );

		if ( iParameter == -1 || a_pStyle->GetParameterInfo( iParameter )->GetType() == PPropertyValue::TYPE_UNDEF )
		{
			iParameter = a_pStyle->DeclareParameter( pParam->GetTPString8(), pElement->GetParameterType() );
		}
		else if ( pElement->GetParameterType() != a_pStyle->GetParameterInfo( iParameter )->GetType() )
		{
			TGUIManager::Error( TString8( TString8( "TGUIComponent: Element parameter '" ) + pParam->GetTPString8().GetString() ) + "' for element already declared in style with different type" );
			return TFALSE;
		}

		pElement->SetParameter( iParameter );
	}

	// The functions
	for ( TINT i = 0; i < Function_NUMOF; i++ )
	{
		const PPropertyValue* pFunctionValue = a_pProperties->GetOptionalProperty( str_function, ms_saFunctionName[ i ], PPropertyValue::TYPE_PROPS );

		if ( !pFunctionValue )
			continue;

		const PProperties*    pFunctionProperties = pFunctionValue->GetProperties();
		const PPropertyValue* pType               = pFunctionProperties->GetOptionalProperty( str_type, PPropertyValue::TYPE_TPSTRING8 );

		if ( !pType )
		{
			TGUIManager::Error( "TGUIComponent: Unspecified type for function '" );
			return TFALSE;
		}

		const TClass* pClass = TClass::Find( pType->GetTPString8().GetString(), &TGetClass( TGUIFunction ) );

		if ( !pClass )
		{
			TGUIManager::Error( TString8( TString8( "TGUIComponent: Unknown function type '" ) + pType->GetTPString8().GetString() ) + "'" );
			return TFALSE;
		}

		TGUIFunction* pFunction = TSTATICCAST( TGUIFunction, pClass->CreateObject() );

		// NOTE: the function is leaked if it fails to deserialise
		if ( !pFunction->Deserialise( pFunctionProperties ) )
			return TFALSE;

		m_apFunctions[ i ] = pFunction;
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 0037aae8
// $JPOG: FUNCTION TGuiInterface.dll+000256c0
TBOOL TGUIComponent::Serialise( PProperties* a_pProperties, TGUIStyle* a_pStyle )
{
	a_pProperties->PutProperty( str_type, PPropertyValue( GetComponentInfo()->GetComponentName() ) );

	if ( m_iMarginLeft != 0 )
		a_pProperties->PutProperty( str_marginleft, PPropertyValue( TINT( m_iMarginLeft ) ) );

	if ( m_iMarginRight != 0 )
		a_pProperties->PutProperty( str_marginright, PPropertyValue( TINT( m_iMarginRight ) ) );

	if ( m_iMarginTop != 0 )
		a_pProperties->PutProperty( str_margintop, PPropertyValue( TINT( m_iMarginTop ) ) );

	if ( m_iMarginBottom != 0 )
		a_pProperties->PutProperty( str_marginbottom, PPropertyValue( TINT( m_iMarginBottom ) ) );

	if ( m_bOptimiseSize == TTRUE )
		a_pProperties->PutProperty( str_optimise_size, PPropertyValue( TTRUE ) );

	// The elements that are defined
	TINT         iNumElements = GetComponentInfo()->GetElementCount();
	TGUIElement* pElement     = GetElements();

	for ( TINT i = 0; i < iNumElements; i++, pElement++ )
	{
		if ( pElement->GetElementType() == TGUIElement::ElementType_Undefined )
			continue;

		PProperties* pElementProperties = new PProperties();
		a_pProperties->PutProperty( str_element, GetComponentInfo()->GetElementName( i ), PPropertyValue( pElementProperties ) );

		if ( !pElement->Serialise( pElementProperties ) )
			return TFALSE;

		if ( pElement->GetParameterID() > 0 )
			pElementProperties->PutProperty( str_param, PPropertyValue( a_pStyle->GetParameterInfo( pElement->GetParameterID() - 1 )->GetName() ) );
	}

	// The functions
	for ( TINT i = 0; i < Function_NUMOF; i++ )
	{
		TGUIFunction* pFunction = m_apFunctions[ i ];

		if ( !pFunction )
			continue;

		PProperties* pFunctionProperties = new PProperties();
		a_pProperties->PutProperty( str_function, ms_saFunctionName[ i ], PPropertyValue( pFunctionProperties ) );
		pFunctionProperties->PutProperty( str_type, PPropertyValue( TPString8( pFunction->GetClass()->GetName() ) ) );

		if ( !pFunction->Serialise( pFunctionProperties ) )
			return TFALSE;
	}

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00025a50
TINT TGUIComponent::FindElementName( const TPString8& a_rcName ) const
{
	const TGUIComponentInfo* pInfo = GetComponentInfo();

	for ( TINT i = 0; i < pInfo->GetElementCount(); i++ )
	{
		if ( pInfo->GetElementName( i ) == a_rcName )
			return i;
	}

	return -1;
}

// $JPOG: FUNCTION TGuiInterface.dll+00026130
TGUIElement& TGUIComponent::GetElement( TINT a_iIndex )
{
	return GetElements()[ a_iIndex ];
}

// $JPOG: FUNCTION TGuiInterface.dll+00026150
TGUIElement& TGUIComponent::GetElement( const TPString8& a_rcName )
{
	return GetElement( FindElementName( a_rcName ) );
}

// $JPOG: FUNCTION TGuiInterface.dll+00025ab0
TBOOL TGUIComponent::IsReferencingTextureSection( const TGUITextureSection* a_pTextureSection ) const
{
	const TGUIElement* pElements = const_cast<TGUIComponent*>( this )->GetElements();

	for ( TINT i = 0; i < GetComponentInfo()->GetElementCount(); i++ )
	{
		if ( pElements[ i ].GetElementType() == TGUIElement::ElementType_Texture && pElements[ i ].GetTextureSection() == a_pTextureSection )
			return TTRUE;
	}

	return TFALSE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00025b20
TBOOL TGUIComponent::IsReferencingFont( const TGUIFont* a_pFont ) const
{
	const TGUIElement* pElements = const_cast<TGUIComponent*>( this )->GetElements();

	for ( TINT i = 0; i < GetComponentInfo()->GetElementCount(); i++ )
	{
		if ( pElements[ i ].GetElementType() == TGUIElement::ElementType_Label && pElements[ i ].GetFont() == a_pFont )
			return TTRUE;
	}

	return TFALSE;
}

// NOTE: inlined in the Paint functions of the components in Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+00006bc0
void TGUIComponent::ComputeSizeFunction( TFLOAT a_fTime, TGUIRectangle& a_rOffset ) const
{
	TINT iSize = TINT( m_apFunctions[ Function_Size ]->Compute( a_fTime ) );

	a_rOffset.m_iX      = -iSize / 2;
	a_rOffset.m_iY      = -iSize / 2;
	a_rOffset.m_iWidth  = iSize;
	a_rOffset.m_iHeight = iSize;
}

// NOTE: inlined in the Paint functions of the components in Nicktoons
// $JPOG: FUNCTION TGuiInterface.dll+00006c50
TINT TGUIComponent::ComputeAlphaFunction( TFLOAT a_fTime ) const
{
	TINT iAlpha = TUINT8( TINT( m_apFunctions[ Function_Alpha ]->Compute( a_fTime ) * 255.0f ) );
	TMath::Clip( iAlpha, 0, 255 );

	return iAlpha;
}

// $NicktoonsPS2: FUNCTION 0037af20
// $JPOG: FUNCTION TGuiInterface.dll+00025c10
void TGUIComponent::CopyElements( TGUIElement* a_pDestination, const TGUIElement* a_pSource, TINT a_iCount )
{
	for ( TINT i = 0; i < a_iCount; i++ )
		a_pDestination[ i ] = a_pSource[ i ];
}

TOSHI_NAMESPACE_END
