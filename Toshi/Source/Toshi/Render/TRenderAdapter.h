#pragma once

#include "Toshi/Core/TGenericNodeList.h"

namespace Toshi
{
	class TRenderAdapter : TNodeList<TRenderAdapter>::TNode
	{
		class Mode : TNodeList<Mode>::TNode
		{
			class Device : TNodeList<Device>::TNode
			{

				Device() : TNodeList<Device>::TNode() {}

				Device(const Device& a_device) : TNodeList<Device>::TNode(a_device) { }

			};

			Mode() : TNodeList<Mode>::TNode() {}

			Mode(const Mode& a_mode) : TNodeList<Mode>::TNode(a_mode) { }

		};

		TNodeList<Mode> modes; // 0x10

		TRenderAdapter() : TNodeList<TRenderAdapter>::TNode(), modes() {}

	public:
		inline TNodeList<Mode>* GetModeList() { return &modes; }
		
	};
}
