#ifndef PSDUALSHOCK4_CONTROLLER_H
#define PSDUALSHOCK4_CONTROLLER_H

#include "PSMoveConfig.h"
#include "DeviceEnumerator.h"
#include "DeviceInterface.h"
#include "hidapi.h"
#include <string>
#include <vector>
#include <deque>
#include <chrono>

struct PSDualShock4HIDDetails {
    std::string Device_path;
    hid_device *Handle;
    std::string Bt_addr;      // The bluetooth address of the controller
    std::string Host_bt_addr; // The bluetooth address of the adapter registered with the controller
};

struct PSDualShock4DataInput;   // See .cpp for declaration
struct PSDualShock4DataOutput;  // See .cpp for declaration

class PSDualShock4ControllerConfig : public PSMoveConfig
{
public:
    static const int CONFIG_VERSION;

    PSDualShock4ControllerConfig(const std::string &fnamebase = "PSDualShock4ControllerConfig")
        : PSMoveConfig(fnamebase)
        , is_valid(false)
        , version(CONFIG_VERSION)
        , accelerometer_fit_error(0.f)
        , gyro_fit_error(0.f)
        , max_poll_failure_count(100)
        , prediction_time(0.f)
    {
        // Accelerometer defaults computed from accelerometer calibration in the config tool
        accelerometer_gain.i = 0.000173128647f;
        accelerometer_gain.j = 0.00013985172f;
        accelerometer_gain.k = 0.000100948688f;
        
        accelerometer_bias.i = -0.0126388613f;
        accelerometer_bias.j = -0.0440602154f;
        accelerometer_bias.k = -0.138208047f;

        // Gyro details not yet calibrated
        gyro_gain.i = 1.f;
        gyro_gain.j = 1.f;
        gyro_gain.k = 1.f;

        gyro_bias.i = 0.f;
        gyro_bias.j = 0.f;
        gyro_bias.k = 0.f;
    };

    virtual const boost::property_tree::ptree config2ptree();
    virtual void ptree2config(const boost::property_tree::ptree &pt);

    bool is_valid;
    long version;

    // calibrated_acc= raw_acc*acc_gain + acc_bias
    CommonDeviceVector accelerometer_gain;
    CommonDeviceVector accelerometer_bias;
    float accelerometer_fit_error;

    // calibrated_gyro= raw_gyro*gyro_gain + gyro_bias
    CommonDeviceVector gyro_gain;
    CommonDeviceVector gyro_bias;
    float gyro_fit_error;

    long max_poll_failure_count;
    float prediction_time;
};

struct PSDualShock4ControllerState : public CommonControllerState
{
    int RawSequence;                               // 6-bit  (counts up by 1 per report)

    unsigned int RawTimeStamp;                     // 16-bit (time since ?, units?)

    float LeftAnalogX;  // [-1.f, 1.f]
    float LeftAnalogY;  // [-1.f, 1.f]
    float RightAnalogX;  // [-1.f, 1.f]
    float RightAnalogY;  // [-1.f, 1.f]
    float LeftTrigger;  // [0.f, 1.f]
    float RightTrigger;  // [0.f, 1.f]

    ButtonState DPad_Up;
    ButtonState DPad_Down;
    ButtonState DPad_Left;
    ButtonState DPad_Right;

    ButtonState Square;
    ButtonState Cross;
    ButtonState Circle;
    ButtonState Triangle;

    ButtonState L1;
    ButtonState R1;
    ButtonState L2;
    ButtonState R2;
    ButtonState L3;
    ButtonState R3;

    ButtonState Share;
    ButtonState Options;

    ButtonState PS;
    ButtonState TrackPadButton;

    int RawAccelerometer[3]; // Raw 12-bit Accelerometer Value
    int RawGyro[3]; // Raw 16-bit Gyroscope Value

    CommonDeviceVector CalibratedAccelerometer; // units of g (where 1g = 9.8m/s^2)
    CommonDeviceVector CalibratedGyro; // rad/s

    PSDualShock4ControllerState()
    {
        clear();
    }

