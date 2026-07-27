#pragma once

#include "motor_intent.h"
#include "status.h"
#include "time.h"
#include "ultrasonic_ranger.h"

#include <stdint.h>

namespace adk {

    enum struct RoverMode : uint8_t
    {
        Inactive,
        Ready,
        Driving,
        Turning,
        Paused,
        ObstacleHold,
        MotionFault,
        SensorFault,
        RouteComplete,
        Stopped
    };

    enum struct RoverStopReason : uint8_t
    {
        None,
        StopRequested,
        RangeInvalid,
        RangeStale,
        EncoderFault,
        MotionDidNotStart,
        WheelMismatch,
        RouteTimeout,
        UnexpectedMotion,
        InvalidTime
    };

    enum struct RouteAction : uint8_t
    {
        Drive,
        TurnLeft,
        TurnRight,
        Pause,
        Finish
    };

    struct RouteStep
    {
        RouteStep (RouteAction action          = RouteAction::Finish,
                   uint16_t    targetEdges     = 0,
                   Duration    maximumDuration = Duration (0),
                   uint8_t     duty            = 0) noexcept;

        RouteAction action;
        uint16_t    targetEdges;
        Duration    maximumDuration;
        uint8_t     duty;
    };

    struct RoverControllerConfig
    {
        uint16_t minimumClearanceMm         = 200;
        uint16_t resumeClearanceMm          = 250;
        Duration rangeMaximumAge            = Duration (250);
        Duration motionStartTimeout         = Duration (500);
        Duration wheelMismatchTimeout       = Duration (500);
        uint16_t maximumWheelEdgeDifference = 4;
        uint8_t  maximumMotorDuty           = 180;
    };

    struct RoverWheelObservation
    {
        RoverWheelObservation (uint32_t totalEdges = 0,
                               Status   status     = Status ()) noexcept;

        uint32_t totalEdges;
        Status   status;
    };

    struct RoverInput
    {
        RoverInput (const RangeReading&          range,
                    TimePoint                    rangeObservedAt,
                    const RoverWheelObservation& leftWheel,
                    const RoverWheelObservation& rightWheel,
                    bool                         startEvent,
                    bool                         stopRequested,
                    TimePoint                    observedAt) noexcept;

        RangeReading          range;
        TimePoint             rangeObservedAt;
        RoverWheelObservation leftWheel;
        RoverWheelObservation rightWheel;
        bool                  startEvent;
        bool                  stopRequested;
        TimePoint             observedAt;
    };

    struct RoverControllerSnapshot
    {
        MotorCommand          leftMotor;
        MotorCommand          rightMotor;
        RoverMode             mode;
        RoverStopReason       stopReason;
        Status                status;
        RangeReading          range;
        TimePoint             rangeObservedAt;
        RoverWheelObservation leftWheel;
        RoverWheelObservation rightWheel;
        TimePoint             modeSince;
        TimePoint             nextDeadline;
        uint32_t              leftStepEdges;
        uint32_t              rightStepEdges;
        uint8_t               routeIndex;
        bool                  hasDeadline;
        bool                  routeAdvanced;
        bool                  stopDominated;
        uint32_t              transitionCount;
    };

    struct RoverController
    {
        static constexpr uint8_t routeCapacity = 16;

        RoverController  (const RoverControllerConfig& config,
                          const RouteStep*              route,
                          uint8_t                       routeLength) noexcept;
        ~RoverController () noexcept;

        Status initialize () noexcept;
        void   shutdown   () noexcept;
        Status update     (const RoverInput& input) noexcept;

        bool                    initialized () const noexcept;
        RoverControllerSnapshot snapshot    () const noexcept;

      private:
        bool configValid         () const noexcept;
        bool routeValid          () const noexcept;
        bool timeValid           (TimePoint now) const noexcept;
        bool rangeFresh          (const RoverInput& input) const noexcept;
        bool rangeUsable         (const RoverInput& input) const noexcept;
        bool wheelsStationary    (const RoverInput& input) const noexcept;
        bool wheelsProgressValid (const RoverInput& input) const noexcept;
        bool stepTimedOut        (TimePoint now) const noexcept;
        bool motionTimedOut      (TimePoint now) const noexcept;
        bool stepComplete        () const noexcept;
        bool wheelMismatch       () const noexcept;
        void resetRoute          (const RoverInput& input) noexcept;
        void beginStep           (TimePoint now) noexcept;
        void advanceRoute        (TimePoint now) noexcept;
        void commandForStep      () noexcept;
        void stopWith            (RoverMode       mode,
                                  RoverStopReason reason,
                                  Status          status,
                                  TimePoint       now,
                                  bool            dominated) noexcept;
        void enterMode           (RoverMode mode, TimePoint now) noexcept;
        void commandStop         () noexcept;
        void observe             (const RoverInput& input) noexcept;

        RoverControllerConfig config_;
        RouteStep             route_[routeCapacity];
        uint8_t               routeLength_;
        MotorCommand          leftMotor_;
        MotorCommand          rightMotor_;
        RoverMode             mode_;
        RoverStopReason       stopReason_;
        Status                status_;
        RangeReading          range_;
        TimePoint             rangeObservedAt_;
        RoverWheelObservation leftWheel_;
        RoverWheelObservation rightWheel_;
        TimePoint             modeSince_;
        TimePoint             lastUpdate_;
        TimePoint             mismatchSince_;
        uint32_t              leftStepStart_;
        uint32_t              rightStepStart_;
        uint32_t              leftStepEdges_;
        uint32_t              rightStepEdges_;
        uint32_t              transitionCount_;
        uint8_t               routeIndex_;
        bool                  initialized_;
        bool                  hasLastUpdate_;
        bool                  mismatchActive_;
        bool                  routeAdvanced_;
        bool                  stopDominated_;
    };
}
