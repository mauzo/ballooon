. ./mk/auto.var.sh
. ./mk/auto.opt.sh
. ./mk/auto.log.sh
. ./mk/auto.find.sh
. ./mk/auto.make.sh

. ./mk/auto.try.sh
. ./mk/auto.func.sh

automk_init () {
    [ -z "$TARGET" ] && TARGET="$(uname -m)"
    export TARGET
    
    log_init
    say "Configuring for $TARGET."

    process_opts "$@"
    find_make
    make_obj
    log_open

    try_init
}
