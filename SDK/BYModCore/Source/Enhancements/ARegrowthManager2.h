#pragma once
#include <Toshi/T2SList.h>
#include <Toshi/TSceneObject.h>
#include <Math/TMatrix44.h>
#include <Math/TQuaternion.h>
#include <Render/TModelManager.h>

class ARegrowthManager2
{
public:
	struct Object
	    : Toshi::T2SList<Object>::Node
	{
		Toshi::TQuaternion qRotation;     // 0x04
		Toshi::TVector4    vPosition;     // 0x14
		TFLOAT             flSpawnTime;   // 0x24
		TUINT8             uiFlag;        // 0x28
		TUINT8             uiUnknown;     // 0x29
		TUINT8             uiScalePacked; // 0x2A: scale = packed / 255 * 0.4 + 0.8
	};

	struct GrowthMapping
	{
		TFLOAT flGrowthRate;     // 0x00
		TINT   iNumFrames;       // 0x04
		TCHAR  PADDING1[ 0x18 ]; // 0x08
		TUINT8 uiModelIndex;     // 0x20: 0xFF = unused mapping
		TUINT8 uiAnimType;       // 0x21: 1 = scale-up growth animation
		TUINT8 uiFlags;          // 0x22: bit 5 = ignore per-object scale
		TUINT8 uiFruitType;      // 0x23
		TCHAR  PADDING2[ 4 ];    // 0x24
	};

	struct ModelRef
	{
		Toshi::TManagedModel* pManagedModel;
		Toshi::TSceneObject*  pSceneObject;
	};

	struct ObjectList
	{
		Toshi::T2SList<Object> llInactive; // objects parked here are not rendered
		Toshi::T2SList<Object> llUsed;
	};

	struct RenderListEntry
	{
		Toshi::TMatrix44 oMatrix;
		RenderListEntry* pPrev;
	};

	inline static constexpr TINT MAX_MAPPINGS = 255;

public:
	void Render();

private:
	void FillRenderList();
	void FillRenderList_Visible();

public:
	TCHAR             PADDING0[ 4 ];                  // 0x00
	TINT              m_iNumModels;                   // 0x04
	ModelRef*         m_pModels;                      // 0x08
	GrowthMapping*    m_pGrowthMappings;              // 0x0C
	RenderListEntry** m_ppRenderLists;                // 0x10: m_iNumModels * 2 heads (near/far LOD)
	TCHAR             PADDING1[ 8 ];                  // 0x14
	ObjectList        m_aObjectLists[ MAX_MAPPINGS ]; // 0x1C

	TCHAR PADDING2[ 0x133D0 - 0x1C - MAX_MAPPINGS * sizeof( ObjectList ) ]; // 0x814

	TFLOAT           m_fLODSwitchDistance; // 0x133D0
	TFLOAT           m_fRenderDistance;    // 0x133D4
	TFLOAT           m_fBlendingDistance;  // 0x133D8
	RenderListEntry* m_pRenderBuffer;      // 0x133DC
	Toshi::TMatrix44 m_oModelViewMatrix;   // 0x133E0
	TFLOAT           m_fBlendingFactor;    // 0x13420
	TINT8            m_eFlags;             // 0x13424: bit 0 = enabled, sign bit = VIS-filtered fill
	TCHAR            PADDING3[ 7 ];        // 0x13425
};
