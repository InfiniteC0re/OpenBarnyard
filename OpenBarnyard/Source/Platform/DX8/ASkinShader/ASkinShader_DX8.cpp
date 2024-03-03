#include "pch.h"
#include "ASkinShader_DX8.h"
#include "ASkinMesh_DX8.h"
#include "ASkinMaterial_DX8.h"

#include <Platform/DX8/TRenderContext_DX8.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderAdapter_DX8.h>
#include <Platform/DX8/TTextureResourceHAL_DX8.h>
#include <Platform/DX8/TVertexBlockResource_DX8.h>
#include <Platform/DX8/TVertexPoolResource_DX8.h>
#include <Platform/DX8/TIndexBlockResource_DX8.h>
#include <Platform/DX8/TIndexPoolResource_DX8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS(ASkinShaderHAL);

#define USE_ONE_TEXTURE_LIGHTING TFALSE
#define VERTEX_SHADER_REGISTER_MVP 0
#define VERTEX_SHADER_REGISTER_BONES 5

ASkinShaderHAL::ASkinShaderHAL() :
	m_hUnknownPixelShader(0),
	m_hVertexShader(0),
	m_hVertexShaderHD(0),
	m_hPixelShader(0),
	m_iAlphaRef(128),
	m_bIsAlphaBlendMaterial(TFALSE),
	m_fAMDPatch1(0.0f),
	m_bCPUSupportsFeature1(TFALSE),
	m_bCPUSupportsFeature2(TFALSE),
	m_bUnkFlag(TFALSE),
	m_bHighEndSkinning(TTRUE),
	m_bLightScattering(TTRUE),
	m_fAMDPatch2(1.0f)
{

}

ASkinShaderHAL::~ASkinShaderHAL()
{
	
}

void ASkinShaderHAL::Flush()
{
	if (IsValidated())
	{
		auto pRenderInterface = TRenderD3DInterface::Interface();
		auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentContext());
		auto pDevice = pRenderInterface->GetDirect3DDevice();

		if (IsHighEndSkinning())
		{
			pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1);
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
			pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);

			DWORD bWasSpecularEnabled;
			pDevice->GetRenderState(D3DRS_SPECULARENABLE, &bWasSpecularEnabled);
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
			m_aOrderTables[0].Render();

			pDevice->SetRenderState(D3DRS_SPECULARENABLE, bWasSpecularEnabled);
			pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
			pDevice->SetPixelShader(0);
		}
		else
		{
			SetupLowEndMode();
		}
	}

	BaseClass::Flush();
}

void ASkinShaderHAL::StartFlush()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentContext());
	auto pDevice = pRenderInterface->GetDirect3DDevice();

	if (IsValidated())
	{
		pDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, 1);
		pDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, 1);

		if (IsHighEndSkinning())
		{
			pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1);
			pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
			pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);

			DWORD bWasSpecularEnabled;
			pDevice->GetRenderState(D3DRS_SPECULARENABLE, &bWasSpecularEnabled);
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
		}
		else
		{
			SetupLowEndMode();
			return;
		}
	}
}

void ASkinShaderHAL::EndFlush()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice = pRenderInterface->GetDirect3DDevice();

	pDevice->SetRenderState(D3DRS_SPECULARENABLE, 0);
	pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
	pDevice->SetPixelShader(0);
}

TBOOL ASkinShaderHAL::Create()
{
	m_aOrderTables[0].Create(this, -390);
	m_aOrderTables[1].Create(this, 1000);
	m_aOrderTables[2].Create(this, -400);
	return BaseClass::Create();
}

static TBOOL CreateSkinVertexShader(const char* a_szFileName, const DWORD* a_pFunction, DWORD* a_pOutVertexShader)
{
	static constexpr DWORD s_ShaderDeclaration[] =
	{
		   0x20000000, 0x40020000, 0x40020001, 0x40040002,
		   0x40040003, 0x40010004, 0xFFFFFFFF
	};

	return TRenderD3DInterface::CreateVertexShader(s_ShaderDeclaration, a_pFunction, a_pOutVertexShader);
}

