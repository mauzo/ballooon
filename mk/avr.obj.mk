MAKEOBJDIR?=	${.CURDIR}/obj.${MACHINE}

.if !make(obj) && !exists(${MAKEOBJDIR})
.BEGIN:
	@echo "Run 'make obj' first!" >&2
	@false
.else
.OBJDIR:	${MAKEOBJDIR}
.endif

.PHONY: obj clean cleandir

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
