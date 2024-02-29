#pragma once
#include "AWorld.h"

class AWorldVis
{
public:
	static constexpr TUINT MAX_VISIBLE_CELLS = 1;

	using t_RenderCallback = void(*)(CellMeshSphere* a_pMeshSphere, RenderData* a_pRenderData);

public:
	AWorldVis();
	~AWorldVis();

	void Create(World* a_pWorld);
	void Reset();

	void Build(const Toshi::TMatrix44& a_rModelView, const Toshi::TMatrix44& a_rProjection, TINT a_iCellIndex);
	void Build(const Toshi::TMatrix44& a_rModelView, const Toshi::TMatrix44& a_rProjection, TINT a_iCellIndex, const CullBox& a_rCullBox);

	void Render(const Toshi::TMatrix44& a_rModelView);
	void RenderTreeIntersectNonRecurse(CellSphereTreeBranchNode* a_pNode, RenderData* a_pRenderData);
	void RenderLeafNodeIntersect(CellSphereTreeBranchNode* a_pNode, RenderData* a_pRenderData);

private:
	inline static TCHAR* s_pStack;

public:
	World* m_pWorld;
	CellSettings m_aCellSettings[MAX_VISIBLE_CELLS];
	CellNode* m_pCellNodes;
	TINT m_iNumBuiltCells;
	TINT m_iNumCellSettings;
	TINT Unknown;
	Toshi::TVector3 m_ViewModelPos;
	t_RenderCallback m_pfnRenderCallback;
};