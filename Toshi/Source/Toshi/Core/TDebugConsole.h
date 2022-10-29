#pragma once
#include "Core.h"

namespace Toshi
{
	class TDebugConsole
	{
	public:
		virtual ~TDebugConsole() = default;

		// Returns true if the console was previously visible
		virtual bool Show(bool state) = 0;

		// Returns true if the console became visible
		virtual bool Toggle() = 0;

	public:
		/*
		* Platform specific methods
		* Define them in TDebugConsole_{Platform}.cpp
		*/
		static STL::Scope<TDebugConsole> Create();
	};
}