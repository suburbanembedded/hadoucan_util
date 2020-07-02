# hadoucan_util library

## What is it for?
This library was made to support the HadouCAN USB to CAN FD adapter and its interaction with Linux without the use of slcan. The files described in this README allow for reading and writing to the HadouCAN over serial port, support for Linux SocketCAN conversions and I/O, and the generation of random CAN frames.

Documentation for Linux SocketCAN described in this README can be found [here](https://www.kernel.org/doc/Documentation/networking/can.txt).

### lawicel_linux_conversions
Conversion between an ASCII text string used by the HadouCAN and a canfd_frame used by SocketCAN.
Specifications for the format of the ASCII text string for the HadouCAN can be found [here](https://suburbanmarine.io/public/hadoucan/doc/Hadou-CAN_User_Guide.pdf).

### lin_can_helper
A wrapper class for the SocketCAN can_frame that provides an object called __Linux_can__ which contains the canfd_frame itself, its size and a time variable that can be utilized to store the objects read or write time.
This class also provides methods that return and set the object's attributes.
### Hadoucan_IO_mgr
Provides read and write support for the HadouCAN over serial port file I/O.
### Socketcan_IO_mgr
Provides read and write support for the SocketCAN interface. 
### Gen_can::gen_rand_can
Generation of randomized CAN frames in an ASCII text string format.
Currently generates randomized frames that may be of standard or extended length ID's and may be FD or non FD. In addition, depending on its frame type, it will also generate with a randomized 11 or 29-bit ASCII text represented Hexadecimal ID and  randomized data of either 0-8 or 0-64 bytes.

## Requirements
- C++ 11 or newer
- can-utils library
- Boost ASIO library 
