#pragma once

#include "status.h"

#include <stdint.h>

namespace adk {

    struct Storage;

    struct StableRecord
    {
        static const uint8_t capacity = 96;

        char    text[capacity];
        uint8_t length;
    };

    struct RecordSink
    {
        virtual ~RecordSink () noexcept;

        virtual Status initialize  () noexcept                           = 0;
        virtual void   shutdown    () noexcept                           = 0;
        virtual Status append      (const StableRecord& record) noexcept = 0;
        virtual bool   initialized () const noexcept                     = 0;
    };

    struct StorageRecordSink final : RecordSink
    {
        explicit StorageRecordSink  (Storage& storage) noexcept;
        ~StorageRecordSink          () noexcept override;

        StorageRecordSink (const StorageRecordSink&)            = delete;
        StorageRecordSink& operator= (const StorageRecordSink&) = delete;
        StorageRecordSink (StorageRecordSink&&)                 = delete;
        StorageRecordSink& operator= (StorageRecordSink&&)      = delete;

        Status initialize  () noexcept override;
        void   shutdown    () noexcept override;
        Status append      (const StableRecord& record) noexcept override;
        bool   initialized () const noexcept override;

      private:
        Status discardStaged () noexcept;

        Storage* storage_;
        bool     initialized_;
    };
} // namespace adk
