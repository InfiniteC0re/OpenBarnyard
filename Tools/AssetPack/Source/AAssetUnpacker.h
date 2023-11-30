#pragma once
#include "AAssetPack.h"

class AAssetUnpacker
{
public:
	static TBOOL Unpack(AAssetPack& a_rAssetPack, const Toshi::TString8& a_rOutPath, TBOOL a_bUseBTEC = TFALSE);
};
