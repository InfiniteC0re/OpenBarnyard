#pragma once
#include "Hunk.h"
#include "SECT.h"

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

			bool Is(int index, const char* name)
			{
				TASSERT(index >= 0 && index < m_Symbols.size());
				auto hash = Toshi::TTRB::HashString(name);

				if (m_Symbols[index].NameHash == hash)
				{
					if (m_SymbolNames[index] == name)
					{
						return true;
					}
				}

				return false;
			}

			int FindIndex(SECT& sect, const char* name)
			{
				auto hash = Toshi::TTRB::HashString(name);

				for (size_t i = 0; i < m_Symbols.size(); i++)
				{
					if (m_Symbols[i].NameHash == hash)
					{
						if (m_SymbolNames[i] == name)
						{
							return i;
						}
					}
				}

				return -1;
			}

			int FindIndex(SECT* sect, const char* name)
			{
				return FindIndex(*sect, name);
			}

			template <class T>
			SECT::Stack::Ptr<T> Find(SECT& sect, const char* name)
			{
				int index = FindIndex(sect, name);

				if (index != -1)
				{
					auto stack = sect.GetStack(m_Symbols[index].HDRX);
					return { stack, m_Symbols[index].DataOffset };
				}

				return { TNULL, (size_t)0 };
			}

			template <class T>
			SECT::Stack::Ptr<T> Find(SECT* sect, const char* name)
			{
				return Find<T>(*sect, name);
			}

			template <class T>
			SECT::Stack::Ptr<T> GetByIndex(SECT& sect, int index)
			{
				TASSERT(index >= 0 && index < m_Symbols.size());

				return {
					sect.GetStack(m_Symbols[index].HDRX),
					m_Symbols[index].DataOffset
				};
			}

			template <class T>
			SECT::Stack::Ptr<T> GetByIndex(SECT* sect, int index)
			{
				return GetByIndex<T>(*sect, index);
			}

			size_t GetCount()
			{
				return m_Symbols.size();
			}

			void Add(SECT::Stack* pStack, const char* name, void* ptr)
			{
				m_Symbols.emplace_back(pStack->GetIndex(), 0, 0, 0, pStack->GetOffset(ptr));
				m_SymbolNames.push_back(name);
			}

			void UpdateSymbolsIndexes(SECT::Stack* pStack, uint32_t newIndex)
			{
				auto stackIndex = pStack->GetIndex();

				for (size_t i = 0; i < m_Symbols.size(); i++)
				{
					if (m_Symbols[i].HDRX == stackIndex)
					{
						m_Symbols[i].HDRX = newIndex;
					}
				}
				
				pStack->SetIndex(newIndex);
			}

			void Remove(int index)
			{
				TASSERT(index >= 0 && index < m_Symbols.size());

				m_Symbols.erase(m_Symbols.begin() + index);
				m_SymbolNames.erase(m_SymbolNames.begin() + index);
			}

			void RemoveAllWithStackIndex(int stackIndex)
			{
				for (size_t i = 0; i < m_Symbols.size();)
				{
					if (m_Symbols[i].HDRX == stackIndex)
					{
						Remove(i);
					}
					else
					{
						i++;
					}
				}
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

			std::vector<std::string>::iterator begin()
			{
				return m_SymbolNames.begin();
			}

			std::vector<std::string>::iterator end()
			{
				return m_SymbolNames.end();
			}

		private:
			std::vector<Toshi::TTRB::TTRBSymbol> m_Symbols;
			std::vector<std::string> m_SymbolNames;
		};
	}
}