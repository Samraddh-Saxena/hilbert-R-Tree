// HEADER INCLUSIONS
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#define MAX_ENTRIES 4
#define MIN_ENTRIES 2
#define INT_MIN -2147483648
#define MAX(i, j) (((i) > (j)) ? (i) : (j))
int order; // global variable

// Change the filename here
#define FILENAME "s1data1lac.txt"

// STRUCTURES
typedef struct Point {
    int x;
    int y;
    long long int hilbert;
} Point;

typedef struct Rectangle {
    int lower_x;
    int upper_x;
    int lower_y;
    int upper_y;
} Rectangle;

typedef struct node {
    Rectangle MBR;
    bool is_leaf;
    bool is_root;
    int num_entries;
    long long int LHV;
    Point** entries;
    struct node** sub_nodes;
    struct node* parent;
}Node;

typedef struct HilbertRtree {
    Node* root;
    int max_entries;
} HilbertRtree;

//  DECLARATIONS
long long int getHilbertValue(int x, int y, int n);
Point* createPoint(int x, int y);
Node* createNode(bool is_leaf, int max_entries);
HilbertRtree* createHilbertTree(int max_entries);
void adjustLeafMBR(Node* n);
void adjustSubNodeMBR(Node* n);
void insertIntoSubNode(Node* parent, Node* child, HilbertRtree* h);
Node* chooseLeaf(Point* p, HilbertRtree* H);
void sortBubble(struct Point**list, int s);
void insertIntoLeafNode(Node* n, Point* p,HilbertRtree* h);
void handleSplit(Node* n, Point* p,HilbertRtree* h);
void insertIntoLeafNode(Node* n, Point* p,HilbertRtree* h);
void traverseHilbertTree(Node* node, int l);
void insertIntoTree(HilbertRtree* h, Point* p);
int orderCalc(HilbertRtree* h);
void readDataIntoHBT(HilbertRtree* h);
bool overlaps(Rectangle r1, Rectangle r2);
bool contains(Point* p, Rectangle r);
void searchHTree(Node* n, Rectangle r);
void adjustTree(Node* n);

// DRIVER CODE
int main(){
    HilbertRtree* h = createHilbertTree(MAX_ENTRIES);
    order= orderCalc(h);
    readDataIntoHBT(h);
    Rectangle r = {730000, 750000, 720000, 790000};

    int choice;
    while (true)
    {
      printf("\n");
      printf("===============================================================\n");
      printf("\n");

      printf("Select 1 for PreOrder Traversal, 2 for Search, Any Key for Exit\n");
      printf("Enter your choice :  ");
      scanf("%d",&choice);

      switch (choice)
      {
      case 1:
        traverseHilbertTree(h->root, 0);
        break;

      case 2:
        searchHTree(h->root,r);
        break;
      
      default:
        printf("Exiting...\n");
        exit(0);
      }
    }
    return 0;
}


// GET HILBERT VALUE FOR A POINT
long long int getHilbertValue(int x, int y, int order) {
    long long int hilbertValue = 0;
    long long int sideLength = pow(2, order);

    for (unsigned long long int s = sideLength; s > 0; s /= 2) {
        long long int xBit = (x & s) > 0;
        long long int yBit = (y & s) > 0;

        // Compute the Hilbert value increment for this iteration
        hilbertValue += s * s * ((3 * xBit) ^ yBit);

        // Perform the rotation and reflection transformation
        if (yBit == 0) {
            if (xBit == 1) {
                x = (1 << order) - 1 - x;
                y = (1 << order) - 1 - y;
            }
            int temp = x;
            x = y;
            y = temp;
        }
    }
    return hilbertValue;
}


// CREATE POINT + HILBERT TREE + NODE FUNC

Point* createPoint(int x, int y)
{   
    Point * p =(Point*)malloc(sizeof(Point));
    p->x=x;
    p->y=y;
    p->hilbert=getHilbertValue(p->x,p->y,order);
    return p;
} 

Node* createNode(bool is_leaf, int max_entries) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->is_leaf = is_leaf;
    node->is_root=false;
    node->num_entries = 0;
    node->entries = (Point**)malloc(max_entries * sizeof(Point*));
    node->sub_nodes= (Node**)malloc(max_entries*sizeof(Node));
    node->parent = NULL;
    node->MBR.lower_x=-0;
    node->MBR.lower_y=-1;
    node->MBR.upper_x= -1;
    node->MBR.upper_y= 0;
    return node;
}

