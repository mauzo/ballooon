.if !target(__<avr.obj.init.mk>__)
__<avr.obj.init.mk>__:

.ifdef MAKEOBJDIRPREFIX
MAKEOBJDIR=	${MAKEOBJDIRPREFIX}${.CURDIR}
.else
MAKEOBJDIR?=	${.CURDIR}/obj.${TARGET}
.endif

__out!=		mkdir -p ${MAKEOBJDIR} 2>&1 && echo ok
.if ${__out} != ok
.  error Can't create object dir '${MAKEOBJDIR}': ${__out}	# ' grr
.endif

.OBJDIR:	${MAKEOBJDIR}

.endif
