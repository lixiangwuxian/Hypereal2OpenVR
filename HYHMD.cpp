#include "HYHMD.h"

using namespace vr;

HyHMD::HyHMD(std::string id, ETrackedControllerRole type, HyDevice* Device) {
	m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;
	m_sSerialNumber = id;
	m_Type = type;
	m_sModelNumber = "";
	HMDDevice = Device;
	
}

HyHMD::~HyHMD(){}

EVRInitError HyHMD::Activate(uint32_t unObjectId)
{
	m_unObjectId = unObjectId;
	initPos();
	m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(m_unObjectId);
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, m_sSerialNumber.c_str());
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, "HYHMD");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ManufacturerName_String, "HYPEREAL");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_InputProfilePath_String, "");//no need..

	ETrackedPropertyError erro;
	return VRInitError_None;
}

void HyHMD::Deactivate()
{//do nothing..
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
		return static_cast<IVRVirtualDisplay*>(this);
	}
	return nullptr;
}

void HyHMD::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
}

PropertyContainerHandle_t HyHMD::GetPropertyContainer()
{
	return PropertyContainerHandle_t();
}

std::string HyHMD::GetSerialNumber()
{
	return std::string();
}

void HyHMD::UpdatePose(HyTrackingState ctrData)
{
}

//display component

void HyHMD::GetWindowBounds(int32_t* pnX, int32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
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
}

void HyHMD::GetEyeOutputViewport(EVREye eEye, uint32_t* pnX, uint32_t* pnY, uint32_t* pnWidth, uint32_t* pnHeight)
{
}

void HyHMD::GetProjectionRaw(EVREye eEye, float* pfLeft, float* pfRight, float* pfTop, float* pfBottom)
{
}

DistortionCoordinates_t HyHMD::ComputeDistortion(EVREye eEye, float fU, float fV)
{
	return DistortionCoordinates_t();
}

//vritual display

void HyHMD::Present(const PresentInfo_t* pPresentInfo, uint32_t unPresentInfoSize)
{
	if (!pPresentInfo->backbufferTextureHandle)
	{
		m_VirtualDisplay.m_FrameReady = false;
		return;
	}
	m_VirtualDisplay.TextureFromHandle(pPresentInfo->backbufferTextureHandle);
}

void HyHMD::WaitForPresent()
{
	//already presented
}

bool HyHMD::GetTimeSinceLastVsync(float* pfSecondsSinceLastVsync, uint64_t* pulFrameCounter)
{
	*pfSecondsSinceLastVsync = m_VirtualDisplay.m_FrameTime;
	*pulFrameCounter = m_VirtualDisplay.m_FrameCount;
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

	m_Pose.poseTimeOffset = -0.016f;
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

	m_Pose.vecDriverFromHeadTranslation[0] = 0.000f;
	m_Pose.vecDriverFromHeadTranslation[1] = 0.000f;
	m_Pose.vecDriverFromHeadTranslation[2] = 0.000f;

	m_Pose.vecAcceleration[0] = 0.0;
	m_Pose.vecAcceleration[1] = 0.0;
	m_Pose.vecAcceleration[2] = 0.0;
	m_Pose.vecAngularAcceleration[0] = 0.0;
	m_Pose.vecAngularAcceleration[1] = 0.0;
	m_Pose.vecAngularAcceleration[2] = 0.0;
}

DriverPose_t HyHMD::GetPose(HyTrackingState ctrData)
{
	return DriverPose_t();
}
