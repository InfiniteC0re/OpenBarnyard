#pragma once

class AUIVerticalStack
{
public:
	void Reset() { flHeight = 0.0f; }
	void AddElement( TFLOAT a_flHeight ) { flHeight += a_flHeight; }

public:
	TFLOAT flHeight = 0.0f;
};
