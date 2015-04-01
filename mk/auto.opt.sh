Verbose=

usage () {
    echo "Usage: ./configure [-v] [-Dvar=value] [--var=value]" >&2
    exit 1
}

process_opts () {
    local_enter
    local opt= OPTIND=0 OPTARG= n= v=

    Verbose=

    while getopts ":vhD:-:" opt "$@"
    do
        case "$opt" in
        \?) echo "Unknown option -$OPTARG." >&2
            usage
            ;;
        :)  echo "Option -$OPTARG requires an argument." >&2
            usage
            ;;
        h)  usage
            ;;
        v)  Verbose=YES
            ;;
        D|-)
            case "$OPTARG" in
            *=*)    n="${OPTARG%%=*}"
                    v="${OPTARG#*=}"
                    ;;
            *)      n="$OPTARG"
                    v=1
                    ;;
            esac
            n="$(echo "$n" | tr a-z- A-Z_)"
            var_set "$n" "$v"
            ;;
        esac
    done

    if [ "$OPTIND" -ne "$(( $# + 1 ))" ]
    then
        usage
    fi

    local_leave
}

do_opts () {
    local_enter opt OPTARG q v
    local opts="$1"
    shift
    
    while getopts ":$opts" opt "$@"
    do
        case "$opt" in
        \?|:)
            err "Internal error: bad option -$OPTARG"
            ;;
        [a-zA-Z0-9])
            q="$(quote "$OPTARG")"
            [ -z "$q" ] && q=1
            eval "opt_$opt=\"$q\""
            ;;
        *)  err "Internal error: bad option -$opt"
            ;;
        esac
    done

    local_leave
}
