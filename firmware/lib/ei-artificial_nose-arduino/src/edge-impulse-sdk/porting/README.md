WIP

# Core functions
Chip specific implementations must be provided for the following header files:
- ei_classifier_porting.h 
- ei_device_info_lib.h
- ei_device_interface.h

# Sensors

## Camera

- Ingestion (snapshot, snapshot_stream) format:
The daemon (and the frame-to-jpeg debug tool) expect packed RGB 
(3B per pixel, as opposed to 4B with high B to 0 for inference)
 - Big endian format (uint8_t image[i] is R, image[i+1] is G, ...

- Inference format:
 - The "get_data" function expects RGB encoded into floats, respecting the endianness of the platform
  - If you encode with bit shifting, you'll encode with the endianness of the chip
   - something like: static_cast<float>((r << 16) + (g << 8 ) + b))
  - The encoding is 4B, so there's a pad byte in the MSB of the float

### Porting
- The top level of all firmware is chip specific. ei_image_lib provides portable, convenient functions for binding to AT commands.
```
    config_ctx.take_snapshot = &ei_camera_take_snapshot_output_on_serial;
    config_ctx.start_snapshot_stream = &ei_camera_start_snapshot_stream;
```
- To support these two functions, one must provide a driver implementation for the following interface class: ei_camera_interface.h
 - This is a singleton class pattern with a port provided factory.  All needed functions are prototyped in ei_camera_interface.h
- (The "core" functions must also be present, see "Core functions" above)
- Other support functions exist in ei_image_lib
 - YUV422 to RGB conversion
- ei_image_nn.h supports inference, but does not need to be touched.  It calls the camera factory. 

### Interface notes

at_get_snapshot_list : Return possible resolutions, greyscale, etc
-     const char **color_depth >-should be-> "Greyscale" or "RGB"