MAKE=
Objdir=

find_make () {
    local cwd m out

    say -N "Looking for a BSD make..."

    for m in "$MAKE" make bmake
    do
        [ -z "$m" ]         && continue
        say -v "    trying $m... "

        out="$($m -V".CURDIR:?ok:nok" 2>&1)"
        log_write "$out"
        [ "$out" = ok ] || continue

        say " $m"
        MAKE="$m"
        return
    done

    echo "not found."
    cat <<ERR >&2
I can't find a BSD make. You may need to install a 'bmake' package, or
you can get a portable version from
    http://www.crufty.net/help/sjg/bmake.htm

ERR
    exit 2
}

make_obj () {
    local out

    [ -n "$MAKE" ] || err "make_obj called with no MAKE"

    say -N "Creating obj directory..."

    out="$($MAKE obj 2>&1)"
    log_write "$out"

    Objdir="$($MAKE -V.OBJDIR)"
    say " $Objdir"
}
