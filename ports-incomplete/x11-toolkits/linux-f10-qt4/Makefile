# New ports collection makefile for:	x11-toolkits/linux-f10-qt33
# Date created:				2009-05-03
# Whom:					bsam
#
# $FreeBSD: ports/x11-toolkits/linux-f10-qt33/Makefile,v 1.12 2012/11/17 06:03:36 svnexp Exp $
#

PORTNAME=	${LINUX_NAME}
PORTVERSION=	4.4.3
PORTREVISION=	0
CATEGORIES=	x11-toolkits linux
MASTER_SITES=	http://archives.fedoraproject.org/pub/archive/fedora/linux/releases/10/Fedora/i386/os/Packages/
PKGNAMEPREFIX=	linux-f10-
DISTNAME=	qt-4.4.3-2.fc10.i386.rpm   qt-x11-4.4.3-2.fc10  

MAINTAINER=	emulation@FreeBSD.org
COMMENT=	RPM of QT4 (Linux Fedora 10)

USE_LINUX_RPM=	yes
USE_LINUX_APPS=	fontconfig libmng png
LINUX_NAME=	qt4
LINUX_DIST_VER=	10
RPMVERSION=	17.fc10
USE_LDCONFIG=	yes
PLIST=		${PKGDIR}/pkg-plist.${LINUX_RPM_ARCH}

DESCR=		${.CURDIR}/../${PORTNAME}/pkg-descr

.include <bsd.port.mk>
