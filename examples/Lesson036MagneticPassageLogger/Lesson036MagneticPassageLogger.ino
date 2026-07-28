// Mega 2560, USB 5 V only. D30, D31, and D32 each drive an LED through a
// separate 1 kOhm resistor to GND. D22, D23, and D24 drive the same inert
// 74HC595 seven-segment fixture used in Lesson 010. Passage, RTC, and storage
// evidence are deterministic software fixtures. Do not connect a magnetic
// specimen, physical RTC, removable media, motor, gate, lock, launcher, or
// ignition path. This sketch proves software composition only.
#include <Adk.h>
#include <magnetic_passage_logger.h>
#include <passage_ledger.h>
#include <rtc.h>

namespace {

    constexpr adk::ShiftRegisterPins displayPins          = {22, 23, 24};
    constexpr adk::PinId             acceptedEvidencePin  = 30;
    constexpr adk::PinId             committedEvidencePin = 31;
    constexpr adk::PinId             faultEvidencePin     = 32;
    constexpr adk::PinId             labelButtonPin       = 28;
    constexpr uint32_t               presentationStepMs   = 500;
    constexpr uint32_t               faultEvidenceMs      = 2000;
    constexpr uint32_t               labelPreviewMs       = 1000;
    constexpr uint32_t               evidenceDurationMs   = 120000;

    const adk::ButtonConfig labelButtonConfig (labelButtonPin);

    struct FixtureRtc final : adk::Rtc
    {
        adk::Status initialize () noexcept override
        {
            initialized_ = true;
            return adk::StatusCode::Ok;
        }

        void shutdown () noexcept override
        {
            initialized_ = false;
        }

        adk::Result<adk::ClockReading> read () noexcept override
        {
            if (!initialized_)
            {
                return {adk::StatusCode::NotInitialized, {0, adk::ClockState::NotSet}};
            }

            if (failNextRead_)
            {
                failNextRead_ = false;
                return {adk::StatusCode::HardwareFailure,
                        {0, adk::ClockState::TransportFault}};
            }

            const adk::ClockReading reading = {
                static_cast<uint32_t> (1700000000UL + reads_), adk::ClockState::Valid};
            ++reads_;
            return {adk::StatusCode::Ok, reading};
        }

      private:
        uint32_t reads_        = 0;
        bool     initialized_  = false;
        bool     failNextRead_ = true;
    };

    struct FixtureStorage final : adk::PassageLedgerStorage
    {
        FixtureStorage () noexcept
        {
            for (uint16_t index = 0; index < capacity (); ++index)
            {
                bytes_[index] = 0xff;
            }
        }

        uint16_t capacity () const noexcept override
        {
            return adk::TwoSlotPassageLedger::requiredCapacity;
        }

        adk::Result<uint8_t> read (uint16_t address) noexcept override
        {
            if (address >= capacity ())
            {
                return {adk::StatusCode::InvalidArgument, 0};
            }

            return {adk::StatusCode::Ok, bytes_[address]};
        }

        adk::Status write (uint16_t address, uint8_t value) noexcept override
        {
            if (address >= capacity ())
            {
                return adk::StatusCode::InvalidArgument;
            }

            bytes_[address] = value;
            return adk::StatusCode::Ok;
        }

        adk::Status synchronize () noexcept override
        {
            return adk::StatusCode::Ok;
        }

      private:
        uint8_t bytes_[adk::TwoSlotPassageLedger::requiredCapacity];
    };

    adk::PassageRecord acceptedRecord (uint32_t              sequence,
                                       adk::PassageDirection direction,
                                       uint32_t acceptedAt, int32_t delta)
    {
        return {sequence,
                direction,
                adk::PassageDisposition::Accepted,
                adk::TimePoint (acceptedAt - 20),

                adk::TimePoint (acceptedAt),

                adk::Duration (20),
                adk::MagneticPolarity::Unspecified,
                adk::MagneticPolarity::Unspecified,
                {true, true, false, 0, delta, delta},
                sequence,
                0,
                adk::StatusCode::Ok};
    }

    const adk::PassageRecord fixture[] = {
        acceptedRecord (1, adk::PassageDirection::AToB, 100, 4),
        acceptedRecord (2, adk::PassageDirection::BToA, 200, -4),
        acceptedRecord (3, adk::PassageDirection::AToB, 300, 4)};

