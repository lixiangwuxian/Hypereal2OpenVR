#include "openvr_driver.h"

#include "ServerDriver.h"

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>

#pragma comment(lib,"shell32.lib")

#pragma comment(lib, "User32.lib")


std::thread ErrorAlarmThreadWorker;
std::thread BoardcastThreadWorker;
void ErrorAlarm(HyResult result);
void Boardcast();

vr::EVRInitError ServerDriver::Init(vr::IVRDriverContext* DriverContext) {

	vr::EVRInitError eError = vr::InitServerDriverContext(DriverContext);
		if (eError != vr::VRInitError_None) {
			return eError;
	}
	HyStartup();
	HyResult ifCreate = HyCreateInterface(HyDevice_InterfaceName, 0, &HyTrackingDevice);
	
	//BoardcastThreadWorker = std::thread::thread(&Boardcast);
	if (ifCreate >= 100) {//we got an error..
		ErrorAlarmThreadWorker = std::thread::thread(&ErrorAlarm, ifCreate);
		return vr::VRInitError_Driver_Failed;
	}
	
	this->HyLeftController = new HyController("LctrTEST@LXWX", TrackedControllerRole_LeftHand,HyTrackingDevice);
	this->HyRightController = new HyController("RctrTEST@LXWX", TrackedControllerRole_RightHand,HyTrackingDevice);

	vr::VRServerDriverHost()->TrackedDeviceAdded(HyLeftController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, this->HyLeftController);
	vr::VRServerDriverHost()->TrackedDeviceAdded(HyRightController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, this->HyRightController);

	m_bEventThreadRunning = false;
	if (!m_bEventThreadRunning)
	{
		m_bEventThreadRunning = true;
		send_haptic_thread_worker = std::thread::thread(&ServerDriver::Send_haptic_event_thread, this);
		send_haptic_thread_worker.detach();
		updatePoseThreadWorker = std::thread::thread(&ServerDriver::UpdatePoseThread, this);
		updatePoseThreadWorker.detach();
		updateKeyThreadWorker = std::thread::thread(&ServerDriver::UpdateKeyThread, this);
		updateKeyThreadWorker.detach();
		checkBatteryThreadWorker = std::thread::thread(&ServerDriver::UpdateControllerBatteryThread, this);
		checkBatteryThreadWorker.detach();
	}
	return vr::VRInitError_None;
}

void ErrorAlarm(HyResult result) {
	switch (result)
	{
	case hyError_NeedStartup:
		MessageBox(NULL, L"hyError_NeedStartup\n未初始化", L"错误", MB_OK);
		break;
	case hyError_DeviceNotStart:
		MessageBox(NULL, L"hyError_DeviceNotStart\n设备未启动", L"错误", MB_OK);
		break;
	case hyError_InvalidHeadsetOrientation:
		MessageBox(NULL, L"hyError_InvalidHeadsetOrientation\n无效的头显旋转坐标", L"错误", MB_OK);
		break;
	case hyError_RenderNotCreated:
		MessageBox(NULL, L"hyError_RenderNotCreated\n未创建渲染组件", L"错误", MB_OK);
		break;
	case hyError_TextureNotCreated:
		MessageBox(NULL, L"hyError_TextureNotCreated\n未创建材质", L"错误", MB_OK);
		break;
	case hyError_DisplayLost:
		MessageBox(NULL, L"hyError_DisplayLost\n显示接口丢失", L"错误", MB_OK);
		break;
	case hyError_NoHmd:
		MessageBox(NULL, L"hyError_NoHmdFound\n未发现头显", L"错误", MB_OK);
		break;
	case hyError_DeviceNotConnected:
		MessageBox(NULL, L"hyError_DeviceNotConnected\n设备未连接", L"错误", MB_OK);
		break;
	case hyError_ServiceConnection:
		MessageBox(NULL, L"hyError_ServiceConnection\n服务连接错误", L"错误", MB_OK);
		break;
	case hyError_ServiceError:
		MessageBox(NULL, L"hyError_ServiceError\n服务错误", L"错误", MB_OK);
		break;
	case hyError_InvalidParameter:
		MessageBox(NULL, L"hyError_InvalidParameter\n无效参数", L"错误", MB_OK);
		break;
	case hyError_NoCalibration:
		MessageBox(NULL, L"hyErrrr_NoCalibration\n需要在HY客户端进行校准", L"错误", MB_OK);
		break;
	case hyError_NotImplemented:
		MessageBox(NULL, L"hyError_NotImplemented\n未实例化", L"错误", MB_OK);
		break;
	case hyError_InvalidClientType:
		MessageBox(NULL, L"hyError_InvalidClientType\n无效的客户端类型", L"错误", MB_OK);
		break;
	case hyError_BufferTooSmall:
		MessageBox(NULL, L"hyError_BufferTooSmall\n缓冲区过小", L"错误", MB_OK);
		break;
	case hyError_InvalidState:
		MessageBox(NULL, L"hyError_InvalidState\n设备状态无效", L"错误", MB_OK);
		break;
	default:
		break;
	}
}

