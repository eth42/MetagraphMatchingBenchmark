1. Compilation

This software requires a Boost version of at least 1.5.5 [1]. To compile it, simply run the build_unix.sh file. Without modification, it will create a binary, that supports a set of graph generators and solvers described below. If you wish to add the Blossom IV [3] (which requires Concorde-97 [4]) and V [5] solvers, you will have to download them on their respective website. The additional solver from the Lemon graph library can be downloaded on their site [2] as well. An additional graph generator for delaunay triangulations can be obtained from the Fade2D page [6]. To compile external libraries into the binary, unpack them in their respective folder in the libs directory and uncomment the respective lines in the build_unix.sh script.
Notice: Some make files had to be adapted. Take care not to overwrite the files provided with this software.
If you wish to experiment on nearest neighbor graphs of TSPLIB instances [7], download the .tsp files and unpack them somewhere in the assets directory. They can be referenced relative to this path.

#####################################################################

2. General

This is a benchmark suite and not an API! Making proper calls from the command line will run an experiment and output the results in a measure.csv file next to the binary. All generated graphs will be cached in the assets directory. You probably want to empty it from time to time.

#####################################################################

3. Execution

To run the binary, it has to be called with a graph source and a solver type. The number of iterations, shuffling and seed are optional and have default values. A call to the binary is build as follows:

Masterarbeit.exe [-o] [-s | -S <SEED> | -nS <SEED>] -i <ITERATIONS> <SOLVER> <SOURCE>

Where <ITERATIONS> and <SEED> are positive integers
... <SOLVER> is one of the following
	-mg				The metagraphs base algorithm
	-mgwr <B>		Same as mg but with bounded meta degree <B>
	-mgqpt <B> <C>	Same as mgwr but with lazy blossom creation up to a meta degree of <C>. <C> has to be smaller or equal to <B>
	-mt				A cherry tree algorithm not using metagraphs
	-eb				The Edmonds' implementation in Boost
	-el				The Edmonds' implementation in Lemon (Requires Lemon)
	-biv <E>		Blossom IV implementation. If <E> != 0, the instances will be doubled in size to ensure a perfect matching. (Requires Blossom IV and Concorde-97)
	-bv <E>			Blossom V implementation. If <E> != 0, the instances will be doubled in size to ensure a perfect matching. Behavior for instances without perfect matching is undefined. (Requires Blossom V)
... <SOURCE> is one of the following
	-f <F> <N>		<N>-nearest-neighbor-graphs of the instances in either the file <F> or all files in the directory <F>.
					<F> has to be relative to the assets folder and be a .tsp file or a folder only containing .tsp files. For files, the file extensions is optional.
	-Gr <V> <N>		Creates a random graph using the boost graph library with <V> vertices and <N>*<V> edges.
	-Grd <V> <N>	Creates a random graph using code from the DIMACS challenge with <V> vertices and <N>*<V> edges.
	-Grt <V>		Creates a random delaunay triangulation graph with <V> vertices. (Requires Fade2D)
	-Gre2 <V> <N>	Creates a <N>-nearest-neighbor-graph for a random graph with <V> vertices, that have 2-dimensional coordinates.
	-Gre3 <V> <N>	Creates a <N>-nearest-neighbor-graph for a random graph with <V> vertices, that have 3-dimensional coordinates.
	-Gre10 <V> <N>	Creates a <N>-nearest-neighbor-graph for a random graph with <V> vertices, that have 10-dimensional coordinates.
	-Gre20 <V> <N>	Creates a <N>-nearest-neighbor-graph for a random graph with <V> vertices, that have 20-dimensional coordinates. This does not work due to double precision!
	-Gg <M>			Generates one of Gabows worst-case-graphs with 6*<M> vertices.
	-Gta <N>		Generates a graph that is a series of <N> triangles, where two consecutive triangles (A1,B1,C1) and (A2,B2,C2) are connected with edges (A1,A2), (B1,B2) and (C1,C2)
	-Gtb <N>		Generates a graph that is a series of <N> triangles, where two consecutive triangles (A1,B1,C1) and (A2,B2,C2) are alternating connected with either the edge (A1,A2) or (B1,B2)
	-Ghc <C> <R>	Generates a honey comb graph with <C> columns and <R> rows.
	-Ghcp <C> <R>	Generates a honey comb graph with "fringes" with <C> columns and <R> rows.
	-Ghcc <C> <R>	Generates a honey comb graph with "caps" with <C> columns and <R> rows.
	-Ghci <C> <R>	Generates a honey comb graph with "fringes" and "caps" with <C> columns and <R> rows.

