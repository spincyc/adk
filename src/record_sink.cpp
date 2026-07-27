#include "record_sink.h"

#include "storage.h"

namespace adk {

    RecordSink::~RecordSink () noexcept
    {
    }

    StorageRecordSink::StorageRecordSink (Storage& storage) noexcept
        : storage_ (&storage), initialized_ (false)
    {
    }

    StorageRecordSink::~StorageRecordSink () noexcept
    {
        shutdown ();
    }

    Status StorageRecordSink::initialize () noexcept
    {
        if (initialized_)
        {
            return StatusCode::Ok;
        }

        const Status status = storage_->initialize ();

        if (!status.ok ())
        {
            storage_->shutdown ();
            return status;
        }

        initialized_ = true;
        return StatusCode::Ok;
    }

    void StorageRecordSink::shutdown () noexcept
    {
        if (!initialized_)
        {
            return;
        }

        storage_->shutdown ();
        initialized_ = false;
    }

    Status StorageRecordSink::append (const StableRecord& record) noexcept
    {
        if (!initialized_)
        {
            return StatusCode::NotInitialized;
        }

        Status status = storage_->append (
            reinterpret_cast<const uint8_t*> (record.text), record.length);

        if (!status.ok ())
        {
            const Status rollbackStatus = discardStaged ();
            return rollbackStatus.ok                    () ? status : rollbackStatus;
        }

        status = storage_->sync ();

        if (!status.ok ())
        {
            const Status rollbackStatus = discardStaged ();
            return rollbackStatus.ok                    () ? status : rollbackStatus;
        }

        return StatusCode::Ok;
    }

    bool StorageRecordSink::initialized () const noexcept
    {
        return initialized_;
    }

    Status StorageRecordSink::discardStaged () noexcept
    {
        storage_->shutdown ();
        initialized_ = false;

        const Status status = storage_->initialize ();

        initialized_ = status.ok ();
        return status;
    }
} // namespace adk
