#include "pch.h"
#include "ASysMesh_DX8.h"

#include <Render/TRenderInterface.h>
#include <Render/TOrderTable.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( ASysMeshHAL );

ASysMeshHAL::ASysMeshHAL()
{
	m_iZBias = 0;
}

TBOOL ASysMeshHAL::Render()
{
	if ( ms_bStopRendering == FALSE )
	{
		auto pRenderPacket = GetMaterial()->AddRenderPacket( this );

		pRenderPacket->SetModelViewMatrix(
		    TRenderInterface::GetSingleton()->GetCurrentContext()->GetModelViewMatrix()
		);
	}

	return TTRUE;
}

void ASysMeshHAL::SetZBias( TINT a_iZBias )
{
	m_iZBias = a_iZBias;
}
