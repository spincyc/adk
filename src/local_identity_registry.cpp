#include "local_identity_registry.h"

#include <stddef.h>
#include <string.h>

namespace adk {
    namespace {
        constexpr uint8_t entryBytes  = 16;
        constexpr uint8_t entryOffset = 16;

        uint16_t read16 (const uint8_t* bytes) noexcept
        {
            return static_cast<uint16_t> (bytes[0]) |
                   static_cast<uint16_t> (static_cast<uint16_t> (bytes[1]) << 8);
        }

        uint32_t read32 (const uint8_t* bytes) noexcept
        {
            return static_cast<uint32_t> (bytes[0]) |
                   (static_cast<uint32_t> (bytes[1]) << 8) |
                   (static_cast<uint32_t> (bytes[2]) << 16) |
                   (static_cast<uint32_t> (bytes[3]) << 24);
        }

        void write16 (uint8_t* bytes, uint16_t value) noexcept
        {
            bytes[0] = static_cast<uint8_t> (value);
            bytes[1] = static_cast<uint8_t> (value >> 8);
        }

        void write32 (uint8_t* bytes, uint32_t value) noexcept
        {
            bytes[0] = static_cast<uint8_t> (value);
            bytes[1] = static_cast<uint8_t> (value >> 8);
            bytes[2] = static_cast<uint8_t> (value >> 16);
            bytes[3] = static_cast<uint8_t> (value >> 24);
        }

        uint16_t crc16 (const uint8_t* bytes, uint16_t length) noexcept
        {
            uint16_t crc = 0xffff;
            for (uint16_t index = 0; index < length; ++index)
            {
                crc ^= static_cast<uint16_t> (bytes[index]) << 8;
                for (uint8_t bit = 0; bit < 8; ++bit)
                {
                    crc = (crc & 0x8000) != 0
                              ? static_cast<uint16_t> ((crc << 1) ^ 0x1021)
                              : static_cast<uint16_t> (crc << 1);
                }
            }
            return crc;
        }

        bool allValue (const uint8_t* bytes, uint16_t length, uint8_t value) noexcept
        {
            for (uint16_t index = 0; index < length; ++index)
            {
                if (bytes[index] != value)
                {
                    return false;
                }
            }
            return true;
        }

        bool identityValid (const LocalIdentity& identity) noexcept
        {
            if (identity.length < 4 || identity.length > maximumLocalIdentityBytes)
            {
                return false;
            }
            bool allZero   = true;
            bool allErased = true;
            for (uint8_t index = 0; index < maximumLocalIdentityBytes; ++index)
            {
                if (index >= identity.length && identity.bytes[index] != 0)
                {
                    return false;
                }
                if (index < identity.length)
                {
                    allZero &= identity.bytes[index] == 0;
                    allErased &= identity.bytes[index] == 0xff;
                }
            }
            return !allZero && !allErased;
        }

        bool identityEqual (const LocalIdentity& left,
                            const LocalIdentity& right) noexcept
        {
            return left.length == right.length &&
                   memcmp (left.bytes, right.bytes, left.length) == 0;
        }

        bool sourceEqual (const CarouselSource& left,
                          const CarouselSource& right) noexcept
        {
            return left.kind == right.kind && left.sourceId == right.sourceId &&
                   left.configurationRevision == right.configurationRevision;
        }

        bool sourceValid (const CarouselSource& source) noexcept
        {
            return source.kind == CarouselSourceKind::SyntheticIdentity &&
                   source.sourceId != 0 && source.configurationRevision != 0;
        }

        bool presentOrPast (TimePoint now, TimePoint observedAt, uint32_t& age) noexcept
        {
            age = now.milliseconds () - observedAt.milliseconds ();
            return age <= 0x7fffffffUL;
        }

        bool newer (uint32_t candidate, uint32_t baseline, bool& ambiguous) noexcept
        {
            const uint32_t delta = candidate - baseline;
            ambiguous            = delta == 0x80000000UL;
            return delta != 0 && delta < 0x80000000UL;
        }

