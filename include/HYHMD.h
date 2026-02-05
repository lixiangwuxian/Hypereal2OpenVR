#pragma once
#include<openvr_driver.h>
#include"Hypereal_VR.h"
#include<string.h>
#include"driverlog.h"
#include<d3d11.h>
#include<time.h>
#include"FrameEncoder.h"
#include <systemtime.h>

//typedef void(UpdateHyPoseCallBack)(const HyTrackingState& newData, bool leftOrRight);



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
	void UpdatePose();
	
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
	void InitializePosition();
	ID3D11Texture2D* GetSharedTexture(HANDLE hSharedTexture);
	DriverPose_t GetPose(HyTrackingState ctrData);
	HyDevice* m_pHMDDevice = nullptr;
	vr::DriverPose_t  m_Pose;
	vr::TrackedDeviceIndex_t m_unObjectId = 0;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;
	std::string m_sSerialNumber;
	std::string m_sModelNumber;
	ETrackedControllerRole  m_Type;
	//for display component..
	int32_t m_nWindowX = 0;
	int32_t m_nWindowY = 0;
	int32_t m_nWindowWidth = 0;
	int32_t m_nWindowHeight = 0;
	int32_t m_nRenderWidth = 0;
	int32_t m_nRenderHeight = 0;
	//virtual display
	SharedTextures_t m_SharedTextureCache;
	float m_flAdditionalLatencyInSeconds = 0.00f;
	uint32_t m_uDropFrames = 0;
	volatile uint32_t m_nFrameCounter = 0;
	HyGraphicsContext* m_pDispHandle = nullptr;
	HyGraphicsContextDesc m_DispDesc;

	// VSync timing (following Valve's virtual_display pattern)
	double m_flLastVsyncTimeInSeconds = 0.0;
	uint64_t m_nVsyncCounter = 0;
	uint64_t m_nCurrentFrameId = 0;
	static constexpr double kFrameIntervalInSeconds = 1.0 / 90.0;  // 90Hz

	FrameEncoder* m_pFrameEncoder = nullptr;
	ID3D11Device* m_pD3D11Device = nullptr;
	ID3D11DeviceContext* m_pD3D11DeviceContext = nullptr;
	ID3D11Texture2D* m_pTexture = nullptr;
	ID3D11Texture2D* m_pFlushTexture = nullptr;
	IDXGIKeyedMutex* m_pKeyedMutex = nullptr;
};