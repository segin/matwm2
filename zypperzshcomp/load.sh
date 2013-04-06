#!/usr/bin/env zsh

setfp() {
	local exists=0
	for f in $fpath; do if [[ "$f" == "." ]]; then exists=1; fi; done
	if [[ "$exists" == "0" ]]; then
		fpath=(. $fpath)
	fi
}

setfp
unfunction _zypper 2> /dev/null
autoload -U _zypper