        IdentityDisposition validateImage (const uint8_t* bytes,
                                           uint32_t configurationId, uint8_t binCount,
                                           uint8_t capacity, IdentityBinding* decoded,
                                           uint8_t&  count,
                                           uint32_t& generation) noexcept
        {
            count      = 0;
            generation = 0;
            if (allValue (bytes, localIdentityImageBytes, 0xff))
            {
                return IdentityDisposition::ImageCorrupt;
            }
            if (read16 (bytes) != localIdentityImageMagic ||
                bytes[2] != localIdentityImageVersion)
            {
                return IdentityDisposition::ImageUnsupported;
            }
            if (bytes[3] != 0 || read16 (bytes + 4) != localIdentityImageBytes ||
                read32 (bytes + 6) != configurationId || bytes[15] != 0 ||
                bytes[14] > capacity || !allValue (bytes + 144, 14, 0))
            {
                return IdentityDisposition::ImageCorrupt;
            }
            if (read16 (bytes + 158) != crc16 (bytes, 158))
            {
                return IdentityDisposition::ImageCorrupt;
            }

            count      = bytes[14];
            generation = read32 (bytes + 10);
            for (uint8_t index = 0; index < maximumLocalIdentities; ++index)
            {
                const uint8_t* entry =
                    bytes + entryOffset + static_cast<uint16_t> (index) * entryBytes;
                if (index >= count)
                {
                    if (!allValue (entry, entryBytes, 0))
                    {
                        return IdentityDisposition::ImageCorrupt;
                    }
                    continue;
                }
                IdentityBinding binding{};
                binding.identity.length = entry[0];
                memcpy (binding.identity.bytes, entry + 1, maximumLocalIdentityBytes);
                binding.binId    = entry[11];
                binding.revision = read16 (entry + 12);
                binding.checksum = read16 (entry + 14);
                if (!identityValid (binding.identity) || binding.binId >= binCount ||
                    binding.revision == 0 || binding.checksum != crc16 (entry, 14))
                {
                    return IdentityDisposition::ImageCorrupt;
                }
                for (uint8_t prior = 0; prior < index; ++prior)
                {
                    if (decoded[prior].binId == binding.binId ||
                        identityEqual (decoded[prior].identity, binding.identity))
                    {
                        return IdentityDisposition::ImageCorrupt;
                    }
                }
                decoded[index] = binding;
            }
            return IdentityDisposition::None;
        }

        void encodeImage (uint8_t* bytes, uint32_t configurationId, uint32_t generation,
                          const IdentityBinding* bindings, uint8_t count) noexcept
        {
            memset (bytes, 0, localIdentityImageBytes);
            write16 (bytes, localIdentityImageMagic);
            bytes[2] = localIdentityImageVersion;
            write16 (bytes + 4, localIdentityImageBytes);
            write32 (bytes + 6, configurationId);
            write32 (bytes + 10, generation);
            bytes[14] = count;
            for (uint8_t index = 0; index < count; ++index)
            {
                uint8_t* entry =
                    bytes + entryOffset + static_cast<uint16_t> (index) * entryBytes;
                entry[0] = bindings[index].identity.length;
                memcpy (entry + 1, bindings[index].identity.bytes,
                        maximumLocalIdentityBytes);
                entry[11] = bindings[index].binId;
                write16 (entry + 12, bindings[index].revision);
                write16 (entry + 14, crc16 (entry, 14));
            }
            write16 (bytes + 158, crc16 (bytes, 158));
        }

        bool rangesOverlap (const uint8_t* first, uint16_t firstLength,
                            const uint8_t* second, uint16_t secondLength) noexcept
        {
            const uintptr_t firstBegin  = reinterpret_cast<uintptr_t> (first);
            const uintptr_t secondBegin = reinterpret_cast<uintptr_t> (second);
            return firstBegin <= secondBegin ? secondBegin - firstBegin < firstLength
                                             : firstBegin - secondBegin < secondLength;
        }
    } // namespace

