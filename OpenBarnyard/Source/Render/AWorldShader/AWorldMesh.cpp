#include "pch.h"
#include "AWorldMesh.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME(AWorldMesh);

AWorldMesh::AWorldMesh() :
	m_uiFlags(0),
	m_uiMaxVertices(0),
	m_pVertexPool(TNULL)
{
	
}

TBOOL AWorldMesh::Validate()
{
	if (!IsValidated())
	{
		TVALIDPTR(m_pVertexPool);

		m_pVertexPool->Validate();
		TMesh::Validate();
	}

	return TTRUE;
}

void AWorldMesh::Invalidate()
{
	TMesh::Invalidate();
}

void AWorldMesh::OnDestroy()
{
	TTODO("Call AModelLoader::DestroyMaterial\n");

	if (m_pVertexPool)
	{
		TRenderInterface::GetSingleton()->DestroyResource(m_pVertexPool);
		m_pVertexPool = TNULL;
	}

	m_pSomeArray.Destroy();
	m_uiFlags = 0;
	m_uiMaxVertices = 0;

	TMesh::OnDestroy();
}

void AWorldMesh::Create(TUINT32 a_uiUnknown, TUINT16 a_uiMaxVertices)
{
	TASSERT(!IsCreated());

	m_uiMaxVertices = a_uiMaxVertices;
	m_uiFlags = a_uiUnknown;

	if (CreateResource())
	{
		TMesh::Create();
	}
	else
	{
		TASSERT(!"Couldn't create AWorldMesh");
	}
}

TBOOL AWorldMesh::Lock(LockBuffer& a_rLockBuffer)
{
	if (m_pVertexPool->Lock(&a_rLockBuffer))
	{
		m_uiFlags |= FLAG_LOCKED;
		return TTRUE;
	}

	return TFALSE;
}

void AWorldMesh::Unlock(TUINT32 a_uiNumVertices)
{
	if (TINT16(a_uiNumVertices) == -1)
	{
		a_uiNumVertices = m_pVertexPool->GetNumVertices();
	}

	m_pVertexPool->Unlock(a_uiNumVertices);
	m_uiFlags &= ~FLAG_LOCKED;
}

TVertexPoolResourceInterface* AWorldMesh::GetVertexPool()
{
	return m_pVertexPool;
}

TBOOL AWorldMesh::CreateResource()
{
	auto pVertexFactory = TRenderInterface::GetSingleton()->GetSystemResource<TVertexFactoryResourceInterface>(TRenderInterface::SYSRESOURCE_VFWORLD);
	TVALIDPTR(pVertexFactory);

	m_pVertexPool = pVertexFactory->CreatePoolResource(m_uiMaxVertices, 1);
	m_pSomeArray.Create(NUM_SUBMESHES);

	return TTRUE;
}
