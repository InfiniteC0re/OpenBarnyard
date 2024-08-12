#include "ToshiPCH.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{

void TGenericListener::Connect( TGenericEmitter& a_rEmitter, void* a_pCaller, t_Callback a_pCallback, TINT a_iPriority )
{
    TASSERT( IsLinked() == TFALSE );
    m_pCaller   = a_pCaller;
    m_pCallback = a_pCallback;
    SetPriority( a_iPriority );
    a_rEmitter.GetListeners().Insert( this );
}

void TGenericListener::Disconnect()
{
    Remove();
    m_pCaller   = TNULL;
    m_pCallback = TNULL;
}

} // namespace Toshi
