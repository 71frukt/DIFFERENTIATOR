#ifndef DIFF_GRAPH_H
#define DIFF_GRAPH_H

#define TMP_DOTFILE_NAME   "tmp_doc_code.doc"
#define GRAPH_NAME_PREFIX  "graph_"
#define NODE_NAME_PREFIX   "node_"


#define BACKGROUND_COLOR   "#FFF3D1"

#define VAR_NODE_COLOR     "aquamarine"
#define VAR_NODE_SHAPE     "square"

#define NUM_NODE_COLOR     "pink"
#define NUM_NODE_SHAPE     "oval"

#define OP_NODE_COLOR      "azure"
#define OP_NODE_SHAPE      "record"

#define CHANGE_NODE_COLOR          "aqua"
#define CHANGE_NODE_POINTER_COLOR  "grey"
#define CHANGE_NODE_SHAPE          "tab"


const int GRAPH_IMG_WIDTH = 20;     // (%)
const int CMD_COMMAND_LEN = 100;

void  DrawGraph        (Tree *tree, char *dest_picture_path);
void  InitNodesInDot   (Tree *tree, FILE *dot_file);
void  MakeLinksInDot   (Tree *tree, FILE *dot_file);
void  MakeGraphPicture (const char *dotfile_path, const char *picture_path);


#endif