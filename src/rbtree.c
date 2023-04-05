#include "rbtree.h"

#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

rbtree *new_rbtree(void);
void delete_node(rbtree *t, node_t *x);
void delete_rbtree(rbtree *t);
node_t* rbtree_insert(rbtree* t, const key_t key);
void right_rotate(rbtree* t, node_t* x);
void left_rotate(rbtree* t, node_t* x);
void insert_fixup(rbtree* t, node_t* z);
node_t *rbtree_find(const rbtree *t, const key_t key);
int rbtree_erase(rbtree *t, node_t *z);
void delete_fixup(rbtree *t, node_t *x);
node_t *find_successor(rbtree *t, node_t *x);
void transplant(rbtree* t, node_t* old, node_t* new);
node_t *rbtree_min(const rbtree *t);
node_t *rbtree_max(const rbtree *t);
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n);
int in_order(node_t *x, key_t *arr, int i, const rbtree *t);


rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  node_t *NIL = (node_t *)calloc(1, sizeof(node_t));
  NIL->color = RBTREE_BLACK;
  p->root = NIL;
  p->nil = NIL;
  return p;
}

// inorder-tree-walk DFS로 모든 노드를 삭제
void delete_node(rbtree *t, node_t *x){
  if(x != t->nil){
    delete_node(t, x->left);
    free(x);
    delete_node(t, x->right);
  }  
}

void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  node_t *x;
  x = t->root;    // NIL

  delete_node(t, x);
  free(t->nil);
  free(t);
}

// insert: 핵심은 z.color = RED, while(x != t->nil)에 한해 비교한다는 것. 
node_t* rbtree_insert(rbtree* t, const key_t key)
{
    node_t* z = (node_t*)malloc(sizeof(node_t));  // allocate memory for a new node z
    z->key = key;
    
    node_t* x = t->root;      // x node being compared with z 
    node_t* y = t->nil;       // y will be parent of z (초기화)

    // descend until reaching the sentinel
    while(x != t->nil)
    {
        y = x;
        if (z->key < x->key)
            x = x->left;
        else    
            x = x->right;
    }
    z->parent = y;

    if (y == t->nil)
        t->root = z;
    else if (z->key < y->key)
        y->left = z;
    else
        y->right = z;
    
    z->left = z->right = t->nil; // 리프에 삽입 후 nil 처리 
    z->color = RBTREE_RED;   // red
    insert_fixup(t, z);   // insert_single에선 BLACK으로 바뀜.

    return t->root;
}

// Left rotation of the passed node x
void left_rotate(rbtree* t, node_t* x)
{
    node_t* y = x->right;     // y is x's right Child
    
    // 1. y's left child & x 관계 정의
    x->right = y->left;        // turn y's left subtree into x's right subtree
    if (y->left)           // if y has right subtree
        y->left->parent = x;    // make it's parent to x
    
    // 2. y's parent & x's parent 관계 정의
    //  case 1: x was root
    //  case 2: x was left child
    //  case 3: x was right child
    if (x->parent == t->nil)              // Case 1
        t->root = y;
    else if (x == x->parent->left)  // Case 2
        x->parent->left = y;
    else                    // Case 3
        x->parent->right = y;
    // 3. y & x 관계 정의: x is y's child and y is parent of x
    y->left = x;
    x->parent = y;
}

// Rigth rotation 
void right_rotate(rbtree* t, node_t* x)
{
    node_t* y = x->left;

    // y's subtree
    x->left = y->right;        // 양방향 정의 필요
    if (y->right)           
        y->right->parent = x;

    // y's parent & x's parent
    if (x->parent == t->nil)
        t->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    
    y->right = x;
    x->parent = y;
}

