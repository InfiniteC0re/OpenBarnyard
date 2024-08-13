#pragma once
#include "Toshi/TNodeList.h"

TOSHI_NAMESPACE_START

class TString8;

class TRenderAdapter : public TNodeList<TRenderAdapter>::TNode
{
public:
	class Mode : public TNodeList<Mode>::TNode
	{
	public:
		class Device : public TNodeList<Device>::TNode
		{
		public:
			Device()		  = default;
			virtual ~Device() = default;

			virtual Mode*		 GetMode() const										 = 0;
			virtual TUINT32		 GetDeviceIndex() const									 = 0;
			virtual const TCHAR* GetTypeString() const									 = 0;
			virtual TBOOL		 IsSoftware() const										 = 0;
			virtual TBOOL		 CanRenderWindowed() const								 = 0;
			virtual TBOOL		 SupportsHardwareTransfomations() const					 = 0;
			virtual TBOOL		 IsDepthStencilFormatSupported( TUINT32 a_iIndex ) const = 0;
		};

	public:
		Mode() = default;

		virtual ~Mode() = default;

		virtual TRenderAdapter*				  GetAdapter() const			 = 0;
		virtual size_t						  GetModeIndex() const			 = 0;
		virtual TUINT32						  GetWidth() const				 = 0;
		virtual TUINT32						  GetHeight() const				 = 0;
		virtual TBOOL						  Is32Bit() const				 = 0;
		virtual TBOOL						  Is16Bit() const				 = 0;
		virtual TUINT32						  GetRefreshRate() const		 = 0;
		virtual TRenderAdapter::Mode::Device* GetDevice( TUINT32 a_iDevice ) = 0;
	};

public:
	TRenderAdapter()		  = default;
	virtual ~TRenderAdapter() = default;

	// TODO: complete vftable
	virtual TUINT32			GetAdapterIndex() const		   = 0;
	virtual const TString8& GetDriver() const			   = 0;
	virtual const TString8& GetDriverDescription() const   = 0;
	virtual TUINT16			GetProductID() const		   = 0;
	virtual TUINT16			GetVersion() const			   = 0;
	virtual TUINT16			GetSubVersion() const		   = 0;
	virtual TUINT16			GetBuild() const			   = 0;
	virtual const GUID&		GetDeviceIdentifier() const	   = 0;
	virtual TUINT32			GetNumSupportedDevices() const = 0;

	TNodeList<Mode>* GetModeList() { return &m_Modes; }

private:
	TNodeList<Mode> m_Modes; // 0x10
};

TOSHI_NAMESPACE_END
