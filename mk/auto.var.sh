nl="
"
fs="$(printf "\034")"

Write_Vars=

var_clean_name () {
    echo "$1" | tr -dc A-Za-z0-9_
}

quote () {
    echo "$1" | sed -ne's/[$`"\\]/\\&/g;p;q'
}

var_set () {
    local n="$(var_clean_name "$1")"
    local v="$(quote "$2")"

    eval "$n=\"$v\""
}

var_get () {
    local n="$(var_clean_name "$1")"
    eval "echo \"\$$n\""
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
    local n="$1" v=
    shift

    for v in "$@"
    do
        var_set "$n" "$(var_get "$n")$v$fs"
    done
}

list_add_if () {
    local n="$1" v=
    shift

    for v in "$@"
    do
        [ -n "$v" ] || continue
        var_set "$n" "$(var_get "$n")$v$fs"
    done
}

list_find () {
    local n="$1"
    local k="$2" 
    local v=

    case "$fs$(var_get $n)" in
    *"$fs$k$fs"*)   return 0    ;;
    *)              return 1    ;;
    esac
}

write_var () {
    local v=

    for v in "$@"
    do
        list_find Write_Vars "$v"    && continue
        list_add Write_Vars "$v"
    done
}

write_config_mk () {
    local n= v= IFS="$fs"

    [ -d "$Objdir" ]        || err "write_config_mk called with no Objdir"
    [ -n "$Write_Vars" ]    || return

    for n in ${Write_Vars%$fs}
    do
        v="$(var_get $n)"
        echo "$n=$v"
    done >"$Objdir/config.mk"
}