// insertFix
// 핵심은 (속성2)루트를 BLACK으로, (속성4)RED->BLACK을 충족하도록 recolor 혹은 회전시킨다는 것.
// z is red
// Case 1. uncle is red -> recolor z.p and s as black
// Case 2. uncle is black 
//      2-1. z = z.p.r -> transit to Case 3
//      2-2. z = z.p.l -> make z.p black, right-rotate
void insert_fixup(rbtree* t, node_t* z)
{
    // iterate until z's parent color is red
    while (z->parent != t->nil && z->parent->color == RBTREE_RED){
    // z's parent is left child
        if (z->parent == z->parent->left){
            node_t* y = z->parent->right;
            // Case 1. y is red
            if (y->color == RBTREE_RED) {
                z->parent->color = RBTREE_BLACK;
                y->color = RBTREE_BLACK;
                z->parent->color = RBTREE_RED;
                z = z->parent;
            }
            // Case 2, 3: y is black
            else {
                if (z == z->parent->right){
                    z = z->parent;
                    left_rotate(t, z);
                }
                z->parent->color = RBTREE_BLACK;
                z->parent->color = RBTREE_RED;
                right_rotate(t, z->parent);
            }      
        }
    // z's parent is right child
        else {
            node_t* y = z->parent->left;
            // Case 1. y is red
            if (y->color == RBTREE_RED) {
                z->parent->color = RBTREE_BLACK;
                y->color = RBTREE_BLACK;
                z->parent->color = RBTREE_RED;
                z = z->parent;
            }
            // Case 2, 3: y is black
            else {
                if (z == z->parent->left){
                    z = z->parent;
                    right_rotate(t, z);
                z->parent->color = RBTREE_BLACK;
                z->parent->color = RBTREE_RED;
                left_rotate(t, z->parent);
                }
            }
        }
    }
    t->root->color = RBTREE_BLACK;      // property 2. 루트는 black
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  node_t* x = t->root;
  while (x != t->nil){
    if (key == x->key)
      return x;
    else if (key < x->key)
      x = x->left;
    else if (key > x->key)
      x = x->right;
  }
  return NULL;
}

// 삭제: rbtree_erase, transplant, delete_fixup, find_successor, delete_rbtree()
int rbtree_erase(rbtree *t, node_t *z) {
  // delete z 순서: find successor y, replace z with y, fixup after y
  node_t* y;    // z's origin color 저장하기 위해 만든 변수.
  node_t* x;
  color_t y_original_color;

  y = z;
  y_original_color = y->color; 

  if (z->left == t->nil){
    x = z->right;
    transplant(t, z, x);  // z's color is being preserved.
  }
  else if (z->right == t->nil){
    x = z->left;
    transplant(t, z, x);
  }
  else{       // has both child-> y = successor, translate, change color. 
    y = find_successor(t, z); 
    y_original_color = y->color; // 바뀔 노드 색상을 미리 저장.
    
    x = y->right;  // 삭제된 노드 y의 right Child 기준으로 sibling, fixup 진행
    if (y != z->right){
      transplant(t, y, x);
      y->right = z->right;
      y->right->parent = y;
    }
    else
      x->parent = y;
    
    transplant(t, z, y);
    y->left = z->left;
    y->left->parent = y;
    y->color = z->color;
    free(z);
  }
  // 삭제한 노드 y가 black일 때만 y's right Child인 x 기준으로 fixup 진행
  if (y_original_color == RBTREE_BLACK){    
    delete_fixup(t, x);
  }

  return 0;
}

void delete_fixup(rbtree *t, node_t *x){
  node_t *w;
  while (x != t->root && x->color == RBTREE_BLACK){
    if (x == x->parent->left){
      w = x->parent->right;
      // Case 1: w is Red
      if (w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        left_rotate(t, x->parent);
        w = x->parent->right;       // 회전 후 sibling w 재정의
      }   
      // Case 2: w is Black, w's r.C, l.C Both Black
      if (w->left->color == RBTREE_BLACK && w->right->color == RBTREE_BLACK){
        w->color = RBTREE_RED;
        x = x->parent;          // while문으로 x == Black인 경우 fixup 반복 진행
      }

      // Case 3: w is Black, w's l.C is Black
      else{
        if (w->right->color == RBTREE_BLACK){
          w->left->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          right_rotate(t, w);
          w = x->parent->right;
        }
        // Case 3 leads to Case 4
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->right->color = RBTREE_BLACK;
        left_rotate(t, x->parent);
        x = t->root;       // x를 t->root로 초기화하는 이유는, 속성 2- "루트는 블랙"을 부여하기 위해.
      }
    }
    // x was right child
    else{
      w = x->parent->left;
      // Case 1.
      if (w->color == RBTREE_RED){
        w->color = RBTREE_BLACK;
        x->parent->color = RBTREE_RED;
        right_rotate(t, x->parent);
        w = x->parent->left;     
      }
      // Case 2.
      if (w->right->color == RBTREE_BLACK && w->left->color == RBTREE_BLACK){
        w->color = RBTREE_RED;
        x = x->parent;          // while문으로 x == Black인 경우 fixup 반복 진행
      }
      // Case 3. 
      else{
        if (w->left->color == RBTREE_BLACK){
          w->right->color = RBTREE_BLACK;
          w->color = RBTREE_RED;
          left_rotate(t, w);
          w = x->parent->left;
        }
        // Case 3 leads to Case 4
        w->color = x->parent->color;
        x->parent->color = RBTREE_BLACK;
        w->left->color = RBTREE_BLACK;
        right_rotate(t, x->parent);
        x = t->root;
      }
    }
  x->color = RBTREE_BLACK;
  }
}

