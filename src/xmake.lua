set_xmakever("2.9.8")
add_rules("mode.debug","mode.release")
set_languages("c11","cxx20")
rule("asm.elf",function()
    set_extensions(".asm")
    on_build_file(function (target, sourcefile, opt)
        -- print("sourcefile",sourcefile)
        local objfile = target:objectfile(sourcefile)
        os.mkdir(path.directory(objfile))
		local includes=""
		for _,v in ipairs(target:get("includedirs")) do
			includes=includes.."-I"..v
		end
        os.runv("nasm", {"-f", "elf32", includes, "-o", objfile, sourcefile})
		table.insert(target:objectfiles(),objfile)
    end)	
	on_link(function(target)
		local objfiles = target:objectfiles()
		local outputfile = target:targetdir().."/"..target:filename()
		os.mkdir(target:targetdir())
		os.runv("ld", {"-m", "elf_i386","-e","entry_kernel","-Ttext","0x00020000", "-o", os.projectdir().."/run/kernel.o", unpack(objfiles)})
		os.runv("ld", {"-m", "elf_i386","-e","entry_kernel","-Ttext","0x00020000","--oformat","binary", "-o", outputfile, unpack(objfiles)})
	end)
end)

rule("asm.bin",function()
    set_extensions(".asm")
    on_build_file(function (target, sourcefile, opt)
        -- print("sourcefile",sourcefile)
        os.mkdir(target:targetdir())
		local includes=""
		for _,v in ipairs(target:get("includedirs")) do
			includes=includes.."-I"..v
		end
		os.runv("nasm", {"-f", "bin", includes, "-o", target:targetdir().."/"..target:filename(), sourcefile})
    end)
	on_link(function(target)
	end)
end)

rule("kernel.build",function()
    before_build(function(target)
        os.runv("mkdir",{"-p","run"})
    end)
	after_build(function(target)
        print("target:",target:name())
        print("targetdir:",target:targetdir())
        mbr = target:dep(target:name()..".mbr")
        bootloader = target:dep(target:name()..".bootloader")
        kernel = target:dep(target:name()..".kernel")

        local mbr_file = path.join(mbr:targetdir(),mbr:filename())
        local bootloader_file = path.join(bootloader:targetdir(), bootloader:filename())
        local kernel_file = path.join(kernel:targetdir(), kernel:filename())

        local hd_file = path.join(os.projectdir(), "run", "hd.img")

        os.runv("dd", {
            "if=" .. mbr_file, "of=" .. hd_file, "bs=512", "count=1", "seek=0",
            "conv=notrunc"
        })
        os.runv("dd", {
            "if=" .. bootloader_file, "of=" .. hd_file, "bs=512", "count=5",
            "seek=1", "conv=notrunc"
        })
        os.runv("dd", {
            "if=" .. kernel_file, "of=" .. hd_file, "bs=512", "count=200", "seek=6",
            "conv=notrunc"
        })
    end)
end)

includes("**/xmake.lua")