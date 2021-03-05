## Project for CYBER PHYSICAL SYSTEMS course

Welcome to Energy Meter application. This project implements an APP which
measures the AC Energy Consumption. The system will use a Nucleo64 board which
runs RIOT.

The final doc release is available here (only ita) [Release documentation](./media/ITA_final_documentation.pdf)
Note: Documentation was written for tag v1.0.

### Instructions
1. Setup your system. I'm using fedora so the command that you find are based
   on my distro.
   ```
   $ sudo dnf update
   $ sudo dnf install arm-none-eabi-binutils-cs arm-none-eabi-newlib arm-none-eabi-gcc-cs arm-none-eabi-gcc-cs-c++
   $ sudo dnf install git-core make stlink make patch vim
   ```

2. Checkout of the code:
   ```
   $ mkdir app
   $ cd app
   $ git clone https://github.com/RIOT-OS/RIOT.git
   $ git clone https://github.com/Ciusss89/_riot-os_app.git
   ```

   The app directory should be contain two `RIOT` and  `_riot-os_app` directory,
   setup the code:

   ```
   cd RIOT/
   git checkout <LATEST_RELEASE>
   cd ../_riot-os_app
   git checkout <LATEST_RELEASE>
   ```

### Compile the app and Run

1.  Compile and write the application on the nucleo board:
    ```
    make clean all flash
    ```
2.  The Nucleo board has debug chip on-board. It connectes its virtual COM
    `/dev/ttyACM0` to the SUART2 of the mcu. Open new terminal and start uart
    communication:
    ```
    sudo picocom -b 115200 /dev/ttyACM0 --imap lfcrlf
    ```
    Otherwise you can run the command `make term`
3.  Run `help` to see a list of all available commands
```
> 2021-03-05 10:35:02,990 # main(): This is RIOT! (Version: 2021.04-devel-931-gbf93d)
2021-03-05 10:35:02,993 # RIOT on a nucleo-l476rg board, MCU stm32
2021-03-05 10:35:02,996 # [###] DEBUG LEVEL=3
2021-03-05 10:35:02,998 # Starting EnergMeter service...
2021-03-05 10:35:03,000 # [*] CT sensor setup:
2021-03-05 10:35:03,002 # 	 RMS MAX current: 15A
2021-03-05 10:35:03,005 # 	 Max primary peak current: 21.213203A
2021-03-05 10:35:03,009 # 	 Max secondary peak current: 0.010607A
2021-03-05 10:35:03,012 # 	 Burden resistor: 141.421356Ω
2021-03-05 10:35:03,013 # [*] ADC setup:
2021-03-05 10:35:03,014 # 	 ADC bits: 8
2021-03-05 10:35:03,016 # 	 ADC bias offset: 128
2021-03-05 10:35:03,019 # 	 ADC scale factor: 0.011765
2021-03-05 10:35:03,022 # 	 ADC sampling frequency: 600HZ
2021-03-05 10:35:03,025 # 	 ADC gets [12] sample each 1666 usec
2021-03-05 10:35:04,231 # [*] ADC Calibration: Target=[128], Mesured=[127], Bias=[1.63711V]
2021-03-05 10:35:04,235 # [*] Energy Measuring: sampling has started
2021-03-05 10:35:04,240 # [*] Energy Measuring: collect_1m has started
2021-03-05 10:35:04,243 # [*] Energy Measuring: collect_10m has started
2021-03-05 10:35:04,247 # [*] Energy Measuring: collect_60m has started
2021-03-05 10:42:14,259 # help
2021-03-05 10:42:14,262 # Command              Description
2021-03-05 10:42:14,266 # ---------------------------------------
2021-03-05 10:42:14,271 # em                   em - energy meter application
2021-03-05 10:42:14,273 # reboot               Reboot the node
2021-03-05 10:42:14,277 # version              Prints current RIOT_VERSION
2021-03-05 10:42:14,282 # pm                   interact with layered PM subsystem
2021-03-05 10:42:14,289 # ps                   Prints information about running threads.
```
### Project Overview

1. Hardware:
> I tested the project with a STM Nucleo64 board and it works obviously with
> others boards supported by Riot.
> Regarding the current probe I used a cheap current transformer (CT) which
> converts the induced current into a tension using burden resistor as
> transconductance amplifier.

2. Option setup:
> All relevant parameter are configurable by Makefile throught CFLAGS
> CT supported: YHDC TA1020, YHDC SCT013-000 

2. Algoirthms:
> The ADC samples a signal plus the DC bias. It collects continuously 12 samples
> for each 20ms and remove the DC bias offset. The data is filtered through
> moving average algorithm.
> There are four thread, the first take care of ADC conversion, the others
> store the realtime measure into two array (current, voltage). Each array
> collects 240 samples of last 60 seconds, last 10minutes, last 60minutes.
> Each array has been implemented as circular buffer.

3. Implementation:
![system](./media/energy_meter_bb.jpg)
![system](./media/energy_meter_cc.jpeg)

### Examples:
Set debug mode 3 which prints the contents of array, then saves the printed
output into csv file. Check the directory tests or you can take a look at
this [Google sheet](https://docs.google.com/spreadsheets/d/1U7BSr7nPFPo_V7PK5NfKe2i75gcsldSk4o3UBebkcwg/edit#gid=716511268)
```
> ps
	pid | state    Q | pri
	  1 | running  Q |   7
	  2 | bl mutex _ |   6
	  3 | bl mutex _ |   5
	  4 | bl mutex _ |   5
	  5 | bl mutex _ |   5
> help
Command              Description
---------------------------------------
em                   em - energy meter application
reboot               Reboot the node
version              Prints current RIOT_VERSION
pm                   interact with layered PM subsystem
ps                   Prints information about running threads.
> em
Last 60s samples:
 id;Current;Voltage
  0; 0.24; 230.00
  1; 0.24; 230.00
  2; 0.29; 230.00
.
..
...
237; 0.73; 230.00
239; 0.53; 230.00
Last 10m samples:
 id;Current;Voltage
  0; 0.73; 230.00
  1; 0.53; 230.00
  2; 0.47; 230.00
.
..
...
238; 0.53; 230.00
239; 0.53; 230.00
Last 60m samples:
 id;Current;Voltage
  0; 0.73; 230.00
  1; 0.53; 230.00
  2; 0.47; 230.00
.
..
...
236; 0.53; 230.00
237; 0.55; 230.00
238; 2.33; 230.00
239; 2.53; 230.00
Current 0.407541A
Voltage 230V
last 60 seconds current average 0.39A
last 60 seconds voltage average 230.00V
last 10 minutes current average 0.41A
last 10 minute voltage average 230.00V
last 60 minutes current average 0.50A
last 60 minutes voltage average 230.00V
```
