# HSB Waves

## Compile and flash
You need:
* avr-gcc
* avr-libc
* avr-binutils
* avrdude
Go in the root directory of the repo, build and flash:
```bash
git clone https://github.com/plushvoxel/HSB-Wave.git
cd HSB-Wavecd HSB-Wave
make && AVRDUDE_PROGRAMMER=stk500v2 PORT=/dev/ttyUSB0 make flash
```
