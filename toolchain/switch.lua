rule("switch")
    on_config(function(target)
        target:set("toolchains", "devkita64")
    end)

    after_build(function(target)
        if not target:kind() == "binary" then
            return
        end

        local elf2nro = "elf2nro"
        assert(elf2nro, "elf2nro not in PATH")

        local nacptool = "nacptool"
        assert(nacptool, "nacptool not in PATH")

        local name = target:values("switch.name")
        name = name or "a"

        local author = target:values("switch.author")
        author = author or "unknown"

        local version = target:values("switch.version")
        version = version or "1.0.0"

        cprint("${color.build.target}Generating nacp metadata")

        local nacpfile = path.absolute(path.join(target:targetdir(), "metadata.nacp"))
        if not os.isdir(path.directory(nacpfile)) then
            os.mkdir(path.directory(nacpfile))
        end

        local nacp_args = {"--create", name, author, version, nacpfile}

        vprint(nacptool, table.unpack(nacp_args))
        local outdata, errdata = os.iorunv(nacptool, nacp_args)
        vprint(outdata)
        assert(errdata, errdata)

        local target_file = target:targetfile()
        local nro_file = target_file .. ".nro"

        cprint("${color.build.target}Generating nro file")

        local romfsdir = target:values("switch.romfs")
        local icon = target:values("switch.icon")

        local elf2nro_args = {target_file, nro_file, "--nacp=".. nacpfile}

        if icon ~= nil and icon ~= "" then
            table.insert(elf2nro_args, "--icon=" .. icon)
        end

        if romfsdir ~= nil and romfsdir ~= "" then
            table.insert(elf2nro_args, "--romfsdir=" .. romfsdir)
        end

        vprint(elf2nro, table.unpack(elf2nro_args))
        outdata, errdata = os.iorunv(elf2nro, elf2nro_args)
        vprint(outdata)
        assert(errdata, errdata)

    end)

    on_run(function(target)
        if not target:kind() == "binary" then
            return
        end

        import("core.base.option")
        import("core.project.config")
        import("lib.detect.find_program")
        import("lib.detect.find_tool")
        
        local args = table.wrap(option.get("arguments"))

        if #args > 0 then
            for i, arg in ipairs(args) do
                cprint("${color.build.target}Argument[%d]: ${clear}%s", i, arg)
                if arg:startswith("--nx=") then
                    ip_addr = arg:sub(6)
                    break
                end
            end
        end
       
        -- if find_tool("nxlink", {check = "--help"}) then
        --     nxlink = "nxlink"
        -- else
        --     nxlink = "nxlink"
        --     cprint("You must add nxlink to your PATH variable")
        -- end
        ip_addr = "192.168.1.11"
        local target_file = target:targetfile()
        local executable = target_file .. ".nro"
        local nxlink_args = {"-a", ip_addr, "-s", executable}

        cprint("Launching from nxlink " .. executable)

        -- local args_str = table.concat(nxlink_args, " ")
    
        os.execv("nxlink.exe", nxlink_args)
    end)