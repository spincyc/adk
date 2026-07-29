#include <multiplexed_digit_policy.h>

#include <cstdlib>
#include <iostream>
#include <type_traits>
// clang-format off

namespace adk {
    struct MultiplexedDigitPolicyTestAccess
    {
        static void setLifecycleGeneration (MultiplexedDigitPolicy& policy,
                                            uint32_t                generation)
        {
            policy.lifecycleGeneration_ = generation;
        }
    };
} // namespace adk

namespace {
    void require (bool condition, const char* message)
    {
        if (!condition)
        {
            std::cerr << "FAIL: " << message << '\n';
            std::exit (EXIT_FAILURE);
        }
    }

    adk::MultiplexedDigitConfig config (
        adk::SevenSegmentPolarity segment = adk::SevenSegmentPolarity::CommonCathode,
        adk::DigitSelectPolarity  select  = adk::DigitSelectPolarity::ActiveHigh)
    {
        return {17, 3, segment, select, adk::Duration (1), adk::Duration (4)};
    }

    void commit (adk::MultiplexedDigitPolicy& policy, uint32_t value, bool leadingZeros,
                 uint8_t decimals, uint32_t source)
    {
        adk::MultiplexedDigitPreview preview;
        require (policy.preview (value, leadingZeros, decimals, source, preview).ok (),
                 "preview succeeds");
        require (policy.canCommit (preview), "preview can commit");
        require (policy.commit (preview).ok (), "commit succeeds");
    }

    void testLifecycleAndFormatting ()
    {
        static_assert (!std::is_copy_constructible<adk::MultiplexedDigitPolicy>::value,
                       "policy is not copy constructible");
        static_assert (!std::is_move_constructible<adk::MultiplexedDigitPolicy>::value,
                       "policy is not move constructible");

        adk::MultiplexedDigitPolicy policy (config ());
        require                            (!policy.initialized (), "construction is inert");
        require                            (policy.refresh (adk::TimePoint ()).error () ==
                     adk::StatusCode::NotInitialized,
                 "refresh before initialization rejects");
        require (policy.initialize (adk::TimePoint (10)).ok (), "initialize succeeds");
        require (policy.initialize (adk::TimePoint (99)).ok (),
                 "initialize is idempotent");

        commit                            (policy, 42, false, 0x09U, 8);
        const auto first = policy.refresh (adk::TimePoint (10));
        require                           (first.ok () && first.value ().emitted,
                 "same-time first refresh is explicit");
        require (first.value ().digitIndex == 0 && first.value ().frameGeneration == 1,
                 "first refresh swaps complete frame at digit zero");
        const auto frame = policy.snapshot ().activeFrame;
        require                            (frame.glyphs[0] == adk::SevenSegmentGlyph::Blank &&
                     frame.glyphs[1] == adk::SevenSegmentGlyph::Blank &&
                     frame.glyphs[2] == adk::SevenSegmentGlyph::Four &&
                     frame.glyphs[3] == adk::SevenSegmentGlyph::Two,
                 "leading zeros blank while value remains");
        require (frame.decimalMask == 0x09U && first.value ().segmentLevels[2] == 0x80U,
                 "decimal on suppressed cell remains point-only");

        policy.reset (adk::TimePoint (20));
        commit       (policy, 10000, false, 0x0fU, 9);
        require      (policy.refresh (adk::TimePoint (20)).ok (),
                 "overflow frame publishes");
        const auto overflow = policy.snapshot ().activeFrame;
        require                               (overflow.overflow && overflow.decimalMask == 0,
                 "overflow clears decimals");
        for (const auto glyph : overflow.glyphs)
        {
            require (glyph == adk::SevenSegmentGlyph::Dash,
                     "overflow renders four dashes");
        }

        policy.shutdown ();
        policy.shutdown ();
        require         (!policy.initialized (), "shutdown is idempotent");

        const adk::MultiplexedDigitConfig invalid[] = {
            {0, 1, adk::SevenSegmentPolarity::CommonCathode,
             adk::DigitSelectPolarity::ActiveHigh},
            {1, 0, adk::SevenSegmentPolarity::CommonCathode,
             adk::DigitSelectPolarity::ActiveHigh},
            {1, 1, static_cast<adk::SevenSegmentPolarity> (2),
             adk::DigitSelectPolarity::ActiveHigh},
            {1, 1, adk::SevenSegmentPolarity::CommonCathode,
             static_cast<adk::DigitSelectPolarity> (2)},
            {1, 1, adk::SevenSegmentPolarity::CommonCathode,
             adk::DigitSelectPolarity::ActiveHigh, adk::Duration (0),
             adk::Duration                                       (4)},
            {1, 1, adk::SevenSegmentPolarity::CommonCathode,
             adk::DigitSelectPolarity::ActiveHigh, adk::Duration (5),
             adk::Duration                                       (4)}};
        for (const auto& invalidConfig : invalid)
        {
            adk::MultiplexedDigitPolicy rejected (invalidConfig);
            require                              (rejected.initialize (adk::TimePoint ()).error () ==
                         adk::StatusCode::InvalidConfiguration,
                     "invalid configuration remains inert");
        }
    }