    LocalIdentityRegistry::LocalIdentityRegistry (
        const LocalIdentityRegistryConfig& config, IdentityBinding* liveStorage,
        uint8_t capacity, uint8_t* imageSlotBytes, uint16_t imageSlotByteExtent,
        uint16_t imageSlotStride, uint8_t imageSlotCount, uint8_t* candidateImageBytes,
        uint16_t candidateImageCapacity) noexcept
        : config_ (config), liveStorage_ (liveStorage),
          imageSlotBytes_ (imageSlotBytes), candidateImageBytes_ (candidateImageBytes),
          snapshot_{}, lastIdentity_{}, lastSource_{}, candidate_{},
          imageSlotByteExtent_ (imageSlotByteExtent),
          imageSlotStride_ (imageSlotStride),
          candidateImageCapacity_ (candidateImageCapacity),
          owner_ (static_cast<uint32_t> (
              reinterpret_cast<uintptr_t> (this) ^
              (static_cast<uintptr_t> (config.registryConfigurationId) << 1))),
          candidateGeneration_ (0), operationId_ (0), candidateSequence_ (0),
          lastObservedAt_ (), candidateObservedAt_ (), lockoutStartedAt_ (),
          candidateSource_{}, capacity_ (capacity), imageSlotCount_ (imageSlotCount),
          activeSlot_ (0), initialized_ (false), hasObservation_ (false),
          reconciliationRequired_ (false), installedEvidenceValid_ (false)
    {
        if (owner_ == 0)
        {
            owner_ = 1;
        }
        snapshot_.status = Status (StatusCode::NotInitialized);
    }

    Status LocalIdentityRegistry::initialize () noexcept
    {
        if (initialized_)
        {
            return Status ();
        }
        if (config_.registryConfigurationId == 0 || config_.binCount == 0 ||
            config_.binCount > maximumCarouselBins || config_.maximumFailures == 0 ||
            config_.lockoutDuration.milliseconds () == 0 ||
            config_.lockoutDuration.milliseconds () >= 0x80000000UL ||
            config_.maximumEvidenceAge.milliseconds () > 0x7fffffffUL ||
            liveStorage_ == nullptr || capacity_ == 0 ||
            capacity_ > maximumLocalIdentities || imageSlotBytes_ == nullptr ||
            imageSlotCount_ != localIdentitySlotCount ||
            imageSlotStride_ != localIdentityImageBytes ||
            imageSlotByteExtent_ != localIdentitySlotCount * localIdentityImageBytes ||
            candidateImageBytes_ == nullptr ||
            candidateImageCapacity_ != localIdentityImageBytes ||
            rangesOverlap (imageSlotBytes_, imageSlotByteExtent_, candidateImageBytes_,
                           candidateImageCapacity_))
        {
            return Status (StatusCode::InvalidConfiguration);
        }

        IdentityBinding     decoded[localIdentitySlotCount][maximumLocalIdentities]{};
        uint8_t             counts[localIdentitySlotCount]{};
        uint32_t            generations[localIdentitySlotCount]{};
        IdentityDisposition validity[localIdentitySlotCount]{};
        for (uint8_t slot = 0; slot < localIdentitySlotCount; ++slot)
        {
            validity[slot] = validateImage (
                imageSlotBytes_ + static_cast<uint16_t> (slot) * imageSlotStride_,
                config_.registryConfigurationId, config_.binCount, capacity_,
                decoded[slot], counts[slot], generations[slot]);
        }
        const bool firstValid  = validity[0] == IdentityDisposition::None;
        const bool secondValid = validity[1] == IdentityDisposition::None;
        if (!firstValid && !secondValid)
        {
            snapshot_.disposition =
                validity[0] == IdentityDisposition::ImageUnsupported ||
                        validity[1] == IdentityDisposition::ImageUnsupported
                    ? IdentityDisposition::ImageUnsupported
                    : IdentityDisposition::ImageCorrupt;
            snapshot_.status = Status (StatusCode::InvalidConfiguration);
            return snapshot_.status;
        }

        uint8_t chosen = firstValid ? 0 : 1;
        if (firstValid && secondValid)
        {
            if (generations[0] == generations[1])
            {
                if (memcmp (imageSlotBytes_, imageSlotBytes_ + imageSlotStride_,
                            localIdentityImageBytes) != 0)
                {
                    snapshot_.disposition = IdentityDisposition::ImageCorrupt;
                    snapshot_.status      = Status (StatusCode::InvalidConfiguration);
                    return snapshot_.status;
                }
            }
            else
            {
                bool ambiguous = false;
                chosen = newer (generations[1], generations[0], ambiguous) ? 1 : 0;
                if (ambiguous)
                {
                    snapshot_.disposition = IdentityDisposition::ImageCorrupt;
                    snapshot_.status      = Status (StatusCode::InvalidConfiguration);
                    return snapshot_.status;
                }
            }
        }

        memcpy (liveStorage_, decoded[chosen],
                static_cast<size_t> (counts[chosen]) * sizeof (IdentityBinding));
        snapshot_                 = {};
        snapshot_.bindingCount    = counts[chosen];
        snapshot_.imageGeneration = generations[chosen];
        snapshot_.status          = Status ();
        activeSlot_               = chosen;
        candidate_                = {};
        initialized_              = true;
        hasObservation_           = false;
        reconciliationRequired_   = false;
        installedEvidenceValid_   = false;
        return Status ();
    }

