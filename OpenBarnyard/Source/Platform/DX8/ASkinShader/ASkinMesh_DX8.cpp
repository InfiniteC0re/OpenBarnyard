#include "pch.h"
#include "ASkinMesh_DX8.h"
#include "ASkinShader_DX8.h"
#include "ASkinMaterial_DX8.h"

#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS(ASkinMeshHAL);

TBOOL ASkinMeshHAL::Render()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentContext());

	if (pCurrentContext->GetSkeletonInstance())
	{
		TMaterial* pMaterial;

		if (!TDYNAMICCAST(ASkinShaderHAL, m_pOwnerShader)->IsAlphaBlendMaterial() ||
			pCurrentContext->GetAlphaBlend() >= 1.0f)
		{
			pMaterial = TDYNAMICCAST(ASkinMaterialHAL, m_pMaterial);
		}
		else
		{
			pMaterial = TDYNAMICCAST(ASkinMaterialHAL, m_pMaterial)->GetAlphaBlendMaterial();
		}

		auto pRenderPacket = pMaterial->AddRenderPacket(this);
		pRenderPacket->SetModelViewMatrix(pCurrentContext->GetModelViewMatrix());
		pRenderPacket->SetAmbientColour(pCurrentContext->GetAmbientColour().AsVector3());
		pRenderPacket->SetLightColour(pRenderInterface->GetLightColour().AsBasisVector3(0));
		pRenderPacket->SetLightDirection(pRenderInterface->GetLightDirection().AsBasisVector3(0));
		pRenderPacket->SetAlpha(pCurrentContext->GetAlphaBlend());
		pRenderPacket->SetShadeCoeff(TUINT(pCurrentContext->GetShadeCoeff() * 255.0f));

		ASkinShaderHAL::sm_oWorldViewMatrix = pCurrentContext->GetWorldViewMatrix();
		ASkinShaderHAL::sm_oViewModelMatrix = pCurrentContext->GetViewModelMatrix();
	}

	return TTRUE;
}
