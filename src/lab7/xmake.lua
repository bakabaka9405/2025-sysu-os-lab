for _,i in ipairs({1,2,4}) do
	target("lab7-"..i..".mbr", function()
		add_includedirs("common/include")
		add_rules("asm.bin")
		add_files("common/src/boot/mbr.asm")
		set_filename("mbr.bin")
	end)

	target("lab7-"..i..".bootloader", function()
		add_includedirs("common/include")
		add_rules("asm.bin")
		add_files("common/src/boot/bootloader.asm")
		set_filename("bootloader.bin")
	end)

	target("lab7-"..i..".kernel", function()
		add_includedirs("common/include")
		add_includedirs("assignment"..i.."/include")
		add_rules("asm.elf")
		add_files("common/src/boot/entry.asm",  "common/src/kernel/*.cpp","assignment"..i.."/src/*.cpp","common/src/utils/*.asm")
		set_filename("kernel.bin")
		add_cxxflags("-march=i386", "-m32", "-nostdlib", "-fno-builtin", "-ffreestanding", "-fno-pic")
		
	end)

	target("lab7-"..i, function()
		set_kind("phony")
		add_deps("lab7-"..i..".mbr", "lab7-"..i..".bootloader", "lab7-"..i..".kernel")
		add_rules("kernel.build")
	end)
end

target("lab7-3",function()
	set_kind("binary")
	add_files("assignment3/src/*.cpp")
end)