// finding node x's successor
// x->right Child가 있거나 없거나 둘 중 하나 - return으로 다른 조건 실행 X
node_t *find_successor(rbtree *t, node_t *x){
  if (x->right != t->nil){
    x = x->right;
    while (x->left != t->nil){
      x = x->left;}
    return x;
  }
  node_t *y = x->parent;
  while (y != t->nil && x == y->right){
    x = y;          // x is right Child 일때만 성립
    y = y->parent;
  }
  return y;
}

// linked list로 삭제 효과 가능하나, old 노드에 할당되었던 메모리는 따로 해제해줘야 함.
void transplant(rbtree* t, node_t* old, node_t* new)
{
    if (old->parent == t->nil)
        t->root = new;
    else if (old == old->parent->left)
        old->parent->left = new;
    else
        old->parent->right = new;

    new->parent = old->parent;    // 양방향 설정
}

// 매개변수가 subtree이므로 root부터 접근
node_t *rbtree_min(const rbtree *t){
  node_t *x = t->root;

  if (x = t->nil)
    return NULL;

  while (x->left != t->nil)
    x = x->left;

  return x;
}

node_t *rbtree_max(const rbtree *t){
  node_t *x = t->root;

  if (x = t->nil)
    return NULL;

  while (x->right != t->nil)
    x = x->right;
    
  return x;
}


// rbtree를 순서대로 arr에 저장
int in_order(node_t *x, key_t *arr, int i, const rbtree *t){
  if (x != t->nil){
    i = in_order(x->left, arr, i, t);
    arr[i] = x->key;
    i++;
    i = in_order(x->right, arr, i, t);
  }
  return i;
}

// t->root를 인자로 넘겨 루트부터 순회
int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  int k;
  k = in_order(t->root, arr, 0, t);
  return 0;
}



