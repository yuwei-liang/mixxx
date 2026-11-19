#include <gtest/gtest.h>

#include "library/externalinboxpatience.h"

namespace {

const QDateTime kStart = QDateTime::fromSecsSinceEpoch(1'800'000'000);

TEST(ExternalInboxPatienceTest, KeepsAResultSeenForTheFirstTime) {
    // A result written days before Mixxx started is new to this session:
    // it gets its full hour, however old the file is.
    ExternalInboxPatience patience;
    EXPECT_FALSE(patience.giveUp("2640.json", kStart));
}

TEST(ExternalInboxPatienceTest, KeepsItThroughTheHour) {
    ExternalInboxPatience patience;
    patience.giveUp("2640.json", kStart);
    EXPECT_FALSE(patience.giveUp("2640.json", kStart.addSecs(60)));
    EXPECT_FALSE(patience.giveUp("2640.json", kStart.addSecs(3600)));
}

TEST(ExternalInboxPatienceTest, GivesUpAfterAnHourInThisSession) {
    ExternalInboxPatience patience;
    patience.giveUp("2640.json", kStart);
    EXPECT_TRUE(patience.giveUp("2640.json", kStart.addSecs(3601)));
}

TEST(ExternalInboxPatienceTest, EachResultHasItsOwnClock) {
    ExternalInboxPatience patience;
    patience.giveUp("1.json", kStart);
    patience.giveUp("2.json", kStart.addSecs(3000));
    EXPECT_TRUE(patience.giveUp("1.json", kStart.addSecs(3601)));
    EXPECT_FALSE(patience.giveUp("2.json", kStart.addSecs(3601)));
}

TEST(ExternalInboxPatienceTest, AFreshFileUnderAForgottenNameWaitsAfresh) {
    ExternalInboxPatience patience;
    patience.giveUp("2640.json", kStart);
    patience.forget("2640.json");
    EXPECT_FALSE(patience.giveUp("2640.json", kStart.addSecs(7200)));
}

} // namespace
