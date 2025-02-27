======
ChibiOS
======

What is ChibiOS?
---------------

`ChibiOS/RT <https://www.chibios.org//>`__ is a free and efficient
RTOS designed for deeply embedded applications. It offers a 
comprehensive set of kernel primitives and supports many 
architectures: ARM7, Cortex-M0, Cortex-M3, Cortex-M4, 
PowerPC e200z, STM8, AVR, MSP430, ColdFire, H8S, x86.
 
Highlights of ChibiOS
~~~~~~~~~~~~~~~~~~~~
- Tiny memory footprint, high performance, easily portable, clean and readable.

- Preemptive kernel, 128 priority levels, reliable static architecture.

- Kernel support for Semaphores, Mutexes, CondVars, Messages, Event Flags, Mailboxes, Virtual Timers.

- IRQ abstraction, support for non-OS fast IRQ sources (zero latency buzzword).

- Support for ARM, ARM-CM0, ARM-CM3, ARM-CM4, PowerPC, STM8, MSP430, AVR, ColdFire, H8S, Linux/Win32/MacOS simulators.

- HAL component makes applications portable across the supported platforms.

- HAL support for Port, Serial, ADC, CAN, DAC, EXT, GPT, I2C, I2S, ICU, MAC, MMC, PWM, RTC, SDC, SPI, UART, USB, WDG device driver models.

- Support for the uIP and lwIP TCP/IP stacks (demo included).

- Support for the FatFS file system library (demo included).
  
How to run LVGL on ChibiOS?
--------------------------

To setup your development environment refer to the
`How to setup ChibiStudio guide <https://www.playembedded.org/blog/how-to-setup-chibistudio/>`__.

Extract ChibiOS/ext/lvgl-9.2.2.7z in ChibiOS/ext/lvgl path.

Import an LVGL demo from ChibiOS/demos/LVGL folder.

To import a project in ChibiStudio refer to the
`How to import a project in ChibiStudio guide <https://www.playembedded.org/blog/how-to-import-a-project/>`__.

To build a project in ChibiStudio refer to the
`How to build a project in ChibiStudio guide <https://www.playembedded.org/blog/how-to-build-a-project/>`__.

To Flash & Run a project in ChibiStudio refer to the
`How to Flash & Run a project in ChibiStudio guide <https://www.playembedded.org/blog/flash-run/>`__.