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
	m_EulerOrder[ 0 ] = EULERAXIS_Z;
	m_EulerOrder[ 1 ] = EULERAXIS_Y;
	m_EulerOrder[ 2 ] = EULERAXIS_X;
}

TTransformObject::~TTransformObject()
{
}

// $Barnyard: FUNCTION 006c8180
TTransformObject& TTransformObject::operator=( const TTransformObject& a_rcOther )
{
	if ( &a_rcOther != this )
		TUtil::MemCopy( this, &a_rcOther, sizeof( TTransformObject ) );

	return *this;
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
				case EULERAXIS_X:
					pPushTransform->RotateX( m_Euler.x );
					break;
				case EULERAXIS_Y:
					pPushTransform->RotateY( m_Euler.y );
					break;
				case EULERAXIS_Z:
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
				case EULERAXIS_X:
					outMatrix.RotateX( m_Euler.x );
					break;
				case EULERAXIS_Y:
					outMatrix.RotateY( m_Euler.y );
					break;
				case EULERAXIS_Z:
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

void TTransformObject::SetEulerOrder( EULERAXIS x, EULERAXIS y, EULERAXIS z )
{
	m_EulerOrder[ 0 ] = x;
	m_EulerOrder[ 1 ] = y;
	m_EulerOrder[ 2 ] = z;
}

// $Barnyard: FUNCTION 006c8210
void TTransformObject::SetTranslate( const TVector3& a_rTranslation )
{
	m_Translation = a_rTranslation;

	if ( m_eMode == Mode::Matrix )
		m_Matrix.SetTranslation( a_rTranslation );
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

// $Barnyard: FUNCTION 006c81a0
TTransformObject::Mode TTransformObject::SetMode( Mode a_eMode )
{
	Mode eOldMode = m_eMode;
	m_eMode       = a_eMode;

	return eOldMode;
}

void TTransformObject::SetScale( TFLOAT a_flScaleX, TFLOAT a_flScaleY, TFLOAT a_flScaleZ )
{
	m_Scale.x = a_flScaleX;
	m_Scale.y = a_flScaleY;
	m_Scale.z = a_flScaleZ;
}

void TTransformObject::SetScale( const TVector3& a_rScale )
{
	m_Scale = a_rScale;
}

TOSHI_NAMESPACE_END
