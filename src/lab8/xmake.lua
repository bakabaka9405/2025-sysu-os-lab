for i=1,3 do
	target("lab8-"..i..".mbr", function()
		add_includedirs("common/include")
		add_rules("asm.bin")
		add_files("common/src/boot/mbr.asm")
		set_filename("mbr.bin")
	end)

	target("lab8-"..i..".bootloader", function()
		add_includedirs("common/include")
		add_rules("new_bootloader")
		add_files("common/src/boot/bootloader.asm", "common/src/boot/page.cpp")
		set_filename("bootloader.bin")
		add_cxxflags("-march=i386", "-m32", "-nostdlib", "-fno-builtin", "-ffreestanding", "-fno-pic")
	end)

	target("lab8-"..i..".kernel", function()
		add_includedirs("common/include")
		add_includedirs("assignment"..i.."/include")
		add_rules("asm.elf2")
		add_files("common/src/boot/entry.asm", "common/src/kernel/*.cpp", "assignment"..i.."/src/*.cpp","common/src/utils/*.asm")
		set_filename("kernel.bin")
		add_cxxflags("-march=i386", "-m32", "-nostdlib", "-fno-builtin", "-ffreestanding", "-fno-pic")
		
	end)

	target("lab8-"..i, function()
		set_kind("phony")
		add_deps("lab8-"..i..".mbr", "lab8-"..i..".bootloader", "lab8-"..i..".kernel")
		add_rules("kernel.build")
	end)
end