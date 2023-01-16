#include "HYHMD.h"
#include <thread>
#pragma comment(lib,"d3d11.lib")
using namespace vr;



HyHMD::HyHMD(std::string id, HyDevice* Device) {
	//m_fptr_UpdateHyPose = fptr_UpdateHyPose;
	m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;
	m_sSerialNumber = id;
	m_sModelNumber = "";
	HMDDevice = Device;
	initDisplayConfig();
	D3D_FEATURE_LEVEL eFeatureLevel;
	D3D_FEATURE_LEVEL pFeatureLevels[2]{};
	pFeatureLevels[0] = D3D_FEATURE_LEVEL_11_1;
	pFeatureLevels[1] = D3D_FEATURE_LEVEL_11_0;
	D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, pFeatureLevels, 2, D3D11_SDK_VERSION, &pD3D11Device, &eFeatureLevel, &pD3D11DeviceContext);
	memset(&m_DispDesc, 0, sizeof(HyGraphicsContextDesc));
	m_DispDesc.m_graphicsDevice = pD3D11Device;
	m_DispDesc.m_graphicsAPI = HY_GRAPHICS_D3D11;
	m_DispDesc.m_pixelFormat = HY_TEXTURE_R8G8B8A8_UNORM_SRGB;
	m_DispDesc.m_pixelDensity = 1.0f;
	m_DispDesc.m_mirrorWidth = 2160;
	m_DispDesc.m_mirrorHeight = 1200;
	m_DispDesc.m_flags = 0;
	HyResult hr=Device->CreateGraphicsContext(m_DispDesc, &m_DispHandle);
	m_DispTexDesc.m_uvOffset = HyVec2{ 0.0f, 0.0f };
	m_DispTexDesc.m_uvSize = HyVec2{ 1.0f, 1.0f };
}


HyHMD::~HyHMD(){
	m_DispHandle->Release();
}


void HyHMD::initDisplayConfig() {
	m_nWindowX = 0;
	m_nWindowY = 0;
	m_nWindowWidth = 2160;
	m_nWindowHeight = 1200;
#ifdef DISPLAY_DEBUG
	m_nWindowWidth /= 10;
	m_nWindowHeight /= 10;//smaller window
#endif // DISPLAY_DEBUG
}

EVRInitError HyHMD::Activate(uint32_t unObjectId)
{
	m_unObjectId = unObjectId;
	initPos();
	m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);
	vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_CurrentUniverseId_Uint64, 2);
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, m_sSerialNumber.c_str());
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, "HYHMD");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ManufacturerName_String, "HYPEREAL");
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_UserIpdMeters_Float, 0.068);//soft ipd
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_UserHeadToEyeDepthMeters_Float, 0.16f);
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_DisplayFrequency_Float, 90);
	vr::VRProperties()->SetFloatProperty(m_ulPropertyContainer, Prop_SecondsFromVsyncToPhotons_Float, 0.0);
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{revive_hypereal}/icons/hypereal_headset_off.png");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{revive_hypereal}/icons/hypereal_headset_searching.gif");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{revive_hypereal}/icons/hypereal_headset_searching_alert.gif");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{revive_hypereal}/icons/hypereal_headset_ready.png");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{revive_hypereal}/icons/hypereal_headset_ready_alert.png");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{revive_hypereal}/icons/hypereal_headset_not_ready.png");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{revive_hypereal}/icons/hypereal_headset_standby.png");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{revive_hypereal}/icons/hypereal_headset_ready_alert.png");
	return VRInitError_None;
}

void HyHMD::Deactivate()
{
	m_unObjectId = k_unTrackedDeviceIndexInvalid;
}

void HyHMD::EnterStandby()
{
}