The -o flag will print all generated graphs as .dot and .gml files into the debug directory. This can easily create an immense amount of files, so take care!
	
If neither -s nor -S is set, the input graphs will not be shuffled. -s uses the current time as seed, whilst -S gets a seed provided by the user. -nS specifies a seed without shuffling the instances. Reusing the same seed is guaranteed to result in exactly the same results.

The order of the arguments is entirely arbitrary, as long as all necessary arguments are provided.

All numeral arguments, except for the seed, can also be provided as a range. To enter a range, simply enter the argument either as <MIN>:<MAX> or <MIN>:<MAX>:<STEP>, where <MIN> and <MAX> are the ends of your range and <STEP> denotes how many values in the range shall be used. The default for <STEP> is 1 for every value. The range 4:10 will result in using the values 4, 5, 6, 7, 8, 9 and 10, whereas the range 4:10:3 results in the values 4, 7 and 10. However, if you enter more than one range, every possible combination of values of the ranges will be executed, possibly resulting in a lot more calls (and computation time) than you intended.

Here are some examples of how to call the binary correctly:
	Masterarbeit.exe -f tsp 4:7 -mg -i 100 -S 42
	Masterarbeit.exe -i 1000 -f tsp/d198 6 -mt -s
	Masterarbeit.exe -Gg 10:100:10 -i 100 -eb

The results of an execution will be printed into the measure.csv file, which can be opened with Excel and similar programs.

#####################################################################

4. Note on Gabows worst-case-graphs

To disable the sorting of vertices and edges prior to the execution of the algorithms, you will need to change the source code and recompile it. To do so, open the MetaGraphsSolver.tpp and MultiTreeSolver.tpp and change the value for the preSortStrat in the constructor to "None". This will disable sorting in the algorithms described in the thesis. To disable sorting in the boost implementation of Edmonds algorithm, open EdmondsBoostSolver.tpp and follow the instructions in the comments in the calculateMaxMatching method. Sorting does not need to be disabled for the lemon implementation, since it doesn't use sorting if the number of edges is larger than or equal to twice the number of vertices. The Micali & Vazirani implementation from Github does not use any sorting anyhow.

#####################################################################

5. Remark on buffering files

To decrease the computation time upon testing the same instances over and over again, all base graphs are stored as a .g file in the assets/tmp folder. It has been observed, that during read and write of those files, the order of vertices can vary. Thus it is recommended to always parse files into .g files with a call of the binary with the option -i 1 and then perform benchmarks, since all experiments will use the same base graph across different calls of the binary. If this is unfavored, you might aswell just clear the assets/tmp folder after every call, thus forcing the program to never use the buffer files. Even though this is a code flaw, it did not affect the correctness of the benchmarks, since the workarounds described above do work. It only increases the effort by the user, but since I performed most of my benchmarks using batch files, it did not end up to be a problem big enough for me, that I invested the time necessary to fix this bug, since it does not seem to be an obvious one and I would probably have spend more than a day on it.
	


[1] https://www.boost.org/
[2] http://lemon.cs.elte.hu/trac/lemon
[3] https://www.math.uwaterloo.ca/~bico/blossom4/
[4] http://www.math.uwaterloo.ca/tsp/concorde/downloads/downloads.htm
[5] https://pub.ist.ac.at/~vnk/software.html
[6] https://www.geom.at/products/fade2d/
[7] http://elib.zib.de/pub/mp-testdata/tsp/tsplib/tsp/index.html