    constexpr uint8_t fixtureCount =
        static_cast<uint8_t> (sizeof (fixture) / sizeof (fixture[0]));

    adk::Runtime runtime;

    FixtureRtc     rtc;
    FixtureStorage storage;

    adk::TwoSlotPassageLedger ledger (storage);

    adk::SevenSegmentDisplay display (runtime.resources (), displayPins,
                                      adk::SevenSegmentPolarity::CommonCathode);
    adk::SevenSegmentPassageCountDisplay countDisplay (display);

    adk::MagneticPassageLogger logger ({adk::PassageLabel::A}, rtc, ledger,
                                       countDisplay);

    adk::MonoLed acceptedEvidence (runtime.resources (), acceptedEvidencePin);

    adk::MonoLed committedEvidence (runtime.resources (), committedEvidencePin);

    adk::MonoLed faultEvidence (runtime.resources (), faultEvidencePin);

    adk::Button labelButton (runtime.resources (), labelButtonConfig);

    adk::PassageRecord  observedPassage;
    adk::LoggerSnapshot decidedLogger;
    uint32_t            nextStepAtMs            = 0;
    uint32_t            startedAtMs             = 0;
    uint32_t            reinitializeStartedAtMs = 0;
    uint32_t            labelPreviewStartedAtMs = 0;
    uint8_t             fixtureIndex            = 0;
    bool                acceptedIntent          = false;
    bool                committedIntent         = false;
    bool                faultIntent             = false;
    bool                reinitializeScheduled   = false;
    bool                labelPreviewActive      = false;
    bool                running                 = false;

    bool acquireEvidencePanel ();

    bool configureLogger ();

    bool startFixture ();

    void observeAcceptedPassage ();

    bool decideLabelAndCommit ();

    bool actuateDurableEvidence ();

    bool recoverAfterVisibleFault (uint32_t nowMs);

    bool restoreDurableCountAfterPreview (uint32_t nowMs);

    adk::Status showLabelPreview (adk::PassageLabel label);

    void stopSafely ();

} // namespace

void setup ()
{
    if (acquireEvidencePanel () && configureLogger ())
    {
        running = startFixture ();
    }

    if (!running)
    {
        stopSafely ();
    }
}

void loop ()
{
    if (!running)
    {
        return;
    }

    const uint32_t nowMs = millis ();

    if (nowMs - startedAtMs >= evidenceDurationMs)
    {
        stopSafely ();
        return;
    }

    if (nowMs - nextStepAtMs < presentationStepMs)
    {
        return;
    }

    nextStepAtMs += presentationStepMs;

    if (!restoreDurableCountAfterPreview (nowMs))
    {
        actuateDurableEvidence ();

        stopSafely ();
        return;
    }

    if (labelPreviewActive)
    {
        return;
    }

    if (!recoverAfterVisibleFault (nowMs))
    {
        stopSafely ();
        return;
    }

    if (reinitializeScheduled)
    {
        return;
    }

    observeAcceptedPassage ();

    const bool decisionOk = decideLabelAndCommit ();

    const bool evidenceOk = actuateDurableEvidence ();

    if (!decisionOk || !evidenceOk)
    {
        stopSafely ();
    }
}

namespace {

    bool acquireEvidencePanel ()
    {
        if (!labelButton.initialize ().ok ())
        {
            return false;
        }

        if (!acceptedEvidence.initialize ().ok ())
        {
            labelButton.shutdown ();
            return false;
        }

        if (!committedEvidence.initialize ().ok ())
        {
            acceptedEvidence.shutdown ();

            labelButton.shutdown ();
            return false;
        }

        if (!faultEvidence.initialize ().ok ())
        {
            committedEvidence.shutdown ();

            acceptedEvidence.shutdown ();

            labelButton.shutdown ();
            return false;
        }

        return true;
    }

    bool configureLogger ()
    {
        if (!logger.initialize ().ok ())
        {
            return false;
        }

        decidedLogger = logger.snapshot ();

        return acceptedEvidence.off ().ok () && committedEvidence.off ().ok () &&
               faultEvidence.set (decidedLogger.persistentFault).ok ();
    }

