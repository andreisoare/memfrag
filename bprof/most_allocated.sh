#!/bin/bash

rm -f massif_output_file

echo "Valgrind is running ..."

G_SLICE=always-malloc valgrind --tool=massif --alloc-fn=g_mem_chunk_alloc --alloc-fn=g_malloc --alloc-fn=g_malloc0 --alloc-fn=g_realloc --time-unit=B --massif-out-file=massif_output_file $@ 2> /dev/null

ms_print massif_output_file | grep -e '0$' -e '%' | grep -v -e 'within' -e 'places' -e 'snapshots' -e '???'  | tr -s ' ' | while read a; do
	if [[ $a =~ (^->) ]]; then
		if ! [[ $a =~ (^->0) ]]; then
			ok=1
		else ok=0
		fi
	fi
	if [[ $a =~ (0$) ]]; then echo -e "\nSNAPSHOT:$a\n"; fi
	if (($ok)); then echo $a; fi
done

rm -f massif_output_file
