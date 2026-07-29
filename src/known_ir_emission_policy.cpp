#include "known_ir_emission_policy.h"

namespace adk {
    namespace {
        constexpr uint32_t halfRange           = 0x80000000UL;
        constexpr uint16_t catalogRevision     = 1;
        constexpr uint32_t envelopeDuration    = 67980UL;
        constexpr uint32_t leaderMarkDuration  = 9000UL;
        constexpr uint32_t leaderSpaceDuration = 4500UL;
        constexpr uint32_t bitMarkDuration     = 560UL;
        constexpr uint32_t zeroSpaceDuration   = 560UL;
        constexpr uint32_t oneSpaceDuration    = 1690UL;
        constexpr uint32_t bitBaseDuration     = bitMarkDuration + zeroSpaceDuration;
        constexpr uint32_t oneExtraDuration    = oneSpaceDuration - zeroSpaceDuration;

        struct CatalogEntry
        {
            LocalIrCodeId codeId;
            uint32_t      payload;
            uint8_t       encodingRevision;
            uint8_t       repeatCount;
        };

        const CatalogEntry catalog[] = {
            {LocalIrCodeId::StationPing, 0xef10ff00UL, 1, 1},
            {LocalIrCodeId::StationReady, 0xee11ff00UL, 1, 1},
            {LocalIrCodeId::StationCancel, 0xed12ff00UL, 1, 1},
            {LocalIrCodeId::StationAcknowledge, 0xec13ff00UL, 1, 1}};

        uint32_t mixCatalogByte (uint32_t digest, uint8_t value) noexcept
        {
            return (digest ^ value) * UINT32_C (16777619);
        }

        uint32_t computeCatalogDigest () noexcept
        {
            uint32_t digest = UINT32_C (2166136261);
            for (const CatalogEntry& entry : catalog)
            {
                digest = mixCatalogByte (digest, static_cast<uint8_t> (entry.codeId));
                for (uint8_t shift = 0; shift < 32; shift += 8)
                {
                    digest = mixCatalogByte (
                        digest, static_cast<uint8_t> (entry.payload >> shift));
                }
                digest = mixCatalogByte (digest, entry.encodingRevision);
                digest = mixCatalogByte (digest, entry.repeatCount);
            }
            return digest;
        }

        const uint32_t catalogDigest = computeCatalogDigest ();

        KnownIrCatalogIdentity catalogIdentity () noexcept
        {
            return {catalogRevision, catalogDigest};
        }

        bool validCodeId (LocalIrCodeId codeId) noexcept
        {
            return static_cast<uint8_t> (codeId) <
                   sizeof (catalog) / sizeof (catalog[0]);
        }

        const CatalogEntry& entryFor (LocalIrCodeId codeId) noexcept
        {
            return catalog[static_cast<uint8_t> (codeId)];
        }

        uint8_t populationCount (uint32_t value) noexcept
        {
            value = value - ((value >> 1U) & 0x55555555UL);
            value = (value & 0x33333333UL) + ((value >> 2U) & 0x33333333UL);
            value = (value + (value >> 4U)) & 0x0f0f0f0fUL;
            return static_cast<uint8_t> ((value * 0x01010101UL) >> 24U);
        }

        uint32_t bitsBeforeDuration (uint32_t payload, uint8_t count) noexcept
        {
            const uint32_t mask = count == 32 ? UINT32_C (0xffffffff)
                                              : (uint32_t (1) << count) - uint32_t (1);
            return uint32_t (count) * bitBaseDuration +
                   uint32_t (populationCount (payload & mask)) * oneExtraDuration;
        }

        uint32_t digestMix (uint32_t digest, uint32_t value) noexcept
        {
            digest ^= value;
            digest *= 16777619UL;
            return digest;
        }

        uint32_t candidateDigest (const KnownIrEmissionConfig& config,
                                  uint32_t                     policyGeneration,
                                  uint32_t candidateGeneration, uint32_t transactionId,
                                  LocalIrCodeId        codeId,
                                  MicrosecondTimePoint startAt) noexcept
        {
            uint32_t digest = 2166136261UL;
            digest          = digestMix (digest, config.configurationRevision);
            digest          = digestMix (digest, config.instanceEpoch);
            digest          = digestMix (digest, policyGeneration);
            digest          = digestMix (digest, candidateGeneration);
            digest          = digestMix (digest, transactionId);
            digest          = digestMix (digest, static_cast<uint8_t> (codeId));
            digest          = digestMix (digest, catalogRevision);
            digest          = digestMix (digest, catalogDigest);
            digest          = digestMix (digest, startAt.microseconds ());
            return digest == 0 ? 1 : digest;
        }

