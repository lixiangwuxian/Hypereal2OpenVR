#include"Hypereal_VR.h"
#include <d3d11.h>
#include <ctime>
#include<thread>
#include<semaphore>
#include<queue>
#include"../source/ScreenGrab/sgb.h"

class FrameCoder {
public:
	FrameCoder(HyGraphicsContext* pGraphicsDevivce, ID3D11Device* pD3D11Device);
	void NewFrameGo(ID3D11Texture2D* newFrame);
	clock_t GetTimeSinceLastVsync();
	void VsyncLoop();
private:
	ID3D11Device* m_pD3D11Device = nullptr;
	HyTextureDesc m_DispTexDesc;
	HyGraphicsContext* m_pHyGraphicsDevivce = nullptr;
	std::counting_semaphore<10>* m_pTextureSem = nullptr;
	std::queue<ID3D11Texture2D*> mQueue_pTexture;
	clock_t m_cLastVsyncTime;
};
