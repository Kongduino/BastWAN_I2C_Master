# BastWAN I2C Master

This is the first half of a two-part demonstration, where a [BastWAN](https://github.com/ElectronicCats/Bast-WAN) acts as the master and sends messages to an ESP32 slave, in this case an [Inkplate 6](https://github.com/e-radionicacom/Inkplate-6-Arduino-library).

It uses on the ESP32 the [ESP32 I2C Slave library](https://github.com/gutierrezps/ESP32_I2C_Slave). This code rips the WirePacker part of this library for the BastWAN code to work properly.

The second half of the demonstration is [here](https://github.com/Kongduino/Inkplate_I2C_Slave).

