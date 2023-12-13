#include "ToshiPCH.h"
#include "TIndexPoolResource_DX8.h"
#include "TIndexBlockResource_DX8.h"
#include "TIndexFactoryResource_DX8.h"
#include "TRenderInterface_DX8.h"
#include "Toshi2/T2Pair.h"

namespace Toshi {

	TFreeList TIndexBlockResource::ms_oFreeList = TFreeList(sizeof(TIndexBlockResource), 0, 8);

	TIndexBlockResource::TIndexBlockResource()
	{
		m_pFactory = TNULL;
		m_uiFlags = 0;
		m_uiMaxIndices = 0;
		m_uiOffset = 0;
		m_uiIndicesUsed = 0;
		m_uiLockCount = 0;
		m_Unk1 = 0;
	}

	TBOOL TIndexBlockResource::AttachPool(TIndexPoolResource* a_pPool)
	{
		TVALIDPTR(a_pPool);

		m_uiIndicesUsed += a_pPool->GetNumIndices();
		a_pPool->SetParent(this);

		if (m_uiFlags & 1)
		{
			Invalidate();
		}

		return TTRUE;
	}

	TBOOL TIndexBlockResource::DettachPool(TIndexPoolResource* a_pPool)
	{
		TASSERT(TTRUE == Parent()->IsExactly(&TGetClass(TIndexFactoryResource)));
		a_pPool->SetParent(Parent() == Tree()->GetRoot() ? TNULL : Parent());

		m_uiIndicesUsed -= a_pPool->GetNumIndices();

		if (m_uiIndicesUsed == 0 && Child() == TNULL)
		{
			GetRenderer()->DestroyResource(this);
			return TTRUE;
		}

		return TFALSE;
	}

	TBOOL TIndexBlockResource::Validate()
	{
		if (IsValid() && IsCreated())
		{
			return TTRUE;
		}

		if (!CreateHAL())
		{
			return TFALSE;
		}

		if (m_uiFlags & 1)
		{
			using Pair = T2Pair<TIndexBlockResource*, TIndexPoolResourceInterface::LockBuffer>;
			Pair pair;

			if (Lock(&pair.m_Second, 0))
			{
				pair.m_First = this;

				TResource::RecurseSimple(
					[](TResource* a_pResource, void* a_pUserData)
					{
						if (a_pResource->IsExactly(&TGetClass(TIndexPoolResource)))
						{
							auto pPool = TSTATICCAST(TIndexPoolResource*, a_pResource);
							auto pPair = TSTATICCAST(Pair*, a_pUserData);

							if (pPool->m_uiFlags & 1)
							{
								pPool->m_uiIndexOffset = pPair->GetSecond().uiOffset;
								pPair->GetSecond().uiOffset += pPool->GetNumIndices();

								TUtil::MemCopy(
									pPair->GetSecond().pBuffer + pPool->m_uiIndexOffset,
									pPool->GetIndices(),
									pPool->GetNumIndices() * sizeof(TIndexType)
								);
							}
						}

						return TTRUE;
					},
					this,
					&pair
				);

				Unlock();
			}
		}

		return TResource::Validate();
	}

	void TIndexBlockResource::Invalidate()
	{
		if (IsValid() && IsCreated())
		{
			TASSERT(m_uiLockCount == 0);

			DestroyHAL();
			TResource::Invalidate();
		}
	}

	TBOOL TIndexBlockResource::TryInvalidate()
	{
		if (m_uiFlags & 1)
		{
			Invalidate();
		}

		return TTRUE;
	}

	TBOOL TIndexBlockResource::TryValidate()
	{
		if (m_uiFlags & 1)
		{
			Validate();
		}

		return TTRUE;
	}

	TBOOL TIndexBlockResource::CreateHAL()
	{
		DestroyHAL();

		TMemory::HALMemInfo memInfoHAL;
		TMemory::GetHALMemInfo(memInfoHAL);

		auto pRenderer = TRenderD3DInterface::Interface();
		DWORD usage = D3DUSAGE_WRITEONLY;

		if (m_uiFlags & 1)
		{
			usage = D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY;
			m_uiOffset = 0;
		}

		if (!pRenderer->GetCurrentDevice()->SupportsHardwareTransfomations())
		{
			usage |= D3DUSAGE_SOFTWAREPROCESSING;
		}

		HRESULT hRes = pRenderer->GetDirect3DDevice()->CreateIndexBuffer(
			m_uiMaxIndices * sizeof(TIndexType),
			usage,
			D3DFMT_INDEX16,
			(m_uiFlags & 1) ? D3DPOOL_MANAGED : D3DPOOL_DEFAULT,
			&m_HALBuffer.pIndexBuffer
		);

		if (FAILED(hRes))
		{
			TRenderD3DInterface::PrintError(hRes, "Unable to create a new vertex buffer!");
			return TFALSE;
		}

		TMemory::GetHALMemInfo(memInfoHAL);
		s_iHALMemoryUsage += memInfoHAL.m_uiMemUsage;

		return TTRUE;
	}

