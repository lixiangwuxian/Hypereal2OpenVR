#include "openvr_driver.h"
#include "ServerDriver.h"



static std::shared_ptr<ServerDriver> mainServerDirver;


extern "C" __declspec(dllexport) void* HmdDriverFactory(const char* InterfaceName, int* ReturnCode) {
	if (std::strcmp(InterfaceName, vr::IServerTrackedDeviceProvider_Version) == 0) {
		if (!mainServerDirver) {
			mainServerDirver = std::make_shared<ServerDriver>();
		}
		return mainServerDirver.get();
	}
	else {
		*ReturnCode = vr::VRInitError_Init_InterfaceNotFound;
		return nullptr;
	}
}

std::shared_ptr<ServerDriver> GetDriver() {
	return mainServerDirver;
}
