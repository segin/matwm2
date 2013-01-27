# New ports collection makefile for:   axel
# Date modified:       18 January 2013
# Whom:                Kirn Gill <segin2005@gmail.com>
#
# $FreeBSD: $
#

PORTNAME=	axel
PORTVERSION=	2.4
CATEGORIES=	net
MASTER_SITES=	http://alioth.debian.org/frs/download.php/3015/
DISTNAME=	${PORTNAME}-${PORTVERSION}

MAINTAINER=	segin2005@gmail.com
COMMENT=	A tiny lightweight download accelerator for Unix.

LICENSE=	GPLv2
LICENSE_FILE=	${WRKSRC}/COPYING

USE_GMAKE=	yes
USE_ICONV=	yes
USE_GETTEXT=	yes
HAS_CONFIGURE=	yes
CONFIGURE_ARGS=	--mandir=

PLIST_FILES=	bin/axel
MAN1=		axel.1
DOCS=		LICENSE CHANGES COPYING CREDITS README
PORTEXAMPLES=	axelrc.example 

WRKSRC=		${WRKDIR}/${PORTNAME}-${PORTVERSION}

.include <bsd.port.mk>
