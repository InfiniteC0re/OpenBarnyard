#pragma once
#include "AGUIPicture.h"
#include "Assets/AMaterialLibrary.h"

//-----------------------------------------------------------------------------
// Purpose: A picture whose images are the textures "<name>_N.tga" of the
// material library data/matlibs/<name>.ttl. TClass "AGUIMatLibPicture",
// vtable 0x007297e0, size 0x28.
//-----------------------------------------------------------------------------
class AGUIMatlibPicture : public AGUIPicture
{
public:
	TDECLARE_CLASS( AGUIMatlibPicture, AGUIPicture );

public:
	AGUIMatlibPicture();
	~AGUIMatlibPicture();

	virtual void Flush() override;
	virtual void Cache() override;
	virtual void Create( const Toshi::TPString8& a_rName ) override;

private:
	void LoadMatLib();
	void UnloadMatLib();

private:
	AMaterialLibrary* m_pMaterialLibrary; // 0x24
};
