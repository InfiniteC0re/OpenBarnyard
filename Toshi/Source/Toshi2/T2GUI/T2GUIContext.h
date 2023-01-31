#pragma once
#include "T2GUIElement.h"
#include "Toshi/Render/TRender.h"

namespace Toshi
{
	class T2GUIContext
	{
	public:
		T2GUIContext()
		{
			auto pDisplayParams = TRender::GetSingleton()->GetCurrentDisplayParams();

			m_RootElement.SetDimensions((float)pDisplayParams->Width, (float)pDisplayParams->Height);
			m_RootElement.SetTransform(0, 0, 0);
		}

		T2GUIElement* GetRootElement()
		{
			return &m_RootElement;
		}

	private:
		T2GUIElement m_RootElement;
	};
}