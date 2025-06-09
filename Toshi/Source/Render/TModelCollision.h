#pragma once
#include "TCollisionCommon.h"
#include "Toshi/TString8.h"
#include "Render/TTMDBase.h"
#include "Math/TVector3.h"
#include "ToshiTools/T2DynamicVector.h"

TOSHI_NAMESPACE_START

struct TModelCollisionType
{
	TModelCollisionType();
	~TModelCollisionType();

	TString8                                   strName;
	TUINT                                      uiUnk1 = 0;
	TUINT                                      uiUnk2 = 0;
	T2DynamicVector<TTMDBase::CollisionTypeS1> vecS1;
	TUINT                                      uiUnk3 = 0;
	TCollisionCommon::TOSHICGROUP              eCollGroup;
};

class TModelCollisionMesh
{
public:
	friend class TModel;

public:
	TModelCollisionMesh();
	~TModelCollisionMesh();

	TINT                 GetBoneID() const { return m_iBoneID; }
	TVector3*            GetVertices() const { return m_pVertices; }
	TUINT                GetNumVertices() const { return m_uiNumVertices; }
	TUINT16*             GetIndices() const { return m_pIndices; }
	TUINT                GetNumIndices() const { return m_uiNumIndices; }
	TINT                 GetNumCollTypes() const { return m_vecCollTypes.Size(); }
	TModelCollisionType& GetCollType( TINT a_iIndex ) { return m_vecCollTypes[ a_iIndex ]; }

protected:
	TINT                                 m_iBoneID;
	TVector3*                            m_pVertices;
	TUINT                                m_uiNumVertices;
	TUINT16*                             m_pIndices;
	TUINT                                m_uiNumIndices;
	T2DynamicVector<TModelCollisionType> m_vecCollTypes;
};

TOSHI_NAMESPACE_END
