#pragma once
#include "TTransformObject.h"
#include "TRenderInterface.h"

TOSHI_NAMESPACE_START

class TCameraObject
{
public:
	inline static constexpr TFLOAT s_kMinFOV = ( 1.0f / 180.0f ) * TMath::PI;
	inline static constexpr TFLOAT s_kMaxFOV = TMath::PI;

public:
	TCameraObject();
	~TCameraObject();

	void Render();

	// $Barnyard: FUNCTION 006cd080
	TFLOAT SetNear( TFLOAT fNear )
	{
		return std::exchange( m_fNear, fNear );
	}

	// $Barnyard: FUNCTION 006cd090
	TFLOAT SetFar( TFLOAT fFar )
	{
		return std::exchange( m_fFar, fFar );
	}

	// $Barnyard: FUNCTION 006cd220
	TFLOAT SetFOV( TFLOAT fFOV )
	{
		TMath::Clip( fFOV, s_kMinFOV, s_kMaxFOV );
		return std::exchange( m_fFOV, fFOV );
	}

	// $Barnyard: FUNCTION 006cd270
	TFLOAT SetProjectionCentreX( TFLOAT fCentreX )
	{
		TMath::Clip( fCentreX, 0.0f, 1.0f );
		return std::exchange( m_fCentreX, fCentreX );
	}

	// $Barnyard: FUNCTION 006cd2d0
	TFLOAT SetProjectionCentreY( TFLOAT fCentreY )
	{
		TMath::Clip( fCentreY, 0.0f, 1.0f );
		return std::exchange( m_fCentreY, fCentreY );
	}

	TFLOAT GetNear() const
	{
		return m_fNear;
	}

	TFLOAT GetFar() const
	{
		return m_fFar;
	}

	TFLOAT GetFOV() const
	{
		return m_fFOV;
	}

	TRenderContext::CameraMode GetMode() const
	{
		return m_eMode;
	}

	void SetMode( TRenderContext::CameraMode a_eMode )
	{
		m_eMode = a_eMode;
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
	TFLOAT                     m_fNear;
	TFLOAT                     m_fFar;
	TFLOAT                     m_fFOV;
	TINT                       m_Unk1;
	TRenderContext::CameraMode m_eMode;
	TFLOAT                     m_fCentreX;
	TFLOAT                     m_fCentreY;
	TTransformObject           m_TransformObject;
	TBOOL                      m_bEnabled;
};

TOSHI_NAMESPACE_END
