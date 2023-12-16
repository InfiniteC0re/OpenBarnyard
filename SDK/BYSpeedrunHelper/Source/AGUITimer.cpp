#include "pch.h"
#include "AGUITimer.h"

AGUITimer::AGUITimer()
{
	m_pTextBox = TNULL;
	m_fTotalTime = 0.0f;
	m_bIsVisible = TTRUE;
	m_bIsStopped = TTRUE;
}

AGUITimer::~AGUITimer()
{
	if (m_pTextBox)
	{
		delete m_pTextBox;
		m_pTextBox = TNULL;
	}
}

void AGUITimer::Create()
{
	auto pFont = AGUI2FontManager::FindFont("Rekord18");
	m_pTextBox = AGUI2TextBox::CreateFromEngine();
	m_pTextBox->Create(pFont, 200.0f);
	m_pTextBox->SetText(L"00:00:00.000");
	m_pTextBox->SetColour(TCOLOR(253, 226, 1));
	m_pTextBox->SetTransform(-394.0f, 0.0f);
	m_pTextBox->SetAlpha(1.0f);
	m_pTextBox->SetInFront();
	m_pTextBox->SetTextAlign(AGUI2Font::TextAlign_Left);
	m_pTextBox->SetAttachment(AGUI2Element::Anchor_MiddleLeft, AGUI2Element::Pivot_MiddleLeft);
}

void AGUITimer::Update(TFLOAT a_fDeltaTime)
{
	if (!IsStopped())
	{
		TINT iMilliseconds = Toshi::TMath::FloorToInt(fmod(m_fTotalTime, 1.0f) * 1000);
		TINT iSeconds = Toshi::TMath::FloorToInt(fmod(m_fTotalTime, 60.0f));
		TINT iMinutes = Toshi::TMath::FloorToInt(m_fTotalTime / 60.0f);
		TINT iHours = Toshi::TMath::FloorToInt(m_fTotalTime / 3600.0f);

		static wchar_t s_buffer[48];

		if (iHours != 0)
		{
			Toshi::TStringManager::String16Format(s_buffer, sizeof(s_buffer), L"%02d:%02d:%02d.%03d", iHours, iMinutes, iSeconds, iMilliseconds);
		}
		else if (iMinutes != 0)
		{
			Toshi::TStringManager::String16Format(s_buffer, sizeof(s_buffer), L"%02d:%02d.%03d", iMinutes, iSeconds, iMilliseconds);
		}
		else
		{
			Toshi::TStringManager::String16Format(s_buffer, sizeof(s_buffer), L"%d.%03d", iSeconds, iMilliseconds);
		}

		m_pTextBox->SetText(s_buffer);
		m_fTotalTime += a_fDeltaTime;
	}
}

void AGUITimer::Render()
{
	if (m_pTextBox && m_bIsVisible)
	{
		m_pTextBox->PreRender();
		m_pTextBox->Render();
		m_pTextBox->PostRender();
	}
}
