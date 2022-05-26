# MSP430_LoRa
This program is takes a temperature reading using one-wire commmunication, 
sends the reading to the LoRa board using SPI which transmits to the reciever, 
then goes into low power mode for 10 minutes.

Pin Outputs:<br />
p1.4 -> Temp sensor data line<br />
p5.2 -> Temp sensor power

p5.4 and p4.5 -> LoRa power in series
p1.5 (SCK)      -> LoRa SCK (ICSP pins)<br />
p1.6 (MISO/SOMI) -> LoRa MISO/SOMI (ICSP pins)<br />
p1.7 (MOSI/SIMO) -> LoRa MOSI/SIMO (ICSP pins)<br />
p5.1  -> LoRa D10 (chip select)<br />


Vcc = 3.3V

ICSP Pins:<br />
[]MISO | Vcc<br />
SCK | MOSI<br />
Reset | GND<br />
