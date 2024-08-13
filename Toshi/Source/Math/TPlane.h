#pragma once
#include "TVector4.h"

TOSHI_NAMESPACE_START

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
	constexpr TPlane() = default;

	constexpr TPlane( const TVector4& a_PlaneData ) :
		m_Normal( a_PlaneData.AsVector3() ),
		m_fDistance( a_PlaneData.w )
	{}

	constexpr TPlane( const TVector3& a_Normal, TFLOAT a_fDistance ) :
		m_Normal( a_Normal ),
		m_fDistance( a_fDistance )
	{}

	constexpr TPlane( const TVector3& a_Normal, const TVector3& a_Point ) :
		m_Normal( a_Normal ),
		m_fDistance( TVector3::DotProduct( a_Normal, a_Point ) )
	{}

	constexpr TPlane( TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ, TFLOAT a_fDistance ) :
		m_Normal( a_fNormalX, a_fNormalY, a_fNormalZ ),
		m_fDistance( a_fDistance )
	{}

	constexpr TPlane( const TPlane& a_Other ) :
		m_Normal( a_Other.m_Normal ),
		m_fDistance( a_Other.m_fDistance )
	{}

	constexpr void Set( const TVector4& a_PlaneData )
	{
		m_Normal	= a_PlaneData.AsVector3();
		m_fDistance = a_PlaneData.w;
	}

	constexpr void Set( const TVector3& a_Normal, TFLOAT a_fDistance )
	{
		m_Normal	= a_Normal;
		m_fDistance = a_fDistance;
	}

	constexpr void Set( const TVector3& a_Normal, const TVector3& a_Point )
	{
		m_Normal	= a_Normal;
		m_fDistance = TVector3::DotProduct( a_Normal, a_Point );
	}

	constexpr void Set( TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ, TFLOAT a_fDistance )
	{
		m_Normal.Set( a_fNormalX, a_fNormalY, a_fNormalZ );
		m_fDistance = a_fDistance;
	}

	constexpr void Set( const TPlane& a_Other )
	{
		m_Normal	= a_Other.m_Normal;
		m_fDistance = a_Other.m_fDistance;
	}

	TFORCEINLINE constexpr TFLOAT GetD() const
	{
		return m_fDistance;
	}

	TFORCEINLINE constexpr void SetD( TFLOAT a_fDistance )
	{
		m_fDistance = a_fDistance;
	}

	TFORCEINLINE constexpr const TVector3& GetNormal() const
	{
		return m_Normal;
	}

	TFORCEINLINE constexpr void SetNormal( const TVector3& a_rNormal )
	{
		m_Normal = a_rNormal;
	}

	TFORCEINLINE constexpr void SetNormal( TFLOAT a_fNormalX, TFLOAT a_fNormalY, TFLOAT a_fNormalZ )
	{
		m_Normal.Set( a_fNormalX, a_fNormalY, a_fNormalZ );
	}

	TFORCEINLINE void GetPoint( TVector3& a_rPoint )
	{
		a_rPoint.Multiply( m_Normal, m_fDistance );
	}

	TFORCEINLINE void SetPoint( const TVector3& a_rPoint )
	{
		m_fDistance = TVector3::DotProduct( m_Normal, a_rPoint );
	}

	TFORCEINLINE void Negate( const TPlane& a_rPlane )
	{
		AsVector4().Negate4( a_rPlane.AsVector4() );
	}

	TFORCEINLINE void Negate()
	{
		AsVector4().Negate4();
	}

	constexpr TPlane& operator=( const TPlane& a_rPlane )
	{
		m_Normal	= a_rPlane.m_Normal;
		m_fDistance = a_rPlane.m_fDistance;
		return *this;
	}

	TFORCEINLINE constexpr TVector3&	   AsNormal() { return m_Normal; }
	TFORCEINLINE constexpr const TVector3& AsNormal() const { return m_Normal; }

	TFORCEINLINE TVector4&		 AsVector4() { return *TREINTERPRETCAST( TVector4*, this ); }
	TFORCEINLINE const TVector4& AsVector4() const { return *TREINTERPRETCAST( const TVector4*, this ); }

private:
	TVector3 m_Normal;
	TFLOAT	 m_fDistance;
};

TOSHI_NAMESPACE_END
