/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PLUGIN_DIRECTSHOW_GRABBER_H_)
#define _COMPV_PLUGIN_DIRECTSHOW_GRABBER_H_

#include "compv/ds/compv_ds_config.h"
#include "compv/ds/compv_ds_graph_capture.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

//
//	CompVDSGrabber
//
COMPV_OBJECT_DECLARE_PTRS(DSGrabber)

class CompVDSGrabber : public ISampleGrabberCB, public CUnknown
{
public:
	CompVDSGrabber();
	virtual ~CompVDSGrabber();

	virtual HRESULT STDMETHODCALLTYPE SampleCB(
		double SampleTime,
		IMediaSample *pSample) override /* Overrides(ISampleGrabberCB)*/;

	virtual HRESULT STDMETHODCALLTYPE BufferCB(
		double SampleTime,
		BYTE *pBuffer,
		long BufferLen) override /* Overrides(ISampleGrabberCB)*/;

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(
		/* [in] */ REFIID riid,
		/* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR *__RPC_FAR *ppvObject) override /*Overrides(IUnknown)*/;

	virtual ULONG STDMETHODCALLTYPE AddRef(void) override /*Overrides(IUnknown)*/;

	virtual ULONG STDMETHODCALLTYPE Release(void) override /*Overrides(IUnknown)*/;

	COMPV_ERROR_CODE start(const std::string& deviceId = "");
	COMPV_ERROR_CODE stop();

private:
	CompVDSGraphCapturePtr m_ptrGraphCapture;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PLUGIN_DIRECTSHOW_GRABBER_H_ */