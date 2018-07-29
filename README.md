# playa-lights

## Desired Features

### Arduino

* Toggle switch for Bluetooth (Might be redundant with just unplugging)
* On board switch for physical backup if bluetooth/app/phone fail. (Simple pattern cycle?)

### App

* LED mode switch (Bike vs. Backpack)
* Pattern Selector Dropdown
  - allow input to patterns from phone
    * For example, picking color, speed, etc.
  - For grids, some sort of drawing interface
 
* Dynamic patterns which fetch input from phone sensors 
  - Accelerometers
  - Camera
  - Light sensor 

### Communication Protocol

* 1 letter control code (upper/lowercase letter) gives us 52 commands.
  - Easily extensible if more commands needed
* Depending on control code, various formats for following data.
* Commands are length encoded

### LEDs

* 150 total LEDs, although possible to buy more
* Setup 1: 24 LEDs on backpack in arch with 3 strips of 5 in fan shape on back
* Other possible setups
  - Small grid (5x5 to 8x8 or so)
  - Wrapped around bike frame

