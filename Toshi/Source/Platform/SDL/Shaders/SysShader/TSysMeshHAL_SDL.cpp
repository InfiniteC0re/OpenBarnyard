#include "ToshiPCH.h"
#include "TSysMeshHAL_SDL.h"
#include "Toshi/Render/TOrderTable.h"
#include "../../TRender_SDL.h"

namespace Toshi {

    TBOOL TSysMeshHAL::Render()
    {
        if (!s_bStopRendering)
        {
            auto pRenderContext = TRender::GetSingleton()->GetCurrentRenderContext();
            auto pRegMaterial = GetMaterial()->GetRegMaterial();
            auto pRenderPacket = pRegMaterial->AddRenderPacket(this);

            pRenderPacket->SetModelViewMatrix(&pRenderContext->GetModelViewMatrix());
            TTODO("Set some value for the render packet (fade?)");
        }

        return TTRUE;
    }

    TBOOL TSysMeshHAL::Create(FLAGS uiFlags, uint16_t uiMaxVertices, uint16_t uiMaxIndices)
    {
        m_uiFlags = uiFlags;
        m_uiMaxVertices = uiMaxVertices;
        m_uiMaxIndices = uiMaxIndices;

		/*auto pRender = TRenderDX11::Interface();
		m_pVertexBuffer = pRender->CreateBuffer(0, uiMaxVertices * 24, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		m_pIndexBuffer = pRender->CreateBuffer(1, uiMaxIndices * 2, TNULL, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);*/
        
        m_State.Set(State::Created);
        return TTRUE;
    }

    TBOOL TSysMeshHAL::Lock(TLockBuffer* pBufferLock)
    {
        TASSERT((m_uiFlags & FLAGS_LOCKED) == 0);
		/*auto pDeviceContext = TRenderSDL::Interface()->m_pDeviceContext;

		D3D11_MAPPED_SUBRESOURCE mappedSubresource;
		pDeviceContext->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		pBufferLock->pVertexBufferData = mappedSubresource.pData;

		pDeviceContext->Map(m_pIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
		pBufferLock->pIndexBufferData = mappedSubresource.pData;*/

        pBufferLock->Unknown = 1;
        m_uiFlags |= FLAGS_LOCKED;

        return TTRUE;
    }

    void TSysMeshHAL::Unlock(uint16_t uiNumVertices, uint16_t uiNumIndices)
    {
        TASSERT(m_uiFlags & FLAGS_LOCKED);
        TASSERT(uiNumVertices >= 3);
        TASSERT(uiNumIndices >= 3);

        /*auto pDeviceContext = TRenderDX11::Interface()->m_pDeviceContext;
        pDeviceContext->Unmap(m_pVertexBuffer, 0);
        pDeviceContext->Unmap(m_pIndexBuffer, 0);*/

        m_uiFlags &= ~FLAGS_LOCKED;
        m_NumVertices = uiNumVertices;
        m_IndexCount = uiNumIndices;
    }

    void TSysMeshHAL::SetZBias(int zBias)
    {
        TASSERT(zBias >= 0);
    }

    void TSysMeshHAL::OnDestroy()
    {
		/*if (m_pVertexBuffer)
			m_pVertexBuffer->Release();

		if (m_pIndexBuffer)
			m_pIndexBuffer->Release();*/
    }

}
