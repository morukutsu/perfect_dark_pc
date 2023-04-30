# Some notes about the PC port

# Todo

1. Remove the break commented in main.c: void mainLoop(void) NOTE PC THIS SHOULD NOT BE HERE
2. For handling 32bits pointers in the display list, we can add additional storage outside of the display list by creating a mapping: (display_list_command_addr -> storage_space). Then the renderer can get access to the storage space by requesting data. The hashmap should be reset every frame.

# Defines

```
PC_PORT_DMA_DISABLED
PC_PORT_FAULT_DISABLED
PC_PORT_AUDIO_ENABLED

```

# Useful links

http://n64devkit.square7.ch/
https://www.moria.us/tags/nintendo-64
https://n64squid.com/homebrew/n64-sdk/textures/image-formats/
https://jerrywester.github.io/f3djs/modules.html#gsdploadblock
https://www.moria.us/blog/2020/11/n64-part20-tmem-format-and-mip-maps
