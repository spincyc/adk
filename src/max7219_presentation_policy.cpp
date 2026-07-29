#include "max7219_presentation_policy.h"

namespace adk {
    // clang-format off
    namespace {
        constexpr uint32_t halfRange = 0x80000000UL;

        Max7219Frame blankFrame () noexcept
        {
            return {{0, 0, 0, 0, 0, 0, 0, 0}, 0, 0};
        }

        Max7219Command noCommand () noexcept
        {
            return {0, 0, 0, 0, 0, 0, 0, Max7219Operation::Configure, false};
        }

        Max7219Receipt noReceipt () noexcept
        {
            return {0, 0, 0, 0, 0, 0, 0, Max7219Operation::Configure,
                    0, true, TimePoint (), StatusCode::NotInitialized};
        }

        Max7219Failure noFailure () noexcept
        {
            return {Max7219Operation::Configure, 0, 0, 0xffU, 0,
                    StatusCode::Ok, StatusCode::Ok};
        }

        bool validOrientation (Max7219Orientation orientation) noexcept
        {
            return orientation == Max7219Orientation::Identity ||
                   orientation == Max7219Orientation::Rotate90 ||
                   orientation == Max7219Orientation::Rotate180 ||
                   orientation == Max7219Orientation::Rotate270;
        }

        bool validConfig (const Max7219PresentationConfig& config) noexcept
        {
            return config.ownerToken != 0 && config.configurationRevision != 0 &&
                   validOrientation (config.orientation) && config.intensity <= 15;
        }

        void transformFrame (const uint8_t source[8], Max7219Orientation orientation,
                             uint8_t destination[8]) noexcept
        {
            for (uint8_t row = 0; row < 8; ++row)
            {
                destination[row] = 0;
            }
            for (uint8_t row = 0; row < 8; ++row)
            {
                for (uint8_t column = 0; column < 8; ++column)
                {
                    if ((source[row] &
                         static_cast<uint8_t> (0x80U >> column)) == 0)
                    {
                        continue;
                    }

                    uint8_t transformedRow    = row;
                    uint8_t transformedColumn = column;
                    if (orientation == Max7219Orientation::Rotate90)
                    {
                        transformedRow    = column;
                        transformedColumn = static_cast<uint8_t> (7U - row);
                    }
                    else if (orientation == Max7219Orientation::Rotate180)
                    {
                        transformedRow    = static_cast<uint8_t> (7U - row);
                        transformedColumn = static_cast<uint8_t> (7U - column);
                    }
                    else if (orientation == Max7219Orientation::Rotate270)
                    {
                        transformedRow    = static_cast<uint8_t> (7U - column);
                        transformedColumn = row;
                    }
                    destination[transformedRow] |=
                        static_cast<uint8_t> (0x80U >> transformedColumn);
                }
            }
        }

        bool commandMatches (const Max7219Command& command,
                             const Max7219Receipt& receipt) noexcept
        {
            return command.ownerToken == receipt.ownerToken &&
                   command.lifecycleGeneration == receipt.lifecycleGeneration &&
                   command.configurationRevision ==
                       receipt.configurationRevision &&
                   command.presentationGeneration ==
                       receipt.presentationGeneration &&
                   command.operationIndex == receipt.operationIndex &&
                   command.registerAddress == receipt.registerAddress &&
                   command.data == receipt.data &&
                   command.operation == receipt.operation;
        }

        bool receiptEqual (const Max7219Receipt& left,
                           const Max7219Receipt& right) noexcept
        {
            return left.ownerToken == right.ownerToken &&
                   left.lifecycleGeneration == right.lifecycleGeneration &&
                   left.configurationRevision == right.configurationRevision &&
                   left.presentationGeneration == right.presentationGeneration &&
                   left.operationIndex == right.operationIndex &&
                   left.registerAddress == right.registerAddress &&
                   left.data == right.data && left.operation == right.operation &&
                   left.acceptedByteCount == right.acceptedByteCount &&
                   left.chipSelectInactive == right.chipSelectInactive &&
                   left.observedAt == right.observedAt &&
                   left.status == right.status;
        }

        Status receiptOutcome (const Max7219Receipt& receipt) noexcept
        {
            if (receipt.acceptedByteCount > 2 ||
                (receipt.status.ok  () && receipt.acceptedByteCount != 2) ||
                (!receipt.status.ok () && receipt.acceptedByteCount == 2))
            {
                return StatusCode::InternalInvariant;
            }
            if (!receipt.status.ok ())
            {
                return receipt.status;
            }
            return receipt.chipSelectInactive ? StatusCode::Ok
                                              : StatusCode::HardwareFailure;
        }

