.if !target(__<avr.obj.mk>__)
__<avr.obj.mk>__:

_OBJDIR=	${.CURDIR}/obj.${MACHINE}

.if !make(obj) && !exists(${_OBJDIR})
.  error Run 'make obj' first.
.else
.OBJDIR:	${_OBJDIR}
.endif

.PHONY: obj clean cleandir

obj:
	mkdir -p ${_OBJDIR} ${OBJDIRS:S!^!${_OBJDIR}/!}

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

.endif
