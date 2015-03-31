MAKE=
Objdir=

_check_make () {
    [ "$("$1" -V'${.CURDIR:?ok:nok}' 2>&1)" = ok ]
}

find_make () {
    look_for MAKE "a BSD make" \
        _check_make -- \
        make bmake pmake \
        <<MSG
I can't find a BSD make. You may need to install a 'bmake' package, or
you can get a portable version from
    http://www.crufty.net/help/sjg/bmake.htm

MSG
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

make_depend () {
    say "Running $MAKE depend..."
    log_write "$($MAKE depend 2>&1)"
    say
    say "Now you need to run $MAKE."
}
