#include <iostream>
#include "cool-tree.h"
#include "stringtab.h"
#include "cool-parse.h"
#include "utilities.h"

/************************************************************************/
/* Definitions and declarations */

extern char *curr_filename;

void yyerror();        /*  defined below; called for each parse error */
void printerr(const char *s);       /*  called for errors in addition to parse errors */
extern int VERBOSE_ERRORS;
int cool_yydebug;

Program ast_root;             /* the result of the parse  */
Classes parse_results;        /* for use in semantic analysis */
int omerrs = 0;               /* number of errors in lexing and parsing */
bool errorstate = false;

int next_token;
bool isfirst = true;

extern YYSTYPE cool_yylval;
extern int curr_lineno;

int lookNextToken();                /*  returns the next token */
void consumeNextToken();            /*  consumes the next token */

extern int cool_yylex();

/************************************************************************/
/* Implementations of utility functions */
void yyerror()
{
  cerr << "\"" << curr_filename << "\", line " << curr_lineno << ": " \
    << "syntax error at or near ";
  print_cool_token(lookNextToken());
  cerr << endl;
  omerrs++;

  if(omerrs>20) {fprintf(stdout, "More than 20 errors\n"); exit(1);}
}

void printerr(const char *s)
{
    if(VERBOSE_ERRORS)
        cerr << "line " << curr_lineno << s << endl;
}

int lookNextToken()
{
    if(isfirst) {
        consumeNextToken();
        isfirst = false;
    }
    return next_token;
}

void consumeNextToken()
{
    next_token = cool_yylex();
}
/************************************************************************/
/************************************************************************/
/* Declarations of recursive descent parser functions */
// ADD YOUR DECLARATIONS HERE
YYSTYPE cool_program();
YYSTYPE cool_class();
YYSTYPE cool_features();

// SIMPLE FUNCTION - You will need to modify this (or remove it) to handle
// errors robustly
YYSTYPE handle_error();
/************************************************************************/

/* The main parse function */
int cool_yyparse()
{
    YYSTYPE pr = cool_program();
    if(!errorstate) {
        ast_root = pr.program;
        return 0;
    }
    else {
        return 1;
    }
    ast_root = pr;
    return ast_root;
}

/* You need to modify this function! */
YYSTYPE handle_error()
{
    yyerror();
    errorstate = true;
    YYSTYPE retval;
    retval.error_msg = "Parser error";
    return retval;
}

YYSTYPE cool_program()
{
    /* Elements for program in AST */
    YYSTYPE retval;
    Classes classes = nil_Classes();

    /* program := [class;]+ */
    do {
        YYSTYPE cl = cool_class();
        if(!errorstate) {
            classes = append_Classes(classes, single_Classes(cl.class_));
        }
        else {
            return cl;
        }

        if(lookNextToken() == ';') {
            consumeNextToken();
        } else {
            return handle_error();
        }
    } while(lookNextToken() != 0);

    parse_results = classes;
    retval.program = program(classes);
    return retval;
}

YYSTYPE cool_class()
{
    /* Elements for class in AST */
    YYSTYPE retval;
    Symbol name;
    Symbol parent;
    Features features;
    Symbol filename;

    /* class := CLASS TYPEID [INHERITS TYPEID] { [features;]* } */
    if(lookNextToken() == CLASS) {
        consumeNextToken();
    } else {
        return handle_error();
    }

    if(lookNextToken() == TYPEID) {
        name = cool_yylval.symbol;
        consumeNextToken();
    } else {
        return handle_error();
    }

    if(lookNextToken() == INHERITS) {
        consumeNextToken();
        if(lookNextToken() == TYPEID) {
            parent = cool_yylval.symbol;
            consumeNextToken();
        } else {
            return handle_error();
        }
    } else {
        parent = idtable.add_string("Object");
    }

    if(lookNextToken() == '{') {
        consumeNextToken();
    } else {
        return handle_error();
    }

    YYSTYPE ft = cool_features();
    if(!errorstate)
        features = ft.features;
    else
        return ft;

    if(lookNextToken() == '}') {
        consumeNextToken();
    } else {
        return handle_error();
    }


    filename = stringtable.add_string(curr_filename);
    retval.class_ = class_(name, parent, features, filename);
    return retval;
}