    Status LocalIdentityRegistry::reset () noexcept
    {
        if (!initialized_)
        {
            return Status (StatusCode::NotInitialized);
        }
        if (reconciliationRequired_)
        {
            return Status (StatusCode::HardwareFailure);
        }
        const uint8_t  bindingCount     = snapshot_.bindingCount;
        const uint32_t imageGeneration  = snapshot_.imageGeneration;
        const uint32_t acceptedSequence = snapshot_.acceptedSequence;
        snapshot_                       = {};
        snapshot_.bindingCount          = bindingCount;
        snapshot_.imageGeneration       = imageGeneration;
        snapshot_.acceptedSequence      = acceptedSequence;
        snapshot_.status                = Status ();
        candidate_                      = {};
        return Status ();
    }

    void LocalIdentityRegistry::shutdown () noexcept
    {
        initialized_          = false;
        snapshot_.disposition = reconciliationRequired_
                                    ? IdentityDisposition::CommitIndeterminate
                                    : IdentityDisposition::None;
        snapshot_.selectedBin = 0;
        snapshot_.matchedBindingRevision = 0;
        snapshot_.enrollmentPending      = false;
        snapshot_.externalCommitPending  = reconciliationRequired_;
        snapshot_.status                 = Status (StatusCode::NotInitialized);
        hasObservation_                  = false;
        if (!reconciliationRequired_)
        {
            candidate_ = {};
        }
    }

    Status LocalIdentityRegistry::observe (TimePoint               now,
                                           const IdentityEvidence& evidence) noexcept
    {
        if (!initialized_)
        {
            return Status (StatusCode::NotInitialized);
        }
        if (reconciliationRequired_)
        {
            return Status (StatusCode::HardwareFailure);
        }
        if (snapshot_.enrollmentPending)
        {
            return Status (StatusCode::ResourceBusy);
        }

        uint32_t age = 0;
        if (!evidence.status.ok () || !sourceValid (evidence.source) ||
            !identityValid (evidence.identity) ||
            !presentOrPast (now, evidence.observedAt, age) ||
            age > config_.maximumEvidenceAge.milliseconds ())
        {
            snapshot_.disposition            = IdentityDisposition::Malformed;
            snapshot_.selectedBin            = 0;
            snapshot_.matchedBindingRevision = 0;
            snapshot_.status = !evidence.status.ok ()
                                   ? evidence.status
                                   : Status (StatusCode::InvalidArgument);
            return snapshot_.status;
        }

        if (hasObservation_)
        {
            const uint32_t delta = evidence.sequence - snapshot_.acceptedSequence;
            if (delta == 0)
            {
                if (!sourceEqual (evidence.source, lastSource_) ||
                    !identityEqual (evidence.identity, lastIdentity_) ||
                    evidence.observedAt != lastObservedAt_)
                {
                    snapshot_.disposition            = IdentityDisposition::Malformed;
                    snapshot_.selectedBin            = 0;
                    snapshot_.matchedBindingRevision = 0;
                    snapshot_.status = Status (StatusCode::InvalidArgument);
                    return snapshot_.status;
                }
                snapshot_.disposition            = IdentityDisposition::Duplicate;
                snapshot_.selectedBin            = 0;
                snapshot_.matchedBindingRevision = 0;
                snapshot_.status                 = Status ();
                return Status ();
            }
            if (!sourceEqual (evidence.source, lastSource_))
            {
                snapshot_.disposition            = IdentityDisposition::Malformed;
                snapshot_.selectedBin            = 0;
                snapshot_.matchedBindingRevision = 0;
                snapshot_.status                 = Status (StatusCode::InvalidArgument);
                return snapshot_.status;
            }
            if (delta >= 0x80000000UL)
            {
                snapshot_.disposition            = IdentityDisposition::Malformed;
                snapshot_.selectedBin            = 0;
                snapshot_.matchedBindingRevision = 0;
                snapshot_.status                 = Status (StatusCode::InvalidArgument);
                return snapshot_.status;
            }
        }

        if (snapshot_.failedAttempts >= config_.maximumFailures &&
            now.milliseconds () - lockoutStartedAt_.milliseconds () > 0x7fffffffUL)
        {
            snapshot_.disposition            = IdentityDisposition::Malformed;
            snapshot_.selectedBin            = 0;
            snapshot_.matchedBindingRevision = 0;
            snapshot_.status                 = Status (StatusCode::InvalidArgument);
            return snapshot_.status;
        }

        snapshot_.acceptedSequence       = evidence.sequence;
        lastIdentity_                    = evidence.identity;
        lastSource_                      = evidence.source;
        lastObservedAt_                  = evidence.observedAt;
        hasObservation_                  = true;
        snapshot_.selectedBin            = 0;
        snapshot_.matchedBindingRevision = 0;

        if (snapshot_.failedAttempts >= config_.maximumFailures)
        {
            const uint32_t elapsed =
                now.milliseconds () - lockoutStartedAt_.milliseconds ();
            if (elapsed > 0x7fffffffUL)
            {
                snapshot_.disposition = IdentityDisposition::Malformed;
                snapshot_.status      = Status (StatusCode::InvalidArgument);
                return snapshot_.status;
            }
            if (elapsed < config_.lockoutDuration.milliseconds ())
            {
                snapshot_.disposition = IdentityDisposition::LockedOut;
                snapshot_.status      = Status ();
                return Status ();
            }
            snapshot_.failedAttempts = 0;
        }

        for (uint8_t index = 0; index < snapshot_.bindingCount; ++index)
        {
            if (identityEqual (liveStorage_[index].identity, evidence.identity))
            {
                snapshot_.disposition            = IdentityDisposition::Known;
                snapshot_.selectedBin            = liveStorage_[index].binId;
                snapshot_.matchedBindingRevision = liveStorage_[index].revision;
                snapshot_.failedAttempts         = 0;
                snapshot_.status                 = Status ();
                return Status ();
            }
        }

        if (snapshot_.failedAttempts != 0xff)
        {
            ++snapshot_.failedAttempts;
        }
        if (snapshot_.failedAttempts >= config_.maximumFailures)
        {
            lockoutStartedAt_     = now;
            snapshot_.disposition = IdentityDisposition::LockedOut;
        }
        else
        {
            snapshot_.disposition = IdentityDisposition::Unknown;
        }
        snapshot_.status = Status ();
        return Status ();
    }

