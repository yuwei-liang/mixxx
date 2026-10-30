#include "controllers/controllermanager.h"

#include <gtest/gtest.h>

#include "test/mixxxtest.h"

class ControllerManagerTest : public MixxxTest {
};

// ~MixxxMainWindow shuts the controllers down before it destroys the
// WaveformWidgetFactory the controller screens render with, and
// CoreServices::finalize() does it again before releasing everything else.
TEST_F(ControllerManagerTest, shutdownCanBeCalledMoreThanOnce) {
    auto pManager = std::make_unique<ControllerManager>(config());

    pManager->shutdown();
    // No dangling pointers to the controllers the enumerators deleted.
    EXPECT_TRUE(pManager->getControllers().isEmpty());

    // The thread is gone: this must return at once rather than block.
    pManager->shutdown();
    pManager.reset();
}
