Write_Vars=

var_clean_name () {
    echo "$1" | tr -dc A-Z0-9_
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

var_append () {
    local d=

    if [ $# -eq 3 ]
    then
        d="$3"
    else
        d=" "
    fi

    var_set "$(var_get "$1")$d$2"
}

var_append_if () {
    local n="$1"
    local v="$(var_get "$2")"
    local d="$3"

    if [ -n "$v" ] 
    then
        if [ $# -eq 3 ]
        then
            var_append "$n" "$v" "$d"
        else
            var_append "$n" "$v"
        fi
    fi
}

write_var () {
    local v="$(var_clean_name "$1")"

    case " $Write_Vars " in
    *" $v "*)   ;;
    *)          Write_Vars="$Write_Vars $v" ;;
    esac
}

write_config_mk () {
    local n= v=

    [ -d "$Objdir" ] || err "write_config_mk called with no Objdir"

    for n in $Write_Vars
    do
        v="$(var_get $n)"
        echo "$n=$v"
    done >"$Objdir/config.mk"
}