        void configurationRegister (uint8_t index, uint8_t intensity,
                                    uint8_t& address, uint8_t& data) noexcept
        {
            if (index == 0)
            {
                address = 0x0cU;
                data    = 0;
            }
            else if (index == 1)
            {
                address = 0x0fU;
                data    = 0;
            }
            else if (index == 2)
            {
                address = 0x09U;
                data    = 0;
            }
            else if (index == 3)
            {
                address = 0x0bU;
                data    = 7;
            }
            else if (index == 4)
            {
                address = 0x0aU;
                data    = intensity;
            }
            else if (index < 13)
            {
                address = static_cast<uint8_t> (index - 4U);
                data    = 0;
            }
            else
            {
                address = 0x0cU;
                data    = 1;
            }
        }
    } // namespace

    Max7219PresentationConfig::Max7219PresentationConfig (
        uint32_t ownerTokenValue, uint16_t configurationRevisionValue,
        Max7219Orientation orientationValue, uint8_t intensityValue) noexcept
        : ownerToken            (ownerTokenValue),
          configurationRevision (configurationRevisionValue),
          orientation           (orientationValue),
          intensity             (intensityValue)
    {
    }

    Max7219PresentationPreview::Max7219PresentationPreview () noexcept
        : owner               (nullptr),
          lifecycleGeneration (0),
          baseFrameGeneration (0),
          frame               (blankFrame ())
    {
    }

    Max7219PresentationPolicy::Max7219PresentationPolicy (
        const Max7219PresentationConfig& config) noexcept
        : config_                     (config),
          desiredFrame_               (blankFrame ()),
          submittedFrame_             (blankFrame ()),
          outstandingCommand_         (noCommand ()),
          lastReceipt_                (noReceipt ()),
          failure_                    (noFailure ()),
          lifecycleGeneration_        (0),
          nextOperationIndex_         (0),
          partialPrefix_              (0),
          fault_                      (Max7219Fault::None),
          status_                     (StatusCode::NotInitialized),
          configured_                 (false),
          outstanding_                (false),
          haveLastReceipt_            (false),
          blankRequested_             (false),
          cleanupPending_             (false),
          cleanupAttempted_           (false),
          shutdownCommandAccepted_    (false),
          physicallyIndeterminate_    (false),
          initialized_                (false)
    {
    }

    Status Max7219PresentationPolicy::initialize () noexcept
    {
        if (fault_ == Max7219Fault::LifecycleExhausted)
        {
            return StatusCode::CapacityExceeded;
        }
        if (initialized_)
        {
            return status_;
        }
        if (!validConfig (config_))
        {
            status_ = StatusCode::InvalidConfiguration;
            return status_;
        }
        initialized_ = true;
        reset ();
        return status_;
    }

    void Max7219PresentationPolicy::reset () noexcept
    {
        if (fault_ == Max7219Fault::LifecycleExhausted)
        {
            return;
        }

        const bool preserveUncertainty =
            physicallyIndeterminate_ || fault_ != Max7219Fault::None ||
            outstanding_;
        if (outstanding_ && failure_.status.ok ())
        {
            failure_.operation         = outstandingCommand_.operation;
            failure_.operationIndex    = outstandingCommand_.operationIndex;
            failure_.registerAddress   = outstandingCommand_.registerAddress;
            failure_.rowIndex =
                outstandingCommand_.operation == Max7219Operation::SubmitRow
                    ? outstandingCommand_.operationIndex
                    : 0xffU;
            failure_.acceptedByteCount = 0;
            failure_.status            = StatusCode::ResourceBusy;
            failure_.cleanupStatus     = StatusCode::Ok;
        }
        if (initialized_)
        {
            ++lifecycleGeneration_;
            if (lifecycleGeneration_ == 0)
            {
                desiredFrame_            = blankFrame         ();
                submittedFrame_          = blankFrame         ();
                outstandingCommand_      = noCommand          ();
                nextOperationIndex_      = 0;
                partialPrefix_           = 0;
                fault_                   = Max7219Fault::LifecycleExhausted;
                status_                  = StatusCode::CapacityExceeded;
                configured_              = false;
                outstanding_             = false;
                blankRequested_          = true;
                cleanupPending_          = false;
                cleanupAttempted_        = false;
                shutdownCommandAccepted_ = false;
                physicallyIndeterminate_ = true;
                initialized_             = false;
                return;
            }
        }

        desiredFrame_            = blankFrame         ();
        submittedFrame_          = blankFrame         ();
        outstandingCommand_      = noCommand          ();
        lastReceipt_             = noReceipt          ();
        if (!preserveUncertainty)
        {
            failure_ = noFailure ();
        }
        nextOperationIndex_      = 0;
        partialPrefix_           = 0;
        fault_                   = Max7219Fault::None;
        status_                  = initialized_ ? StatusCode::Ok
                                                : StatusCode::NotInitialized;
        configured_              = false;
        outstanding_             = false;
        haveLastReceipt_         = false;
        blankRequested_          = initialized_;
        cleanupPending_          = false;
        cleanupAttempted_        = false;
        if (!preserveUncertainty)
        {
            shutdownCommandAccepted_ = false;
        }
        physicallyIndeterminate_ = preserveUncertainty;
    }

