#pragma once
#include "Render/TModelManager.h"
#include "Render/TTransformObject.h"

namespace Toshi {

	class T2ModelInstance
	{
	public:
		T2ModelInstance();

		void Create(TModelPtr* a_pModelPtr);
		void Delete();

		void Update(TFLOAT a_fDeltaTime);
		TBOOL Render(TUINT a_uiClipFlags, const TVector3& a_rBounding);
		TBOOL RenderIfVisible();

		TModelPtr* GetModelRef() { return m_pModelRef; }
		TModelInstance* GetInstance() { return m_pModelInstance; }
		TTransformObject& GetTransform() { return m_TransformObject; }

		void EnableSkeletonUpdate() { m_eFlags |= 1; }
		void DisableSkeletonUpdate() { m_eFlags &= ~1; }

		void EnableUnknown1() { m_eFlags |= 4; }
		void DisableUnknown1() { m_eFlags &= ~4; }

		TVector4& GetSomeVector() { return m_SomeVector; }

	private:
		TModelPtr* m_pModelRef = TNULL;
		TModelInstance* m_pModelInstance = TNULL;
		TTransformObject m_TransformObject;
		// ...
		TVector4 m_SomeVector;
		// ...
		TUINT8 m_eFlags = 3;
	};

}
