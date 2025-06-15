#pragma once
#include "TCollisionCommon.h"
#include "Toshi/TString8.h"
#include "Render/TTMDBase.h"
#include "Math/TVector3.h"
#include "ToshiTools/T2DynamicVector.h"

TOSHI_NAMESPACE_START

struct TModelCollisionGroup
{
	TModelCollisionGroup();
	~TModelCollisionGroup();

	TCollisionCommon::TOSHICGROUPFLAG GetGroupFlag() const { return TCollisionCommon::TOSHICGROUPFLAG( 1U << ( eCollGroup % 32U ) ); }

	TString8                                   strName;
	TUINT                                      uiUnk1     = 0;
	TUINT                                      uiNumFaces = 0;
	T2DynamicVector<TTMDBase::CollisionTypeS1> vecS1;
	TUINT                                      uiUnk3 = 0;
	TCollisionCommon::TOSHICGROUP              eCollGroup;
};

class TModelCollisionData
{
public:
	friend class TModel;

public:
	TModelCollisionData();
	~TModelCollisionData();

	void GetTriangleVertices( TUINT a_uiFaceId, Toshi::TVector3* a_rVertices[ 3 ] );
	TINT GetGroupForTriangleImp( TUINT a_uiFaceId );

	TINT                 GetBoneID() const { return m_iBoneID; }
	TVector3*            GetVertices() const { return m_pVertices; }
	TUINT                GetNumVertices() const { return m_uiNumVertices; }
	TUINT16*             GetIndices() const { return m_pIndices; }
	TUINT                GetNumIndices() const { return m_uiNumIndices; }
	TINT                 GetNumCollGroups() const { return m_vecCollGroups.Size(); }
	TModelCollisionGroup& GetCollGroup( TINT a_iIndex ) { return m_vecCollGroups[ a_iIndex ]; }

protected:
	TINT                                 m_iBoneID;
	TVector3*                            m_pVertices;
	TUINT                                m_uiNumVertices;
	TUINT16*                             m_pIndices;
	TUINT                                m_uiNumIndices;
	T2DynamicVector<TModelCollisionGroup> m_vecCollGroups;
};

TOSHI_NAMESPACE_END
