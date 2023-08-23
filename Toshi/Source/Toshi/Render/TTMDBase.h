#pragma once

namespace Toshi {

	namespace TTMDBase {

		struct SkeletonHeader
		{
			const char* m_pTKLName;
			int m_iTKeyCount;
			int m_iQKeyCount;
			int m_iSKeyCount;
			int m_iTBaseIndex;
			int m_iQBaseIndex;
			int m_iSBaseIndex;
		};

	}

}