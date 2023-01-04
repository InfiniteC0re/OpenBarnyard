#pragma once
#include "Hunk.h"

namespace TLib
{
	namespace TRBF
	{
		class SYMB
		{
		public:
			SYMB()
			{
				Reset();
			}

			void Reset()
			{
				m_Symbols.clear();
				m_SymbolNames.clear();
				m_Symbols.reserve(5);
				m_SymbolNames.reserve(5);
			}

			template <class T>
			SECT::Stack::Ptr<T> Find(SECT& sect, const char* name)
			{
				auto hash = Toshi::TTRB::HashString(name);

				for (size_t i = 0; i < m_Symbols.size(); i++)
				{
					if (m_Symbols[i].NameHash == hash)
					{
						if (m_SymbolNames[i] == name)
						{
							auto stack = sect.GetSection(m_Symbols[i].HDRX);
							return { stack, m_Symbols[i].DataOffset };
						}
					}
				}

				return { TNULL, 0 };
			}

			template <class T>
			SECT::Stack::Ptr<T> Find(SECT* sect, const char* name)
			{
				return Find<T>(*sect, name);
			}

			void Add(SECT::Stack* stack, const char* name, void* ptr)
			{
				m_Symbols.emplace_back(stack->GetIndex(), 0, 0, 0, stack->GetOffset(ptr));
				m_SymbolNames.push_back(name);
			}

			void Write(Toshi::TTSFO& ttsfo)
			{
				TASSERT(m_Symbols.size() == m_SymbolNames.size(), "");

				uint32_t nameOffset = 0;
				uint32_t symbolCount = m_Symbols.size();
				ttsfo.Write(symbolCount);

				for (size_t i = 0; i < m_Symbols.size(); i++)
				{
					m_Symbols[i].NameHash = Toshi::TTRB::HashString(m_SymbolNames[i].c_str());
					m_Symbols[i].NameOffset = nameOffset;
					nameOffset += m_SymbolNames[i].length() + 1;
					ttsfo.Write(m_Symbols[i]);
				}

				for (auto& name : m_SymbolNames)
				{
					ttsfo.WriteRaw(name.c_str(), name.length());
					ttsfo.Write((uint8_t)0);
				}
			}

			void Read(Toshi::TTSFI& ttsfi, SECT& sect)
			{
				uint32_t symbolCount = 0;
				ttsfi.Read(&symbolCount);

				// Read symbols
				size_t symbolsSize = sizeof(Toshi::TTRB::TTRBSymbol) * symbolCount;
				m_Symbols.resize(symbolCount);
				ttsfi.ReadRaw(m_Symbols.data(), symbolsSize);

				// Read symbol names
				size_t namesSize = ttsfi.GetCurrentHunk().Size - symbolsSize;
				m_SymbolNames.reserve(symbolCount);
				char* namesBuffer = new char[namesSize];
				ttsfi.ReadRaw(namesBuffer, namesSize);

				for (auto& symbol : m_Symbols)
				{
					const char* symbolName = &namesBuffer[symbol.NameOffset];
					m_SymbolNames.push_back(symbolName);
				}

				delete[] namesBuffer;
			}

		private:
			std::vector<Toshi::TTRB::TTRBSymbol> m_Symbols;
			std::vector<std::string> m_SymbolNames;
		};
	}
}