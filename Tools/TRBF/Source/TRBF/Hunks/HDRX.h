#pragma once
#include "Hunk.h"
#include "SECT.h"

namespace TLib
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

			void Read(Toshi::TTSFI& ttsfi, SECT& sect)
			{
				ttsfi.Read(&m_Header);

				for (size_t i = 0; i < m_Header.m_i32SectionCount; i++)
				{
					Toshi::TTRB::SecInfo sectionInfo;
					ttsfi.Read(&sectionInfo);
					
					auto stack = sect.CreateSection();
					stack->SetExpectedSize(sectionInfo.m_Size);
				}
			}

		private:
			Toshi::TTRB::Header m_Header;
		};
	}
}