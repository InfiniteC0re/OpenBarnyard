#include "pch.h"
#include "ADisplayModes_Win.h"

#include <Platform/DX8/TRenderInterface_DX8.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS(ADisplayModes_Win);

void ADisplayModes_Win::Initialise()
{
	auto pRenderer = TSTATICCAST(TRenderD3DInterface*, TRenderInterface::GetSingleton());
	auto pAdapterList = pRenderer->GetAdapterList();

	for (auto pAdapter = pAdapterList->Begin(); pAdapter != pAdapterList->End(); pAdapter++)
	{
		auto uiNumSupportedDevices = pAdapter->GetNumSupportedDevices();
		auto pModeList = pAdapter->GetModeList();

		for (auto pMode = pModeList->Begin(); pMode != pModeList->End(); pMode++)
		{
			for (TUINT32 i = 0; i < uiNumSupportedDevices; i++)
			{
				auto pDevice = pMode->GetDevice(i);

				if (!pDevice->IsSoftware())
				{
					if (!pDevice->GetMode()->Is32Bit() && !pDevice->GetMode()->Is16Bit())
					{
						continue;
					}

					auto uiWidth = pDevice->GetMode()->GetWidth();
					auto uiHeight = pDevice->GetMode()->GetHeight();

					if (uiWidth == (uiHeight / 3) * 4   || // 4:3
						uiHeight == (uiWidth / 3) * 4   || // 4:3
						uiWidth == (uiHeight / 4) * 5   || // 5:4
						uiWidth == (uiHeight / 5) * 4   || // 4:5
						uiWidth == (uiHeight / 9) * 16  || // 16:9
						uiWidth == (uiHeight / 10) * 16)   // 16:10
					{
						if ((640 <= uiWidth && 480 <= uiHeight)   || // 4:3
							(1280 <= uiWidth && 1024 <= uiHeight) || // 5:4
							(1024 <= uiWidth && 1280 <= uiHeight) || // 4:5
							(1280 <= uiWidth && 720 <= uiHeight)  || // 16:9
							(1440 <= uiWidth && 900 <= uiHeight))    // 16:10
						{
							if (pDevice->GetMode()->Is16Bit())
							{
								if (!IsModeExists(uiWidth, uiHeight, 16))
								{
									m_Modes.PushBack({ .m_uiWidth = uiWidth, .m_uiHeight = uiHeight, .m_uiColourDepth = 16 });
								}
							}

							if (pDevice->GetMode()->Is32Bit())
							{
								if (!IsModeExists(uiWidth, uiHeight, 32))
								{
									m_Modes.PushBack({ .m_uiWidth = uiWidth, .m_uiHeight = uiHeight, .m_uiColourDepth = 32 });
								}
							}
						}
					}
				}
			}
		}
	}
}

TBOOL ADisplayModes_Win::IsModeExists(const TUINT32& a_rWidth, const TUINT32& a_rHeight, const TUINT32& a_rColourDepth)
{
	for (auto it = m_Modes.Begin(); it != m_Modes.End(); it++)
	{
		if (it->m_uiWidth == a_rWidth && it->m_uiHeight == a_rHeight && it->m_uiColourDepth == a_rColourDepth)
		{
			return TTRUE;
		}
	}

	return TFALSE;
}
