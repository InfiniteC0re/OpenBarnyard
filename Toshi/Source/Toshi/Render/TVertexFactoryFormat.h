#pragma once

namespace Toshi {

	struct TVertexStreamFormat
	{
		TUINT16 m_uiVertexSize = 0;
		TUINT m_Unk = 0;
	};

	struct TVertexFactoryFormat
	{
		static constexpr TUINT MAX_NUM_STREAMS = 8;

		TUINT m_uiNumStreams = 0;
		TVertexStreamFormat m_aStreamFormats[MAX_NUM_STREAMS];

		TUINT GetNumStreams() const { return m_uiNumStreams; }
	};

}