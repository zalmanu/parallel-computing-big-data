#! /usr/bin/env bash
 
readonly radius=5

compile() {
	echo "Compiling..."
	gcc -o standard ./standard.c -lm
	gcc -fopenmp -o openmp ./openmp.c -lm
}

run() {
	echo "Running..."

	for img in ./input/*; do 
	    echo "Running for $img"
	    echo "Standard"
	    /usr/bin/time -f "%e" ./standard "$img" "$radius"
	    echo

	    echo "Openmp"
	    /usr/bin/time -f "%e"  ./openmp "$img" "$radius"
	    echo
	    echo "-----------------"
	    echo
	done
} 

main() {
	compile
	run
}

main $@
