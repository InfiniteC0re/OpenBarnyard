#include "ToshiPCH.h"
#include "TTRB.h"

bool Toshi::TTRB::LoadTrb(const char* param_1)
{
	TCString str(param_1);

	TFile* f = TFile::Create(str, 1);

	str.~TCString();



	return false;
}
