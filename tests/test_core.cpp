#include <resource.h>
#include <runtime.h>
#include <status.h>
#include <time.h>

#include <array>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <type_traits>

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    void requireStatus (
        adk::Status     actual,
        adk::StatusCode expected,
        const char*     message)
    {
        require (actual.error () == expected, message);
    }

    adk::ResourceId pin (uint8_t index)
    {
        return {adk::ResourceKind::Pin, index};
    }

    struct PairClaim
    {
        explicit PairClaim (adk::ResourceRegistry& resources) noexcept
            : resources_ (resources)
            , first_     ()
            , second_    ()
            , active_    (false)
        {
        }

        ~PairClaim () noexcept
        {
            shutdown ();
        }

        adk::Status initialize (
            adk::ResourceId first,
            adk::ResourceId second) noexcept
        {
            if (active_)
            {
                return adk::StatusCode::Ok;
            }

            adk::Status status = resources_.claim (first, first_);

            if (!status.ok ())
            {
                return status;
            }

            status = resources_.claim (second, second_);

            if (!status.ok ())
            {
                first_.release ();
                return status;
            }

            active_ = true;
            return adk::StatusCode::Ok;
        }

        void shutdown () noexcept
        {
            second_.release ();
            first_.release  ();
            active_ = false;
        }

        bool initialized () const noexcept
        {
            return active_;
        }

      private:
        adk::ResourceRegistry& resources_;
        adk::ResourceClaim     first_;
        adk::ResourceClaim     second_;
        bool                   active_;
    };

    void testStatus ()
    {
        constexpr adk::Result<int> compileTimeSuccess (
            adk::StatusCode::Ok, 42);
        constexpr adk::Result<int> compileTimeFailure (
            adk::StatusCode::HardwareFailure, 0);

        static_assert (compileTimeSuccess.ok (), "constexpr result success");
        static_assert (
            compileTimeSuccess.value () == 42,
            "constexpr result value");
        static_assert (
            compileTimeFailure.error () == adk::StatusCode::HardwareFailure,
            "constexpr result error");
        static_assert (
            compileTimeFailure.transient (),
            "constexpr result transient cause");

        const adk::Result<int> success (adk::StatusCode::Ok, 42);
        const adk::Result<int> failure (adk::StatusCode::HardwareFailure, 0);

        require (success.ok (), "successful result");
        require (success.status ().ok (), "successful status");
        require (success.value () == 42, "successful value");
        require (!failure.ok (), "failed result");
        require (
            failure.status ().error () == adk::StatusCode::HardwareFailure,
            "failed status");
        require (
            failure.error () == adk::StatusCode::HardwareFailure,
            "result exposes error code");
        require (failure.transient (), "result exposes transient failure");
        require (adk::Status ().ok (), "default status succeeds");

        const adk::Status invalidPin      (adk::StatusCode::InvalidPin);
        const adk::Status resourceBusy    (adk::StatusCode::ResourceBusy);
        const adk::Status hardwareFailure (adk::StatusCode::HardwareFailure);
        const adk::Status invalidArgument (adk::StatusCode::InvalidArgument);
        const adk::Status capacityFailure (adk::StatusCode::CapacityExceeded);

        require (!invalidPin.ok (), "error status fails");
        require (
            invalidPin.error () == adk::StatusCode::InvalidPin,
            "status exposes error code");
        require (resourceBusy.transient (), "resource contention is transient");
        require (hardwareFailure.transient (), "hardware failure is transient");
        require (!invalidArgument.transient (), "invalid argument is permanent");
        require (!capacityFailure.transient (), "capacity failure is permanent");

        const adk::Status statuses[] = {
            adk::StatusCode::Ok,
            adk::StatusCode::InvalidArgument,
            adk::StatusCode::InvalidPin,
            adk::StatusCode::Unsupported,
            adk::StatusCode::ResourceBusy,
            adk::StatusCode::NotInitialized,
            adk::StatusCode::CapacityExceeded,
            adk::StatusCode::HardwareFailure
        };

        for (adk::Status status : statuses)
        {
            const char* name = adk::statusName (status);

            require (name != nullptr, "status name exists");
            require (name[0] != '\0', "status name is not empty");
        }
    }

    void testTime ()
    {
        const adk::Duration zero;
        const adk::Duration shortDelay (5);
        const adk::Duration longDelay  (10);

        require (zero.milliseconds () == 0, "default duration");
        require (shortDelay < longDelay, "duration less than");
        require (shortDelay <= shortDelay, "duration less than or equal");
        require (longDelay > shortDelay, "duration greater than");
        require (longDelay >= longDelay, "duration greater than or equal");
        require (shortDelay != longDelay, "duration inequality");

        const adk::TimePoint earlier (100);
        const adk::TimePoint later   (125);

        require (
            later.elapsedSince (earlier) == adk::Duration (25),
            "elapsed duration");

        const adk::TimePoint beforeWrap (
            std::numeric_limits<adk::TimePoint::Raw>::max () - 4);
        const adk::TimePoint afterWrap (5);

        require (
            afterWrap.elapsedSince (beforeWrap) == adk::Duration (10),
            "elapsed duration across wrap");
    }

    void testResourceKindsAndBounds ()
    {
        struct Boundary
        {
            adk::ResourceKind kind;
            uint8_t           last;
            uint8_t           invalid;
        };

        const Boundary boundaries[] = {
            {adk::ResourceKind::Pin,        69, 70},
            {adk::ResourceKind::Timer,       5,  6},
            {adk::ResourceKind::Interrupt,   5,  6},
            {adk::ResourceKind::I2cBus,      0,  1},
            {adk::ResourceKind::SpiBus,      0,  1},
            {adk::ResourceKind::SerialPort,  3,  4}
        };

        for (const Boundary& boundary : boundaries)
        {
            adk::ResourceRegistry resources;
            adk::ResourceClaim    claim;

            const adk::ResourceId valid {
                boundary.kind,
                boundary.last
            };
            const adk::ResourceId invalid {
                boundary.kind,
                boundary.invalid
            };

            requireStatus     (
                resources.claim (valid, claim),
                adk::StatusCode::Ok,
                "valid boundary claim");
            require           (resources.claimed (valid), "valid boundary recorded");
            claim.release     ();
            require           (!resources.claimed (valid), "boundary release");
            requireStatus     (
                resources.claim (invalid, claim),
                adk::StatusCode::Unsupported,
                "invalid boundary rejected");
            require           (!resources.claimed (invalid), "invalid not recorded");
        }
    }

    void testConflictAndReuse ()
    {
        adk::ResourceRegistry resources;
        adk::ResourceClaim    owner;
        adk::ResourceClaim    contender;

        requireStatus (
            resources.claim (pin (12), owner),
            adk::StatusCode::Ok,
            "first owner claims pin");
        requireStatus (
            resources.claim (pin (12), contender),
            adk::StatusCode::ResourceBusy,
            "second owner rejected");
        require (!contender.active (), "rejected claim remains inert");

        owner.release ();
        requireStatus (
            resources.claim (pin (12), contender),
            adk::StatusCode::Ok,
            "released resource reusable");
    }

    void testClaimStateRules ()
    {
        adk::ResourceRegistry firstRegistry;
        adk::ResourceRegistry secondRegistry;
        adk::ResourceClaim    claim;

        requireStatus (
            firstRegistry.claim (pin (1), claim),
            adk::StatusCode::Ok,
            "claim starts active");
        requireStatus (
            firstRegistry.claim (pin (2), claim),
            adk::StatusCode::InvalidArgument,
            "active claim cannot be overwritten");
        requireStatus (
            secondRegistry.claim (pin (2), claim),
            adk::StatusCode::InvalidArgument,
            "active claim cannot change registry");
        require (firstRegistry.claimed (pin (1)), "original claim preserved");
        require (!firstRegistry.claimed (pin (2)), "replacement not claimed");
        require (!secondRegistry.claimed (pin (2)), "other registry unchanged");

        claim.release   ();
        claim.release   ();
        require         (!firstRegistry.claimed (pin (1)), "release is idempotent");
    }

    void testClaimLifetime ()
    {
        adk::ResourceRegistry resources;

        {
            adk::ResourceClaim claim;

            requireStatus (
                resources.claim (pin (9), claim),
                adk::StatusCode::Ok,
                "scoped claim");
            require (claim.active (), "scoped claim active");
        }

        require (!resources.claimed (pin (9)), "claim destructor releases");
    }

    void testRuntimeOwnership ()
    {
        adk::Runtime       runtime;
        adk::ResourceClaim claim;

        requireStatus (
            runtime.resources ().claim (pin (4), claim),
            adk::StatusCode::Ok,
            "runtime registry claim");
        require (
            runtime.resources ().claimed (pin (4)),
            "runtime retains registry state");
    }

    void testTransactionalRollback ()
    {
        adk::ResourceRegistry resources;
        adk::ResourceClaim    blocker;

        requireStatus (
            resources.claim (pin (8), blocker),
            adk::StatusCode::Ok,
            "rollback blocker");

        PairClaim pair (resources);

        requireStatus (
            pair.initialize (pin (7), pin (8)),
            adk::StatusCode::ResourceBusy,
            "second claim failure returned");
        require (!pair.initialized (), "failed pair remains inert");
        require (!resources.claimed (pin (7)), "first claim rolled back");
        require (resources.claimed (pin (8)), "foreign claim preserved");

        blocker.release  ();
        requireStatus    (
            pair.initialize (pin (7), pin (8)),
            adk::StatusCode::Ok,
            "pair retries after rollback");
        requireStatus    (
            pair.initialize (pin (7), pin (8)),
            adk::StatusCode::Ok,
            "initialize is idempotent");

        pair.shutdown   ();
        pair.shutdown   ();
        require         (!resources.claimed (pin (7)), "first pair claim released");
        require         (!resources.claimed (pin (8)), "second pair claim released");
    }

    void testSharedClaims ()
    {
        const adk::ResourceId timer = {adk::ResourceKind::Timer, 2};
        adk::ResourceRegistry resources;
        adk::SharedResourceClaim first;
        adk::SharedResourceClaim second;

        requireStatus (
            resources.claimShared (timer, first),
            adk::StatusCode::Ok,
            "first shared claim");
        requireStatus (
            resources.claimShared (timer, second),
            adk::StatusCode::Ok,
            "second shared claim");
        require (resources.claimed (timer), "shared resource claimed");

        first.release  ();
        require        (resources.claimed (timer), "one shared lease remains");
        second.release ();
        require        (!resources.claimed (timer), "final shared release");
    }

    void testSharedExclusiveConflicts ()
    {
        const adk::ResourceId timer = {adk::ResourceKind::Timer, 3};
        adk::ResourceRegistry resources;
        adk::SharedResourceClaim shared;
        adk::ResourceClaim       exclusive;

        requireStatus (
            resources.claimShared (timer, shared),
            adk::StatusCode::Ok,
            "shared before exclusive");
        requireStatus (
            resources.claim (timer, exclusive),
            adk::StatusCode::ResourceBusy,
            "shared blocks exclusive");
        shared.release  ();
        requireStatus   (
            resources.claim (timer, exclusive),
            adk::StatusCode::Ok,
            "exclusive after shared release");
        requireStatus (
            resources.claimShared (timer, shared),
            adk::StatusCode::ResourceBusy,
            "exclusive blocks shared");
        exclusive.release ();
        requireStatus     (
            resources.claimShared (timer, shared),
            adk::StatusCode::Ok,
            "shared after exclusive release");
    }

    void testSharedBoundsAndState ()
    {
        adk::ResourceRegistry resources;
        adk::ResourceRegistry otherResources;
        adk::SharedResourceClaim claim;

        requireStatus (
            resources.claimShared ({adk::ResourceKind::Timer, 5}, claim),
            adk::StatusCode::Ok,
            "last shared timer");
        requireStatus (
            resources.claimShared ({adk::ResourceKind::Timer, 4}, claim),
            adk::StatusCode::InvalidArgument,
            "active shared destination");
        requireStatus (
            otherResources.claimShared (
                {adk::ResourceKind::Timer, 4},
                claim),
            adk::StatusCode::InvalidArgument,
            "active shared destination other registry");
        claim.release ();
        requireStatus (
            resources.claimShared ({adk::ResourceKind::Timer, 6}, claim),
            adk::StatusCode::Unsupported,
            "shared timer out of range");
        requireStatus (
            resources.claimShared ({adk::ResourceKind::Pin, 0}, claim),
            adk::StatusCode::Unsupported,
            "only timers are shareable");
    }

    void testSharedLifetime ()
    {
        const adk::ResourceId timer = {adk::ResourceKind::Timer, 4};
        adk::ResourceRegistry resources;

        {
            adk::SharedResourceClaim claim;

            requireStatus (
                resources.claimShared (timer, claim),
                adk::StatusCode::Ok,
                "scoped shared claim");
        }

        require (!resources.claimed (timer), "shared destructor releases");
    }

    void testSharedCapacity ()
    {
        const adk::ResourceId timer = {adk::ResourceKind::Timer, 5};
        adk::ResourceRegistry resources;
        std::array<adk::SharedResourceClaim, 255> claims;
        adk::SharedResourceClaim overflow;

        for (adk::SharedResourceClaim& claim : claims)
        {
            requireStatus (
                resources.claimShared (timer, claim),
                adk::StatusCode::Ok,
                "fill shared capacity");
        }

        requireStatus (
            resources.claimShared (timer, overflow),
            adk::StatusCode::CapacityExceeded,
            "shared capacity overflow");
        require (!overflow.active (), "overflow remains inert");

        claims[0].release ();
        requireStatus     (
            resources.claimShared (timer, overflow),
            adk::StatusCode::Ok,
            "released shared capacity reusable");
    }

    void testSharedTransactionalRollback ()
    {
        const adk::ResourceId firstTimer  = {adk::ResourceKind::Timer, 0};
        const adk::ResourceId secondTimer = {adk::ResourceKind::Timer, 1};
        adk::ResourceRegistry resources;
        adk::ResourceClaim    blocker;
        adk::SharedResourceClaim first;
        adk::SharedResourceClaim second;

        requireStatus (
            resources.claim (secondTimer, blocker),
            adk::StatusCode::Ok,
            "shared transaction blocker");
        requireStatus (
            resources.claimShared (firstTimer, first),
            adk::StatusCode::Ok,
            "shared transaction first");

        const adk::Status status =
            resources.claimShared (secondTimer, second);

        if (!status.ok ())
        {
            first.release ();
        }

        requireStatus (
            status,
            adk::StatusCode::ResourceBusy,
            "shared transaction failure");
        require (!first.active (), "shared transaction rolled back");
        require (!resources.claimed (firstTimer), "first shared lease released");
        require (!second.active (), "failed second lease inert");
        require (resources.claimed (secondTimer), "foreign claim preserved");
    }

