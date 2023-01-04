#pragma once
#include "Hunk.h"
#include "SECT.h"

namespace TTool
{
	namespace TRBF
	{
		class HDRX : public Hunk
		{
		public:
			HDRX(TVersion version)
			{
				m_Header.m_ui32Version = version;
				m_Header.m_i32SectionCount = 0;
			}

			void SetVersion(TVersion version)
			{
				m_Header.m_ui32Version = version;
			}

			void SetSectionCount(int32_t count)
			{
				m_Header.m_i32SectionCount = count;
			}

			void Write(Toshi::TTSFO& ttsfo, SECT& sect)
			{
				ttsfo.Write(m_Header);

				for (SECT::Stack* stack : sect)
				{
					Toshi::TTRB::SecInfo sectionInfo = { };
					sectionInfo.m_Size = stack->GetUsedSize();
					ttsfo.Write(sectionInfo);
				}
			}

		private:
			Toshi::TTRB::Header m_Header;
		};
	}
}