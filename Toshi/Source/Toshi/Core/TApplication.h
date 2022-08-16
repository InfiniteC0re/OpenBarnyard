#pragma once
#include "../Strings/TCString.h"
#include <timeapi.h>

namespace Toshi
{
	class TApplication
	{
	public:

		const char * someKindOfKey = "{1B5D8052-529A-4c88-9DDE-EE523D440B00}";

		unsigned int unk1; // 0x4
		unsigned int unk; // 0x10

		virtual void Start() = 0;
		virtual void Run() = 0;

		virtual bool OnCreate();
		virtual bool OnDestroy();
		bool IsCreated();
		HWND UnkFun();
	};
	
	TApplication* CreateApplication(int argc, char** argv);

}