vertex=$1
degree=$2
mkdir ../dataset/kron$vertex-$degree
g++ Generator.cpp -O3 -o Generator
./Generator ../dataset/kron$vertex-$degree/ $vertex $degree 1 0
/home/wzb/bc/GPU-butterfly/preprocess/sort /home/wzb/bc/dataset/kron$vertex-$degree/ 1