    Result<EnrollmentCandidate> LocalIdentityRegistry::previewEnrollment (
        TimePoint now, const IdentityEvidence& evidence, uint8_t binId) noexcept
    {
        EnrollmentCandidate empty{};
        if (!initialized_)
        {
            return Result<EnrollmentCandidate> (Status (StatusCode::NotInitialized),
                                                empty);
        }
        if (reconciliationRequired_)
        {
            return Result<EnrollmentCandidate> (Status (StatusCode::HardwareFailure),
                                                empty);
        }
        if (snapshot_.enrollmentPending)
        {
            return Result<EnrollmentCandidate> (Status (StatusCode::ResourceBusy),
                                                candidate_);
        }
        if (binId >= config_.binCount || snapshot_.bindingCount >= capacity_)
        {
            return Result<EnrollmentCandidate> (
                Status (snapshot_.bindingCount >= capacity_
                            ? StatusCode::CapacityExceeded
                            : StatusCode::InvalidArgument),
                empty);
        }
        uint32_t age = 0;
        if (!evidence.status.ok () || !sourceValid (evidence.source) ||
            !identityValid (evidence.identity) ||
            !presentOrPast (now, evidence.observedAt, age) ||
            age > config_.maximumEvidenceAge.milliseconds ())
        {
            const Status status = !evidence.status.ok ()
                                      ? evidence.status
                                      : Status (StatusCode::InvalidArgument);
            return Result<EnrollmentCandidate> (status, empty);
        }
        const uint32_t lockoutElapsed =
            now.milliseconds () - lockoutStartedAt_.milliseconds ();
        if (snapshot_.failedAttempts >= config_.maximumFailures &&
            (lockoutElapsed > 0x7fffffffUL ||
             lockoutElapsed < config_.lockoutDuration.milliseconds ()))
        {
            return Result<EnrollmentCandidate> (Status (StatusCode::ResourceBusy),
                                                empty);
        }
        if (hasObservation_)
        {
            const uint32_t delta = evidence.sequence - snapshot_.acceptedSequence;
            if (!sourceEqual (evidence.source, lastSource_) || delta >= 0x80000000UL ||
                (delta == 0 && (!identityEqual (evidence.identity, lastIdentity_) ||
                                evidence.observedAt != lastObservedAt_ ||
                                snapshot_.disposition != IdentityDisposition::Unknown)))
            {
                return Result<EnrollmentCandidate> (
                    Status (StatusCode::InvalidArgument), empty);
            }
        }
        for (uint8_t index = 0; index < snapshot_.bindingCount; ++index)
        {
            if (identityEqual (liveStorage_[index].identity, evidence.identity) ||
                liveStorage_[index].binId == binId)
            {
                return Result<EnrollmentCandidate> (
                    Status (StatusCode::InvalidArgument), empty);
            }
        }

        IdentityBinding staged[maximumLocalIdentities]{};
        memcpy (staged, liveStorage_,
                static_cast<size_t> (snapshot_.bindingCount) *
                    sizeof (IdentityBinding));
        IdentityBinding& added   = staged[snapshot_.bindingCount];
        added.identity           = evidence.identity;
        added.binId              = binId;
        uint16_t highestRevision = 0;
        for (uint8_t index = 0; index < snapshot_.bindingCount; ++index)
        {
            if (staged[index].revision > highestRevision)
            {
                highestRevision = staged[index].revision;
            }
        }
        added.revision = static_cast<uint16_t> (highestRevision + 1);
        if (added.revision == 0)
        {
            added.revision = 1;
        }

        ++candidateGeneration_;
        if (candidateGeneration_ == 0)
        {
            ++candidateGeneration_;
        }
        ++operationId_;
        if (operationId_ == 0)
        {
            ++operationId_;
        }
        const uint32_t nextImageGeneration = snapshot_.imageGeneration + 1;
        encodeImage (candidateImageBytes_, config_.registryConfigurationId,
                     nextImageGeneration, staged,
                     static_cast<uint8_t> (snapshot_.bindingCount + 1));
        candidate_.owner               = owner_;
        candidate_.candidateGeneration = candidateGeneration_;
        candidate_.baseImageGeneration = snapshot_.imageGeneration;
        candidate_.operationId         = operationId_;
        candidate_.scratchIndex = static_cast<uint8_t> (activeSlot_ == 0 ? 1 : 0);
        candidate_.checksum     = read16 (candidateImageBytes_ + 158);
        candidate_.status       = Status ();
        candidateSequence_      = evidence.sequence;
        candidateObservedAt_    = evidence.observedAt;
        candidateSource_        = evidence.source;
        snapshot_.disposition   = IdentityDisposition::EnrollmentPending;
        snapshot_.selectedBin   = 0;
        snapshot_.matchedBindingRevision = 0;
        snapshot_.enrollmentPending      = true;
        snapshot_.externalCommitPending  = false;
        snapshot_.status                 = Status ();
        installedEvidenceValid_          = false;
        return Result<EnrollmentCandidate> (Status (), candidate_);
    }

