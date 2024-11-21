#ifndef DIFF_DEBUG_H
#define DIFF_DEBUG_H

#include "diff_tree.h"

#ifdef DIFF_DEBUG
#define ON_DIFF_DEBUG(...)  __VA_ARGS__
#define DIFF_DUMP(tree)  DiffDump(tree, __FILE__, __LINE__, __func__)

#else
#define ON_DIFF_DEBUG(...)
#define DIFF_DUMP(tree) DiffDump
#endif

#define LOGS_FOLDER      "logs/"
#define GRAPH_FOLDER     "graphs/"

#define LOGFILE_NAME     "tree_logfile.html"
const int PATH_NAME_LEN   = 100;

FILE *OpenLogFile  ();
void  CloseLogFile ();
char *GetFilePath  (const char *name, const char *folder, char *path);
void  DiffDump     (Tree *tree, const char *file, int line, const char *func);

#endif