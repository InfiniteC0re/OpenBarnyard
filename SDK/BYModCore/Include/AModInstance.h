#pragma once
#include <Toshi/T2DList.h>
#include <Toshi/TString8.h>
#include <File/TTRB.h>

#include <Plugins/PPropertyParser/PBProperties.h>

#include <BYardSDK/THookedRenderD3DInterface.h>

class AModInstance :
	public Toshi::T2DList<AModInstance>::Node
{
public:
	AModInstance() = default;

	virtual ~AModInstance()
	{
		OnUnload();
	}

	virtual TBOOL				OnLoad() = 0;
	virtual void				OnUnload() {}
	virtual TBOOL				OnUpdate( TFLOAT a_fDeltaTime ) { return TTRUE; }
	virtual void				OnRenderInterfaceReady( Toshi::TRenderD3DInterface* a_pRenderInterface ) {}
	virtual void				OnAppRendererReady() {}
	virtual void				OnAllModsLoaded() {}
	virtual void				OnAGUI2Ready() {}
	virtual void				OnImGuiRender() {}
	virtual TBOOL				HasSettingsUI() { return TFALSE; }
	virtual Toshi::TTRB*		GetAssetFile() { return TNULL; }
	virtual const PBProperties* GetFileOverrides() { return TNULL; }

	const TCHAR* GetName() const { return m_strName.GetString(); }

public:
	HMODULE         m_hModule;
	Toshi::TString8 m_strName;
	Toshi::TVersion m_uiVersion;
};
