# rvswdog

STM8-based programmer for CH32V microcontrolles

Tested on STM8S103F3.

#### Usage

```
# make
# make flash
# minicom -b 115200 -D /dev/ttyXXX
```

##### Pinout

| PIN      |                                       |
|----------|---------------------------------------|
| UART1_TX |                                       |
| UART1_RX |                                       |
| PC3      | Target SWDIO + pullup resistor (4.7k) |
| PC4      | Target SWDCLK                         |

Connect to STM8S MCU via UART with baudrate 115200.

#### Commands

**s - scan - Scan microcontroller and connect**

No arguments.
````
>s
````

**r - read - Read memory, 4 bytes word**
````
>r <address 4 bytes in HEX>
````

**w - write - Write memory, 4 bytes word**
````
>w <address 4 bytes in HEX> <value 4 bytes in HEX>
````

#### TODO

- CH32V00x single wire protocol
- Reset target
- Fast binary interface for rvswdog-util
- Other platforms: stm32, apm32f003
- USB CDC for stm32