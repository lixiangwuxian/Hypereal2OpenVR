#include "HyController.h"

using namespace  vr;

HyController::HyController(std::string id, ETrackedControllerRole type,HyDevice *Device)
{
	m_ulPropertyContainer = vr::k_ulInvalidPropertyContainer;
	m_sSerialNumber = id;
	if(m_sSerialNumber[0]=='L')
		m_sModelNumber = "{revive_hypereal}/rendermodels/hypereal_controller_left";
	else if(m_sSerialNumber[0]=='R')
		m_sModelNumber = "{revive_hypereal}/rendermodels/hypereal_controller_right";
	m_Type = type;
	ControllerDevice = Device;
}

HyController::~HyController()
{
}

EVRInitError HyController::Activate(uint32_t unObjectId)
{
	VrObjectId = unObjectId;
	initPos();
	m_ulPropertyContainer = vr::VRProperties()->TrackedDeviceToPropertyContainer(VrObjectId);
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, m_sSerialNumber.c_str());
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ModelNumber_String, "Vive");
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_RenderModelName_String, m_sModelNumber.c_str());
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_ManufacturerName_String, "HTC");
	vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_Axis0Type_Int32, k_eControllerAxis_TrackPad);
	vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_Axis1Type_Int32, k_eControllerAxis_Trigger);
	vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_InputProfilePath_String, "{revive_hypereal}/input/hypereal_controller_profile.json");

	if (m_sSerialNumber[0] == 'L') {
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{revive_hypereal}/icons/hypereal_left_controller_off.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{revive_hypereal}/icons/hypereal_left_controller_searching.gif");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{revive_hypereal}/icons/hypereal_left_controller_searching_alert.gif");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{revive_hypereal}/icons/hypereal_left_controller_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{revive_hypereal}/icons/hypereal_left_controller_ready_alert.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{revive_hypereal}/icons/hypereal_left_controller_not_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{revive_hypereal}/icons/hypereal_left_controller_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{revive_hypereal}/icons/hypereal_left_controller_ready_low.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{revive_hypereal}/icons/hypereal_left_controller_ready_alert.png");
	}
	else
	{
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceOff_String, "{revive_hypereal}/icons/hypereal_right_controller_off.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearching_String, "{revive_hypereal}/icons/hypereal_right_controller_searching.gif");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceSearchingAlert_String, "{revive_hypereal}/icons/hypereal_right_controller_searching_alert.gif");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReady_String, "{revive_hypereal}/icons/hypereal_right_controller_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceReadyAlert_String, "{revive_hypereal}/icons/hypereal_right_controller_ready_alert.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceNotReady_String, "{revive_hypereal}/icons/hypereal_right_controller_not_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandby_String, "{revive_hypereal}/icons/hypereal_right_controller_ready.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceAlertLow_String, "{revive_hypereal}/icons/hypereal_right_controller_ready_low.png");
		vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_NamedIconPathDeviceStandbyAlert_String, "{revive_hypereal}/icons/hypereal_right_controller_ready_alert.png");
	}
	//ÊÖ±úÀàÐÍ
	vr::VRProperties()->SetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, m_Type);
	ETrackedPropertyError erro;
	int DevClass = vr::VRProperties()->GetInt32Property(m_ulPropertyContainer, Prop_ControllerRoleHint_Int32, &erro);


	uint64_t reval = vr::ButtonMaskFromId(vr::k_EButton_ApplicationMenu) |
		vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Touchpad) |
		vr::ButtonMaskFromId(vr::k_EButton_SteamVR_Trigger) |
		vr::ButtonMaskFromId(vr::k_EButton_System) |
		vr::ButtonMaskFromId(vr::k_EButton_Grip);
	vr::VRProperties()->SetUint64Property(m_ulPropertyContainer, Prop_SupportedButtons_Uint64, reval);
	InitEventHandler();
	return VRInitError_None;
}


