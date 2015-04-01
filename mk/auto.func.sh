_check_function () {
    local_enter
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
    local_leave
    return $rv
}

find_function () {
    local_enter
    local func="$1"
    shift
    local libs=

    look_for libs $func \
        _check_function $func -- \
        "$@"

    if [ -n "$libs" ]
    then
        LIBS="$LIBS $libs"
        write_var LIBS
    fi

    local_leave
}
