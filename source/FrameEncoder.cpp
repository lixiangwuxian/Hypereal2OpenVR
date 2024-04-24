#include "FrameEncoder.h"

FrameEncoder::FrameEncoder(HyGraphicsContext* pGraphicsDevivce, ID3D11Device* pD3D11Device,ID3D11DeviceContext* pD3D11DeviceContext)
{
	m_nVsyncCounter = 0;
	m_flLastVsyncTimeInSeconds = SystemTime::GetInSeconds();
	m_pHyGraphicsDevivce = pGraphicsDevivce;
	m_pD3D11Device = pD3D11Device;
	m_pD3D11DeviceContext = pD3D11DeviceContext;
	m_DispTexDesc.m_uvOffset = HyVec2{ 0.0f, 0.0f };
	m_DispTexDesc.m_uvSize = HyVec2{ 1.0f, 1.0f };
}

void FrameEncoder::NewFrameGo()
{
	m_DispTexDesc.m_texture = m_pStagingTexture;
	m_pStagingTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&m_pKeyedMutex);
	if (m_pKeyedMutex->AcquireSync(0, 10) != S_OK) {
		m_pKeyedMutex->Release();
		//continue;
	}
	//DriverLog("AcquireSync Successful!");
	m_flLastVsyncTimeInSeconds = SystemTime::GetInSeconds();
	m_nVsyncCounter++;
	m_pHyGraphicsDevivce->Submit(0, &m_DispTexDesc, 1);
	if (m_pKeyedMutex) {
		m_pKeyedMutex->ReleaseSync(0);
		m_pKeyedMutex->Release();
		//DriverLog("Unlocked");
	}
	//m_pTextureSem->release();
}

void FrameEncoder::GetInfoForNextVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	*pfSecondsSinceLastVsync = m_flLastVsyncTimeInSeconds;
	*pulFrameCounter = m_nVsyncCounter;
}

void FrameEncoder::VsyncLoop()
{
	DriverLog("Created Vsync Loop");
	while (1) {
		//m_pTextureSem->acquire();
		DriverLog("Sending Texture to Screen");
		m_DispTexDesc.m_texture = m_pStagingTexture; 
		m_pStagingTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&m_pKeyedMutex);
		if (m_pKeyedMutex->AcquireSync(0, 10) != S_OK) {
			m_pKeyedMutex->Release();
			continue;
		}
		m_flLastVsyncTimeInSeconds = SystemTime::GetInSeconds();
		m_nVsyncCounter++;
		m_pHyGraphicsDevivce->Submit(0, &m_DispTexDesc, 1);
		if (m_pKeyedMutex) {
			m_pKeyedMutex->ReleaseSync(0);
			m_pKeyedMutex->Release();
			DriverLog("Unlocked");
		}
		//m_pProviderSem->release();
	}
}

bool FrameEncoder::copyToStaging(ID3D11Texture2D* pTexture)
{
	D3D11_MAPPED_SUBRESOURCE mapped_r = { 0 };
	m_pD3D11DeviceContext->Map(pTexture, 0, D3D11_MAP_READ, 0, &mapped_r);
	D3D11_MAPPED_SUBRESOURCE mapped_w = { 0 };
	D3D11_TEXTURE2D_DESC srcDesc;
	pTexture->GetDesc(&srcDesc);
	D3D11_TEXTURE2D_DESC stagingTextureDesc;
	memcpy(&stagingTextureDesc, &srcDesc, sizeof(D3D11_TEXTURE2D_DESC));
	stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
	stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	if (FAILED(m_pD3D11Device->CreateTexture2D(&stagingTextureDesc, NULL, &m_pStagingTexture)))
	{
		DriverLog("Create m_pFlushTexture failed.");
		return false;
	}
	m_pD3D11DeviceContext->Map(m_pStagingTexture, 0, D3D11_MAP_WRITE, 0, &mapped_w);
	for (int y = 0; y < mapped_r.DepthPitch; y++) {
		memcpy((uint8_t*)mapped_r.pData + mapped_r.RowPitch * y,
			(uint8_t*)mapped_w.pData + mapped_r.RowPitch*y, mapped_r.RowPitch);
	}
	m_pD3D11DeviceContext->Unmap(pTexture,0);
	m_pD3D11DeviceContext->Unmap(m_pStagingTexture, 0);
	return true;
}