void HyController::InitEventHandler()
{
	//in
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/grip/click", &m_grip);
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trigger/click", &m_trigger);
	vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trigger/value", &m_trigger_value, VRScalarType_Absolute, VRScalarUnits_NormalizedOneSided);
	vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/grip/value", &m_grip_value, VRScalarType_Absolute, VRScalarUnits_NormalizedOneSided);
#ifndef SIMULATE_OCULUS_TOUCH
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/click", &m_trackpad);
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/trackpad/touch", &m_touch);
	vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trackpad/x",&m_trackpadx, VRScalarType_Absolute, VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/trackpad/y",&m_trackpady, VRScalarType_Absolute, VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/system/click", &m_system);
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/application_menu/click", &m_application_menu);
#endif
	//out
	vr::VRDriverInput()->CreateHapticComponent(m_ulPropertyContainer, "/output/haptic", &m_out_Haptic);
#ifdef SIMULATE_OCULUS_TOUCH
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/joystick/click", &m_trackpad);
	vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/joystick/touch", &m_touch);
	vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/joystick/x", &m_trackpadx, VRScalarType_Absolute, VRScalarUnits_NormalizedTwoSided);
	vr::VRDriverInput()->CreateScalarComponent(m_ulPropertyContainer, "/input/joystick/y", &m_trackpady, VRScalarType_Absolute, VRScalarUnits_NormalizedTwoSided);
	if (m_sSerialNumber[0] == 'R') {
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/a/click", &m_system);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/b/click", &m_application_menu);
	}
	else {
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/x/click", &m_system);
		vr::VRDriverInput()->CreateBooleanComponent(m_ulPropertyContainer, "/input/y/click", &m_application_menu);//no system button now, bind one manually
	}
#endif
}


void HyController::Deactivate()
{
	VrObjectId = k_unTrackedDeviceIndexInvalid;
}

void HyController::EnterStandby()
{
}

void HyController::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize)
{
	if (unResponseBufferSize >= 1)
		pchResponseBuffer[0] = 0;
}

DriverPose_t HyController::GetPose()
{
	return m_Pose;
}

PropertyContainerHandle_t HyController::GetPropertyContainer()
{
	return m_ulPropertyContainer;
}

void HyController::initPos()
{
	m_Pose.result = vr::TrackingResult_Running_OK;
	m_Pose.poseIsValid = true;
	m_Pose.willDriftInYaw = true;
	m_Pose.shouldApplyHeadModel = false;
	m_Pose.deviceIsConnected = true;

	m_Pose.poseTimeOffset = 0.0;
	m_Pose.qWorldFromDriverRotation.w =1.0;
	m_Pose.qWorldFromDriverRotation.x = 0.0;
	m_Pose.qWorldFromDriverRotation.y = 0.0;
	m_Pose.qWorldFromDriverRotation.z = 0.0;
	m_Pose.vecWorldFromDriverTranslation[0] = 0.0;
	m_Pose.vecWorldFromDriverTranslation[1] = 0.0;
	m_Pose.vecWorldFromDriverTranslation[2] = 0.0;

	m_Pose.qDriverFromHeadRotation.w = 0.94;
	m_Pose.qDriverFromHeadRotation.x = 0.342;
	m_Pose.qDriverFromHeadRotation.y = 0.0;
	m_Pose.qDriverFromHeadRotation.z = 0.0;

	m_Pose.vecDriverFromHeadTranslation[0] = 0.000;
	m_Pose.vecDriverFromHeadTranslation[1] = 0.000;
	m_Pose.vecDriverFromHeadTranslation[2] = 0.000;

	m_Pose.vecAcceleration[0] = 0.0;
	m_Pose.vecAcceleration[1] = 0.0;
	m_Pose.vecAcceleration[2] = 0.0;
	m_Pose.vecAngularAcceleration[0] = 0.0;
	m_Pose.vecAngularAcceleration[1] = 0.0;
	m_Pose.vecAngularAcceleration[2] = 0.0;
}


