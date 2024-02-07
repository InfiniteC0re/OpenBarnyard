#pragma once
#include "TResource.h"
#include "TIndexFactoryFormat.h"

namespace Toshi {

	class TIndexPoolResourceInterface;

	class TIndexFactoryResourceInterface : public TResource
	{
	public:
		TDECLARE_CLASS(TResource);

	public:
		friend TIndexPoolResourceInterface;

	public:
		TIndexFactoryResourceInterface();

		virtual TBOOL Create(TIndexFactoryFormat* a_pIndexFormat, TUINT16 a_uiMaxStaticIndices, TUINT32 a_uiFlags);
		virtual TIndexPoolResourceInterface* CreatePoolResource(TUINT16 a_uiMaxStaticIndices, TUINT16 a_uiFlags) = 0;

		const TIndexFactoryFormat& GetIndexFormat() const { return m_IndexFormat; }
		TUINT16 GetMaxStaticIndices() const { return m_uiMaxStaticIndices; }

	protected:
		TIndexPoolResourceInterface* CreatePool(TUINT16 a_uiMaxStaticIndices, TUINT32 a_uiFlags);

	protected:
		TUINT m_uiNumPools;
		TIndexFactoryFormat m_IndexFormat;
		TUINT16 m_uiMaxStaticIndices;
		TUINT m_uiFlags;
	};

}
