_avr_check_arduino () {
    [ -d "$1" ]                             || return 1
    [ -e "$1/hardware/arduino/boards.txt" ] || return 1
    return 0
}

avr_find_arduino () {
    look_for ARDUINO_DIR "Arduino SDK" _avr_check_arduino \
        /usr/local/arduino /usr/share/arduino \
        <<MSG
I can't find the Arduino SDK. You may need to install the 'arduino' or
'arduino-core' package, or pass -Darduino_dir= to configure.

MSG
}

