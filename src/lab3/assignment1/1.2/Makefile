HD_IMG := ../../../../hd/hd.img
BUILD_DIR := ../../../../build
MBR_O := $(BUILD_DIR)/mbr.o
BL_O := $(BUILD_DIR)/bootloader.o
MBR_SYMBOL := $(BUILD_DIR)/mbr.symbol
BL_SYMBOL := $(BUILD_DIR)/bootloader.symbol
MBR_BIN := $(BUILD_DIR)/mbr.bin
BL_BIN := $(BUILD_DIR)/bootloader.bin

run:
	@qemu-system-i386 -hda $(HD_IMG) -serial null -parallel stdio 
debug:
	@gnome-terminal -e "qemu-system-i386 -s -S -hda $(HD_IMG) -serial null -parallel stdio"
build:
	@nasm -g -f elf32 mbr.asm -o $(MBR_O)
	@ld -o $(MBR_SYMBOL) -melf_i386 -N $(MBR_O) -Ttext 0x7c00
	@ld -o $(MBR_BIN) -melf_i386 -N $(MBR_O) -Ttext 0x7c00 --oformat binary

	@nasm -g -f elf32 bootloader.asm -o $(BL_O)
	@ld -o $(BL_SYMBOL) -melf_i386 -N $(BL_O) -Ttext 0x7e00
	@ld -o $(BL_BIN) -melf_i386 -N $(BL_O) -Ttext 0x7e00 --oformat binary

	@dd if=$(MBR_BIN) of=$(HD_IMG) bs=512 count=1 seek=0 conv=notrunc
	@dd if=$(BL_BIN) of=$(HD_IMG) bs=512 count=5 seek=1 conv=notrunc
clean:
	@rm $(BUILD_DIR)/*.bin $(BUILD_DIR)/*.o $(BUILD_DIR)/*.symbol