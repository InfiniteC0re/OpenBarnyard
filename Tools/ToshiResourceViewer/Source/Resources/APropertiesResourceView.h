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

	}

	~APropertiesResourceView()
	{
		TOSHI_INFO("Destroying APropertiesResourceView");
	}

	virtual void Render();

	PProperties& GetProperties() { return m_Properties; }
	const PProperties& GetProperties() const { return m_Properties; }

private:
	PProperties m_Properties;
};
