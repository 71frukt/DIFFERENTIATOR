#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "reader.h"
#include "diff_tree.h"
#include "diff_debug.h"
#include "operations.h"

void GetTreeData(Tree *tree, FILE *source)
{
    char str[EXPRESSION_LEN] = {};
    fscanf(source, "%[^\n]", str);

    MakeTokens(tree, str);

    GetExpr(tree);
}

void MakeTokens(Tree *tree, char *str)
{    
    size_t ip = 0;

    while(str[ip] != *END_OF_TRANSLATION)
    {
        size_t old_ip = ip;

        if (isspace(str[ip]))
        {
            ip++;
            continue;
        }

        if (isdigit(str[ip]))
        {
            TreeElem_t val = 0;
            int shift = 0;
            sscanf(str + ip, TREE_ELEM_SCANF_SPECIFIER "%n", &val, &shift);

            ip += shift;

            NewNode(tree, NUM, {.num = val}, NULL, NULL);
        }

        else if (str[ip] == *BRACKET_OPEN)
        {
            ip++;
            NewNode(tree, OP, {.op = OPEN}, NULL, NULL);
        }

        else if (str[ip] == *BRACKET_CLOSE)
        {
            ip++;
            NewNode(tree, OP, {.op = CLOSE}, NULL, NULL);
        }

        else if (str[ip] == *SEPARATOR)
        {
            ip++;
            NewNode(tree, OP, {.op = COMMA}, NULL, NULL);
        }

        else 
        {
            char label[LABEL_LENGTH] = {};
            int shift = 0;
            sscanf(str + ip, "%[^ " BRACKET_OPEN BRACKET_CLOSE SEPARATOR END_OF_TRANSLATION"]%n", label, &shift);
            ip += shift;

            const Operation *cur_op    = GetOperationBySymbol (label);
            const Constant  *cur_const = GetConstantBySymbol  (*label);

            if (cur_op != NULL)
                NewNode(tree, OP, {.op = cur_op->num}, NULL, NULL);
            
            else if (cur_const != NULL)
            {
                fprintf(stderr, "is const in tokenization\n");

                Change *new_change = &(tree->changed_vars.data[tree->changed_vars.size++]);
                Node *constant = NewNode(tree, CHANGE, {}, NULL, NULL);
                *new_change = {.target_node = NewNode(tree, NUM, {.num = cur_const->val}, NULL, NULL), .name = cur_const->sym, .derivative_num = 0};
                constant->val.change = new_change;
            }

            else
                NewNode(tree, VAR, {.var = label[0]}, NULL, NULL);
        }

        if (ip == old_ip)
            SYNTAX_ERROR(ip, "Incorrect syntax");
    }
    
    NewNode(tree, POISON_TYPE, {.var = *END_OF_TRANSLATION}, NULL, NULL);

    DIFF_DUMP(tree);
}

Node *GetExpr(Tree *dest_tree)
{
    size_t ip = 0;

    dest_tree->root_ptr = GetSum(dest_tree, &ip);

    if (dest_tree->node_ptrs[ip]->type != POISON_TYPE)
    {
        DIFF_DUMP(dest_tree);
        SYNTAX_ERROR(ip, "expr->data[expr->ip] != END_OF_TRANSLATION");
    }

    DIFF_DUMP(dest_tree);
    return dest_tree->root_ptr;
}

Node *GetSum(Tree *dest_tree, size_t *ip)
{
    Node **tokens  = dest_tree->node_ptrs;
    Node *res_node = GetMul(dest_tree, ip);

    while (CHECK_NODE_OP(tokens[*ip], ADD) || CHECK_NODE_OP(tokens[*ip], SUB))
    {
        Node *arg_1 = res_node;
        res_node    = tokens[(*ip)++];
        Node *arg_2 = GetMul(dest_tree, ip);

        res_node->left  = arg_1;
        res_node->right = arg_2;
    }

    return res_node;
}

Node *GetMul(Tree *dest_tree, size_t *ip)
{
    Node **tokens  = dest_tree->node_ptrs;
    Node *res_node = GetPow(dest_tree, ip);

    while (CHECK_NODE_OP(tokens[*ip], MUL) || CHECK_NODE_OP(tokens[*ip], DIV))
    {
        Node *arg_1 = res_node;
        res_node    = tokens[(*ip)++];
        Node *arg_2 = GetPow(dest_tree, ip);

        res_node->left  = arg_1;
        res_node->right = arg_2;
    }

    return res_node;
}

