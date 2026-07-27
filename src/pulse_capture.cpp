#include "pulse_capture.h"

#include "board.h"

namespace adk {

    Pulse::Pulse (PulseLevel level, MicrosecondDuration duration) noexcept
        : level (level), duration (duration)
    {
    }

    PulseCapture::PulseCapture (ResourceRegistry& resources, PulseCaptureIo& io,
                                PinId pin, const PulseCaptureConfig& config) noexcept
        : resources_ (&resources), io_ (&io), pinClaim_ (), interruptClaim_ (),
          config_                                       (config), captureDurations_{}, captureLevels_{}, published_{},
          edgeTimes_{}, edgeLevels_{}, initializedAtUs_ (0), lastEdgeUs_ (0),
          frame_                                        ({published_, 0, 0, CaptureState::Idle}),
          captureState_                                 (CaptureState::Idle), pendingState_ (CaptureState::Idle),
          pin_                                          (pin), captureSize_ (0), edgeCounts_{}, activeEdgeQueue_ (0),
          nextSequence_                                 (1), inputHigh_ (true),
          haveEdge_                                     (false), idleGapSeen_ (false), initialized_ (false),
          awaitingAcknowledgement_                      (false), pendingPublication_ (false),
          edgeQueueOverflow_                            (false), overrun_ (false)
    {
    }

    PulseCapture::~PulseCapture () noexcept
    {
        shutdown ();
    }

