#include "ToshiPCH.h"
#include "TModelManager.h"

namespace Toshi {
	
	void TModelManager::AddModel(T2ModelPtr model)
	{
		TModel* pModel = model.GetData();
		pModel->m_NextModelResource = m_HeadModel;
		pModel->m_PrevModelResource = T2ResourcePtr::IDINVALID;

		if (m_HeadModel)
		{
			TModel* pFirstModel = m_HeadModel.GetData();
			pFirstModel->m_PrevModelResource = model;
		}

		m_HeadModel = model;
	}

	void TModelManager::RemoveModel(T2ModelPtr model)
	{
		TModel* pModel = model.GetData();

		if (pModel->m_NextModelResource)
		{
			TModel* pNextModel = pModel->m_NextModelResource.GetData();
			pNextModel->m_PrevModelResource = pModel->m_PrevModelResource;
		}

		if (pModel->m_PrevModelResource)
		{
			TModel* pPrevModel = pModel->m_PrevModelResource.GetData();
			pPrevModel->m_NextModelResource = pModel->m_NextModelResource;
		}

		if (m_HeadModel == model)
		{
			TModel* pHeadModel = m_HeadModel.GetData();
			m_HeadModel = pHeadModel->m_NextModelResource;
		}
	}

	void TModelManager::FindModel(T2ModelPtr& outModel, const char* name)
	{
		TASSERT(T2String8::IsLowerCase(name));

		T2ModelPtr currentModel = m_HeadModel;
		
		while (TTRUE)
		{
			if (!currentModel)
			{
				outModel = 0;
				return;
			}

			TModel* pModel = currentModel.GetData();
			int iResult = TStringManager::String8CompareNoCase(pModel->GetName(), name);

			if (iResult == 0)
				break;

			currentModel = pModel->m_NextModelResource;
		}

		outModel = currentModel;
	}

}
