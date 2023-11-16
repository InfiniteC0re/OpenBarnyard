#pragma once
#include "AGUIPicture.h"
#include "Assets/AMaterialLibrary.h"

TOBJECT(AGUIMatlibPicture, AGUIPicture, TTRUE)
{
public:
	AGUIMatlibPicture();
	~AGUIMatlibPicture();

	virtual void Flush() override;
	virtual void Cache() override;
	virtual void Create(const Toshi::TPString8 & a_rName) override;

private:
	void LoadMatLib();
	void UnloadMatLib();

private:
	AMaterialLibrary* m_pMaterialLibrary;
};
