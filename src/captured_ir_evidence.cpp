#include "captured_ir_evidence.h"

namespace adk {

    namespace {

        constexpr uint32_t halfRange = UINT32_C (0x80000000);

        bool validSourceKind (IrSourceKind kind) noexcept
        {
            return kind == IrSourceKind::SyntheticFixture ||
                   kind == IrSourceKind::QualifiedReceiver ||
                   kind == IrSourceKind::LocalCatalog ||
                   kind == IrSourceKind::QualifiedEmitter;
        }

        bool sameSource (const IrSourceIdentity& left,
                         const IrSourceIdentity& right) noexcept
        {
            return left.kind == right.kind && left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision &&
                   left.sessionEpoch == right.sessionEpoch;
        }

        bool validSource (const IrSourceIdentity& source) noexcept
        {
            return validSourceKind (source.kind) && source.sourceId != 0 &&
                   source.configurationRevision != 0 && source.sessionEpoch != 0;
        }

        bool validStatus (Status status) noexcept
        {
            switch (status.error ())
            {
                case StatusCode::Ok:
                case StatusCode::InvalidArgument:
                case StatusCode::InvalidConfiguration:
                case StatusCode::InvalidPin:
                case StatusCode::Unsupported:
                case StatusCode::ResourceBusy:
                case StatusCode::NotInitialized:
                case StatusCode::CapacityExceeded:
                case StatusCode::Timeout:
                case StatusCode::InternalInvariant:
                case StatusCode::HardwareFailure: return true;
                default: return false;
            }
        }

        bool validPulseLevel (PulseLevel level) noexcept
        {
            return level == PulseLevel::Mark || level == PulseLevel::Space;
        }

        uint32_t compactPulse (const Pulse& pulse) noexcept
        {
            return pulse.duration.microseconds () |
                   (pulse.level == PulseLevel::Mark ? capturedIrMarkMask : 0U);
        }

        bool validProtocol (InfraredProtocol protocol) noexcept
        {
            return protocol == InfraredProtocol::Unknown ||
                   protocol == InfraredProtocol::Nec;
        }

        bool validFrameValidity (FrameValidity validity) noexcept
        {
            return validity == FrameValidity::Valid ||
                   validity == FrameValidity::Repeat ||
                   validity == FrameValidity::UnknownProtocol ||
                   validity == FrameValidity::TimingInvalid ||
                   validity == FrameValidity::IntegrityInvalid ||
                   validity == FrameValidity::Truncated ||
                   validity == FrameValidity::Overflow;
        }

        bool canonicalDecodedFields (const InfraredFrame& decoded) noexcept
        {
            if (!validProtocol (decoded.protocol) ||
                !validFrameValidity (decoded.validity))
            {
                return false;
            }

            if (decoded.validity == FrameValidity::Valid)
            {
                return decoded.protocol == InfraredProtocol::Nec;
            }

            if (decoded.address != 0 || decoded.command != 0)
            {
                return false;
            }

            if (decoded.validity == FrameValidity::Repeat ||
                decoded.validity == FrameValidity::IntegrityInvalid)
            {
                return decoded.protocol == InfraredProtocol::Nec;
            }

            if (decoded.validity == FrameValidity::UnknownProtocol)
            {
                return decoded.protocol == InfraredProtocol::Unknown;
            }

            return true;
        }

        bool validCapturePair (CaptureState         state,
                               const InfraredFrame& decoded) noexcept
        {
            if (!canonicalDecodedFields (decoded))
            {
                return false;
            }

            if (state == CaptureState::Complete)
            {
                if (decoded.validity == FrameValidity::Overflow)
                {
                    return decoded.protocol == InfraredProtocol::Nec;
                }

                return decoded.validity != FrameValidity::TimingInvalid ||
                       decoded.protocol == InfraredProtocol::Nec;
            }

            if (state == CaptureState::Overflow)
            {
                return decoded.protocol == InfraredProtocol::Unknown &&
                       decoded.validity == FrameValidity::Overflow;
            }

            if (state == CaptureState::TimingFault)
            {
                return decoded.protocol == InfraredProtocol::Unknown &&
                       decoded.validity == FrameValidity::TimingInvalid;
            }

            return false;
        }

        IrCaptureDisposition dispositionFor (CaptureState  state,
                                             FrameValidity validity) noexcept
        {
            if (state == CaptureState::Overflow)
            {
                return IrCaptureDisposition::CaptureOverflow;
            }

            if (state == CaptureState::TimingFault)
            {
                return IrCaptureDisposition::CaptureTimingFault;
            }

            switch (validity)
            {
                case FrameValidity::Valid: return IrCaptureDisposition::KnownValid;
                case FrameValidity::Repeat: return IrCaptureDisposition::KnownRepeat;
                case FrameValidity::UnknownProtocol:
                    return IrCaptureDisposition::UnknownProtocol;
                case FrameValidity::TimingInvalid:
                    return IrCaptureDisposition::TimingInvalid;
                case FrameValidity::IntegrityInvalid:
                    return IrCaptureDisposition::IntegrityInvalid;
                case FrameValidity::Truncated: return IrCaptureDisposition::Truncated;
                case FrameValidity::Overflow:
                    return IrCaptureDisposition::DecoderOverflow;
                default: return IrCaptureDisposition::None;
            }
        }

