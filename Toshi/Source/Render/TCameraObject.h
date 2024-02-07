#pragma once
#include "TTransformObject.h"
#include "TRenderInterface.h"

namespace Toshi {

	class TCameraObject
	{
	public:
		static constexpr float s_kMinFOV = (1.0f / 180.0f) * TMath::PI;
		static constexpr float s_kMaxFOV = TMath::PI;

	public:
		TCameraObject();

		void Render();

		float SetNear(float fNear)
		{
			return std::exchange(m_fNear, fNear);
		}

		float SetFar(float fFar)
		{
			return std::exchange(m_fFar, fFar);
		}

		float SetFOV(float fFOV)
		{
			TMath::Clip(fFOV, s_kMinFOV, s_kMaxFOV);
			return std::exchange(m_fFOV, fFOV);
		}

		float SetProjectionCentreX(float fCentreX)
		{
			TMath::Clip(fCentreX, 0.0f, 1.0f);
			return std::exchange(m_fCentreX, fCentreX);
		}

		float SetProjectionCentreY(float fCentreY)
		{
			TMath::Clip(fCentreY, 0.0f, 1.0f);
			return std::exchange(m_fCentreY, fCentreY);
		}

		float GetNear() const
		{
			return m_fNear;
		}

		float GetFar() const
		{
			return m_fFar;
		}

		float GetFOV() const
		{
			return m_fFOV;
		}

		TRenderContext::CameraMode GetMode() const
		{
			return m_eMode;
		}

		TBOOL IsEnabled() const
		{
			return m_bEnabled;
		}

		TTransformObject& GetTransformObject()
		{
			return m_TransformObject;
		}

	private:
		float m_fNear;
		float m_fFar;
		float m_fFOV;
		int m_Unk1;
		TRenderContext::CameraMode m_eMode;
		float m_fCentreX;
		float m_fCentreY;
		TTransformObject m_TransformObject;
		TBOOL m_bEnabled;
	};

}