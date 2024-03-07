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

	static constexpr TUINT NUM_ORDER_TABLES = 9;

public:
	AWorldShaderHAL();
	~AWorldShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void Flush() override;
	virtual void StartFlush() override;
	virtual void EndFlush() override;
	virtual TBOOL Create() override;
	virtual TBOOL Validate() override;
	virtual void Invalidate() override;
	virtual TBOOL TryInvalidate() override;
	virtual TBOOL TryValidate() override;
	virtual void Render(Toshi::TRenderPacket* a_pRenderPacket) override;

	//-----------------------------------------------------------------------------
	// AWorldShader
	//-----------------------------------------------------------------------------
	virtual void EnableRenderEnvMap(TBOOL a_bEnable) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Returns TTRUE if alpha blend material is enabled
	virtual TBOOL IsAlphaBlendMaterial() override;

	// Enabled alpha blend material
	virtual void SetAlphaBlendMaterial(TBOOL a_bIsAlphaBlendMaterial) override;

	// Creates AWorldMaterial and returns it
	virtual AWorldMaterial* CreateMaterial(const TCHAR* a_szName) override;

	// Creates AWorldMesh and returns it
	virtual AWorldMesh* CreateMesh(const TCHAR* a_szName) override;

	// Returns TTRUE if shaders are supported
	virtual TBOOL IsHighEndMode();

	// Enables high end mode (compiled shader) if they are supported by hardware
	virtual void SetHighEndMode(TBOOL a_bEnable);

	// Returns TTRUE if shaders are supported by hardware
	virtual TBOOL IsCapableShaders();

	// Returns TTRUE if rendering of env map is enabled
	virtual TBOOL IsRenderEnvMapEnabled();

	// Probably used in debug mode but is stripped out in release
	virtual void* CreateUnknown(void*, void*, void*, void*);

	void SetColours(const Toshi::TVector4& a_rShadowColour, const Toshi::TVector4 a_rAmbientColour);

	TUINT GetAlphaRef() const { return m_iAlphaRef; }
	void SetAlphaRef(TUINT val) { m_iAlphaRef = val; }

	const Toshi::TVector4& GetShadowColour() const { return m_ShadowColour; }
	const Toshi::TVector4& GetAmbientColour() const { return m_AmbientColour; }

	Toshi::TOrderTable* GetOrderTable(TUINT a_uiIndex)
	{
		TASSERT(a_uiIndex < NUM_ORDER_TABLES);
		return &m_aOrderTables[a_uiIndex];
	}

	inline static TUINT s_RenderStateFlags = 27;

private:
	void SetupLowEndMode();

private:
	Toshi::TNodeList<AUnknown> m_SomeList;
	DWORD m_hVertexShader;
	TINT* m_pUnk2;
	TUINT m_iAlphaRef;
	TBOOL m_bRenderEnvMap;
	Toshi::TOrderTable m_aOrderTables[NUM_ORDER_TABLES];
	TBOOL m_bIsHighEndMode;
	TBOOL m_bAlphaBlendMaterial;
	TBOOL m_bUnkFlag3;
	TBOOL m_bUnkFlag4;
	Toshi::TVector4 m_ShadowColour;
	Toshi::TVector4 m_AmbientColour;
};