#pragma once
#include "Toshi/Render/TModel.h"

namespace Toshi {

	class TModelHAL :
		public TModel
	{
	public:
		virtual TBOOL Create(TTMD* a_pTMD, TBOOL a_bLoad) override;
		virtual TBOOL Create(const char* a_szFileName, TBOOL a_bLoad) override;
		virtual void Delete() override;
		virtual TBOOL LoadFromFile(const char* a_szFileName) override;
		virtual void Unload() override;
		virtual TBOOL GetUnknownFlag() override;
		virtual TBOOL LoadTMDHAL(TTMD* a_pTMD);
		virtual TBOOL LoadTMDHAL(const char* a_szFileName, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen);
	
		TBOOL Create(const char* a_szFileName, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen);
	};

}