HilbertRtree* createHilbertTree(int max_entries) {
    HilbertRtree* tree = (HilbertRtree*)malloc(sizeof(HilbertRtree));
    tree->max_entries = max_entries;
    tree->root = NULL;
    return tree;
}

// ADJUST LEAF MBR AND SUBNODE MBR

void adjustLeafMBR(Node* n)
{   

    int low_x, low_y, high_x, high_y;
    low_x=n->entries[0]->x;
    low_y=n->entries[0]->y;
    high_x=n->entries[0]->x;
    high_y=n->entries[0]->y;
    for(int i=0; i<n->num_entries;i++)
    {
        if(n->entries[i]->x>high_x)
        {
            high_x = n->entries[i]->x;
        }
        if(n->entries[i]->y>high_y)
        {
            high_y = n->entries[i]->y;
        }
        if(n->entries[i]->x<low_x)
        {
            low_x = n->entries[i]->x;
        }
        if(n->entries[i]->y<low_y)
        {
            low_y = n->entries[i]->y;
        }
    }
    n->MBR.lower_x=low_x;
    n->MBR.lower_y=low_y;
    n->MBR.upper_x=high_x;
    n->MBR.upper_y=high_y;
    long long int lhv = -1;
    for(int i=0;i<n->num_entries;i++)
    {
    if(lhv<n->entries[i]->hilbert)
    lhv=n->entries[i]->hilbert;    
    }
    n->LHV=lhv;

}

void adjustSubNodeMBR(Node* n)
{   
    if(n->is_root && n->is_leaf)
    {
    adjustLeafMBR(n);
    return;
    }
    int low_x, low_y, high_x, high_y;
    low_x = n->sub_nodes[0]->MBR.lower_x;
    low_y=n->sub_nodes[0]->MBR.lower_y;
    high_x=n->sub_nodes[0]->MBR.upper_x;
    high_y=n->sub_nodes[0]->MBR.upper_y;
    for(int i=0; i<n->num_entries;i++)
    {   Node* n1 = n->sub_nodes[i];
        if(n->sub_nodes[i]->MBR.upper_x>high_x)
        {
            high_x = n->sub_nodes[i]->MBR.upper_x;
        }
        if(n->sub_nodes[i]->MBR.upper_y>high_y)
        {
            high_y = n->sub_nodes[i]->MBR.upper_y;
        }
        if(n->sub_nodes[i]->MBR.lower_x<low_x)
        {
            low_x = n->sub_nodes[i]->MBR.lower_x;
        }
        if(n->sub_nodes[i]->MBR.lower_y<low_y)
        {
            low_y = n->sub_nodes[i]->MBR.lower_y;
        }
    }
    n->MBR.lower_x=low_x;
    n->MBR.lower_y=low_y;
    n->MBR.upper_x=high_x;
    n->MBR.upper_y=high_y;
    long long int lhv=-1;
    for(int i=0;i<n->num_entries;i++)
    {
        if(lhv<n->sub_nodes[i]->LHV)
        lhv=n->sub_nodes[i]->LHV;
    }
    n->LHV=lhv;
  }
// INSERT INTO SUBNODE

