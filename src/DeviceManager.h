#pragma once
#include <juce_core/juce_core.h>
#include <array>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "GuitarProfile.h"

// ============================================================
// DeviceManager.h  –  Manage multiple MIDI devices
// Supports up to MAX_DEVICES (8) simultaneous MIDI connections
// ============================================================

// Maximum number of supported devices
static constexpr int MAX_DEVICES = 8;

// Device states
enum class DeviceState {
    Disconnected,
    Connecting,
    Connected,
    Error,
    Closed
};

// Protocol types
enum class DeviceProtocol {
    MIDI1_0,
    MIDI2_0,
    Unknown
};

// Device information structure
struct MIDIDeviceInfo {
    int deviceID;
    juce::String deviceName;
    juce::String manufacturer;
    juce::String productName;
    DeviceState state;
    DeviceProtocol protocol;
    bool supportsMIDI_CI;
    bool supportsHighResVelocity;
    bool supportsHighResPitchBend;
    uint8_t numChannels;
    uint8_t numGroups;
    
    MIDIDeviceInfo()
        : deviceID(-1)
        , state(DeviceState::Disconnected)
        , protocol(DeviceProtocol::Unknown)
        , supportsMIDI_CI(false)
        , supportsHighResVelocity(false)
        , supportsHighResPitchBend(false)
        , numChannels(16)
        , numGroups(16)
    {}
};

// Callback types for device events
using DeviceConnectionCallback = std::function<void(int deviceID, bool connected)>;
using DeviceErrorCallback = std::function<void(int deviceID, const juce::String& error)>;
using MIDIMessageCallback = std::function<void(int deviceID, const void* data, size_t size)>;

// Device Manager class
class DeviceManager {
public:
    DeviceManager();
    ~DeviceManager();
    
    // ==================== Device Discovery ====================
    
    // Scan for available MIDI devices
    std::vector<MIDIDeviceInfo> scanDevices();
    
    // Get device by ID
    MIDIDeviceInfo* getDevice(int deviceID);
    const MIDIDeviceInfo* getDevice(int deviceID) const;
    
    // Get number of active devices
    int getActiveDeviceCount() const;
    
    // ==================== Device Connection ====================
    
    // Open a device by ID
    bool openDevice(int deviceID);
    
    // Close a device by ID
    bool closeDevice(int deviceID);
    
    // Close all devices
    void closeAllDevices();
    
    // Connect to device (with MIDI-CI negotiation)
    bool connectToDevice(int deviceID, bool preferMIDI2 = true);
    
    // Disconnect from device
    bool disconnectDevice(int deviceID);
    
    // ==================== Device State ====================
    
    // Get device state
    DeviceState getDeviceState(int deviceID) const;
    
    // Get device protocol
    DeviceProtocol getDeviceProtocol(int deviceID) const;
    
    // Check if device is connected
    bool isDeviceConnected(int deviceID) const;
    
    // Check if device supports MIDI-CI
    bool deviceSupportsMIDI_CI(int deviceID) const;
    
    // ==================== MIDI Messaging ====================
    
    // Send MIDI message to specific device
    bool sendMessage(int deviceID, const void* data, size_t size);
    
    // Send MIDI message to all connected devices
    void broadcastMessage(const void* data, size_t size);
    
    // Send MIDI 2.0 UMP to specific device
    bool sendUMP(int deviceID, uint64_t ump);
    
    // Send MIDI 2.0 UMP to all devices
    void broadcastUMP(uint64_t ump);
    
    // ==================== MIDI-CI Profile Management ====================
    
    // Enable guitar profile on device
    bool enableGuitarProfile(int deviceID);
    
    // Disable guitar profile on device
    bool disableGuitarProfile(int deviceID);
    
    // Get guitar profile for device
    GuitarProfile* getGuitarProfile(int deviceID);
    
    // ==================== Callbacks ====================
    
    // Set connection callback
    void setConnectionCallback(DeviceConnectionCallback callback);
    
    // Set error callback
    void setErrorCallback(DeviceErrorCallback callback);
    
    // Set MIDI message callback
    void setMIDIMessageCallback(MIDIMessageCallback callback);
    
