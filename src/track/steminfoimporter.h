#pragma once

#include <QMimeType>

#include "track/steminfo.h"

namespace mixxx {

/// Importer class for StemInfo objects that contains metadata about the stems of a track.
class StemInfoImporter {
  public:
    static QList<StemInfo> importStemInfos(
            const QString& filePath);

    static bool maybeStemFile(
            const QString& aFileName, QMimeType mimeType = QMimeType());

    static bool hasStemAtom(
            const QString& aFileName);

    /// TEMP(dj-library): a track's stem companion. With MIXXX_STEMS_DIR=<dir>,
    /// <dir>/<folder>/<name>.stem.mp4 stands in for <tracks>/<folder>/<name>.mp3
    /// wherever the audio is played and stems are read, while the library,
    /// cues, grid and tags stay with the original. Outside the library's
    /// folders, so the scanner never lists it as a track of its own.
    /// -> the companion's path, or the track's own location when it has none.
    static QString stemSource(const QString& location);
};

} // namespace mixxx
