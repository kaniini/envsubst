prefix       ?= /usr/local
bindir	     ?= /bin

CFLAGS       ?= -Os -fomit-frame-pointer
CC	     ?= ${CROSS_COMPILE}cc
INSTALL      ?= install

ENVSUBST_SRC = envsubst.c
ENVSUBST_OBJ = ${ENVSUBST_SRC:.c=.o}
ENVSUBST_VER = 0.1

CFLAGS       += -DENVSUBST_VERSION='"${ENVSUBST_VER}"'

envsubst: ${ENVSUBST_OBJ}
	${CC} -o $@ ${ENVSUBST_OBJ}

clean:
	${RM} ${ENVSUBST_OBJ} envsubst

install:
	${INSTALL} -D -m755 envsubst ${DESTDIR}${prefix}${bindir}/envsubst