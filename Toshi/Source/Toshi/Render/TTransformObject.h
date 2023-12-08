#pragma once
#include "Toshi/Math/TMatrix44.h"
#include "Toshi/Math/TQuaternion.h"
#include "Toshi/Math/TVector3.h"

namespace Toshi {

class TTransformObject
{
public:
	enum class Mode
	{
		Quat = 0, Euler, Matrix
	};

public:
	TTransformObject();

	void Push();
	void Pop();

	TVector3& GetTranslation();
	void GetLocalMatrixImp(TMatrix44& outMatrix);

	void SetEulerOrder(TUINT8 x, TUINT8 y, TUINT8 z);
	void SetTranslate(const TVector3& translation);
	void SetQuat(const TQuaternion& quaternion);
	void SetMatrix(const TMatrix44& matrix);

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

private:
	void RotateX(float angle)
	{
		TASSERT(Mode::Euler == GetMode());
		m_Matrix.RotateX(angle);
	}

	void RotateY(float angle)
	{
		TASSERT(Mode::Euler == GetMode());
		m_Matrix.RotateY(angle);
	}

	void RotateZ(float angle)
	{
		TASSERT(Mode::Euler == GetMode());
		m_Matrix.RotateZ(angle);
	}

private:
	union
	{
		TMatrix44 m_Matrix;
		TQuaternion m_Quat;
		TVector3 m_Euler;
	};

	Mode m_eMode;
	TVector3 m_Translation;
	TVector3 m_Scale;
	TUINT8 m_EulerOrder[3];
	TBOOL m_bChanged;
};

}