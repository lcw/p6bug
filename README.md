This is an example of computing the global node number of cubesphere elements
using `p6est`.  Each rank dumps the global node numbers for its local elements
to a file.

## Build and run

This build assumes you have [GNU Make][1].  To build the code you can run
something like:
```sh
make -j
```

The code can be run in parallel simply with
```sh
mpirun -np 2 ./p6bug
```
The global node numbers are written out `rank_*.txt` files.

## Possible Bug

If the code is run on 1 rank the global numbering is different than on 2 ranks.
See for example:

```sh
mpirun -np 1 ./p6bug
mv rank_00.txt onerank.txt
mpirun -np 2 ./p6bug
cat rank_*.txt > tworanks.txt
diff -u onerank.txt tworanks.txt
```

which gives the difference

```diff
--- onerank.txt 2019-08-07 17:00:47.000000000 -0700
+++ tworanks.txt        2019-08-07 17:00:47.000000000 -0700
@@ -2,5 +2,5 @@
   2  3  8  9  6  7 10 11
   0  1 12 13  2  3  8  9
  12 13 14 15  8  9 10 11
-  0  1  4  5 12 13 14 15
+  2  3  4  5 12 13 14 15
   4  5  6  7 14 15 10 11
```

[1]: https://www.gnu.org/software/make
