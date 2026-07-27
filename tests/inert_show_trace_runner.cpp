#include <stdio.h>
#include <string.h>

#include "inert_show_simulator.h"

namespace {

    adk::InertCueSchedulerConfig schedulerConfig () noexcept
    {
        adk::InertCueSchedulerConfig result;

        result.plan.cues[0] = {3, adk::Duration (0), adk::Duration (10)};
        result.plan.cues[1] = {7, adk::Duration (20), adk::Duration (10)};
        result.plan.count   = 2;
        result.confirmationWindow = adk::Duration (5);
        return result;
    }

    adk::InertCueChannelMap channelMap () noexcept
    {
        adk::InertCueChannelMap result = {};

        result.channels[0] = 5;
        result.channels[1] = 2;
        result.count       = 2;
        return result;
    }

    bool observationPair (char token, adk::InertObservation& primary,
                          adk::InertObservation& redundant) noexcept
    {
        switch (token)
        {
            case 'O':
                primary = redundant = adk::InertObservation::Open;
                return true;
            case 'C':
                primary = redundant = adk::InertObservation::Closed;
                return true;
            case 'S':
                primary = redundant = adk::InertObservation::ShortSimulated;
                return true;
            case 'U':
                primary = redundant = adk::InertObservation::Unavailable;
                return true;
            case 'X':
                primary   = adk::InertObservation::Closed;
                redundant = adk::InertObservation::Open;
                return true;
        }

        return false;
    }

    bool printAudit (const adk::CueAuditBuffer& audit) noexcept
    {
        adk::CueAuditEncoder encoder;
        char                 line[adk::CueAuditEncoder::maximumLength];

        for (uint8_t index = 0; index < audit.count (); ++index)
        {
            const adk::Result<adk::CueAuditEntry> entry = audit.entry (index);

            if (!entry.ok ())
            {
                return false;
            }

            const adk::Result<uint8_t> length =
                encoder.encode (entry.value (), line, sizeof line);

            if (!length.ok () ||
                fwrite (line, 1, length.value (), stdout) != length.value ())
            {
                return false;
            }
        }

        return true;
    }
} // namespace

int main (int argumentCount, char** arguments)
{
    if (argumentCount < 2 || argumentCount > 3)
    {
        fputs ("usage: inert_show_trace_runner TRACE [--audit]\n", stderr);
        return 2;
    }

    const bool auditOnly = argumentCount == 3 &&
                           strcmp (arguments[2], "--audit") == 0;

    if (argumentCount == 3 && !auditOnly)
    {
        fputs ("unknown mode\n", stderr);
        return 2;
    }

    FILE* trace = fopen (arguments[1], "r");

    if (trace == nullptr)
    {
        perror ("trace");
        return 2;
    }

    char header[64];

    if (fgets (header, sizeof header, trace) == nullptr ||
        strcmp (header, "adk-inert-show-trace,1\n") != 0)
    {
        fputs ("invalid trace header\n", stderr);

        fclose (trace);
        return 2;
    }

    adk::InertChannelAssessor assessor (adk::Duration (50));
    adk::CueAuditEntry        auditStorage[64];
    adk::CueAuditBuffer       audit (auditStorage, 64);

    adk::InertCueScheduler    scheduler (schedulerConfig (), audit);

    adk::InertShowSimulator   simulator (
        channelMap (), assessor, scheduler, audit);

    if (!simulator.initialize ().ok ())
    {
        fputs ("simulator initialization failed\n", stderr);

        fclose (trace);
        return 1;
    }

    char line[160];
    bool hasRows = false;

    while (fgets (line, sizeof line, trace) != nullptr)
    {
        if (line[0] == '#')
        {
            continue;
        }

        unsigned long ticks = 0;
        char          observationToken = '\0';
        unsigned int  review = 0;
        unsigned int  run = 0;
        unsigned int  confirm = 0;
        unsigned int  skip = 0;
        unsigned int  cancel = 0;
        int           parsedLength = 0;

        if (sscanf (line, "%lu,%c,%u,%u,%u,%u,%u%n", &ticks,
                    &observationToken, &review, &run, &confirm, &skip,
                    &cancel, &parsedLength) != 7 ||
            (line[parsedLength] != '\n' && line[parsedLength] != '\r' &&
             line[parsedLength] != '\0') ||
            (line[parsedLength] == '\r' &&
             (line[parsedLength + 1] != '\n' ||
              line[parsedLength + 2] != '\0')) ||
            (line[parsedLength] == '\n' && line[parsedLength + 1] != '\0') ||
            ticks > 0xfffffffful || review > 1 || run > 1 || confirm > 1 ||
            skip > 1 || cancel > 1)
        {
            fputs ("invalid trace row\n", stderr);

            fclose (trace);
            return 2;
        }

        hasRows = true;

        adk::InertObservation primary;
        adk::InertObservation redundant;

        if (!observationPair (observationToken, primary, redundant))
        {
            fputs ("invalid observation token\n", stderr);

            fclose (trace);
            return 2;
        }

        const adk::TimePoint now (static_cast<uint32_t> (ticks));
        adk::InertChannelObservation
            observations[adk::InertChannelAssessor::capacity];

        for (uint8_t channel = 0; channel < adk::InertChannelAssessor::capacity;
             ++channel)
        {
            observations[channel] = {channel, primary, redundant, now};
        }

        const adk::CueOperatorInput operatorInput =
            {review != 0, run != 0, confirm != 0, skip != 0, cancel != 0};
        const adk::InertShowInput input =
            {observations, adk::InertChannelAssessor::capacity, operatorInput};
        const adk::Status status = simulator.update (now, input);

        if (!status.ok ())
        {
            fprintf (stderr, "trace update failed: %s\n", adk::statusName (status));

            fclose (trace);
            return 1;
        }

        if (!auditOnly)
        {
            const adk::InertShowSnapshot snapshot = simulator.snapshot ();

            printf ("adk-show,1,%lu,%u,%u,%u,%u,%u,%u,%u,%lu,%lu,%s\n",
                    ticks, static_cast<unsigned int> (snapshot.state),
                    static_cast<unsigned int> (snapshot.fault),
                    static_cast<unsigned int> (snapshot.schedule.phase),
                    static_cast<unsigned int> (snapshot.schedule.decision),
                    static_cast<unsigned int> (snapshot.schedule.hasCue),
                    static_cast<unsigned int> (snapshot.schedule.cue),
                    static_cast<unsigned int> (snapshot.schedule.cueIndex),
                    static_cast<unsigned long> (snapshot.auditSequence),
                    static_cast<unsigned long> (snapshot.traceDigest),
                    adk::statusName (snapshot.status));
        }
    }

    if (ferror (trace) != 0)
    {
        fputs ("trace read failed\n", stderr);

        fclose (trace);
        return 2;
    }

    if (!hasRows)
    {
        fputs ("trace has no rows\n", stderr);

        fclose (trace);
        return 2;
    }

    fclose (trace);

    simulator.shutdown ();

    if (auditOnly && !printAudit (audit))
    {
        fputs ("audit encoding failed\n", stderr);
        return 1;
    }

    return 0;
}
