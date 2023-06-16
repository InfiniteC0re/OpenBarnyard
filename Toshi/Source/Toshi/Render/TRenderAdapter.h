#pragma once
#include "Toshi/Core/TNodeList.h"

namespace Toshi
{
	class TRenderAdapter : public TNodeList<TRenderAdapter>::TNode
	{
	public:
		class Mode : public TNodeList<Mode>::TNode
		{
		public:
			class Device : public TNodeList<Device>::TNode
			{
			public:
				Device() = default;
			};

		public:
			Mode() = default;

			virtual ~Mode() = default;

			virtual TRenderAdapter* GetAdapter() const = 0;
			virtual size_t GetModeIndex() const = 0;
			virtual uint32_t GetWidth() const = 0;
			virtual uint32_t GetHeight() const = 0;
			virtual TBOOL SomeCheck1() const = 0;
			virtual TBOOL SomeCheck2() const = 0;
			virtual float GetRefreshRate() const = 0;
			virtual TRenderAdapter::Mode::Device* GetDevice(int device) = 0;
		};

	public:
		TRenderAdapter() = default;

		virtual ~TRenderAdapter() = default;

		TNodeList<Mode>* GetModeList() { return &m_Modes; }
		
	private:
		TNodeList<Mode> m_Modes; // 0x10
	};
}
