#include "ToshiPCH.h"
#include "TTransformObject.h"
#include "TRenderInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006c8340
TTransformObject::TTransformObject()
    : m_eMode( Mode::Quat ), m_Quat( TQuaternion::IDENTITY ), m_Translation( TVector3::VEC_ZERO ), m_Scale( 1.0f, 1.0f, 1.0f )
{
	m_EulerOrder[ 0 ] = 2;
	m_EulerOrder[ 1 ] = 1;
	m_EulerOrder[ 2 ] = 0;
}

TTransformObject::~TTransformObject()
{
}

// $Barnyard: FUNCTION 006c83b0
void TTransformObject::Push()
{
	auto pRender = TRenderInterface::GetSingleton();

	auto pPrevTransform = &pRender->GetTransforms().Top();

	pRender->GetTransforms().PushNull();
	auto pPushTransform = &pRender->GetTransforms().Top();

	if ( m_eMode == Mode::Quat )
	{
		pPushTransform->PushQuaternion( m_Quat, *pPrevTransform, m_Translation );
		pPushTransform->Scale( m_Scale.x, m_Scale.y, m_Scale.z );
	}
	else if ( m_eMode == Mode::Euler )
	{
		*pPushTransform = *pPrevTransform;
		TMatrix44::TransformVector( pPushTransform->GetTranslation().AsVector3(), *pPushTransform, GetTranslation() );

		for ( TINT i = 0; i < 3; i++ )
		{
			switch ( m_EulerOrder[ i ] )
			{
				case 0:
					pPushTransform->RotateX( m_Euler.x );
					break;
				case 1:
					pPushTransform->RotateY( m_Euler.y );
					break;
				case 2:
					pPushTransform->RotateZ( m_Euler.z );
					break;
			}
		}

		pPushTransform->Scale( m_Scale.x, m_Scale.y, m_Scale.z );
	}
	else
	{
		TASSERT( m_eMode == Mode::Matrix );
		pPushTransform->Multiply( *pPrevTransform, m_Matrix );
	}

	pRender->GetCurrentContext()->SetModelViewMatrix( pRender->GetTransforms().Top() );
}

// $Barnyard: FUNCTION 006c84c0
void TTransformObject::Pop()
{
	auto pRender = TRenderInterface::GetSingleton();

	pRender->GetTransforms().Pop();
	pRender->GetCurrentContext()->SetModelViewMatrix( pRender->GetTransforms().Top() );
}

Toshi::TVector3& TTransformObject::GetTranslation()
{
	return ( m_eMode != Mode::Matrix ) ? m_Translation : m_Matrix.GetTranslation().AsVector3();
}

// $Barnyard: FUNCTION 006c8250
void TTransformObject::GetLocalMatrixImp( TMatrix44& outMatrix )
{
	if ( m_eMode == Mode::Quat )
	{
		outMatrix.SetFromQuaternion( m_Quat );
		outMatrix.SetTranslation( m_Translation );
		outMatrix.Scale( m_Scale.x, m_Scale.y, m_Scale.z );
	}
	else if ( m_eMode == Mode::Euler )
	{
		outMatrix.Identity();
		outMatrix.SetTranslation( GetTranslation() );

		for ( TINT i = 0; i < 3; i++ )
		{
			switch ( m_EulerOrder[ i ] )
			{
				case 0:
					outMatrix.RotateX( m_Euler.x );
					break;
				case 1:
					outMatrix.RotateY( m_Euler.y );
					break;
				case 2:
					outMatrix.RotateZ( m_Euler.z );
					break;
			}
		}

		outMatrix.Scale( m_Scale.x, m_Scale.y, m_Scale.z );
	}
	else
	{
		TASSERT( m_eMode == Mode::Matrix );
		outMatrix = m_Matrix;
	}
}

// $Barnyard: FUNCTION 006c81b0
void TTransformObject::SetEuler( const TVector3& a_rEuler )
{
	m_eMode = Mode::Euler;
	m_Euler = a_rEuler;
}

void TTransformObject::SetEulerOrder( TUINT8 x, TUINT8 y, TUINT8 z )
{
	m_EulerOrder[ 0 ] = x;
	m_EulerOrder[ 1 ] = y;
	m_EulerOrder[ 2 ] = z;
}

// $Barnyard: FUNCTION 006c8210
void TTransformObject::SetTranslate( const TVector3& translation )
{
	m_Translation = translation;

	if ( m_eMode == Mode::Matrix )
		m_Matrix.SetTranslation( m_Translation );
}

void TTransformObject::SetQuat( const TQuaternion& quaternion )
{
	m_eMode = Mode::Quat;
	m_Quat  = quaternion;
}

// $Barnyard: FUNCTION 006c81d0
void TTransformObject::SetMatrix( const TMatrix44& matrix )
{
	m_eMode       = Mode::Matrix;
	m_Matrix      = matrix;
	m_Translation = matrix.GetTranslation3();
}

TOSHI_NAMESPACE_END
