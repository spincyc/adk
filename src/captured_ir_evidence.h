#pragma once

#include "infrared_decoder.h"
#include "status.h"

#include <stdint.h>

namespace adk {

    static constexpr uint8_t  capturedIrPulseCapacity = PulseCapture::capacity;
    static constexpr uint32_t capturedIrMarkMask      = UINT32_C (0x80000000);
    static constexpr uint32_t capturedIrDurationMask  = UINT32_C (0x7fffffff);

    struct IrPulseStorage
    {
        uint32_t* data;
        uint8_t   capacity;
    };

    enum struct IrSourceKind : uint8_t
    {
        SyntheticFixture  = 0,
        QualifiedReceiver = 1,
        LocalCatalog      = 2,
        QualifiedEmitter  = 3
    };

    struct IrSourceIdentity
    {
        IrSourceKind kind;
        uint8_t      sourceId;
        uint16_t     configurationRevision;
        uint32_t     sessionEpoch;
    };

    enum struct IrCaptureDisposition : uint8_t
    {
        None               = 0,
        KnownValid         = 1,
        KnownRepeat        = 2,
        UnknownProtocol    = 3,
        TimingInvalid      = 4,
        IntegrityInvalid   = 5,
        Truncated          = 6,
        DecoderOverflow    = 7,
        CaptureOverflow    = 8,
        CaptureTimingFault = 9,
        SourceFault        = 10
    };

    enum struct EvidenceStrength : uint8_t
    {
        None              = 0,
        ShapeRecognized   = 1,
        IntegrityVerified = 2
    };

    struct CapturedIrProvenance
    {
        IrSourceIdentity     source;
        MicrosecondTimePoint observedAt;
        uint32_t             captureSequence;
        CaptureState         captureState;
        InfraredProtocol     protocol;
        FrameValidity        decoderValidity;
        Status               sourceStatus;
    };

    struct CapturedIrSnapshot
    {
        IrCaptureDisposition disposition;
        EvidenceStrength     strength;
        CapturedIrProvenance provenance;
        uint32_t             address;
        uint32_t             command;
        uint32_t             evidenceGeneration;
        uint8_t              pulseCount;
        Status               status;
    };

    struct CapturedIrView
    {
        const uint32_t* words;
        uint8_t         size;
        const void*     owner;
        uint32_t        evidenceGeneration;
    };

    struct CapturedIrEvidence
    {
        CapturedIrEvidence (InfraredDecoder& decoder, IrPulseStorage pulseStorage,
                            uint8_t maximumPulseCount) noexcept;
        ~CapturedIrEvidence () noexcept;

        CapturedIrEvidence (const CapturedIrEvidence&)            = delete;
        CapturedIrEvidence& operator= (const CapturedIrEvidence&) = delete;
        CapturedIrEvidence (CapturedIrEvidence&&)                 = delete;
        CapturedIrEvidence& operator= (CapturedIrEvidence&&)      = delete;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        void   reset      () noexcept;

        Status admit (const PulseFrame& frame, const IrSourceIdentity& source,
                      Status sourceStatus, MicrosecondTimePoint observedAt) noexcept;

        CapturedIrSnapshot     snapshot () const noexcept;
        Result<CapturedIrView> view     () const noexcept;

        Result<uint8_t> requiredWords (const CapturedIrView& view) const noexcept;
        Result<uint8_t> exportWords   (const CapturedIrView& view,
                                     IrPulseStorage        destination) const noexcept;

      private:
        bool validConfiguration () const noexcept;
        bool validView          (const CapturedIrView& view) const noexcept;

        InfraredDecoder*   decoder_;
        IrPulseStorage     pulseStorage_;
        CapturedIrSnapshot snapshot_;
        uint32_t           generation_;
        uint8_t            maximumPulseCount_;
        bool               initialized_;
        bool               hasEvidence_;
    };
} // namespace adk
