#include "HYHMD.h"
#include <thread>
#pragma comment(lib,"d3d11.lib")
using namespace vr;

HyHMD::HyHMD(std::string id, HyDevice* Device) {
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
		return static_cast<IVRVirtualDisplay*>(this);//commit this to get display on screen
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
		*pfTop = -fov.m_upTan;
		*pfBottom = fov.m_downTan;
	}
	else if (eEye == Eye_Right) {
		HMDDevice->GetFloatArray(HY_PROPERTY_HMD_RIGHT_EYE_FOV_FLOAT4_ARRAY, fov.val, 4);
		*pfLeft = -fov.m_leftTan;
		*pfRight = fov.m_rightTan;
		*pfTop = -fov.m_upTan;
		*pfBottom = fov.m_downTan;
	}
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
	m_tLastSubmitTime = clock();
	if (m_pKeyedMutex->AcquireSync(0, 50) != S_OK)
	{
		m_pKeyedMutex->Release();
		return;
	}//wait randering
	m_nFrameCounter = pPresentInfo->nFrameId;
}

void HyHMD::setViewMatrix() {
	HyFov fov[2];
	HMDDevice->GetFloatArray(HY_PROPERTY_HMD_LEFT_EYE_FOV_FLOAT4_ARRAY, fov[0].val, 4);
	HMDDevice->GetFloatArray(HY_PROPERTY_HMD_RIGHT_EYE_FOV_FLOAT4_ARRAY, fov[1].val, 4);
	HyMat4 projMatrix[2];
	m_DispHandle->GetProjectionMatrix(fov[0], 0.1f, 1000.0f, true, projMatrix[0]);
	m_DispHandle->GetProjectionMatrix(fov[1], 0.1f, 1000.0f, true, projMatrix[1]);
	vr::HmdMatrix34_t* projMatrix34[2];
	projMatrix34[0] = (HmdMatrix34_t*)&projMatrix[0];
	projMatrix34[1] = (HmdMatrix34_t*)&projMatrix[1];
	for (int i = 0; i < 4; i++) {
		projMatrix34[0]->m[0][i] = projMatrix34[0]->m[0][i];
		projMatrix34[0]->m[1][i] = projMatrix34[0]->m[1][i];
		projMatrix34[0]->m[2][i] = -projMatrix34[0]->m[2][i];
	}
	for (int i = 0; i < 4; i++) {
		projMatrix34[1]->m[0][i] = projMatrix34[1]->m[0][i];
		projMatrix34[1]->m[1][i] = projMatrix34[1]->m[1][i];
		projMatrix34[1]->m[2][i] = -projMatrix34[1]->m[2][i];
	}
	VRServerDriverHost()->SetDisplayEyeToHead(m_unObjectId, *projMatrix34[0], *projMatrix34[1]);
	DriverLog("Matrix:\n");
	for (int i = 0; i < 4; i++) {
		DriverLog("%f %f %f %f\n", projMatrix34[0]->m[i][0], projMatrix34[0]->m[i][1], projMatrix34[0]->m[i][2], projMatrix34[0]->m[i][3]);
	}
}

void HyHMD::WaitForPresent()
{
	m_DispTexDesc.m_texture = m_pTexture;
	HyPose eyePoses[HY_EYE_MAX];
	HyTrackingState trackInform;
	HMDDevice->GetTrackingState(HY_SUBDEV_HMD, m_nFrameCounter, trackInform);
	m_DispHandle->GetEyePoses(trackInform.m_pose, nullptr, eyePoses);
	//setViewMatrix();
	//m_Pose.vecDriverFromHeadTranslation[0] = (eyePoses[0].m_position.x + eyePoses[1].m_position.x) / 2-trackInform.m_pose.m_position.x;
	//m_Pose.vecDriverFromHeadTranslation[1]=(eyePoses[0].m_position.y + eyePoses[1].m_position.y) / 2- trackInform.m_pose.m_position.y;
	//m_Pose.vecDriverFromHeadTranslation[2] = (eyePoses[0].m_position.z + eyePoses[1].m_position.z) / 2- trackInform.m_pose.m_position.z;
	UpdatePose(trackInform);
	m_DispHandle->Submit(m_nFrameCounter, &m_DispTexDesc, 1);
	if (m_pKeyedMutex)
	{
		m_pKeyedMutex->ReleaseSync(0);
		m_pKeyedMutex->Release();
	}
}

bool HyHMD::GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	*pfSecondsSinceLastVsync = (float)(clock() - m_tLastSubmitTime) / CLOCKS_PER_SEC;
	*pulFrameCounter = m_nFrameCounter;
	return true;
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
	m_Pose.vecDriverFromHeadTranslation[1] = -0.06f;
	m_Pose.vecDriverFromHeadTranslation[2] = -0.10f;//to adjust..

	m_Pose.vecAcceleration[0] = 0.0;
	m_Pose.vecAcceleration[1] = 0.0;
	m_Pose.vecAcceleration[2] = 0.0;
	m_Pose.vecAngularAcceleration[0] = 0.0;
	m_Pose.vecAngularAcceleration[1] = 0.0;
	m_Pose.vecAngularAcceleration[2] = 0.0;
}

