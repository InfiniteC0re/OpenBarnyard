#include "pch.h"
#include "AWorldVIS.h"

#include <Render/TRenderInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AWorldVis::AWorldVis()
{
	m_pWorld = TNULL;
	m_pCellNodes = TNULL;
	m_iNumBuiltCells = 0;
	m_iNumCellSettings = 0;
	m_pfnRenderCallback = TNULL;
}

AWorldVis::~AWorldVis()
{
	Reset();

	if (m_pCellNodes)
	{
		delete[] m_pCellNodes;
		m_pCellNodes = TNULL;
	}
}

void AWorldVis::Create(World* a_pWorld)
{
	m_pWorld = a_pWorld;
	m_pCellNodes = new CellNode[a_pWorld->m_iNumCells];
	Reset();
}

void AWorldVis::Reset()
{
	for (TINT i = 0; i < m_iNumBuiltCells; i++)
	{
		m_pWorld->m_ppCells[m_pCellNodes[i].iCellIndex]->pNode = TNULL;
	}

	m_iNumBuiltCells = 0;
	m_iNumCellSettings = 0;
}

void AWorldVis::Build(const Toshi::TMatrix44& a_rModelView, const Toshi::TMatrix44& a_rProjection, TINT a_iCellIndex)
{
	TMatrix44 viewModel = a_rModelView;
	viewModel.InvertOrthogonal();
	m_ViewModelPos = viewModel.GetTranslation().AsVector3();

	CullBox cullBox;
	cullBox.x = 0.0f;
	cullBox.y = 0.0f;
	cullBox.width = TFLOAT(TRenderInterface::GetSingleton()->GetCurrentDisplayParams()->uiWidth);
	cullBox.height = TFLOAT(TRenderInterface::GetSingleton()->GetCurrentDisplayParams()->uiHeight);

	Build(a_rModelView, a_rProjection, a_iCellIndex, cullBox);
}

// Seems to be unused in Barnyard
static TUINT s_uiBuildFlags = 0;

void AWorldVis::Build(const Toshi::TMatrix44& a_rModelView, const Toshi::TMatrix44& a_rProjection, TINT a_iCellIndex, const CullBox& a_rCullBox)
{
	TASSERT(a_iCellIndex < m_pWorld->m_iNumCells);

	if (ISZERO(s_uiBuildFlags & 1))
	{
		s_uiBuildFlags |= 1;
	}

	auto pCell = m_pWorld->m_ppCells[a_iCellIndex];

	if (ISZERO(pCell->uiFlags & Cell::FLAGS_BUILDING))
	{
		pCell->uiFlags |= Cell::FLAGS_BUILDING;

		auto pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();
		auto pNode = pCell->pNode;

		if (TNULL == pNode)
		{
			TASSERT(m_iNumBuiltCells < MAX_VISIBLE_CELLS);
			pNode = m_pCellNodes + (m_iNumBuiltCells++);
			pCell->pNode = pNode;
			pNode->iCellIndex = a_iCellIndex;
			pNode->pCellSettings = TNULL;
		}

		TVALIDPTR(pNode);

		auto pCellSettings = &m_aCellSettings[m_iNumCellSettings++];

		pCellSettings->pNext = pNode->pCellSettings;
		pCellSettings->oCullBox = a_rCullBox;
		pNode->pCellSettings = pCellSettings;

		for (TINT i = 0; i < pCell->m_iSomeCount; i++)
		{
			// Seems to be never used in Barnyard so I'll just pray it will never get here
			TASSERT(!"Well, we are fucked up");
		}

		pCell->uiFlags &= ~Cell::FLAGS_BUILDING;
	}
}

