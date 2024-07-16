#pragma once
#include "ACamera.h"

class ACameraHelper :
	public Toshi::TObject
{
public:
	TDECLARE_CLASS(ACameraHelper, Toshi::TObject);

public:
	ACameraHelper();
	~ACameraHelper();

	virtual void UpdateCurrentCamera();
	virtual void Unk1() = 0;
	virtual void Unk2() = 0;
	virtual void Unk3() = 0;
	virtual void Unk4() = 0;
	virtual void Unk5() = 0;
	virtual void Unk6() = 0;
	virtual void GetCameraPosition(Toshi::TVector4& a_rPosition) const;
	virtual void Unk7(void*);
	// ...
	virtual TFLOAT GetFOV() const;

private:
	// ...
	ACamera* m_pCamera;
};