void* HyHMD::GetComponent(const char* pchComponentNameAndVersion) {
	if (_stricmp(pchComponentNameAndVersion, ITrackedDeviceServerDriver_Version) == 0) {
		return static_cast<ITrackedDeviceServerDriver*>(this);
	}
	if (_stricmp(pchComponentNameAndVersion, IVRDisplayComponent_Version) == 0) {
		return static_cast<IVRDisplayComponent*>(this);
	}
	if (_stricmp(pchComponentNameAndVersion, IVRVirtualDisplay_Version) == 0) {
#ifndef DISPLAY_DEBUG
		return static_cast<IVRVirtualDisplay*>(this);
#endif
	}
	return nullptr;
}

void HyHMD::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

DriverPose_t HyHMD::GetPose()
{
	return m_Pose;
}

PropertyContainerHandle_t HyHMD::GetPropertyContainer()
{
	return PropertyContainerHandle_t();
}

std::string HyHMD::GetSerialNumber()
{
	return m_sSerialNumber;
}

void HyHMD::UpdatePose(HyTrackingState HMDData)
{
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, GetPose(HMDData), sizeof(DriverPose_t));
}

//display component

void HyHMD::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
	*pnX = 0;
	*pnY = 0;
	*pnWidth = m_nWindowWidth;
	*pnHeight = m_nWindowHeight;
}

bool HyHMD::IsDisplayOnDesktop()
{
	return false;
}

bool HyHMD::IsDisplayRealDisplay()
{
	return false;
}

void HyHMD::GetRecommendedRenderTargetSize(uint32_t* pnWidth, uint32_t* pnHeight)
{
	uint32_t leftWidth = 0, leftHeight = 0;
	uint32_t rightWidth = 0, rightHeight = 0;
	m_DispHandle->GetRenderTargetSize(HY_EYE_LEFT, leftWidth, leftHeight);
	m_DispHandle->GetRenderTargetSize(HY_EYE_RIGHT, rightWidth, rightHeight);
	uint32_t finalWidth = leftWidth + rightWidth;
	uint32_t finalHeight = leftHeight > rightHeight ? leftHeight : rightHeight;
	*pnWidth = finalWidth/2;//for single eye
	*pnHeight = finalHeight;
}

void HyHMD::GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
	*pnY = 0;
	*pnWidth = m_nWindowWidth / 2;
	*pnHeight = m_nWindowHeight;

	if (eEye == vr::Eye_Left) {
		*pnX = 0;
	}
	else {
		*pnX = m_nWindowWidth / 2;
	}

}

void HyHMD::GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
{
	//set fov
	HyFov fov;
	if (eEye == Eye_Left) {
		HMDDevice->GetFloatArray(HY_PROPERTY_HMD_LEFT_EYE_FOV_FLOAT4_ARRAY, fov.val, 4);
		*pfLeft = -fov.m_leftTan;
		*pfRight = fov.m_rightTan;
		*pfTop = fov.m_upTan;
		*pfBottom = -fov.m_downTan;
	}
	else if (eEye == Eye_Right) {
		HMDDevice->GetFloatArray(HY_PROPERTY_HMD_RIGHT_EYE_FOV_FLOAT4_ARRAY, fov.val, 4);
		*pfLeft = -fov.m_leftTan;
		*pfRight = fov.m_rightTan;
		*pfTop = fov.m_upTan;
		*pfBottom = -fov.m_downTan;
	}
	std::swap(*pfTop, *pfBottom);
	//DriverLog("GetProjectionRaw lrtb:%f%f%f%f", *pfLeft, *pfRight, *pfTop, *pfBottom);//same as official
}

DistortionCoordinates_t HyHMD::ComputeDistortion(EVREye eEye, float fU, float fV)
{
	DistortionCoordinates_t coordinates;
	coordinates.rfBlue[0] = fU;
	coordinates.rfBlue[1] = fV;
	coordinates.rfGreen[0] =fU;
	coordinates.rfGreen[1] =fV;
	coordinates.rfRed[0] = fU;
	coordinates.rfRed[1] =fV;
	return coordinates;
}

