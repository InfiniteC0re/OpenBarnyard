#include "ToshiPCH.h"
#include "TQuaternion.h"
#include "TMatrix44.h"

TOSHI_NAMESPACE_START

const TQuaternion TQuaternion::IDENTITY = { 0.0f, 0.0f, 0.0f, 1.0f };

void TQuaternion::SetFromEulerYX( const TFLOAT* fVal )
{
	TFLOAT f1;
	TFLOAT f2;

	TMath::SinCos( fVal[ 0 ] * 0.5f, f1, f2 );
	TQuaternion quat = TQuaternion( f1, 0, 0, f2 );
	TMath::SinCos( fVal[ 1 ] * 0.5f, f1, f2 );
	Set( 0, f1, 0, f2 );
	*this *= quat;
}

void TQuaternion::SetFromEulerRollPitchYaw( TFLOAT a_fRoll, TFLOAT a_fPitch, TFLOAT a_fYaw )
{
	TFLOAT fCosRoll  = cos( a_fRoll * 0.5f );
	TFLOAT fSinRoll  = sin( a_fRoll * 0.5f );
	TFLOAT fCosPitch = cos( a_fPitch * 0.5f );
	TFLOAT fSinPitch = sin( a_fPitch * 0.5f );
	TFLOAT fCosYaw   = cos( a_fYaw * 0.5f );
	TFLOAT fSinYaw   = sin( a_fYaw * 0.5f );

	TFLOAT fX = fCosYaw * fCosPitch * fSinRoll - fSinYaw * fSinPitch * fCosRoll;
	TFLOAT fY = fCosPitch * fSinRoll * fSinYaw + fSinPitch * fCosRoll * fCosYaw;
	TFLOAT fZ = fCosRoll * fCosPitch * fSinYaw - fCosYaw * fSinPitch * fSinRoll;
	TFLOAT fW = fSinYaw * fSinPitch * fSinRoll + fCosYaw * fCosPitch * fCosRoll;

	Set( fX, fY, fZ, fW );
}

// $Barnyard: FUNCTION 006c93b0
void TQuaternion::SetRotation( const TVector3& a_rVec3, TFLOAT a_fVal )
{
	TFLOAT fVal;
	TFLOAT fVal2;

	TMath::SinCos( a_fVal * 0.5f, fVal, fVal2 );
	Set( a_rVec3.x * fVal, a_rVec3.y * fVal, a_rVec3.z * fVal, fVal2 );
}

// $Barnyard: FUNCTION 006c9840
void TQuaternion::SetVectorDirection( const TVector3& a_rVec3, const TVector3& a_rVec3_2 )
{
	TVector3 vec3;
	vec3.CrossProduct( a_rVec3_2, a_rVec3 );

	TFLOAT mag = vec3.Magnitude();

	if ( mag > TMath::TFLOAT_EPSILON )
	{
		vec3.Divide( mag );
		TFLOAT dotProduct = TVector3::DotProduct( a_rVec3_2, a_rVec3 );
		TFLOAT rot        = 0.0f;
		if ( ( dotProduct <= 1.0f - TMath::TFLOAT_EPSILON ) && ( rot = TMath::PI, -1.0f + TMath::TFLOAT_EPSILON <= dotProduct ) )
		{
			rot = TMath::ACos( dotProduct );
		}
		SetRotation( vec3, rot );
		return;
	}

	TFLOAT dotProduct = TVector3::DotProduct( a_rVec3_2, a_rVec3 );

	if ( dotProduct < 0.0f )
	{
		TVector3 vec = a_rVec3_2;
		if ( TMath::Abs( a_rVec3_2.y ) < 0.95f )
		{
			vec.RotateY( TMath::PI * 0.0055555557f * 90.0f );
		}
		else
		{
			vec.RotateX( TMath::PI * 0.0055555557f * 90.0f );
		}
		SetIdentity();
		RotateAroundAxis( vec, TMath::PI * 0.0055555557f * 180.0f );
	}
}

void TQuaternion::RotateVector( TVector3& param_1, const TQuaternion& param_2, const TVector3 param_3 )
{
	TQuaternion quat = param_2;
	quat.Negate();
	TQuaternion quat2 = TQuaternion( param_3.x, param_3.y, param_3.z, 0.0f );
	TQuaternion quat3 = param_2;
	quat3 *= quat2;
	quat3 *= quat;

	param_1.Set( quat3.x, quat3.y, quat3.z );
}

