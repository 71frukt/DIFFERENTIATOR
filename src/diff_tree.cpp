#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <assert.h>

#include "diff_tree.h"
#include "operations.h"
#include "diff_debug.h"
#include "diff_graph.h"


void TreeCtor(Tree *tree, size_t start_capacity ON_DIFF_DEBUG(, const char *name))
{ 
    assert(tree);
    assert(start_capacity > 0);

    tree->capacity = 0;
    tree->size     = 0;

    TreeRecalloc(tree, start_capacity);

    ON_DIFF_DEBUG(
        tree->name = name;

        char path[PATH_NAME_LEN] = {};
        GetFilePath(name, LOGS_FOLDER GRAPH_FOLDER, path);

        mkdir(path);
    );

    // DIFF_DUMP(tree);
}

void TreeDtor(Tree *tree)
{
    assert(tree);

    DIFF_DUMP(tree);

    for (size_t i = 0; i < tree->alloc_marks.size; i++)
        free(tree->alloc_marks.data[i]);

    free(tree->node_ptrs);

    tree->capacity = 0;
    tree->size     = 0;

    ON_DIFF_DEBUG( 
        tree->name = NULL;
        // remove(TMP_DOTFILE_NAME);
    );
}

void TreeRecalloc(Tree *tree, size_t new_capacity)
{
    assert(tree);
    assert(new_capacity != 0 && "new_capacity = 0 in TreeRecalloc()");

    size_t prev_capacity = tree->capacity;
    tree->capacity       = new_capacity;

    tree->node_ptrs = (Node **) realloc(tree->node_ptrs, new_capacity * sizeof(Node *));
    Node *new_nodes = (Node *)   calloc(new_capacity - prev_capacity, sizeof(Node));

    assert(tree->node_ptrs);
    assert(new_nodes);

    tree->alloc_marks.data[tree->alloc_marks.size++] = new_nodes;

    for (size_t i = 0; i < new_capacity - prev_capacity; i++)
        tree->node_ptrs[prev_capacity + i] = new_nodes + i;
}

Node *NewNode(Tree *tree, NodeType type, NodeVal val, Node *left, Node *right)
{
    assert(tree);

    if (tree->size >= tree->capacity)
    {
        size_t new_capacity = tree->capacity * 2;
        TreeRecalloc(tree, new_capacity);
    }

    Node *new_node = tree->node_ptrs[tree->size];
    assert(new_node);
    
    new_node->type  = type;
    new_node->val   = val;
    new_node->left  = left;
    new_node->right = right;

    tree->size++;

    return new_node;
}

void RemoveNode(Tree *tree, Node **node)
{
    assert(tree);
    assert(node);
    assert(*node);

    (*node)->left    = NULL;
    (*node)->right   = NULL;
    (*node)->type    = POISON_TYPE;
    (*node)->val.num = 0;
    *node = NULL;
}

void RemoveSubtree(Tree *tree, Node **start_node)
{
    if ((*start_node)->left != NULL)
        RemoveSubtree(tree, &(*start_node)->left);
    if ((*start_node)->right != NULL)
        RemoveSubtree(tree, &(*start_node)->right);

    RemoveNode(tree, start_node);
}

char *NodeValToStr(Node *node, char *res_str)
{
// fprintf(stderr, "called NodeValToStr(), val = %d\n", val);
    assert(node);
    assert(res_str);

    if (node->type == NUM)
        sprintf(res_str, TREE_ELEM_SPECIFIER, node->val.num);
    
    else if (node->type == VAR)
        sprintf(res_str, "%c", node->val.var);

    else if (node->type == OP)
    {
        const Operation *cur_op = GetOperationByNode(node);

        sprintf(res_str, "%s", cur_op->symbol);
    }

    else if (node->type == CHANGE)
        sprintf(res_str, "%c(%d)", node->val.change->name, node->val.change->derivative_num);

    else    // POISON_TYPE
        sprintf(res_str, "%s", POISON_TYPE_MARK);

    return res_str;
}

void NodeValFromStr(char *dest_str, Node *node)
{
    NodeVal val = {};

    if (sscanf(dest_str, TREE_ELEM_SPECIFIER, &val.num) == 1)       // is NUM
    {
        node->type  = NUM;
        node->val   = val;
    }

    else if (strlen(dest_str) == 1 && isalpha(dest_str[0]))         // is VAR
    {
        node->type    = VAR;
        node->val.var = dest_str[0] - 'a';
    }

    else                                                            // is OP
    {
        const Operation *cur_op = GetOperationBySymbol(dest_str);

        if (cur_op != NULL)
        {
            node->type = OP;
            node->val.op = cur_op->num;
        }

        else
        {
            node->type    = POISON_TYPE;
            node->val.num = POISON_VAL;
        }
    }
}

