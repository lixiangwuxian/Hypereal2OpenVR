#include "FrameEncoder.h"

FrameEncoder::FrameEncoder(HyGraphicsContext* pGraphicsDevivce, ID3D11Device* pD3D11Device,ID3D11DeviceContext* pD3D11DeviceContext)
{
	m_nVsyncCounter = 0;
	m_flLastVsyncTimeInSeconds = SystemTime::GetInSeconds();
	m_pHyGraphicsDevivce = pGraphicsDevivce;
	m_pD3D11Device = pD3D11Device;
	m_pD3D11DeviceContext = pD3D11DeviceContext;
	m_pTextureSem=new std::counting_semaphore<1>(0);
	//m_pProviderSem = new std::counting_semaphore<1>(1);
	m_DispTexDesc.m_uvOffset = HyVec2{ 0.0f, 0.0f };
	m_DispTexDesc.m_uvSize = HyVec2{ 1.0f, 1.0f };
	//std::thread::thread(&FrameEncoder::VsyncLoop,this).detach();
}

void FrameEncoder::NewFrameGo()
{
	//DriverLog("Sending Texture to Screen");
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
		m_pTextureSem->acquire();
		DriverLog("Sending Texture to Screen");
		m_DispTexDesc.m_texture = m_pStagingTexture; 
		m_pStagingTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&m_pKeyedMutex);
		if (m_pKeyedMutex->AcquireSync(0, 10) != S_OK) {
			m_pKeyedMutex->Release();
			continue;
		}
		//DriverLog("AcquireSync Successful!");
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
	//m_pProviderSem->acquire();
	if (m_pStagingTexture == NULL){
		D3D11_TEXTURE2D_DESC srcDesc;
		pTexture->GetDesc(&srcDesc);
		if (FAILED(m_pD3D11Device->CreateTexture2D(&srcDesc, NULL, &m_pStagingTexture))){
			DriverLog("Failed to create staging texture!");
			return false;
		}
	}
	m_pD3D11DeviceContext->CopyResource(m_pStagingTexture, pTexture);
	//m_pStagingTexture = pTexture;
	//DriverLog("Copyed");
	return true;
}
