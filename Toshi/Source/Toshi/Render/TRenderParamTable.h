#pragma once
#include "Toshi/Strings/TStringTable.h"
#include "Toshi/Render/TTexture.h"

namespace Toshi
{
	class TRenderParamTable
	{
	public:
		enum V4PARAM
		{
			V4PARAM_AMBIENTCOLOUR = 1,
			V4PARAM_DIRLIGHT0,
			V4PARAM_DIRLIGHT0COLOUR,
			V4PARAM_DIRLIGHT1,
			V4PARAM_DIRLIGHT1COLOUR,
			V4PARAM_DIRLIGHT2,
			V4PARAM_DIRLIGHT2COLOUR,
			V4PARAM_EYEPOSMS,
			V4PARAM_LIGHTSCOUNTS,
			V4PARAM_POINTLIGHT0,
			V4PARAM_POINTLIGHT0COLOUR,
			V4PARAM_POINTLIGHT0PARAMS,
			V4PARAM_POINTLIGHT1,
			V4PARAM_POINTLIGHT1COLOUR,
			V4PARAM_POINTLIGHT1PARAMS,
			V4PARAM_POINTLIGHT2,
			V4PARAM_POINTLIGHT2COLOUR,
			V4PARAM_POINTLIGHT2PARAMS,
			V4PARAM_POINTLIGHT3,
			V4PARAM_POINTLIGHT3COLOUR,
			V4PARAM_POINTLIGHT3PARAMS,
			V4PARAM_SHADOWLIGHTDIR0,
			V4PARAM_SPOTLIGHT0,
			V4PARAM_SPOTLIGHT0COLOUR,
			V4PARAM_SPOTLIGHT0DIR,
			V4PARAM_SPOTLIGHT0PARAMS,
			V4PARAM_SPOTLIGHT1,
			V4PARAM_SPOTLIGHT1COLOUR,
			V4PARAM_SPOTLIGHT1DIR,
			V4PARAM_SPOTLIGHT1PARAMS,
			V4PARAM_SPOTLIGHT2,
			V4PARAM_SPOTLIGHT2COLOUR,
			V4PARAM_SPOTLIGHT2DIR,
			V4PARAM_SPOTLIGHT2PARAMS,
			V4PARAM_DUMMY,
			V4PARAM_NUMOF,
		};

		enum M44PARAM
		{
			M44PARAM_MODELVIEW = 1,
			M44PARAM_MODELVIEWPROJ,
			M44PARAM_MODELWORLD,
			M44PARAM_PROJ,
			M44PARAM_DUMMY,
			M44PARAM_NUMOF,
		};

		enum TEXPARAM
		{
			TEXPARAM_CUBEENVMAP = 1,
			TEXPARAM_FRAMEBUF,
			TEXPARAM_LIGHTFALLOFF,
			TEXPARAM_PLANARENVMAP,
			TEXPARAM_SHADOWTEX,
			TEXPARAM_DUMMY,
			TEXPARAM_NUMOF
		};

	public:
		static constexpr const char* s_V4ParamNames[] = {
			"",
			"tambcolour",
			"tdirlight0",
			"tdirlight0col",
			"tdirlight1",
			"tdirlight1col",
			"tdirlight2",
			"tdirlight2col",
			"teyeposms",
			"tlightscounts",
			"tpointlight0",
			"tpointlight0col",
			"tpointlight0params",
			"tpointlight1",
			"tpointlight1col",
			"tpointlight1params",
			"tpointlight2",
			"tpointlight2col",
			"tpointlight2params",
			"tpointlight3",
			"tpointlight3col",
			"tpointlight3params",
			"tshadowlightdir0",
			"tspotlight0",
			"tspotlight0col",
			"tspotlight0dir",
			"tspotlight0param",
			"tspotlight1",
			"tspotlight1col",
			"tspotlight1dir",
			"tspotlight1param",
			"tspotlight2",
			"tspotlight2col",
			"tspotlight2dir",
			"tspotlight2param",
			"{"
		};