Node *GetPow(Tree *dest_tree, size_t *ip)
{
    Node **tokens  = dest_tree->node_ptrs;
    Node *res_node = GetFunc(dest_tree, ip);

    const Operation *cur_op = GetOperationByNode(tokens[*ip]);

    if (cur_op == NULL)
        return res_node;

    if (cur_op->num == DEG)
    {
        Node *basis = res_node;
        res_node = tokens[(*ip)++];
        Node *degree = GetPow(dest_tree, ip);
        
        res_node->left  = basis;
        res_node->right = degree;

        return res_node;
    }

    else
        return res_node;
}

Node *GetFunc(Tree *dest_tree, size_t *ip)
{
    Node **tokens = dest_tree->node_ptrs;

    if (tokens[*ip]->type != OP || IsBracket(tokens[*ip]))
        return GetExprInBrackets(dest_tree, ip);

    else
    {
        Node *op_node = tokens[(*ip)++];
        const Operation *op = GetOperationByNode(op_node);

        if (op == NULL)
            SYNTAX_ERROR(*ip, "unknown operation");

        if (op->life_form == INFIX)
            SYNTAX_ERROR(*ip, "op->life_form == INFIX");

        if (op->type == UNARY)
        {
            Node *arg = GetExprInBrackets(dest_tree, ip);
            op_node->left  = arg;
            op_node->right = arg;

            return op_node;
        }

        else
        {
            if (!CHECK_NODE_OP(tokens[*ip], OPEN))
                SYNTAX_ERROR(*ip, "no open bracket in arg of binary func");
            RemoveNode(dest_tree, &tokens[*ip]);
            (*ip)++;

            Node *arg_1 = GetSum(dest_tree, ip);

            if (!CHECK_NODE_OP(tokens[*ip], COMMA))
                SYNTAX_ERROR(*ip, "no separator in arg of binary func");
            RemoveNode(dest_tree, &tokens[*ip]);
            (*ip)++;

            Node *arg_2 = GetSum(dest_tree, ip);

            if (!CHECK_NODE_OP(tokens[*ip], CLOSE))
                SYNTAX_ERROR(*ip, "no close bracket in arg of binary func");
            RemoveNode(dest_tree, &tokens[*ip]);
            (*ip)++;

            op_node->left  = arg_1;
            op_node->right = arg_2;

            return op_node;
        }        
    }
}

Node *GetExprInBrackets(Tree *dest_tree, size_t *ip)
{
    Node **tokens = dest_tree->node_ptrs;

    if (CHECK_NODE_OP(tokens[*ip], OPEN))
    {
        RemoveNode(dest_tree, &tokens[*ip]);
        (*ip)++;

        Node *res_node = GetSum(dest_tree, ip);

        if (!CHECK_NODE_OP(tokens[*ip], CLOSE))
            SYNTAX_ERROR(*ip, "expr->data[expr->ip] != BRACKET_CLOSE");

        RemoveNode(dest_tree, &tokens[*ip]);
        (*ip)++;
        return res_node;
    }
    
    else
        return GetNumber(dest_tree, ip);
}

Node *GetNumber(Tree *dest_tree, size_t *ip)
{
    Node **tokens = dest_tree->node_ptrs;

    if (tokens[*ip]->type == VAR || tokens[*ip]->type == NUM || tokens[*ip]->type == CHANGE)
    {
        fprintf(stderr, "in getN\n");

        if (tokens[*ip]->type == CHANGE)
        {
            fprintf(stderr, "CHANGE\n");
            Node *change_node = tokens[(*ip)++];
            (*ip) += GetTreeHeight(change_node->val.change->target_node);
            return change_node;   
        }
        
        else
            return tokens[(*ip)++];
    }

    else
    {
        SYNTAX_ERROR(*ip, "incorrect syntax");
        return NULL;
    }
}

void SyntaxError(size_t ip, const char *error, const char *file, int line, const char *func)
{
    fprintf(stderr, "SyntaxError called in %s:%d |%s()|, ip = %lld\nerror: '%s'\n", file, line, func, ip, error);
    abort();
}

char *SkipSpaces(Expression *expr)
{
    while (isspace(expr->data[expr->ip]))
        expr->ip++;

    return expr->data + expr->ip;
}

bool IsBracket(Node *node)
{
    return (CHECK_NODE_OP(node, OPEN) || CHECK_NODE_OP(node, CLOSE));
}