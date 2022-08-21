#pragma once
namespace Toshi
{
	class TGenericDList
	{

		class TNode
		{
		protected:
			TNode();

			inline TNode* Next() const { return m_pNextNode; }

			TNode* m_pNextNode;
			TNode* m_pPrevNode;
		};
	};


}


