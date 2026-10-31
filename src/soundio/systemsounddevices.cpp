#include "soundio/systemsounddevices.h"

#ifdef __APPLE__
#include <CoreAudio/CoreAudio.h>
#include <TargetConditionals.h>

#include <vector>
#endif

namespace mixxx {

std::optional<QStringList> systemSoundDeviceNames() {
#if defined(__APPLE__) && TARGET_OS_OSX
    AudioObjectPropertyAddress address = {
            kAudioHardwarePropertyDevices,
            kAudioObjectPropertyScopeGlobal,
            0}; // kAudioObjectPropertyElementMain, only named from macOS 12
    UInt32 size = 0;
    if (AudioObjectGetPropertyDataSize(
                kAudioObjectSystemObject, &address, 0, nullptr, &size) != noErr) {
        return std::nullopt;
    }
    std::vector<AudioObjectID> devices(size / sizeof(AudioObjectID));
    if (AudioObjectGetPropertyData(kAudioObjectSystemObject,
                &address,
                0,
                nullptr,
                &size,
                devices.data()) != noErr) {
        return std::nullopt;
    }
    devices.resize(size / sizeof(AudioObjectID));
    QStringList names;
    // The same property PortAudio names its Core Audio devices after
    address.mSelector = kAudioObjectPropertyName;
    for (const AudioObjectID device : devices) {
        CFStringRef name = nullptr;
        size = sizeof(name);
        if (AudioObjectGetPropertyData(device, &address, 0, nullptr, &size, &name) ==
                        noErr &&
                name) {
            names.append(QString::fromCFString(name));
            CFRelease(name);
        }
    }
    return names;
#else
    return std::nullopt;
#endif
}

QStringList soundDevicesThatWentAway(const QStringList& configured,
        const QStringList& listedBefore,
        const QStringList& listedNow) {
    QStringList gone;
    for (const auto& name : configured) {
        if (listedBefore.contains(name) && !listedNow.contains(name) &&
                !gone.contains(name)) {
            gone.append(name);
        }
    }
    return gone;
}

bool allSoundDevicesListed(const QStringList& names, const QStringList& listed) {
    for (const auto& name : names) {
        if (!listed.contains(name)) {
            return false;
        }
    }
    return true;
}

} // namespace mixxx
