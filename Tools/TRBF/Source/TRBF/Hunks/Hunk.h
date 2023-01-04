#pragma once
#include <Toshi/File/TTSF.h>

class Hunk
{
public:
	void Write(Toshi::TTSFO& ttsfo)
	{
		TOSHI_ERROR("This hunk doesn't support writing");
	}
};