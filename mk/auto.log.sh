Log_Use_Buffer=1
Log_Buffer=

NL="
"

log_init () {
    Log_Use_Buffer=1
    Log_Buffer=
}

log_write () {
    if [ -n "$Log_Use_Buffer" ]
    then
        Log_Buffer="$Log_Buffer$NL$1"
    else
        echo "$1" >&3
    fi
}

log_open () {
    [ -d "$Objdir" ] || err "log_open called with no Objdir"
    exec 3>"$Objdir/config.log"
    echo "$Log_Buffer" >&3
    Log_Use_Buffer=
}

say () {
    local opt_N= opt_v=
    do_opts "Nv" "$@"
    shift $((OPTIND - 1))

    log_write "$*"

    if [ -n "$opt_v" ]
    then
        [ -n "$Verbose" ] && echo "$*"
    else
        if [ -n "$opt_N" -a -z "$Verbose" ]
        then
            echo -n "$*"
        else
            echo "$*"
        fi
    fi
}

warn () {
    log_write "$*"
    echo "$*" >&2
}

fail () {
    local msg="$(cat)"
    warn "$nl$msg"
    exit 1
}

err () {
    warn "$*"
    exit 2
}
