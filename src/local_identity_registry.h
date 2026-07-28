#pragma once

#include "carousel_evidence.h"

#include <stdint.h>

namespace adk {
    // clang-format off

    static constexpr uint8_t  maximumLocalIdentities    = 8;
    static constexpr uint8_t  maximumCarouselBins       = 8;
    static constexpr uint16_t localIdentityImageMagic   = 0x4944;
    static constexpr uint8_t  localIdentityImageVersion = 1;
    static constexpr uint8_t  localIdentitySlotCount    = 2;
    static constexpr uint16_t localIdentityImageBytes   = 160;

    enum struct IdentityDisposition : uint8_t
    {
        None,
        Known,
        Unknown,
        Duplicate,
        LockedOut,
        DirectoryFull,
        AuthorizationRequired,
        EnrollmentPending,
        Malformed,
        ImageCorrupt,
        ImageUnsupported,
        CommitIndeterminate,
        StorageFault
    };

    struct IdentityBinding
    {
        LocalIdentity identity;
        uint8_t       binId;
        uint16_t      revision;
        uint16_t      checksum;
    };

    struct IdentityImageView
    {
        const uint8_t* bytes;
        uint16_t       length;
        uint8_t        slot;
        uint32_t       generation;
    };

    struct LocalIdentityRegistryConfig
    {
        uint32_t registryConfigurationId;
        uint8_t  binCount;
        uint8_t  maximumFailures;
        Duration lockoutDuration;
        Duration maximumEvidenceAge;
    };

    struct IdentityRegistrySnapshot
    {
        IdentityDisposition disposition;
        uint8_t             selectedBin;
        uint8_t             bindingCount;
        uint8_t             failedAttempts;
        bool                enrollmentPending;
        bool                externalCommitPending;
        uint32_t            imageGeneration;
        uint16_t            matchedBindingRevision;
        uint32_t            acceptedSequence;
        Status              status;
    };

    struct EnrollmentCandidate
    {
        uint32_t owner;
        uint32_t candidateGeneration;
        uint32_t baseImageGeneration;
        uint32_t operationId;
        uint8_t  scratchIndex;
        uint16_t checksum;
        Status   status;
    };

    struct IdentityDurableCommitEvidence
    {
        uint32_t          owner;
        uint32_t          candidateGeneration;
        uint32_t          operationId;
        uint8_t           slot;
        IdentityImageView reconciledImage;
        bool              synchronized;
        bool              rereadValidated;
        Status            durableStatus;
    };

    struct LocalIdentityRegistry
    {
        LocalIdentityRegistry (const LocalIdentityRegistryConfig& config,
                               IdentityBinding*                   liveStorage,
                               uint8_t                            capacity,
                               uint8_t*                           imageSlotBytes,
                               uint16_t                           imageSlotByteExtent,
                               uint16_t                           imageSlotStride,
                               uint8_t                            imageSlotCount,
                               uint8_t*                           candidateImageBytes,
                               uint16_t candidateImageCapacity) noexcept;

        Status initialize () noexcept;
        Status reset      () noexcept;
        void   shutdown   () noexcept;

        Status observe (TimePoint now, const IdentityEvidence& evidence) noexcept;
        Result<EnrollmentCandidate> previewEnrollment (
            TimePoint now, const IdentityEvidence& evidence, uint8_t binId) noexcept;
        Result<IdentityImageView> previewExport (
            const EnrollmentCandidate& candidate) const noexcept;
        Status acknowledgeExternalCommit (
            const EnrollmentCandidate& candidate,
            const IdentityDurableCommitEvidence& evidence) noexcept;
        Status cancelEnrollment () noexcept;

        bool                     initialized () const noexcept;
        IdentityRegistrySnapshot snapshot    () const noexcept;

        LocalIdentityRegistry (const LocalIdentityRegistry&)            = delete;
        LocalIdentityRegistry& operator= (const LocalIdentityRegistry&) = delete;
        LocalIdentityRegistry (LocalIdentityRegistry&&)                 = delete;
        LocalIdentityRegistry& operator= (LocalIdentityRegistry&&)      = delete;

      private:
        LocalIdentityRegistryConfig config_;
        IdentityBinding*            liveStorage_;
        uint8_t*                    imageSlotBytes_;
        uint8_t*                    candidateImageBytes_;
        IdentityRegistrySnapshot    snapshot_;
        LocalIdentity               lastIdentity_;
        CarouselSource              lastSource_;
        EnrollmentCandidate         candidate_;
        uint16_t                    imageSlotByteExtent_;
        uint16_t                    imageSlotStride_;
        uint16_t                    candidateImageCapacity_;
        uint32_t                    owner_;
        uint32_t                    candidateGeneration_;
        uint32_t                    operationId_;
        uint32_t                    candidateSequence_;
        TimePoint                   lastObservedAt_;
        TimePoint                   candidateObservedAt_;
        TimePoint                   lockoutStartedAt_;
        CarouselSource              candidateSource_;
        uint8_t                     capacity_;
        uint8_t                     imageSlotCount_;
        uint8_t                     activeSlot_;
        bool                        initialized_;
        bool                        hasObservation_;
        bool                        reconciliationRequired_;
        bool                        installedEvidenceValid_;
    };
    // clang-format on
} // namespace adk
