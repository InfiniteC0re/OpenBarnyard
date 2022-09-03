#include "pch.h"
#include "TQuaternion.h"

void Toshi::TQuaternion::Set(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

void Toshi::TQuaternion::SetIdentity()
{
	Set(TQuaternion::IDENTITY.x, TQuaternion::IDENTITY.y, TQuaternion::IDENTITY.z, TQuaternion::IDENTITY.w);
}

void Toshi::TQuaternion::SetFromEulerYX(const float* fVal)
{
	float f1;
	float f2;
	
	TMath::SinCos(fVal[0] * 0.5f, f1, f2);
	TQuaternion quat = TQuaternion(f1, 0, 0, f2);
	TMath::SinCos(fVal[1] * 0.5f, f1, f2);
	Set(0, f1, 0, f2);
	*this *= quat;
}

void Toshi::TQuaternion::SetFromEulerRollPitchYaw(float a_fRoll, float a_fPitch, float a_fYaw)
{
	float fCosRoll = cos(a_fRoll * 0.5f);
	float fSinRoll = sin(a_fRoll * 0.5f);
	float fCosPitch = cos(a_fPitch * 0.5f);
	float fSinPitch = sin(a_fPitch * 0.5f);
	float fCosYaw = cos(a_fYaw * 0.5f);
	float fSinYaw = sin(a_fYaw * 0.5f);

	float fX = fCosYaw * fCosPitch * fSinRoll - fSinYaw * fSinPitch * fCosRoll;
	float fY = fCosPitch * fSinRoll * fSinYaw + fSinPitch * fCosRoll * fCosYaw;
	float fZ = fCosRoll * fCosPitch * fSinYaw - fCosYaw * fSinPitch * fSinRoll;
	float fW = fSinYaw * fSinPitch * fSinRoll + fCosYaw * fCosPitch * fCosRoll;

	Set(fX, fY, fZ, fW);
}

void Toshi::TQuaternion::Negate()
{
	Set(-x, -y, -z, -w);
}

Toshi::TQuaternion& Toshi::TQuaternion::operator*=(const TQuaternion& a_Quat)
{
	float fX = ((x * a_Quat.w + a_Quat.z * y) - a_Quat.y * z) + a_Quat.x * w;
	float fY = a_Quat.x * z + (y * a_Quat.w - a_Quat.z * x) + a_Quat.y * w;
	float fZ = w * a_Quat.z + (a_Quat.y * x - a_Quat.x * y) + z * a_Quat.w;
	float fW = (-(a_Quat.y * y + a_Quat.x * x) - a_Quat.z * z) + a_Quat.w * w;
	Set(fX, fY, fZ, fW);
	return *this;
}

Toshi::TQuaternion::TQuaternion(const TQuaternion& a_pQuaternion)
{
	Set(a_pQuaternion.x, a_pQuaternion.y, a_pQuaternion.z, a_pQuaternion.w);
}

Toshi::TQuaternion::TQuaternion(float x, float y, float z, float w)
{
	Set(x, y, z, w);
}

Toshi::TQuaternion::TQuaternion()
{

}