		static constexpr const char* s_M44ParamNames[] = {
			"",
			"tmodelview",
			"tmodelviewproj",
			"tmodelworld",
			"tproj",
			"{"
		};

		static constexpr const char* s_TexParamNames[] = {
			"",
			"tcubeenvmap",
			"tframebuftex",
			"tlightfalloffmap",
			"tplanarenvmap",
			"tshadowtex",
			"{"
		};

	public:
		TRenderParamTable() :
			m_V4Table(s_V4ParamNames, V4PARAM_NUMOF),
			m_M44Table(s_M44ParamNames, M44PARAM_NUMOF),
			m_TexTable(s_TexParamNames, TEXPARAM_NUMOF)
		{
			m_iNumV4Params = V4PARAM_NUMOF;
			m_iNumMatrixParams = M44PARAM_NUMOF;
			m_iNumTexParams = TEXPARAM_NUMOF;

			m_Vectors = new TVector4[V4PARAM_NUMOF];
			m_Matrices = new TMatrix44[M44PARAM_NUMOF];
			m_Textures = new TTexture*[TEXPARAM_NUMOF];

			for (size_t i = 0; i < TEXPARAM_NUMOF; i++)
			{
				m_Textures[i] = TNULL;
			}

			TASSERT(V4PARAM_AMBIENTCOLOUR == FindParameterV4Handle("tambcolour"));
			TASSERT(V4PARAM_SHADOWLIGHTDIR0 == FindParameterV4Handle("tshadowlightdir0"));
			TASSERT(V4PARAM_DIRLIGHT0 == FindParameterV4Handle("tdirlight0"));
			TASSERT(V4PARAM_DIRLIGHT1 == FindParameterV4Handle("tdirlight1"));
			TASSERT(V4PARAM_DIRLIGHT2 == FindParameterV4Handle("tdirlight2"));
			TASSERT(V4PARAM_DIRLIGHT0COLOUR == FindParameterV4Handle("tdirlight0col"));
			TASSERT(V4PARAM_DIRLIGHT1COLOUR == FindParameterV4Handle("tdirlight1col"));
			TASSERT(V4PARAM_DIRLIGHT2COLOUR == FindParameterV4Handle("tdirlight2col"));
			TASSERT(V4PARAM_EYEPOSMS == FindParameterV4Handle("teyeposms"));
		
			SetParameterV4(V4PARAM_DIRLIGHT0, { 0.47f, -0.74f, 0.47f, 0.0f });
			SetParameterV4(V4PARAM_DIRLIGHT1, { 1.00f, 1.00f, 0.73f, 0.0f });
			m_Vectors[4].w = 0;
			m_Vectors[5].w = 0;
			m_Vectors[6].w = 0;
			m_Vectors[7].w = 0;

			TASSERT(V4PARAM_SPOTLIGHT0 == FindParameterV4Handle("tspotlight0"));
			TASSERT(V4PARAM_SPOTLIGHT1 == FindParameterV4Handle("tspotlight1"));
			TASSERT(V4PARAM_SPOTLIGHT0COLOUR == FindParameterV4Handle("tspotlight0col"));
			TASSERT(V4PARAM_SPOTLIGHT1COLOUR == FindParameterV4Handle("tspotlight1col"));
			TASSERT(V4PARAM_SPOTLIGHT0DIR == FindParameterV4Handle("tspotlight0dir"));
			TASSERT(V4PARAM_SPOTLIGHT1DIR == FindParameterV4Handle("tspotlight1dir"));
			TASSERT(V4PARAM_SPOTLIGHT0PARAMS == FindParameterV4Handle("tspotlight0param"));
			TASSERT(V4PARAM_SPOTLIGHT1PARAMS == FindParameterV4Handle("tspotlight1param"));
			TASSERT(V4PARAM_POINTLIGHT0 == FindParameterV4Handle("tpointlight0"));
			TASSERT(V4PARAM_POINTLIGHT1 == FindParameterV4Handle("tpointlight1"));
			TASSERT(V4PARAM_POINTLIGHT2 == FindParameterV4Handle("tpointlight2"));
			TASSERT(V4PARAM_POINTLIGHT3 == FindParameterV4Handle("tpointlight3"));
			TASSERT(V4PARAM_POINTLIGHT0COLOUR == FindParameterV4Handle("tpointlight0col"));
			TASSERT(V4PARAM_POINTLIGHT1COLOUR == FindParameterV4Handle("tpointlight1col"));
			TASSERT(V4PARAM_POINTLIGHT2COLOUR == FindParameterV4Handle("tpointlight2col"));
			TASSERT(V4PARAM_POINTLIGHT3COLOUR == FindParameterV4Handle("tpointlight3col"));
			TASSERT(V4PARAM_POINTLIGHT0PARAMS == FindParameterV4Handle("tpointlight0params"));
			TASSERT(V4PARAM_POINTLIGHT1PARAMS == FindParameterV4Handle("tpointlight1params"));
			TASSERT(V4PARAM_POINTLIGHT2PARAMS == FindParameterV4Handle("tpointlight2params"));
			TASSERT(V4PARAM_POINTLIGHT3PARAMS == FindParameterV4Handle("tpointlight3params"));
			TASSERT(V4PARAM_LIGHTSCOUNTS == FindParameterV4Handle("tlightscounts"));

			TASSERT(M44PARAM_MODELVIEW == FindParameterM44Handle("tmodelview"));
			TASSERT(M44PARAM_MODELWORLD == FindParameterM44Handle("tmodelworld"));
			TASSERT(M44PARAM_MODELVIEWPROJ == FindParameterM44Handle("tmodelviewproj"));
			TASSERT(M44PARAM_PROJ == FindParameterM44Handle("tproj"));

			TASSERT(TEXPARAM_CUBEENVMAP == FindParameterTexHandle("tcubeenvmap"));
			TASSERT(TEXPARAM_FRAMEBUF == FindParameterTexHandle("tframebuftex"));
			TASSERT(TEXPARAM_LIGHTFALLOFF == FindParameterTexHandle("tlightfalloffmap"));
			TASSERT(TEXPARAM_PLANARENVMAP == FindParameterTexHandle("tplanarenvmap"));
			TASSERT(TEXPARAM_SHADOWTEX == FindParameterTexHandle("tshadowtex"));
		}

