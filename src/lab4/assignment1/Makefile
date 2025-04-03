BUILD_DIR := ../../../build

build:
	@gcc -o $(BUILD_DIR)/c_func.o -m32 -c c_func.c
	@g++ -o $(BUILD_DIR)/cpp_func.o -m32 -c cpp_func.cpp 
	@g++ -o $(BUILD_DIR)/main.o -m32 -c main.cpp
	@nasm -o $(BUILD_DIR)/asm_func.o -f elf32 asm_func.asm
	@g++ -o $(BUILD_DIR)/main.out $(BUILD_DIR)/main.o $(BUILD_DIR)/c_func.o $(BUILD_DIR)/cpp_func.o $(BUILD_DIR)/asm_func.o -m32

run:
	@$(BUILD_DIR)/main.out