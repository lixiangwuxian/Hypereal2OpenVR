#include "ServerDriver.h"
#pragma comment(lib,"shell32.lib")
#pragma comment(lib, "User32.lib")
void ErrorAlarm(HyResult result);
void Boardcast();




bool killProcessByName(const wchar_t* filename){

	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, NULL);
	PROCESSENTRY32 pEntry;
	pEntry.dwSize = sizeof(pEntry);
	BOOL hRes = Process32First(hSnapShot, &pEntry);
	bool foundProcess = false;
	while (hRes){
		if (lstrcmpW(pEntry.szExeFile, filename) == 0){
			foundProcess = true;
			HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, 0,
				(DWORD)pEntry.th32ProcessID);
			if (hProcess != NULL){
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
		if (eError != vr::VRInitError_None){
			return eError;
	}
	InitDriverLog(vr::VRDriverLog());
	HyStartup();
	HyResult ifCreate = HyCreateInterface(HyDevice_InterfaceName, 0, &m_pHyTrackingDevice);
	
	//std::thread::thread(&Boardcast).detach();

	std::thread::thread(&ErrorAlarm, ifCreate).detach();

	while (killProcessByName(L"bkdrop.exe")){
		Sleep(5000);
	}
	
	if (ifCreate >= 100) {//we got an error.. Don't initialize any device or steamvr would crash.
		return vr::VRInitError_None;
	}

#ifdef USE_HMD
	m_pHyHead = new HyHMD("HYHMD@LXWX",m_pHyTrackingDevice);
#endif // USE_HMD
	m_pHyLeftController = new HyController("LctrTEST@LXWX", TrackedControllerRole_LeftHand,m_pHyTrackingDevice);
	m_pHyRightController = new HyController("RctrTEST@LXWX", TrackedControllerRole_RightHand,m_pHyTrackingDevice);

#ifdef USE_HMD
	m_pframeID=m_pHyHead->getFrameIDptr();
	vr::VRServerDriverHost()->TrackedDeviceAdded(m_pHyHead->GetSerialNumber().c_str(), vr::TrackedDeviceClass_HMD, m_pHyHead);
#endif // USE_HMD
	vr::VRServerDriverHost()->TrackedDeviceAdded(m_pHyLeftController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pHyLeftController);
	vr::VRServerDriverHost()->TrackedDeviceAdded(m_pHyRightController->GetSerialNumber().c_str(), vr::TrackedDeviceClass_Controller, m_pHyRightController);

	m_bEventThreadRunning = false;
	if (!m_bEventThreadRunning){
		m_bEventThreadRunning = true;
		std::thread::thread(&ServerDriver::UpdateControllerThread, this).detach();
		//std::thread::thread(&ServerDriver::UpdateControllerBattery, this).detach();
	}
	return vr::VRInitError_None;
}

void ErrorAlarm(HyResult result) {
	switch (result){
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
	int result=MessageBoxW(NULL, L"2022/10/27 release2.0。\n\
似乎所有功能都正常\n\
更新内容请于Github页面查看\n\
Created By lixiangwuxian@github\n"\
, L"提示", MB_OK);
	if (result != IDOK) {
		ShellExecute(NULL, szOperation, szAddress, NULL, NULL, SW_SHOWNORMAL);
	}
}

void ServerDriver::Cleanup() {
	m_bEventThreadRunning = false;
	delete m_pHyTrackingDevice;
#ifdef USE_HMD
	delete m_pHyHead;
#endif // USE_HMD
	delete m_pHyLeftController;
	delete m_pHyRightController;
	VR_CLEANUP_SERVER_DRIVER_CONTEXT();
	HyShutdown();
}

const char* const* ServerDriver::GetInterfaceVersions() {
	return vr::k_InterfaceVersions;
}

bool ServerDriver::ShouldBlockStandbyMode() {
	return true;
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
		if (m_pHyLeftController->GetPropertyContainer() == data.containerHandle) {
			m_pHyTrackingDevice->SetControllerVibration(HY_SUBDEV_CONTROLLER_LEFT,duration, amplitude);
		}
		else if (m_pHyRightController->GetPropertyContainer() == data.containerHandle) {
			m_pHyTrackingDevice->SetControllerVibration(HY_SUBDEV_CONTROLLER_RIGHT, duration, amplitude);
		}
	}
}

void ServerDriver::UpdateControllerThread() {
	while (m_bEventThreadRunning) {
		m_pHyTrackingDevice->GetTrackingState(HY_SUBDEV_CONTROLLER_LEFT, 0, m_trackInform);
		UpdateHyControllerState(m_trackInform, true);
		m_pHyTrackingDevice->GetTrackingState(HY_SUBDEV_CONTROLLER_RIGHT, 0, m_trackInform);
		UpdateHyControllerState(m_trackInform, false);
		HyInputState keyInput;
		m_pHyTrackingDevice->GetControllerInputState(HY_SUBDEV_CONTROLLER_LEFT, keyInput);
		UpdateHyKey(HY_SUBDEV_CONTROLLER_LEFT, keyInput);
		m_pHyTrackingDevice->GetControllerInputState(HY_SUBDEV_CONTROLLER_RIGHT, keyInput);
		UpdateHyKey(HY_SUBDEV_CONTROLLER_RIGHT, keyInput);
		UpdateControllerBattery();
	}
}

void ServerDriver::UpdateHyControllerState(const HyTrackingState& newData, bool leftOrRight) {
	VREvent_t pEventHandle;
	if (vr::VRServerDriverHost()->PollNextEvent(&pEventHandle, sizeof(VREvent_t))){
		UpdateHaptic(pEventHandle);
	}
	if (leftOrRight) {
		m_pHyLeftController->UpdatePose(newData);
	}
	else {
		m_pHyRightController->UpdatePose(newData);
	}
}

inline void ServerDriver::UpdateHyKey(HySubDevice device, HyInputState type)
{
	if (device == HY_SUBDEV_CONTROLLER_LEFT) {
		m_pHyLeftController->SendButtonUpdate(type);
	}
	else if(device == HY_SUBDEV_CONTROLLER_RIGHT){
		m_pHyRightController->SendButtonUpdate(type);
	}
}

inline void ServerDriver::UpdateControllerBattery()
{
	int64_t batteryValue = 3;
	m_pHyTrackingDevice->GetIntValue(HY_PROPERTY_DEVICE_BATTERY_INT, batteryValue, HY_SUBDEV_CONTROLLER_LEFT);
	m_pHyLeftController->UpdateBattery(batteryValue);
	m_pHyTrackingDevice->GetIntValue(HY_PROPERTY_DEVICE_BATTERY_INT, batteryValue, HY_SUBDEV_CONTROLLER_RIGHT);
	m_pHyRightController->UpdateBattery(batteryValue);
}

void ServerDriver::RunFrame() {}
void ServerDriver::EnterStandby() {}
void ServerDriver::LeaveStandby() {}