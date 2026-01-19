#pragma once
#include <Toshi/TClass.h>
#include <Toshi/TSingleton.h>

class AMiscFunctions
    : public Toshi::TObject
    , public Toshi::TSingleton<AMiscFunctions>
	// TODO: AFunctionTable...
{
public:
	TDECLARE_CLASS( AMiscFunctions, Toshi::TObject );

public:
	AMiscFunctions();
	~AMiscFunctions();

};
