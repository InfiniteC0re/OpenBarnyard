#include "pch.h"
#include "ALoadScreen.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUI2TextureSectionManager.h"
#include "GUI/AGUI2FontManager.h"
#include "Locale/ALocaleManager.h"

#include "Render/ARenderer.h"

ALoadScreen::ALoadScreen()
{
	m_bIsLoadingScreen = TFALSE;
	m_bFlag = TFALSE;
	m_iCowIndex = 1;
	m_fTotalTime = 0.0f;
	m_fTimer = 0.8f;
}

ALoadScreen::~ALoadScreen()
{

}

void ALoadScreen::Create()
{
	for (TUINT i = 0; i < NUM_LOAD_TEXTURES; i++)
	{
		TASSERT(m_aTextureSections[i] == TNULL, "ALoadScreen is already created");
		m_aTextureSections[i] = AGUI2TextureSectionManager::GetTextureSection(ms_LoadTextures[i]);
		TASSERT(m_aTextureSections[i] != TNULL, "Unable to find texture section");
	}

	auto pRootElement = AGUI2::GetRootElement();
	
	TFLOAT fWidth;
	TFLOAT fHeight;
	pRootElement->GetDimensions(fWidth, fHeight);

	m_CanvasElement.SetDimensions(fWidth, fHeight);
	m_CanvasElement.SetColour(TCOLOR(0, 0, 0));
	m_CanvasElement.Hide();

	pRootElement->AddChildHead(&m_CanvasElement);

	TFLOAT fRectPosX = fWidth * 0.1f;

	for (TUINT i = 0; i < 9; i++)
	{
		TINT iTexSection = i & 0x80000001;

		if (iTexSection < 0)
		{
			iTexSection = (iTexSection - 1U | 0xfffffffe) + 1;
		}

		auto pTexSection = m_aTextureSections[iTexSection];
		auto pRectangle = &m_aRectangles[i];

		pRectangle->SetDimensions(pTexSection->GetWidth(), pTexSection->GetHeight());
		pRectangle->SetTextureSection(pTexSection);
		pRectangle->SetAttachment(AGUI2Element::Anchor_BottomLeft, AGUI2Element::Pivot_BottomLeft);
		pRectangle->SetTransform(fRectPosX, -fHeight * 0.075f);
		pRectangle->Hide();

		pRootElement->AddChildHead(pRectangle);
		fRectPosX += (fWidth * 0.8f) / 9;
	}

	auto pFont = AGUI2FontManager::FindFont("Rekord26");
	auto pLoadingString = ALocaleManager::Interface()->GetString(1123);

	m_TextBox.Create( pFont, pFont->GetTextWidth(pLoadingString, 1.0f));
	m_TextBox.SetScale(1.0f);
	m_TextBox.SetText(pLoadingString);
	m_TextBox.SetAttachment(AGUI2Element::Pivot_BottomCenter, AGUI2Element::Pivot_BottomCenter);
	m_TextBox.SetTransform(0.0f, -fHeight * 0.08f);
	m_TextBox.Hide();

	pRootElement->AddChildHead(&m_TextBox);
	m_oTimer.Reset();

	m_fTimer = 0.8f;
	m_fTotalTime = 0.0f;
}

void ALoadScreen::StartLoading(TINT a_iUnused, TBOOL a_bRender)
{
	m_bIsLoadingScreen = TTRUE;

	if (!m_bFlag)
	{
		m_CanvasElement.Hide();
	}

	m_TextBox.Show();
	m_iCowIndex = 0;
	m_iCowPosIndex = 0;
	m_bFlag = TFALSE;

	m_oTimer.Reset();
	m_fTimer = 0.0f;
	m_fTotalTime = 0.0f;

	for (TUINT i = 0; i < 9; i++)
	{
		m_aRectangles[i].Show();
	}

	m_TextBox.Show();
	UpdateUI(a_bRender);
}

void ALoadScreen::SetLoadingState(TBOOL a_bIsLoading, TBOOL a_bUpdateUI)
{
	m_bIsLoadingScreen = a_bIsLoading;

	if (!m_bFlag)
	{
		if (a_bIsLoading) m_CanvasElement.Show();
		else m_CanvasElement.Hide();
	}

	if (!a_bIsLoading)
	{
		m_iCowIndex = 0;
	}

	if (a_bIsLoading) m_TextBox.Show();
	else m_TextBox.Hide();

	if (a_bUpdateUI) UpdateUI(a_bIsLoading);
}

void ALoadScreen::Update(TFLOAT a_fDeltaTime, TBOOL a_bRender)
{
	if (m_bIsLoadingScreen)
	{
		m_oTimer.Update();
		m_fTimer += m_oTimer.GetDelta();
		m_fTotalTime += m_oTimer.GetDelta();

		if (0.4f <= m_fTimer)
		{
			m_fTimer = 0.0f;

			if (m_iCowIndex < 3) m_iCowIndex++;
			m_iCowPosIndex = (m_iCowPosIndex + 1) % 9;
			UpdateUI(a_bRender);
		}
	}
}

void ALoadScreen::UpdateUI(TBOOL a_bRender)
{
	for (TUINT i = 0; i < 9; i++)
	{
		auto pRectangle = &m_aRectangles[i];

		if (m_iCowIndex == 3 && ((i + 2) % 9 == m_iCowPosIndex))
		{
			pRectangle->SetAlpha(0.2f);
			pRectangle->Show();
		}
		else if (m_iCowIndex < 2 || ((i + 1) % 9 != m_iCowPosIndex))
		{
			if (m_iCowIndex < 1 || i != m_iCowPosIndex)
			{
				pRectangle->Hide();
			}
			else
			{
				pRectangle->SetAlpha(1.0f);
				pRectangle->Show();
			}
		}
		else
		{
			pRectangle->SetAlpha(0.5f);
			pRectangle->Show();
		}
	}

	if (a_bRender)
	{
		ARenderer::GetSingleton()->ForceUpdate30FPS();
	}
}
