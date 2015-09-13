to temporary load the completion, run ". ./load.sh"

for installing:
	make install
	chattr +i /usr/share/zsh/functions/Completion/openSUSE/_zypper

the chattr +i command makes the file immutable so suse won't overwrite it
to undo this run:
	chattr -i /usr/share/zsh/functions/Completion/openSUSE/_zypper
