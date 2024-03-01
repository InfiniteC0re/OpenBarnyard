#include "ToshiPCH.h"
#include "T2ModelInstance.h"
#include "Render/TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi {

	T2ModelInstance::T2ModelInstance()
	{
		m_SomeVector.w = 1.0f;
	}

	void T2ModelInstance::Create(TModelPtr* a_pModelPtr)
	{
		m_eFlags |= 0b00001000;
		m_pModelRef = a_pModelPtr;
		m_pModelInstance = a_pModelPtr->GetModel()->CreateInstance();
		TTODO("Initialise some unknown values");
	}

	void T2ModelInstance::Delete()
	{
		if (this)
		{
			if (m_pModelInstance)
			{
				m_pModelInstance->Delete();
				m_pModelInstance = TNULL;
				m_eFlags &= ~0b00001000;
			}

			delete this;
		}
	}

	void T2ModelInstance::Update(TFLOAT a_fDeltaTime)
	{
		m_pModelInstance->Update(a_fDeltaTime);
	}

	TBOOL T2ModelInstance::Render(TUINT a_uiClipFlags, const TVector3& a_rBounding)
	{
		TIMPLEMENT();
		auto pContext = TRenderInterface::GetSingleton()->GetCurrentContext();
		
		m_pModelInstance->SetLOD(0);

		auto uiOldClipFlags = pContext->SetClipFlags(a_uiClipFlags);

		m_TransformObject.Push();
		m_pModelInstance->Render();
		m_TransformObject.Pop();
		
		pContext->SetClipFlags(uiOldClipFlags);

		return TTRUE;
	}

	TBOOL T2ModelInstance::RenderIfVisible()
	{
		auto pContext = TRenderInterface::GetSingleton()->GetCurrentContext();
		auto pModel = m_pModelInstance->GetModel();
		auto& lod = pModel->GetLOD(0);

		TVector4 transformScale = m_TransformObject.GetScale();
		TSphere bounding = lod.BoundingSphere;
		bounding.AsVector4().Multiply(TVector4(
			transformScale.x,
			transformScale.y,
			transformScale.z,
			TMath::Max(TMath::Max(transformScale.x, transformScale.y), transformScale.z)
		));

		TMatrix44 transformMatrix;
		m_TransformObject.GetLocalMatrixImp(transformMatrix);

		TMatrix44::TransformVector(bounding.GetOrigin(), transformMatrix, bounding.GetOrigin());
		
		if (bounding.GetRadius() > 0.0f)
		{
			auto pWorldPlanes = pContext->GetWorldPlanes();
			
			if (!pContext->CullSphereToFrustumSimple(bounding, pWorldPlanes, 6))
			{
				return TFALSE;
			}
		}

		return Render(pContext->GetClipFlags(), bounding.GetOrigin());
	}

}
