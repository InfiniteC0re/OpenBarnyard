#include "pch.h"
#include "AGUISlideshow.h"
#include "Locale/ALocaleManager.h"
#include "AGUISystem.h"
#include "AGUI2.h"

AGUISlideshow::AGUISlideshow() :
	m_Images(&Toshi::T2Allocator::s_GlobalAllocator, 64, 64)
{
	m_pHUDParams = TNULL;
	m_eFlags = Flags_None;
	m_bIsActive = TFALSE;
	m_fCurrentSlideTime = 0.0f;
	m_fUnk2 = 0.0f;
	m_fDuration = 1.0f;
	m_fFadeInTime = 0.0f;
	m_fFadeOutTime = 0.0f;
	m_fUnk4 = -1.0f;
	m_bIsAppearing = TFALSE;
	m_bShouldLocalise = TFALSE;
}

Toshi::TPString8* AGUISlideshow::LocaliseBackgroundFileName(Toshi::TPString8& a_rOutName, const Toshi::TPString8& a_rName)
{
	LocaliseBackgroundFileName(a_rOutName, a_rName.GetString8().GetString());
	return &a_rOutName;
}

void AGUISlideshow::LocaliseBackgroundFileName(Toshi::TPString8& a_rOutName, const char* a_szName)
{
	char szFormattedName[256];
	auto eLang = ALocaleManager::GetSingleton()->GetLangId();

	if (eLang != ALocaleManager::Lang_English && eLang != ALocaleManager::Lang_EnglishUK)
	{
		char szLang[4];
		Toshi::TStringManager::String8Copy(
			szLang,
			ALocaleManager::Interface()->GetCurrentLanguageName(),
			sizeof(szLang)
		);

		szLang[1] = '\0';
		Toshi::TStringManager::String8Format(szFormattedName, sizeof(szFormattedName), "%s%s", a_szName, szLang);
		a_szName = szFormattedName;
	}

	a_rOutName = Toshi::TPString8(a_szName);
}

void AGUISlideshow::Activate()
{
	TASSERT(m_Images.Size() > 0);

	m_ImageIterator = m_Images;
	m_fCurrentSlideTime = 0.0f;
	m_fUnk2 = 0.0f;

	if (!m_bIsAppearing)
	{
		if (!m_bShouldLocalise)
		{
			AGUISystem::GetSingleton()->SetPicture(m_Images[0]);
		}
		else
		{
			Toshi::TPString8 pictureName;
			LocaliseBackgroundFileName(pictureName, m_Images[0]);
			AGUISystem::GetSingleton()->SetPicture(pictureName);
		}
	}

	UpdateFadeOverlay();
	m_bIsActive = TTRUE;
}

void AGUISlideshow::UpdateFadeOverlay()
{
	TFLOAT fOpacity;

	if (!m_bIsAppearing)
	{
		if (m_fFadeInTime <= m_fCurrentSlideTime)
		{
			if (m_fCurrentSlideTime <= m_fDuration + m_fFadeInTime)
			{
				fOpacity = 0.0f;
			}
			else
			{
				fOpacity = (m_fCurrentSlideTime - m_fFadeInTime - m_fDuration) / m_fFadeOutTime;

				if (fOpacity > 1.0f)
				{
					fOpacity = 1.0f;
				}
			}
		}
		else
		{
			fOpacity = 1.0f - m_fCurrentSlideTime / m_fFadeInTime;
		}
	}
	else
	{
		fOpacity = m_fCurrentSlideTime / m_fFadeInTime;
		
		if (fOpacity >= 1.0f)
		{
			Toshi::TPString8 pictureName;
			LocaliseBackgroundFileName(pictureName, m_ImageIterator.Current());
			AGUISystem::GetSingleton()->SetPicture(pictureName);

			m_bIsAppearing = TFALSE;
			m_fCurrentSlideTime = 0.0f;
			fOpacity = 1.0f;
		}
	}

	TASSERT(fOpacity <= 1.0f);
	m_FadeOverlay.SetColour(TUINT32(fOpacity * 255) << 24);
	m_FadeOverlay.Unlink();
	AGUI2::GetRootElement()->AddChildHead(&m_FadeOverlay);
	m_FadeOverlay.Show();
}

void AGUISlideshow::Update(TFLOAT a_fDeltaTime)
{
	if (m_bIsActive && !IsSlideshowOver())
	{
		m_fCurrentSlideTime += a_fDeltaTime;
		
		if (m_bIsAppearing)
		{
			UpdateFadeOverlay();
			return;
		}

		m_fUnk2 += a_fDeltaTime;

		if (m_fCurrentSlideTime >= m_fFadeOutTime + m_fFadeInTime + m_fDuration)
		{
			SwitchToNextSlide();
		}

		UpdateFadeOverlay();
		return;
	}

	m_FadeOverlay.Unlink();
}

