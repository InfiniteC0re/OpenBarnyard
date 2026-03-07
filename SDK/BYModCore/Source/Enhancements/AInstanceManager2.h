#pragma once
#include <BYardSDK/AInstanceManager.h>

//-----------------------------------------------------------------------------
// Purpose: better version of the instance manager that allows rendering more
// objects at once.
//-----------------------------------------------------------------------------
class AInstanceManager2 : protected AInstanceManager
{
public:
	TBOOL Render();
	void  FillRenderList_All( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic );
	void  FillRenderList_Visible( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic );
};