    // ==================== Configuration ====================
    
    // Set preferred protocol for new connections
    void setPreferredProtocol(DeviceProtocol protocol);
    DeviceProtocol getPreferredProtocol() const;
    
    // Set auto-reconnect preference
    void setAutoReconnect(bool autoReconnect);
    bool getAutoReconnect() const;
    
    // Set MIDI-CI discovery enabled
    void setMIDICIEnabled(bool enabled);
    bool isMIDICIEnabled() const;
    
    // ==================== Utility ====================
    
    // Get list of connected device IDs
    std::vector<int> getConnectedDeviceIDs() const;
    
    // Get last error message
    juce::String getLastError() const;
    
    // Clear last error
    void clearLastError();
    
    // Force MIDI 1.0 mode on device
    bool forceMIDI1(int deviceID);
    
    // Force MIDI 2.0 mode on device
    bool forceMIDI2(int deviceID);
    
private:
    // Internal device data
    struct DeviceData {
        MIDIDeviceInfo info;
        std::unique_ptr<GuitarProfile> guitarProfile;
        bool guitarProfileEnabled;
        juce::String lastError;
        
        DeviceData() : guitarProfileEnabled(false) {}
    };
    
    std::array<DeviceData, MAX_DEVICES> devices;
    int activeDeviceCount;
    
    DeviceProtocol preferredProtocol;
    bool autoReconnectEnabled;
    bool midiCIEnabled;
    
    juce::String lastErrorMessage;
    
    // Callbacks
    DeviceConnectionCallback onConnection;
    DeviceErrorCallback onError;
    MIDIMessageCallback onMIDIMessage;
    
    // Internal methods
    int findFreeDeviceSlot() const;
    bool validateDeviceID(int deviceID) const;
    void notifyConnection(int deviceID, bool connected);
    void notifyError(int deviceID, const juce::String& error);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DeviceManager)
};

// ============================================================
// DeviceManager Inline Methods
// ============================================================

inline DeviceManager::DeviceManager()
    : activeDeviceCount(0)
    , preferredProtocol(DeviceProtocol::MIDI2_0)
    , autoReconnectEnabled(false)
    , midiCIEnabled(true)
{
    // Initialize device slots
    for (int i = 0; i < MAX_DEVICES; ++i) {
        devices[i].info.deviceID = i;
        devices[i].info.deviceName = "Device " + juce::String(i + 1);
    }
}

inline DeviceManager::~DeviceManager()
{
    closeAllDevices();
}

inline std::vector<MIDIDeviceInfo> DeviceManager::scanDevices()
{
    std::vector<MIDIDeviceInfo> result;
    
    // In a real implementation, this would query the system's MIDI drivers
    // For now, return placeholder devices for testing
    
    for (int i = 0; i < MAX_DEVICES; ++i) {
        if (devices[i].info.state != DeviceState::Disconnected) {
            result.push_back(devices[i].info);
        }
    }
    
    return result;
}

inline MIDIDeviceInfo* DeviceManager::getDevice(int deviceID)
{
    if (!validateDeviceID(deviceID)) return nullptr;
    return &devices[deviceID].info;
}

inline const MIDIDeviceInfo* DeviceManager::getDevice(int deviceID) const
{
    if (!validateDeviceID(deviceID)) return nullptr;
    return &devices[deviceID].info;
}

inline int DeviceManager::getActiveDeviceCount() const
{
    return activeDeviceCount;
}

inline bool DeviceManager::openDevice(int deviceID)
{
    if (!validateDeviceID(deviceID)) {
        lastErrorMessage = "Invalid device ID";
        return false;
    }
    
    auto& dev = devices[deviceID];
    
    if (dev.info.state == DeviceState::Connected) {
        return true;  // Already open
    }
    
    dev.info.state = DeviceState::Connecting;
    
    // In a real implementation, this would open the actual MIDI port
    // For now, simulate connection
    
    dev.info.state = DeviceState::Connected;
    activeDeviceCount++;
    
    notifyConnection(deviceID, true);
    return true;
}

