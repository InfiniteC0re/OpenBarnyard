#include "pch.h"
#include "AConversationManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00543ec0, CreateObject 00544050, CreateObjectInPlace 005440b0
TDEFINE_CLASS( AConversationManager );

// $Barnyard: FUNCTION 00543f90
AConversationManager::AConversationManager()
    : m_iUnk30( 0 )
    , m_fUnk34( 200.0f )
    , m_fUnk38( 5.0f )
    , m_bUnk3C( TTRUE )
    , m_uiTargetSequenceId( TUINT( -1 ) )
    , m_iUnk60( 0 )
    , m_iUnk64( 0 )
{
}

// $Barnyard: FUNCTION 00543ee0
// $Barnyard: FUNCTION 00544030
AConversationManager::~AConversationManager()
{
	m_llUnk58.RemoveAll();
	m_iUnk60 = 0;
	m_iUnk64 = 0;
	m_oListener.Disconnect();
}
