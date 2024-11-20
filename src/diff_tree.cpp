#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "diff_tree.h"
#include "diff_debug.h"
#include "diff_graph.h"

void TreeCtor(Tree *tree, size_t start_capacity)
{ 
    assert(tree);

    tree->capacity = 0;
    tree->size     = 0;

    TreeRecalloc(tree, start_capacity);

    TREE_DUMP(tree);
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