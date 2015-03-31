try_init () {
    CC="$($MAKE -VCC)"
}

_try_dir () {
    [ -d "$Objdir" ]    || err "try_* called with no objdir"
    mkdir -p "$Objdir/.try"
    echo "$Objdir/.try"
}

try_write () {
    local name="$1"
    local content="$2"
    local try

    try="$(_try_dir)/$name"

    log_write "Writing $try <<<"
    log_write "$content"
    log_write ">>>"

    echo "$content" >"$try"
}

try_clean () {
    local dir="$(_try_dir)"
    log_write "Cleaning $dir"
    rm -rf "$dir"
}

_try_run () {
    local dir="$(_try_dir)" IFS
    unset IFS
    log_write "Running [$*]"
    (cd "$dir" && "$@" 2>&1) >&3
}

try_cc () {
    local cmd

    [ -n "$CC" ]        || err "try_cc called with no CC"
    _try_run $CC -o try "$@"
}
