#include "ToshiPCH.h"
#include "TSysShaderHAL_DX11.h"
#include "../../TRender_DX11.h"
#include "../../TRenderContext_DX11.h"

namespace Toshi {

	TSysShaderHAL::TSysShaderHAL(OrderTablePriorities* priorities)
	{
		m_pVertexShader = TNULL;
		m_pPixelShaderNoTex = TNULL;
		m_pPixelShaderTexture = TNULL;
		m_pPixelShaderAlphaTexture = TNULL;
		m_pInputLayout = TNULL;

		if (priorities)
		{
			m_Priorities = *priorities;
		}
		else
		{
			m_Priorities = {
				100,
				200,
				300,
				400,
				500,
				500,
				400
			};
		}
	}

	TSysShaderHAL::~TSysShaderHAL()
	{
		
	}

	void TSysShaderHAL::OnDestroy()
	{
		if (m_pVertexShader)
		{
			m_pVertexShader->Release();
			m_pVertexShader = TNULL;
		}

		if (m_pPixelShaderNoTex)
		{
			m_pPixelShaderNoTex->Release();
			m_pPixelShaderNoTex = TNULL;
		}

		if (m_pPixelShaderTexture)
		{
			m_pPixelShaderTexture->Release();
			m_pPixelShaderTexture = TNULL;
		}

		if (m_pPixelShaderAlphaTexture)
		{
			m_pPixelShaderAlphaTexture->Release();
			m_pPixelShaderAlphaTexture = TNULL;
		}

		if (m_pInputLayout)
		{
			m_pInputLayout->Release();
			m_pInputLayout = TNULL;
		}

		TSysShader::OnDestroy();
	}

	void TSysShaderHAL::Flush()
	{
		m_OrderTables[0].Render();
		m_OrderTables[1].Render();
	}