ID3D11Texture2D* HyHMD::GetSharedTexture(HANDLE hSharedTexture)
{
	if (!hSharedTexture)
		return NULL;
	for (SharedTextures_t::iterator it = m_SharedTextureCache.begin();
		it != m_SharedTextureCache.end(); ++it)
	{
		if (it->m_hSharedTexture == hSharedTexture)
		{
			return it->m_pTexture;
		}
	}

	ID3D11Texture2D* pTexture;
	if (SUCCEEDED(pD3D11Device->OpenSharedResource(
		hSharedTexture, __uuidof(ID3D11Texture2D), (void**)&pTexture)))
	{
		SharedTextureEntry_t entry{ hSharedTexture, pTexture };
		m_SharedTextureCache.push_back(entry);
		return pTexture;
	}
	return NULL;
}//from virtual display simple

void HyHMD::Present(const PresentInfo_t* pPresentInfo, uint32_t unPresentInfoSize)
{
	m_pTexture = GetSharedTexture((HANDLE)pPresentInfo->backbufferTextureHandle);
	m_pKeyedMutex = NULL;
	m_pTexture->QueryInterface(__uuidof(IDXGIKeyedMutex), (void**)&m_pKeyedMutex);
	if (m_pKeyedMutex->AcquireSync(0, 50) != S_OK)
	{
		m_pKeyedMutex->Release();
		return;
	}//go randering
	
	if (m_pFlushTexture == NULL)
	{
		D3D11_TEXTURE2D_DESC srcDesc;
		m_pTexture->GetDesc(&srcDesc);

		// Create a second small texture for copying and reading a single pixel from
		// in order to block on the cpu until rendering is finished.
		D3D11_TEXTURE2D_DESC flushTextureDesc;
		ZeroMemory(&flushTextureDesc, sizeof(flushTextureDesc));
		flushTextureDesc.Width = 32;
		flushTextureDesc.Height = 32;
		flushTextureDesc.MipLevels = 1;
		flushTextureDesc.ArraySize = 1;
		flushTextureDesc.Format = srcDesc.Format;
		flushTextureDesc.SampleDesc.Count = 1;
		flushTextureDesc.Usage = D3D11_USAGE_STAGING;
		flushTextureDesc.BindFlags = 0;
		flushTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		if (FAILED(pD3D11Device->CreateTexture2D(&flushTextureDesc, NULL, &m_pFlushTexture)))
		{
			return;
		}
	}
	D3D11_BOX box = { 0, 0, 0, 1, 1, 1 };
	pD3D11DeviceContext->CopySubresourceRegion(m_pFlushTexture, 0, 0, 0, 0, m_pTexture, 0, &box);
	//pD3D11DeviceContext->Flush();
	copyToStaging();
	m_nFrameCounter +=1;
	pre = clock();
}

void HyHMD::WaitForPresent()
{
	if (m_pFlushTexture)
	{
		D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
		if (SUCCEEDED(pD3D11DeviceContext->Map(m_pFlushTexture, 0, D3D11_MAP_READ, 0, &mapped)))
		{
			pD3D11DeviceContext->Unmap(m_pFlushTexture, 0);
		}
	}
	clock_t done = clock();
	//DriverLog("time:%d", done - pre); 
	if (done - pre > 11) {
		DriverLog("Frame drop in %dms!!!!", done - pre);
		if (m_pKeyedMutex)
		{
			m_pKeyedMutex->ReleaseSync(0);
			m_pKeyedMutex->Release();
		}
		return;
	}
	HyTrackingState trackInform;
	HMDDevice->GetTrackingState(HY_SUBDEV_HMD, 0, trackInform);
	UpdatePose(trackInform);
	//HMDDevice->GetTrackingState(HY_SUBDEV_CONTROLLER_LEFT, 0, trackInform);
	//(*m_fptr_UpdateHyPose)(trackInform, true);
	//HMDDevice->GetTrackingState(HY_SUBDEV_CONTROLLER_RIGHT, 0, trackInform);
	//(*m_fptr_UpdateHyPose)(trackInform, false);
	m_DispTexDesc.m_texture = m_pTexture;
	m_DispHandle->Submit(0, &m_DispTexDesc, 1);//takes about 11ms since last submit
	if (m_pKeyedMutex)
	{
		m_pKeyedMutex->ReleaseSync(0);
		m_pKeyedMutex->Release();
	}
}