    Status PulseCapture::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }
        if (!validConfig ())
        {
            return StatusCode::InvalidArgument;
        }
        if (!Mega2560Board::validPin (pin_))
        {
            return StatusCode::InvalidPin;
        }
        if (!Mega2560Board::supports (pin_, PinCapability::ExternalInterrupt))
        {
            return StatusCode::Unsupported;
        }

        Status status = resources_->claim ({ResourceKind::Pin, pin_}, pinClaim_);
        if (!status.ok                    ())
        {
            return status;
        }

        status = resources_->claim ({ResourceKind::Interrupt, interruptFor (pin_)},
                                    interruptClaim_);
        if (!status.ok ())
        {
            pinClaim_.release ();
            return status;
        }

        status = io_->start (pin_, *this);
        if (!status.ok      ())
        {
            interruptClaim_.release ();
            pinClaim_.release       ();
            return status;
        }

        inputHigh_               = io_->inputHigh ();
        initializedAtUs_         = io_->now       ().microseconds ();
        frame_                   = {published_, 0, 0, CaptureState::Idle};
        captureState_            = CaptureState::Idle;
        pendingState_            = CaptureState::Idle;
        captureSize_             = 0;
        haveEdge_                = false;
        idleGapSeen_             = false;
        awaitingAcknowledgement_ = false;
        pendingPublication_      = false;
        edgeCounts_[0]           = 0;
        edgeCounts_[1]           = 0;
        activeEdgeQueue_         = 0;
        edgeQueueOverflow_       = false;
        overrun_                 = false;
        initialized_             = true;
        return StatusCode::Ok;
    }

    void PulseCapture::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        io_->stop               ();
        interruptClaim_.release ();
        pinClaim_.release       ();
        resetCapture            ();
        frame_                   = {published_, 0, 0, CaptureState::Idle};
        pendingPublication_      = false;
        awaitingAcknowledgement_ = false;
        initialized_             = false;
    }

    Status PulseCapture::update (MicrosecondTimePoint now) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        io_->lock ();
        const uint8_t processingQueue = activeEdgeQueue_;
        const uint8_t edgeCount       = edgeCounts_[processingQueue];
        const bool    queueOverflow   = edgeQueueOverflow_;
        activeEdgeQueue_              = static_cast<uint8_t> (processingQueue ^ 1U);
        edgeCounts_[activeEdgeQueue_] = 0;
        edgeQueueOverflow_            = false;
        io_->unlock ();

        for (uint8_t index = 0; index < edgeCount; ++index)
        {
            processEdge (MicrosecondTimePoint (edgeTimes_[processingQueue][index]),
                         edgeLevels_[processingQueue][index] != 0);
        }
        edgeCounts_[processingQueue] = 0;
        if (queueOverflow)
        {
            pendingState_       = CaptureState::Overflow;
            pendingPublication_ = true;
        }

        if (captureState_ == CaptureState::Idle &&
            ((!haveEdge_ &&
              now.elapsedSince (MicrosecondTimePoint (initializedAtUs_))
                      .microseconds () >= config_.frameGap.microseconds ()) ||
             (haveEdge_ &&
              now.elapsedSince (MicrosecondTimePoint (lastEdgeUs_)).microseconds () >=
                  config_.frameGap.microseconds ())))
        {
            idleGapSeen_ = true;
        }
        else if (captureState_ == CaptureState::Capturing &&
                 now.elapsedSince (MicrosecondTimePoint (lastEdgeUs_))
                         .microseconds () >= config_.frameGap.microseconds ())
        {
            pendingState_ =
                captureSize_ == 0 ? CaptureState::TimingFault : CaptureState::Complete;
            pendingPublication_ = true;
        }

        if (pendingPublication_)
        {
            publish (pendingState_);
            pendingPublication_ = false;
            resetCapture ();
            idleGapSeen_ = !overrun_;
        }

        const bool overrun = overrun_;
        overrun_           = false;
        return overrun ? StatusCode::CapacityExceeded : StatusCode::Ok;
    }

    PulseFrame PulseCapture::frame () const noexcept
    {
        return frame_;
    }

    Status PulseCapture::acknowledge (uint32_t sequence) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }
        io_->lock ();
        if (!awaitingAcknowledgement_ || sequence != frame_.sequence)
        {
            io_->unlock ();
            return StatusCode::InvalidArgument;
        }

        awaitingAcknowledgement_ = false;
        frame_                   = {published_, 0, frame_.sequence, CaptureState::Idle};
        io_->unlock ();
        return StatusCode::Ok;
    }

    void PulseCapture::recordActiveLowEdge (MicrosecondTimePoint now,
                                            bool                 inputHigh) noexcept
    {
        if (!initialized_)
        {
            return;
        }

        const uint8_t queue = activeEdgeQueue_;
        const uint8_t count = edgeCounts_[queue];
        if (count >= capacity + 1)
        {
            edgeQueueOverflow_ = true;
            return;
        }
        edgeTimes_[queue][count]  = now.microseconds ();
        edgeLevels_[queue][count] = inputHigh ? 1U : 0U;
        edgeCounts_[queue]        = static_cast<uint8_t> (count + 1U);
    }

    void PulseCapture::processEdge (MicrosecondTimePoint now, bool inputHigh) noexcept
    {
        if (awaitingAcknowledgement_ || pendingPublication_)
        {
            overrun_     = true;
            inputHigh_   = inputHigh;
            lastEdgeUs_  = now.microseconds ();
            haveEdge_    = true;
            idleGapSeen_ = false;
            return;
        }

        if (captureState_ != CaptureState::Capturing)
        {
            const MicrosecondTimePoint idleStarted (haveEdge_ ? lastEdgeUs_
                                                              : initializedAtUs_);
            if (now.elapsedSince (idleStarted).microseconds () >=
                config_.frameGap.microseconds ())
            {
                idleGapSeen_ = true;
            }

            if (!idleGapSeen_)
            {
                inputHigh_  = inputHigh;
                lastEdgeUs_ = now.microseconds ();
                haveEdge_   = true;
                return;
            }

            resetCapture ();
            captureState_ = CaptureState::Capturing;
            inputHigh_    = inputHigh;
            lastEdgeUs_   = now.microseconds ();
            haveEdge_     = true;
            idleGapSeen_  = false;
            return;
        }

        const MicrosecondDuration duration =
            now.elapsedSince (MicrosecondTimePoint (lastEdgeUs_));
        if (duration.microseconds () < config_.minimumPulse.microseconds () ||
            duration.microseconds () > config_.maximumPulse.microseconds ())
        {
            pendingState_       = CaptureState::TimingFault;
            pendingPublication_ = true;
            captureState_       = CaptureState::Idle;
            inputHigh_          = inputHigh;
            lastEdgeUs_         = now.microseconds ();
            haveEdge_           = true;
            return;
        }

        if (captureSize_ == capacity)
        {
            pendingState_       = CaptureState::Overflow;
            pendingPublication_ = true;
            captureState_       = CaptureState::Idle;
            inputHigh_          = inputHigh;
            lastEdgeUs_         = now.microseconds ();
            haveEdge_           = true;
            return;
        }

        captureLevels_[captureSize_] =
            static_cast<uint8_t> (inputHigh_ ? PulseLevel::Space : PulseLevel::Mark);
        captureDurations_[captureSize_] = duration.microseconds ();
        ++captureSize_;
        inputHigh_  = inputHigh;
        lastEdgeUs_ = now.microseconds ();
    }

    PinId PulseCapture::pin () const noexcept
    {
        return pin_;
    }

    bool PulseCapture::initialized () const noexcept
    {
        return initialized_;
    }

    bool PulseCapture::validConfig () const noexcept
    {
        const uint32_t gap     = config_.frameGap.microseconds     ();
        const uint32_t minimum = config_.minimumPulse.microseconds ();
        const uint32_t maximum = config_.maximumPulse.microseconds ();
        return minimum != 0 && minimum <= maximum && maximum < gap &&
               gap < 0x80000000UL;
    }

    Status PulseCapture::publish (CaptureState state) noexcept
    {
        if (awaitingAcknowledgement_)
        {
            overrun_ = true;
            return StatusCode::CapacityExceeded;
        }

        for (uint8_t index = 0; index < captureSize_; ++index)
        {
            published_[index] = {static_cast<PulseLevel> (captureLevels_[index]),
                                 MicrosecondDuration (captureDurations_[index])};
        }
        frame_        = {published_, captureSize_, nextSequence_, state};
        nextSequence_ = nextSequence_ == UINT32_MAX ? 1U : nextSequence_ + 1U;
        awaitingAcknowledgement_ = true;
        return StatusCode::Ok;
    }

    void PulseCapture::resetCapture () noexcept
    {
        captureSize_  = 0;
        haveEdge_     = false;
        captureState_ = CaptureState::Idle;
    }

    uint8_t PulseCapture::interruptFor (PinId pin) const noexcept
    {
        switch (pin)
        {
            case 2: return 0;
            case 3: return 1;
            case 21: return 2;
            case 20: return 3;
            case 19: return 4;
            case 18: return 5;
            default: return UINT8_MAX;
        }
    }
} // namespace adk
