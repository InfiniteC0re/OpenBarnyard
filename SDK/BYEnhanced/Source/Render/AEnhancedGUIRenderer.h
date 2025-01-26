#pragma once
#include <BYardSDK/AGUI2Transform.h>
#include <Toshi/TSingleton.h>

class AEnhancedGUIRenderer
    : public Toshi::TSingleton<AEnhancedGUIRenderer>
{
public:
	struct Vertex
	{
		Toshi::TVector3 Position;
		TUINT32         Colour;
		Toshi::TVector2 UV;
	};

public:
	AEnhancedGUIRenderer();
	~AEnhancedGUIRenderer();

	void BeginScene();
	void RenderRectangle( const Toshi::TVector2& a, const Toshi::TVector2& b, const Toshi::TVector2& uv1, const Toshi::TVector2& uv2 );
	
	void SetColour( TUINT32 a_uiColour );
	void SetTransforms( AGUI2Transform* a_pTransforms, TINT a_iNumTransforms );

private:
	void InstallHooks();
	void CreateVertexArray();

	inline static Vertex ms_aVertices[ 8 ];

private:
	Toshi::T2CompiledShader m_hVertexShader;
	Toshi::T2CompiledShader m_hFragmentShader;
	Toshi::T2Shader         m_oShaderProgram;

	Toshi::T2VertexArray m_oVertexArray;

	// State
	TUINT32         m_uiColour        = 0;
	AGUI2Transform* m_pTransforms     = TNULL;
	TINT            m_iTransformCount = 0;
};
