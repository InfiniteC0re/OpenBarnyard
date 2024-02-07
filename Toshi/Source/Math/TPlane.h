#pragma once
#include "TVector4.h"

namespace Toshi {

	class TPlane
	{
	public:
		enum PlaneComparison
		{
			PlaneComparison_InFront = -1,
			PlaneComparison_Intersects,
			PlaneComparison_InBack,
		};

	public:
		TPlane() = default;

		TPlane(const TVector4& a_PlaneData)
		{
			m_Normal = a_PlaneData.AsVector3();
			m_fDistance = a_PlaneData.w;
		}

		TPlane(const TVector3& a_Normal, TFLOAT a_fDistance)
		{
			m_Normal = a_Normal;
			m_fDistance = a_fDistance;
		}

		TPlane(const TVector3& a_Normal, const TVector3& a_Point)
		{
			m_Normal = a_Normal;
			m_fDistance = TVector3::DotProduct(a_Normal, a_Point);
		}

		TPlane(TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ, TFLOAT a_fDistance)
		{
			m_Normal.Set(a_fNormalX, a_fNormalY, a_fNormalZ);
			m_fDistance = a_fDistance;
		}

		TPlane(const TPlane& a_Other)
		{
			m_Normal = a_Other.m_Normal;
			m_fDistance = a_Other.m_fDistance;
		}

		void Set(const TVector4& a_PlaneData)
		{
			m_Normal = a_PlaneData.AsVector3();
			m_fDistance = a_PlaneData.w;
		}

		void Set(const TVector3& a_Normal, TFLOAT a_fDistance)
		{
			m_Normal = a_Normal;
			m_fDistance = a_fDistance;
		}

		void Set(const TVector3& a_Normal, const TVector3& a_Point)
		{
			m_Normal = a_Normal;
			m_fDistance = TVector3::DotProduct(a_Normal, a_Point);
		}

		void Set(TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ, TFLOAT a_fDistance)
		{
			m_Normal.Set(a_fNormalX, a_fNormalY, a_fNormalZ);
			m_fDistance = a_fDistance;
		}

		void Set(const TPlane& a_Other)
		{
			m_Normal = a_Other.m_Normal;
			m_fDistance = a_Other.m_fDistance;
		}

		__forceinline TFLOAT GetD() const
		{
			return m_fDistance;
		}

		void SetD(TFLOAT a_fDistance)
		{
			m_fDistance = a_fDistance;
		}

		__forceinline const TVector3& GetNormal() const
		{
			return m_Normal;
		}

		__forceinline void SetNormal(const TVector3& a_rNormal)
		{
			m_Normal = a_rNormal;
		}

		__forceinline void SetNormal(TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ)
		{
			m_Normal.Set(a_fNormalX, a_fNormalY, a_fNormalZ);
		}

		__forceinline void GetPoint(TVector3& a_rPoint)
		{
			a_rPoint.Multiply(m_Normal, m_fDistance);
		}

		__forceinline void SetPoint(const TVector3& a_rPoint)
		{
			m_fDistance = TVector3::DotProduct(m_Normal, a_rPoint);
		}

		__forceinline void Negate(const TPlane& a_rPlane)
		{
			AsVector4().Negate4(a_rPlane.AsVector4());
		}

		__forceinline void Negate()
		{
			AsVector4().Negate4();
		}

		TPlane& operator=(const TPlane& a_rPlane)
		{
			m_Normal = a_rPlane.m_Normal;
			m_fDistance = a_rPlane.m_fDistance;
			return *this;
		}

		__forceinline TVector3& AsNormal() { return m_Normal; }
		__forceinline const TVector3& AsNormal() const { return m_Normal; }
		
		__forceinline TVector4& AsVector4() { return *TREINTERPRETCAST(TVector4*, this); }
		__forceinline const TVector4& AsVector4() const { return *TREINTERPRETCAST(const TVector4*, this); }

	private:
		TVector3 m_Normal;
		TFLOAT m_fDistance;
	};

}