#include "openvr_driver.h"
#include "HYController.h"
#include <Hypereal_VR.h>
using namespace vr;

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
	void UpdateHyPose(const HyTrackingState& newData,bool leftOrRight);
	void UpdateHaptic(VREvent_t& eventHandle);
	void UpdateHyKey(HySubDevice device, HyInputState type);
	void UpdateControllerBatteryThread();
private:
	HyDevice* HyTrackingDevice= nullptr;
	HyController* HyLeftController;
	HyController* HyRightController;
	
	HyPose LastPoseL;
	HyPose LastPoseR;

	HyTrackingState trackInform;

	bool m_bEventThreadRunning;
	std::thread updatePoseThreadWorker;
	std::thread send_haptic_thread_worker;
	std::thread updateKeyThreadWorker;
	std::thread checkBatteryThreadWorker;

	
	void UpdatePoseThread();
	void Send_haptic_event_thread();
	void UpdateKeyThread();
};