TBOOL ASkinShaderHAL::Validate()
{
	if (IsValidated())
		return TTRUE;

	EnableHighEndSkinning(TTRUE);

	if (IsHighEndSkinning())
	{
		// High end mode

		constexpr static TUINT8 s_VertexShaderFunction[] = { 0x01, 0x01, 0xfe, 0xff, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x04, 0x00, 0xe4, 0xa0, 0x03, 0x00, 0xe4, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x00, 0x80, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x05, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x90, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x06, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x90, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x07, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x55, 0x80, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x05, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x55, 0x90, 0x03, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x06, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x55, 0x90, 0x04, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x07, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x55, 0x90, 0x05, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0xaa, 0x80, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x05, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xaa, 0x90, 0x03, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x06, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xaa, 0x90, 0x04, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x07, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xaa, 0x90, 0x05, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0xff, 0x80, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x05, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xff, 0x90, 0x03, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x06, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xff, 0x90, 0x04, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x07, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xff, 0x90, 0x05, 0x00, 0xe4, 0x80, 0x09, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x80, 0x03, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0x90, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x80, 0x03, 0x00, 0xe4, 0x80, 0x01, 0x00, 0xe4, 0x90, 0x09, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x80, 0x04, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0x90, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x02, 0x80, 0x04, 0x00, 0xe4, 0x80, 0x01, 0x00, 0xe4, 0x90, 0x09, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x80, 0x05, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0x90, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x80, 0x05, 0x00, 0xe4, 0x80, 0x01, 0x00, 0xe4, 0x90, 0x0d, 0x00, 0x00, 0x00, 0x02, 0x00, 0x08, 0x80, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x14, 0x00, 0x00, 0x00, 0x07, 0x00, 0x0f, 0x80, 0x02, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x07, 0x00, 0xe4, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x80, 0x02, 0x00, 0x54, 0x80, 0x5f, 0x00, 0x54, 0xa0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xaa, 0x80, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x80, 0x02, 0x00, 0x54, 0x80, 0x5f, 0x00, 0xfe, 0xa0, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x00, 0x00, 0x55, 0x80, 0x00, 0x00, 0xaa, 0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x06, 0x00, 0xe4, 0x80, 0x59, 0x00, 0xe4, 0xa1, 0x0b, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x80, 0x00, 0x00, 0x00, 0x80, 0x5d, 0x00, 0xaa, 0xa0, 0x0a, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x80, 0x02, 0x00, 0x00, 0x80, 0x5d, 0x00, 0xff, 0xa0, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x02, 0x00, 0x00, 0x80, 0x5b, 0x00, 0xa4, 0xa0, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x80, 0x02, 0x00, 0xff, 0x80, 0x02, 0x00, 0x00, 0x81, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x02, 0x00, 0x00, 0x80, 0x5a, 0x00, 0xe4, 0xa0, 0x00, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xd0, 0x00, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xd0, 0x5a, 0x00, 0xff, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x04, 0x00, 0xe4, 0x90, 0xff, 0xff, 0x00, 0x00 };
		constexpr static TUINT8 s_VertexHDShaderFunction[] = { 0x01, 0x01, 0xfe, 0xff, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x04, 0x00, 0xe4, 0xa0, 0x03, 0x00, 0xe4, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x00, 0x80, 0x05, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x05, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x90, 0x05, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x06, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x90, 0x05, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x07, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x00, 0x90, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0x55, 0x80, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x05, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x55, 0x90, 0x03, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x06, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x55, 0x90, 0x04, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x07, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0x55, 0x90, 0x05, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0xaa, 0x80, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x05, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xaa, 0x90, 0x03, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x06, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xaa, 0x90, 0x04, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x07, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xaa, 0x90, 0x05, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xb0, 0x00, 0x00, 0xff, 0x80, 0x04, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x05, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xff, 0x90, 0x03, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x06, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xff, 0x90, 0x04, 0x00, 0xe4, 0x80, 0x04, 0x00, 0x00, 0x00, 0x05, 0x00, 0x0f, 0x80, 0x07, 0x20, 0xe4, 0xa0, 0x02, 0x00, 0xff, 0x90, 0x05, 0x00, 0xe4, 0x80, 0x09, 0x00, 0x00, 0x00, 0x02, 0x00, 0x01, 0x80, 0x03, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0x90, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x01, 0x80, 0x03, 0x00, 0xe4, 0x80, 0x01, 0x00, 0xe4, 0x90, 0x09, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x80, 0x04, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0x90, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x02, 0x80, 0x04, 0x00, 0xe4, 0x80, 0x01, 0x00, 0xe4, 0x90, 0x09, 0x00, 0x00, 0x00, 0x02, 0x00, 0x04, 0x80, 0x05, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0x90, 0x08, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x80, 0x05, 0x00, 0xe4, 0x80, 0x01, 0x00, 0xe4, 0x90, 0x0d, 0x00, 0x00, 0x00, 0x02, 0x00, 0x08, 0x80, 0x02, 0x00, 0x00, 0x80, 0x02, 0x00, 0x00, 0x80, 0x14, 0x00, 0x00, 0x00, 0x07, 0x00, 0x0f, 0x80, 0x02, 0x00, 0xe4, 0x80, 0x00, 0x00, 0xe4, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xc0, 0x07, 0x00, 0xe4, 0x80, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x80, 0x06, 0x00, 0xe4, 0x80, 0x59, 0x00, 0xe4, 0xa1, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x80, 0x06, 0x00, 0xe4, 0x80, 0x5e, 0x00, 0xe4, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0xd0, 0x5a, 0x00, 0xff, 0xa0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xe0, 0x04, 0x00, 0xe4, 0x90, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x54, 0x80, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x54, 0x80, 0x01, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x54, 0x80, 0x01, 0x00, 0x00, 0x00, 0x04, 0x00, 0x03, 0xe0, 0x00, 0x00, 0x54, 0x80, 0xff, 0xff, 0x00, 0x00 };
		
		if (CreateSkinVertexShader("ASkinShader_D3D8_Win.vsh", (DWORD*)s_VertexShaderFunction, &m_hVertexShader) &&
			CreateSkinVertexShader("ASkinShader_D3D8_Win.vsh", (DWORD*)s_VertexHDShaderFunction, &m_hVertexShaderHD))
		{
			constexpr static TUINT8 s_PixelShaderFunction[] = { 0x04, 0x01, 0xff, 0xff, 0xfd, 0xff, 0x00, 0x00, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x80, 0x00, 0x00, 0xe4, 0xb0, 0x42, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0f, 0x80, 0x01, 0x00, 0xe4, 0xb0, 0x42, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0f, 0x80, 0x02, 0x00, 0xe4, 0xb0, 0x42, 0x00, 0x00, 0x00, 0x03, 0x00, 0x0f, 0x80, 0x03, 0x00, 0xe4, 0xb0, 0x42, 0x00, 0x00, 0x00, 0x04, 0x00, 0x0f, 0x80, 0x04, 0x00, 0xe4, 0xb0, 0x12, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0f, 0x80, 0x00, 0x00, 0xe4, 0xa0, 0x01, 0x00, 0xe4, 0x80, 0x03, 0x00, 0xe4, 0x80, 0x12, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0f, 0x80, 0x00, 0x00, 0xe4, 0xa0, 0x02, 0x00, 0xe4, 0x80, 0x04, 0x00, 0xe4, 0x80, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x80, 0x00, 0x00, 0xe4, 0x80, 0x01, 0x00, 0xe4, 0x80, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x80, 0x00, 0x00, 0xe4, 0x80, 0x02, 0x00, 0xe4, 0x80, 0x05, 0x00, 0x00, 0x40, 0x00, 0x00, 0x08, 0x80, 0x00, 0x00, 0xff, 0x80, 0x00, 0x00, 0xff, 0x90, 0xff, 0xff, 0x00, 0x00 };
			constexpr static TUINT8 s_PixelShaderFunctionLowEnd[] = { 0x01, 0x01, 0xff, 0xff, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xb0, 0x42, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0f, 0xb0, 0x42, 0x00, 0x00, 0x00, 0x02, 0x00, 0x0f, 0xb0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x80, 0x01, 0x00, 0xe4, 0xb1, 0x00, 0x00, 0xe4, 0xa0, 0x00, 0x00, 0xe4, 0xb0, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x17, 0x80, 0x01, 0x00, 0xe4, 0xa0, 0x02, 0x00, 0xe4, 0xb0, 0x00, 0x00, 0xe4, 0x80, 0x01, 0x00, 0x00, 0x40, 0x00, 0x00, 0x08, 0x80, 0x00, 0x00, 0xff, 0xb0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
			
			auto pRenderInterface = TRenderD3DInterface::Interface();
			auto pD3DDevice = pRenderInterface->GetDirect3DDevice();
			auto pDevice = TSTATICCAST(TD3DAdapter::Mode::Device*, pRenderInterface->GetCurrentDevice());

			if (pDevice->GetD3DCaps().PixelShaderVersion < 0xffff0104)
			{
				TRenderD3DInterface::CreatePixelShader((DWORD*)s_PixelShaderFunctionLowEnd, &m_hPixelShader);
			}
			else
			{
				TRenderD3DInterface::CreatePixelShader((DWORD*)s_PixelShaderFunction, &m_hPixelShader);
			}

			ApplyGPUSpecificPatches();
			CheckCPUFeature1();
			CheckCPUFeature2();

			return BaseClass::Validate();
		}
		else
		{
			return TFALSE;
		}
	}
	else
	{
		// Low end mode

		ApplyGPUSpecificPatches();
		CheckCPUFeature1();
		CheckCPUFeature2();

		return BaseClass::Validate();
	}
}

void ASkinShaderHAL::Invalidate()
{
	if (!IsValidated()) return;

	if (!IsHighEndSkinning())
	{
		m_SomeList.DeleteAll();
	}
	else
	{
		if (m_hVertexShader != 0)
		{
			TRenderD3DInterface::DestroyVertexShader(m_hVertexShader);
			m_hVertexShader = 0;
		}

		if (m_hVertexShaderHD != 0)
		{
			TRenderD3DInterface::DestroyVertexShader(m_hVertexShaderHD);
			m_hVertexShaderHD = 0;
		}

		if (m_hUnknownPixelShader != 0)
		{
			TRenderD3DInterface::DestroyPixelShader(m_hUnknownPixelShader);
			m_hUnknownPixelShader = 0;
		}

		if (m_hPixelShader != 0)
		{
			TRenderD3DInterface::DestroyPixelShader(m_hPixelShader);
			m_hPixelShader = 0;
		}
	}

	BaseClass::Invalidate();
}

TBOOL ASkinShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL ASkinShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void ASkinShaderHAL::Render(Toshi::TRenderPacket* a_pRenderPacket)
{
	if (IsHighEndSkinning())
	{
		if (a_pRenderPacket && a_pRenderPacket->GetMesh())
		{
			auto pMesh = ASkinMeshHAL::Upcast(a_pRenderPacket->GetMesh());
			auto pMaterial = ASkinMaterialHAL::Upcast(pMesh->GetMaterial());

			if (pMesh->GetNumSubMeshes() == 0) return;

			auto pSkeletonInstance = a_pRenderPacket->GetSkeletonInstance();
			TVALIDPTR(pSkeletonInstance);

			TVector4 vLightingLerp1 = TVector4(1.0f, 1.0f, 1.0f, 1.0f);
			TVector4 vLightingLerp2 = TVector4(1.0f, 1.0f, 1.0f, 1.0f);

			auto pRenderInterface = TRenderD3DInterface::Interface();
			auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentContext());
			auto pD3DDevice = pRenderInterface->GetDirect3DDevice();

			if (pMaterial->IsSkin() && pMaterial->HasLighting1Tex() && pMaterial->HasLighting2Tex())
			{
				// Apply lighting to the skin mesh that uses the HD shader
				// HD shader uses 4 shading textures to character models look good

				Validate();
				pD3DDevice->SetVertexShader(m_hVertexShaderHD);
				pD3DDevice->SetPixelShader(m_hPixelShader);

				TMatrix44 oViewModel;
				oViewModel.Invert(a_pRenderPacket->GetModelViewMatrix());

				TVector4 upVector;
				upVector.Negate3(oViewModel.AsBasisVector4(2));

				pD3DDevice->SetVertexShaderConstant(94, &upVector, 1);

				TUINT ui8ShadeCoeff = a_pRenderPacket->GetShadeCoeff();
				TFLOAT fShadeCoeff = a_pRenderPacket->GetShadeCoeff() * (1.0f / 255.0f);

				auto pDevice = TSTATICCAST(TD3DAdapter::Mode::Device*, pRenderInterface->GetCurrentDevice());

				if (pDevice->GetD3DCaps().PixelShaderVersion < 0xffff0104)
				{
					// Can't set all 4 textures at once

					TTextureResourceHAL* pLightingTex1;
					TTextureResourceHAL* pLightingTex2;

					if (ui8ShadeCoeff == 0)
					{
						pLightingTex1 = pMaterial->GetLightingTexture(ASkinMaterial::LT_0);
						pLightingTex2 = pMaterial->GetLightingTexture(ASkinMaterial::LT_1);
					}
					else if (ui8ShadeCoeff == 255)
					{
						pLightingTex1 = pMaterial->GetLightingTexture(ASkinMaterial::LT_2);
						pLightingTex2 = pMaterial->GetLightingTexture(ASkinMaterial::LT_3);
					}
					else if (ui8ShadeCoeff < 127)
					{
						pLightingTex1 = pMaterial->GetLightingTexture(ASkinMaterial::LT_0);
						pLightingTex2 = pMaterial->GetLightingTexture(ASkinMaterial::LT_1);
						
						vLightingLerp2.x = (1.0f - (fShadeCoeff + fShadeCoeff)) * 0.3f + 0.7f;
						vLightingLerp2.y = vLightingLerp2.x;
						vLightingLerp2.z = vLightingLerp2.x;
					}
					else
					{
						pLightingTex1 = pMaterial->GetLightingTexture(ASkinMaterial::LT_2);
						pLightingTex2 = pMaterial->GetLightingTexture(ASkinMaterial::LT_3);
						
						vLightingLerp1.x = ((fShadeCoeff - 0.5f) + (fShadeCoeff - 0.5f)) * 0.3f + 0.7f;
						vLightingLerp1.y = vLightingLerp1.x;
						vLightingLerp1.z = vLightingLerp1.x;
					}

					pD3DDevice->SetPixelShaderConstant(0, &vLightingLerp1, 1);
					pD3DDevice->SetPixelShaderConstant(1, &vLightingLerp2, 1);
					pD3DDevice->SetTexture(1, pLightingTex1->GetD3DTexture());
					pD3DDevice->SetTextureStageState(1, D3DTSS_MIPFILTER, 0);
					pD3DDevice->SetTexture(2, pLightingTex2->GetD3DTexture());
					pD3DDevice->SetTextureStageState(2, D3DTSS_MIPFILTER, 0);
				}
				else
				{
					// Can set all 4 textures at once

					if (pMaterial->GetSomeTexture() && pRenderInterface->GetCurrentDevice() && !USE_ONE_TEXTURE_LIGHTING)
						fShadeCoeff = fShadeCoeff - 0.3f;

					vLightingLerp1.x = 1.0f - fShadeCoeff;
					vLightingLerp1.y = vLightingLerp1.x;
					vLightingLerp1.z = vLightingLerp1.x;
					vLightingLerp1.w = 0.0f;
					
					vLightingLerp2.x = fShadeCoeff;
					vLightingLerp2.y = fShadeCoeff;
					vLightingLerp2.z = fShadeCoeff;
					vLightingLerp2.w = 0.0f;

					pD3DDevice->SetTexture(1, pMaterial->GetLightingTexture(ASkinMaterial::LT_0)->GetD3DTexture());
					pD3DDevice->SetTextureStageState(1, D3DTSS_MIPFILTER, 0);

					pD3DDevice->SetTexture(2, pMaterial->GetLightingTexture(ASkinMaterial::LT_1)->GetD3DTexture());
					pD3DDevice->SetTextureStageState(2, D3DTSS_MIPFILTER, 0);

					pD3DDevice->SetTexture(3, pMaterial->GetLightingTexture(ASkinMaterial::LT_2)->GetD3DTexture());
					pD3DDevice->SetTextureStageState(3, D3DTSS_MIPFILTER, 0);

					pD3DDevice->SetTexture(4, pMaterial->GetLightingTexture(ASkinMaterial::LT_3)->GetD3DTexture());
					pD3DDevice->SetTextureStageState(4, D3DTSS_MIPFILTER, 0);

					pD3DDevice->SetPixelShaderConstant(0, &vLightingLerp1, 1);
					pD3DDevice->SetPixelShaderConstant(1, &vLightingLerp2, 1);
				}
			}
			else
			{
				// Apply lighting to the skin mesh that doesn't use the HD shader

				TVector4 upVector = TVector4(0.0f, 0.0f, 0.0f, 1.0f);
				pD3DDevice->SetVertexShaderConstant(94, &upVector, 1);

				Validate();
				pD3DDevice->SetVertexShader(m_hVertexShader);
				pD3DDevice->SetPixelShader(0);

				auto pSomeTexture = pMaterial->GetSomeTexture();

				if (pSomeTexture && pRenderInterface->GetCurrentDevice() && USE_ONE_TEXTURE_LIGHTING)
				{
					constexpr TFLOAT kLightingLerp1 = 0.00f;
					constexpr TFLOAT kLightingLerp2 = 0.25f;
					
					pD3DDevice->SetTextureStageState(3, D3DTSS_COLOROP, 1);
					pD3DDevice->SetTexture(1, pSomeTexture->GetD3DTexture());
					pD3DDevice->SetTexture(2, pSomeTexture->GetD3DTexture());
					pD3DDevice->SetTextureStageState(1, D3DTSS_MIPFILTER, 2);
					pD3DDevice->SetTextureStageState(2, D3DTSS_MIPFILTER, 2);
					vLightingLerp1.x = kLightingLerp1;
					vLightingLerp1.y = kLightingLerp1;
					vLightingLerp1.z = kLightingLerp1;
					vLightingLerp1.w = 0.0f;

					vLightingLerp2.x = kLightingLerp2;
					vLightingLerp2.y = kLightingLerp2;
					vLightingLerp2.z = kLightingLerp2;
					vLightingLerp2.w = 0.0f;

					pD3DDevice->SetPixelShaderConstant(0, &vLightingLerp1, 1);
					pD3DDevice->SetPixelShaderConstant(1, &vLightingLerp2, 1);
					pD3DDevice->SetPixelShader(m_hPixelShader);
				}
			}

			TMatrix44 oModelViewProjection;
			D3DXMatrixMultiply(oModelViewProjection, a_pRenderPacket->GetModelViewMatrix(), pCurrentContext->GetProjectionMatrix());
			D3DXMatrixTranspose(oModelViewProjection, oModelViewProjection);

			static constexpr TVector4 s_vUnknown = TVector4(256.0f, 256.0f, 256.0f, 256.0f);
			pD3DDevice->SetVertexShaderConstant(VERTEX_SHADER_REGISTER_MVP, oModelViewProjection, 4);
			pD3DDevice->SetVertexShaderConstant(4, &s_vUnknown, 1);

			TFLOAT fLightDirX = -a_pRenderPacket->GetLightDirection().x;
			TFLOAT fLightDirY = -a_pRenderPacket->GetLightDirection().y;
			TFLOAT fLightDirZ = -a_pRenderPacket->GetLightDirection().z;
			TVector4 vLightColour = a_pRenderPacket->GetLightColour();

			TVector4 vAmbientColour = a_pRenderPacket->GetAmbientColour();
			vAmbientColour.w = a_pRenderPacket->GetAlpha();

			TMatrix44 oModelView = a_pRenderPacket->GetModelViewMatrix();
			TMatrix44 oWorldModelView;

			for (TINT i = 0; i < 4; i++)
			{
				oWorldModelView.AsBasisVector4(i).x = oModelView.m_f11 * sm_oWorldViewMatrix.AsBasisVector3(i).x + oModelView.m_f12 * sm_oWorldViewMatrix.AsBasisVector3(i).y + oModelView.m_f13 * sm_oWorldViewMatrix.AsBasisVector3(i).z;
				oWorldModelView.AsBasisVector4(i).y = oModelView.m_f21 * sm_oWorldViewMatrix.AsBasisVector3(i).x + oModelView.m_f22 * sm_oWorldViewMatrix.AsBasisVector3(i).y + oModelView.m_f23 * sm_oWorldViewMatrix.AsBasisVector3(i).z;
				oWorldModelView.AsBasisVector4(i).z = oModelView.m_f31 * sm_oWorldViewMatrix.AsBasisVector3(i).x + oModelView.m_f32 * sm_oWorldViewMatrix.AsBasisVector3(i).y + oModelView.m_f33 * sm_oWorldViewMatrix.AsBasisVector3(i).z;
				oWorldModelView.AsBasisVector4(i).w = oModelView.m_f14 * sm_oWorldViewMatrix.AsBasisVector3(i).x + oModelView.m_f24 * sm_oWorldViewMatrix.AsBasisVector3(i).y + oModelView.m_f34 * sm_oWorldViewMatrix.AsBasisVector3(i).z;
			}

			Toshi::TVector3 vLightDirWorld;
			vLightDirWorld.x = oWorldModelView.m_f11 * fLightDirX + oWorldModelView.m_f21 * fLightDirY + oWorldModelView.m_f31 * fLightDirZ;
			vLightDirWorld.y = oWorldModelView.m_f12 * fLightDirX + oWorldModelView.m_f22 * fLightDirY + oWorldModelView.m_f32 * fLightDirZ;
			vLightDirWorld.z = oWorldModelView.m_f13 * fLightDirX + oWorldModelView.m_f23 * fLightDirY + oWorldModelView.m_f33 * fLightDirZ;
			vLightDirWorld.Normalize();

			TVector4 vUnkVector = TVector4(1.0f, 0.0f, 0.0f, 1.0f);

			pD3DDevice->SetVertexShaderConstant(89, &vLightDirWorld, 1);
			pD3DDevice->SetVertexShaderConstant(90, &vAmbientColour, 1);
			pD3DDevice->SetVertexShaderConstant(91, &vLightColour, 1);
			pD3DDevice->SetVertexShaderConstant(93, &vUnkVector, 1);

			TINT iAlpha = TINT(a_pRenderPacket->GetAlpha() * 128.0f);
			if (m_iAlphaRef < iAlpha) iAlpha = m_iAlphaRef;
			pD3DDevice->SetRenderState(D3DRS_ALPHAREF, iAlpha);

			if (pMaterial->GetBlendMode() != 0 || a_pRenderPacket->GetAlpha() < 1.0f)
			{
				if (ISZERO(sm_eRenderStateFlags & 0x1b))
				{
					pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
					sm_eRenderStateFlags |= 0x1b;
				}
			}
			else if (!ISZERO(sm_eRenderStateFlags & 0x1b))
			{
				pD3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				sm_eRenderStateFlags &= ~0x1b;
			}

			auto pVertexPool = TVertexPoolResource::Upcast(pMesh->GetVertexPool());

			TVertexBlockResource::HALBuffer vertexHALBuffer;
			pVertexPool->GetHALBuffer(&vertexHALBuffer);
			pD3DDevice->SetStreamSource(0, vertexHALBuffer.apVertexBuffers[0], 0x28);

			for (TINT i = 0; i < pMesh->GetNumSubMeshes(); i++)
			{
				auto pSubMesh = pMesh->GetSubMesh(i);

				TINT iCurrentRegister = VERTEX_SHADER_REGISTER_BONES;
				
				for (TUINT k = 0; k < pSubMesh->uiNumBones; k++)
				{
					auto pBone = &pSkeletonInstance->GetBone(pSubMesh->aBones[k]);
					
					TMatrix44 oBoneTransform;
					D3DXMatrixTranspose(oBoneTransform, pBone->m_Transform);
					pD3DDevice->SetVertexShaderConstant(iCurrentRegister, oBoneTransform, 3);

					iCurrentRegister += 3;
				}

				auto pIndexPool = TIndexPoolResource::Upcast(pSubMesh->pIndexPool);
				TVALIDPTR(pIndexPool);

				TIndexBlockResource::HALBuffer indexHALBuffer;
				pIndexPool->GetHALBuffer(&indexHALBuffer);

				pD3DDevice->SetIndices(indexHALBuffer.pIndexBuffer, vertexHALBuffer.uiVertexOffset);
				pD3DDevice->DrawIndexedPrimitive(
					D3DPT_TRIANGLESTRIP,
					0,
					pVertexPool->GetNumVertices(),
					indexHALBuffer.uiIndexOffset,
					pIndexPool->GetNumIndices()
				);
			}
		}
	}
	else
	{
		RenderLowEnd(a_pRenderPacket);
	}
}

