#include "ServerDriver.h"
#pragma comment(lib,"shell32.lib")
#pragma comment(lib, "User32.lib")
std::thread ErrorAlarmThreadWorker;
std::thread BoardcastThreadWorker;
void ErrorAlarm(HyResult result);
void Boardcast();

bool killProcessByName(const wchar_t* filename)
{
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	bool foundProcess = false;
	while (hRes)
	{
		if (lstrcmpW(pEntry.szExeFile, filename) == 0)
		{
			foundProcess = true;
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL)
			{
				TerminateProcess(hProcess, 9);
				CloseHandle(hProcess);
			}
		}
		hRes = Process32Next(hSnapShot, &pEntry);
	}
	CloseHandle(hSnapShot);
	return foundProcess;
}

vr::EVRInitError ServerDriver::Init(vr::IVRDriverContext* DriverContext) {
	vr::EVRInitError eError = vr::InitServerDriverContext(DriverContext);
		if (eError != vr::VRInitError_None) {
			return eError;
	}
	InitDriverLog(vr::VRDriverLog());
	HyStartup();
	HyResult ifCreate = HyCreateInterface(HyDevice_InterfaceName, 0, &HyTrackingDevice);
	
	BoardcastThreadWorker = std::thread::thread(&Boardcast);
	BoardcastThreadWorker.detach();

	ErrorAlarmThreadWorker = std::thread::thread(&ErrorAlarm, ifCreate);
	ErrorAlarmThreadWorker.detach();
	
	if (ifCreate >= 100) {//we got an error.. Don't initialize any device or steamvr would crash.
		return vr::VRInitError_None;
	}

#ifdef USE_HMD
	HyHead = new HyHMD("HYHMD@LXWX",HyTrackingDevice);
#endif // USE_HMD
	HyLeftController = new HyController("LctrTEST@LXWX", TrackedControllerRole_LeftHand,HyTrackingDevice);
	HyRightController = new HyController("RctrTEST@LXWX", TrackedControllerRole_RightHand,HyTrackingDevice);

#ifdef USE_HMD
	frameID=HyHead->getFrameIDptr();
	vr::VRServerDriverHost()->TrackedDeviceAdded(HyHead->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, this->HyHead);
#endif // USE_HMD
	vr::VRServerDriverHost()->TrackedDeviceAdded(HyLeftController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, this->HyLeftController);
	vr::VRServerDriverHost()->TrackedDeviceAdded(HyRightController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, this->HyRightController);

	m_bEventThreadRunning = false;
	if (!m_bEventThreadRunning)
	{
		m_bEventThreadRunning = true;
		send_haptic_thread_worker = std::thread::thread(&ServerDriver::Send_haptic_event_thread, this);//震动线程
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
	case hyError:
		MessageBox(NULL, L"hyError\n不清楚什么原因但是报错了\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_NeedStartup:
		MessageBox(NULL, L"hyError_NeedStartup\n未初始化\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_DeviceNotStart:
		MessageBox(NULL, L"hyError_DeviceNotStart\n设备未启动\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_InvalidHeadsetOrientation:
		MessageBox(NULL, L"hyError_InvalidHeadsetOrientation\n无效的头显四元数坐标\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_RenderNotCreated:
		MessageBox(NULL, L"hyError_RenderNotCreated\n未创建渲染组件\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_TextureNotCreated:
		MessageBox(NULL, L"hyError_TextureNotCreated\n未创建材质\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_DisplayLost:
		MessageBox(NULL, L"hyError_InvalidParameter\n显示接口丢失\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_NoHmd:
		MessageBox(NULL, L"hyError_NoHmd\n未发现头显\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_DeviceNotConnected:
		MessageBox(NULL, L"hyError_DeviceNotConnected\n设备未连接\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_ServiceConnection:
		MessageBox(NULL, L"hyError_ServiceConnection\n服务连接错误\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_ServiceError:
		MessageBox(NULL, L"hyError_ServiceError\n服务错误\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_InvalidParameter:
		MessageBox(NULL, L"hyError_InvalidParameter\n无效参数\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_NoCalibration:
		MessageBox(NULL, L"hyError_NoCalibration\n需要在HY客户端进行校准\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_NotImplemented:
		MessageBox(NULL, L"hyError_NotImplemented\n未实例化\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_InvalidClientType:
		MessageBox(NULL, L"hyError_InvalidClientType\n无效的客户端类型\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_BufferTooSmall:
		MessageBox(NULL, L"hyError_BufferTooSmall\n缓冲区过小\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	case hyError_InvalidState:
		MessageBox(NULL, L"hyError_InvalidState\n设备状态无效\n请确认设备连接状态并手动重启SteamVR", L"错误", MB_OK);
		break;
	default:
		break;
	}
	if (result>=100&&MessageBox(NULL, L"点击确定可以一键结束hypereal所有进程", L"帮助", MB_YESNO) == IDYES) {
		while (killProcessByName(L"bkdrop.exe")|| killProcessByName(L"HvrService.exe")|| killProcessByName(L"HyperealVR.exe") || killProcessByName(L"HvrCaptain.exe") || killProcessByName(L"HvrPlatformService.exe")){
			Sleep(50);
		}
	}
}

void Boardcast() {
	const TCHAR szOperation[] = _T("open");
	wchar_t* szAddress = (wchar_t*)L"https://github.com/lixiangwuxian/HyperealDriverTest";
	int result=MessageBox(NULL, L"2022/9/22 release1.0。\n\
更新内容请于Github页面查看\n\
Created By lixiangwuxian@github\n\
点击否可打开此驱动Github页面检查更新\n\
点击是可打开作者的bilibili主页（求三连）\n\
什么都不点即可继续使用"\
, L"提示", MB_YESNO);
	if (result == IDNO) {
		ShellExecute(NULL, szOperation, szAddress, NULL, NULL, SW_SHOWNORMAL);
	}
	if (result == IDYES) {
		szAddress = (wchar_t*)L"https://space.bilibili.com/12770378";
		ShellExecute(NULL, szOperation, szAddress, NULL, NULL, SW_SHOWNORMAL);
	}
}

void ServerDriver::Cleanup() {
	delete this->HyTrackingDevice;
#ifdef USE_HMD
	delete this->HyHead;
#endif // USE_HMD
	delete this->HyLeftController;
	delete this->HyRightController;

	this->HyTrackingDevice=NULL;
#ifdef USE_HMD
	this->HyHead = NULL;
#endif // USE_HMD
	this->HyLeftController = NULL;
	this->HyRightController = NULL;
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
		duration = fmaxf(15,data.fDurationSeconds*1000);
		amplitude = fmaxf(0.3, data.fAmplitude);
		amplitude = fminf(1, data.fAmplitude);
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
	while (killProcessByName(L"bkdrop.exe")) {
		Sleep(3000);
	}
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
#ifndef USE_HMD
	frameID = new uint32_t;
	*frameID = 0;
#endif // !USE_HMD
	while (m_bEventThreadRunning) {
		HyTrackingDevice->GetTrackingState(HY_SUBDEV_CONTROLLER_LEFT, *frameID, trackInform);
		UpdateHyPose(trackInform, true);
		HyTrackingDevice->GetTrackingState(HY_SUBDEV_CONTROLLER_RIGHT, *frameID, trackInform);
		UpdateHyPose(trackInform, false);
		Sleep(1);
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