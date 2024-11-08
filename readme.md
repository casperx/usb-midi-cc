# USB MIDI CC

Map Air pressure from TM7711 sensor into MIDI channel 0, MIDI CC 2.

![device picture 1](doc/img/20241109_031341.webp)
![device picture 2](doc/img/20241109_031404.webp)

Based on Raspberry Pi Pico.

Store following configuration in Flash.

- MIDI channel
- MIDI CC
- Sensor Input and MIDI CC Output range
- Mapping curve (50 points max.)

To configure the device, please go [here](https://casperx.github.io/usb-midi-cc-settings/)

>  please use Chromium-based browser, as it need WebUSB to function.

![settings ui](doc/img/ui-20241109-035551.png)

On the left, you'll have live display of signal values from the device.
It shows two lines, green and blue. Blue line is the MIDI CC value currently output from device.
Green line is the preview of your mapping. If you press apply, it will become the same  value.

Press save to persist settings to device's Flash. It'll be loaded on startup.

![settings ui signals](doc/img/ui-20241109-040929.png)

You can calibrate the device by adjusting input range until it lies inside the display frame.
