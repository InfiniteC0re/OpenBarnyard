#pragma once
#include "AResourceView.h"
#include <Plugins/PPropertyParser/PProperties.h>

class APropertiesResourceView : public AResourceView
{
public:
	APropertiesResourceView(AResourceFilePtr a_pResourceFile) :
		AResourceView(a_pResourceFile),
		m_Properties(*PProperties::LoadFromTRB(a_pResourceFile->GetTRB()))
	{
		static int s_iGlobalId = 0;

		m_pSelectedProperty = TNULL;
		m_pSelectedValue = TNULL;
		m_PropertyNameBuffer[0] = '\0';
		m_PropertyValueBuffer[0] = '\0';

		Toshi::TStringManager::String8Format(m_szIdListWindowName, 48, "PPropertyList%d", s_iGlobalId);
		Toshi::TStringManager::String8Format(m_szIdEditorWindowName, 48, "PPropertyEditor%d", s_iGlobalId);
		Toshi::TStringManager::String8Format(m_szIdPropertyName, 48, "##PropertyName%d", s_iGlobalId);
		Toshi::TStringManager::String8Format(m_szIdPropertyValue, 48, "##PropertyValue%d", s_iGlobalId);
		s_iGlobalId++;
	}

	~APropertiesResourceView()
	{
		TOSHI_INFO("Destroying APropertiesResourceView");
	}

	virtual void Render() override;

	virtual TBOOL SupportsSave() override { return TTRUE; }
	virtual void Save() override;

	PProperties& GetProperties() { return m_Properties; }
	const PProperties& GetProperties() const { return m_Properties; }

private:
	void ShowProperties(PProperties* a_pProperties);

private:
	PProperties m_Properties;
	PProperties::PProperty* m_pSelectedProperty;
	PPropertyValue* m_pSelectedValue;
	char m_PropertyNameBuffer[512];
	char m_PropertyValueBuffer[512];
	TBOOL m_bInitialised = TFALSE;
	char m_szIdListWindowName[48];
	char m_szIdEditorWindowName[48];
	char m_szIdPropertyName[48];
	char m_szIdPropertyValue[48];
};
