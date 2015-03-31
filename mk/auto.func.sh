_check_function () {
    local libs="$1"
    local func="$2"
    local rv=

    try_write try.c "
int $func (void);

int
main (void)
{
    $func();
}
"
    try_cc try.c $libs
    rv=$?

    try_clean
    return $rv
}

find_function () {
    local func="$1"
    shift 1
    local libs=

    look_for libs $func \
        _check_function $func -- \
        "$@"

    if [ -n "$libs" ]
    then
        LIBS="$LIBS $libs"
        write_var LIBS
    fi
}
