#pragma once
#include <Toshi/TObject.h>

//-----------------------------------------------------------------------------
// Unused TObject class registered next to AFunctionTable (only created by name).
// Its members are never read in the retail binary, the constructor zeroes them.
// TClass 0x0078bf90, vtable 0x0072cc80, size 0x48.
//-----------------------------------------------------------------------------
class AFunctionCollection
    : public Toshi::TObject
{
public:
	TDECLARE_CLASS( AFunctionCollection, Toshi::TObject );

public:
	AFunctionCollection();
	~AFunctionCollection();

private:
	void* m_apUnknown[ 17 ]; // 0x04
};
