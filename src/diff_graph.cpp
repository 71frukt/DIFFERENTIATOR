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
    assert(tree);
    assert(dot_file);

    for (size_t i = 0; i < tree->size; i++)
    {
        Node *cur_node = tree->node_ptrs[i];

        char node_val_str[LABEL_LENGTH] = {};

        NodeValToStr(cur_node, node_val_str);

        if (cur_node->type == OP)
            fprintf(dot_file, "%s%p [shape = \"%s\", style = filled, fillcolor = \"%s\", label = \"{%s | { <%s> %s | <%s> %s } }\"]\n",
                NODE_NAME_PREFIX, cur_node, OP_NODE_SHAPE, OP_NODE_COLOR, node_val_str, LEFT_MARK, LEFT_MARK, RIGHT_MARK, RIGHT_MARK);

        else if (cur_node->type == VAR)
            fprintf(dot_file, "%s%p [shape = \"%s\", style = filled, fillcolor = \"%s\", label = \"%s\"]\n",
                NODE_NAME_PREFIX, cur_node, VAR_NODE_SHAPE, VAR_NODE_COLOR, node_val_str);

        else if (cur_node->type == NUM)
            fprintf(dot_file, "%s%p [shape = \"%s\", style = filled, fillcolor = \"%s\", label = \"%s\"]\n",
                NODE_NAME_PREFIX, cur_node, NUM_NODE_SHAPE, NUM_NODE_COLOR, node_val_str);
    }
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