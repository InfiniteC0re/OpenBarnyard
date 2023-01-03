#pragma once

#include "Toshi/Core/TGenericNodeList.h"

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
				Device() : TNodeList<Device>::TNode() {}
				Device(const Device& a_device) : TNodeList<Device>::TNode(a_device) { }
			};

		public:
			Mode() : TNodeList<Mode>::TNode() {}
			Mode(const Mode& a_mode) : TNodeList<Mode>::TNode(a_mode) { }
		};

	public:
		TRenderAdapter() : TNodeList<TRenderAdapter>::TNode(), modes() {}

		TNodeList<Mode>* GetModeList() { return &modes; }
		
	private:
		TNodeList<Mode> modes; // 0x10
	};
}