void ASkinShaderHAL::EnableRenderEnvMap(TBOOL a_bEnable)
{
	m_bRenderEnvMap = a_bEnable;
}

TBOOL ASkinShaderHAL::IsHighEndSkinning()
{
	return m_bHighEndSkinning;
}

void ASkinShaderHAL::EnableHighEndSkinning(TBOOL a_bEnable)
{
	m_bHighEndSkinning = a_bEnable && IsCapableHighEndSkinning();
}

TBOOL ASkinShaderHAL::IsCapableHighEndSkinning()
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

	return ((caps.VertexShaderVersion & 0xFFFF) > 256) && (caps.DevCaps & 0x10000);
}

TBOOL ASkinShaderHAL::IsLightScattering()
{
	return m_bLightScattering;
}

void ASkinShaderHAL::SetLightScattering(TBOOL a_bEnable)
{

}

TBOOL ASkinShaderHAL::IsAlphaBlendMaterial()
{
	return m_bIsAlphaBlendMaterial;
}

void ASkinShaderHAL::SetAlphaBlendMaterial(TBOOL a_bIsAlphaBlendMaterial)
{
	m_bIsAlphaBlendMaterial = a_bIsAlphaBlendMaterial;
}

ASkinMaterial* ASkinShaderHAL::CreateMaterial(const TCHAR* a_szName)
{
	Validate();

	auto pMaterial = new ASkinMaterialHAL();
	pMaterial->SetShader(this);

	if (TNULL != a_szName)
		pMaterial->SetName(a_szName);

	if (ASkinShaderHAL::IsAlphaBlendMaterial())
	{
		auto pAlphaBlendMaterial = new ASkinMaterialHAL();
		pAlphaBlendMaterial->SetShader(this);
		pAlphaBlendMaterial->Create(1);

		pMaterial->SetAlphaBlendMaterial(pAlphaBlendMaterial);
	}

	return pMaterial;
}

