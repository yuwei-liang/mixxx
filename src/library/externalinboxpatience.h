#pragma once

#include <QDateTime>
#include <QHash>
#include <QString>

/// How long a result in the external analysis inbox waits for its track.
///
/// A result whose track can't be used yet (right after start the library is
/// not connected) is kept and tried again on the next tick. It is given up
/// only after waiting an hour *in this Mixxx*: the clock starts the first time
/// this session sees the file, not at the file's mtime. Counted from the
/// mtime, 223 results delivered while Mixxx was closed were already "three
/// days old" on start and were all dropped in its first second (2026-11-18),
/// exactly the loss the hour was meant to prevent (2026-11-14).
class ExternalInboxPatience {
  public:
    explicit ExternalInboxPatience(qint64 patienceSecs = 3600)
            : m_patienceSecs(patienceSecs) {
    }

    /// The track for `name` isn't usable at `now`: give up on it?
    bool giveUp(const QString& name, const QDateTime& now) {
        auto it = m_firstSeen.constFind(name);
        if (it == m_firstSeen.constEnd()) {
            m_firstSeen.insert(name, now);
            return false;
        }
        if (it.value().secsTo(now) <= m_patienceSecs) {
            return false;
        }
        m_firstSeen.remove(name);
        return true;
    }

    /// `name` was applied or removed: a new file under that name waits afresh.
    void forget(const QString& name) {
        m_firstSeen.remove(name);
    }

  private:
    qint64 m_patienceSecs;
    QHash<QString, QDateTime> m_firstSeen;
};
