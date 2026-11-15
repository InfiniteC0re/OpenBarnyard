#pragma once
#include "Render/ASkinShader/ASkinShader.h"

#include <Render/TOrderTable.h>

#include "Platform/GL/TGLHeaders.h"

class MODAPI ASkinShaderHAL : public ASkinShader
{
public:
	TDECLARE_CLASS( ASkinShaderHAL, ASkinShader );

	// Render states set by the last packet, the material's PreRender sets RENDERSTATE_ALPHABLEND to force them again
	using RENDERSTATE = TUINT;
	enum RENDERSTATE_ : RENDERSTATE
	{
		RENDERSTATE_NONE       = 0,
		RENDERSTATE_UNK0       = BITFLAG( 0 ),
		RENDERSTATE_UNK1       = BITFLAG( 1 ),
		RENDERSTATE_UNK3       = BITFLAG( 3 ),
		RENDERSTATE_UNK4       = BITFLAG( 4 ),
		RENDERSTATE_ALPHAREF   = RENDERSTATE_UNK3 | RENDERSTATE_UNK4,                                       // the alpha reference is scaled by the packet alpha
		RENDERSTATE_ALPHABLEND = RENDERSTATE_UNK0 | RENDERSTATE_UNK1 | RENDERSTATE_UNK3 | RENDERSTATE_UNK4, // alpha blending is on
	};

	static constexpr TUINT NUM_ORDER_TABLES = 3;
	static constexpr TUINT MAX_BONES        = 28;

	// Most skin meshes are trees and other static things of one submesh and one bone. The same mesh drawn
	// several times in a row is one instanced draw, its packets' constants go in an array
	static constexpr TUINT MAX_INSTANCES = 20;
	static constexpr TUINT INSTANCE_SIZE = 11; // in constants: MVP 4, bone 3, light direction, ambient, light colour, up vector (w: lighting lerp)

	enum CONSTANT_ : TINT
	{
		CONSTANT_MVP             = 0,
		CONSTANT_BONEINDEXSCALE  = 4,
		CONSTANT_LIGHTDIRECTION  = 5,
		CONSTANT_AMBIENTCOLOUR   = 6,
		CONSTANT_LIGHTCOLOUR     = 7,
		CONSTANT_LIGHTINGRANGE   = 8,
		CONSTANT_UPVECTOR        = 9,
		CONSTANT_ALPHATEST       = 10,
		CONSTANT_FOG             = 11,
		CONSTANT_FOGCOLOUR       = 12,
		CONSTANT_LIGHTINGLERP    = 13,
		CONSTANT_STAGES          = 14,
		CONSTANT_BONES           = 15, // last, a submesh only uploads the bones it uses
	};

public:
	ASkinShaderHAL();
	~ASkinShaderHAL();

	//-----------------------------------------------------------------------------
	// Toshi::TShader
	//-----------------------------------------------------------------------------
	virtual void  Flush() override;
	virtual void  StartFlush() override;
	virtual void  EndFlush() override;
	virtual TBOOL Create() override;
	virtual TBOOL Validate() override;
	virtual void  Invalidate() override;
	virtual TBOOL TryInvalidate() override;
	virtual TBOOL TryValidate() override;
	virtual void  Render( Toshi::TRenderPacket* a_pRenderPacket ) override;

	// Draws the instances collected by Render. The material calls it before it changes the textures
	void FlushInstances();

	//-----------------------------------------------------------------------------
	// ASkinShader
	//-----------------------------------------------------------------------------
	virtual void           EnableRenderEnvMap( TBOOL a_bEnable ) override;
	virtual TBOOL          IsHighEndSkinning() override;
	virtual void           EnableHighEndSkinning( TBOOL a_bEnable ) override;
	virtual TBOOL          IsCapableHighEndSkinning() override;
	virtual TBOOL          IsLightScattering() override;
	virtual void           SetLightScattering( TBOOL a_bEnable ) override;
	virtual TBOOL          IsAlphaBlendMaterial() override;
	virtual void           SetAlphaBlendMaterial( TBOOL a_bIsAlphaBlendMaterial ) override;
	virtual ASkinMaterial* CreateMaterial( const TCHAR* a_szName ) override;
	virtual ASkinMesh*     CreateMesh( const TCHAR* a_szName ) override;
	virtual TINT           AddLight( const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity ) override;
	virtual void           SetLight( TINT a_iIndex, const Toshi::TVector3& a_rPosition, TFLOAT a_fIntensity ) override;
	virtual void           RemoveLight( TINT a_iIndex ) override;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL IsEnableRenderEnvMap();
	virtual void  SetSomeColour( TUINT a_uiR, TUINT a_uiG, TUINT a_uiB, TUINT a_uiA );
	virtual TINT  SetUnknown1( TINT a_Unknown, TUINT8 a_fAlpha );
	virtual void  SetUnknown2( TINT a_Unknown );

	void SetAlphaRef( TINT a_iAlphaRef );
	TINT GetAlphaRef() const { return m_iAlphaRef; }

	Toshi::TOrderTable* GetOrderTable( TUINT a_uiIndex )
	{
		TASSERT( a_uiIndex < NUM_ORDER_TABLES );
		return &m_aOrderTables[ a_uiIndex ];
	}

private:
	void SetStates();

public:
	inline static RENDERSTATE      sm_eRenderStateFlags = RENDERSTATE_ALPHABLEND;
	inline static Toshi::TMatrix44 sm_oWorldViewMatrix;

private:
	GLuint m_uiProgram;
	GLuint m_uiProgramHD;
	GLuint m_uiProgramInstanced;
	GLuint m_uiProgramInstancedHD;

	class ASkinMeshHAL* m_pInstancedMesh;
	TBOOL               m_bInstancedHD;
	TUINT               m_uiNumInstances;
	Toshi::TVector4     m_aInstanceData[ MAX_INSTANCES * INSTANCE_SIZE ];
	TINT                m_iAlphaRef;
	TBOOL               m_bRenderEnvMap;
	Toshi::TOrderTable  m_aOrderTables[ NUM_ORDER_TABLES ];
	TBOOL               m_bHighEndSkinning;
	TBOOL               m_bLightScattering;
	TBOOL               m_bIsAlphaBlendMaterial;
};
