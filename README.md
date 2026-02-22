# PicoDisplay
A multifunctional HUB75 LED Matrix display project intended for a Raspberry Pi 2 W

## Requirements
I developed this project using the Raspberry Pi Pico extension on VSCode to build the project and the `picotool` commandline utility to flash the board via serial with as USB cable. I'm sure that there are many other ways to get this project to work but that would be up to you to figure out

## Getting Started
With the Raspberry Pi Pico extension in VSCode, build the project

Flash the board with the new firmware with `picotool -f load build/PicoDisplay.uf2`

## Inner Workings

### HUB75 Display Driver


## Attributions
Much of the HUB75 pipeline was informed by this amazing repository: [https://github.com/JuPfu/hub75](https://github.com/JuPfu/hub75).