
#include <windows.h>
#include <math.h>
#include <thread>
#include "openvr_driver.h"
#include "Hypereal_VR.h"
#include <driverlog.h>


using namespace vr;

class HyController : public ITrackedDeviceServerDriver
{
public:
	HyController(std::string id, ETrackedControllerRole type,HyDevice *Device);
	~HyController();
	virtual EVRInitError Activate(uint32_t unObjectId);
	virtual void Deactivate();
	virtual void EnterStandby();
	virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize);

	virtual DriverPose_t GetPose();

	PropertyContainerHandle_t GetPropertyContainer();

	void SendButtonUpdate(HyInputState type);

	std::string GetSerialNumber();
	
	void UpdatePose(HyTrackingState ctrData);

	void UpdateBattery(int value);

	virtual void* GetComponent(const char* pchComponentNameAndVersion) {
		if (_stricmp(pchComponentNameAndVersion, ITrackedDeviceServerDriver_Version) == 0) {
			return static_cast<ITrackedDeviceServerDriver*>(this);
		}
		return nullptr;
	}

private:
	HyDevice* ControllerDevice;
	void initPos();
	void InitEventHandler();
	DriverPose_t GetPose(HyTrackingState ctrData);
	VRControllerState_t m_ControllerState;
	vr::DriverPose_t  m_Pose;
	vr::TrackedDeviceIndex_t VrObjectId;
	vr::PropertyContainerHandle_t m_ulPropertyContainer;

	//ContolerData Data update To Openvr
	vr::VRInputComponentHandle_t  m_system;			  // "/input/system/click"
	vr::VRInputComponentHandle_t  m_grip;		      // "/input/grip/click"
	vr::VRInputComponentHandle_t  m_grip_value;		  // "/input/grip/value"
	vr::VRInputComponentHandle_t  m_application_menu; // "/input/application_menu/click"
	vr::VRInputComponentHandle_t  m_trigger;          // "/input/trigger/click"
	vr::VRInputComponentHandle_t  m_trigger_value;    // "/input/trigger/value"
	vr::VRInputComponentHandle_t  m_trackpad;		  // "/input/trackpad/click"
	vr::VRInputComponentHandle_t  m_touch;            // "/input/trackpad/touch"
	vr::VRInputComponentHandle_t  m_trackpadx;		  // "/input/trackpad/x"
	vr::VRInputComponentHandle_t  m_trackpady;		  // "/input/trackpad/y"
	vr::VRInputComponentHandle_t  m_out_Haptic;       // "/output/haptic"


	std::string m_sSerialNumber;
	std::string m_sModelNumber;
	ETrackedControllerRole  m_Type;
};