void insertIntoSubNode(Node* parent, Node* child, HilbertRtree* h)
{
if(child->LHV>parent->LHV)
parent->LHV=child->LHV;
parent->is_leaf=false;
child->parent=parent;
if(parent->num_entries<4)
{
parent->sub_nodes[parent->num_entries]=child;
parent->num_entries++;
adjustSubNodeMBR(parent);
}
else
{
Node* new = createNode(false,4);
for(int i=2;i<4;i++)
    {
    insertIntoSubNode(new,parent->sub_nodes[i],h);
    parent->entries[i]=NULL;
    }  
    adjustSubNodeMBR(new);
    parent->num_entries-=2;
    adjustSubNodeMBR(parent);    
    insertIntoSubNode(parent,child,h);
    if(parent->parent==NULL)
    {
    parent->is_root=false;
    Node* new_root = createNode(false,4);
    new_root->is_root=true;
    //parent->parent->is_root=false;
    h->root= new_root;
    insertIntoSubNode(new_root,parent,h);
    insertIntoSubNode(new_root,new,h);
    adjustSubNodeMBR(h->root);
    }
    else
    {
    insertIntoSubNode(parent->parent,new,h);
    adjustSubNodeMBR(parent->parent);
    }
    }
    if(parent->MBR.upper_x<parent->MBR.lower_x)
    {
    parent->MBR.upper_x=child->MBR.upper_x; 
    parent->MBR.upper_y=child->MBR.upper_y;
    parent->MBR.lower_x=child->MBR.lower_x;
    parent->MBR.lower_y=child->MBR.lower_y;
    }
    if(child->MBR.upper_x > parent->MBR.upper_x)
    parent->MBR.upper_x = child->MBR.upper_x;
    if(child->MBR.lower_x < parent->MBR.lower_x)
    parent->MBR.lower_x = child->MBR.lower_x;
    if(child->MBR.upper_y > parent->MBR.upper_y)
    parent->MBR.upper_y = child->MBR.upper_y;
    if(child->MBR.lower_y < parent->MBR.lower_y)
    parent->MBR.lower_y = child->MBR.lower_y; 

    //adjustSubNodeMBR(parent->parent);

}

// UTILITY FUNCTIONS : CHOOSING A LEAF, SORTING

Node* chooseLeaf(Point* p, HilbertRtree* H)
{ // returns the leaf node in which the point should be inserted
  Node* n = H->root;
  while(!n->is_leaf)
  {
    int i;
    for(i=0;i<n->num_entries;i++)
    { 
      // if the hilbert value of the point is less than the hilbert value of the node, then break
      if(n->sub_nodes[i]->LHV > p->hilbert) 
      break;   
    }
    if(i==n->num_entries)
      i--;
    n=n->sub_nodes[i];
  }
  return n;
}

void sortBubble(struct Point**list, int s)
{
    int i, j;
    struct Point*temp;
    
    for (i = 0; i < s - 1; i++)
    {
        for (j = 0; j < (s - 1-i); j++)
        {
            if (list[j]->hilbert > list[j + 1]->hilbert)
            {
                temp = list[j];
                list[j] = list[j + 1];
                list[j + 1] = temp;
            } 
        }
    }
}

// HANDLING OVERFLOW

void handleSplit(Node* n, Point* p,HilbertRtree* h)
{
  
  Node* new_root = createNode(true,4);
  new_root->is_root=false;
  for(int i=MIN_ENTRIES;i<MAX_ENTRIES;i++)
  {
    insertIntoLeafNode(new_root,n->entries[i],h);
    n->entries[i]=NULL;
  }
  if(n->is_root==true)
  {
    n->is_root=false;
    h->root = createNode(false,4);
    h->root->is_root=true;
    insertIntoSubNode(h->root,n,h);
    n->parent=h->root;
    adjustSubNodeMBR(h->root);
  }
  new_root->parent=n->parent; //uncommented
  insertIntoSubNode(n->parent,new_root,h);
  n->num_entries-=2;
  insertIntoLeafNode(n,p,h);
  adjustLeafMBR(n);
  adjustLeafMBR(new_root); 
}

// INSERTING INTO LEAF NODE

void insertIntoLeafNode(Node* n, Point* p,HilbertRtree* h)
{
  if(n->num_entries>3)
  {   
    handleSplit(n,p,h);      
    return; // handle split
  }
  else
  {
      n->entries[n->num_entries]=p;
      n->num_entries++;
      if(n->MBR.upper_x<n->MBR.lower_x)
      {
        n->MBR.upper_x=p->x; 
        n->MBR.lower_x=p->x; 
        n->MBR.upper_y=p->y; 
        n->MBR.lower_y=p->y;
      }
      if(p->x>n->MBR.upper_x)
      n->MBR.upper_x=p->x;
      if(p->x<n->MBR.lower_x)
      n->MBR.lower_x=p->x;
      if(p->y>n->MBR.upper_y)
      n->MBR.upper_y=p->y;
      if(p->y<n->MBR.lower_y)
      n->MBR.lower_y=p->y;   
      sortBubble(n->entries,n->num_entries);
  }
}

void insertIntoTree(HilbertRtree* h, Point* p)
{
  if(h->root==NULL)
  { // inserts a blank point in the tree
    h->root=createNode(true,4);
    h->root->is_root=true;
    h->root->is_leaf=true;  // added later
  }
  Node* n = chooseLeaf(p,h);
  insertIntoLeafNode(n,p,h);
  adjustTree(n);
}


