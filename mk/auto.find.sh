look_for () {
    [ $# -ge 3 ] || err "Not enough args for look_for"
    local var="$1"
    local title="$2"
    local check="$3"
    shift 3
    local args= try= t= IFS="$fs"

    while [ $# -gt 0 -a "$1" != "--" ]
    do
        list_add args "$1"
        shift
    done
    shift

    if [ -n "$(var_get $var)" ]
    then
        list_add try "$(var_get $var)"
    else
        list_add try "$@"
    fi

    [ -z "$try" ] && fail

    say -N "Looking for $title..."

    for t in ${try%$fs}
    do
        say -v "    $t"
        $check "$t" ${args%$fs} || continue

        say " $t"
        var_set $var "$t"
        write_var $var
        return
    done

    fail
}
