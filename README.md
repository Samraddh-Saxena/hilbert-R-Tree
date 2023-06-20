This is a Data Structure Hilbert-R-tree. The file HilbertRtree.c contains the structure. The data used to run thix structure is in s1data1lac.txt file. 

More Details:
1. This program is menu-driven. There are three options in the CLI:
   1 For PreOrder Traversal
   2 For Search
   Any other key to exit

2. In order to accept any other dataset:
   Add the data file in the same folder as the C file
   Change the filename in the #define section in file hilbertRTree.c

3. In order to use the Search functionality, please modify the following line in the file 
   hilbertRTree.c:
   int main() {
     // ...
      Rectangle r = {730000, 750000, 720000, 790000};
     // ...
   }
   The dimensions of the query rectangle can be modified here.