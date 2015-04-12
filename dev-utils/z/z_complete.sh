# gt complete config

function _get_dir() {
	echo `cat ~/.z/path.db | cut -d ':' -f 1 | tr "\n" " "`
}

function z_comp() {
	local cur opts

	COMPREPLY=()

	cur="${COMP_WORDS[COMP_CWORD]}"
	opts="-a --add -d --delete -l --list -h --help"

	if [[ ${cur} == -* ]] ; then
		COMPREPLY=( $(compgen -W "${opts}" -- ${cur}) )
		return 0
	else
		gt_dirs=$(_get_dir)
		COMPREPLY=( $(compgen -W "${gt_dirs}" -- ${cur}) )
	fi
}
