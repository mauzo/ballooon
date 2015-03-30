find_avr_cc () {
    local try= d= u= out= IFS=:

    [ -n "$AVR_TOOLS_DIR" ] && try=":$AVR_TOOLS_DIR"
    try="$try:$PATH:/usr/bin:/usr/local/bin"
    [ -d "$ARDUINO_DIR" ] && try="$try:$ARDUINO_DIR/hardware/tools"

    say -N "Looking for avr-gcc..."

    for d in ${try#:}
    do
        say -v "    $d"
        [ -x "$d/avr-gcc" ] || continue

        for u in avr-g++ avr-ar avr-objcopy
        do
            [ -x "$d/$u" ] && continue
            fail <<MSG
$d/avr-gcc exists but the other utilities (g++, ar, objcopy)
are not all there. Make sure your installation of avr-gcc and
avr-binutils is complete.
MSG
        done

        say " $d"
        AVR_TOOLS_DIR="$d"
        write_var AVR_TOOLS_DIR
        return
    done

    fail <<MSG
I can't find avr-gcc. You may need to install it from a package, or you
may need to pass -Davr-tools-dir=<dir>.
MSG
}