void Boardcast() {
	const TCHAR szOperation[] = _T("open");
	const TCHAR szAddress[] = _T("https://github.com/lixiangwuxian/HyperealDriverTest");
	int result=MessageBox(NULL, L"2022/7/1 beta 0.4，和Hypereal的SteamVR驱动叠加使用。\n\
Bug:无震动。\n\
Created By lixiangwuxian@github\n\
点击否可打开此驱动Github页面检查更新\n\
点击是继续使用"\
, L"提示", MB_YESNO);
	if (result == IDNO) {
		ShellExecute(NULL, szOperation, szAddress, NULL, NULL, SW_SHOWNORMAL);
	}
}

void ServerDriver::Cleanup() {
	this->HyTrackingDevice=NULL;
	this->HyLeftController=NULL;
	this->HyRightController=NULL;

	delete this->HyTrackingDevice;
	delete this->HyLeftController;
	delete this->HyRightController;
	VR_CLEANUP_SERVER_DRIVER_CONTEXT();
}

const char* const* ServerDriver::GetInterfaceVersions() {
	return vr::k_InterfaceVersions;
}


bool ServerDriver::ShouldBlockStandbyMode() {
	return false;
}



void ServerDriver::UpdateHaptic(VREvent_t& eventHandle)
{
	if (eventHandle.eventType == VREvent_Input_HapticVibration)
	{
		float amplitude=0, duration=0;
		VREvent_HapticVibration_t data = eventHandle.data.hapticVibration;
		duration = fmaxf(10,data.fDurationSeconds*1000);
		amplitude =fmaxf(0.3,data.fAmplitude);
		if (HyLeftController->GetPropertyContainer() == data.containerHandle) {
			HyTrackingDevice->SetControllerVibration(HY_SUBDEV_CONTROLLER_LEFT,duration, amplitude);
		}
		else if (HyRightController->GetPropertyContainer() == data.containerHandle) {
			HyTrackingDevice->SetControllerVibration(HY_SUBDEV_CONTROLLER_RIGHT, duration, amplitude);
		}
	}
}

void ServerDriver::UpdateHyPose(const HyTrackingState& newData,bool leftOrRight) {
	if (leftOrRight){
		HyLeftController->UpdatePose(newData);
	}else{
		HyRightController->UpdatePose(newData);
	}
}

void ServerDriver::UpdateHyKey(HySubDevice device, HyInputState type)
{
	if (device == HY_SUBDEV_CONTROLLER_LEFT) {
		HyLeftController->SendButtonUpdate(type);
	}
	else if(device == HY_SUBDEV_CONTROLLER_RIGHT){
		HyRightController->SendButtonUpdate(type);
	}
}

void ServerDriver::UpdateControllerBatteryThread()
{
	int64_t batteryValue = 3;
	while (true) {
		HyTrackingDevice->GetIntValue(HY_PROPERTY_DEVICE_BATTERY_INT, batteryValue, HY_SUBDEV_CONTROLLER_LEFT);
		HyLeftController->UpdateBattery(batteryValue);
		HyTrackingDevice->GetIntValue(HY_PROPERTY_DEVICE_BATTERY_INT, batteryValue, HY_SUBDEV_CONTROLLER_RIGHT);
		HyRightController->UpdateBattery(batteryValue);
		Sleep(1000);
	}
}


void ServerDriver::Send_haptic_event_thread()
{
	VREvent_t pEventHandle;
	bool bHasEvent = false;
	while (m_bEventThreadRunning)
	{
		bHasEvent = vr::VRServerDriverHost()->PollNextEvent(&pEventHandle, sizeof(VREvent_t));
		if (bHasEvent)
		{
			UpdateHaptic(pEventHandle);
		}
		else
		{
			Sleep(1);
		}
		memset(&pEventHandle, 0, sizeof(VREvent_t));
	}
}


void ServerDriver::UpdatePoseThread() {
	while (m_bEventThreadRunning) {
		HyTrackingDevice->GetTrackingState(HY_SUBDEV_CONTROLLER_LEFT, 0, trackInform,0);
		UpdateHyPose(trackInform, true);
		HyTrackingDevice->GetTrackingState(HY_SUBDEV_CONTROLLER_RIGHT, 0, trackInform,0);
		UpdateHyPose(trackInform, false);
		Sleep(8);
	}
}

void ServerDriver::UpdateKeyThread() {
	HyInputState keyInput;
	while(m_bEventThreadRunning) {
		HyTrackingDevice->GetControllerInputState(HY_SUBDEV_CONTROLLER_LEFT, keyInput);
		UpdateHyKey(HY_SUBDEV_CONTROLLER_LEFT, keyInput);
		HyTrackingDevice->GetControllerInputState(HY_SUBDEV_CONTROLLER_RIGHT, keyInput);
		UpdateHyKey(HY_SUBDEV_CONTROLLER_RIGHT, keyInput);
		Sleep(1);
	}
}

void ServerDriver::RunFrame() {}
void ServerDriver::EnterStandby() {}
void ServerDriver::LeaveStandby() {}