#include "pch.h"
#include "ASysMesh_DX8.h"

#include <Toshi/Render/TRenderInterface.h>
#include <Toshi/Render/TOrderTable.h>

TOSHI_NAMESPACE_USING

TBOOL ASysMeshHAL::Render()
{
	if (ms_bStopRendering == FALSE)
	{
		auto pRenderPacket = GetMaterial()->GetRegMaterial()->AddRenderPacket(this);

		pRenderPacket->SetModelViewMatrix(
			TRenderInterface::GetSingleton()->GetCurrentRenderContext()->GetModelViewMatrix()
		);
	}

	return TTRUE;
}

void ASysMeshHAL::SetZBias(TINT a_iZBias)
{
	m_iZBias = a_iZBias;
}
