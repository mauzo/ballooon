.PHONY: obj clean cleandir .broken

obj:
	mkdir -p ${MAKEOBJDIR} ${OBJDIRS:S!^!${MAKEOBJDIR}/!}

.if !empty(CLEANFILES)
.NOPATH: ${CLEANFILES}
.endif

clean:
.if !empty(CLEANFILES)
	rm -f ${CLEANFILES}
.endif
.if !empty(CLEANDIRS)
	rm -rf ${CLEANDIRS}
.endif

.if ${.OBJDIR} != ${.CURDIR}
cleandir:
	rm -rf ${.OBJDIR}
.else
cleandir: clean cleandepend
.endif

all: .broken

.broken:
	@if [ -n '${BROKEN}' ]; then \
		echo '${BROKEN}' >&2; \
		false; \
	else \
		true; \
	fi
