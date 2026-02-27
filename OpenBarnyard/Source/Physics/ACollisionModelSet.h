#pragma once
#include "ACollisionModel.h"

#include <Toshi/T2Vector.h>

class ACollisionModelSet
{
public:
	ACollisionModelSet();
	~ACollisionModelSet();

	TBOOL Create( Toshi::TModel* a_pModel );

	Toshi::TModel* GetModel() const { return m_pModel; }

private:
	Toshi::T2Vector<ACollisionModel*, 32> m_vecCollModels;
	Toshi::TModel*                        m_pModel;
	TINT                                  m_iNumMeshes;
	TINT                                  field5_0x90;

	friend class ACollisionModelInstance;
};
