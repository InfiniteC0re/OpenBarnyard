#pragma once
#include "TTexture_DX11.h"

namespace Toshi
{
	class TPrimShader : public TSingleton<TPrimShader>
	{
	public:
		static constexpr size_t ONE_VERTEX_BUFFER_SIZE = 40000;

		struct Mesh
		{
			UINT uiStartPrim;
			UINT uiNumPrims;
		};

		struct Vertex
		{
			TVector3 Position;
			UINT Color;
			TVector2 UV;
		};

		typedef UINT PrimType;
		enum PrimType_ : PrimType
		{
			PrimType_TriangleStrip,
			PrimType_TriangleList,
			PrimType_LineStrip,
			PrimType_LineList,
		};

		typedef UINT BlendMode;

	public:
		TPrimShader();
		~TPrimShader();

		void Initialize();
		void Destroy();

		bool AddMesh();
		void AddVert();

		void Unlock();
		void StartRendering(PrimType primType);
		void StopRendering();

		void SetBlendModeAndUnlock(BlendMode blendMode)
		{
			if (m_eBlendMode != blendMode && (m_eBlendMode = blendMode, m_bIsLocked))
			{
				Unlock();
			}
		}

		void SetImageAndUnlock(TTexture* pTexture)
		{
			if (m_pImage != pTexture && (m_pImage = pTexture, m_bIsLocked))
			{
				Unlock();
			}
		}

		Mesh* GetCurrentMesh()
		{
			TASSERT(m_uiNumUsedMeshes - 1 < m_uiNumMeshes);
			return &m_pMeshes[m_uiNumUsedMeshes - 1];
		}

		Vertex* GetCurrentVertex()
		{
			return m_pCurrentVertex;
		}

		ID3D11VertexShader* GetVertexShader() const
		{
			return m_pVShader;
		}

		ID3D11PixelShader* GetPixelBasicShader() const
		{
			return m_pPShaderBasic;
		}

		ID3D11PixelShader* GetPixelImageShader() const
		{
			return m_pPShaderImage;
		}

		ID3D11InputLayout* GetInputLayout() const
		{
			return m_pInputLayout;
		}

	private:
		void DrawPrim(PrimType primType, UINT numPrim, UINT startPrim);
	
	private:
		Mesh* m_pMeshes;
		UINT m_uiNumMeshes;
		UINT m_uiNumUsedMeshes;
		UINT m_uiNumDrawnFrames;
		UINT m_uiNumUsedVerts;
		Vertex* m_pCurrentVertex;
		bool m_bIsLocked;
		bool m_bIsRendering;
		PrimType m_ePrimType;
		BlendMode m_eBlendMode;
		TTexture* m_pImage;
		ID3D11VertexShader* m_pVShader;
		ID3D11PixelShader* m_pPShaderImage;
		ID3D11PixelShader* m_pPShaderBasic;
		ID3D11InputLayout* m_pInputLayout;
		ID3D11Buffer* m_pBuffer;
	};
}