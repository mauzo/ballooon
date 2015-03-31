avr_find_arduino () {
    local try= d= IFS="$fs"

    if [ -n "$ARDUINO_DIR" ]
    then
        list_add try "$ARDUINO_DIR"
    else
        list_add try /usr/local/arduino /usr/share/arduino
    fi

    say -N "Looking for Arduino SDK..."

    for d in ${try%$fs}
    do
        say -v "    $d"
        [ -d "$d" ]                             || continue
        [ -e "$d/hardware/arduino/boards.txt" ] || continue

        say " $d"
        ARDUINO_DIR="$d"
        write_var ARDUINO_DIR
        return
    done

    fail <<MSG
I can't find the Arduino SDK. You may need to install the 'arduino' or
'arduino-core' package, or pass -Darduino_dir= to configure.

MSG
}