ASkinMesh* ASkinShaderHAL::CreateMesh(const TCHAR* a_szName)
{
	Validate();

	auto pMesh = new ASkinMeshHAL();
	pMesh->SetOwnerShader(this);

	return pMesh;
}

TINT ASkinShaderHAL::AddLight(const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity)
{
	TASSERT(!"Not implemeted in the original game");
	return 0;
}

void ASkinShaderHAL::SetLight(TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity)
{
	TASSERT(!"Not implemeted in the original game");
}

void ASkinShaderHAL::RemoveLight(TINT a_iIndex)
{
	TASSERT(!"Not implemeted in the original game");
}

TBOOL ASkinShaderHAL::IsEnableRenderEnvMap()
{
	return m_bRenderEnvMap;
}

void ASkinShaderHAL::SetSomeColour(TUINT a_uiR, TUINT a_uiG, TUINT a_uiB, TUINT a_uiA)
{
	m_SomeColour = ((a_uiA << 8 | a_uiB) << 8 | a_uiG) << 8 | a_uiR;
}

TINT ASkinShaderHAL::SetUnknown1(TINT a_Unknown, TUINT8 a_fAlpha)
{
	if (m_SomeColourComponents[1] < 2)
		m_Unk3 = a_fAlpha * (1.0f / 128.0f);
	else if (m_SomeColourComponents[0] == 0)
		m_Unk3 = 1.0f;
	else
		m_Unk3 = 0.0f;

	return m_Unk2;
}

