#pragma once
#include "Toshi/Shaders/SysShader/TSysShader.h"
#include "TSysMaterialHAL_DX11.h"
#include "TSysMeshHAL_DX11.h"

#include <d3d11.h>

namespace Toshi {
	
	class TSysShaderHAL :
		public TGenericClassDerived<TSysShaderHAL, TSysShader, "TSysShaderHAL", TMAKEVERSION(1, 0), false>
	{
	public:
		struct OrderTablePriorities
		{
			uint32_t Priority1;
			uint32_t Priority2;
			uint32_t Priority3;
			uint32_t Priority4;
			uint32_t Priority5;
			uint32_t Priority6;
			uint32_t Priority7;
		};

	public:
		TSysShaderHAL(OrderTablePriorities* priorities);
		~TSysShaderHAL();

		virtual void OnDestroy() override;
		virtual void Flush() override;
		virtual void StartFlush() override;
		virtual void EndFlush() override;
		virtual TBOOL Create() override;
		virtual TBOOL Validate() override;
		virtual void Invalidate() override;
		virtual void Render(TRenderPacket* pRenderPacket) override;
		virtual TSysMaterialHAL* CreateMaterial();
		virtual TSysMeshHAL* CreateMesh(const char* name);

	private:
		TOrderTable m_OrderTables[6];
		OrderTablePriorities m_Priorities;
		ID3D11VertexShader* m_pVertexShader;
		ID3D11PixelShader* m_pPixelShader;
		ID3D11PixelShader* m_pPixelShaderTexture;
		ID3D11PixelShader* m_pPixelShaderAlphaTexture;
		ID3D11InputLayout* m_pInputLayout;
	};

}