        KnownIrEmissionPreview emptyPreview () noexcept
        {
            return {nullptr,
                    0,
                    0,
                    0,
                    0,
                    0,
                    LocalIrCodeId::StationPing,
                    {0, 0},
                    0,
                    MicrosecondTimePoint (),
                    MicrosecondTimePoint (),
                    IrEnvelopeIntent::Inactive};
        }
    } // namespace

    KnownIrEmissionPolicy::KnownIrEmissionPolicy (
        const KnownIrEmissionConfig& config) noexcept
        : config_ (config), snapshot_ (), candidateDigest_ (0), lastTime_ (),
          nextPolicyGeneration_ (0), nextCandidateGeneration_ (0), initialized_ (false),
          hasTime_              (false)
    {
        snapshot_.status = StatusCode::NotInitialized;
    }

    Status KnownIrEmissionPolicy::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        if (!validConfig ())
        {
            return StatusCode::InvalidConfiguration;
        }

        initialized_ = true;
        ++nextPolicyGeneration_;
        if (nextPolicyGeneration_ == 0)
        {
            ++nextPolicyGeneration_;
        }
        clearToIdle ();
        snapshot_.policyGeneration = nextPolicyGeneration_;
        snapshot_.status           = StatusCode::Ok;
        hasTime_                   = false;
        return StatusCode::Ok;
    }

    void KnownIrEmissionPolicy::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        if (snapshot_.disposition == IrEmissionDisposition::Prepared)
        {
            finish (IrEmissionDisposition::Shutdown,
                    IrEmissionTerminalCause::ShutdownBeforeCommit, lastTime_);
        }
        else if (snapshot_.disposition == IrEmissionDisposition::Active)
        {
            finish (IrEmissionDisposition::Shutdown,
                    IrEmissionTerminalCause::ShutdownActive, lastTime_);
        }
        else
        {
            snapshot_.intent      = IrEnvelopeIntent::Inactive;
            snapshot_.disposition = IrEmissionDisposition::Shutdown;
        }

        initialized_     = false;
        candidateDigest_ = 0;
    }

    void KnownIrEmissionPolicy::reset () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        ++nextPolicyGeneration_;
        if (nextPolicyGeneration_ == 0)
        {
            ++nextPolicyGeneration_;
        }
        clearToIdle ();
        snapshot_.policyGeneration = nextPolicyGeneration_;
        snapshot_.status           = StatusCode::Ok;
        hasTime_                   = false;
    }

    Result<KnownIrEmissionPreview>
    KnownIrEmissionPolicy::prepare (LocalIrCodeId codeId, uint32_t transactionId,
                                    MicrosecondTimePoint now) noexcept
    {
        KnownIrEmissionPreview preview = emptyPreview ();

        if (!initialized_)
        {
            return {StatusCode::NotInitialized, preview};
        }
        if (!validCodeId (codeId) || transactionId == 0)
        {
            return {StatusCode::InvalidArgument, preview};
        }
        if (snapshot_.disposition == IrEmissionDisposition::Prepared ||
            snapshot_.disposition == IrEmissionDisposition::Active)
        {
            return {StatusCode::ResourceBusy, preview};
        }
        if (!validTime (now))
        {
            return {StatusCode::InvalidArgument, preview};
        }

        ++nextCandidateGeneration_;
        if (nextCandidateGeneration_ == 0)
        {
            ++nextCandidateGeneration_;
        }

        const MicrosecondTimePoint completeAt (now.microseconds () + envelopeDuration);
        const uint32_t             digest =
            candidateDigest (config_, snapshot_.policyGeneration,
                             nextCandidateGeneration_, transactionId, codeId, now);

        preview = {this,
                   config_.configurationRevision,
                   config_.instanceEpoch,
                   snapshot_.policyGeneration,
                   nextCandidateGeneration_,
                   transactionId,
                   codeId,
                   catalogIdentity (),
                   digest,
                   now,
                   completeAt,
                   IrEnvelopeIntent::CarrierOn};

        snapshot_.candidateGeneration   = nextCandidateGeneration_;
        snapshot_.codeId                = codeId;
        snapshot_.catalog               = catalogIdentity ();
        snapshot_.transactionId         = transactionId;
        snapshot_.startAt               = now;
        snapshot_.completeAt            = completeAt;
        snapshot_.repeatIndex           = 0;
        snapshot_.intent                = IrEnvelopeIntent::Inactive;
        snapshot_.disposition           = IrEmissionDisposition::Prepared;
        snapshot_.terminalCause         = IrEmissionTerminalCause::None;
        snapshot_.terminalTransactionId = 0;
        snapshot_.terminalAt            = MicrosecondTimePoint ();
        snapshot_.status                = StatusCode::Ok;
        candidateDigest_                = digest;
        rememberTime (now);
        return {StatusCode::Ok, preview};
    }

    bool KnownIrEmissionPolicy::canCommit (const KnownIrEmissionPreview& preview,
                                           MicrosecondTimePoint now) const noexcept
    {
        return initialized_ &&
               snapshot_.disposition == IrEmissionDisposition::Prepared &&
               validPreview     (preview) &&
               now.microseconds () == preview.startAt.microseconds () &&
               validTime        (now);
    }

    Status KnownIrEmissionPolicy::commit (const KnownIrEmissionPreview& preview,
                                          MicrosecondTimePoint          now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (!canCommit (preview, now))
        {
            return StatusCode::InvalidArgument;
        }

        snapshot_.intent      = IrEnvelopeIntent::CarrierOn;
        snapshot_.disposition = IrEmissionDisposition::Active;
        snapshot_.status      = StatusCode::Ok;
        rememberTime (now);
        return StatusCode::Ok;
    }

    Status KnownIrEmissionPolicy::cancel (const KnownIrEmissionPreview& preview,
                                          MicrosecondTimePoint          now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        const bool repeated =
            snapshot_.disposition == IrEmissionDisposition::Cancelled &&
            snapshot_.terminalCause == IrEmissionTerminalCause::CancelledBeforeCommit &&
            validPreview (preview);
        if ((!repeated && snapshot_.disposition != IrEmissionDisposition::Prepared) ||
            !validPreview (preview) || !validTime (now))
        {
            return StatusCode::InvalidArgument;
        }

        rememberTime (now);
        if (!repeated)
        {
            finish (IrEmissionDisposition::Cancelled,
                    IrEmissionTerminalCause::CancelledBeforeCommit, now);
        }
        return StatusCode::Ok;
    }

    Status KnownIrEmissionPolicy::cancel (uint32_t             transactionId,
                                          MicrosecondTimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        const bool repeated =
            snapshot_.disposition == IrEmissionDisposition::Cancelled &&
            snapshot_.terminalCause == IrEmissionTerminalCause::CancelledActive &&
            snapshot_.terminalTransactionId == transactionId && transactionId != 0;
        if ((!repeated && snapshot_.disposition != IrEmissionDisposition::Active) ||
            transactionId == 0 || snapshot_.transactionId != transactionId ||
            !validTime (now))
        {
            return StatusCode::InvalidArgument;
        }

        rememberTime (now);
        if (!repeated)
        {
            finish (IrEmissionDisposition::Cancelled,
                    IrEmissionTerminalCause::CancelledActive, now);
        }
        return StatusCode::Ok;
    }

    Status KnownIrEmissionPolicy::update (MicrosecondTimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (snapshot_.disposition != IrEmissionDisposition::Active)
        {
            return StatusCode::InvalidArgument;
        }
        if (!validTime (now))
        {
            return StatusCode::InvalidArgument;
        }

        const MicrosecondDuration elapsed = now.elapsedSince (snapshot_.startAt);
        if (elapsed.microseconds                             () >= envelopeDuration)
        {
            rememberTime (now);
            finish       (IrEmissionDisposition::Complete, IrEmissionTerminalCause::Completed,
                    now);
            return StatusCode::Ok;
        }

        snapshot_.intent = intentAt (snapshot_.codeId, elapsed);
        snapshot_.status = StatusCode::Ok;
        rememberTime (now);
        return StatusCode::Ok;
    }

    KnownIrEmissionSnapshot KnownIrEmissionPolicy::snapshot () const noexcept
    {
        return snapshot_;
    }

    bool KnownIrEmissionPolicy::validConfig () const noexcept
    {
        const uint32_t maximum = config_.maximumEnvelopeDuration.microseconds ();
        return config_.configurationRevision != 0 && config_.instanceEpoch != 0 &&
               maximum >= envelopeDuration && maximum < halfRange;
    }

    bool KnownIrEmissionPolicy::validPreview (
        const KnownIrEmissionPreview& preview) const noexcept
    {
        return preview.owner == this &&
               preview.configurationRevision == config_.configurationRevision &&
               preview.instanceEpoch == config_.instanceEpoch &&
               preview.policyGeneration == snapshot_.policyGeneration &&
               preview.candidateGeneration == snapshot_.candidateGeneration &&
               preview.transactionId == snapshot_.transactionId &&
               preview.codeId == snapshot_.codeId &&
               preview.catalog.revision == catalogRevision &&
               preview.catalog.digest == catalogDigest &&
               preview.candidateDigest == candidateDigest_ &&
               preview.startAt.microseconds    () == snapshot_.startAt.microseconds () &&
               preview.completeAt.microseconds () ==
                   snapshot_.completeAt.microseconds () &&
               preview.firstIntent == IrEnvelopeIntent::CarrierOn;
    }

    bool KnownIrEmissionPolicy::validTime (MicrosecondTimePoint now) const noexcept
    {
        if (!hasTime_)
        {
            return true;
        }
        const uint32_t elapsed = now.elapsedSince (lastTime_).microseconds ();
        return elapsed < halfRange;
    }

    void KnownIrEmissionPolicy::rememberTime (MicrosecondTimePoint now) noexcept
    {
        lastTime_ = now;
        hasTime_  = true;
    }

    void KnownIrEmissionPolicy::clearToIdle () noexcept
    {
        snapshot_.configurationRevision = config_.configurationRevision;
        snapshot_.instanceEpoch         = config_.instanceEpoch;
        snapshot_.policyGeneration      = 0;
        snapshot_.candidateGeneration   = 0;
        snapshot_.codeId                = LocalIrCodeId::StationPing;
        snapshot_.catalog               = catalogIdentity ();
        snapshot_.transactionId         = 0;
        snapshot_.startAt               = MicrosecondTimePoint ();
        snapshot_.completeAt            = MicrosecondTimePoint ();
        snapshot_.repeatIndex           = 0;
        snapshot_.intent                = IrEnvelopeIntent::Inactive;
        snapshot_.disposition           = IrEmissionDisposition::Idle;
        snapshot_.terminalCause         = IrEmissionTerminalCause::None;
        snapshot_.terminalTransactionId = 0;
        snapshot_.terminalAt            = MicrosecondTimePoint ();
        snapshot_.status                = StatusCode::NotInitialized;
        candidateDigest_                = 0;
    }

    void KnownIrEmissionPolicy::finish (IrEmissionDisposition   disposition,
                                        IrEmissionTerminalCause cause,
                                        MicrosecondTimePoint    now) noexcept
    {
        snapshot_.intent                = IrEnvelopeIntent::Inactive;
        snapshot_.disposition           = disposition;
        snapshot_.terminalCause         = cause;
        snapshot_.terminalTransactionId = snapshot_.transactionId;
        snapshot_.terminalAt            = now;
        snapshot_.status                = StatusCode::Ok;
    }

    IrEnvelopeIntent
    KnownIrEmissionPolicy::intentAt (LocalIrCodeId       codeId,
                                     MicrosecondDuration elapsed) const noexcept
    {
        const uint32_t offset = elapsed.microseconds ();
        if (offset < leaderMarkDuration)
        {
            return IrEnvelopeIntent::CarrierOn;
        }
        if (offset < leaderMarkDuration + leaderSpaceDuration)
        {
            return IrEnvelopeIntent::CarrierOff;
        }

        const CatalogEntry& entry   = entryFor (codeId);
        const uint32_t bodyOffset   = offset - leaderMarkDuration - leaderSpaceDuration;
        const uint32_t bitsDuration = envelopeDuration - leaderMarkDuration -
                                      leaderSpaceDuration - bitMarkDuration;
        if (bodyOffset >= bitsDuration)
        {
            return IrEnvelopeIntent::CarrierOn;
        }

        uint8_t low  = 0;
        uint8_t high = 32;
        for (uint8_t step = 0; step < 5; ++step)
        {
            const uint8_t middle = static_cast<uint8_t> ((low + high) / 2);
            if (bitsBeforeDuration (entry.payload, middle) <= bodyOffset)
            {
                low = middle;
            }
            else
            {
                high = middle;
            }
        }

        const uint32_t bitOffset = bodyOffset - bitsBeforeDuration (entry.payload, low);
        return bitOffset < bitMarkDuration ? IrEnvelopeIntent::CarrierOn
                                           : IrEnvelopeIntent::CarrierOff;
    }
} // namespace adk
