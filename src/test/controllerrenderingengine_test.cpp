#include "controllers/rendering/controllerrenderingengine.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDomDocument>
#include <QSignalSpy>
#include <QTest>

#include "controllers/controllerenginethreadcontrol.h"
#include "controllers/legacycontrollermappingfilehandler.h"
#include "test/controller_mapping_validation_test.h"
#include "helpers/log_test.h"
#include "test/mixxxtest.h"

using ::testing::_;
using namespace std::chrono_literals;

class ControllerRenderingEngineTest : public MixxxTest {
  public:
    void SetUp() override {
        mixxx::Time::setTestMode(true);
        mixxx::Time::addTestTime(10ms);
    }

    QList<QImage::Format> supportedPixelFormat() const {
        return LegacyControllerMappingFileHandler::kSupportedPixelFormat.values();
    }

  private:
    LogCaptureGuard m_logCaptureGuard;
};

class MockRenderingEngine : public ControllerRenderingEngine {
  public:
    MockRenderingEngine(const LegacyControllerMapping::ScreenInfo& info)
            : ControllerRenderingEngine(info, new ControllerEngineThreadControl){};
};

TEST_F(ControllerRenderingEngineTest, createValidRendererWithSupportedTypes) {
    const auto& supportedPixelFormats = supportedPixelFormat();
    for (const auto& pixelFormat : supportedPixelFormats) {
        MockRenderingEngine screenTest(LegacyControllerMapping::ScreenInfo{
                "",                                                    // identifier
                QSize(0, 0),                                           // size
                10,                                                    // target_fps
                1,                                                     // msaa
                std::chrono::milliseconds(10),                         // splash_off
                pixelFormat,                                           // pixelFormat
                LegacyControllerMapping::ScreenInfo::ColorEndian::Big, // endian
                false,                                                 // reversedColor
                false                                                  // rawData
        });
        EXPECT_TRUE(screenTest.isValid());
        EXPECT_TRUE(screenTest.stop());
    }
}

namespace {
// A controller whose device has been unplugged.
class UnpluggedController : public FakeController {
  protected:
    bool sendBytes(const QByteArray& data) override {
        Q_UNUSED(data);
        return false;
    }
};
} // namespace

// The render thread must not tear itself down when a frame can't be sent:
// finish() deletes the window holding the scene while the controller thread
// may be running it. It asks to be stopped instead.
TEST_F(ControllerRenderingEngineTest, failedSendAsksToStopButKeepsThreadRunning) {
    MockRenderingEngine screen(LegacyControllerMapping::ScreenInfo{
            "",                                                    // identifier
            QSize(0, 0),                                           // size
            10,                                                    // target_fps
            1,                                                     // msaa
            std::chrono::milliseconds(10),                         // splash_off
            QImage::Format_RGB16,                                  // pixelFormat
            LegacyControllerMapping::ScreenInfo::ColorEndian::Big, // endian
            false,                                                 // reversedColor
            false                                                  // rawData
    });
    ASSERT_TRUE(screen.isValid());
    UnpluggedController controller;
    QSignalSpy stopping(&screen, &ControllerRenderingEngine::stopping);
    EXPECT_LOG_MSG(QtWarningMsg, "Unable to send frame to device");

    screen.requestSendingFrameData(&controller, QByteArray("frame"));

    EXPECT_TRUE(stopping.wait(1000));
    EXPECT_TRUE(screen.isRunning());
    ASSERT_ALL_EXPECTED_MSG();
    EXPECT_TRUE(screen.stop());
}