/*
////////  Test Case  ////////////////////////////////////

// new_rbtree should return rbtree struct with null root node
void test_init(void) {
  rbtree *t = new_rbtree();
  assert(t != NULL);
  assert(t->nil != NULL);
  assert(t->root == t->nil);
  delete_rbtree(t);
}



// root node should have proper values and pointers
void test_insert_single(const key_t key) {
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);
  assert(p != NULL);
  assert(t->root == p);
  assert(p->key == key);
  // assert(p->color == RBTREE_BLACK);  // color of root node should be black
  assert(p->left == t->nil);
  assert(p->right == t->nil);
  assert(p->parent == t->nil);
  delete_rbtree(t);
}


// find should return the node with the key or NULL if no such node exists
void test_find_single(const key_t key, const key_t wrong_key) {
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);

  node_t *q = rbtree_find(t, key);
  assert(q != NULL);
  assert(q->key == key);
  assert(q == p);

  q = rbtree_find(t, wrong_key);
  assert(q == NULL);

  delete_rbtree(t);
}

// erase should delete root node
void test_erase_root(const key_t key) {
  rbtree *t = new_rbtree();
  node_t *p = rbtree_insert(t, key);
  assert(p != NULL);
  assert(t->root == p);
  assert(p->key == key);

  rbtree_erase(t, p);
  assert(t->root == t->nil);
  delete_rbtree(t);
}

static void insert_arr(rbtree *t, const key_t *arr, const size_t n) {
  for (size_t i = 0; i < n; i++) {
    rbtree_insert(t, arr[i]);
  }
}

static int comp(const void *p1, const void *p2) {
  const key_t *e1 = (const key_t *)p1;
  const key_t *e2 = (const key_t *)p2;
  if (*e1 < *e2) {
    return -1;
  } else if (*e1 > *e2) {
    return 1;
  } else {
    return 0;
  }
};

// min/max should return the min/max value of the tree
void test_minmax(key_t *arr, const size_t n) {
  // null array is not allowed
  assert(n > 0 && arr != NULL);

  rbtree *t = new_rbtree();
  assert(t != NULL);

  insert_arr(t, arr, n);
  assert(t->root != NULL);
  assert(t->root != t->nil);

  qsort((void *)arr, n, sizeof(key_t), comp);
  node_t *p = rbtree_min(t);
  assert(p != NULL);
  assert(p->key == arr[0]);

  node_t *q = rbtree_max(t);
  assert(q != NULL);
  assert(q->key == arr[n - 1]);

  rbtree_erase(t, p);
  p = rbtree_min(t);
  assert(p != NULL);
  assert(p->key == arr[1]);

  if (n >= 2) {
    rbtree_erase(t, q);
    q = rbtree_max(t);
    assert(q != NULL);
    assert(q->key == arr[n - 2]);
  }

  delete_rbtree(t);
}

void test_to_array(rbtree *t, const key_t *arr, const size_t n) {
  assert(t != NULL);

  insert_arr(t, arr, n);
  qsort((void *)arr, n, sizeof(key_t), comp);

  key_t *res = calloc(n, sizeof(key_t));
  rbtree_to_array(t, res, n);
  for (int i = 0; i < n; i++) {
    assert(arr[i] == res[i]);
  }
  free(res);
}

void test_multi_instance() {
  rbtree *t1 = new_rbtree();
  assert(t1 != NULL);
  rbtree *t2 = new_rbtree();
  assert(t2 != NULL);

  key_t arr1[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
  const size_t n1 = sizeof(arr1) / sizeof(arr1[0]);
  insert_arr(t1, arr1, n1);
  qsort((void *)arr1, n1, sizeof(key_t), comp);

  key_t arr2[] = {4, 8, 10, 5, 3};
  const size_t n2 = sizeof(arr2) / sizeof(arr2[0]);
  insert_arr(t2, arr2, n2);
  qsort((void *)arr2, n2, sizeof(key_t), comp);

  key_t *res1 = calloc(n1, sizeof(key_t));
  rbtree_to_array(t1, res1, n1);
  for (int i = 0; i < n1; i++) {
    assert(arr1[i] == res1[i]);
  }

  key_t *res2 = calloc(n2, sizeof(key_t));
  rbtree_to_array(t2, res2, n2);
  for (int i = 0; i < n2; i++) {
    assert(arr2[i] == res2[i]);
  }

  free(res2);
  free(res1);
  delete_rbtree(t2);
  delete_rbtree(t1);
}

// Search tree constraint
// The values of left subtree should be less than or equal to the current node
// The values of right subtree should be greater than or equal to the current
// node

static bool search_traverse(const node_t *p, key_t *min, key_t *max,
                            node_t *nil) {
  if (p == nil) {
    return true;
  }

  *min = *max = p->key;

  key_t l_min, l_max, r_min, r_max;
  l_min = l_max = r_min = r_max = p->key;

  const bool lr = search_traverse(p->left, &l_min, &l_max, nil);
  if (!lr || l_max > p->key) {
    return false;
  }
  const bool rr = search_traverse(p->right, &r_min, &r_max, nil);
  if (!rr || r_min < p->key) {
    return false;
  }

  *min = l_min;
  *max = r_max;
  return true;
}

void test_search_constraint(const rbtree *t) {
  assert(t != NULL);
  node_t *p = t->root;
  key_t min, max;
#ifdef SENTINEL
  node_t *nil = t->nil;
#else
  node_t *nil = NULL;
#endif
  assert(search_traverse(p, &min, &max, nil));
}

// Color constraint
// 1. Each node is either red or black. (by definition)
// 2. All NIL nodes are considered black.
// 3. A red node does not have a red child.
// 4. Every path from a given node to any of its descendant NIL nodes goes
// through the same number of black nodes.

bool touch_nil = false;
int max_black_depth = 0;

static void init_color_traverse(void) {
  touch_nil = false;
  max_black_depth = 0;
}

static bool color_traverse(const node_t *p, const color_t parent_color,
                           const int black_depth, node_t *nil) {
  if (p == nil) {
    if (!touch_nil) {
      touch_nil = true;
      max_black_depth = black_depth;
    } else if (black_depth != max_black_depth) {
      return false;
    }
    return true;
  }
  if (parent_color == RBTREE_RED && p->color == RBTREE_RED) {
    return false;
  }
  int next_depth = ((p->color == RBTREE_BLACK) ? 1 : 0) + black_depth;
  return color_traverse(p->left, p->color, next_depth, nil) &&
         color_traverse(p->right, p->color, next_depth, nil);
}

void test_color_constraint(const rbtree *t) {
  assert(t != NULL);
  node_t *nil = t->nil;
  node_t *p = t->root;
  assert(p == nil || p->color == RBTREE_BLACK);

  init_color_traverse();
  assert(color_traverse(p, RBTREE_BLACK, 0, nil));
}

// rbtree should keep search tree and color constraints
void test_rb_constraints(const key_t arr[], const size_t n) {
  rbtree *t = new_rbtree();
  assert(t != NULL);

  insert_arr(t, arr, n);
  assert(t->root != NULL);

  test_color_constraint(t);
  test_search_constraint(t);

  delete_rbtree(t);
}

// rbtree should manage distinct values
void test_distinct_values() {
  const key_t entries[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_rb_constraints(entries, n);
}

// rbtree should manage values with duplicate
void test_duplicate_values() {
  const key_t entries[] = {10, 5, 5, 34, 6, 23, 12, 12, 6, 12};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_rb_constraints(entries, n);
}

void test_minmax_suite() {
  key_t entries[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_minmax(entries, n);
}

void test_to_array_suite() {
  rbtree *t = new_rbtree();
  assert(t != NULL);

  key_t entries[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
  const size_t n = sizeof(entries) / sizeof(entries[0]);
  test_to_array(t, entries, n);

  delete_rbtree(t);
}

void test_find_erase(rbtree *t, const key_t *arr, const size_t n) {
  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_insert(t, arr[i]);
    assert(p != NULL);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_find(t, arr[i]);
    // printf("arr[%d] = %d\n", i, arr[i]);
    assert(p != NULL);
    assert(p->key == arr[i]);
    rbtree_erase(t, p);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_find(t, arr[i]);
    assert(p == NULL);
  }

  for (int i = 0; i < n; i++) {
    node_t *p = rbtree_insert(t, arr[i]);
    assert(p != NULL);
    node_t *q = rbtree_find(t, arr[i]);
    assert(q != NULL);
    assert(q->key == arr[i]);
    assert(p == q);
    rbtree_erase(t, p);
    q = rbtree_find(t, arr[i]);
    assert(q == NULL);
  }
}

void test_find_erase_fixed() {
  const key_t arr[] = {10, 5, 8, 34, 67, 23, 156, 24, 2, 12, 24, 36, 990, 25};
  const size_t n = sizeof(arr) / sizeof(arr[0]);
  rbtree *t = new_rbtree();
  assert(t != NULL);

  test_find_erase(t, arr, n);

  delete_rbtree(t);
}

void test_find_erase_rand(const size_t n, const unsigned int seed) {
  srand(seed);
  rbtree *t = new_rbtree();
  key_t *arr = calloc(n, sizeof(key_t));
  for (int i = 0; i < n; i++) {
    arr[i] = rand();
  }

  test_find_erase(t, arr, n);

  free(arr);
  delete_rbtree(t);
}


int main(void) {
  test_init();
  test_insert_single(1024);
  test_find_single(512, 1024);
  test_erase_root(128);
  test_find_erase_fixed();
  test_minmax_suite();
  test_to_array_suite();
  test_distinct_values();
  test_duplicate_values();
  test_multi_instance();
  test_find_erase_rand(10000, 17);
  printf("Passed all tests!\n");
}


*/