// INORDER TRAVERSING THE TREE
void print_gap(int l)
{
    for(int i=0;i<l;i++)
    printf("| \t|");
}

void traverseHilbertTree(Node* node, int l) {
    if (node == NULL) {
        return;
    }
    if (node->is_leaf) {
        print_gap(l);
        printf("\t \t    leaf_node\n");
        print_gap(l);
        printf("\t \t    MBR: (%d,%d)     (%d,%d) LHV = %lld \n",node->MBR.lower_x,node->MBR.upper_y,node->MBR.upper_x,node->MBR.upper_y,node->LHV);
        print_gap(l);
        printf("\t \t    MBR: (%d,%d)     (%d,%d) \n",node->MBR.lower_x,node->MBR.lower_y,node->MBR.upper_x,node->MBR.lower_y);
        
        for (int i = 0; i < node->num_entries; i++) {
            print_gap(l);
            printf("\t \t          (%d, %d)\n", node->entries[i]->x, node->entries[i]->y);
        }
    } else {
        if(node->is_root && !node->is_leaf)
        {
          printf("ROOT \n");       
        }
        else if(!node->is_leaf && !node->is_root)
        {
          print_gap(l);
          printf("\t SUBNODES \n");
        }
        print_gap(l);        
        printf("\t    MBR: (%d,%d)     (%d,%d) LHV = %lld \n",node->MBR.lower_x,node->MBR.upper_y,node->MBR.upper_x,node->MBR.upper_y,node->LHV);
        print_gap(l);
        printf("\t    MBR: (%d,%d)     (%d,%d) \n",node->MBR.lower_x,node->MBR.lower_y,node->MBR.upper_x,node->MBR.lower_y);
        l++;
        for (int i = 0; i < node->num_entries; i++) {
            traverseHilbertTree(node->sub_nodes[i],l);
        }
    }
}

// order calculation
int orderCalc(HilbertRtree* h){
    FILE* fp = fopen(FILENAME,"r");
    int x,y;
    float maxi=INT_MIN;
    while(fscanf(fp,"%d %d",&x,&y) != EOF){
        if(maxi < MAX(x,y)){
        maxi=MAX(x,y);}
    }
    return (int)log2(maxi)+1;
}

// INPUT FROM FILE
void readDataIntoHBT(HilbertRtree* h){
    FILE* fp = fopen(FILENAME,"r");
    int x,y;
    while(fscanf(fp,"%d %d",&x,&y) != EOF){
        Point* p = createPoint(x,y);
        insertIntoTree(h,p);
    }
}

// SEARCHING THE TREEs
bool overlaps(Rectangle r1, Rectangle r2) {
    if (r1.lower_x > r2.upper_x || r2.lower_x > r1.upper_x) {
        return false; // r1 and r2 don't overlap on the x-axis
    }
    if (r1.lower_y > r2.upper_y || r2.lower_y > r1.upper_y) {
        return false; // r1 and r2 don't overlap on the y-axis
    }
    return true; // r1 and r2 overlap on both the x-axis and the y-axis
}

bool contains(Point* p, Rectangle r)
{
    if(p->x<=r.upper_x && p->x>=r.lower_x)
        {
        if(p->y<=r.upper_y && p->y>=r.lower_y)
            return true;
        }       
    return false;
}

void searchHTree(Node* n, Rectangle r)
{
  if(!overlaps(n->MBR,r))
    return;
  if(!n->is_leaf)
    {
      for(int i=0; i<n->num_entries;i++)
      {
          if(overlaps(n->sub_nodes[i]->MBR,r))
          searchHTree(n->sub_nodes[i],r);
      }
    }
    else
    {
      for(int i= 0; i<n->num_entries;i++)
      {
          Point* p = n->entries[i]; 
          if(contains(n->entries[i],r))
              printf("(%d,%d) ",n->entries[i]->x,n->entries[i]->y);

      }
    }
}

void adjustTree(Node* n)
{
  while(!n->is_root)
  {
      if(n->is_leaf)
      adjustLeafMBR(n);
      else
      adjustSubNodeMBR(n);
      n=n->parent;
  }
    adjustSubNodeMBR(n);
}