cache="$HOME/.zzccache"
[[ ! -d "$cache" ]] && mkdir "$cache"

_cache_update() {
	local allpackages=""
	for d in /var/cache/zypp/raw/*; do
		local pkgs=$d/suse/setup/descr/packages.gz;
		local base=${d:t}
		local update=0
		local date=0
		
		# figure when was this repo last cached
		if [[ -e "$cache/$base" ]]; then
			date=`stat -c "%Y" "$cache/$base"`
		fi
		
		# figure out if it needs an update
		if [[ -e "$pkgs" ]]; then
			if [[ $date < `stat -c "%Y" "$pkgs"` ]]; then
				update=1
			fi
		fi
		for f in $d/repodata/*primary.xml.gz(N); do
			if [[ $date < `stat -c "%Y" "$f"` ]]; then
				update=1
			fi
		done
		
		#update the cache, if neccessary
		if [[ $update == 1 ]]; then
			rm -f "$cache/$base"
			if [[ -e "$pkgs" ]]; then
				if [[ $date < `stat -c "%Y" "$pkgs"` ]]; then
					# using grep before sed really speeds things up
					#gunzip -c $pkgs | grep "^=Pkg: " | sed -n 's/^=Pkg:\s\s*\([^ ]*\).*/\1/p' >> "$cache/$base"
					# using my pkgnames thing is even much faster
					zypperpkgnames "$pkgs" >> "$cache/$base"
				fi
			fi
			for f in $d/repodata/*primary.xml.gz(N); do
				#gunzip -c $f | grep "<name>" | sed -n 's/.*<name>\(.*\)<\/name>.*/\1/p' >> "$cache/$base"
				zypperpkgnames "$f" >> "$cache/$base"
			done
		fi
	done
	for d in $cache/*(N); do
		if [[ ! -e /var/cache/zypp/raw/${d:t} ]]; then
			rm $d
		fi
	done
	cat $cache/* | uniq > "$cache/all"
}

_cache_update
