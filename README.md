# A Fluidsynth running on a Raspberry Pi with a 64x128 OLED display.

## Prerequisits
* Add overlay ssd1306-spi to /boot/firmware/config.txt  
  `dtoverlay=ssd1306-spi`  
  After doing so, you can use fbtft staging. Simply write to dev/fb0, 16-bit per pixel.
* Add overlay rotary encoder to /boot/firmware/config.txt  
  `dtoverlay=rotary-encoder,pin_a=17,pin_b=27,relative_axis=1`
* Add overlay gpio-key to /boot/firmware/config.txt  
`dtoverlay=gpio-key,gpio=23,keycode=68,label="BUTTON",gpio_pull=up`  
keycode 68 is for the F10 keyboard key
* get essential libs for building fluidsynth
```shell
sudo apt-get update
sudo apt-get install build-essential cmake libglib2.0-dev libasound2-dev libjack-jackd2-dev libpulse-dev libsndfile1-dev libreadline-dev
```
* Clone the fluidsynth repo
`git clone https://github.com/FluidSynth/fluidsynth.git`
* Build fluidsynth
    ```shell
    cd fluidsynth  
    mkdir build   
    cd build   
    cmake ..  
    make   
    sudo make install
    ```

## Build
Run this script:
`
    ./build.sh
`
## Auto startup
Copy KerSynth.service to etc/systemd/system and enable the service. You can also start the service right away if you want... 
```sh
sudo cp /etc/systemd/system/KerSynth.service
sudo systemctl enable KerSynth.service

sudo systemctl start KerSynth.service
```
Check the service status with
```sh
sudo systemctl status KerSynth.service
```
## Hardware
Connect the hardware as below.
### OLED
| OLED pin    | RPi4 BCM pin # |
| ----------- | --------- |
| DC          | 24        |
| RST         | 25        |
| SCK         | 11 (SCLK) |
| SDA         | 10 (MOSI) |
| CS          | 8 (CE0)   |
| VCC          | 3V3   |
| VEE          | GND   |

No pull ups are required.

### Encoder
| Encoder pin | RPi4 BCM pin # |
| ----------- | --------- |
| A           | 17        |
| GND         | GND       |
| B           | 27        |

No pull ups are required.

### Button
| Button pin | RPi4 BCM pin # |
| ----------- | --------- |
| NO           | 23        |

No pull ups are required.
