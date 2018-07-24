# playa-lights

## Desired Features

### Arduino

* Toggle switch for Bluetooth (Might be redundant with just unplugging)
* On board switch for physical backup if bluetooth/app/phone fail. (Simple pattern cycle?)
* Patterns encoded as 'init_[name]' and 'loop_[name]'. Loop function guaranteed to return after one update of LEDs so that arduino can handle bluetooth commands.  
* Master list of function names with associated function pointers to use for switching modes
* Reset function to switch off all lights before switching modes 
* Code to restrict power draw to max permitted by power supply 

### App

* Power switch
* Live bluetooth status indicator and reconnect button
* Dimmer slider
* Power supply amperage selector
* Number of LED selector / Daisy chain selector
  - Allow designing LED hookup pattern in app. (for example, first 20 are jacket, next 49 are grid)
* Pattern Selector Dropdown
  - with dynamically updated patterns fetched from arduino at App start.
    * Maybe cache this.
  - allow input to patterns from phone
    * For example, picking color, speed, etc.
  - For grids, some sort of drawing interface
 
* Dynamic patterns which fetch input from phone sensors 
  - Accelerometers
  - Camera
  - Light sensor 

### Communication Protocol

* 1 byte control code (upper/lowercase letter) gives us 52 commands.
* Depending on control code, various formats for following data.
* ack code sent after all received transmissions, by both arduino and app.
* No sending of multiple commands without intervening acks. 

### LEDs

* 150 total LEDs, although possible to buy more
* Multiple setups, possibly one small grid (5x5 to 8x8 or so)
* Outputs on ends of strips to allow daisy chaining

