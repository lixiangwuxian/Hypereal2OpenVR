#include"Hypereal_VR.h"
#include <d3d11.h>
#include <ctime>

class FrameCoder {
public:
	void NewFrameGo(ID3D11Texture2D* newFrame);
	int GetTimeSinceLastVsnc();
	void VsyncLoop();
private:
	HyDevice* m_pDevivce;
	ID3D11Texture2D* m_pTexture;
	clock_t m_cLastVsyncTime;
};
