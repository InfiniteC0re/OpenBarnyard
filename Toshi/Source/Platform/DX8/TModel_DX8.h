#pragma once
#include "Render/TModel.h"

TOSHI_NAMESPACE_START

class TModelHAL :
    public TModel
{
public:
	TModelHAL();
	~TModelHAL();

	virtual TBOOL Create( TTMD* a_pTMD, TBOOL a_bLoad ) override;
	virtual TBOOL Create( const TCHAR* a_szFileName, TBOOL a_bLoad ) override;
	virtual void  Delete() override;
	virtual TBOOL LoadFromFile( const TCHAR* a_szFileName ) override;
	virtual void  Unload() override;
	virtual TBOOL GetUnknownFlag() override;
	virtual TBOOL LoadTMDHAL( TTMD* a_pTMD );
	virtual TBOOL LoadTMDHAL( const TCHAR* a_szFileName, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen );

	TBOOL Create( const TCHAR* a_szFileName, TBOOL a_bLoad, TTRB* a_pAssetTRB, TUINT8 a_ui8FileNameLen );
};

TOSHI_NAMESPACE_END
