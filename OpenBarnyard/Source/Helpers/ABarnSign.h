#pragma once
#include "Animation/AModel.h"

class ABarnSign
{
public:
	ABarnSign();
	~ABarnSign();

	void SpawnAtLocator( const TCHAR* a_pszLocatorName );

private:
	AModelInstance* m_pModelInstance;
	TINT            m_iUnk1;
	TINT            m_iUnk2;
	TFLOAT          m_fUnk3;
	TFLOAT          m_fUnk4;
};
