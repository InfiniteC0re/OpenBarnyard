#pragma once
#include <Math/TVector4.h>

class AGameTimeFX
{
public:
	AGameTimeFX();
	~AGameTimeFX();

	void Apply();
	void Interpolate( const AGameTimeFX& a_rcFrom, const AGameTimeFX& a_rcTo, TFLOAT a_fProgress );

public:
	Toshi::TVector4 m_vecLightDir;
	Toshi::TVector4 field1_0x10;
	Toshi::TVector4 m_vecShadowColor;
	Toshi::TVector4 m_vecAmbientColor;
	Toshi::TVector4 m_vecFogColor;
	Toshi::TVector4 field5_0x50;
	Toshi::TVector4 field6_0x60;
	Toshi::TVector4 field7_0x70;
	Toshi::TVector4 field8_0x80;
	Toshi::TVector4 field9_0x90;
	Toshi::TVector4 field10_0xa0;
	Toshi::TVector4 field11_0xb0;
	Toshi::TVector4 field12_0xc0;
	Toshi::TVector4 field13_0xd0;
	Toshi::TVector4 field14_0xe0;
	Toshi::TVector4 field15_0xf0;
	Toshi::TVector4 field16_0x100;
	TFLOAT          field17_0x110;
	TFLOAT          field18_0x114;
	TFLOAT          field19_0x118;
	TFLOAT          m_fFogStart;
	TFLOAT          m_fFogEnd;
	TFLOAT          field22_0x124;
	TFLOAT          m_fShadowAmbientProgress;
	TFLOAT          field24_0x12c;
	TFLOAT          field25_0x130;
	TFLOAT          field26_0x134;
	TFLOAT          field27_0x138;
	TFLOAT          field28_0x13c;
	TFLOAT          m_fStarsOpacity;
	TFLOAT          field30_0x144;
	TFLOAT          field31_0x148;
	TFLOAT          field32_0x14c;
	TFLOAT          m_fDarkeningFactor;
	TFLOAT          m_fLightIntensity;
	TFLOAT          field35_0x158;
	TFLOAT          field36_0x15c;
	TFLOAT          field37_0x160;
	TFLOAT          field38_0x164;
	TFLOAT          field39_0x168;
	TFLOAT          field40_0x16c;
	TFLOAT          field41_0x170;
	TFLOAT          field42_0x174;
};
