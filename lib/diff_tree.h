#ifndef DIFF_TREE_H
#define DIFF_TREE_H

#include <stdio.h>

#define DIFF_DEBUG

typedef int TreeElem_t;
#define TREE_ELEM_SPECIFIER  "%d"


const int TREE_ALLOC_MARKS_NUM = 20;         // конечная вместимость labels (START_TREE_SIZE)^20  минимум равна 2^20 = 1 048 576
const int START_TREE_SIZE      = 100;
const int LABEL_LENGTH         = 50;

const TreeElem_t POISON_VAL    = 0xDEB11; 

const char *const LEFT_MARK  = "L";
const char *const RIGHT_MARK = "R";

const char *const BASE_OUTPUT_FILE_NAME = "dest_file.tex";

#include "operations.h"

enum NodeType
{
    POISON_TYPE,
    NUM,
    VAR,
    OP
};

enum Variable
{
    VAR_X = 23,
    VAR_Y = 24,
    VAR_Z = 25
};

struct Node
{
    NodeType type;

    TreeElem_t value;

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

void  TreeCtor        (Tree *tree, size_t start_capacity);
void  TreeDtor        (Tree *tree);
void  TreeRecalloc    (Tree *tree, size_t new_capacity);
Node *NewNode         (Tree *tree, NodeType type, TreeElem_t val, Node *left, Node *right);
char *NodeValToStr    (TreeElem_t val, NodeType node_type, char *res_str);
void  GetStrTreeData  (Node *start_node, char *dest_str);
void  NodeValFromStr  (char *dest_str, Node *node);
void  GetTreeFromFile (Tree *tree, const char *source_file_name);
Node *GetNodeFamily   (Tree *tree, FILE *source_file);
Node *GetNodeFamily_prefix   (Tree *tree, FILE *source_file);

// tex
const char *OperationToTex     (int node_op);
const char *GetTexTreeData     (Node *start_node, char *dest_str, bool need_brackets);
void        ParamsNeedBrackets (Node *op_node, bool *param_1, bool *param_2);

FILE *GetOutputFile   (const int argc, const char *argv[]);
void  CloseOutputFile ();

#endif