First byte of any message is message length.

## Arduino Receiver

Name: Power
Syntax: P0 or P1
Effects: P0 turns off strip, P1 turns on strip

Name: Set pattern
Syntax: S followed by a pattern index (in hex)

Name: Get power
Syntax: GP
Returns: 0 if power is off, 1 if power is on.
