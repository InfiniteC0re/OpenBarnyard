#pragma once
#include "Render/AWorldShader/AWorldShader.h"

#include <Toshi/TNodeList.h>
#include <Render/TOrderTable.h>

class AWorldShaderHAL : public AWorldShader
{
public:
	TDECLARE_CLASS(AWorldShader);

	struct AUnknown : Toshi::TNodeList<AUnknown>::TNode
	{

	};

public:
	AWorldShaderHAL();
	~AWorldShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void Flush() override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	/**
	 * Returns TTRUE if shaders are supported
	 */
	virtual TBOOL IsHighEndMode();

	/**
	 * Enabled high end mode (compiled shader) if they are supported by hardware
	 */
	virtual void SetHighEndMode(TBOOL a_bEnable);

	/**
	 * Returns TTRUE if shaders are supported by hardware
	 */
	virtual TBOOL IsCapableShaders();

	/**
	 * Returns TTRUE if rendering of env map is enabled
	 */
	virtual TBOOL IsRenderEnvMapEnabled();

	/**
	 * Probably used in debug mode but is stripped out in release
	 */
	virtual void* CreateUnknown(void*, void*, void*, void*);

private:
	void FlushLowEnd();

private:
	Toshi::TNodeList<AUnknown> m_SomeList;
	TFLOAT m_fUnk1;
	TINT* m_pUnk2;
	TUINT m_eAlphaRef;
	TBOOL m_bRenderEnvMap;
	Toshi::TOrderTable m_aOrderTables[9];
	TBOOL m_bIsHighEndMode;
	TBOOL m_bAlphaBlendMaterial;
	TBOOL m_bUnkFlag3;
	TBOOL m_bUnkFlag4;
	Toshi::TVector4 m_ShadowColour;
	Toshi::TVector4 m_AmbientColour;
};