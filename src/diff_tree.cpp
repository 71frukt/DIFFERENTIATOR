#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#include "diff_tree.h"
#include "diff_debug.h"
#include "diff_graph.h"

extern FILE *OutputFile;

void TreeCtor(Tree *tree, size_t start_capacity)
{ 
    assert(tree);
    assert(start_capacity > 0);

    tree->capacity = 0;
    tree->size     = 0;

    TreeRecalloc(tree, start_capacity);

    DIFF_DUMP(tree);
}

void TreeDtor(Tree *tree)
{
    assert(tree);

    for (size_t i = 0; i < tree->alloc_marks.size; i++)
        free(tree->alloc_marks.data[i]);

    free(tree->node_ptrs);

    tree->capacity = 0;
    tree->size     = 0;

    ON_DIFF_DEBUG( remove(TMP_DOTFILE_NAME) );
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

Node *NewNode(Tree *tree, NodeType type, TreeElem_t val, Node *left, Node *right)
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
    new_node->value = val;
    new_node->left  = left;
    new_node->right = right;

    tree->root_ptr = new_node;
    tree->size++;

    return new_node;
}

char *NodeValToStr(TreeElem_t val, NodeType node_type, char *res_str)
{
// fprintf(stderr, "called NodeValToStr(), val = %d\n", val);

    assert(res_str);

    if (node_type == NUM)
        sprintf(res_str, TREE_ELEM_SPECIFIER, val);
    
    else if (node_type == VAR)
        sprintf(res_str, "%c", 'a' + val);

    else 
    {
        const Operation *cur_op = GetOperationByNum(val);

        sprintf(res_str, "%s", cur_op->symbol);
    }

    return res_str;
}

void NodeValFromStr(char *dest_str, Node *node)
{
    TreeElem_t val = 0;

    if (sscanf(dest_str, TREE_ELEM_SPECIFIER, &val) == 1)       // is NUM
    {
        node->type  = NUM;
        node->value = val;
    }

    else if (strlen(dest_str) == 1 && isalpha(dest_str[0]))     // is VAR
    {
        node->type = VAR;
        node->value = dest_str[0] - 'a';
    }

    else                                                        // is OP
    {
        node->type = OP;
        
        const Operation *cur_op = GetOperationBySymbol(dest_str);
        node->value = cur_op->num;
    }
}

void GetStrTreeData(Node *start_node, char *dest_str)
{
    char node_val_str[LABEL_LENGTH] = {};
    NodeValToStr(start_node->value, start_node->type, node_val_str);

    if (start_node->type != OP)
        sprintf(dest_str + strlen(dest_str), "%s", node_val_str);

    else
    {
        sprintf(dest_str + strlen(dest_str), "(");

        GetStrTreeData(start_node->left, dest_str);

        sprintf(dest_str + strlen(dest_str), " %s ", node_val_str);

        GetStrTreeData(start_node->right, dest_str);

        sprintf(dest_str + strlen(dest_str), ")");
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

        Node *cur_node = NewNode(tree, POISON_TYPE, POISON_VAL, NULL, NULL);
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

        getc(source_file);   // סתוסעü ')'

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
    // DIFF_DUMP(tree);

    fscanf(source_file, "%*[ ]");

    if (getc(source_file) == '(')
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
        getc(source_file);  // סתוסעü ')'
        return op;
    }

    else        // ‎עמ NUM טכט VAR
    {
        fseek(source_file, -1L, SEEK_CUR);

        char node_val_str[LABEL_LENGTH] = {};

        fscanf(source_file, "%[^( )]", node_val_str);

        Node *cur_node = NewNode(tree, POISON_TYPE, POISON_VAL, NULL, NULL);
        NodeValFromStr(node_val_str, cur_node);

        return cur_node;
    }
}

FILE *GetOutputFile(const int argc, const char *argv[])
{
    if (argc < 2)
        OutputFile = fopen(BASE_OUTPUT_FILE_NAME, "w");
    
    else
        OutputFile = fopen(argv[1], "w");

    // fprintf(OutputFile, "");

    atexit(CloseOutputFile);

    return OutputFile;
}

void CloseOutputFile()
{
    fclose(OutputFile);
}