// $Barnyard: FUNCTION 006c9340
void TQuaternion::RotateAroundAxis( const TVector3& param_1, TFLOAT param_2 )
{
	TFLOAT fVal;
	TFLOAT fVal2;

	TMath::SinCos( param_2 * 0.5f, fVal, fVal2 );
	*this *= TQuaternion( param_1.x * fVal, param_1.y * fVal, param_1.z * fVal, fVal2 );
}

void TQuaternion::GetEulerXYZ( TVector3& outVec ) const
{
	TMatrix44 matrix;
	matrix.SetFromQuaternion( *this );

	matrix.GetEulerXYZ( outVec );
}

// $Barnyard: FUNCTION 006c9190
TQuaternion& TQuaternion::operator*=( const TQuaternion& a_Quat )
{
	TFLOAT fX = ( ( x * a_Quat.w + a_Quat.z * y ) - a_Quat.y * z ) + a_Quat.x * w;
	TFLOAT fY = a_Quat.x * z + ( y * a_Quat.w - a_Quat.z * x ) + a_Quat.y * w;
	TFLOAT fZ = w * a_Quat.z + ( a_Quat.y * x - a_Quat.x * y ) + z * a_Quat.w;
	TFLOAT fW = ( -( a_Quat.y * y + a_Quat.x * x ) - a_Quat.z * z ) + a_Quat.w * w;
	Set( fX, fY, fZ, fW );
	return *this;
}

// $Barnyard: FUNCTION 006c9ae0
void TQuaternion::Nlerp( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress )
{
	TFLOAT fDot = TVector4::DotProduct4( a_rStart.AsVector4(), a_rEnd.AsVector4() );

	if ( 0.0f <= fDot )
	{
		TVector4 vec = a_rEnd.AsVector4();

		vec.Substract4( a_rStart.AsVector4() );
		vec.Multiply4( vec, a_fProgress );

		a_rOut.x = vec.x + a_rStart.x;
		a_rOut.y = vec.y + a_rStart.y;
		a_rOut.z = vec.z + a_rStart.z;
		a_rOut.w = vec.w + a_rStart.w;

		a_rOut.AsVector4().Normalise4();
	}
	else
	{
		TVector4 vec = a_rEnd.AsVector4();
		vec.Add4( a_rStart.AsVector4() );
		vec.Multiply4( vec, a_fProgress );

		a_rOut.x = vec.x - a_rStart.x;
		a_rOut.y = vec.y - a_rStart.y;
		a_rOut.z = vec.z - a_rStart.z;
		a_rOut.w = vec.w - a_rStart.w;
	
		a_rOut.AsVector4().Normalise4();
	}
}

// $Barnyard: FUNCTION 006c9400
void TQuaternion::Slerp( TQuaternion& a_rOut, const TQuaternion& a_rStart, const TQuaternion& a_rEnd, TFLOAT a_fProgress )
{
	if ( a_fProgress == 0.0f )
	{
		a_rOut = a_rStart;
		return;
	}

	if ( a_fProgress == 1.0f )
	{
		a_rOut = a_rEnd;
		return;
	}

	TFLOAT fDot         = TVector4::DotProduct4( a_rStart.AsVector4(), a_rEnd.AsVector4() );
	TBOOL  bNegativeDot = fDot < 0.0f;

	if ( bNegativeDot )
		fDot *= -1;

	TFLOAT fProg1 = a_fProgress;
	TFLOAT fProg2;

	if ( 1.0f - fDot <= 0.001f )
	{
		fProg2 = 1.0f - a_fProgress;
	}
	else
	{
		TFLOAT fAcos = TMath::ACos( fDot );
		TFLOAT fSin  = TMath::Sin( fAcos );

		fProg1 = TMath::Sin( fAcos * a_fProgress ) * ( 1.0f / fSin );
		fProg2 = TMath::Sin( ( 1.0f - a_fProgress ) * fAcos ) * ( 1.0f / fSin );
	}

	if ( bNegativeDot )
	{
		a_rOut.x = a_rStart.x * fProg2 - a_rEnd.x * fProg1;
		a_rOut.y = a_rStart.y * fProg2 - a_rEnd.y * fProg1;
		a_rOut.z = a_rStart.z * fProg2 - a_rEnd.z * fProg1;
		a_rOut.w = a_rStart.w * fProg2 - a_rEnd.w * fProg1;
	}
	else
	{
		a_rOut.x = a_rStart.x * fProg2 + a_rEnd.x * fProg1;
		a_rOut.y = a_rStart.y * fProg2 + a_rEnd.y * fProg1;
		a_rOut.z = a_rStart.z * fProg2 + a_rEnd.z * fProg1;
		a_rOut.w = a_rStart.w * fProg2 + a_rEnd.w * fProg1;
	}
}

TOSHI_NAMESPACE_END
