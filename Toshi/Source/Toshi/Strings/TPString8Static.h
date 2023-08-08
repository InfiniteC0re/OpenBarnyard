#pragma once
#include "TPString8.h"

#include <string>

namespace Toshi {

	// This is a custom implementation made with the use of modern C++,
	// but it acts exactly the same as it does in Barnyard.
	// The only difference is that originally it's possible to create
	// two or even more static pools but the feature is not used since
	// only one pool is created.

	// Use this macro to get predefined string that is stored in the s_aStrings array.
	// The macro looks for the string at compile time so it doesn't take any execution time.
	// Before TPS8 is allowed to use, TPString8StaticPool::Undump should be called.
#define TPS8(STR) Toshi::TPString8StaticPool::Find(#STR)

	class TPString8StaticPool
	{
	public:
		inline constexpr static const char* s_aStrings[] =
		{
			"walk_lp",
			"run_lp",
			"sprint_lp",
			// ...
		};

		inline constexpr static size_t NumOfStrings = sizeof(s_aStrings) / sizeof(*s_aStrings);
		inline static TPString8 s_apPooledStrings[NumOfStrings];

	public:
		// Creates TPString8 in a_pPool for every defined string
		static void Undump(TPString8Pool* a_pPool)
		{
			for (size_t i = 0; i < NumOfStrings; i++)
			{
				s_apPooledStrings[i].SetPooledString(TNULL);
			}

			for (size_t i = 0; i < NumOfStrings; i++)
			{
				TPString8 string(a_pPool, s_aStrings[i]);

				if (string.GetPooledString() &&
					string.GetPooledString() != s_apPooledStrings[i].GetPooledString())
				{
					s_apPooledStrings[i] = string;
				}
			}
		}

		// Returns pointer to TPString8 if it is stored statically.
		// Executes at compile time and doesn't take any execution time.
		static consteval const TPString8* Find(std::string_view a_String)
		{
			auto it = std::find(s_aStrings, s_aStrings + NumOfStrings, a_String);

			if (it != std::end(s_aStrings))
				return &s_apPooledStrings[std::distance(s_aStrings, it)];
		}
	};

}
