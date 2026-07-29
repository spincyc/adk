    const adk::PulseCaptureConfig maximumCaptureConfig = {
        adk::MicrosecondDuration (12000),
        adk::MicrosecondDuration (100),
        adk::MicrosecondDuration (10000)};
    adk::Runtime            maximumRuntime;
    adk::MegaPulseCaptureIo maximumCaptureIo;
    adk::PulseCapture       maximumCapture (
        maximumRuntime.resources (), maximumCaptureIo, 2,
        maximumCaptureConfig);
