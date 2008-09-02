#!/bin/bash

rm -f massif_output_file

echo "Valgrind is running ..."

G_SLICE=always-malloc valgrind --tool=massif --alloc-fn=g_mem_chunk_alloc --alloc-fn=g_malloc --alloc-fn=g_malloc0 --alloc-fn=g_realloc --time-unit=B --massif-out-file=massif_output_file --detailed-freq=1 --threshold=0 $@ 2> /dev/null

echo -e "Processing data ...\n"

ms_print --threshold=0 massif_output_file | fgrep '.c:' | cut -d '(' -f3 | cut -d ')' -f1 | sort | uniq -c | sort -nr

rm -f massif_output_file
