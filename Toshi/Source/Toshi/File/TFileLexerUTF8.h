#pragma once

namespace Toshi {

	class TFileLexerUTF8 :
		public TGenericClassDerived<TFileLexerUTF8, TObject, "TFileLexerUTF8", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		TFileLexerUTF8();
		~TFileLexerUTF8();

		void Define(const char* a_szDefinition);

	private:
		TGenericEmitter m_oEmitter; // 0x80
		//T2Array
	};

}