void GetStrTreeData(Node *start_node, char *dest_str)
{
    assert(start_node);

    char node_val_str[LABEL_LENGTH] = {};
    NodeValToStr(start_node, node_val_str);

    if (start_node->type != OP)
        sprintf(dest_str + strlen(dest_str), "%s", node_val_str);

    else
    {
        const Operation *cur_op = GetOperationByNode(start_node);

        if (cur_op->type == BINARY)
        {
            sprintf(dest_str + strlen(dest_str), "(");

            GetStrTreeData(start_node->left, dest_str);

            sprintf(dest_str + strlen(dest_str), " %s ", node_val_str);

            GetStrTreeData(start_node->right, dest_str);

            sprintf(dest_str + strlen(dest_str), ")");
        }

        else
        {
            sprintf(dest_str + strlen(dest_str), "%s", node_val_str);

            sprintf(dest_str + strlen(dest_str), "(");

            GetStrTreeData(start_node->left, dest_str);

            sprintf(dest_str + strlen(dest_str), ") ");
        }
    }
}

void GetTreeFromFile(Tree *tree, const char *source_file_name)
{
    FILE *source_file = fopen(source_file_name, "r");
    tree->root_ptr = GetNodeFamily(tree, source_file);
    fclose(source_file);
}

Node *GetNodeFamily_prefix(Tree *tree, FILE *source_file)
{
    // DIFF_DUMP(tree);

    char node_val_str[LABEL_LENGTH] = {};

    if (fscanf(source_file, "(%[^( )]", node_val_str) == 1)
    {
    // fprintf(stderr, "in cycle, node_val_str = '%s'\n", node_val_str);

        NodeVal val = {.num = POISON_VAL};

        Node *cur_node = NewNode(tree, POISON_TYPE, val, NULL, NULL);
        NodeValFromStr(node_val_str, cur_node);

        if (cur_node->type == OP)
        {

    // fprintf(stderr, "cursor before left = %ld\n", ftell(source_file));
            Node *left  = GetNodeFamily_prefix(tree, source_file);
    // fprintf(stderr, "cursor before right = %ld\n", ftell(source_file));
            Node *right  = GetNodeFamily_prefix(tree, source_file);

            cur_node->left  = left;
            cur_node->right = right;
        }

        getc(source_file);   // съесть ')'

        fprintf(stderr, "end of rec it\n");
        return cur_node;
    }

    else 
    {
        fprintf(stderr, "doshel do ret.\n");
        return NULL;
    }
}

Node *GetNodeFamily(Tree *tree, FILE *source_file)
{
    fscanf(source_file, "%*[ ]");

    if (getc(source_file) == '(')       // это бинарная инфиксная функция
    {
        // Node *op = NewNode(tree, OP, POISON_VAL, NULL, NULL);
// fprintf(stderr, "cursor before left = %ld\n", ftell(source_file));
        Node *left  = GetNodeFamily(tree, source_file);
// fprintf(stderr, "cursor before op = %ld\n", ftell(source_file));
        Node *op    = GetNodeFamily(tree, source_file);
// fprintf(stderr, "cursor before right = %ld\n", ftell(source_file));
        Node *right = GetNodeFamily(tree, source_file);

        op->left  = left;
        op->right = right;

        fscanf(source_file, "%*[ ]");
        getc(source_file);  // съесть ')'
        return op;
    }

    else
    {
        fseek(source_file, -1L, SEEK_CUR);

        char node_val_str[LABEL_LENGTH] = {};
        fscanf(source_file, "%[^( ) ,]", node_val_str);

        const Operation *cur_op = GetOperationBySymbol(node_val_str);

        Node *cur_node = NULL;
        if (cur_op != NULL && cur_op->life_form == PREFIX)     // это префиксная операция
        {
            getc(source_file);  // съесть '('
            
            NodeVal val = {.op = cur_op->num};

            cur_node = NewNode(tree, OP, val, NULL, NULL);

            if (cur_op->type == UNARY)
            {
                Node *arg = GetNodeFamily(tree, source_file);
                cur_node->left  = arg;
                cur_node->right = arg;
            }

            else    // BINARY
            {
                Node *arg_1 = GetNodeFamily(tree, source_file);
                fscanf(source_file, "%*[ ,]");      // съесть ','

                Node *arg_2 = GetNodeFamily(tree, source_file);

                cur_node->left  = arg_1;
                cur_node->right = arg_2;
            }

            getc(source_file);  // съесть ')'
        }

        else    // это VAR, NUM или символ бинарной инфиксной операции
        {
            cur_node = NewNode(tree, POISON_TYPE, {.num = POISON_VAL}, NULL, NULL);
            NodeValFromStr(node_val_str, cur_node);
        }        

        return cur_node;
    }

    DIFF_DUMP(tree);
}