TBOOL AGUISlideshow::IsSlideshowOver()
{
	if (!HASFLAG(m_eFlags & Flags_Repeat))
	{
		return m_ImageIterator.IsOver();
	}

	if (!HASFLAG(m_eFlags & Flags_Ended) && (m_fUnk4 <= 0.0f || m_fUnk2 <= m_fUnk4))
	{
		return TFALSE;
	}

	return TTRUE;
}

void AGUISlideshow::SwitchToNextSlide()
{
	if (!m_ImageIterator.IsOver())
	{
		m_ImageIterator++;
	}

	if (m_ImageIterator.IsOver())
	{
		if (!HASFLAG(m_eFlags & Flags_Repeat)) return;
		m_ImageIterator = m_Images;
	}

	m_fCurrentSlideTime = 0.0f;

	Toshi::TPString8 pictureName;
	LocaliseBackgroundFileName(pictureName, m_ImageIterator.Current());
	AGUISystem::GetSingleton()->SetPicture(pictureName);
}

void AGUISlideshow::Deactivate()
{
	TASSERT(TTRUE == m_bIsActive);
	
	m_bIsActive = TFALSE;
	AGUISystem::GetSingleton()->SetPicture(Toshi::TPString8());
	m_eFlags |= Flags_Ended;
}

TBOOL AGUISlideshow::Setup(AGameState::HUDParams* a_pHUDParams, const Params& a_rParams, TBOOL a_bShouldLocalise)
{
	if (m_pHUDParams)
	{
		return TFALSE;
	}

	m_bShouldLocalise = a_bShouldLocalise;
	m_eFlags = Flags_None;

	if (a_rParams.iUnk1 != 0)
	{
		return TFALSE;
	}

	m_pHUDParams = a_pHUDParams;

	if (a_rParams.bSlideSkippable)
	{
		m_eFlags |= Flags_Skippable;
	}

	if (a_rParams.bUnk2)
	{
		m_eFlags |= Flags_Unk1;
	}

	if (a_rParams.bInstaSkippable)
	{
		m_eFlags |= Flags_InstaSkippable;
	}

	if (a_rParams.bHasFadeIn)
	{
		m_eFlags |= Flags_HasFadeIn;
		m_fFadeInTime = a_rParams.fFadeInTime;
	}

	if (a_rParams.bHasFadeOut)
	{
		m_eFlags |= Flags_HasFadeOut;
		m_fFadeOutTime = a_rParams.fFadeOutTime;
	}

	if (a_rParams.bRepeat)
	{
		m_eFlags |= Flags_Repeat;
	}

	m_fDuration = a_rParams.fDuration;
	m_fUnk4 = a_rParams.fUnk5;

	TFLOAT fWidth, fHeight;
	AGUI2::GetSingleton()->GetDimensions(fWidth, fHeight);
	m_FadeOverlay.SetDimensions(fWidth, fHeight);
	m_FadeOverlay.SetAttachment(AGUI2Element::Anchor_TopCenter, AGUI2Element::Pivot_TopCenter);
	m_FadeOverlay.Hide();

	if (a_rParams.bDelayAppear)
	{
		m_bIsAppearing = TTRUE;
	}

	return TTRUE;
}

TBOOL AGUISlideshow::ProcessInput(const Toshi::TInputInterface::InputEvent* a_pEvent)
{
	if (a_pEvent->GetEventType() == Toshi::TInputInterface::EVENT_TYPE_GONE_DOWN)
	{
		if (m_eFlags & Flags_InstaSkippable)
		{
			m_ImageIterator = m_Images.End();
			m_eFlags |= Flags_Ended;
			return TTRUE;
		}

		if (m_eFlags & Flags_Skippable)
		{
			SwitchToNextSlide();
			return TTRUE;
		}
	}

	return TFALSE;
}

TBOOL AGUISlideshow::ProcessCommand(AInputCommand a_eCommand)
{
	if (((a_eCommand != AInputCommand_Back && a_eCommand != AInputCommand_Cancel && a_eCommand != AInputCommand_Quit) || !HASFLAG(m_eFlags & Flags_Unk1)) && !HASFLAG(m_eFlags & Flags_InstaSkippable))
	{
		if (m_eFlags & Flags_Skippable)
		{
			SwitchToNextSlide();
			return TTRUE;
		}

		return TFALSE;
	}

	m_ImageIterator = m_Images.End();
	m_eFlags |= Flags_Ended;
	return TTRUE;
}

void AGUISlideshow::Reset()
{
	m_ImageIterator = m_Images;
	m_fCurrentSlideTime = 0.0f;
	m_fUnk2 = 0.0f;
}
