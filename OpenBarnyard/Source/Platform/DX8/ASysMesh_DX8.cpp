#include "pch.h"
#include "ASysMesh_DX8.h"

#include <Render/TRenderInterface.h>
#include <Render/TOrderTable.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS(ASysMeshHAL);

ASysMeshHAL::ASysMeshHAL()
{
	m_iZBias = 0;
}

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
