#include "pch.h"
#include "AWorldShader_DX8.h"

#include <Render/TRenderPacket.h>
#include <Platform/DX8/TRenderInterface_DX8.h>
#include <Platform/DX8/TRenderContext_DX8.h>
#include <d3d8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME(AWorldShaderHAL);

AWorldShaderHAL::AWorldShaderHAL() :
	m_ShadowColour(0.3f, 0.3f, 0.3f, 1.0f),
	m_AmbientColour(1.0f, 1.0f, 1.0f, 1.0f)
{
	m_hVertexShader = 0;
	m_pUnk2 = TNULL;
	m_iAlphaRef = 128;

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
	if (IsValidated())
	{
		auto pRenderInterface = TRenderD3DInterface::Interface();
		auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentRenderContext());
		auto pDevice = pRenderInterface->GetDirect3DDevice();

		if (!IsHighEndMode())
		{
			FlushLowEnd();
			return;
		}

		Validate();
		pDevice->SetVertexShader(m_hVertexShader);
		pDevice->SetPixelShader(NULL);

		static constexpr TVector4 s_SomeVector(1.0f, 1.0f, 1.0f, 1.0f);
		pDevice->SetVertexShaderConstant(4, &m_AmbientColour, 1);
		pDevice->SetVertexShaderConstant(5, &m_ShadowColour, 1);
		pDevice->SetVertexShaderConstant(6, &s_SomeVector, 1);

		pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);
		
		DWORD dwWasSpecularEnabled;
		pDevice->GetRenderState(D3DRS_SPECULARENABLE, &dwWasSpecularEnabled);
		pDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
		pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
		pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, 0);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
		pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
		pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
		pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);
		pDevice->SetRenderState(D3DRS_CULLMODE, m_bRenderEnvMap ? D3DCULL_CCW : D3DCULL_CW);
		pDevice->SetRenderState(D3DRS_SPECULARENABLE, dwWasSpecularEnabled);
		pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
		pDevice->SetPixelShader(NULL);
	}
}

void AWorldShaderHAL::StartFlush()
{
	if (IsValidated())
	{
		auto pRenderInterface = TRenderD3DInterface::Interface();
		auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentRenderContext());
		auto pDevice = pRenderInterface->GetDirect3DDevice();

		pDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, 1);
		pDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, 1);
		
		if (!IsHighEndMode())
		{
			FlushLowEnd();
			return;
		}

		Validate();
		pDevice->SetVertexShader(m_hVertexShader);
		pDevice->SetPixelShader(NULL);

		static constexpr TVector4 s_SomeVector(1.0f, 1.0f, 1.0f, 1.0f);
		pDevice->SetVertexShaderConstant(4, &m_AmbientColour, 1);
		pDevice->SetVertexShaderConstant(5, &m_ShadowColour, 1);
		pDevice->SetVertexShaderConstant(6, &s_SomeVector, 1);

		pDevice->SetRenderState(D3DRS_ZWRITEENABLE, 1);
		pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, 1);
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, 1);
		pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);

		DWORD dwWasSpecularEnabled;
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
		pDevice->SetRenderState(D3DRS_CULLMODE, m_bRenderEnvMap ? D3DCULL_CCW : D3DCULL_CW);
	}
}

void AWorldShaderHAL::EndFlush()
{
	auto pRenderInterface = TRenderD3DInterface::Interface();
	auto pDevice = pRenderInterface->GetDirect3DDevice();

	pDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	pDevice->SetPixelShader(NULL);
}

TBOOL AWorldShaderHAL::Create()
{
	m_aOrderTables[0].Create(this, -3000);
	m_aOrderTables[1].Create(this, 100);
	m_aOrderTables[2].Create(this, 101);
	m_aOrderTables[3].Create(this, 601);
	m_aOrderTables[4].Create(this, -400);
	m_aOrderTables[5].Create(this, 500);
	m_aOrderTables[6].Create(this, -6005);
	m_aOrderTables[7].Create(this, -7000);
	return AWorldShader::Create();
}

