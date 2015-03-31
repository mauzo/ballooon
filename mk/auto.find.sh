look_for () {
    local var="$1"
    local title="$2"
    local check="$3"
    shift 3
    local try= t= IFS="$fs"


    if [ -n "$(var_get $var)" ]
    then
        list_add try "$(var_get $var)"
    else
        list_add try "$@"
    fi

    say -N "Looking for $title..."

    for t in ${try%$fs}
    do
        say -v "    $t"
        $check "$t"     || continue

        say " $t"
        var_set $var "$t"
        write_var $var
        return
    done

    fail
}
