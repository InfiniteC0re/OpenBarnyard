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

		TModelInstance* GetInstance() { return m_pModelInstance; }

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
