#pragma once

namespace Toshi {

	class TFileLexer
	{
	public:
		enum TOKEN {
			TOKEN_UNKNOWN = 0,
			TOKEN_IDENT = 16,
			TOKEN_STRING = 17,
			TOKEN_COMMENT = 21,
		};

	};

}