DriverPose_t HyHMD::GetPose(HyTrackingState HMDData)
{
	/*if (GetAsyncKeyState(VK_UP) != 0) {
		m_Pose.vecDriverFromHeadTranslation[2] += 0.003;
		DriverLog("vecWorldFromDriverTranslation_z:%f", m_Pose.vecDriverFromHeadTranslation[2]);
	}
	if (GetAsyncKeyState(VK_DOWN) != 0) {
		m_Pose.vecDriverFromHeadTranslation[2] -= 0.003;
		DriverLog("vecWorldFromDriverTranslation_z:%f", m_Pose.vecDriverFromHeadTranslation[2]);
	}
	if (GetAsyncKeyState(VK_LCONTROL) != 0) {
		m_Pose.vecDriverFromHeadTranslation[0] += 0.003;
		DriverLog("vecDriverFromHeadTranslation_y:%f", m_Pose.vecDriverFromHeadTranslation[0]);
	}
	if (GetAsyncKeyState(VK_LSHIFT)!= 0) {
		m_Pose.vecDriverFromHeadTranslation[0] -= 0.003;
		DriverLog("vecDriverFromHeadTranslation_y:%f", m_Pose.vecDriverFromHeadTranslation[0]);
	}
	if (GetAsyncKeyState(VK_LEFT) != 0) {
		m_Pose.vecDriverFromHeadTranslation[1] += 0.003;
		DriverLog("vecDriverFromHeadTranslation_x:%f", m_Pose.vecDriverFromHeadTranslation[1]);
	}
	if (GetAsyncKeyState(VK_RIGHT) != 0) {
		m_Pose.vecDriverFromHeadTranslation[1] -= 0.003;
		DriverLog("vecDriverFromHeadTranslation_x:%f", m_Pose.vecDriverFromHeadTranslation[1]);
	}*/

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

//a1 for output,a2 for input.
double* __fastcall sub_1800072C0(float* a2)
{
	double* a1 = new double[4];
	if (a2[0] + a2[5] + a2[10] <= 0.0)
	{
		if (a2[0] <= a2[5] || a2[0] <= a2[10])
		{
			if (a2[5] <= a2[10])//a2[10] is max
			{
				a1[0] = (a2[4] - a2[1]) / (sqrtf(a2[10] + 1.0 - a2[0] - a2[5]) * 2.0);
				a1[1] = (a2[8] + a2[2]) / (sqrtf(a2[10] + 1.0 - a2[0] - a2[5]) * 2.0);
				a1[2] = (a2[9] + a2[6]) / (sqrtf(a2[10] + 1.0 - a2[0] - a2[5]) * 2.0);
				a1[3] = sqrtf(a2[10] + 1.0 - a2[0] - a2[5]) *0.5;
			}
			else//s2[5] is max
			{
				a1[0] = ((a2[2] - a2[8]) / (sqrtf(a2[5] + 1.0 - a2[0] - a2[10]) * 2.0));
				a1[1] = (a2[4] + a2[1]) / (sqrtf(a2[5] + 1.0 - a2[0] - a2[10]) * 2.0);
				a1[2] = sqrtf(a2[5] + 1.0 - a2[0] - a2[10]) * 0.5;
				a1[3] = (a2[9] + a2[6]) / (sqrtf(a2[5] + 1.0 - a2[0] - a2[10]) * 2.0);
			}
		}
		else//a2[0] is max
		{
			a1[0] = ((a2[9] - a2[6]) / (sqrtf(a2[0] + 1.0 - a2[5] - a2[10]) * 2.0));
			a1[1] = sqrtf(a2[0] + 1.0 - a2[5] - a2[10]) * 0.5;
			a1[2] = (a2[4] + a2[1]) / (sqrtf(a2[0] + 1.0 - a2[5] - a2[10]) * 2.0);
			a1[3] = (a2[8] + a2[2]) / (sqrtf(a2[0] + 1.0 - a2[5] - a2[10]) * 2.0);
		}
	}
	else//a2[0] + a2[5] + a2[10]<0
	{
		a1[0] = sqrtf(a2[0] + a2[5] + a2[10] + 1.0) * 0.5;
		a1[1] = (a2[9] - a2[6]) * (0.5 / sqrtf(a2[0] + a2[5] + a2[10] + 1.0));
		a1[2] = (a2[2] - a2[8]) * (0.5 * sqrtf(a2[0] + a2[5] + a2[10] + 1.0));
		a1[3] = (a2[4] - a2[1]) * (0.5 * sqrtf(a2[0] + a2[5] + a2[10] + 1.0));
	}
	return a1;
} //from 00hypereal00.dll ,might be for distortion