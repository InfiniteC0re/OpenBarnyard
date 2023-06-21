#pragma once
#include "TRBF/Hunks/HDRX.h"
#include "TRBF/Hunks/SECT.h"
#include "TRBF/Hunks/RELC.h"
#include "TRBF/Hunks/SYMB.h"

namespace TLib
{
	namespace TRBF
	{
		class TRBF
		{
		public:
			static constexpr TVersion VERSION = { TMAKEVERSION(1, 1) };

		public:
			TRBF() : m_HDRX(VERSION) { }
			TRBF(const std::string& filepath) : m_HDRX(VERSION) { ReadFromFile(filepath); }

			void Reset()
			{
				m_SECT.Reset();
				m_SYMB.Reset();
			}

			bool ReadFromFile(const std::string& filepath)
			{
				Reset();

				Toshi::TTSFI ttsfi;
				auto pFile = Toshi::TFile::Create(filepath.c_str());
				
				if (pFile && ttsfi.Open(pFile) == Toshi::TTRB::ERROR_OK)
				{
					int32_t leftSize = ttsfi.GetCurrentHunk().Size - 4;

					while (leftSize > sizeof(Toshi::TTSF::Hunk))
					{
						if (ttsfi.ReadHunk() != Toshi::TTRB::ERROR_OK) break;
						leftSize -= ttsfi.GetCurrentHunk().Size + sizeof(Toshi::TTSF::Hunk);

						switch (ttsfi.GetCurrentHunk().Name)
						{
						case TMAKEFOUR("HDRX"):
							m_HDRX.Read(ttsfi, m_SECT);
							break;
						case TMAKEFOUR("SECT"):
							m_SECT.Read(ttsfi);
							break;
						case TMAKEFOUR("SECC"):
							m_SECT.Read(ttsfi, true);
							break;
						case TMAKEFOUR("RELC"):
							m_RELC.Read(ttsfi, m_SECT);
							break;
						case TMAKEFOUR("SYMB"):
							m_SYMB.Read(ttsfi, m_SECT);
							break;
						}

						ttsfi.SkipHunk();
					}

					ttsfi.Close(TFALSE);
					pFile->Destroy();
					return true;
				}

				if (pFile != TNULL)
				{
					ttsfi.Close(TFALSE);
					pFile->Destroy();
				}

				return false;
			}

			void WriteToFile(const std::string& filepath, bool compress = false, Toshi::TTSF::Endianess endianess = Toshi::TTSF::Endianess_Little)
			{
				Toshi::TTSFO ttsfo;
				Toshi::TTSFO::HunkMark mark;
				ttsfo.Create(filepath.c_str(), "TRBF", endianess);

				// HDRX
				ttsfo.OpenHunk(&mark, "HDRX");
				m_HDRX.SetSectionCount(m_SECT.GetStackCount());
				m_HDRX.Write(ttsfo, m_SECT);
				ttsfo.CloseHunk(&mark);

				// SECT
				ttsfo.OpenHunk(&mark, compress ? "SECC" : "SECT");
				m_SECT.Write(ttsfo, compress);
				ttsfo.CloseHunk(&mark);

				// RELC
				ttsfo.OpenHunk(&mark, "RELC");
				m_RELC.Write(ttsfo, m_SECT);
				ttsfo.CloseHunk(&mark);

				// SYMB
				ttsfo.OpenHunk(&mark, "SYMB");
				m_SYMB.Write(ttsfo);
				ttsfo.CloseHunk(&mark);

				ttsfo.Close();
			}

			SYMB* GetSYMB()
			{
				return &m_SYMB;
			}

			SECT* GetSECT()
			{
				return &m_SECT;
			}

		private:
			HDRX m_HDRX;
			SECT m_SECT;
			RELC m_RELC;
			SYMB m_SYMB;
		};
	}
}