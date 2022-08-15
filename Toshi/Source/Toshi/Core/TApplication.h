#pragma once

namespace Toshi
{
	class TApplication
	{
	public:
		virtual void Start() = 0;
		virtual void Run() = 0;
	};
	
	TApplication* CreateApplication(int argc, char** argv);
}