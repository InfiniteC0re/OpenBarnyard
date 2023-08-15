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
		m_pSelectedProperty = TNULL;
		m_pSelectedValue = TNULL;
		m_PropertyNameBuffer[0] = '\0';
		m_PropertyValueBuffer[0] = '\0';
	}

	~APropertiesResourceView()
	{
		TOSHI_INFO("Destroying APropertiesResourceView");
	}

	virtual void Render();

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
};