TBOOL AWorldShaderHAL::Validate()
{
	if (IsValidated())
	{
		return TTRUE;
	}

	// Enable high end mode if it is supported
	SetHighEndMode(TTRUE);
	
	if (IsHighEndMode())
	{
		constexpr static DWORD s_VertexShaderFunction[] = {
			0xFFFE0101, 0x14,       0xC00F0000, 0x90E40000,
			0xA0E40000, 0x2,        0x800F0001, 0xA0E40006,
			0x91E40002, 0x5,        0x800F0000, 0x90E40002,
			0xA0E40004, 0x5,        0x800F0002, 0x80E40001,
			0xA0E40005, 0x2,        0x800F0000, 0x80E40000,
			0x80E40002, 0x1,        0x80080000, 0xA0FF0006,
			0x1,	    0xD00F0000, 0x80E40000, 0x5,
			0x80010001, 0x90000002, 0xA0000008, 0x4,
			0xD0080000, 0x80FF0000, 0xA0550008, 0x80000001,
			0x2,        0xE0030000, 0xA0540007, 0x90540003,
			0x2,        0xE0030001, 0xA0FE0007, 0x90540003,
			0xFFFF
		};

		if (!TRenderD3DInterface::CreateVertexShader("AWorldShader_D3D8_Win.vsh", s_VertexShaderFunction, &m_hVertexShader))
		{
			return TFALSE;
		}
	}

	return AWorldShader::Validate();
}

void AWorldShaderHAL::Invalidate()
{
	TASSERT(!"Not implemented yet");
}

TBOOL AWorldShaderHAL::TryInvalidate()
{
	Invalidate();
	return TTRUE;
}

TBOOL AWorldShaderHAL::TryValidate()
{
	Validate();
	return TTRUE;
}

void AWorldShaderHAL::Render(TRenderPacket* a_pRenderPacket)
{
	if (IsHighEndMode() && a_pRenderPacket && a_pRenderPacket->GetMesh())
	{
		auto pRenderInterface = TRenderD3DInterface::Interface();
		auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentRenderContext());
		auto pDevice = pRenderInterface->GetDirect3DDevice();

		auto mMVP = DirectX::XMMatrixTranspose(
			DirectX::XMMatrixMultiply(a_pRenderPacket->GetModelViewMatrix(), pCurrentContext->GetProjectionMatrix())
		);

		pDevice->SetVertexShaderConstant(0, &mMVP, 4);

		static TUINT s_RenderStateFlags = 27;

		if (a_pRenderPacket->GetAlpha() >= 1.0f || isnan(a_pRenderPacket->GetAlpha()))
		{
			if (s_RenderStateFlags & 24)
			{
				pDevice->SetRenderState(D3DRS_ALPHAREF, m_iAlphaRef);
				s_RenderStateFlags &= ~24;
			}
		}
		else
		{
			pDevice->SetRenderState(D3DRS_ALPHAREF, TUINT8(m_iAlphaRef * a_pRenderPacket->GetAlpha()));
			s_RenderStateFlags |= 24;
		}
	}

	// TODO...
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
	auto pCurrentContext = TRenderContextD3D::Upcast(pRenderInterface->GetCurrentRenderContext());
	auto pDevice = pRenderInterface->GetDirect3DDevice();

	pDevice->SetVertexShader(D3DFVF_TEX1 | D3DFVF_DIFFUSE | D3DFVF_SPECULAR | D3DFVF_NORMAL | D3DFVF_XYZ);
	pDevice->SetPixelShader(NULL);
	pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
	pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	pDevice->SetRenderState(D3DRS_SRCBLEND, 5);
	pDevice->SetRenderState(D3DRS_DESTBLEND, 6);
	pDevice->SetRenderState(D3DRS_ALPHAFUNC, 5);

	DWORD dwWasSpecularEnabled;
	pDevice->GetRenderState(D3DRS_SPECULARENABLE, &dwWasSpecularEnabled);
	pDevice->SetRenderState(D3DRS_SPECULARENABLE, FALSE);
	
	pDevice->SetTextureStageState(0, D3DTSS_COLOROP, 4);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG1, 2);
	pDevice->SetTextureStageState(0, D3DTSS_COLORARG2, 0);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, 4);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, 2);
	pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, 0);
	pDevice->SetTextureStageState(1, D3DTSS_COLOROP, 1);
	pDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, 1);

	pDevice->SetRenderState(D3DRS_CULLMODE, m_bRenderEnvMap ? D3DCULL_CCW : D3DCULL_CW);

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
	pDevice->SetRenderState(D3DRS_FOGENABLE, FALSE);
}
