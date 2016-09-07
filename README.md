# Dallas DS18B20 Temperature Sensor API
This repository contains a C API for the Dallas DS18B20 temperature sensor
for Raspberry Pi. Sample code is also provided to show how to
use the API.

<h3>Data Types</h3>
Two data types are defined in the library to facilitate its use:
<ul>
  <li><b>ds_port_t</b> is an enumerated type with symbols
      DS_PORT_A, DS_PORT_B, DS_PORT_C, DS_PORT_D to identify which I/O
      port should be used for the DS18B20 device.
  <li><b>ds18b20_t</b> is a structure to store the device configuration</b>
</ul>


<h3>Functions</h3>
<b>ds18b20_init</b> must be called first. It initialises the ds18b20_t structure,
sends a reset pulse on the 1-wire bus and listens for a response.<br>
<b>ds18b20_read_temperature</b> is used to initiate a temperature reading from
the sensor. The value returned is multiples of 1/16th degrees C. This method avoids
any precision errors in floating point libraries and is also more portable to
microcontrollers. The function will also filter out any precision bits which are
higher than the requested precision. This situation can arise when a reading is
made with high precision and contains high precision bits, and subsequent readings
are requested with lower precision. The nature of the A/D conversion means that
the high precision bits are left as they are.


