#pragma once
#include "TTransformObject.h"
#include "TRenderInterface.h"

TOSHI_NAMESPACE_START

class TCameraObject
{
public:
	static constexpr TFLOAT s_kMinFOV = ( 1.0f / 180.0f ) * TMath::PI;
	static constexpr TFLOAT s_kMaxFOV = TMath::PI;

public:
	TCameraObject();

	void Render();

	TFLOAT SetNear( TFLOAT fNear )
	{
		return std::exchange( m_fNear, fNear );
	}

	TFLOAT SetFar( TFLOAT fFar )
	{
		return std::exchange( m_fFar, fFar );
	}

	TFLOAT SetFOV( TFLOAT fFOV )
	{
		TMath::Clip( fFOV, s_kMinFOV, s_kMaxFOV );
		return std::exchange( m_fFOV, fFOV );
	}

	TFLOAT SetProjectionCentreX( TFLOAT fCentreX )
	{
		TMath::Clip( fCentreX, 0.0f, 1.0f );
		return std::exchange( m_fCentreX, fCentreX );
	}

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

	TBOOL IsEnabled() const
	{
		return m_bEnabled;
	}

	TTransformObject& GetTransformObject()
	{
		return m_TransformObject;
	}

private:
	TFLOAT                      m_fNear;
	TFLOAT                      m_fFar;
	TFLOAT                      m_fFOV;
	TINT                       m_Unk1;
	TRenderContext::CameraMode m_eMode;
	TFLOAT                      m_fCentreX;
	TFLOAT                      m_fCentreY;
	TTransformObject           m_TransformObject;
	TBOOL                      m_bEnabled;
};

TOSHI_NAMESPACE_END
