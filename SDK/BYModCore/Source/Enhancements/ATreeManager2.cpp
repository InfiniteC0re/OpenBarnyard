#include "pch.h"
#include "ATreeManager2.h"
#include "ACoreSettings.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

void ATreeManager2::Render()
{
	// NOTE: just a stub used to spoof settings
	// Adjust parameters based on current settings

	// m_fLODSwitchDistance
	*(TFLOAT*)( (TCHAR*)this + 0x6698 ) = ( g_oSettings.bDisableInstanceLODs ) ? 9999.0f : 30.0f;

	// m_fBlendingDistance
	*(TFLOAT*)( (TCHAR*)this + 0x66A0 ) = g_oSettings.flInstanceRenderDistance;

	// m_fRenderDistance
	*(TFLOAT*)( (TCHAR*)this + 0x669C ) = g_oSettings.flInstanceRenderDistance + 15.0f;
}
