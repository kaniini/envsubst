PACKAGE_NAME := envsubst
PACKAGE_VERSION := 0.1

prefix       ?= /usr/local
bindir	     ?= /bin

CFLAGS       ?= -Os -fomit-frame-pointer
CC	     ?= ${CROSS_COMPILE}cc
INSTALL      ?= install

ENVSUBST_SRC = envsubst.c
ENVSUBST_OBJ = ${ENVSUBST_SRC:.c=.o}

CFLAGS       += -DENVSUBST_VERSION='"${PACKAGE_VERSION}"'

envsubst: ${ENVSUBST_OBJ}
	${CC} ${LDFLAGS} -o $@ ${ENVSUBST_OBJ}

clean:
	${RM} ${ENVSUBST_OBJ} envsubst

install:
	${INSTALL} -D -m755 envsubst ${DESTDIR}${prefix}${bindir}/envsubst

DIST_NAME = ${PACKAGE_NAME}-${PACKAGE_VERSION}
DIST_TARBALL = ${DIST_NAME}.tar.xz

check:
distcheck: check dist
dist: ${DIST_TARBALL}
${DIST_TARBALL}:
	git archive --format=tar --prefix=${DIST_NAME}/ -o ${DIST_NAME}.tar ${DIST_NAME}
	xz ${DIST_NAME}.tar