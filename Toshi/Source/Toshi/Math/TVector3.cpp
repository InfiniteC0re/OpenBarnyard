#include "pch.h"
#include "TVector3.h"

void Toshi::TVector3::Set(const float* const a_pFloat)
{
	x = a_pFloat[0];
	y = a_pFloat[1];
	z = a_pFloat[2];
}

void Toshi::TVector3::Set(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Toshi::TVector3::Abs(const TVector3& a_Vec3)
{
	Set(TMath::Abs(a_Vec3.x), TMath::Abs(a_Vec3.y), TMath::Abs(a_Vec3.z));
}

void Toshi::TVector3::Abs()
{
	Set(TMath::Abs(x), TMath::Abs(y), TMath::Abs(z));
}

void Toshi::TVector3::Clip(float fVal, float fVal2)
{
	TMath::Clip(x, fVal, fVal2);
	TMath::Clip(y, fVal, fVal2);
	TMath::Clip(z, fVal, fVal2);
}

void Toshi::TVector3::CrossProduct(const TVector3& param_1, const TVector3& param_2)
{
	float fX = param_2.z * param_1.y - param_2.y * param_1.z;
	float fY = param_1.z * param_2.x - param_2.z * param_1.x;
	float fZ = param_1.x * param_2.y - param_1.y * param_2.x;
	Set(fX, fY, fZ);
}

float Toshi::TVector3::Distance(const TVector3& param_1, const TVector3& param_2)
{
	float fVar1 = param_2.x - param_1.x;
	float fVar2 = param_2.y - param_1.y;
	float fVar3 = param_2.z - param_1.z;

	return TMath::Sqrt(fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3);
}

float Toshi::TVector3::DistanceSq(const TVector3& param_1, const TVector3& param_2)
{
	float fVar1 = param_2.x - param_1.x;
	float fVar2 = param_2.y - param_1.y;
	float fVar3 = param_2.z - param_1.z;

	return fVar1 * fVar1 + fVar2 * fVar2 + fVar3 * fVar3;
}

void Toshi::TVector3::Divide(const TVector3& param_1, const TVector3& param_2)
{
	x = param_1.x / param_2.x;
	y = param_1.y / param_2.y;
	z = param_1.z / param_2.z;
}

void Toshi::TVector3::Divide(const TVector3& param_1)
{
	x /= param_1.x;
	y /= param_1.y;
	z /= param_1.z;
}

void Toshi::TVector3::Divide(float param_1)
{
	float ratio = 1.0f / param_1;
	x *= ratio;
	y *= ratio;
	z *= ratio;
}

void Toshi::TVector3::Divide(const TVector3& param_1, float param_2)
{
	float ratio = 1.0f / param_2;
	x = param_1.x * ratio;
	y = param_1.y * ratio;
	z = param_1.z * ratio;
}

void Toshi::TVector3::RotateX(float a_fRotation)
{
	float f1;
	float f2;
	float oldY = y;

	TMath::SinCos(a_fRotation, f1, f2);
	
	y = -f1 * z + f2 * oldY;
	z = oldY * f1 + z * f2;
}

void Toshi::TVector3::RotateY(float a_fRotation)
{
	float f1;
	float f2;
	float oldX = x;

	TMath::SinCos(a_fRotation, f1, f2);

	x = f1 * z + f2 * oldX;
	z = z * f2 + -f1 * oldX;
}

void Toshi::TVector3::RotateZ(float a_fRotation)
{
	float f1;
	float f2;
	float oldX = x;

	TMath::SinCos(a_fRotation, f1, f2);

	x = -f1 * y + f2 * oldX;
	y = f1 * oldX + y * f2;
}

Toshi::TVector3::TVector3(float x, float y, float z)
{
	Set(x, y, z);
}

Toshi::TVector3::TVector3(const float* const a_pFloat)
{
	Set(a_pFloat);
}

Toshi::TVector3::TVector3(const TVector3& a_Vec3)
{
	Set(a_Vec3.x, a_Vec3.y, a_Vec3.z);
}

