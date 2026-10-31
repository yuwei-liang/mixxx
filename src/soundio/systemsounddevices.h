#pragma once

#include <QStringList>
#include <optional>

namespace mixxx {

/// Names of the sound devices the operating system lists right now, asked
/// from the OS directly because PortAudio only enumerates once per
/// initialization. Only implemented for Core Audio (macOS); std::nullopt
/// elsewhere.
std::optional<QStringList> systemSoundDeviceNames();

/// The configured device names that were listed before and are not listed
/// now, i.e. the devices that just went away. A configured name that was never
/// listed (e.g. a name the OS spells differently) is not reported.
QStringList soundDevicesThatWentAway(const QStringList& configured,
        const QStringList& listedBefore,
        const QStringList& listedNow);

/// Whether every one of names is listed.
bool allSoundDevicesListed(const QStringList& names, const QStringList& listed);

} // namespace mixxx
