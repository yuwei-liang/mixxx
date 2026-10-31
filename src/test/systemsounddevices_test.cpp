#include "soundio/systemsounddevices.h"

#include <gtest/gtest.h>

namespace {

const QStringList kConfigured = {QStringLiteral("Traktor Kontrol S5")};
const QStringList kWithS5 = {QStringLiteral("MacBook Pro Speakers"),
        QStringLiteral("Traktor Kontrol S5")};
const QStringList kWithoutS5 = {QStringLiteral("MacBook Pro Speakers")};

TEST(SystemSoundDevicesTest, SwitchingTheS5OffMakesItGoAway) {
    EXPECT_EQ(kConfigured,
            mixxx::soundDevicesThatWentAway(kConfigured, kWithS5, kWithoutS5));
}

TEST(SystemSoundDevicesTest, NothingGoesAwayWhileItStaysListed) {
    EXPECT_TRUE(mixxx::soundDevicesThatWentAway(kConfigured, kWithS5, kWithS5).isEmpty());
}

TEST(SystemSoundDevicesTest, TheFirstCheckHasNothingToCompareWith) {
    EXPECT_TRUE(mixxx::soundDevicesThatWentAway(kConfigured, {}, kWithoutS5).isEmpty());
}

TEST(SystemSoundDevicesTest, ANameTheOsNeverListedIsNotReportedGone) {
    const QStringList configured = {QStringLiteral("Traktor Kontrol S5 (2)")};
    EXPECT_TRUE(mixxx::soundDevicesThatWentAway(configured, kWithS5, kWithoutS5).isEmpty());
}

TEST(SystemSoundDevicesTest, AnotherDeviceGoingAwayIsNotOurs) {
    EXPECT_TRUE(mixxx::soundDevicesThatWentAway(kConfigured, kWithS5, {kConfigured})
                    .isEmpty());
}

TEST(SystemSoundDevicesTest, LostDevicesAreBackOnlyWhenAllAreListed) {
    EXPECT_TRUE(mixxx::allSoundDevicesListed(kConfigured, kWithS5));
    EXPECT_FALSE(mixxx::allSoundDevicesListed(kConfigured, kWithoutS5));
    const QStringList twoDevices = {
            QStringLiteral("Traktor Kontrol S5"), QStringLiteral("Scarlett 2i2 USB")};
    EXPECT_FALSE(mixxx::allSoundDevicesListed(twoDevices, kWithS5));
}

#ifdef __APPLE__
TEST(SystemSoundDevicesTest, CoreAudioListsDevices) {
    // Any Mac lists at least its own speakers or a virtual device; on a CI box
    // without audio the call must still answer instead of failing.
    EXPECT_TRUE(mixxx::systemSoundDeviceNames().has_value());
}
#endif

} // namespace
