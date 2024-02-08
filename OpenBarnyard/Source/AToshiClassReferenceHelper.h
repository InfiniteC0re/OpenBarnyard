#pragma once

//-----------------------------------------------------------------------------
// Create this class as a static object in order to link TOSHI
// classes that will not link without a reference.
//-----------------------------------------------------------------------------
class AToshiClassReferenceHelper
{
public:
	AToshiClassReferenceHelper();
};
