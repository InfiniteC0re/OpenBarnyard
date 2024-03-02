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

		TVector2& GetSomeVector1() { return m_SomeVector1; }
		TVector2& GetSomeVector2() { return m_SomeVector2; }

	private:
		TModelPtr* m_pModelRef = TNULL;
		TModelInstance* m_pModelInstance = TNULL;
		TTransformObject m_TransformObject;
		// ...
		TVector2 m_SomeVector1;
		TVector2 m_SomeVector2;
		// ...
		TUINT8 m_eFlags = 3;
	};

}