	void TIndexBlockResource::DestroyHAL()
	{
		TMemory::HALMemInfo memInfoHAL;
		TMemory::GetHALMemInfo(memInfoHAL);

		if (m_HALBuffer.pIndexBuffer)
		{
			m_HALBuffer.pIndexBuffer->Release();
			m_HALBuffer.pIndexBuffer = TNULL;
		}

		TMemory::GetHALMemInfo(memInfoHAL);
		s_iHALMemoryUsage = s_iHALMemoryUsage - memInfoHAL.m_uiMemUsage;
	}

	TBOOL TIndexBlockResource::Lock(TIndexPoolResourceInterface::LockBuffer* a_pLockBuffer, TUINT16 a_uiNumIndices)
	{
		TVALIDPTR(a_pLockBuffer);

		DWORD uiFlags;
		TUINT uiNumIndices = 0;
		TUINT uiUnk1 = m_uiFlags & 7;

		if (uiUnk1 == 1)
		{
			uiFlags = D3DLOCK_NOSYSLOCK;
			a_pLockBuffer->uiOffset = 0;
		}
		else
		{
			if (uiUnk1 != 2)
			{
				if (uiUnk1 != 4)
				{
					return TFALSE;
				}

				Validate();
				uiNumIndices = a_uiNumIndices;

				if (m_uiMaxIndices < m_uiOffset + uiNumIndices)
				{
					uiFlags = D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
					a_pLockBuffer->uiOffset = 0;
					m_uiOffset = uiNumIndices;
				}
				else
				{
					uiFlags = D3DLOCK_NOOVERWRITE | D3DLOCK_NOSYSLOCK;
					a_pLockBuffer->uiOffset = m_uiOffset;
					m_uiOffset += uiNumIndices;
				}
			}
			else
			{
				Validate();
				uiFlags = D3DLOCK_DISCARD | D3DLOCK_NOSYSLOCK;
				a_pLockBuffer->uiOffset = 0;
			}
		}

		HRESULT hRes = m_HALBuffer.pIndexBuffer->Lock(
			a_pLockBuffer->uiOffset * sizeof(TIndexType),
			uiNumIndices * sizeof(TIndexType),
			(BYTE**)&a_pLockBuffer->pBuffer,
			uiFlags
		);

		if (FAILED(hRes))
		{
			TRenderD3DInterface::PrintError(hRes, "Couldn\'t lock stream index buffer");
		}

		m_uiLockCount += 1;
		return TTRUE;
	}

	void TIndexBlockResource::Unlock()
	{
		TASSERT(0 != m_uiLockCount);

		if (m_uiLockCount > 0)
		{
			HRESULT hRes = m_HALBuffer.pIndexBuffer->Unlock();

			if (FAILED(hRes))
			{
				TRenderD3DInterface::PrintError(hRes, "Couldn\'t unlock stream index buffer!");
			}

			m_uiLockCount -= 1;
		}
	}

	TBOOL TIndexBlockResource::Create(TIndexFactoryResourceInterface* a_pFactory, TUINT16 a_uiMaxIndices, TUINT32 a_uiFlags)
	{
		m_pFactory = a_pFactory;
		m_uiMaxIndices = a_uiMaxIndices;
		m_uiFlags = a_uiFlags;
		return TResource::Create();
	}

	TBOOL TIndexBlockResource::CanFit(TIndexPoolResource* a_pPoolResource) const
	{
		if (!HASFLAG(m_uiFlags & 1))
		{
			if (m_uiFlags & 4 && a_pPoolResource->m_uiFlags & 4)
			{
				return TTRUE;
			}
		}
		else if (a_pPoolResource->m_uiFlags & 1)
		{
			return m_uiMaxIndices > a_pPoolResource->GetNumIndices() + m_uiIndicesUsed;
		}

		return TFALSE;
	}

	void TIndexBlockResource::ChildIndexUsedChanged(TINT a_iChange)
	{
		if (a_iChange < 0)
		{
			TASSERT(m_uiIndicesUsed >= TUINT(-a_iChange));
		}

		m_uiIndicesUsed += a_iChange;
	}

	TBOOL TIndexBlockResource::GetHALBuffer(HALBuffer* a_pHALBuffer) const
	{
		TVALIDPTR(a_pHALBuffer);

		if (a_pHALBuffer)
		{
			*a_pHALBuffer = m_HALBuffer;
			return TTRUE;
		}

		return TFALSE;
	}

}
