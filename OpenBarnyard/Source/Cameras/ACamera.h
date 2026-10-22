#pragma once
#include <Math/TMatrix44.h>

class MODAPI ACamera : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ACamera, Toshi::TObject );

	static TFLOAT                sm_fCurrentFOV;
	static const Toshi::TVector4 sm_vWorldUp;
	static const Toshi::TVector4 sm_vInitialLookDirection;

	friend class ARenderer;

public:
	ACamera();
	~ACamera();

	Toshi::TVector4 GetOrientation() const;

	// Outputs (0, 0, 1) rotated by the rotation of the matrix (w = 0)
	void GetDirection( Toshi::TVector4& a_rOutDirection ) const;

	// Outputs the forward axis of the matrix projected onto the ground plane (sm_vWorldUp) and normalised
	void GetFlatDirection( Toshi::TVector4& a_rOutDirection ) const;

	void LookAtPoint( const Toshi::TVector4& a_rPoint );
	void LookAtDirection( const Toshi::TVector4& a_rDirection );

	// Places the camera at a_rcPosition and makes it look at a_rcTarget
	void LookAt( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcTarget );

	// Returns sm_vInitialLookDirection rotated around Y axis by -a_fAngle
	static void CreateCameraDirection_RotateY( Toshi::TVector4& a_rOutDirection, TFLOAT a_fAngle );

	// Whether a_rcPoint is in the view cone of the camera (half the FOV) and closer than a_fMaxDistance along the view direction.
	TBOOL IsPointInView( const Toshi::TVector4& a_rcPoint, TFLOAT a_fMaxDistance ) const;

	TFLOAT GetFOV() const { return m_fFOV; }
	void   SetFOV( TFLOAT a_fFOV ) { m_fFOV = a_fFOV; }

	// $Barnyard: FUNCTION 0045b190
	void SetMatrix( const Toshi::TMatrix44& a_rcMatrix ) { m_Matrix = a_rcMatrix; }

	Toshi::TMatrix44&       GetMatrix();
	const Toshi::TMatrix44& GetMatrix() const;

	void GetMatrix( Toshi::TMatrix44& a_rOutMatrix ) const { a_rOutMatrix = m_Matrix; }

	// Rotation of the camera as a quaternion
	// $Barnyard: FUNCTION 0045b1b0
	void GetRotation( Toshi::TQuaternion& a_rOutRotation ) const { Toshi::TMatrix44::MatToQuat( a_rOutRotation, m_Matrix ); }

	// $Barnyard: FUNCTION 00420470
	Toshi::TVector4 GetPosition() const { return m_Matrix.GetTranslation(); }

	// $Barnyard: FUNCTION 0045b330
	void SetPosition( const Toshi::TVector4& a_rcPosition ) { m_Matrix.SetTranslation( a_rcPosition ); }

	// Returns the negated up axis of the matrix (w = 1)
	// $Barnyard: FUNCTION 0045b350
	Toshi::TVector4 GetUp() const
	{
		Toshi::TVector4 vUp = m_Matrix.AsBasisVector4( Toshi::BASISVECTOR_UP );
		vUp.w               = 1.0f;
		vUp.x               = -vUp.x;
		vUp.y               = -vUp.y;
		vUp.z               = -vUp.z;

		return vUp;
	}

private:
	Toshi::TMatrix44 m_Matrix;
	TFLOAT           m_fFOV;
	TFLOAT           m_fProjectionCentreX;
	TFLOAT           m_fProjectionCentreY;
};
