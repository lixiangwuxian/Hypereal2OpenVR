#pragma once
#include "Hypereal_VR.h"
#include "driverlog.h"
#include "threadtools.h"
#include <d3d11.h>
#include <ctime>
#include <thread>
#include <atomic>
#include <systemtime.h>

class FrameEncoder {
public:
	FrameEncoder(HyGraphicsContext* pGraphicsDevice, ID3D11Device* pD3D11Device, ID3D11DeviceContext* pD3D11DeviceContext);
	~FrameEncoder();

	// Copy texture to staging buffer (called from Present)
	bool CopyToStaging(ID3D11Texture2D* pTexture);

	// Signal that a new frame is ready for encoding (called from WaitForPresent)
	void NewFrameReady(uint64_t nFrameId, double flVsyncTimeInSeconds);

	// Wait for the previous encode to finish (called from Present before accessing shared resources)
	void WaitForEncode();

	// Get timing info for VSync calculation
	void GetTimingInfo(double* pflLastVsyncTimeInSeconds, uint64_t* pnVsyncCounter);

	// Stop the encoder thread
	void Stop();

private:
	void EncoderThreadFunc();

	// Thread management
	std::thread m_encoderThread;
	std::atomic<bool> m_bExiting;
	CThreadEvent m_newFrameReady;
	CThreadEvent m_encodeFinished;

	// D3D resources
	ID3D11Device* m_pD3D11Device = nullptr;
	ID3D11DeviceContext* m_pD3D11DeviceContext = nullptr;
	ID3D11Texture2D* m_pStagingTexture = nullptr;

	// Hypereal resources
	HyGraphicsContext* m_pHyGraphicsDevice = nullptr;
	HyTextureDesc m_DispTexDesc;

	// Timing
	double m_flLastVsyncTimeInSeconds = 0.0;
	double m_flPendingVsyncTimeInSeconds = 0.0;
	uint64_t m_nVsyncCounter = 0;
	uint64_t m_nPendingFrameId = 0;

	// Frame interval (90Hz = 1/90 seconds)
	static constexpr double kFrameIntervalInSeconds = 1.0 / 90.0;
};