        EvidenceStrength strengthFor (CaptureState         state,
                                      const InfraredFrame& decoded) noexcept
        {
            if (state != CaptureState::Complete)
            {
                return EvidenceStrength::None;
            }

            if (decoded.validity == FrameValidity::Valid)
            {
                return EvidenceStrength::IntegrityVerified;
            }

            if (decoded.protocol == InfraredProtocol::Nec)
            {
                return EvidenceStrength::ShapeRecognized;
            }

            return EvidenceStrength::None;
        }

        CapturedIrSnapshot initialSnapshot (Status status) noexcept
        {
            const IrSourceIdentity source = {IrSourceKind::SyntheticFixture, 0, 0, 0};
            const CapturedIrProvenance provenance = {source,
                                                     MicrosecondTimePoint (),
                                                     0,
                                                     CaptureState::Idle,
                                                     InfraredProtocol::Unknown,
                                                     FrameValidity::Truncated,
                                                     StatusCode::NotInitialized};

            return {IrCaptureDisposition::None,
                    EvidenceStrength::None,
                    provenance,
                    0,
                    0,
                    0,
                    0,
                    status};
        }

        bool sameIdentity (const CapturedIrSnapshot& left,
                           const CapturedIrSnapshot& right) noexcept
        {
            return left.disposition == right.disposition &&
                   left.strength == right.strength &&
                   sameSource (left.provenance.source, right.provenance.source) &&
                   left.provenance.captureSequence ==
                       right.provenance.captureSequence &&
                   left.provenance.captureState == right.provenance.captureState &&
                   left.provenance.protocol == right.provenance.protocol &&
                   left.provenance.decoderValidity ==
                       right.provenance.decoderValidity &&
                   left.provenance.sourceStatus == right.provenance.sourceStatus &&
                   left.address == right.address && left.command == right.command &&
                   left.pulseCount == right.pulseCount && left.status == right.status;
        }
    } // namespace

    CapturedIrEvidence::CapturedIrEvidence (InfraredDecoder& decoder,
                                            IrPulseStorage   pulseStorage,
                                            uint8_t          maximumPulseCount) noexcept
        : decoder_ (&decoder), pulseStorage_ (pulseStorage),
          snapshot_          (initialSnapshot (StatusCode::NotInitialized)), generation_ (0),
          maximumPulseCount_ (maximumPulseCount), initialized_ (false),
          hasEvidence_       (false)
    {
    }

    CapturedIrEvidence::~CapturedIrEvidence () noexcept
    {
        shutdown ();
    }

