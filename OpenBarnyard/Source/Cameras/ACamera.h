#pragma once
#include <Math/TMatrix44.h>

class ACamera :
	public Toshi::TObject
{
public:
	TDECLARE_CLASS( ACamera, Toshi::TObject );

	static TFLOAT				 sm_fCurrentFOV;
	static const Toshi::TVector4 sm_vWorldUp;
	static const Toshi::TVector4 sm_vInitialLookDirection;

	friend class ARenderer;

public:
	ACamera();
	~ACamera();

	void LookAtPoint( const Toshi::TVector4& a_rPoint );
	void LookAtDirection( const Toshi::TVector4& a_rDirection );

	Toshi::TMatrix44&		GetMatrix();
	const Toshi::TMatrix44& GetMatrix() const;

private:
	Toshi::TMatrix44 m_Matrix;
	TFLOAT			 m_fFOV;
	TFLOAT			 m_fProjectionCentreX;
	TFLOAT			 m_fProjectionCentreY;
};
