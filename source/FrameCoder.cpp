#include "FrameCoder.h"

FrameCoder::FrameCoder(HyGraphicsContext* pGraphicsDevivce, ID3D11Device* pD3D11Device)
{
	m_cLastVsyncTime = clock();
	m_pHyGraphicsDevivce = pGraphicsDevivce;
	m_pD3D11Device = pD3D11Device;
	m_pTextureSem=new std::counting_semaphore<10>(0);
	m_DispTexDesc.m_uvOffset = HyVec2{ 0.0f, 0.0f };
	m_DispTexDesc.m_uvSize = HyVec2{ 1.0f, 1.0f };
	std::thread::thread(&FrameCoder::VsyncLoop,this).detach();
}

void FrameCoder::NewFrameGo(ID3D11Texture2D* newFrame)
{
	mQueue_pTexture.push(newFrame);
	m_pTextureSem->release();
}

clock_t FrameCoder::GetTimeSinceLastVsync()
{
	return clock_t()-m_cLastVsyncTime;
}

void FrameCoder::VsyncLoop()
{
	while (1) {
		m_pTextureSem->acquire();
		m_DispTexDesc.m_texture = mQueue_pTexture.front();
		mQueue_pTexture.pop();
		m_pHyGraphicsDevivce->Submit(0, &m_DispTexDesc, 1);
		m_cLastVsyncTime = clock();
		//delete m_DispTexDesc.m_texture;
	}
}