#if defined(__cpp_exceptions)
    void unwindClaim (adk::ResourceRegistry& resources)
    {
        PairClaim pair (resources);

        requireStatus (
            pair.initialize (pin (20), pin (21)),
            adk::StatusCode::Ok,
            "unwind pair initialized");
        throw 1;
    }

    void testExceptionUnwinding ()
    {
        adk::ResourceRegistry resources;

        try
        {
            unwindClaim (resources);
        }
        catch (...)
        {
        }

        require (!resources.claimed (pin (20)), "unwind releases first claim");
        require (!resources.claimed (pin (21)), "unwind releases second claim");
    }
#endif
}

int main ()
{
    static_assert (sizeof (adk::Status) == 1, "status remains one byte");
    static_assert (
        std::is_trivially_copyable<adk::Status>::value,
        "status is a value object");
    static_assert (
        !std::is_copy_constructible<adk::ResourceRegistry>::value,
        "registries own unique state");
    static_assert (
        !std::is_move_constructible<adk::ResourceRegistry>::value,
        "registry addresses remain stable");
    static_assert (
        !std::is_copy_constructible<adk::ResourceClaim>::value,
        "claims are unique");
    static_assert (
        !std::is_move_constructible<adk::ResourceClaim>::value,
        "claims cannot silently transfer");
    static_assert (
        std::is_nothrow_destructible<adk::ResourceClaim>::value,
        "claim destruction supports exception unwinding");
    static_assert (
        !std::is_copy_constructible<adk::SharedResourceClaim>::value,
        "shared claims are unique leases");
    static_assert (
        !std::is_move_constructible<adk::SharedResourceClaim>::value,
        "shared claims cannot silently transfer");
    static_assert (
        std::is_nothrow_destructible<adk::SharedResourceClaim>::value,
        "shared destruction supports exception unwinding");
    static_assert (
        std::is_nothrow_destructible<PairClaim>::value,
        "component destruction supports exception unwinding");

    testStatus                      ();
    testTime                        ();
    testResourceKindsAndBounds      ();
    testConflictAndReuse            ();
    testClaimStateRules             ();
    testClaimLifetime               ();
    testRuntimeOwnership            ();
    testTransactionalRollback       ();
    testSharedClaims                ();
    testSharedExclusiveConflicts    ();
    testSharedBoundsAndState        ();
    testSharedLifetime              ();
    testSharedCapacity              ();
    testSharedTransactionalRollback ();

#if defined(__cpp_exceptions)
    testExceptionUnwinding ();
#endif

    std::cout << "All ADK core tests passed.\n";
}
