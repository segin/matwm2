# Created by: Kirn Gill II <segin2005@gmail.com>
# $FreeBSD: $

PORTNAME=	libapp
PORTVERSION=	0.2.0
CATEGORIES=	devel

MAINTAINER=	segin2005@gmail.com
COMMENT=	A library for automating various development tasks.

USE_GITHUB=	yes
GH_ACCOUNT=	drotiro
GH_PROJECT=	libapp
GH_TAGNAME=	${PORTVERSION}
GH_COMMIT=	b58d1e3

pre-build:
	( cd ${WRKSRC}/.. && if [ ! -L libapp ]; then ln -s $(GH_ACCOUNT)-$(GH_PROJECT)-$(GH_COMMIT) libapp ; fi )

.include <bsd.port.mk>