    void Max7219PresentationPolicy::shutdown () noexcept
    {
        desiredFrame_            = blankFrame    ();
        submittedFrame_          = blankFrame    ();
        outstandingCommand_      = noCommand     ();
        nextOperationIndex_      = 0;
        partialPrefix_           = 0;
        status_                  = fault_ == Max7219Fault::LifecycleExhausted
                                       ? StatusCode::CapacityExceeded
                                       : StatusCode::NotInitialized;
        configured_              = false;
        outstanding_             = false;
        blankRequested_          = true;
        cleanupPending_          = false;
        cleanupAttempted_        = false;
        physicallyIndeterminate_ =
            physicallyIndeterminate_ || initialized_;
        initialized_ = false;
    }

    Status Max7219PresentationPolicy::preview (
        const uint8_t logicalRows[8], uint32_t sourceSnapshotSequence,
        Max7219PresentationPreview& candidate) const noexcept
    {
        candidate.owner = nullptr;
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        if (fault_ != Max7219Fault::None || logicalRows == nullptr ||
            sourceSnapshotSequence == 0)
        {
            return StatusCode::InvalidArgument;
        }
        if (!configured_ || outstanding_ ||
            desiredFrame_.generation != submittedFrame_.generation)
        {
            return StatusCode::ResourceBusy;
        }

        Max7219Frame frame = blankFrame ();

        transformFrame (logicalRows, config_.orientation, frame.rows);
        frame.sourceSnapshotSequence = sourceSnapshotSequence;
        frame.generation             = desiredFrame_.generation + 1U;
        if (frame.generation == 0)
        {
            return StatusCode::CapacityExceeded;
        }

        candidate.owner               = this;
        candidate.lifecycleGeneration = lifecycleGeneration_;
        candidate.baseFrameGeneration = desiredFrame_.generation;
        candidate.frame               = frame;
        return StatusCode::Ok;
    }

    bool Max7219PresentationPolicy::canCommit (
        const Max7219PresentationPreview& candidate) const noexcept
    {
        return initialized_ && configured_ &&
               fault_ == Max7219Fault::None && !outstanding_ &&
               desiredFrame_.generation == submittedFrame_.generation &&
               candidate.owner == this &&
               candidate.lifecycleGeneration == lifecycleGeneration_ &&
               candidate.baseFrameGeneration == desiredFrame_.generation &&
               candidate.frame.generation == desiredFrame_.generation + 1U &&
               candidate.frame.generation != 0;
    }

    Status Max7219PresentationPolicy::commit (
        const Max7219PresentationPreview& candidate) noexcept
    {
        if (!canCommit (candidate))
        {
            return outstanding_ ||
                           desiredFrame_.generation != submittedFrame_.generation
                       ? StatusCode::ResourceBusy
                       : StatusCode::InvalidArgument;
        }
        desiredFrame_      = candidate.frame;
        partialPrefix_     = 0;
        blankRequested_    = false;
        return StatusCode::Ok;
    }

