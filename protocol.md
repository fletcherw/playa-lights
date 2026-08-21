First byte of any message is message length.

## Arduino Receiver

Name: Set brightness
Syntax: B followed by a 1-byte brightness level

Name: Set color
Syntax: C followed by 3-byte RGB color value

Name: Get state
Syntax: G
Returns: Various state values

Name: Set mode
Syntax: M0 or M1
Effects: MG sets to bag mode, MK sets to bike mode

Name: Power
Syntax: P0 or P1
Effects: P0 turns off strip, P1 turns on strip

Name: Set pattern
Syntax: S followed by a pattern index (in hex)

Name: List patterns
Syntax: L
Returns: pattern count (1 byte), followed by, for each pattern in index order: name length (1 byte) + name bytes (ASCII, not null-terminated) + supports-color flag ('0' or '1')
