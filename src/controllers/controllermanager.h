#pragma once

#include <QMutex>
#include <QSharedPointer>
#include <QTimer>
#include <memory>
#include <vector>

#include "controllers/controllerenumerator.h"
#include "preferences/usersettings.h"
#include "util/duration.h"

// Forward declaration(s)
class Controller;
class ControllerLearningEventFilter;
class MappingInfoEnumerator;
class LegacyControllerMapping;
class ControllerEnumerator;

/// Function to sort controllers by name
bool controllerCompare(Controller *a, Controller *b);

/// Manages enumeration/operation/deletion of hardware controllers.
class ControllerManager : public QObject {
    Q_OBJECT
  public:
    explicit ControllerManager(UserSettingsPointer pConfig);
    ~ControllerManager() override;

    static const mixxx::Duration kPollInterval;

    QList<Controller*> getControllers() const;
    QList<Controller*> getControllerList(bool outputDevices=true, bool inputDevices=true);
    ControllerLearningEventFilter* getControllerLearningEventFilter() const;
    QSharedPointer<MappingInfoEnumerator> getMainThreadUserMappingEnumerator() const {
        return m_pMainThreadUserMappingEnumerator;
    }
    QSharedPointer<MappingInfoEnumerator> getMainThreadSystemMappingEnumerator() const {
        return m_pMainThreadSystemMappingEnumerator;
    }
    QString getConfiguredMappingFileForDevice(const QString& name) const;

    /// Prevent other parts of Mixxx from having to manually connect to our slots
    void setUpDevices() { emit requestSetUpDevices(); };

    /// Close all controllers and stop the ControllerManager thread, blocking
    /// until it is done. This also stops and joins the controller screen
    /// render threads, so it must be called before anything they use
    /// (WaveformWidgetFactory, the QML singletons, PlayerManager, ...) goes
    /// away. Called from the main thread; safe to call more than once. The
    /// destructor calls it too.
    void shutdown();

    static QList<QString> getMappingPaths(UserSettingsPointer pConfig);

  signals:
    void initialized();
    void devicesChanged();
    void requestSetUpDevices();
    void requestShutdown();
    void requestInitialize();
    void mappingApplied(bool applied);

  public slots:
    void slotApplyMapping(Controller* pController,
            std::shared_ptr<LegacyControllerMapping> pMapping,
            bool bEnabled);

  private slots:
    /// Perform initialization that should be delayed until the ControllerManager
    /// thread is started.
    void slotInitialize();
    /// Open whatever controllers are selected in the preferences. This currently
    /// only runs on start-up but maybe should instead be signaled by the
    /// preferences dialog on apply, and only open/close changed devices
    void slotSetUpDevices();
    void slotShutdown();
    /// Calls poll() on all devices that have isPolling() true.
    void slotPollDevices();
    /// Reopens lost controllers whose device is back.
    void slotReconnectLostControllers();

  private:
    void updateControllerList();
    void startPolling();
    void stopPolling();
    void pollIfAnyControllersOpen();
    void openController(Controller* pController);
    void closeController(Controller* pController);
    void watchForDeviceLoss(Controller* pController);
    void controllerLost(Controller* pController);

    UserSettingsPointer m_pConfig;
    // WARNING: Do not parent m_pControllerLearningEventFilter to ControllerManager
    // because the CM is moved to its own thread and runs its own event loop.
    std::unique_ptr<ControllerLearningEventFilter> m_pControllerLearningEventFilter;
    QTimer m_pollTimer;
    /// Runs while any controller is lost (its device switched off or
    /// unplugged while open), until all of them are open again.
    QTimer m_reconnectTimer;
    mutable QMutex m_mutex;
    /// Guarded by m_mutex for main-thread reads.
    /// Written/iterated only on the ControllerManager thread
    std::vector<std::unique_ptr<ControllerEnumerator>> m_enumerators;
    /// Non-owning; Controllers are owned by their respective ControllerEnumerator.
    /// Guarded by m_mutex for main-thread reads.
    /// Written only on the ControllerManager thread
    QList<Controller*> m_controllers;
    /// The single shared background thread that drives the entire ControllerManager,
    /// all ControllerEnumerators, and all Controller instances.
    std::unique_ptr<QThread> m_pThread;
    /// Written once on the ControllerManager thread during slotInitialize(),
    /// before initialized() is emitted. Afterwards only read from the main thread
    QSharedPointer<MappingInfoEnumerator> m_pMainThreadUserMappingEnumerator;
    QSharedPointer<MappingInfoEnumerator> m_pMainThreadSystemMappingEnumerator;
    /// Accessed only from the ControllerManager thread via slotPollDevices().
    bool m_skipPoll;
    /// Controllers closed because their device went away, waiting for it to
    /// come back, and whether it was present at the previous check. Accessed
    /// only from the ControllerManager thread.
    QHash<Controller*, bool> m_lostControllers;
    /// Set while controllers are being closed or reopened after a loss.
    /// Closing a controller can run a nested event loop (the screen renderers
    /// stop), which would otherwise deliver another deviceLost or a reconnect
    /// tick in the middle of it.
    bool m_changingLostControllers{false};
    /// Set by slotShutdown(). Closing the controllers runs nested event loops,
    /// which may still deliver a queued deviceLost for a controller that is
    /// being deleted. Accessed only from the ControllerManager thread.
    bool m_isShuttingDown{false};
    QSet<Controller*> m_watchedControllers;
};