static void CreatePortalFrustum(Frustum& a_rFrustum, CullBox& a_rCullBox, TMatrix44& a_rMatrix)
{
	auto pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

	a_rFrustum.aFrustumPlanes[0].oPlane = pRenderContext->GetFrustumPlanes()[0];
	a_rFrustum.aFrustumPlanes[1].oPlane = pRenderContext->GetFrustumPlanes()[1];
	a_rFrustum.aFrustumPlanes[2].oPlane = pRenderContext->GetFrustumPlanes()[2];
	a_rFrustum.aFrustumPlanes[3].oPlane = pRenderContext->GetFrustumPlanes()[3];
	a_rFrustum.aFrustumPlanes[4].oPlane = pRenderContext->GetFrustumPlanes()[4];
	a_rFrustum.aFrustumPlanes[5].oPlane = pRenderContext->GetFrustumPlanes()[5];
	a_rFrustum.aFrustumPlanes[0].uiClipFlag = 1;
	a_rFrustum.aFrustumPlanes[1].uiClipFlag = 2;
	a_rFrustum.aFrustumPlanes[2].uiClipFlag = 4;
	a_rFrustum.aFrustumPlanes[3].uiClipFlag = 8;
	a_rFrustum.aFrustumPlanes[4].uiClipFlag = 16;
	a_rFrustum.aFrustumPlanes[5].uiClipFlag = 32;
}

void AWorldVis::Render(const Toshi::TMatrix44& a_rModelView)
{
	TCHAR aStack[1212];
	AWorldVis::s_pStack = aStack;

	if (TNULL != m_pfnRenderCallback)
	{
		auto pRenderContext = TRenderInterface::GetSingleton()->GetCurrentContext();

		TMatrix44 viewModel = a_rModelView;
		viewModel.InvertOrthogonal();

		TMatrix44 projection;
		TRenderContext::ComputePerspectiveProjection(
			projection,
			pRenderContext->GetViewportParameters(),
			pRenderContext->GetProjectionParams()
		);

		for (TINT i = 0; i < m_iNumBuiltCells; i++)
		{
			Frustum portalFrustum;
			Frustum frustum;

			auto pCell = m_pWorld->m_ppCells[m_pCellNodes[i].iCellIndex];

			for (auto pSettings = pCell->pNode->pCellSettings; pSettings != TNULL; pSettings = pSettings->pNext)
			{
				CreatePortalFrustum(portalFrustum, pSettings->oCullBox, projection);
				frustum.Transform(portalFrustum, viewModel);
				frustum.InitReduce();

				RenderData renderData;
				renderData.pCell = pCell;
				renderData.pFrustum = &frustum;

				RenderTreeIntersectNonRecurse(pCell->pTreeBranchNodes, &renderData);
			}
		}
	}
}

void AWorldVis::RenderTreeIntersectNonRecurse(CellSphereTreeBranchNode* a_pNode, RenderData* a_pRenderData)
{
	TIMPLEMENT();

	auto pRightNode = a_pNode->m_pRight;
	auto pNode = a_pNode;

	while (pRightNode != TNULL)
	{
		pNode = pRightNode;
		pRightNode = pRightNode->m_pRight;
	}
}

void AWorldVis::RenderLeafNodeIntersect(CellSphereTreeBranchNode* a_pNode, RenderData* a_pRenderData)
{
	// Use this to restore active planes after reduce intersect
	TINT iActivePlaneCount = a_pRenderData->pFrustum->iActivePlaneCount;

	for (TUINT i = 0; i < a_pNode->GetData()->m_uiNumMeshes; i++)
	{
		CellMeshSphere* pMeshSphere = a_pRenderData->pCell->ppCellMeshSpheres[a_pNode->GetData()->GetMeshIndex(i)];
		TINT iIntersectResult = a_pRenderData->pFrustum->IntersectSphereReduce(pMeshSphere->m_BoundingSphere);

		if (iIntersectResult != -1)
		{
			m_pfnRenderCallback(pMeshSphere, a_pRenderData);
		}

		a_pRenderData->pFrustum->iActivePlaneCount = iActivePlaneCount;
	}
}