    void testRefreshOrderingAndTiming ()
    {
        adk::MultiplexedDigitPolicy policy (config ());
        require                            (policy.initialize (adk::TimePoint (0xfffffffeUL)).ok (),
                 "wrap fixture initializes");
        commit (policy, 1234, true, 0x01U, 11);

        const auto zero = policy.refresh (adk::TimePoint (0xfffffffeUL)).value ();
        require                          (zero.stages[0] == adk::MultiplexedDigitStage::BlankSelects &&
                     zero.stages[1] == adk::MultiplexedDigitStage::LoadSegments &&
                     zero.stages[2] == adk::MultiplexedDigitStage::SelectDigit,
                 "transaction preserves ghost-prevention ordering");
        require (zero.digitSelectLevels[0] == 0 && zero.digitSelectLevels[1] == 0 &&
                     zero.digitSelectLevels[2] == 1,
                 "only chosen digit becomes active");
        require (!policy.refresh (adk::TimePoint (0xfffffffeUL)).value ().emitted,
                 "repeated timestamp is idempotent");
        require (policy.refresh (adk::TimePoint (0xffffffffUL)).value ().digitIndex ==
                     1,
                 "next due call advances one digit");
        require (policy.refresh (adk::TimePoint (0)).value ().digitIndex == 2,
                 "wrap remains ordered");

        const auto atGap = policy.refresh (adk::TimePoint (4));
        require                           (atGap.ok () && atGap.value ().emitted,
                 "maximum accepted gap is inclusive");
        const auto late = policy.refresh (adk::TimePoint (9));
        require                          (late.error () == adk::StatusCode::Timeout &&
                     late.value ().fault == adk::MultiplexedDigitFault::RefreshLost &&
                     late.value ().emitted,
                 "one tick late latches blank refresh loss");
        require (policy.refresh (adk::TimePoint (10)).error () ==
                     adk::StatusCode::Timeout,
                 "refresh loss remains latched");
        require (policy.snapshot ().activeFrame.glyphs[0] ==
                     adk::SevenSegmentGlyph::Blank,
                 "refresh loss snapshot exposes blank intent");
        policy.reset (adk::TimePoint (10));
        require      (policy.refresh (adk::TimePoint (10)).ok (),
                 "explicit reset recovers refresh loss");

        const auto beforeRegression = policy.snapshot ();
        require                                       (policy.refresh (adk::TimePoint (9)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "backward time rejects");
        const auto afterRegression = policy.snapshot ();
        require                                      (afterRegression.lifecycleGeneration ==
                         beforeRegression.lifecycleGeneration &&
                     afterRegression.nextDigitIndex ==
                         beforeRegression.nextDigitIndex &&
                     afterRegression.fault == beforeRegression.fault,
                 "backward time rejection is atomic");

        adk::MultiplexedDigitPolicy exactHalf (config ());
        require                               (exactHalf.initialize (adk::TimePoint (7)).ok (),
                 "half-range fixture initializes");
        const auto halfBefore = exactHalf.snapshot ();
        require                                    (exactHalf.refresh (adk::TimePoint (0x80000007UL)).error () ==
                     adk::StatusCode::InvalidArgument,
                 "exact half-range time rejects");
        require (exactHalf.snapshot ().nextDigitIndex == halfBefore.nextDigitIndex &&
                     exactHalf.snapshot ().fault == halfBefore.fault,
                 "half-range rejection is atomic");

        adk::MultiplexedDigitPolicy firstAtDeadline (config ());
        require                                     (firstAtDeadline.initialize (adk::TimePoint (20)).ok (),
                 "first deadline fixture initializes");
        require (firstAtDeadline.refresh (adk::TimePoint (24)).ok () &&
                     firstAtDeadline.snapshot ().nextDigitIndex == 1,
                 "late first service at maximum gap remains valid");

        adk::MultiplexedDigitPolicy firstTooLate (config ());
        require                                  (firstTooLate.initialize (adk::TimePoint (20)).ok (),
                 "late first fixture initializes");
        const auto firstLate = firstTooLate.refresh (adk::TimePoint (25));
        require                                     (firstLate.error () == adk::StatusCode::Timeout &&
                     firstLate.value ().emitted &&
                     firstLate.value ().fault ==
                         adk::MultiplexedDigitFault::RefreshLost,
                 "late first service beyond maximum gap latches loss");
    }

    void testCycleBoundaryAndPolarity ()
    {
        adk::MultiplexedDigitPolicy policy (
            config (adk::SevenSegmentPolarity::CommonAnode,
                    adk::DigitSelectPolarity::ActiveLow));
        require (policy.initialize (adk::TimePoint ()).ok (), "polarity initializes");
        commit  (policy, 1111, true, 0, 1);
        require (policy.refresh (adk::TimePoint ()).ok (), "first frame starts");
        require (policy.refresh (adk::TimePoint (1)).ok (), "digit one");
        require (policy.refresh (adk::TimePoint (2)).ok (), "digit two");
        commit  (policy, 2222, true, 0, 2);
        require (policy.refresh (adk::TimePoint (3)).value ().frameGeneration == 1,
                 "pending frame cannot tear digit three");
        const auto swapped = policy.refresh (adk::TimePoint (4)).value ();
        require                             (swapped.frameGeneration == 2 && swapped.digitIndex == 0,
                 "pending frame swaps at digit zero");
        require (swapped.digitSelectLevels[0] == 0x0fU &&
                     swapped.digitSelectLevels[2] == 0x0eU,
                 "active-low selects retain exactly one active digit");
        require (
            swapped.segmentLevels[2] ==
                adk::encodeSevenSegmentGlyph (adk::SevenSegmentGlyph::Two,
                                              adk::SevenSegmentPolarity::CommonAnode),
            "segment polarity uses canonical encoder");

        adk::MultiplexedDigitPreview stale;
        require (policy.preview (3, false, 0, 3, stale).ok (),
                 "preview before reset succeeds");
        policy.reset (adk::TimePoint (5));
        require      (!policy.canCommit (stale) &&
                     policy.commit (stale).error () == adk::StatusCode::InvalidArgument,
                 "reset invalidates previews");
    }

    void testAllPolarityPairsAndDecimalSemantics ()
    {
        const adk::SevenSegmentPolarity segmentPolarities[] = {
            adk::SevenSegmentPolarity::CommonCathode,
            adk::SevenSegmentPolarity::CommonAnode};
        const adk::DigitSelectPolarity digitPolarities[] = {
            adk::DigitSelectPolarity::ActiveHigh, adk::DigitSelectPolarity::ActiveLow};

        for (const auto segmentPolarity : segmentPolarities)
        {
            for (const auto digitPolarity : digitPolarities)
            {
                adk::MultiplexedDigitPolicy policy (
                    config (segmentPolarity, digitPolarity));
                require (policy.initialize (adk::TimePoint ()).ok (),
                         "polarity pair initializes");
                commit (policy, 7, false, 0x0fU, 1);
                for (uint8_t digit = 0; digit < 4; ++digit)
                {
                    const auto transaction =
                        policy.refresh (adk::TimePoint (digit)).value ();
                    const auto glyph = digit == 3 ? adk::SevenSegmentGlyph::Seven
                                                  : adk::SevenSegmentGlyph::Blank;
                    require (
                        transaction.segmentLevels[2] ==
                            adk::encodeSevenSegmentGlyph (glyph, segmentPolarity, true),
                        "each decimal bit follows its left-to-right cell");
                    const uint8_t activeBit = static_cast<uint8_t> (1U << digit);
                    const uint8_t expectedSelect =
                        digitPolarity == adk::DigitSelectPolarity::ActiveHigh
                            ? activeBit
                            : static_cast<uint8_t> (0x0fU &
                                                    static_cast<uint8_t> (~activeBit));
                    require (transaction.digitSelectLevels[2] == expectedSelect,
                             "each polarity pair selects exactly one digit");
                }
            }
        }

        const uint32_t values[] = {0, 1, 9, 10, 99, 100, 999, 1000, 9999};
        for (const auto value : values)
        {
            adk::MultiplexedDigitPolicy policy (config ());
            require                            (policy.initialize (adk::TimePoint ()).ok (),
                     "format fixture initializes");
            commit  (policy, value, false, 0, value + 1U);
            require (policy.refresh (adk::TimePoint ()).ok (),
                     "bounded value frame publishes");
            const auto frame = policy.snapshot ().activeFrame;
            require                            (!frame.overflow, "bounded value never reports overflow");
            require                            (frame.glyphs[3] ==
                         static_cast<adk::SevenSegmentGlyph> (value % 10U),
                     "ones digit remains visible");
            if (value < 10U)
            {
                require (frame.glyphs[0] == adk::SevenSegmentGlyph::Blank &&
                             frame.glyphs[1] == adk::SevenSegmentGlyph::Blank &&
                             frame.glyphs[2] == adk::SevenSegmentGlyph::Blank,
                         "single digit suppresses every leading zero");
            }
        }
    }

    void testLifecycleExhaustionIsTerminal ()
    {
        adk::MultiplexedDigitPolicy policy (config ());
        require                            (policy.initialize (adk::TimePoint ()).ok (),
                 "lifecycle fixture initializes");
        adk::MultiplexedDigitPolicyTestAccess::setLifecycleGeneration (policy,
                                                                       0xffffffffUL);

        policy.reset (adk::TimePoint (1));
        require      (policy.snapshot ().fault ==
                         adk::MultiplexedDigitFault::LifecycleExhausted &&
                     policy.snapshot ().status.error () ==
                         adk::StatusCode::CapacityExceeded,
                 "generation wrap faults before reuse");

        policy.reset (adk::TimePoint (2));
        require      (policy.snapshot ().fault ==
                         adk::MultiplexedDigitFault::LifecycleExhausted &&
                     policy.snapshot ().lifecycleGeneration == 0,
                 "reset cannot recover lifecycle exhaustion");

        policy.shutdown ();
        require         (!policy.initialized () &&
                     policy.snapshot ().fault ==
                         adk::MultiplexedDigitFault::LifecycleExhausted &&
                     policy.snapshot ().status.error () ==
                         adk::StatusCode::CapacityExceeded,
                 "shutdown preserves terminal exhaustion");
        require (policy.initialize (adk::TimePoint (3)).error () ==
                         adk::StatusCode::CapacityExceeded &&
                     !policy.initialized (),
                 "initialize cannot reuse an exhausted lifecycle");
    }
} // namespace

int main ()
{
    testLifecycleAndFormatting              ();
    testRefreshOrderingAndTiming            ();
    testCycleBoundaryAndPolarity            ();
    testAllPolarityPairsAndDecimalSemantics ();
    testLifecycleExhaustionIsTerminal       ();
    std::cout << "multiplexed digit policy tests passed\n";
    return EXIT_SUCCESS;
}
