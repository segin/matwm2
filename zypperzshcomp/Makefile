LDFLAGS+=-lz
DEST=/usr/share/zsh/functions/Completion/openSUSE/_zypper

all: pkgnames

clean:
	rm -f pkgnames

install: all
	cp _zypper $(DEST)
	install -s pkgnames /usr/bin/zyppercomp_pkgnames