    bool startFixture ()
    {
        fixtureIndex            = 0;
        startedAtMs             = millis ();
        nextStepAtMs            = startedAtMs;
        reinitializeStartedAtMs = 0;
        labelPreviewStartedAtMs = 0;
        reinitializeScheduled   = false;
        labelPreviewActive      = false;
        acceptedIntent          = false;
        committedIntent         = false;
        faultIntent             = decidedLogger.persistentFault;
        return true;
    }

    void observeAcceptedPassage ()
    {
        labelButton.update (adk::TimePoint (millis ()));

        observedPassage = fixture[fixtureIndex];
    }

    bool decideLabelAndCommit ()
    {
        const adk::Status status = logger.update (observedPassage);

        decidedLogger   = logger.snapshot ();
        acceptedIntent  = decidedLogger.acceptedPulse;
        committedIntent = decidedLogger.committedPulse;
        faultIntent     = decidedLogger.persistentFault;

        if (decidedLogger.committedPulse && fixtureIndex + 1U < fixtureCount)
        {
            ++fixtureIndex;
        }

        adk::Status cycleStatus = adk::StatusCode::Ok;

        if (labelButton.pressEvent ())
        {
            cycleStatus = logger.cycleLabel ();

            decidedLogger = logger.snapshot ();

            if (cycleStatus.ok ())
            {
                const adk::Status previewStatus =
                    showLabelPreview (decidedLogger.selectedLabel);

                faultIntent = faultIntent || !previewStatus.ok ();

                labelPreviewActive = previewStatus.ok ();

                labelPreviewStartedAtMs = millis ();
                cycleStatus             = previewStatus;
            }
        }

        if (!status.ok () && acceptedIntent && faultIntent)
        {
            return cycleStatus.ok ();
        }

        if (status.ok () && committedIntent && faultIntent && !reinitializeScheduled)
        {
            reinitializeStartedAtMs = millis ();
            reinitializeScheduled   = true;
        }

        return status.ok () && cycleStatus.ok ();
    }

    bool actuateDurableEvidence ()
    {
        return acceptedEvidence.set (acceptedIntent).ok () &&

               committedEvidence.set (committedIntent).ok () &&

               faultEvidence.set (faultIntent).ok ();
    }

    bool recoverAfterVisibleFault (uint32_t nowMs)
    {
        if (!reinitializeScheduled || nowMs - reinitializeStartedAtMs < faultEvidenceMs)
        {
            return true;
        }

        logger.shutdown ();

        const adk::Status status = logger.initialize ();

        decidedLogger         = logger.snapshot ();
        acceptedIntent        = decidedLogger.acceptedPulse;
        committedIntent       = decidedLogger.committedPulse;
        faultIntent           = decidedLogger.persistentFault;
        reinitializeScheduled = false;

        return actuateDurableEvidence () && status.ok ();
    }

    bool restoreDurableCountAfterPreview (uint32_t nowMs)
    {
        if (!labelPreviewActive || nowMs - labelPreviewStartedAtMs < labelPreviewMs)
        {
            return true;
        }

        const uint8_t digit = static_cast<uint8_t> (decidedLogger.committedCount % 10U);
        const bool    decimalPoint = decidedLogger.committedCount >= 10U;
        const adk::Status status   = display.show (
            static_cast<adk::SevenSegmentGlyph> (
                static_cast<uint8_t> (adk::SevenSegmentGlyph::Zero) + digit),
            decimalPoint);

        labelPreviewActive = false;
        faultIntent        = faultIntent || !status.ok ();

        return status.ok ();
    }

    adk::Status showLabelPreview (adk::PassageLabel label)
    {
        switch (label)
        {
            case adk::PassageLabel::None:
                return display.show (adk::SevenSegmentGlyph::Dash);
            case adk::PassageLabel::A: return display.show (adk::SevenSegmentGlyph::A);
            case adk::PassageLabel::B: return display.show (adk::SevenSegmentGlyph::B);
            case adk::PassageLabel::C: return display.show (adk::SevenSegmentGlyph::C);
        }

        return adk::StatusCode::InvalidArgument;
    }

    void stopSafely ()
    {
        running = false;

        logger.shutdown ();

        faultEvidence.shutdown ();

        committedEvidence.shutdown ();

        acceptedEvidence.shutdown ();

        labelButton.shutdown ();
    }

} // namespace
