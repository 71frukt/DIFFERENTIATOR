#ifndef GNUPLOT_H
#define GNUPLOT_H

#define CHART_NAME            "chart.png"
#define TMP_SCRIPT_FILE_NAME  "tmp_gnuplot_script.gpi"

#define EXPR_COLOR        "green"
#define TAILOR_COLOR      "red"
#define DIFFERENCE_COLOR  "blue"

const int X_RANGE = 1;
const int Y_RANGE = 3;

void MakeTailorChart(Tree *expr_tree, Tree *tailor_tree, const char *chart_name);

#endif