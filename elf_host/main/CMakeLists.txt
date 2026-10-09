set(srcs "elf_console_example.c" "cpp_symbols.cpp")

idf_component_register(SRCS "${srcs}"
                       REQUIRES nvs_flash shell littlefs elf_loader)

# Create a LittleFS image from the contents of the 'fs_image' directory
# that fits the partition named 'storage'. With FLASH_IN_PROJECT a plain 'idf.py flash' also writes the image.
littlefs_create_partition_image(storage fs_image FLASH_IN_PROJECT)
