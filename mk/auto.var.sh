nl="
"
fs="$(printf "\034")"

Write_Vars=

var_clean_name () {
    echo "$1" | tr -dc A-Za-z0-9_
}

var_set () {
    eval "$(var_clean_name "$1")=\"$(quote "$2")\""
}

var_get () {
    eval "echo \"\$$(var_clean_name "$1")\""
}

var_default () {
    if [ -z "$(var_get "$1")" ]
    then
        var_set "$1" "$2"
    fi
}

# List entries always have a trailing FS. An empty list is an empty
# variable; a variable containing one FS is a list containing the empty
# string.

list_add () {
    local_enter n="$1" v=
    shift

    for v in "$@"
    do
        var_set "$n" "$(var_get "$n")$v$fs"
    done

    local_leave
}

list_add_if () {
    local_enter n="$1" v=
    shift

    for v in "$@"
    do
        [ -n "$v" ] || continue
        var_set "$n" "$(var_get "$n")$v$fs"
    done

    local_leave
}

list_find () {
    local_enter n="$1"
    local k="$2" 
    local v=

    case "$fs$(var_get $n)" in
    *"$fs$k$fs"*)   true    ;;
    *)              false   ;;
    esac

    local_leave
}

write_var () {
    local_enter v=

    for v in "$@"
    do
        list_find Write_Vars "$v"    && continue
        list_add Write_Vars "$v"
    done

    local_leave
}

write_config_mk () {
    local_enter n= v= c= IFS="$fs"

    [ -d "$Objdir" ]        || err "write_config_mk called with no Objdir"
    [ -n "$Write_Vars" ]    || return

    c="$Objdir/config.mk"
    say "Writing $c..."
    
    for n in ${Write_Vars%$fs}
    do
        v="$(var_get $n)"
        echo "$n=$v"
    done >"$c"

    local_leave
}
