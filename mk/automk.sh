. ./mk/auto.var.sh
. ./mk/auto.opt.sh
. ./mk/auto.log.sh
. ./mk/auto.make.sh

automk_init () {
    process_opts "$@"
    find_make
    make_obj
    log_open
}
