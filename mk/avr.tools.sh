_avr_find_tool () {
    local var="$1"
    local tool="$2"
    local arg="$3"
    local match="$4"
    local try= t= IFS="$fs"

    say -N "Looking for $tool..."

    if [ -n "$(var_get $var)" ]
    then
        list_add try "$(var_get $var)"
    elif [ -n "$AVR_TOOLS" ]
    then
        list_add try "${AVR_TOOLS%/}/$tool"
    else
        list_add try "$tool" "/usr/local/bin/$tool"
        if [ -n "$ARDUINO_DIR" ]
        then
            list_add try "$ARDUINO_DIR/hardware/tools/$tool"
        fi
    fi

    for t in ${try%$fs}
    do
        say -v "    $t"
        "$t" "$arg" 2>&1 | grep -q "$match" || continue
        
        say " $t"
        var_set $var "$t"
        write_var $var
        return
    done

    fail <<MSG
I cannot find '$tool'. You may need to install the 'avr-gcc' and/or
'avr-binutils' package, or you may need to pass -D$var or -Davr-tools
arguments to configure.

MSG
}

avr_find_tools () {
    _avr_find_tool AVR_CC   avr-gcc -v  "Target: avr"
    _avr_find_tool AVR_CXX  avr-g++ -v  "Target: avr"
    _avr_find_tool AVR_AR   avr-ar  V   "GNU ar"
    _avr_find_tool AVR_OBJCOPY avr-objcopy -v \
        "supported targets:.*elf32-avr"
}
