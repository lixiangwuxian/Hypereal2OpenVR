#pragma once
#include<openvr_driver.h>
#include"Hypereal_VR.h"
#include<string.h>
#include"driverlog.h"
#include<d3d11.h>
#include<time.h>


struct SharedTextureEntry_t
{
	HANDLE m_hSharedTexture;
	ID3D11Texture2D* m_pTexture;
};

typedef std::vector< SharedTextureEntry_t > SharedTextures_t;
using namespace vr;
class HyHMD:public ITrackedDeviceServerDriver, public IVRDisplayComponent, public IVRVirtualDisplay
{
public:
	HyHMD(std::string id, HyDevice* Device);
	~HyHMD();
	virtual EVRInitError Activate(uint32_t unObjectId);
	virtual void Deactivate();
	virtual void EnterStandby();

	virtual void* GetComponent(const char* pchComponentNameAndVersion);
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);

	virtual DriverPose_t GetPose();

	PropertyContainerHandle_t GetPropertyContainer();

	std::string GetSerialNumber();

	void UpdatePose(HyTrackingState ctrData);

	//IVRdisplaycomponent
	
	virtual void GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight);
	virtual bool IsDisplayOnDesktop();
	virtual bool IsDisplayRealDisplay();
	virtual void GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight);
	virtual void GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight);
	virtual void GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom);
	virtual DistortionCoordinates_t ComputeDistortion(EVREye eEye, float fU, float fV);
	
	//IVRvirtualdisplay

	virtual void Present(const PresentInfo_t* pPresentInfo, uint32_t unPresentInfoSize);
	virtual void WaitForPresent();
	virtual bool GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter);

	//own public
	volatile uint32_t* getFrameIDptr() {
		return (uint32_t*) & m_nFrameCounter;
	}
	

private:
	void initDisplayConfig();
	void viewMatrixToRaw();
	void initPos();
	HyDevice* HMDDevice;
	DriverPose_t GetPose(HyTrackingState ctrData);
	vr::DriverPose_t  m_Pose;
	vr::TrackedDeviceIndex_t m_unObjectId;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;
	std::string m_sSerialNumber;
	std::string m_sModelNumber;
	ETrackedControllerRole  m_Type;
	//for display component..
	int32_t m_nWindowX;
	int32_t m_nWindowY;
	int32_t m_nWindowWidth;
	int32_t m_nWindowHeight;
	int32_t m_nRenderWidth;
	int32_t m_nRenderHeight;
	int32_t m_iEyeGapOff;
	float m_fDistortionK1 = 0.6;
	float m_fDistortionK2 = 0.6;
	float m_fZoomWidth = 1;
	float m_fZoomHeight = 1;

	SharedTextures_t m_SharedTextureCache;
	ID3D11Texture2D* GetSharedTexture(HANDLE hSharedTexture);
	float m_flAdditionalLatencyInSeconds = 0.01f;
	double m_flLastVsyncTimeInSeconds;
	clock_t m_tLastSubmitTime;
	volatile uint32_t m_nFrameCounter = 0;
	D3D11_TEXTURE2D_DESC desc;
	HyGraphicsContext* m_DispHandle;
	HyGraphicsContextDesc m_DispDesc;
	HyTextureDesc m_DispTexDesc;
	ID3D11Device* pD3D11Device;
	ID3D11DeviceContext* pD3D11DeviceContext;
	ID3D11Texture2D* m_pTexture;
	IDXGIKeyedMutex* m_pKeyedMutex;
};