inline bool DeviceManager::closeDevice(int deviceID)
{
    if (!validateDeviceID(deviceID)) {
        lastErrorMessage = "Invalid device ID";
        return false;
    }
    
    auto& dev = devices[deviceID];
    
    if (dev.info.state == DeviceState::Disconnected) {
        return true;  // Already closed
    }
    
    // Disable guitar profile if enabled
    if (dev.guitarProfileEnabled) {
        dev.guitarProfileEnabled = false;
    }
    
    dev.info.state = DeviceState::Disconnected;
    dev.info.protocol = DeviceProtocol::Unknown;
    activeDeviceCount--;
    
    notifyConnection(deviceID, false);
    return true;
}

inline void DeviceManager::closeAllDevices()
{
    for (int i = 0; i < MAX_DEVICES; ++i) {
        if (devices[i].info.state == DeviceState::Connected) {
            closeDevice(i);
        }
    }
}

inline bool DeviceManager::connectToDevice(int deviceID, bool preferMIDI2)
{
    if (!openDevice(deviceID)) {
        return false;
    }
    
    auto& dev = devices[deviceID];
    
    // In a real implementation, this would perform MIDI-CI negotiation
    // For now, set protocol based on preference
    
    if (preferMIDI2 && midiCIEnabled) {
        dev.info.protocol = DeviceProtocol::MIDI2_0;
        dev.info.supportsMIDI_CI = true;
        dev.info.supportsHighResVelocity = true;
        dev.info.supportsHighResPitchBend = true;
    } else {
        dev.info.protocol = DeviceProtocol::MIDI1_0;
        dev.info.supportsMIDI_CI = false;
        dev.info.supportsHighResVelocity = false;
        dev.info.supportsHighResPitchBend = false;
    }
    
    return true;
}

inline bool DeviceManager::disconnectDevice(int deviceID)
{
    return closeDevice(deviceID);
}

inline DeviceState DeviceManager::getDeviceState(int deviceID) const
{
    if (!validateDeviceID(deviceID)) return DeviceState::Disconnected;
    return devices[deviceID].info.state;
}

inline DeviceProtocol DeviceManager::getDeviceProtocol(int deviceID) const
{
    if (!validateDeviceID(deviceID)) return DeviceProtocol::Unknown;
    return devices[deviceID].info.protocol;
}

inline bool DeviceManager::isDeviceConnected(int deviceID) const
{
    return getDeviceState(deviceID) == DeviceState::Connected;
}

inline bool DeviceManager::deviceSupportsMIDI_CI(int deviceID) const
{
    if (!validateDeviceID(deviceID)) return false;
    return devices[deviceID].info.supportsMIDI_CI;
}

inline bool DeviceManager::sendMessage(int deviceID, const void* data, size_t size)
{
    if (!isDeviceConnected(deviceID)) {
        lastErrorMessage = "Device not connected";
        return false;
    }
    
    // In a real implementation, this would send to the MIDI driver
    juce::ignoreUnused(data, size, deviceID);
    return true;
}

inline void DeviceManager::broadcastMessage(const void* data, size_t size)
{
    for (int i = 0; i < MAX_DEVICES; ++i) {
        if (isDeviceConnected(i)) {
            sendMessage(i, data, size);
        }
    }
}

inline bool DeviceManager::sendUMP(int deviceID, uint64_t ump)
{
    if (!isDeviceConnected(deviceID)) {
        lastErrorMessage = "Device not connected";
        return false;
    }
    
    auto& dev = devices[deviceID];
    if (dev.info.protocol != DeviceProtocol::MIDI2_0) {
        lastErrorMessage = "Device not in MIDI 2.0 mode";
        return false;
    }
    
    // In a real implementation, this would send the UMP to the device
    juce::ignoreUnused(ump);
    return true;
}

inline void DeviceManager::broadcastUMP(uint64_t ump)
{
    for (int i = 0; i < MAX_DEVICES; ++i) {
        if (isDeviceConnected(i) && devices[i].info.protocol == DeviceProtocol::MIDI2_0) {
            sendUMP(i, ump);
        }
    }
}