    Status CapturedIrEvidence::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validConfiguration ())
        {
            return StatusCode::InvalidConfiguration;
        }

        initialized_ = true;
        reset ();
        return StatusCode::Ok;
    }

    void CapturedIrEvidence::shutdown () noexcept
    {
        initialized_ = false;
        hasEvidence_ = false;
        ++generation_;
        snapshot_ = initialSnapshot (StatusCode::NotInitialized);
    }

    void CapturedIrEvidence::reset () noexcept
    {
        hasEvidence_ = false;
        ++generation_;
        snapshot_ = initialSnapshot (initialized_ ? StatusCode::Ok
                                                  : StatusCode::NotInitialized);
    }

    Status CapturedIrEvidence::admit (const PulseFrame&       frame,
                                      const IrSourceIdentity& source,
                                      Status                  sourceStatus,
                                      MicrosecondTimePoint    observedAt) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        if (!validSource (source) || !validStatus (sourceStatus) ||
            (frame.size > 0 && frame.data == nullptr))
        {
            return StatusCode::InvalidArgument;
        }

        if (frame.size > maximumPulseCount_)
        {
            return StatusCode::CapacityExceeded;
        }

        InfraredFrame decoded      = {};
        const Status  decodeStatus = decoder_->decode (frame, decoded);

        if (!decodeStatus.ok ())
        {
            return decodeStatus;
        }

        if (decoded.captureSequence != frame.sequence ||
            !validCapturePair (frame.state, decoded))
        {
            return StatusCode::InvalidArgument;
        }

        for (uint8_t index = 0; index < frame.size; ++index)
        {
            const uint32_t duration = frame.data[index].duration.microseconds ();

            if (!validPulseLevel (frame.data[index].level) || duration == 0 ||
                duration > capturedIrDurationMask)
            {
                return StatusCode::InvalidArgument;
            }
        }

        CapturedIrSnapshot candidate = {
            sourceStatus.ok () ? dispositionFor (frame.state, decoded.validity)
                               : IrCaptureDisposition::SourceFault,
            sourceStatus.ok () ? strengthFor (frame.state, decoded)
                               : EvidenceStrength::None,
            {source, observedAt, frame.sequence, frame.state, decoded.protocol,
             decoded.validity, sourceStatus},
            sourceStatus.ok () && decoded.validity == FrameValidity::Valid
                ? decoded.address
                : 0,
            sourceStatus.ok () && decoded.validity == FrameValidity::Valid
                ? decoded.command
                : 0,
            0,
            frame.size,
            sourceStatus.ok () ? StatusCode::Ok : sourceStatus};

        if (hasEvidence_)
        {
            if (!sameSource (snapshot_.provenance.source, source))
            {
                return StatusCode::InvalidArgument;
            }

            const uint32_t delta =
                frame.sequence - snapshot_.provenance.captureSequence;

            if (delta == 0)
            {
                if (!sameIdentity (snapshot_, candidate))
                {
                    return StatusCode::InvalidArgument;
                }

                for (uint8_t index = 0; index < frame.size; ++index)
                {
                    if (pulseStorage_.data[index] != compactPulse (frame.data[index]))
                    {
                        return StatusCode::InvalidArgument;
                    }
                }

                return StatusCode::Ok;
            }

            if (delta >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }

            const uint32_t observedDelta =
                observedAt.microseconds                      () -
                snapshot_.provenance.observedAt.microseconds ();

            if (observedDelta >= halfRange)
            {
                return StatusCode::InvalidArgument;
            }
        }

        for (uint8_t index = 0; index < frame.size; ++index)
        {
            pulseStorage_.data[index] = compactPulse (frame.data[index]);
        }

        ++generation_;
        if (generation_ == 0)
        {
            ++generation_;
        }

        candidate.evidenceGeneration = generation_;
        snapshot_                    = candidate;
        hasEvidence_                 = true;
        return StatusCode::Ok;
    }

    CapturedIrSnapshot CapturedIrEvidence::snapshot () const noexcept
    {
        return snapshot_;
    }

    Result<CapturedIrView> CapturedIrEvidence::view () const noexcept
    {
        const CapturedIrView none = {nullptr, 0, nullptr, 0};

        if (!initialized_)
        {
            return {StatusCode::NotInitialized, none};
        }

        if (!hasEvidence_)
        {
            return {StatusCode::InvalidArgument, none};
        }

        const CapturedIrView current = {pulseStorage_.data, snapshot_.pulseCount, this,
                                        snapshot_.evidenceGeneration};
        return {StatusCode::Ok, current};
    }

    Result<uint8_t>
    CapturedIrEvidence::requiredWords (const CapturedIrView& viewValue) const noexcept
    {
        if (!initialized_)
        {
            return {StatusCode::NotInitialized, 0};
        }

        if (!validView (viewValue))
        {
            return {StatusCode::InvalidArgument, 0};
        }

        return {StatusCode::Ok, snapshot_.pulseCount};
    }

    Result<uint8_t>
    CapturedIrEvidence::exportWords (const CapturedIrView& viewValue,
                                     IrPulseStorage        destination) const noexcept
    {
        if (!initialized_)
        {
            return {StatusCode::NotInitialized, 0};
        }

        if (!validView (viewValue) ||
            (destination.capacity > 0 && destination.data == nullptr))
        {
            return {StatusCode::InvalidArgument, 0};
        }

        if (destination.capacity < snapshot_.pulseCount)
        {
            return {StatusCode::CapacityExceeded, 0};
        }

        for (uint8_t index = 0; index < snapshot_.pulseCount; ++index)
        {
            destination.data[index] = pulseStorage_.data[index];
        }

        return {StatusCode::Ok, snapshot_.pulseCount};
    }

    bool CapturedIrEvidence::validConfiguration () const noexcept
    {
        return pulseStorage_.data != nullptr && pulseStorage_.capacity != 0 &&
               pulseStorage_.capacity <= capturedIrPulseCapacity &&
               maximumPulseCount_ != 0 && maximumPulseCount_ <= pulseStorage_.capacity;
    }

    bool CapturedIrEvidence::validView (const CapturedIrView& viewValue) const noexcept
    {
        return hasEvidence_ && viewValue.owner == this &&
               viewValue.evidenceGeneration == snapshot_.evidenceGeneration &&
               viewValue.words == pulseStorage_.data &&
               viewValue.size == snapshot_.pulseCount;
    }
} // namespace adk
