# Motion

!!! warning "Motors need their own power"
    A Mega pin gives about 20 mA; a servo or motor wants hundreds. Power
    them from the breadboard power module, and join its GND to the Mega's.

Asking a part for what it is already doing changes nothing, so
`moveTo ()` and `speed ()` can be called from every pass of `loop ()`;
asking for something different starts afresh.

<!-- api servo.h Servo -->

<!-- api stepper.h Stepper -->

<!-- api motor.h Motor -->
