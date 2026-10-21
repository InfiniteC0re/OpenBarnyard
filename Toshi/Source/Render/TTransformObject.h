#pragma once
#include "Math/TMatrix44.h"
#include "Math/TQuaternion.h"
#include "Math/TVector3.h"

TOSHI_NAMESPACE_START

class MODAPI TTransformObject
{
public:
	enum class Mode
	{
		Quat = 0,
		Euler,
		Matrix
	};

	using EULERAXIS = TUINT8;
	enum EULERAXIS_ : EULERAXIS
	{
		EULERAXIS_X = 0,
		EULERAXIS_Y = 1,
		EULERAXIS_Z = 2,
	};

public:
	TTransformObject();
	~TTransformObject();

	TTransformObject& operator=( const TTransformObject& a_rcOther );

	void Push();
	void Pop();

	TVector3& GetTranslation();
	void      GetLocalMatrixImp( TMatrix44& a_rOutMatrix );

	void SetEuler( const TVector3& a_rEuler );
	void SetEulerOrder( EULERAXIS a_uiX, EULERAXIS a_uiy, EULERAXIS a_uiz );
	void SetTranslate( const TVector3& a_rTranslation );
	void SetQuat( const TQuaternion& a_rQuaternion );
	void SetMatrix( const TMatrix44& a_rMatrix );
	Mode SetMode( Mode a_eMode );
	void SetScale( TFLOAT a_flScaleX, TFLOAT a_flScaleY, TFLOAT a_flScaleZ );
	void SetScale( const TVector3& a_rScale );

	// NOTE: only valid in Mode::Matrix
	const TMatrix44& GetMatrix() const { return m_Matrix; }

	void ResetChangedState()
	{
		m_bChanged = TFALSE;
	}

	TBOOL HasChanged() const
	{
		return m_bChanged;
	}

	Mode GetMode() const
	{
		return m_eMode;
	}

	const TVector3& GetScale() const
	{
		return m_Scale;
	}

private:
	void RotateX( TFLOAT a_fAngle )
	{
		TASSERT( Mode::Euler == GetMode() );
		m_Matrix.RotateX( a_fAngle );
	}

	void RotateY( TFLOAT a_fAngle )
	{
		TASSERT( Mode::Euler == GetMode() );
		m_Matrix.RotateY( a_fAngle );
	}

	void RotateZ( TFLOAT a_fAngle )
	{
		TASSERT( Mode::Euler == GetMode() );
		m_Matrix.RotateZ( a_fAngle );
	}

private:
	union
	{
		TMatrix44   m_Matrix;
		TQuaternion m_Quat;
		TVector3    m_Euler;
	};

	Mode      m_eMode;
	TVector3  m_Translation;
	TVector3  m_Scale;
	EULERAXIS m_EulerOrder[ 3 ];
	TBOOL     m_bChanged;
};

TOSHI_NAMESPACE_END