	void TSysShaderHAL::StartFlush()
	{
		TSysMaterialHAL::s_eSysMaterialLazyState = 1;

		auto pRender = TRenderDX11::Interface();
		auto pDeviceContext = pRender->m_pDeviceContext;

		pRender->SetColorUpdate(TTRUE);
		pRender->SetDstAlpha(0.0f);
		pRender->m_CurrentRasterizerId.Flags.Parts.CullMode = D3D11_CULL_NONE;
		pRender->SetBlendMode(TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
		pRender->SetZMode(TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);
		
		pDeviceContext->IASetInputLayout(m_pInputLayout);
		pDeviceContext->VSSetShader(m_pVertexShader, TNULL, 0);
	}

	void TSysShaderHAL::EndFlush()
	{
		auto pRender = TRenderDX11::Interface();
		pRender->SetColorUpdate(TTRUE);
		pRender->SetZMode(TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL);
		pRender->SetBlendMode(TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);
	}

	TBOOL TSysShaderHAL::Create()
	{
		TASSERT(!IsCreated());

		TBOOL bResult;

		bResult = m_OrderTables[0].Create(this, m_Priorities.Priority1, 255);
		TASSERT(bResult);

		bResult = m_OrderTables[1].Create(this, m_Priorities.Priority2, 255);
		TASSERT(bResult);

		bResult = m_OrderTables[2].Create(this, m_Priorities.Priority3, 255);
		TASSERT(bResult);

		bResult = m_OrderTables[3].Create(this, m_Priorities.Priority7, 255);
		TASSERT(bResult);

		bResult = m_OrderTables[4].Create(this, m_Priorities.Priority5, 255);
		TASSERT(bResult);

		bResult = m_OrderTables[5].Create(this, m_Priorities.Priority6, 255);
		TASSERT(bResult);

		auto pRender = TRenderDX11::Interface();
		auto pDevice = pRender->m_pDevice;
		
		// Create vertex shader
		auto pShader = pRender->CompileShaderFromFile(
			"Data/Shaders/SysShader/SysShader.vs",
			"main",
			"vs_4_0_level_9_3",
			TNULL
		);
		pRender->CreateVertexShader(
			pShader->GetBufferPointer(),
			pShader->GetBufferSize(),
			&m_pVertexShader
		);
		
		// Create input layout for the vertex shader
		D3D11_INPUT_ELEMENT_DESC inputDescs[3];
		inputDescs[0].SemanticName = "POSITION";
		inputDescs[0].SemanticIndex = 0;
		inputDescs[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		inputDescs[0].InputSlot = 0;
		inputDescs[0].AlignedByteOffset = -1;
		inputDescs[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDescs[0].InstanceDataStepRate = 0;
		inputDescs[1].SemanticName = "COLOR";
		inputDescs[1].SemanticIndex = 0;
		inputDescs[1].Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		inputDescs[1].InputSlot = 0;
		inputDescs[1].AlignedByteOffset = -1;
		inputDescs[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDescs[1].InstanceDataStepRate = 0;
		inputDescs[2].SemanticName = "TEXCOORD";
		inputDescs[2].SemanticIndex = 0;
		inputDescs[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		inputDescs[2].InputSlot = 0;
		inputDescs[2].AlignedByteOffset = -1;
		inputDescs[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		inputDescs[2].InstanceDataStepRate = 0;

		pDevice->CreateInputLayout(
			inputDescs,
			TARRAYSIZE(inputDescs),
			pShader->GetBufferPointer(),
			pShader->GetBufferSize(),
			&m_pInputLayout
		);
		pShader->Release();
		
		// Create main pixel shader
		pShader = pRender->CompileShaderFromFile(
			"Data/Shaders/SysShader/SysShader.ps",
			"main",
			"ps_4_0_level_9_3",
			TNULL
		);
		pRender->CreatePixelShader(
			pShader->GetBufferPointer(),
			pShader->GetBufferSize(),
			&m_pPixelShaderNoTex
		);
		pShader->Release();

		// Create pixel shader with support of a texture
		pShader = pRender->CompileShaderFromFile(
			"Data/Shaders/SysShader/SysShader_Texture.ps",
			"main",
			"ps_4_0_level_9_3",
			TNULL
		);
		pRender->CreatePixelShader(
			pShader->GetBufferPointer(),
			pShader->GetBufferSize(),
			&m_pPixelShaderTexture
		);
		pShader->Release();

		// Create pixel shader with support of an alpha texture
		pShader = pRender->CompileShaderFromFile(
			"Data/Shaders/SysShader/SysShader_AlphaTexture.ps",
			"main",
			"ps_4_0_level_9_3",
			TNULL
		);
		pRender->CreatePixelShader(
			pShader->GetBufferPointer(),
			pShader->GetBufferSize(),
			&m_pPixelShaderAlphaTexture
		);
		pShader->Release();

		return TSysShader::Create();
	}

	TBOOL TSysShaderHAL::Validate()
	{
		TASSERT(TTRUE == IsCreated());
		return TSysShader::Validate();
	}

	void TSysShaderHAL::Invalidate()
	{
		if (IsValidated())
		{
			TSysShader::Invalidate();
		}
	}

	void TSysShaderHAL::Render(TRenderPacket* pRenderPacket)
	{
		auto pRender        = TRenderDX11::Interface();
		auto pRenderContext = TSTATICCAST(TRenderContextDX11*, pRender->GetCurrentRenderContext());
		auto pMesh          = TSTATICCAST(TSysMeshHAL*, pRenderPacket->GetMesh());

		TMatrix44 worldViewProjection;
		worldViewProjection.Multiply(pRenderContext->GetProjectionMatrix(), pRenderPacket->GetModelViewMatrix());
		pRender->SetValueInVSBuffer(0, worldViewProjection);
		pRender->SetValueInPSBuffer(0, TSysMaterialHAL::s_AlphaTest);

		pRender->DrawIndexed(
			D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP,
			pMesh->GetIndexCount(),
			pMesh->GetIndexBuffer(),
			0,
			DXGI_FORMAT_R16_UINT,
			pMesh->GetVertexBuffer(),
			24,
			0
		);
	}

	TSysMaterial* TSysShaderHAL::CreateMaterial()
	{
		TASSERT(IsCreated());
		Validate();

		auto pMaterial = new TSysMaterialHAL;
		pMaterial->SetOwnerShader(this);

		return pMaterial;
	}

	TSysMesh* TSysShaderHAL::CreateMesh(const char* name)
	{
		TASSERT(IsCreated());
		Validate();

		auto pMesh = new TSysMeshHAL;
		pMesh->SetOwnerShader(this);

		return pMesh;
	}

}