    Result<IdentityImageView> LocalIdentityRegistry::previewExport (
        const EnrollmentCandidate& candidate) const noexcept
    {
        IdentityImageView empty{};
        if (!initialized_)
        {
            return Result<IdentityImageView> (Status (StatusCode::NotInitialized),
                                              empty);
        }
        if (!snapshot_.enrollmentPending || candidate.owner != candidate_.owner ||
            candidate.candidateGeneration != candidate_.candidateGeneration ||
            candidate.baseImageGeneration != candidate_.baseImageGeneration ||
            candidate.operationId != candidate_.operationId ||
            candidate.scratchIndex != candidate_.scratchIndex ||
            candidate.checksum != candidate_.checksum || !candidate.status.ok ())
        {
            return Result<IdentityImageView> (Status (StatusCode::InvalidArgument),
                                              empty);
        }
        IdentityImageView view{candidateImageBytes_, localIdentityImageBytes,
                               candidate_.scratchIndex,
                               read32 (candidateImageBytes_ + 10)};
        return Result<IdentityImageView> (Status (), view);
    }

    Status LocalIdentityRegistry::acknowledgeExternalCommit (
        const EnrollmentCandidate&           candidate,
        const IdentityDurableCommitEvidence& evidence) noexcept
    {
        if (!initialized_)
        {
            return Status (StatusCode::NotInitialized);
        }

        const bool candidateMatches =
            candidate.owner == candidate_.owner &&
            candidate.candidateGeneration == candidate_.candidateGeneration &&
            candidate.baseImageGeneration == candidate_.baseImageGeneration &&
            candidate.operationId == candidate_.operationId &&
            candidate.scratchIndex == candidate_.scratchIndex &&
            candidate.checksum == candidate_.checksum && candidate.status.ok ();
        const bool durableIdentityMatches =
            evidence.owner == candidate_.owner &&
            evidence.candidateGeneration == candidate_.candidateGeneration &&
            evidence.operationId == candidate_.operationId &&
            evidence.slot == candidate_.scratchIndex &&
            evidence.reconciledImage.bytes ==
                imageSlotBytes_ + static_cast<uint16_t> (candidate_.scratchIndex) *
                                      imageSlotStride_ &&
            evidence.reconciledImage.length == localIdentityImageBytes &&
            evidence.reconciledImage.slot == candidate_.scratchIndex &&
            evidence.synchronized && evidence.rereadValidated &&
            evidence.durableStatus.ok () &&
            memcmp (evidence.reconciledImage.bytes, candidateImageBytes_,
                    localIdentityImageBytes) == 0;

        if (installedEvidenceValid_ && candidateMatches && durableIdentityMatches &&
            snapshot_.imageGeneration == evidence.reconciledImage.generation)
        {
            return Status ();
        }
        if (installedEvidenceValid_ && !snapshot_.enrollmentPending)
        {
            return Status (StatusCode::HardwareFailure);
        }
        const bool evidenceMatches =
            durableIdentityMatches &&
            evidence.reconciledImage.generation == snapshot_.imageGeneration + 1;
        if (!snapshot_.enrollmentPending || !candidateMatches || !evidenceMatches)
        {
            reconciliationRequired_         = snapshot_.enrollmentPending;
            snapshot_.externalCommitPending = reconciliationRequired_;
            snapshot_.disposition = evidence.durableStatus.ok ()
                                        ? IdentityDisposition::CommitIndeterminate
                                        : IdentityDisposition::StorageFault;
            snapshot_.status      = Status (StatusCode::HardwareFailure);
            return snapshot_.status;
        }

        IdentityBinding decoded[maximumLocalIdentities]{};
        uint8_t         count      = 0;
        uint32_t        generation = 0;
        if (validateImage (evidence.reconciledImage.bytes,
                           config_.registryConfigurationId, config_.binCount, capacity_,
                           decoded, count, generation) != IdentityDisposition::None ||
            generation != snapshot_.imageGeneration + 1)
        {
            reconciliationRequired_         = true;
            snapshot_.externalCommitPending = true;
            snapshot_.disposition           = IdentityDisposition::CommitIndeterminate;
            snapshot_.status                = Status (StatusCode::HardwareFailure);
            return snapshot_.status;
        }

        memcpy (liveStorage_, decoded,
                static_cast<size_t> (count) * sizeof (IdentityBinding));
        lastIdentity_                   = decoded[count - 1].identity;
        lastSource_                     = candidateSource_;
        lastObservedAt_                 = candidateObservedAt_;
        snapshot_.acceptedSequence      = candidateSequence_;
        hasObservation_                 = true;
        snapshot_.bindingCount          = count;
        snapshot_.imageGeneration       = generation;
        snapshot_.disposition           = IdentityDisposition::None;
        snapshot_.enrollmentPending     = false;
        snapshot_.externalCommitPending = false;
        snapshot_.status                = Status ();
        activeSlot_                     = candidate_.scratchIndex;
        reconciliationRequired_         = false;
        installedEvidenceValid_         = true;
        return Status ();
    }

    Status LocalIdentityRegistry::cancelEnrollment () noexcept
    {
        if (!initialized_)
        {
            return Status (StatusCode::NotInitialized);
        }
        if (reconciliationRequired_)
        {
            return Status (StatusCode::HardwareFailure);
        }
        candidate_                      = {};
        snapshot_.enrollmentPending     = false;
        snapshot_.externalCommitPending = false;
        snapshot_.disposition           = IdentityDisposition::None;
        snapshot_.status                = Status ();
        installedEvidenceValid_         = false;
        return Status ();
    }

    bool LocalIdentityRegistry::initialized () const noexcept
    {
        return initialized_;
    }

    IdentityRegistrySnapshot LocalIdentityRegistry::snapshot () const noexcept
    {
        return snapshot_;
    }
} // namespace adk