inline bool DeviceManager::enableGuitarProfile(int deviceID)
{
    if (!isDeviceConnected(deviceID)) {
        lastErrorMessage = "Device not connected";
        return false;
    }
    
    auto& dev = devices[deviceID];
    
    // Create guitar profile if not exists
    if (!dev.guitarProfile) {
        dev.guitarProfile = std::make_unique<GuitarProfile>();
    }
    
    // In a real implementation, send MIDI-CI profile enabled message
    dev.guitarProfileEnabled = true;
    
    return true;
}

inline bool DeviceManager::disableGuitarProfile(int deviceID)
{
    if (!validateDeviceID(deviceID)) {
        lastErrorMessage = "Invalid device ID";
        return false;
    }
    
    auto& dev = devices[deviceID];
    
    // In a real implementation, send MIDI-CI profile disabled message
    dev.guitarProfileEnabled = false;
    
    return true;
}

inline GuitarProfile* DeviceManager::getGuitarProfile(int deviceID)
{
    if (!validateDeviceID(deviceID)) return nullptr;
    return devices[deviceID].guitarProfile.get();
}

inline void DeviceManager::setConnectionCallback(DeviceConnectionCallback callback)
{
    onConnection = callback;
}

inline void DeviceManager::setErrorCallback(DeviceErrorCallback callback)
{
    onError = callback;
}

inline void DeviceManager::setMIDIMessageCallback(MIDIMessageCallback callback)
{
    onMIDIMessage = callback;
}

inline void DeviceManager::setPreferredProtocol(DeviceProtocol protocol)
{
    preferredProtocol = protocol;
}

inline DeviceProtocol DeviceManager::getPreferredProtocol() const
{
    return preferredProtocol;
}

inline void DeviceManager::setAutoReconnect(bool autoReconnect)
{
    autoReconnectEnabled = autoReconnect;
}

inline bool DeviceManager::getAutoReconnect() const
{
    return autoReconnectEnabled;
}

inline void DeviceManager::setMIDICIEnabled(bool enabled)
{
    midiCIEnabled = enabled;
}

inline bool DeviceManager::isMIDICIEnabled() const
{
    return midiCIEnabled;
}

inline std::vector<int> DeviceManager::getConnectedDeviceIDs() const
{
    std::vector<int> result;
    for (int i = 0; i < MAX_DEVICES; ++i) {
        if (isDeviceConnected(i)) {
            result.push_back(i);
        }
    }
    return result;
}

inline juce::String DeviceManager::getLastError() const
{
    return lastErrorMessage;
}

inline void DeviceManager::clearLastError()
{
    lastErrorMessage.clear();
}

inline bool DeviceManager::forceMIDI1(int deviceID)
{
    if (!validateDeviceID(deviceID)) return false;
    
    auto& dev = devices[deviceID];
    dev.info.protocol = DeviceProtocol::MIDI1_0;
    dev.info.supportsMIDI_CI = false;
    dev.info.supportsHighResVelocity = false;
    dev.info.supportsHighResPitchBend = false;
    
    return true;
}

inline bool DeviceManager::forceMIDI2(int deviceID)
{
    if (!validateDeviceID(deviceID)) return false;
    
    auto& dev = devices[deviceID];
    dev.info.protocol = DeviceProtocol::MIDI2_0;
    dev.info.supportsMIDI_CI = true;
    dev.info.supportsHighResVelocity = true;
    dev.info.supportsHighResPitchBend = true;
    
    return true;
}

inline int DeviceManager::findFreeDeviceSlot() const
{
    for (int i = 0; i < MAX_DEVICES; ++i) {
        if (devices[i].info.state == DeviceState::Disconnected) {
            return i;
        }
    }
    return -1;
}

inline bool DeviceManager::validateDeviceID(int deviceID) const
{
    return deviceID >= 0 && deviceID < MAX_DEVICES;
}

inline void DeviceManager::notifyConnection(int deviceID, bool connected)
{
    if (onConnection) {
        onConnection(deviceID, connected);
    }
}

inline void DeviceManager::notifyError(int deviceID, const juce::String& error)
{
    devices[deviceID].lastError = error;
    if (onError) {
        onError(deviceID, error);
    }
}
