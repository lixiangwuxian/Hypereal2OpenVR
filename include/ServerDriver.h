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
	void UpdateHaptic(VREvent_t& eventHandle);
	void UpdateHyKey(HySubDevice device, HyInputState type);
	void UpdateControllerBattery();

	HyDevice* m_pHyTrackingDevice= nullptr;
#ifdef USE_HMD
	//HyHMD* m_pHyHead=nullptr;
	std::shared_ptr<HyHMD>  m_pHyHead;
#endif // USE_HMD
	std::shared_ptr<HyController> m_pHyLeftController;
	std::shared_ptr<HyController> m_pHyRightController;
	//HyController* m_pHyLeftController = nullptr;
	//HyController* m_pHyRightController = nullptr;

	HyTrackingState m_trackInform;
	bool m_bEventThreadRunning;
	volatile uint32_t* m_pframeID;
	
	void UpdateControllerThread();
	void UpdateHyControllerState(const HyTrackingState& newData, bool leftOrRight);
};
