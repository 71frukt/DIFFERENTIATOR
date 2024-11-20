#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "diff_tree.h"
#include "diff_graph.h"

void DrawGraph(Tree *tree, char *dest_picture_path)
{
    assert(tree);
    assert(dest_picture_path);

    FILE *dot_file = fopen(TMP_DOTFILE_NAME, "w");

    fprintf(dot_file, "digraph G{           \n"   
                        "bgcolor = \"%s\";  \n" , BACKGROUND_COLOR);

    InitNodesInDot(tree, dot_file);

    MakeLinksInDot(tree, dot_file);

    fprintf(dot_file, "} \n");

    fclose(dot_file);

    MakeGraphPicture(TMP_DOTFILE_NAME, dest_picture_path);

    fclose(dot_file);
}

void InitNodesInDot(Tree *tree, FILE *dot_file)
{
    for (size_t i = 0; i < tree->size; i++)
    {
        Node *cur_node = tree->node_ptrs[i];

        char node_val_str[LABEL_LENGTH] = {};
        NodeValToStr(cur_node->value, cur_node->type, node_val_str);

        fprintf(dot_file, "%s%p [shape = \"record\", label = \"{%s | <%s> %s | <%s> %s } }\"]\n",
            NODE_NAME_PREFIX, cur_node, node_val_str, LEFT_MARK, LEFT_MARK, LEFT_MARK, LEFT_MARK);
    }
}

char *NodeValToStr(TreeElem_t val, NodeType node_type, char *res_str)
{
    assert(res_str);

    if (node_type == NUM)
        sprintf(res_str, TREE_ELEM_SPECIFIER, val);
    
    else if (node_type == VAR)                          
        sprintf(res_str, "%c", 'x' + val);

    else 
    {
        char operation_ch = 0;

        switch(val)
        {
            case ADD:
                operation_ch = ADD_MARK;
                break;
            
            case SUB:
                operation_ch = SUB_MARK;
                break;

            case MUL:
                operation_ch = MUL_MARK;
                break;

            case DIV:
                operation_ch = DIV_MARK;
                break;

            default:
                fprintf(stderr, "unknown operation numbered %d in NodeValToStr()\n", val);
                break; 
        }

        sprintf(res_str, "%c", operation_ch);
    }

    return res_str;
}

void MakeLinksInDot(Tree *tree, FILE *dot_file)
{
    assert(tree);
    assert(dot_file);

    for (size_t i = 0; i < tree->size; i++)
    {
        Node *cur_node  = tree->node_ptrs[i];
        Node *left_son  = cur_node->left;
        Node *right_son = cur_node->right;

        if (left_son  != NULL)
            fprintf(dot_file, "%s%p: <%s> -> %s%p\n", NODE_NAME_PREFIX, cur_node, LEFT_MARK,  NODE_NAME_PREFIX, left_son);

        if (right_son != NULL)
            fprintf(dot_file, "%s%p: <%s> -> %s%p\n", NODE_NAME_PREFIX, cur_node, RIGHT_MARK, NODE_NAME_PREFIX, right_son);
    }
}

void MakeGraphPicture(const char *dotfile_path, const char *picture_path)
{
    assert(dotfile_path);
    assert(picture_path);

    char cmd_command[CMD_COMMAND_LEN] = {};

    sprintf(cmd_command, "dot %s -T png -o %s\n", dotfile_path, picture_path);
// fprintf(stderr, "command: %s\n", cmd_command);
    system(cmd_command);
}