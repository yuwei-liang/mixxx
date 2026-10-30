#pragma once

#include <QColor>

#include "rendergraph/geometrynode.h"
#include "util/class.h"
#include "waveform/renderers/allshader/waveformrenderersignalbase.h"

namespace allshader {
class WaveformRendererRGB;
} // namespace allshader

class allshader::WaveformRendererRGB final
        : public allshader::WaveformRendererSignalBase,
          public rendergraph::GeometryNode {
  public:
    explicit WaveformRendererRGB(WaveformWidgetRenderer* waveformWidget,
            ::WaveformRendererAbstract::PositionSource type =
                    ::WaveformRendererAbstract::Play,
            ::WaveformRendererSignalBase::Options options =
                    ::WaveformRendererSignalBase::Option::None);

    // Pure virtual from WaveformRendererSignalBase, not used
    void onSetup(const QDomNode& node) override;

    bool supportsSlip() const override {
        return true;
    }

    // Virtuals for rendergraph::Node
    void preprocess() override;

    // TEMP(dj-library): how this fork draws the RGB waveform, on the desktop
    // and the S5 screens alike, for loud dance masters, whose per-slice peaks
    // sit at the ceiling and drew a flat wall:
    //  - the height follows the bass (kicks and basslines are what a DJ reads)
    //    with a little of the mids and highs, so beatless parts stay visible;
    //  - it ignores the deck's gain (GAIN knob x ReplayGain), which shrank a
    //    loud track to a fifth of the height; every track is drawn to one scale;
    //  - the EQ doesn't shrink it out of sight: the whole signal stays drawn in
    //    the ghost colour and what the EQ lets through goes over it in colour.
    // An invalid ghost colour turns the ghost off (the EQ then scales the height).
    void setEqGhostColor(const QColor& color) {
        m_eqGhost = color.isValid();
        m_eqGhostColor = color;
    }

  private:
    static constexpr float kBassShare = 0.85f;
    bool m_eqGhost{true};
    QColor m_eqGhostColor{0x46, 0x46, 0x4e};
    bool m_isSlipRenderer;
    ::WaveformRendererSignalBase::Options m_options;

    bool preprocessInner();

    DISALLOW_COPY_AND_ASSIGN(WaveformRendererRGB);
};
