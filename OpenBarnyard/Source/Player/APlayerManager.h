#pragma once
#include <Toshi/TObject.h>

class APlayerManager
    : public Toshi::TObject
    , public Toshi::TSingleton<APlayerManager>
{
public:
	APlayerManager();
	~APlayerManager();
};