void HyController::SendButtonUpdate(HyInputState inputState)
{
	const double fTimeOffset = 0;
	float TriggerValue = 0.0f;
	float GripValue = 0.0f;
	
	if (m_Pose.deviceIsConnected = false) {
		return;//avoid noise
	}

	vr::VRDriverInput()->UpdateBooleanComponent(m_trackpad, (bool)((HY_BUTTON_TOUCHPAD_LEFT +HY_BUTTON_TOUCHPAD_RIGHT) & inputState.m_buttons), 0);
#ifdef SWITCH_KEY
	vr::VRDriverInput()->UpdateBooleanComponent(m_system, (bool)( (HY_BUTTON_HOME_LONGPRESS + HY_BUTTON_MENU_LONGPRESS) & inputState.m_buttons), 0);
	vr::VRDriverInput()->UpdateBooleanComponent(m_application_menu, (bool)((HY_BUTTON_HOME+ HY_BUTTON_MENU) & inputState.m_buttons), 0);
#endif
#ifndef SWITCH_KEY
	vr::VRDriverInput()->UpdateBooleanComponent(m_system, (bool)((HY_BUTTON_HOME + HY_BUTTON_MENU) & inputState.m_buttons), 0);
	vr::VRDriverInput()->UpdateBooleanComponent(m_application_menu, (bool)((HY_BUTTON_HOME_LONGPRESS + HY_BUTTON_MENU_LONGPRESS) & inputState.m_buttons), 0);
#endif

	if (inputState.m_indexTrigger > 0.8) {
		vr::VRDriverInput()->UpdateBooleanComponent(m_trigger, true, 0);
	}
	else {
		vr::VRDriverInput()->UpdateBooleanComponent(m_trigger, false, 0);
	}
	if (inputState.m_sideTrigger > 0.8) {
		vr::VRDriverInput()->UpdateBooleanComponent(m_grip, true, 0);
	}
	else {
		vr::VRDriverInput()->UpdateBooleanComponent(m_grip, false, 0);
	}
	vr::VRDriverInput()->UpdateBooleanComponent(m_touch, (bool)(HY_TOUCH_TOUCHPAD_LEFT + HY_TOUCH_TOUCHPAD_RIGHT & inputState.m_touches), 0);

	vr::VRDriverInput()->UpdateScalarComponent(m_trigger_value, inputState.m_indexTrigger, 0);
	vr::VRDriverInput()->UpdateScalarComponent(m_trackpadx, inputState.m_touchpad.x*1.1 , 0);
	vr::VRDriverInput()->UpdateScalarComponent(m_trackpady, inputState.m_touchpad.y*1.1 , 0);
	vr::VRDriverInput()->UpdateScalarComponent(m_grip_value, inputState.m_sideTrigger, 0);
	
}

DriverPose_t HyController::GetPose(HyTrackingState ctrData)
{
	m_Pose.poseIsValid = true;
	m_Pose.deviceIsConnected = true;
	if (ctrData.m_flags == HY_TRACKING_NONE) {
		m_Pose.poseIsValid = false;
		m_Pose.deviceIsConnected = false;
		return m_Pose;
	}
	if (ctrData.m_flags == HY_TRACKING_ROTATION_TRACKED) {
		m_Pose.poseIsValid = false;
		return m_Pose;
	}
	m_Pose.vecPosition[0] = ctrData.m_pose.m_position.x;
	m_Pose.vecPosition[1] = ctrData.m_pose.m_position.y;
	m_Pose.vecPosition[2] = ctrData.m_pose.m_position.z;
	m_Pose.qRotation.x = ctrData.m_pose.m_rotation.x;
	m_Pose.qRotation.y = ctrData.m_pose.m_rotation.y;
	m_Pose.qRotation.z = ctrData.m_pose.m_rotation.z;
	m_Pose.qRotation.w = ctrData.m_pose.m_rotation.w;
	m_Pose.vecVelocity[0] = ctrData.m_linearVelocity.x;
	m_Pose.vecVelocity[1] = ctrData.m_linearVelocity.y;
	m_Pose.vecVelocity[2] = ctrData.m_linearVelocity.z;
	///m_Pose.vecAngularVelocity[0] = ctrData.m_angularVelocity.x;
	//m_Pose.vecAngularVelocity[1] = ctrData.m_angularVelocity.y;
	//m_Pose.vecAngularVelocity[2] = ctrData.m_angularVelocity.z;//Avoid shaking...these data from sdk is too sharp
	m_Pose.vecAngularAcceleration[0] = ctrData.m_angularAcceleration.x;
	m_Pose.vecAngularAcceleration[1] = ctrData.m_angularAcceleration.y;
	m_Pose.vecAngularAcceleration[2] = ctrData.m_angularAcceleration.z;
	m_Pose.vecAcceleration[0] = ctrData.m_linearAcceleration.x;
	m_Pose.vecAcceleration[1] = ctrData.m_linearAcceleration.y;
	m_Pose.vecAcceleration[2] = ctrData.m_linearAcceleration.z;
	return m_Pose;
}

std::string HyController::GetSerialNumber()
{
	return m_sSerialNumber;
}

void HyController::UpdatePose(HyTrackingState ctrData)
{
	vr::VRServerDriverHost()->TrackedDevicePoseUpdated(VrObjectId, GetPose(ctrData), sizeof(DriverPose_t));
}

void HyController::UpdateBattery(int value)
{
	bool ifBatteryEmpty = false;
	if (value <= 1) {//we only got 0,1,2. 0 for empty, 1 for low, 2 for normal
		ifBatteryEmpty = true;
	}
	else {
		ifBatteryEmpty = false;
	}
	vr::VRProperties()->SetBoolProperty(m_ulPropertyContainer, Prop_DeviceProvidesBatteryStatus_Bool, ifBatteryEmpty);
}
