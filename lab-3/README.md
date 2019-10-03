# Matrice Multiplication
## 1. P2P Method
Compile: `mpicc -std=c99 1-p2p-matmul.c -o ./p2p`

Run: `mpirun -np $NPROCS ./p2p` with `$NPROC` is number of processors.

Result will writing into file **p2p.matR**
## 2. Collective Method
Compile: `mpicc -std=c99 2-collective-matmul.c -o ./collective`

Run: `mpirun -np $NPROCS ./collective` with `$NPROC` is number of processors.

Result will writing into file **collective.matR**