#pragma once
#include "Toshi/Render/TModelRegistry.h"
#include "Toshi/Render/TTransformObject.h"

namespace Toshi {

	class T2ModelInstance
	{
	public:
		T2ModelInstance() = default;

		void Create(TModelPtr* a_pModelPtr);
		void Delete();

		void Update(TFLOAT a_fDeltaTime);
		void Render(TUINT a_uiClipFlags, const TVector3& a_rBounding);

		TModelInstance* GetInstance() { return m_pModelInstance; }
		TTransformObject& GetTransform() { return m_TransformObject; }

		void EnableSkeletonUpdate() { m_eFlags |= 1; }
		void DisableSkeletonUpdate() { m_eFlags &= ~1; }

	private:
		TModelPtr* m_pModelRef = TNULL;
		TModelInstance* m_pModelInstance = TNULL;
		TTransformObject m_TransformObject;
		// ...
		TUINT8 m_eFlags = 3;
	};

}
