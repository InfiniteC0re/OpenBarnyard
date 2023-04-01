#pragma once
class AOptions : Toshi::TObject, public Toshi::TSingleton<AOptions>
{
public:
	AOptions()
	{

	}

	enum OPTIONS
	{
		OPTIONS_MAXSIZE = 28
	};
};

