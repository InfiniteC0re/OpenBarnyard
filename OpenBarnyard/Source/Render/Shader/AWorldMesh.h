#pragma once
#include <Toshi/T2SimpleArray.h>

#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>

class AWorldMesh : public Toshi::TMesh
{
public:
	TDECLARE_CLASS(Toshi::TMesh);

	struct S1
	{
		TUINT32 m_uiUnk1 = 0;
		TUINT16 m_uiUnk2 = 0;
		TUINT16 m_uiUnk3 = 0;
		TUINT32 m_uiUnk4 = 0;
	};

	using LockBuffer = Toshi::TVertexPoolResourceInterface::LockBuffer;

public:
	AWorldMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() override;
	virtual void Invalidate() override;
	virtual void OnDestroy() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Create(TUINT32 a_uiUnknown, TUINT16 a_uiMaxVertices);
	virtual TBOOL Lock(LockBuffer& a_rLockBuffer);
	virtual void Unlock(TUINT32 a_uiNumVertices);
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool();

private:
	TBOOL CreateResource();

protected:
	TUINT32 m_uiFlags;
	TUINT16 m_uiMaxVertices;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;
	Toshi::T2SimpleArray<S1> m_pSomeArray;
};