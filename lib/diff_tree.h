#ifndef DIFF_TREE_H
#define DIFF_TREE_H

#include <stdio.h>

#define DIFF_DEBUG

typedef int  TreeElem_t;
#define IS_INT_TREE  true
#define TREE_ELEM_SPECIFIER  "%d"


const int TREE_ALLOC_MARKS_NUM        = 20;            // конечная вместимость labels (START_TREE_SIZE)^20  минимум равна 2^20 = 1 048 576
const int START_TREE_SIZE             = 100;
const int LABEL_LENGTH                = 50;
const int MIN_SPLIT_HEIGHT            = 3;
const int CHANGED_VARS_NUM            = 26;            // A-Z
const int CHANGED_VARS_DERIVATIVE_NUM = 4;

const TreeElem_t POISON_VAL    = 0xDEB11;

const char *const LEFT_MARK  = "L";
const char *const RIGHT_MARK = "R";
const char *const POISON_TYPE_MARK = "#POISON";

#ifdef DIFF_DEBUG
#define ON_DIFF_DEBUG(...)  __VA_ARGS__
#else
#define ON_DIFF_DEBUG(...)
#endif

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

struct TreeAllocMarks
{
    Node   *data[TREE_ALLOC_MARKS_NUM];
    size_t  size;
};

struct ChangedVars
{
    Node *data[CHANGED_VARS_DERIVATIVE_NUM][CHANGED_VARS_NUM];
    size_t size;
};


struct Tree
{
    Node **node_ptrs;
    Node *root_ptr;

    size_t capacity;
    size_t size;

    TreeAllocMarks alloc_marks;

    ChangedVars changed_vars;

    ON_DIFF_DEBUG(const char *name);
};

void    TreeCtor        (Tree *tree, size_t start_capacity ON_DIFF_DEBUG(, const char *name));
void    TreeDtor        (Tree *tree);
void    TreeRecalloc    (Tree *tree, size_t new_capacity);
Node   *NewNode         (Tree *tree, NodeType type, TreeElem_t val, Node *left, Node *right);
void    RemoveNode      (Tree *tree, Node **node);
void    RemoveSubtree   (Tree *tree, Node **start_node);
char   *NodeValToStr    (Node *node, char *res_str);
void    GetStrTreeData  (Node *start_node, char *dest_str);
void    NodeValFromStr  (char *dest_str, Node *node);
void    GetTreeFromFile (Tree *tree, const char *source_file_name);
Node   *GetNodeFamily   (Tree *tree, FILE *source_file);
Node   *TreeCopyPaste   (Tree *source_tree, Tree *dest_tree, Node *coping_node);
size_t  GetTreeHeight   (Node *cur_node);
void    SplitTree       (Tree *tree, Node *cur_node);
Node   *ChangeToVar     (Tree *tree, Node *start_node);
Node   *GetNodeFamily_prefix   (Tree *tree, FILE *source_file);

bool    IsTrigonometric    (int op);
bool    SubtreeContainsVar (Node *cur_node);
bool    SubtreeContComplicOperation(Node *cur_node);
bool    OpNodeIsCommutativity(Node *op_node);

#endif