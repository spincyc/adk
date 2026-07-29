#pragma once

#include "status.h"
#include "time.h"

#include <stdint.h>

namespace adk {
    // clang-format off

    enum struct Max7219Orientation : uint8_t
    {
        Identity,
        Rotate90,
        Rotate180,
        Rotate270
    };

    enum struct Max7219Operation : uint8_t
    {
        Configure,
        SubmitRow,
        CleanupShutdown
    };

    enum struct Max7219Fault : uint8_t
    {
        None,
        Transport,
        ContradictoryReceipt,
        LifecycleExhausted
    };

    struct Max7219PresentationConfig
    {
        Max7219PresentationConfig (uint32_t ownerToken,
                                   uint16_t configurationRevision,
                                   Max7219Orientation orientation,
                                   uint8_t intensity = 1) noexcept;

        uint32_t            ownerToken;
        uint16_t            configurationRevision;
        Max7219Orientation  orientation;
        uint8_t             intensity;
    };

    struct Max7219Frame
    {
        uint8_t   rows[8];
        uint32_t  sourceSnapshotSequence;
        uint32_t  generation;
    };

    struct Max7219Command
    {
        uint32_t          ownerToken;
        uint32_t          lifecycleGeneration;
        uint16_t          configurationRevision;
        uint32_t          presentationGeneration;
        uint8_t           operationIndex;
        uint8_t           registerAddress;
        uint8_t           data;
        Max7219Operation  operation;
        bool              emitted;
    };

    struct Max7219Receipt
    {
        uint32_t          ownerToken;
        uint32_t          lifecycleGeneration;
        uint16_t          configurationRevision;
        uint32_t          presentationGeneration;
        uint8_t           operationIndex;
        uint8_t           registerAddress;
        uint8_t           data;
        Max7219Operation  operation;
        uint8_t           acceptedByteCount;
        bool              chipSelectInactive;
        TimePoint         observedAt;
        Status            status;
    };

    struct Max7219Failure
    {
        Max7219Operation operation;
        uint8_t          operationIndex;
        uint8_t          registerAddress;
        uint8_t          rowIndex;
        uint8_t          acceptedByteCount;
        Status           status;
        Status           cleanupStatus;
    };

    struct Max7219PresentationSnapshot
    {
        Max7219Frame    desiredFrame;
        Max7219Frame    submittedFrame;
        Max7219Failure  failure;
        uint32_t        lifecycleGeneration;
        uint8_t         partialPrefix;
        Max7219Fault    fault;
        Status          status;
        bool            configured;
        bool            outstanding;
        bool            blankRequested;
        bool            cleanupPending;
        bool            shutdownCommandAccepted;
        bool            physicallyIndeterminate;
        bool            initialized;
    };

    struct Max7219PresentationPolicy;
    struct Max7219PresentationPolicyTestAccess;

    struct Max7219PresentationPreview
    {
        Max7219PresentationPreview () noexcept;

      private:
        const Max7219PresentationPolicy* owner;
        uint32_t                         lifecycleGeneration;
        uint32_t                         baseFrameGeneration;
        Max7219Frame                     frame;

        friend struct Max7219PresentationPolicy;
    };

    // Pure register and frame policy. It owns no SPI endpoint or display.
    struct Max7219PresentationPolicy
    {
        explicit Max7219PresentationPolicy (
            const Max7219PresentationConfig& config) noexcept;

        Max7219PresentationPolicy (const Max7219PresentationPolicy&) = delete;
        Max7219PresentationPolicy&
        operator= (const Max7219PresentationPolicy&) = delete;
        Max7219PresentationPolicy (Max7219PresentationPolicy&&) = delete;
        Max7219PresentationPolicy&
        operator= (Max7219PresentationPolicy&&) = delete;

        Status initialize () noexcept;
        void   reset      () noexcept;
        void   shutdown   () noexcept;

        Status preview (const uint8_t logicalRows[8],
                        uint32_t sourceSnapshotSequence,
                        Max7219PresentationPreview& candidate) const noexcept;
        bool   canCommit (const Max7219PresentationPreview& candidate) const
            noexcept;
        Status commit (const Max7219PresentationPreview& candidate) noexcept;

        Result<Max7219Command> service (
            const Max7219Receipt* receipt = nullptr) noexcept;

        bool                             initialized () const noexcept;
        Max7219PresentationSnapshot      snapshot    () const noexcept;

      private:
        Max7219PresentationConfig config_;
        Max7219Frame              desiredFrame_;
        Max7219Frame              submittedFrame_;
        Max7219Command            outstandingCommand_;
        Max7219Receipt            lastReceipt_;
        Max7219Failure            failure_;
        uint32_t                  lifecycleGeneration_;
        uint8_t                   nextOperationIndex_;
        uint8_t                   partialPrefix_;
        Max7219Fault              fault_;
        Status                    status_;
        bool                      configured_;
        bool                      outstanding_;
        bool                      haveLastReceipt_;
        bool                      blankRequested_;
        bool                      cleanupPending_;
        bool                      cleanupAttempted_;
        bool                      shutdownCommandAccepted_;
        bool                      physicallyIndeterminate_;
        bool                      initialized_;

        friend struct Max7219PresentationPolicyTestAccess;
    };
    // clang-format on
} // namespace adk
