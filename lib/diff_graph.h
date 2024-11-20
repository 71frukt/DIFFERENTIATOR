#ifndef DIFF_GRAPH_H
#define DIFF_GRAPH_H

#define TMP_DOTFILE_NAME   "tmp_doc_code.doc"
#define GRAPH_NAME_PREFIX  "graph_"
#define NODE_NAME_PREFIX   "node_"


#define BACKGROUND_COLOR  "#FFF3D1"

const int GRAPH_IMG_WIDTH = 20;     // (%)
const int CMD_COMMAND_LEN = 100;

void  DrawGraph        (Tree *tree, char *dest_picture_path);
void  InitNodesInDot   (Tree *tree, FILE *dot_file);
char *NodeValToStr     (TreeElem_t val, NodeType node_type, char *res_str);
void  MakeLinksInDot   (Tree *tree, FILE *dot_file);
void  MakeGraphPicture (const char *dotfile_path, const char *picture_path);


#endif