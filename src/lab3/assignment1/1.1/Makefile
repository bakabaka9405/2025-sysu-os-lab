HD_IMG := ../../../../hd/hd.img
BUILD_DIR := ../../../../build

run:
	@qemu-system-i386 -hda $(HD_IMG) -serial null -parallel stdio 
build:
	@nasm -f bin mbr.asm -o $(BUILD_DIR)/mbr.bin
	@nasm -f bin bootloader.asm -o $(BUILD_DIR)/bootloader.bin
	@dd if=$(BUILD_DIR)/mbr.bin of=$(HD_IMG) bs=512 count=1 seek=0 conv=notrunc
	@dd if=$(BUILD_DIR)/bootloader.bin of=$(HD_IMG) bs=512 count=5 seek=1 conv=notrunc
clean:
	@rm $(BUILD_DIR)/*.bin