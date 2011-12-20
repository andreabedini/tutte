#!/usr/bin/awk -f

BEGIN {
    RS = ","
    FS = "--"
    edges = 0
    min_vertex_index = 999999999;
}

{
    if ($1 < min_vertex_index)
	min_vertex_index = $1
    if ($2 < min_vertex_index)
	min_vertex_index = $2
    u[edges] = int($1)
    v[edges] = int($2)
    edges ++
}

END {
    i = 0;
    while (1) {
	printf("%d--%d", u[i] - min_vertex_index, v[i] - min_vertex_index)
	i ++
	if (i == edges) break
	else printf(",")
    }
    print ""
}