    Result<Max7219Command> Max7219PresentationPolicy::service (
        const Max7219Receipt* receipt) noexcept
    {
        Max7219Command command = noCommand ();
        if (!initialized_)
        {
            return {StatusCode::NotInitialized, command};
        }
        if (receipt != nullptr)
        {
            if (haveLastReceipt_ && receiptEqual (*receipt, lastReceipt_))
            {
                return {status_, command};
            }
            if (!outstanding_)
            {
                return {StatusCode::InvalidArgument, command};
            }
            if (!commandMatches (outstandingCommand_, *receipt))
            {
                return {StatusCode::InvalidArgument, command};
            }
            if (haveLastReceipt_)
            {
                const uint32_t elapsed =
                    receipt->observedAt.elapsedSince (lastReceipt_.observedAt)
                        .milliseconds ();
                if (elapsed == 0 || elapsed >= halfRange)
                {
                    return {StatusCode::InvalidArgument, command};
                }
            }

            const Status outcome = receiptOutcome (*receipt);
            lastReceipt_         = *receipt;
            haveLastReceipt_     = true;
            outstanding_         = false;
            outstandingCommand_  = noCommand ();

            if (!outcome.ok ())
            {
                if (receipt->operation == Max7219Operation::CleanupShutdown)
                {
                    failure_.cleanupStatus = outcome;
                    cleanupPending_        = false;
                    cleanupAttempted_      = true;
                    return {status_, command};
                }

                failure_.operation         = receipt->operation;
                failure_.operationIndex    = receipt->operationIndex;
                failure_.registerAddress   = receipt->registerAddress;
                failure_.rowIndex =
                    receipt->operation == Max7219Operation::SubmitRow
                        ? receipt->operationIndex
                        : 0xffU;
                failure_.acceptedByteCount = receipt->acceptedByteCount;
                failure_.status            = outcome;
                fault_ = outcome.error () == StatusCode::InternalInvariant
                             ? Max7219Fault::ContradictoryReceipt
                             : Max7219Fault::Transport;
                status_                  = outcome;
                blankRequested_          = true;
                cleanupPending_          = true;
                physicallyIndeterminate_ = true;
                return {status_, command};
            }

            if (receipt->operation == Max7219Operation::CleanupShutdown)
            {
                failure_.cleanupStatus      = StatusCode::Ok;
                cleanupPending_             = false;
                cleanupAttempted_           = true;
                shutdownCommandAccepted_    = true;
                return {status_, command};
            }
            if (receipt->operation == Max7219Operation::Configure)
            {
                ++nextOperationIndex_;
                if (nextOperationIndex_ == 14)
                {
                    configured_              = true;
                    nextOperationIndex_      = 0;
                    blankRequested_          = false;
                    shutdownCommandAccepted_ = false;
                }
            }
            else
            {
                ++partialPrefix_;
                if (partialPrefix_ == 8)
                {
                    submittedFrame_ = desiredFrame_;
                    partialPrefix_  = 0;
                }
            }
            return {StatusCode::Ok, command};
        }

        if (outstanding_)
        {
            return {StatusCode::ResourceBusy, command};
        }
        if (fault_ != Max7219Fault::None)
        {
            if (!cleanupPending_ || cleanupAttempted_)
            {
                return {status_, command};
            }
            command = {config_.ownerToken,
                       lifecycleGeneration_,
                       config_.configurationRevision,
                       desiredFrame_.generation,
                       0,
                       0x0cU,
                       0,
                       Max7219Operation::CleanupShutdown,
                       true};
            cleanupAttempted_   = true;
            outstandingCommand_ = command;
            outstanding_        = true;
            return {StatusCode::Ok, command};
        }

        uint8_t address = 0;
        uint8_t data    = 0;
        Max7219Operation operation = Max7219Operation::Configure;
        uint32_t presentationGeneration = 0;
        uint8_t operationIndex = nextOperationIndex_;
        if (!configured_)
        {
            configurationRegister (nextOperationIndex_, config_.intensity, address,
                                   data);
        }
        else if (desiredFrame_.generation != submittedFrame_.generation)
        {
            operation              = Max7219Operation::SubmitRow;
            operationIndex         = partialPrefix_;
            address                = static_cast<uint8_t> (partialPrefix_ + 1U);
            data                   = desiredFrame_.rows[partialPrefix_];
            presentationGeneration = desiredFrame_.generation;
        }
        else
        {
            return {StatusCode::Ok, command};
        }

        command = {config_.ownerToken,
                   lifecycleGeneration_,
                   config_.configurationRevision,
                   presentationGeneration,
                   operationIndex,
                   address,
                   data,
                   operation,
                   true};
        outstandingCommand_ = command;
        outstanding_        = true;
        return {StatusCode::Ok, command};
    }

    bool Max7219PresentationPolicy::initialized () const noexcept
    {
        return initialized_;
    }

    Max7219PresentationSnapshot
    Max7219PresentationPolicy::snapshot () const noexcept
    {
        return {desiredFrame_,
                submittedFrame_,
                failure_,
                lifecycleGeneration_,
                partialPrefix_,
                fault_,
                status_,
                configured_,
                outstanding_,
                blankRequested_,
                cleanupPending_,
                shutdownCommandAccepted_,
                physicallyIndeterminate_,
                initialized_};
    }
    // clang-format on
} // namespace adk
