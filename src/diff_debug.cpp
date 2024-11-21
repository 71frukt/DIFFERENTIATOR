#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "diff_tree.h"
#include "diff_debug.h"
#include "diff_graph.h"

FILE *LogFile = OpenLogFile();

FILE *OpenLogFile()
{
    char logfile_path[PATH_NAME_LEN] = {};
    GetFilePath(LOGFILE_NAME, LOGS_FOLDER, logfile_path);

    FILE *logfile = fopen(logfile_path, "w");

    fprintf(logfile, "<html>                                                                                                            \n"
                            "\t<head>                                                                                                   \n"
                            "\t<title>Tree Logs</title>                                                                                 \n"
                            "\t<link rel=\"stylesheet\" href=\"https://maxcdn.bootstrapcdn.com/bootstrap/4.5.0/css/bootstrap.min.css\"> \n"
                            "\t</head>                                                                                                  \n"
                            "\t<body>                                                                                                   \n"
                            "\t<title>Tree Logs</title>                                                                                 \n"
                            "\t<div class=\"jumbotron text-center\">                                                                    \n"
                            "\t\t<h1>List logs</h1>                                                                                     \n"
                            "\t</div>                                                                                                   \n"
                            "\t<pre>                                                                                                    \n"
                        );

    atexit(CloseLogFile);

    return (logfile);
}

void CloseLogFile()
{
    fprintf(LogFile,    "\t\t</pre>     \n"
                        "\t</body       \n"
                        "</html>");
    fclose(LogFile);
}

char *GetFilePath(const char *name, const char *folder, char *path)
{
    sprintf(path, "%s%s", folder, name);
    return path;
}

void DiffDump(Tree *tree, const char *file, int line, const char *func)
{
    assert(tree);
    assert(file);
    assert(func);

    static size_t drawn_graphs_num = 0;

    fprintf(LogFile, "   DIFF_DUMP called from %s:%d  (%s)\n  {\n", file, line, func);

    char picture_path[PATH_NAME_LEN] = {};
    char picture_name[PATH_NAME_LEN] = {};
    sprintf(picture_name, "%s%lld.png", GRAPH_NAME_PREFIX, drawn_graphs_num);

    GetFilePath(picture_name, LOGS_FOLDER GRAPH_FOLDER, picture_path);

    DrawGraph(tree, picture_path);

    fprintf(LogFile, "<img src = %s%s%lld.png width = \"%d%%\" style=\"margin-left: 3%%\">\n", GRAPH_FOLDER, GRAPH_NAME_PREFIX, drawn_graphs_num, GRAPH_IMG_WIDTH);
    // fprintf(stderr,  "<img src = %s%s%d.png width = \"%d%%\" style=\"margin-left: 3%%\">\n", GRAPH_FOLDER, GRAPH_NAME_PREFIX, drawn_graphs_num, GRAPH_IMG_WIDTH);

    fprintf(LogFile, "\n  }\n\n");

    drawn_graphs_num++;
}