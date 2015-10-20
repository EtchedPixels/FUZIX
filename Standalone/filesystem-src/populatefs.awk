BEGIN {
	mkdirp("/tmp");
	printf "chmod 01777 /tmp\n";
}

function dirname(f,  d)
{
	d = "";
	if (f ~ /\//)
		d = gensub(/\/[^/]*$/, "", "g", f);
	if (d == "")
		return "/";
	return d;
}

function basename(f)
{
	return gensub(/^.*\/([^/]*$)/, "\\1", "g", f);
}

function mkdirp(dir,  base)
{
	if (!made[dir])
	{
		base = dirname(dir);
		if (base != "/")
			mkdirp(base);
		printf "mkdir " dir "\n";
		printf "chmod 0755 " dir "\n";
		made[dir] = 1;
	}
}

{
	dest = $1
	mode = $2
	arg = $3

	if (NF > 0)
	{
		dir = dirname(dest);
		mkdirp(dir);
		printf "cd " dir "\n";
		if (arg ~ /^[0-9]+$/)
			printf "mknod " dest " " mode " " arg "\n";
		else
		{
			printf "bget " arg " " basename(dest) "\n";
			printf "chmod " mode " " dest "\n";
		}
	}
}

END {
}

