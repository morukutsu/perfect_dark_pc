# Todo

1. Load the ROM in memory then change dmaExec to load assets from the ROM (DONE)
2. Allocate blob of memory for the memory pool instead of the blob at a fixed address. Can ignore the heapstart and just allocate a big buffer (8MB, something like that.) (DONE)
3. mema works with 32 bit addresses, must fix?
4. remove the break commented in main.c: void mainLoop(void) NOTE PC THIS SHOULD NOT BE HERE
5. see how the main loop for the title behaves, open a window, etc...
6. For handling 32bits pointers in the display list, we can add additional storage outside of the display list by creating a mapping: (display_list_command_addr -> storage_space). Then the renderer can get access to the storage space by requesting data. The hashmap should be reset every frame.

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

# Tags

Some tags to research in the code for the PC port:
Tag: Display List Element Size

# Search

- ")gdl" to find wrong display list casts
