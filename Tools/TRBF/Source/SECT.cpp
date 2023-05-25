#include "pch.h"
#include "TRBF/Hunks/SECT.h"
#include "TRBF/Hunks/SYMB.h"

namespace TLib
{
	namespace TRBF
	{
		bool SECT::DeleteStack(SYMB* pSymb, SECT::Stack* pStack)
		{
			auto result = std::find(m_Stacks.begin(), m_Stacks.end(), pStack);

			if (result != m_Stacks.end())
			{
				pSymb->RemoveAllWithStackIndex(pStack->GetIndex());

				m_Stacks.erase(result);
				delete pStack;

				// Update indexes
				uint32_t index = 0;
				for (auto it = m_Stacks.begin(); it != m_Stacks.end(); it++)
				{
					auto stack = *it;
					pSymb->UpdateSymbolsIndexes(stack, index++);
				}

				return true;
			}
			else
			{
				return false;
			}
		}
	}
}