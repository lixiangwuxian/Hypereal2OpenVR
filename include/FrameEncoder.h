#include"Hypereal_VR.h"
#include"driverlog.h"
#include <d3d11.h>
#include <ctime>
#include<thread>
#include<semaphore>
#include<queue>
//#include"../source/ScreenGrab/sgb.h"
#include <systemtime.h>

class FrameEncoder {
public:
	FrameEncoder(HyGraphicsContext* pGraphicsDevivce, ID3D11Device* pD3D11Device, ID3D11DeviceContext* pD3D11DeviceContext);
	void NewFrameGo();
	void GetInfoForNextVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter);
	void VsyncLoop();
	bool copyToStaging(ID3D11Texture2D* pTexture);
private:
	IDXGIKeyedMutex* m_pKeyedMutex=nullptr;
	ID3D11Texture2D* m_pStagingTexture=nullptr;
	ID3D11Device* m_pD3D11Device = nullptr;
	ID3D11DeviceContext* m_pD3D11DeviceContext = nullptr;
	HyTextureDesc m_DispTexDesc;
	HyGraphicsContext* m_pHyGraphicsDevivce = nullptr;
	std::counting_semaphore<1>* m_pTextureSem = nullptr;
	//std::counting_semaphore<1>* m_pProviderSem = nullptr;
	std::queue<ID3D11Texture2D*> mQueue_pTexture;
	//clock_t m_cLastVsyncTime;
	uint64_t m_nVsyncCounter;
	double m_flLastVsyncTimeInSeconds;
};
