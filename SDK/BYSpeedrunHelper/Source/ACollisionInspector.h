#pragma once
#include <BYardSDK/ATerrain.h>
#include <BYardSDK/ASysMaterial.h>

#include <Render/TModel.h>

class ACollisionInspector :
	public Toshi::TSingleton<ACollisionInspector>
{
public:
	ACollisionInspector();

	TBOOL& IsCollisionVisible() { return m_bCollisionVisible; }
	void SetCollisionVisible(TBOOL a_bVisible);

private:
	inline static ASysMaterial* ms_pCollisionMaterial;

private:
	static TBOOL AModelLoader_LoadTRBCallback(Toshi::TModel* a_pModel);
	static void ATerrain_Render(ATerrain* a_pTerrain);

private:
	TBOOL m_bCollisionVisible = TFALSE;
};
