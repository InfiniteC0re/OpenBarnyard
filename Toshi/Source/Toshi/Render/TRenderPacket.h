#pragma once
#include "Toshi/Math/TMatrix44.h"
#include "Toshi/Render/TMaterial.h"
#include "Toshi/Render/TMesh.h"
#include "Toshi/Render/TAnimation.h"

namespace Toshi {

	class TRenderPacket
	{
	public:
		TRenderPacket();

		TRenderPacket* GetNextPacket()
		{
			return m_pNextPacket;
		}
		
		void SetNextPacket(TRenderPacket* a_pNextPacket)
		{
			m_pNextPacket = a_pNextPacket;
		}

		TMesh* GetMesh() const
		{
			return m_pMesh;
		}

		void SetMesh(TMesh* a_pMesh)
		{
			m_pMesh = a_pMesh;
		}

		TMaterial* GetMaterial() const
		{
			return m_pMaterial;
		}

		void SetMaterial(TMaterial* pMaterial)
		{
			m_pMaterial = pMaterial;
		}

		TMatrix44& GetModelViewMatrix()
		{
			return m_oModelView;
		}

		void SetModelViewMatrix(TMatrix44& a_rMatrix44)
		{
			m_oModelView = a_rMatrix44;
		}

		TVector3& GetLightColour()
		{
			return m_oLightColour;
		}

		const TVector3& GetLightColour() const
		{
			return m_oLightColour;
		}

		TVector3& GetLightDirection()
		{
			return m_oLightDirection;
		}

		const TVector3& GetLightDirection() const
		{
			return m_oLightDirection;
		}

	private:
		TRenderPacket* m_pNextPacket;
		void* m_pUnk;
		TMesh* m_pMesh;
		TMatrix44 m_oModelView;
		TSkeletonInstance* m_pSkeletonInstance;
		TMaterial* m_pMaterial;
		TFLOAT m_fAmbientColorR;
		TFLOAT m_fAmbientColorG;
		TFLOAT m_fAmbientColorB;
		TVector3 m_oLightColour;
		TVector3 m_oLightDirection;
		TUINT8 m_ui8Unk1;
		TUINT8 m_ui8Unk2;
		TFLOAT m_fAlpha;
	};

}
