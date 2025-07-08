#! /bin/zsh -

echo -P "%F{yellow}Start compilation %f"
xmake
exitcode=$?

if [ $exitcode -eq 0 ]; then
    echo "\e[42;30m Successful compilation \e[0m"

    for file in build/cross/aarch64/debug/*.nro; do
        if [ -f "$file" ]; then
            echo "\e[45;30m Sending to switch... \e[0m"
            nxlink "$file" -s -r 20
            exit
        fi
    done
else
    echo "\e[41;97m Compilation failed \e[0m" 
fi
