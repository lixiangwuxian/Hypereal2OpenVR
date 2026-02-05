#include "FrameEncoder.h"

FrameEncoder::FrameEncoder(HyGraphicsContext* pGraphicsDevice, ID3D11Device* pD3D11Device, ID3D11DeviceContext* pD3D11DeviceContext)
	: m_newFrameReady(false)      // bManualReset = false (auto-reset)
	, m_encodeFinished(false)     // bManualReset = false (auto-reset)
	, m_bExiting(false)
{
	m_flLastVsyncTimeInSeconds = SystemTime::GetInSeconds();
	m_pHyGraphicsDevice = pGraphicsDevice;
	m_pD3D11Device = pD3D11Device;
	m_pD3D11DeviceContext = pD3D11DeviceContext;

	// Initialize texture descriptor
	m_DispTexDesc.m_uvOffset = HyVec2{ 0.0f, 0.0f };
	m_DispTexDesc.m_uvSize = HyVec2{ 1.0f, 1.0f };

	// Set initial signaled state - no encode in progress
	m_encodeFinished.Set();

	// Start encoder thread
	m_encoderThread = std::thread(&FrameEncoder::EncoderThreadFunc, this);
}

FrameEncoder::~FrameEncoder()
{
	Stop();

	if (m_pStagingTexture) {
		m_pStagingTexture->Release();
		m_pStagingTexture = nullptr;
	}
}

void FrameEncoder::Stop()
{
	if (m_encoderThread.joinable()) {
		m_bExiting = true;
		m_newFrameReady.Set();
		m_encoderThread.join();
	}
}

bool FrameEncoder::CopyToStaging(ID3D11Texture2D* pTexture)
{
	if (!pTexture) return false;

	// Create staging texture if needed
	if (m_pStagingTexture == nullptr) {
		D3D11_TEXTURE2D_DESC srcDesc;
		pTexture->GetDesc(&srcDesc);

		D3D11_TEXTURE2D_DESC stagingDesc;
		ZeroMemory(&stagingDesc, sizeof(stagingDesc));
		stagingDesc.Width = srcDesc.Width;
		stagingDesc.Height = srcDesc.Height;
		stagingDesc.Format = srcDesc.Format;
		stagingDesc.MipLevels = 1;
		stagingDesc.ArraySize = 1;
		stagingDesc.SampleDesc.Count = 1;
		stagingDesc.Usage = D3D11_USAGE_DEFAULT;  // GPU accessible for Submit
		stagingDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

		if (FAILED(m_pD3D11Device->CreateTexture2D(&stagingDesc, NULL, &m_pStagingTexture))) {
			DriverLog("FrameEncoder: Failed to create staging texture!");
			return false;
		}
	}

	// Copy the texture
	m_pD3D11DeviceContext->CopyResource(m_pStagingTexture, pTexture);
	return true;
}

void FrameEncoder::NewFrameReady(uint64_t nFrameId, double flVsyncTimeInSeconds)
{
	m_nPendingFrameId = nFrameId;
	m_flPendingVsyncTimeInSeconds = flVsyncTimeInSeconds;
	m_encodeFinished.Reset();
	m_newFrameReady.Set();
}

void FrameEncoder::WaitForEncode()
{
	m_encodeFinished.Wait();
}

void FrameEncoder::GetTimingInfo(double* pflLastVsyncTimeInSeconds, uint64_t* pnVsyncCounter)
{
	*pflLastVsyncTimeInSeconds = m_flLastVsyncTimeInSeconds;
	*pnVsyncCounter = m_nVsyncCounter;
}

void FrameEncoder::EncoderThreadFunc()
{
	DriverLog("FrameEncoder: Encoder thread started");

	// Set high priority for this thread
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

	while (!m_bExiting) {
		// Wait for a new frame
		m_newFrameReady.Wait();

		if (m_bExiting) break;

		if (m_pStagingTexture && m_pHyGraphicsDevice) {
			// Submit to Hypereal
			m_DispTexDesc.m_texture = m_pStagingTexture;

			HyResult result = m_pHyGraphicsDevice->Submit(
				m_nPendingFrameId,
				&m_DispTexDesc,
				1
			);

			if (result == hySuccess) {
				// Update timing info after successful submit
				m_flLastVsyncTimeInSeconds = m_flPendingVsyncTimeInSeconds;
				m_nVsyncCounter++;
			} else {
				DriverLog("FrameEncoder: Submit failed with result %d", result);
			}
		}

		// Signal that encoding is complete
		m_encodeFinished.Set();
	}

	DriverLog("FrameEncoder: Encoder thread exiting");
}
