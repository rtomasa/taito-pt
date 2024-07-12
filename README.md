# TAITO USB Paddle & Trackball Controller Driver
Linux driver for the TAITO USB Paddle & Trackball Controller.

Both the trackball and the paddle on this device report movements as mouse wheel events. This occurs because the REL_X and REL_Y values from the trackball are interpreted similarly to how mouse wheel movements are reported, rendering the trackball unusable in its default mode.

The driver is primarily designed for use with RePlay OS, which can translate mouse wheel events into mouse X/Y coordinates. If you are using this device with a different system or emulator, you will need to properly configure your emulator to map these events correctly or modify the driver to suit your specific requirements.

## Analog Speed

Different games require different speed adjustments for optimal control. If you find that the trackball or paddle is not functioning as expected, try adjusting the sensitivity settings in your emulator or core options to achieve the desired responsiveness.

### Build and Install

```shell
make modules
sudo make install
sudo depmod -a
sudo modprobe taito_pt
```

To reload after compiling you will first need to unload it using `sudo modprobe -r taito_pt`.
