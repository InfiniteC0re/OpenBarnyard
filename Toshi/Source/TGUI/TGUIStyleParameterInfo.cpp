#include "ToshiPCH.h"
#include "TGUIStyleParameterInfo.h"
#include "TGUIStyle.h"
#include "TGUIWidget.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $NicktoonsPS2: FUNCTION 00369900
TGUIStyleParameterInfo::TGUIStyleParameterInfo()
    : m_pType( PPropertyValue::TYPE_UNDEF )
    , m_iIndex( -1 )
{
	m_pSpec = new TGUIStyleParameterSpec(
	    TPString8(),
	    PPropertyValue::TYPE_UNDEF,
	    TGUIStyle::ms_strStyleParams,
	    TFALSE,
	    &TGetClass( TGUIStyle ),
	    0,
	    static_cast<TGUIStyleParameterSpec::t_Setter>( &TGUIWidget::Set_style_parameter ),
	    static_cast<TGUIStyleParameterSpec::t_Getter>( &TGUIWidget::Get_style_parameter ),
	    this
	);
}

// $NicktoonsPS2: FUNCTION 00369a20
TGUIStyleParameterInfo::~TGUIStyleParameterInfo()
{
	if ( m_pSpec )
		delete m_pSpec;
}

// $NicktoonsPS2: FUNCTION 00369ab0
TGUIStyleParameterInfo& TGUIStyleParameterInfo::operator=( const TGUIStyleParameterInfo& a_rcOther )
{
	m_strName = a_rcOther.m_strName;
	m_pType   = a_rcOther.m_pType;
	m_pSpec->Assign( *a_rcOther.m_pSpec );
	m_iIndex = a_rcOther.m_iIndex;

	return *this;
}

TOSHI_NAMESPACE_END
