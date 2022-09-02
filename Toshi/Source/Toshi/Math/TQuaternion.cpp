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
	Set(TQuaternion::IDENTITY->x, TQuaternion::IDENTITY->y, TQuaternion::IDENTITY->z, TQuaternion::IDENTITY->w);
}

void Toshi::TQuaternion::SetFromEulerYX(const float* fVal)
{
	float f1;
	float f2;
	
	TMath::SinCos(*fVal * 0.5f, f1, f2);
	TQuaternion quat = TQuaternion(f1, 0, 0, f2);
	TMath::SinCos(fVal[1] * 0.5f, f1, f2);
	Set(0, f1, 0, f2);
	//TODO operator*=(this, quat)
}

Toshi::TQuaternion::TQuaternion(const TQuaternion& a_pQuaternion)
{
	x = a_pQuaternion.x;
	y = a_pQuaternion.y;
	z = a_pQuaternion.z;
	w = a_pQuaternion.w;
}

Toshi::TQuaternion::TQuaternion(float x, float y, float z, float w)
{
	Set(x, y, z, w);
}

Toshi::TQuaternion::TQuaternion()
{

}
