#ifndef DIFF_TREE_H
#define DIFF_TREE_H

#include <stdio.h>

#define DIFF_DEBUG

typedef int TreeElem_t;
#define TREE_ELEM_SPECIFIER  "%d"


const int TREE_ALLOC_MARKS_NUM = 20;         // конечная вместимость labels (START_DATATREE_SIZE)^20  минимум равна 2^20 = 1 048 576
const int START_DATATREE_SIZE  = 100;
const int LABEL_LENGTH         = 50;

const char *const LEFT_MARK  = "L";
const char *const RIGHT_MARK = "R";

#define ADD_MARK  '+'
#define SUB_MARK  '-'
#define MUL_MARK  '*'
#define DIV_MARK  '/'

enum Operation
{
    ADD,
    SUB,
    MUL,
    DIV
};

enum NodeType
{
    NUM,
    VAR,
    OP
};

enum Variable
{
    VAR_X,
    VAR_Y,
    VAR_Z
};

struct Node
{
    NodeType type;

    TreeElem_t value;

    Node *parent;
    Node *left;
    Node *right;
};

struct tree_alloc_marks_t
{
    Node   *data[TREE_ALLOC_MARKS_NUM];
    size_t  size;
};

struct Tree
{
    Node **node_ptrs;
    Node *root_ptr;

    size_t capacity;
    size_t size;

    tree_alloc_marks_t alloc_marks;
};

void TreeCtor(Tree *tree, size_t start_capacity);
void TreeDtor(Tree *tree);
void TreeRecalloc(Tree *tree, size_t new_capacity);

#endif