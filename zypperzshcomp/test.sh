cache="$HOME/.zzccache"

[[ ! -d "$cache" ]] && mkdir "$cache"

_cache_update() {
	local allpackages=""
	for d in /var/cache/zypp/raw/*; do
		local pkgs=$d/suse/setup/descr/packages.gz;
		local base=${d:t}
		rm "$cache/$base"
		if [[ -e "$pkgs" ]]; then
			# using grep before sed really speeds things up
			gunzip -c $pkgs | grep "^=Pkg" | sed -n 's/^=Pkg:\s\s*\([^ ]*\).*$/\1/p' >> "$cache/$base"
		fi
		for f in $d/repodata/*primary.xml.gz(N); do
			gunzip -c $f | sed -n 's/.*<name>\(.*\)<\/name>.*/\1/p' >> "$cache/$base"
		done
		
		echo $base
	done
	for d in $cache/*(N); do
		if [[ ! -e /var/cache/zypp/raw/${d:t} ]]; then
			rm $d
		fi
	done
	cat $cache/* | uniq > "$cache/all"
}

_cache_update