bool HyHMD::GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	return false;//Just don't implement this
}

//private

void HyHMD::initPos()
{
	m_Pose.result = vr::TrackingResult_Running_OK;
	m_Pose.poseIsValid = true;
	m_Pose.willDriftInYaw = true;
	m_Pose.shouldApplyHeadModel = false;
	m_Pose.deviceIsConnected = true;

	m_Pose.poseTimeOffset = 0.0f;
	m_Pose.qWorldFromDriverRotation.w = 1.0;
	m_Pose.qWorldFromDriverRotation.x = 0.0;
	m_Pose.qWorldFromDriverRotation.y = 0.0;
	m_Pose.qWorldFromDriverRotation.z = 0.0;
	m_Pose.vecWorldFromDriverTranslation[0] = 0.0;
	m_Pose.vecWorldFromDriverTranslation[1] = 0.0;
	m_Pose.vecWorldFromDriverTranslation[2] = 0.0;

	m_Pose.qDriverFromHeadRotation.w = 1.0;
	m_Pose.qDriverFromHeadRotation.x = 0.0;
	m_Pose.qDriverFromHeadRotation.y = 0.0;
	m_Pose.qDriverFromHeadRotation.z = 0.0;

	m_Pose.vecDriverFromHeadTranslation[0] =  0.00f;
	m_Pose.vecDriverFromHeadTranslation[1] = -0.009f;
	m_Pose.vecDriverFromHeadTranslation[2] = -0.148f;//seems good

	m_Pose.vecAcceleration[0] = 0.0;
	m_Pose.vecAcceleration[1] = 0.0;
	m_Pose.vecAcceleration[2] = 0.0;
	m_Pose.vecAngularAcceleration[0] = 0.0;
	m_Pose.vecAngularAcceleration[1] = 0.0;
	m_Pose.vecAngularAcceleration[2] = 0.0;
}

bool HyHMD::copyToStaging()
{
	if (m_pStagingTexture == NULL)
	{
		D3D11_TEXTURE2D_DESC srcDesc;
		m_pTexture->GetDesc(&srcDesc);

		D3D11_TEXTURE2D_DESC stagingTextureDesc;
		ZeroMemory(&stagingTextureDesc, sizeof(stagingTextureDesc));
		stagingTextureDesc.Width = srcDesc.Width;
		stagingTextureDesc.Height = srcDesc.Height;
		stagingTextureDesc.Format = srcDesc.Format;
		stagingTextureDesc.MipLevels = 1;
		stagingTextureDesc.ArraySize = 1;
		stagingTextureDesc.SampleDesc.Count = 1;
		stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
		stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

		if (FAILED(pD3D11Device->CreateTexture2D(&stagingTextureDesc, NULL, &m_pStagingTexture)))
		{
			DriverLog("Failed to create staging texture!");
			return false;
		}
	}

	pD3D11DeviceContext->CopyResource(m_pStagingTexture, m_pTexture);
	//DriverLog("Copyed");
	return true;
}

