#pragma once
#include <Toshi/File/TTRB.h>
#include <Toshi/File/TTSF.h>

namespace TLib
{
	namespace TRBF
	{
		class Hunk
		{
		public:
			void Write(Toshi::TTSFO& ttsfo)
			{
				TOSHI_ERROR("This hunk doesn't support writing");
			}
		};
	}
}