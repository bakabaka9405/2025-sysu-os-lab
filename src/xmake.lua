set_xmakever("2.9.8")
add_rules("mode.debug", "mode.release")
set_languages("c11","cxx20")
rule("asm.elf",function()
    set_extensions(".asm")
    on_build_file(function (target, sourcefile, opt)
        local objfile = target:objectfile(sourcefile)
        os.mkdir(path.directory(objfile))
		local includes = {}
        for _, v in ipairs(target:get("includedirs")) do
            table.insert(includes, "-I" .. v)
        end
        os.runv("nasm", {"-f", "elf32", unpack(includes), "-o", objfile, sourcefile})
		table.insert(target:objectfiles(),objfile)
    end)	
	on_link(function(target)
		local objfiles = target:objectfiles()
		local outputfile = target:targetdir().."/"..target:filename()
		os.mkdir(target:targetdir())
        for i,v in ipairs(objfiles) do
            -- 将 entry.asm.o 移到最前
            start, _end = string.find(v, "entry.asm.o")
            if start then
                objfiles[i],objfiles[1]=objfiles[1],objfiles[i]
                break
            end
        end
        -- print(objfiles)
		os.runv("ld", {"-m", "elf_i386","-e","enter_kernel","-Ttext","0x00020000", "-o", os.projectdir().."/run/kernel.o", unpack(objfiles)})
		os.runv("ld", {"-m", "elf_i386","-e","enter_kernel","-Ttext","0x00020000","--oformat","binary", "-o", outputfile, unpack(objfiles)})
	end)
end)

rule("asm.elf2",function()
    set_extensions(".asm")
    on_build_file(function (target, sourcefile, opt)
        local objfile = target:objectfile(sourcefile)
        os.mkdir(path.directory(objfile))
		local includes = {}
        for _, v in ipairs(target:get("includedirs")) do
            table.insert(includes, "-I" .. v)
        end
        os.runv("nasm", {"-g" ,"-f", "elf32", unpack(includes), "-o", objfile, sourcefile})
		table.insert(target:objectfiles(),objfile)
    end)	
	on_link(function(target)
		local objfiles = target:objectfiles()
		local outputfile = target:targetdir().."/"..target:filename()
		os.mkdir(target:targetdir())
        for i,v in ipairs(objfiles) do
            -- 将 entry.asm.o 移到最前
            start, _end = string.find(v, "entry.asm.o")
            if start then
                objfiles[i],objfiles[1]=objfiles[1],objfiles[i]
                break
            end
        end
        -- print(objfiles)
		os.runv("ld", {"-m", "elf_i386", "-N" ,"-e","enter_kernel","-Ttext","0xc0020000", "-o", os.projectdir().."/run/kernel.o", unpack(objfiles)})
		os.runv("objcopy", {"-O", "binary", os.projectdir().."/run/kernel.o", outputfile})
	end)
end)

rule("new_bootloader",function()
    set_extensions(".asm")
    on_build_file(function (target, sourcefile, opt)
        local objfile = target:objectfile(sourcefile)
        table.insert(target:objectfiles(),objfile)
        os.mkdir(target:targetdir())
		local includes = {}
        for _, v in ipairs(target:get("includedirs")) do
            table.insert(includes, "-I" .. v)
        end
		os.runv("nasm", {"-g", "-f", "elf32", unpack(includes), "-o", objfile, sourcefile}) 
    end)
	on_link(function(target)
        local objfiles = target:objectfiles()
        start, _end = string.find(objfiles[2], "bootloader.asm.o")
        if start then
            objfiles[1],objfiles[2] = objfiles[2],objfiles[1]
        end
        -- print(objfiles)
        os.runv("ld", {"-m", "elf_i386", "-e", "bootloader_start", "-Ttext","0x7e00", "-N", "-o", target:targetdir().."/bootloader.o", unpack(objfiles)})
        os.runv("objcopy", {"-O", "binary", target:targetdir().."/bootloader.o", target:targetdir().."/bootloader.bin"})
	end)
end)

rule("asm.bin",function()
    set_extensions(".asm")
    on_build_file(function (target, sourcefile, opt)
        os.mkdir(target:targetdir())
		local includes = {}
        for _, v in ipairs(target:get("includedirs")) do
            table.insert(includes, "-I" .. v)
        end
		os.runv("nasm", {"-f", "bin", unpack(includes), "-o", target:targetdir().."/"..target:filename(), sourcefile})
    end)
	on_link(function(target)
	end)
end)

rule("kernel.build",function()
    before_build(function(target)
        os.runv("mkdir",{"-p","run"})
    end)
	after_build(function(target)
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