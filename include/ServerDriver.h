#include "openvr_driver.h"
#include "HYController.h"
#include "driverlog.h"
#include <windows.h>
#include <shellapi.h>
#include <tchar.h>
#ifdef USE_HMD
#include "HYHMD.h"
#endif // USE_HMD

#include <Hypereal_VR.h>
#include <process.h>
#include <Tlhelp32.h>
#include <winbase.h>
#include <string.h>
using namespace vr;

class ServerDriver;

class ServerDriver : public vr::IServerTrackedDeviceProvider
{
public:
	virtual vr::EVRInitError Init(vr::IVRDriverContext* DriverContext) override;
	virtual void Cleanup() override;
	virtual const char* const* GetInterfaceVersions() override;
	virtual void RunFrame() override;
	virtual bool ShouldBlockStandbyMode() override;
	virtual void EnterStandby() override;
	virtual void LeaveStandby() override;
	//static ServerDriver* self;
private:
	HyController* HyLeftController = nullptr;
	HyController* HyRightController = nullptr;
	void UpdateHaptic(VREvent_t& eventHandle);
	void UpdateHyKey(HySubDevice device, HyInputState type);
	void UpdateControllerBatteryThread();
	HyDevice* HyTrackingDevice= nullptr;

#ifdef USE_HMD
	HyHMD* HyHead=nullptr;
#endif // USE_HMD

	HyTrackingState trackInform;

	bool m_bEventThreadRunning;
	std::thread m_tUpdateControllerThreadWorker;
	std::thread m_tCheckBatteryThreadWorker;

	volatile uint32_t* m_pframeID;
	
	void UpdateControllerThread();
	void Send_haptic_event_thread();
	void UpdateKeyThread();
	void UpdateHyControllerState(const HyTrackingState& newData, bool leftOrRight);
};
