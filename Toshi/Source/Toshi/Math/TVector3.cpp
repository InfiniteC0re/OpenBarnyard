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
	Set(abs(a_Vec3.x), abs(a_Vec3.y), abs(a_Vec3.z));
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