    void clear()
    {
        CommonControllerState::clear();

        RawSequence = 0;
        RawTimeStamp = 0;

        DeviceType = PSDualShock4;

        LeftAnalogX = 0.f;
        LeftAnalogY = 0.f;
        RightAnalogX = 0.f;
        RightAnalogY = 0.f;
        LeftTrigger= 0.f;
        RightTrigger= 0.f;

        DPad_Up = Button_UP;
        DPad_Down = Button_UP;
        DPad_Left = Button_UP;
        DPad_Right = Button_UP;

        Square = Button_UP;
        Cross = Button_UP;
        Circle = Button_UP;
        Triangle = Button_UP;

        L1 = Button_UP;
        R1 = Button_UP;
        L2 = Button_UP;
        R2 = Button_UP;
        L3 = Button_UP;
        R3 = Button_UP;

        Share = Button_UP;
        Options = Button_UP;

        PS = Button_UP;
        TrackPadButton = Button_UP;

        memset(RawAccelerometer, 0, sizeof(int) * 3);
        memset(RawGyro, 0, sizeof(int) * 3);

        CalibratedAccelerometer.clear();
        CalibratedGyro.clear();
    }
};

class PSDualShock4Controller : public IControllerInterface {
public:
    PSDualShock4Controller();
    ~PSDualShock4Controller();

    // PSMoveController
    bool open(); // Opens the first HID device for the controller

    // -- IDeviceInterface
    virtual bool matchesDeviceEnumerator(const DeviceEnumerator *enumerator) const override;
    virtual bool open(const DeviceEnumerator *enumerator) override;
    virtual bool getIsOpen() const override;
    virtual bool getIsReadyToPoll() const override;
    virtual IDeviceInterface::ePollResult poll() override;
    virtual void close() override;
    virtual long getMaxPollFailureCount() const override;
    virtual CommonDeviceState::eDeviceType getDeviceType() const override;
    virtual const CommonDeviceState * getState(int lookBack = 0) const override;

    // -- IControllerInterface
    virtual bool setHostBluetoothAddress(const std::string &address) override;
    virtual bool getIsBluetooth() const override;
    virtual std::string getUSBDevicePath() const override;
    virtual std::string getAssignedHostBluetoothAddress() const override;
    virtual std::string getSerial() const override;
    virtual const std::tuple<unsigned char, unsigned char, unsigned char> getColour() const override;
    virtual void getTrackingShape(CommonDeviceTrackingShape &outTrackingShape) const override;

    // -- Getters
    inline const PSDualShock4ControllerConfig *getConfig() const
    {
        return &cfg;
    }
    inline PSDualShock4ControllerConfig *getConfigMutable()
    {
        return &cfg;
    }
    static CommonDeviceState::eDeviceType getDeviceTypeStatic()
    {
        return CommonDeviceState::PSDualShock4;
    }

    // -- Setters
    bool setLED(unsigned char r, unsigned char g, unsigned char b);
    bool setLeftRumbleIntensity(unsigned char value);
    bool setRightRumbleIntensity(unsigned char value);

private:
    bool getBTAddressesViaUSB(std::string& host, std::string& controller);
    void clearAndWriteDataOut();
    bool writeDataOut();                            // Setters will call this

    // Constant while a controller is open
    PSDualShock4ControllerConfig cfg;
    PSDualShock4HIDDetails HIDDetails;
    bool IsBluetooth;                               // true if valid serial number on device opening

    // Cached Setter State
    unsigned char LedR, LedG, LedB;
    unsigned char RumbleRight; // Weak
    unsigned char RumbleLeft; // Strong
    bool bWriteStateDirty;
    std::chrono::time_point<std::chrono::high_resolution_clock> lastWriteStateTime;

    // Read Controller State
    int NextPollSequenceNumber;
    std::deque<PSDualShock4ControllerState> ControllerStates;
    PSDualShock4DataInput* InData;                        // Buffer to read hidapi reports into
    PSDualShock4DataOutput* OutData;                      // Buffer to write hidapi reports out from
};
#endif // PSDUALSHOCK4_CONTROLLER_H