void ASkinShaderHAL::SetUnknown2(TINT a_Unknown)
{
	m_Unk2 = a_Unknown;
}

void ASkinShaderHAL::SetAlphaRef(TINT a_iAlphaRef)
{
	auto pRender = TRenderD3DInterface::Interface();
	auto pD3DDevice = pRender->GetDirect3DDevice();

	pD3DDevice->SetRenderState(D3DRS_ALPHAREF, 1);
	m_iAlphaRef = a_iAlphaRef;
}

void ASkinShaderHAL::RenderLowEnd(Toshi::TRenderPacket* a_pRenderPacket)
{
	TASSERT(!"Hello there");
}

void ASkinShaderHAL::SetupLowEndMode()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentContext());
	auto pDevice = pRenderInterface->GetDirect3DDevice();

	pDevice->SetVertexShader(0x1d2);
	pDevice->SetPixelShader(0);
	pDevice->SetRenderState(D3DRS_LIGHTING, 1);
	pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
	pDevice->SetRenderState(D3DRS_SRCBLEND, 5);
	pDevice->SetRenderState(D3DRS_DESTBLEND, 6);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);
	
	DWORD bWasSpecularEnabled;
	pDevice->GetRenderState(D3DRS_SPECULARENABLE, &bWasSpecularEnabled);
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

	if (ISZERO(s_uiIdentityMatrixFlags & 1))
	{
		s_uiIdentityMatrixFlags |= 1;
		s_uiIdentityMatrix = TMatrix44::IDENTITY;
	}

	pDevice->SetTransform(D3DTS_VIEW, s_uiIdentityMatrix);
	pDevice->SetTransform(D3DTS_PROJECTION, pCurrentContext->GetProjectionMatrix());
	m_aOrderTables[0].Render();

	pDevice->SetRenderState(D3DRS_SPECULARENABLE, bWasSpecularEnabled);
	pDevice->SetRenderState(D3DRS_FOGENABLE, 0);
}

void ASkinShaderHAL::ApplyGPUSpecificPatches()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice = pRenderInterface->GetCurrentDevice();
	auto pMode = pDevice->GetMode();
	auto pAdapter = pMode->GetAdapter();

	if (T2String8::CompareNoCase(pAdapter->GetDriverDescription(), "RADEON 8500 / RADEON 8500LE") == 0)
	{
		m_fAMDPatch1 = 0.005f;
		m_fAMDPatch2 = 0.99995f;
	}
}

void ASkinShaderHAL::CheckCPUFeature1()
{
	TIMPLEMENT();
	m_bCPUSupportsFeature1 = TTRUE;
}

void ASkinShaderHAL::CheckCPUFeature2()
{
	TIMPLEMENT();
	m_bCPUSupportsFeature2 = TTRUE;
}
