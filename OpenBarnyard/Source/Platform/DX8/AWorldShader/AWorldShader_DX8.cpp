#include "pch.h"
#include "AWorldShader_DX8.h"

#include <d3d8.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// TODO: change type to runtime when all methods are implemented
TDEFINE_CLASS_NORUNTIME(AWorldShaderHAL);

AWorldShaderHAL::AWorldShaderHAL() :
	m_ShadowColour(0.3f, 0.3f, 0.3f, 1.0f),
	m_AmbientColour(1.0f, 1.0f, 1.0f, 1.0f)
{
	m_fUnk1 = 0.0f;
	m_pUnk2 = TNULL;
	m_eAlphaRef = 128;

	m_bIsHighEndMode = TTRUE;
	m_bAlphaBlendMaterial = TFALSE;
	m_bUnkFlag3 = TFALSE;
	m_bUnkFlag4 = TFALSE;
}

AWorldShaderHAL::~AWorldShaderHAL()
{

}

void AWorldShaderHAL::Flush()
{

}

TBOOL AWorldShaderHAL::IsHighEndMode()
{
	return m_bIsHighEndMode;
}

void AWorldShaderHAL::SetHighEndMode(TBOOL a_bEnable)
{
	m_bIsHighEndMode = a_bEnable && IsCapableShaders();
}

TBOOL AWorldShaderHAL::IsCapableShaders()
{
	D3DCAPS8 caps;

	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice = pRenderInterface->GetDirect3DDevice();

	if (!pDevice)
	{
		auto pDirect3D = pRenderInterface->GetDirect3D();
		
		TVALIDPTR(pDirect3D);
		if (!pDirect3D) return TFALSE;

		pDirect3D->GetDeviceCaps(0, D3DDEVTYPE_HAL, &caps);
	}
	else
	{
		pDevice->GetDeviceCaps(&caps);
	}

	return ((caps.MaxStreams & 0xFFFF) > 256) && (caps.PresentationIntervals & 0x10000);
}

TBOOL AWorldShaderHAL::IsRenderEnvMapEnabled()
{
	return m_bRenderEnvMap;
}

void* AWorldShaderHAL::CreateUnknown(void*, void*, void*, void*)
{
	return TNULL;
}

void AWorldShaderHAL::FlushLowEnd()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice = pRenderInterface->GetDirect3DDevice();
	auto pCurrentContext = TSTATICCAST(TRenderContextD3D*, pRenderInterface->GetCurrentRenderContext());
	
	DWORD dwWasSpecularEnabled;

	pDevice->SetVertexShader(0x1d2);
	pDevice->SetPixelShader(0);
	pDevice->SetRenderState(D3DRS_LIGHTING, 1);
	pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	pDevice->SetRenderState(D3DRS_SRCBLEND, 5);
	pDevice->SetRenderState(D3DRS_DESTBLEND, 6);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);
	pDevice->GetRenderState(D3DRS_SPECULARENABLE, &dwWasSpecularEnabled);
	pDevice->SetRenderState(D3DRS_SPECULARENABLE, 0);
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, 0);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
	pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
	pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);

	pDevice->SetRenderState(D3DRS_CULLMODE, m_bRenderEnvMap ? 3 : 2);

	static TUINT s_uiIdentityMatrixFlags;
	static TMatrix44 s_uiIdentityMatrix;

	if (!HASFLAG(s_uiIdentityMatrixFlags & 1))
	{
		s_uiIdentityMatrixFlags |= 1;
		s_uiIdentityMatrix = TMatrix44::IDENTITY;
	}

	pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&s_uiIdentityMatrix);
	pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&pCurrentContext->GetProjectionMatrix());
	pDevice->SetRenderState(D3DRS_SPECULARENABLE, dwWasSpecularEnabled);
	pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
}
