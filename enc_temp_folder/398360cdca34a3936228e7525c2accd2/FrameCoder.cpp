#include "FrameCoder.h"

FrameCoder::FrameCoder(HyGraphicsContext* pGraphicsDevivce, ID3D11Device* pD3D11Device,ID3D11DeviceContext* pD3D11DeviceContext)
{
	m_uFramesCount = 0;
	m_cLastVsyncTime = clock();
	m_pHyGraphicsDevivce = pGraphicsDevivce;
	m_pD3D11Device = pD3D11Device;
	m_pD3D11DeviceContext = pD3D11DeviceContext;
	m_pTextureSem=new std::counting_semaphore<1>(0);
	m_pProviderSem = new std::counting_semaphore<1>(1);
	m_DispTexDesc.m_uvOffset = HyVec2{ 0.0f, 0.0f };
	m_DispTexDesc.m_uvSize = HyVec2{ 1.0f, 1.0f };
	std::thread::thread(&FrameCoder::VsyncLoop,this).detach();
}

void FrameCoder::NewFrameGo()
{
	//DriverLog("Pushed New Frame!");
	m_pTextureSem->release(); 
	//m_pHyGraphicsDevivce->Submit(0, &m_DispTexDesc, 1);
	//VsyncLoop();
}

void FrameCoder::GetInfoForNextVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	*pfSecondsSinceLastVsync = (float)(clock_t() - m_cLastVsyncTime) / 1000;
	//*pfSecondsSinceLastVsync -= 0.011;
	*pulFrameCounter = m_uFramesCount + 1;
}

void FrameCoder::VsyncLoop()
{
	DriverLog("Created Vsync Loop");
	while (1) {
		m_pTextureSem->acquire();
		DriverLog("Sending Texture to Screen");
		m_DispTexDesc.m_texture = m_pStagingTexture; 
		m_pStagingTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&m_pKeyedMutex);
		if (m_pKeyedMutex->AcquireSync(0, 50) != S_OK){
			m_pKeyedMutex->Release();
			m_pProviderSem->release();
			DriverLog("Failed to acquire lock");
			continue;
		}
		DriverLog("AcquireSync Successful!");
		m_pHyGraphicsDevivce->Submit(0, &m_DispTexDesc, 1);
		DriverLog("Submited");
		m_cLastVsyncTime = clock();
		//m_uFramesCount++;
		if (m_pKeyedMutex){
			m_pKeyedMutex->ReleaseSync(0);
			m_pKeyedMutex->Release();
			DriverLog("Unlocked");
		}
		m_pProviderSem->release();
	}
}

bool FrameCoder::copyToStaging(ID3D11Texture2D* pTexture)
{
	m_pProviderSem->acquire();
	/*
	if (m_pStagingTexture == nullptr){
		D3D11_TEXTURE2D_DESC srcDesc;
		pTexture->GetDesc(&srcDesc);
		if (FAILED(m_pD3D11Device->CreateTexture2D(&srcDesc, NULL, &m_pStagingTexture))){
			DriverLog("Failed to create staging texture!");
			return false;
		}
	}
	m_pD3D11DeviceContext->CopyResource(m_pStagingTexture, pTexture);*/
	m_pStagingTexture = pTexture;
	DriverLog("Copyed");
	return true;
}
