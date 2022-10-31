

namespace Toshi
{

	class XURXUICanvasData : XURXUIElementData
	{
		TClass* m_txuiCanvasClass;
	public:
		XURXUICanvasData() : XURXUIElementData()
		{
			m_txuiCanvasClass = (TClass*)TClass::Find("TXUICanvas", &TXUIElement::s_Class);
		}
	};

	class TXUICanvas
	{

	};
}