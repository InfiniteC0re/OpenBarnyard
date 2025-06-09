#pragma once
#include "ACollisionModelSet.h"

#include <Toshi/TSceneObject.h>
#include <Render/TTransformObject.h>

class ACollisionModelInstance
{
public:
	ACollisionModelInstance();
	~ACollisionModelInstance();

	void Create( ACollisionModelSet* a_pModelSet, Toshi::TSceneObject* a_pSceneObject );

private:
	ACollisionModelSet*      m_pCollisionModelSet;
	Toshi::TSceneObject*     m_pSceneObject;
	TINT                     m_iUnk;
	Toshi::TTransformObject* m_pTransformObject;
};