		~TRenderParamTable()
		{
			TFree(m_Vectors);
			TFree(m_Matrices);
			TFree(m_Textures);
		}

		int FindParameterV4Handle(const char* paramName)
		{
			return m_V4Table.GetStringID(paramName);
		}

		int FindParameterM44Handle(const char* paramName)
		{
			return m_M44Table.GetStringID(paramName);
		}

		int FindParameterTexHandle(const char* paramName)
		{
			return m_TexTable.GetStringID(paramName);
		}

		void SetParameterV4(int handle, const TVector4& vec4)
		{
			TASSERT(handle > 0 && handle < m_iNumV4Params);
			m_Vectors[handle] = vec4;
		}

		void SetParameterM44(int handle, const TMatrix44& mat44)
		{
			TASSERT(handle > 0 && handle < m_iNumMatrixParams);
			m_Matrices[handle] = mat44;
		}

		void SetParameterTex(int handle, TTexture* tex)
		{
			TASSERT(handle > 0 && handle < m_iNumTexParams);
			m_Textures[handle] = tex;
		}

	private:
		TVector4* m_Vectors;          // 0x00
		TMatrix44* m_Matrices;        // 0x04
		TTexture** m_Textures;        // 0x08
		TStringTable m_V4Table;       // 0x0C
		TStringTable m_M44Table;      // 0x14
		TStringTable m_TexTable;      // 0x1C
		int m_iNumV4Params;        // 0x24
		int m_iNumMatrixParams;    // 0x28
		int m_iNumTexParams;       // 0x2C
	};
}