Node *TreeCopyPaste(Tree *source_tree, Tree *dest_tree, Node *coping_node)
{
    Node *pasted_node = NULL;

    if (coping_node->type == NUM || coping_node->type == VAR)
    {
        pasted_node = NewNode(dest_tree, coping_node->type, coping_node->val, NULL, NULL);
    }

    else
    {
        pasted_node = NewNode(dest_tree, OP, coping_node->val, NULL, NULL);

        pasted_node->left  = TreeCopyPaste(source_tree, dest_tree, coping_node->left);
        pasted_node->right = TreeCopyPaste(source_tree, dest_tree, coping_node->right);
    }

    return pasted_node;
}

size_t GetTreeHeight(Node *cur_node)
{
    if (cur_node->type != OP)
        return 1;

    size_t left_height  = GetTreeHeight(cur_node->left);
    size_t right_height = GetTreeHeight(cur_node->right);

    size_t max_height = (left_height > right_height ? left_height : right_height);

    return (max_height + 1);
}

void SplitTree(Tree *tree, Node *cur_node)
{
    if (cur_node->type != OP)
        return;
    
    SplitTree(tree, cur_node->left);
    SplitTree(tree, cur_node->right);

    if (CHECK_NODE_OP(cur_node, ADD) || CHECK_NODE_OP(cur_node, SUB))
        return;

    if (IsSuitableForChange(cur_node, cur_node->left))
        ChangeToVar(tree, cur_node->left);

    if (IsSuitableForChange(cur_node, cur_node->right))
        ChangeToVar(tree, cur_node->right);
}

Node *ChangeToVar(Tree *tree, Node *cur_node)
{
    assert(tree);
    assert(cur_node);
    
    Node *cur_node_cpy = NewNode(tree, {}, {}, NULL, NULL);
    *cur_node_cpy = *cur_node;

    char new_node_name = (char) ('A' + tree->changed_vars.size);

    Change *new_change = &(tree->changed_vars.data[0][tree->changed_vars.size++]);

    new_change->derivative_num = 0;
    new_change->name = new_node_name;
    new_change->target_node = cur_node_cpy;

    // Node *new_var = NewNode(tree, CHANGE, {.change = new_change}, NULL, NULL);
    
    cur_node->type       = CHANGE;
    cur_node->val.change = new_change;
    cur_node->left       = NULL;
    cur_node->right      = NULL;

    return cur_node;
}

bool IsSuitableForChange(Node *op_node, Node *arg)
{
    assert(op_node);
    assert(arg);
    assert(op_node->type == OP);

    if (arg->type != OP || CHECK_NODE_OP(arg, ADD) || CHECK_NODE_OP(arg, SUB) || !SubtreeContainsVar(arg))
        return false;

    if (CHECK_NODE_OP(op_node, ADD) || CHECK_NODE_OP(op_node, SUB) || CHECK_NODE_OP(op_node, MUL) || CHECK_NODE_OP(op_node, DIV))
    {
        if (GetTreeHeight(arg) > MIN_SPLIT_HEIGHT)
            return true;
        
        else
            return false;
    }

    else
        return true;
}

bool SubtreeContainsVar(Node *cur_node)
{
    assert(cur_node);

    if (cur_node == NULL)
        return false;

    else if (cur_node->type == VAR)
        return true;
    
    else if (cur_node->type == NUM || cur_node->type == CHANGE)
        return false;

    else        // if OP
    {
        bool left_subtree_cont_var = SubtreeContainsVar(cur_node->left);

        const Operation *cur_op = GetOperationByNode(cur_node);

        bool right_subtree_cont_var = false;

        if (cur_op->type == BINARY)
            right_subtree_cont_var = SubtreeContainsVar(cur_node->right);

        return (left_subtree_cont_var || right_subtree_cont_var);
    }
}

bool SubtreeContComplicOperation(Node *cur_node)
{
    assert(cur_node);

    if (cur_node->type == VAR || cur_node->type == NUM)
        return false;
    
    else if (cur_node->type == OP && cur_node->val.op != MUL && cur_node->val.op != DIV)        // дроби и произведения выносим как множители
        return true;

    else
    {
        bool left_subtree_cont_complic_op = SubtreeContComplicOperation(cur_node->left);

        const Operation *cur_op = GetOperationByNode(cur_node);

        bool right_subtree_cont_complic_op = false;

        if (cur_op->type == BINARY)
            right_subtree_cont_complic_op = SubtreeContComplicOperation(cur_node->right);

        return (left_subtree_cont_complic_op || right_subtree_cont_complic_op);
    }
}


bool OpNodeIsCommutativity(Node *op_node)
{
    assert(op_node);

    if (op_node->type == OP && (op_node->val.op == ADD || op_node->val.op == MUL))
        return true;
    
    else return false;
}