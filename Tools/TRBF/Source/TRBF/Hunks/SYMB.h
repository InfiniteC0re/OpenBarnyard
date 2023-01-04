#pragma once
#include "Hunk.h"

namespace TTool
{
	namespace TRBF
	{
		class SYMB
		{
		public:
			SYMB()
			{
				m_Symbols.reserve(5);
				m_SymbolNames.reserve(5);
			}

			void Add(SECT::Stack* stack, const char* name, void* ptr)
			{
				m_Symbols.emplace_back(stack->GetIndex(), 0, 0, Toshi::TTRB::HashString(name), stack->GetOffset(ptr));
				m_SymbolNames.push_back(name);
			}

			void Write(Toshi::TTSFO& ttsfo)
			{
				uint32_t nameOffset = 0;
				uint32_t symbolCount = m_Symbols.size();
				ttsfo.Write(symbolCount);

				for (size_t i = 0; i < m_Symbols.size(); i++)
				{
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

		private:
			std::vector<Toshi::TTRB::TTRBSymbol> m_Symbols;
			std::vector<std::string> m_SymbolNames;
		};
	}
}