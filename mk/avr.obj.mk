.if !target(__<avr.obj.mk>__)
__<avr.obj.mk>__:

.PHONY: clean cleandir

.BEGIN: .objdirs

.objdirs:
	@mkdir -p ${OBJDIRS}

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
