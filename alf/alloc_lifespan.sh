#!/bin/bash

echo -e "Valgrind is running ...\n"

G_SLICE=always-malloc valgrind --tool=massif --alloc-fn=g_mem_chunk_alloc --alloc-fn=g_malloc --alloc-fn=g_malloc0 --alloc-fn=g_realloc $@ &> temporary_file

echo -e "Processing data ...\n"
grep -e '^m\ ' -e '^f\ ' -e '^[1-9]' temporary_file > temporary_file2
./fragmentation temporary_file2 temporary_file
grep '\.c' temporary_file | sort -n > coordinate_file

rm -f temporary_file temporary_file2

max=$(awk '{print $2}' coordinate_file | sort -nr | head -n 1)

sed "/coordinate_file/s/max/$max/g" _gnuplot.conf > gnuplot.conf

echo -e "An image file ( lifespan_graph.png ) has been saved. Type the following command in gnuplot, to get an interactive graph (supports zoom-in, zom-out and higher quality, but takes some time to plot):\n\nload 'gnuplot.conf'"

gnuplot <<< "set terminal png giant; set out 'lifespan_graph.png'; set key top right; set xlabel 'Time (instructions)'; set ylabel 'Lifetime (instructions)'; plot 'coordinate_file' using 1:2 with impulses lt rgb 'green' title 'Lifetime', 'coordinate_file' using 1:($max*\$3/100) lt rgb 'red' title 'Fragmentation'"