// YYSTYPE cool_formal()
// {
//   // formal ::=  ID : TYPE
//   YYSTYPE retval;
//
//   //put stuff here
//
//   //ASK SWETA HOW TO RETURN CORRECTLY
//   retval.formals = nil_Features();
//   return retval;
// }
//
//
// YYSTYPE cool_expression()
// {
//   // formal ::=  ID : TYPE
//   YYSTYPE retval;
//
//   //put stuff here
//   //Ask sweta difference of expression and expressions
//   //ASK SWETA HOW TO RETURN CORRECTLY
//   retval.expressions = nil_Features();
//   return retval;
// }

//Gets all feauttures if repeating
YYSTYPE cool_features()
{
    /* Elements for features in AST */
    YYSTYPE retval;
    Features features = nil_Features();

    // feature;*
    do {
        YYSTYPE ft = cool_feature();
        if(!errorstate) {
            features = append_Features(features, single_Features(ft.class_));
        }
        else {
            return ft;
        }

        //ASK SWETA IF THIS PART IS NEEDED
        if(lookNextToken() == ';') {
            consumeNextToken();
        } else {
            return handle_error();
        }
    } while(lookNextToken() != 0);

    //return features as features attribute of YYSTYPE
    retval.features = features;
    return retval;
}

//Gets each invidual feature
YYSTYPE cool_feature()
{
    // feature ::=  ID(formal,*):TYPE { expr }
    //            | ID : TYPE [ <- expr ]
    YYSTYPE retval;
    Symbol identifier;

    Features features = nil_Features();

    if(lookNextToken() == ID) {
        consumeNextToken();
    } else {
        return handle_error();
    }

    // feature ::=  ID(formal,*):TYPE { expr }
    if(lookNextToken() == '(') {
        consumeNextToken();
        if(lookNextToken() == /*Ask how to check for formal*/) {
            consumeNextToken();
            if(lookNextToken() == ')') {
                consumeNextToken();
                if(lookNextToken() == ':') {
                    consumeNextToken();
                    if(lookNextToken() == TYPEID) {
                        consumeNextToken();
                        if(lookNextToken() == '{') {
                            consumeNextToken();
                            YYSTYPE ft = cool_features();
                            if(!errorstate)
                                features = ft.features;
                            else
                                return ft;
                            if(lookNextToken() == '}') {
                                consumeNextToken();
                            } else {
                                return handle_error();
                            }
                        } else {
                            return handle_error();
                        }
                    } else {
                        return handle_error();
                    }
                } else {
                    return handle_error();
                }
            } else {
                return handle_error();
            }
        } else {
            return handle_error();
        }
    } else {
        return handle_error();
    }

    // feature ::=  ID : TYPE [ <- expr ]
    else if(lookNextToken() == ':') {
        consumeNextToken();
        if(lookNextToken() == TYPEID) {
            consumeNextToken();
            if(lookNextToken() == '[') {
                consumeNextToken();
                if(lookNextToken() == DARROW) {
                    consumeNextToken();

                    //ASK IF THIS IS INITIALIZED CORRECTLY
                    YYSTYPE expr = cool_expression();
                    if(!errorstate)
                        expression = expr.expression;
                    else
                        return expr;

                    if(lookNextToken() == ']') {
                        consumeNextToken();
                    } else {
                        return handle_error();
                    }
                } else {
                    return handle_error();
                }
            } else {
                return handle_error();
            }
        } else {
            return handle_error();
        }
    } else {
        return handle_error();
    }

    //ASK SWETA HOW TO RETURN CORRECTLY
    retval.features = feature
    return retval;
}
