#include "ToshiPCH.h"
#include "TSysMaterialHAL_SDL.h"
#include "TSysShaderHAL_SDL.h"
#include "../../TRender_SDL.h"

namespace Toshi {

	TSysMaterialHAL::~TSysMaterialHAL()
	{
		if (m_pOrderTable)
		{
			TOrderTable::DeregisterMaterial(GetRegMaterial());
			m_pOrderTable = TNULL;
		}
	}

	void TSysMaterialHAL::PreRender()
	{
		/*auto pShader = TSTATICCAST(TSysShaderHAL*, m_pOwnerShader);

		auto pRender = TRenderSDL::Interface();
		auto pDeviceContext = pRender->m_pDeviceContext;

		auto pTexture = GetTexture(0);
		s_eSysMaterialLazyState = 1;

		if (pTexture != TNULL)
		{
			pTexture->Bind(0);

			if (!HASFLAG(m_Flags & Flags_AlphaTexture))
				pDeviceContext->PSSetShader(pShader->GetPixelShaderTexture(), NULL, NULL);
			else
				pDeviceContext->PSSetShader(pShader->GetPixelShaderAlphaTexture(), NULL, NULL);
		}
		else
		{
			pDeviceContext->PSSetShader(pShader->GetPixelShaderNoTex(), NULL, NULL);
		}

		s_AlphaTest.x = 1.0f;
		s_AlphaTest.y = 0.003921569f;
		pRender->SetBlendMode(TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA);

		if (m_Flags & Flags_AlphaUpdate)
		{
			pRender->SetAlphaUpdate(TTRUE);
			pRender->SetDstAlpha(0.0f);
		}
		else
		{
			pRender->SetAlphaUpdate(TFALSE);
		}

		TBOOL bNoDepthTest = m_Flags & Flags_NoDepthTest;
		TBOOL bDepthTestEnable = !bNoDepthTest;

		if (bNoDepthTest)
		{
			pRender->SetZMode(bDepthTestEnable, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);
		}

		switch (m_eBlendMode)
		{
		case BlendMode::Default:
			if (pTexture == TNULL)
			{
				if (s_eSysMaterialLazyState != 8)
				{
					pRender->SetZMode(bDepthTestEnable, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL);
					s_eSysMaterialLazyState = 8;
					return;
				}
			}
			else if (!HASFLAG(pTexture->GetFlags1() & 1))
			{
				if (s_eSysMaterialLazyState != 0x10)
				{
					pRender->SetZMode(bDepthTestEnable, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ALL);
					s_eSysMaterialLazyState = 0x10;
					return;
				}
			}
			else if (s_eSysMaterialLazyState != 4)
			{
				s_eSysMaterialLazyState = 4;
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 0.003921569f;
				return;
			}
			break;
		case BlendMode::Mode1:
			if (s_eSysMaterialLazyState != 0x40)
			{
				s_eSysMaterialLazyState = 0x40;
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 0.003921569f;
				return;
			}
			break;
		case BlendMode::Mode2:
			if (s_eSysMaterialLazyState != 0x80)
			{
				s_eSysMaterialLazyState = 0x80;
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 0.003921569f;
				return;
			}
			break;
		case BlendMode::Mode3:
			if (s_eSysMaterialLazyState != 0x20)
			{
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 0.003921569f;
				pRender->SetBlendMode(TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE);
				s_eSysMaterialLazyState = 0x20;
				return;
			}
			break;
		case BlendMode::Mode4:
			if (s_eSysMaterialLazyState != 0x100)
			{
				s_eSysMaterialLazyState = 0x100;
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 0.003921569f;
				return;
			}
			break;
		case BlendMode::Mode5:
			if (s_eSysMaterialLazyState != 0x200)
			{
				s_eSysMaterialLazyState = 0x200;
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 0.5019608f;
				return;
			}
			break;
		case BlendMode::Mode6:
		case BlendMode::Mode7:
			if (s_eSysMaterialLazyState != 0x400)
			{
				s_AlphaTest.x = 1.0f;
				s_AlphaTest.y = 0.003921569f;
				pRender->SetBlendMode(TTRUE, D3D11_BLEND_OP_ADD, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE);
				s_eSysMaterialLazyState = 0x400;
				return;
			}
			break;
		default:
			TASSERT(TFALSE, "TSysShader default");
			break;
		}*/
	}

	void TSysMaterialHAL::PostRender()
	{
		/*if (HASFLAG(m_Flags & Flags_NoDepthTest))
		{
			auto pRender = TRenderSDL::Interface();
			pRender->SetZMode(TTRUE, D3D11_COMPARISON_LESS_EQUAL, D3D11_DEPTH_WRITE_MASK_ZERO);
		}*/
	}

	TBOOL TSysMaterialHAL::Create(BlendMode eBlendMode)
	{
		TSysMaterialHAL::SetBlendMode(eBlendMode);
		return TMaterial::Create();
	}

	void TSysMaterialHAL::SetBlendMode(BlendMode eBlendMode)
	{
		auto pShader = TSTATICCAST(TSysShaderHAL*, m_pOwnerShader);

		switch (eBlendMode)
		{
		case BlendMode::Default:
			SetOrderTable(pShader->GetOrderTable(0));
			break;
		default:
			SetOrderTable(pShader->GetOrderTable(1));
			break;
		case BlendMode::Mode3:
			SetOrderTable(pShader->GetOrderTable(2));
			break;
		case BlendMode::Mode5:
			SetOrderTable(pShader->GetOrderTable(3));
			break;
		case BlendMode::Mode6:
			SetOrderTable(pShader->GetOrderTable(4));
			break;
		case BlendMode::Mode7:
			SetOrderTable(pShader->GetOrderTable(5));
			break;
		}

		TSysMaterial::SetBlendMode(eBlendMode);
	}

	void TSysMaterialHAL::SetOrderTable(TOrderTable* pOrderTable)
	{
		if (m_pOrderTable != pOrderTable)
		{
			if (m_pOrderTable != TNULL)
			{
				TOrderTable::DeregisterMaterial(m_pRegMaterial);
			}

			if (pOrderTable != TNULL)
			{
				pOrderTable->RegisterMaterial(this);
			}

			m_pOrderTable = pOrderTable;
		}
	}

}