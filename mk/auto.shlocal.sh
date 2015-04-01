quote () {
    echo "$1" | sed -e's/[$`"\\]/\\&/g;s/$/\\/;$s/\\$//'
}

case "$Local_Type" in
ash) eval '
    alias local_enter=local

    local_leave () {
        return $? 
    }
'   ;;
ksh) eval '
    alias local=typeset
    alias local_enter=typeset

    local_leave () { 
        return $?
    }
'   ;;
*)  eval '
    __local_level=0

    local () {
        for __local_a in "$@"
        do
            __local_n="${__local_a%%=*}"

            __local_l="__local_${__local_level}"
            __local_s="${__local_l}_${__local_n}"

            eval "${__local_s}=\"\$(quote \"\$${__local_n}\")\""
            eval "${__local_l}=\"\$${__local_l} ${__local_n}\""

            if [ "$__local_n" != "$__local_a" ]
            then
                __local_v="${__local_a#*=}"

                eval "${__local_n}=\"$(quote "${__local_v}")\""
            fi
        done

        unset __local_a __local_n __local_l __local_s __local_v
    }

    local_enter () {
        __local_level=$(( __local_level + 1 ))
        [ $# -gt 0 ] && local "$@"
    }

    local_leave () {
        __local_r=$?
        __local_l="__local_${__local_level}"

        __local_IFS="$IFS"
        IFS=" "

        for __local_n in $(eval "echo \$${__local_l}")
        do
            __local_s="${__local_l}_${__local_n}"

            [ -n "$(eval "echo \${${__local_s}+ok}")" ] || continue

            eval "${__local_n}=\"\$(quote \"\$${__local_s}\")\""
            unset ${__local_s}
        done

        IFS="$__local_IFS"
        unset __local_IFS

        unset ${__local_l}
        __local_level=$(( __local_level - 1 ))

        unset __local_l __local_n __local_s
        return $__local_r
    }
'   ;;
esac
