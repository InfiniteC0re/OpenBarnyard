#include "ToshiPCH.h"
#include "TGUIFunction.h"

#include <Plugins/PPropertyParser/PProperties.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: abstract, CreateObject returns TNULL in Nicktoons
TDEFINE_CLASS_INIT_NORUNTIME( TGUIFunction );

TPString8 TGUIFunction::ms_strFrequency;
TPString8 TGUIFunction::ms_strPhase;
TPString8 TGUIFunction::ms_strScale;
TPString8 TGUIFunction::ms_strOffset;

// NOTE: Nicktoons creates these strings with the TClass objects of the functions (static initialiser 003640c8)
// $NicktoonsPS2: FUNCTION 003640c8
void TGUIFunction::InitialiseClass()
{
	ms_strFrequency = TPString8( "frequency" );
	ms_strPhase     = TPString8( "phase" );
	ms_strScale     = TPString8( "scale" );
	ms_strOffset    = TPString8( "offset" );
}

void TGUIFunction::DeinitialiseClass()
{
	ms_strFrequency = TPString8();
	ms_strPhase     = TPString8();
	ms_strScale     = TPString8();
	ms_strOffset    = TPString8();
}

// $JPOG: FUNCTION TGuiInterface.dll+00007380
TGUIFunction::TGUIFunction()
    : m_fFrequency( 1.0f )
    , m_fPhase( 0.0f )
    , m_fScale( 1.0f )
    , m_fOffset( 0.0f )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+00007340
TGUIFunction::TGUIFunction( TFLOAT a_fFrequency, TFLOAT a_fPhase, TFLOAT a_fScale, TFLOAT a_fOffset )
    : m_fFrequency( a_fFrequency )
    , m_fPhase( a_fPhase )
    , m_fScale( a_fScale )
    , m_fOffset( a_fOffset )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+000072d0
TGUIFunction::TGUIFunction( const TGUIFunction& a_rcOther )
    : m_fFrequency( a_rcOther.m_fFrequency )
    , m_fPhase( a_rcOther.m_fPhase )
    , m_fScale( a_rcOther.m_fScale )
    , m_fOffset( a_rcOther.m_fOffset )
{
}

// $NicktoonsPS2: FUNCTION 00364360
// $JPOG: FUNCTION TGuiInterface.dll+00007330
TGUIFunction::~TGUIFunction()
{
}

// $NicktoonsPS2: FUNCTION 00363ee0
// $JPOG: FUNCTION TGuiInterface.dll+0000a950
TBOOL TGUIFunction::Deserialise( const PProperties* a_pProperties )
{
	const PPropertyValue* pValue = a_pProperties->GetOptionalProperty( ms_strFrequency, PPropertyValue::TYPE_FLOAT );
	m_fFrequency                 = pValue ? pValue->GetFloat() : 1.0f;

	pValue   = a_pProperties->GetOptionalProperty( ms_strPhase, PPropertyValue::TYPE_FLOAT );
	m_fPhase = pValue ? pValue->GetFloat() : 0.0f;

	pValue   = a_pProperties->GetOptionalProperty( ms_strScale, PPropertyValue::TYPE_FLOAT );
	m_fScale = pValue ? pValue->GetFloat() : 1.0f;

	pValue    = a_pProperties->GetOptionalProperty( ms_strOffset, PPropertyValue::TYPE_FLOAT );
	m_fOffset = pValue ? pValue->GetFloat() : 0.0f;

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 00363fd0
// $JPOG: FUNCTION TGuiInterface.dll+0000ac10
TBOOL TGUIFunction::Serialise( PProperties* a_pProperties )
{
	a_pProperties->PutProperty( ms_strFrequency, PPropertyValue( m_fFrequency ) );
	a_pProperties->PutProperty( ms_strPhase, PPropertyValue( m_fPhase ) );
	a_pProperties->PutProperty( ms_strScale, PPropertyValue( m_fScale ) );
	a_pProperties->PutProperty( ms_strOffset, PPropertyValue( m_fOffset ) );

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00007300
TFLOAT TGUIFunction::Compute( TFLOAT a_fX ) const
{
	return ComputeNormalised( a_fX * m_fFrequency + m_fPhase ) * m_fScale + m_fOffset;
}

TOSHI_NAMESPACE_END
