#include "stdafx.h"
#include "ExtendVideoFrameObserver.h"
#include <iostream>

#include "../LibYUV/include/libyuv.h"
#ifdef _M_IX86
#pragma comment(lib,"../LibYUV/lib/X86/libyuv.lib")
#elif defined _M_X64
#pragma comment(lib,"../LibYUV/lib/X64/libyuv.lib")
#endif

CExtendVideoFrameObserver::CExtendVideoFrameObserver()
{
	m_lpImageBuffer = new BYTE[0x800000];
	ZeroMemory(m_lpImageBuffer, 0x800000);
	m_lpImageBufferRemote = new BYTE[0x800000];
	ZeroMemory(m_lpImageBufferRemote, 0x800000);
	m_lpImageBufferRemoteTemp = new BYTE[0x800000];
	ZeroMemory(m_lpImageBufferRemoteTemp, 0x800000);
	m_pFileYUVLocal = NULL;
	m_pFileYUVLocal = fopen("../Extendvideo360.yuv", "rb+");
}

CExtendVideoFrameObserver::~CExtendVideoFrameObserver()
{
	if (m_pFileYUVLocal)
		fclose(m_pFileYUVLocal);
	m_pFileYUVLocal = NULL;
	delete[] m_lpImageBuffer;
	delete[] m_lpImageBufferRemote;
	delete[] m_lpImageBufferRemoteTemp;
}

bool CExtendVideoFrameObserver::onCaptureVideoFrame(VideoFrame& videoFrame)
{
	if (bCallBack) {

		if (NULL == m_pFileYUVLocal)
			return true;
		static int nWidthSrc = videoFrame.width;
		static int nHeightSrc = videoFrame.height;
		if (nWidthSrc != videoFrame.width || nHeightSrc != videoFrame.height)
			return true;

		nWidthSrc = videoFrame.width;
		nHeightSrc = videoFrame.height;
		int nBufferSize = videoFrame.height * videoFrame.width * 3 / 2;
		int nRet = fread(m_lpImageBuffer, 1, nBufferSize, m_pFileYUVLocal);
		if (feof(m_pFileYUVLocal) || nRet < nBufferSize)
			fseek(m_pFileYUVLocal, 0, SEEK_SET);

		if (nRet == 0)
			return true;

		int nWidth = nWidthSrc;
		int nHeight = nHeightSrc;
		unsigned char* pBufferY = (unsigned char*)m_lpImageBuffer;
		int nYStride = nWidthSrc;
		unsigned char* pBufferU = pBufferY + (nYStride * nHeight);
		int nUStride = nYStride / 2;
		unsigned char* pBufferV = pBufferU + nUStride * nHeight / 2;
		int nVStride = nYStride / 2;

		libyuv::I420Rotate(
			pBufferY, nYStride,
			pBufferU, nUStride,
			pBufferV, nVStride,
			(uint8*)videoFrame.yBuffer, videoFrame.yStride,
			(uint8*)videoFrame.uBuffer, videoFrame.uStride,
			(uint8*)videoFrame.vBuffer, videoFrame.vStride, nWidthSrc, nHeightSrc, libyuv::kRotate0);
		videoFrame.type = IVideoFrameObserver::FRAME_TYPE_YUV420;
	}

	return true;
}

bool CExtendVideoFrameObserver::onRenderVideoFrame(unsigned int uid, VideoFrame& videoFrame)
{
	int nWidth = videoFrame.width;
	int nHeight = videoFrame.height;
	int nYStride = videoFrame.yStride;
	int nUStride = videoFrame.uStride;
	int nVStride = videoFrame.vStride;

	memcpy_s(m_lpImageBufferRemote, nYStride * nHeight, videoFrame.yBuffer, nYStride*nHeight);
	memcpy_s(m_lpImageBufferRemote + nYStride * nHeight, nUStride * nHeight / 2, videoFrame.uBuffer, nUStride * nHeight / 2);
	memcpy_s(m_lpImageBufferRemote + nYStride * nHeight + nUStride * nHeight / 2, nVStride * nHeight / 2, videoFrame.vBuffer, nVStride * nHeight / 2);
	int nBufferLen = nYStride * nHeight + nUStride *nHeight / 2 + nVStride *nHeight / 2;

	return true;
}