DriverPose_t HyHMD::GetPose(HyTrackingState HMDData)
{
	HyPose eyePoses[HY_EYE_MAX];
	m_DispHandle->GetEyePoses(HMDData.m_pose, nullptr, eyePoses);
#ifdef DEBUG_COORDINATE

	if (GetAsyncKeyState(VK_UP) != 0) {
		m_Pose.vecDriverFromHeadTranslation[2] += 0.003;
		DriverLog("vecWorldFromDriverTranslation_[2]:%f", m_Pose.vecDriverFromHeadTranslation[2]);
	}
	if (GetAsyncKeyState(VK_DOWN) != 0) {
		m_Pose.vecDriverFromHeadTranslation[2] -= 0.003;
		DriverLog("vecWorldFromDriverTranslation_[2]:%f", m_Pose.vecDriverFromHeadTranslation[2]);
	}
	if (GetAsyncKeyState(VK_LCONTROL) != 0) {
		m_Pose.vecDriverFromHeadTranslation[0] += 0.003;
		DriverLog("vecDriverFromHeadTranslation[0]:%f", m_Pose.vecDriverFromHeadTranslation[0]);
	}
	if (GetAsyncKeyState(VK_LSHIFT) != 0) {
		m_Pose.vecDriverFromHeadTranslation[0] -= 0.003;
		DriverLog("vecDriverFromHeadTranslation[0]:%f", m_Pose.vecDriverFromHeadTranslation[0]);
	}
	if (GetAsyncKeyState(VK_LEFT) != 0) {
		m_Pose.vecDriverFromHeadTranslation[1] += 0.003;
		DriverLog("vecDriverFromHeadTranslation[1]:%f", m_Pose.vecDriverFromHeadTranslation[1]);
	}
	if (GetAsyncKeyState(VK_RIGHT) != 0) {
		m_Pose.vecDriverFromHeadTranslation[1] -= 0.003;
		DriverLog("vecDriverFromHeadTranslation[1]:%f", m_Pose.vecDriverFromHeadTranslation[1]);
	}

#endif // DEBUG_COORDINATE
	m_Pose.result = vr::TrackingResult_Running_OK;
	m_Pose.poseIsValid = true;
	m_Pose.deviceIsConnected = true;
	m_Pose.vecPosition[0] = HMDData.m_pose.m_position.x;
	m_Pose.vecPosition[1] = HMDData.m_pose.m_position.y;
	m_Pose.vecPosition[2] = HMDData.m_pose.m_position.z;
	m_Pose.qRotation.x = HMDData.m_pose.m_rotation.x;
	m_Pose.qRotation.y = HMDData.m_pose.m_rotation.y;
	m_Pose.qRotation.z = HMDData.m_pose.m_rotation.z;
	m_Pose.qRotation.w = HMDData.m_pose.m_rotation.w;
	m_Pose.vecVelocity[0] = HMDData.m_linearVelocity.x;
	m_Pose.vecVelocity[1] = HMDData.m_linearVelocity.y;
	m_Pose.vecVelocity[2] = HMDData.m_linearVelocity.z;
	//m_Pose.vecAngularVelocity[0] = HMDData.m_angularVelocity.x;
	//m_Pose.vecAngularVelocity[1] = HMDData.m_angularVelocity.y;
	//m_Pose.vecAngularVelocity[2] = HMDData.m_angularVelocity.z;//Avoid shaking
	m_Pose.vecAngularAcceleration[0] = HMDData.m_angularAcceleration.x;
	m_Pose.vecAngularAcceleration[1] = HMDData.m_angularAcceleration.y;
	m_Pose.vecAngularAcceleration[2] = HMDData.m_angularAcceleration.z;
	m_Pose.vecAcceleration[0] = HMDData.m_linearAcceleration.x;
	m_Pose.vecAcceleration[1] = HMDData.m_linearAcceleration.y;
	m_Pose.vecAcceleration[2] = HMDData.m_linearAcceleration.z;
	if (HMDData.m_flags == HY_TRACKING_NONE) {
		m_Pose.result = vr::TrackingResult_Uninitialized;
		m_Pose.poseIsValid = false;
		m_Pose.deviceIsConnected = false;
		return m_Pose;
	}
	/*if (HMDData.m_flags == HY_TRACKING_ROTATION_TRACKED) {
		m_Pose.result = vr::TrackingResult_Fallback_RotationOnly;
		m_Pose.poseIsValid = false;
		return m_Pose;
	}*///¶ª×·×Ù²»»ÒÆÁ
	return m_Pose;
}

