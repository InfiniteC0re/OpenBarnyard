#pragma once
#include <Toshi/T2SimpleArray.h>

#include <Render/TMesh.h>
#include <Render/TVertexPoolResourceInterface.h>
#include <Render/TIndexPoolResourceInterface.h>

class ASkinSubMesh
{
public:
	ASkinSubMesh();
	~ASkinSubMesh();

public:
	TUINT16 uiUnknown;
	TUINT16 uiNumVertices;
	Toshi::TIndexPoolResourceInterface* pIndexPool;
	TUINT32 Zero;
	TUINT32 Unk2;
	TUINT32 Unk3;
	TUINT32 Unk4;
	TUINT32 Unk5;
	TUINT32 Unk6;
	TUINT32 uiNumBones;
	TINT aBones[28];
};

class ASkinMesh :
	public Toshi::TMesh
{
public:
	TDECLARE_CLASS(Toshi::TMesh);

	using LockBuffer = Toshi::TVertexPoolResourceInterface::LockBuffer;

public:
	ASkinMesh();
	~ASkinMesh();

	//-----------------------------------------------------------------------------
	// Toshi::TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Validate() override;
	virtual void Invalidate() override;
	virtual TBOOL Render() override;
	virtual void OnDestroy() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void Create(TUINT32 a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiNumSubMeshes);
	virtual TBOOL Lock(LockBuffer& a_rLockBuffer);
	virtual void Unlock(TUINT32 a_uiNumVertices);
	virtual Toshi::TVertexPoolResourceInterface* GetVertexPool();

	TUINT16 GetNumSubMeshes() const;
	ASkinSubMesh* GetSubMesh(TUINT16 a_uiIndex);

private:
	TBOOL CreateResource();
	void DestroyResource();

protected:
	TUINT32 m_uiFlags;
	TUINT16 m_uiMaxVertices;
	TUINT16 m_uiNumSubMeshes;
	Toshi::TVertexPoolResourceInterface* m_pVertexPool;
	Toshi::T2SimpleArray<ASkinSubMesh> m_pSubMeshes;
	void* m_Unk;
};