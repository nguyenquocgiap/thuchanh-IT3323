/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen (modified)
 * @version 1.1 - completed parser with repeat-until and multi-assignment
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include "token.h"

/* External functions from scanner/reader/error modules */
extern Token* getValidToken(void);
extern void printToken(Token* token);
extern void missingToken(TokenType tokenType, int lineNo, int colNo);
extern int openInputStream(char *fileName);
extern void closeInputStream(void);
extern void error(ErrorType err, int lineNo, int colNo);

Token *currentToken;
Token *lookAhead;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  if (tmp) free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    printToken(lookAhead);
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

/* Forward declarations for helpers used below */
void compileProgram(void);
void compileBlock(void);
void compileBlock2(void);
void compileBlock3(void);
void compileBlock4(void);
void compileBlock5(void);
void compileConstDecls(void);
void compileConstDecl(void);
void compileTypeDecls(void);
void compileTypeDecl(void);
void compileVarDecls(void);
void compileVarDecl(void);
void compileSubDecls(void);
void compileFuncDecl(void);
void compileProcDecl(void);
void compileUnsignedConstant(void);
void compileConstant(void);
void compileConstant2(void);
void compileType(void);
void compileBasicType(void);
void compileParams(void);
void compileParams2(void);
void compileParam(void);
void compileStatements(void);
void compileStatements2(void);
void compileStatement(void);
void compileAssignSt(void);
void compileCallSt(void);
void compileGroupSt(void);
void compileIfSt(void);
void compileElseSt(void);
void compileWhileSt(void);
void compileForSt(void);
void compileRepeatSt(void);
void compileArguments(void);
void compileArguments2(void);
void compileCondition(void);
void compileCondition2(void);
void compileExpression(void);
void compileExpression2(void);
void compileExpression3(void);
void compileTerm(void);
void compileTerm2(void);
void compileFactor(void);
void compileIndexes(void);

/* Implementation */

void compileProgram(void) {
  assert("Parsing a Program ....");
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  assert("Program parsed!");
}

void compileBlock(void) {
  assert("Parsing a Block ....");
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    compileConstDecl();
    compileConstDecls();
    compileBlock2();
  } 
  else compileBlock2();
  assert("Block parsed!");
}

void compileBlock2(void) {
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    compileTypeDecl();
    compileTypeDecls();
    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    compileVarDecl();
    compileVarDecls();
    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

/* Const declarations */

void compileConstDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileConstDecl();
  }
}

void compileConstDecl(void) {
  eat(TK_IDENT);
  eat(SB_EQ);
  compileConstant();
  eat(SB_SEMICOLON);
}

/* Type declarations */

void compileTypeDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileTypeDecl();
  }
}

void compileTypeDecl(void) {
  eat(TK_IDENT);
  eat(SB_EQ);
  compileType();
  eat(SB_SEMICOLON);
}

/* Var declarations */

void compileVarDecls(void) {
  while (lookAhead->tokenType == TK_IDENT) {
    compileVarDecl();
  }
}

void compileVarDecl(void) {
  eat(TK_IDENT);
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    eat(TK_IDENT);
  }
  eat(SB_COLON);
  compileType();
  eat(SB_SEMICOLON);
}

/* Subroutines (function/procedure) */

void compileSubDecls(void) {
  assert("Parsing subtoutines ....");
  while (lookAhead->tokenType == KW_FUNCTION || lookAhead->tokenType == KW_PROCEDURE) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else
      compileProcDecl();
  }
  assert("Subtoutines parsed ....");
}

void compileFuncDecl(void) {
  assert("Parsing a function ....");
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  compileParams();
  eat(SB_COLON);
  compileBasicType();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Function parsed ....");
}

void compileProcDecl(void) {
  assert("Parsing a procedure ....");
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Procedure parsed ....");
}

/* Constants */

void compileUnsignedConstant(void) {
  switch (lookAhead->tokenType) {
    case TK_NUMBER:
      eat(TK_NUMBER);
      break;
    case TK_IDENT:
      eat(TK_IDENT);
      break;
    default:
      missingToken(TK_NUMBER, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

void compileConstant(void) {
  if (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS) {
    eat(lookAhead->tokenType);
  }
  compileUnsignedConstant();
}

void compileConstant2(void) {
  compileConstant();
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    compileConstant();
  }
}

/* Type */

void compileType(void) {
  switch (lookAhead->tokenType) {
    case KW_INTEGER:
    case KW_CHAR:
      compileBasicType();
      break;
    case KW_ARRAY:
      eat(KW_ARRAY);
      eat(SB_LSEL);
      eat(TK_NUMBER);
      eat(SB_RSEL);
      eat(KW_OF);
      compileType();
      break;
    default:
      /* record / ident / ... treat as ident type */
      if (lookAhead->tokenType == TK_IDENT) {
        eat(TK_IDENT);
      } else {
        error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
      }
      break;
  }
}

void compileBasicType(void) {
  if (lookAhead->tokenType == KW_INTEGER) {
    eat(KW_INTEGER);
  } else if (lookAhead->tokenType == KW_CHAR) {
    eat(KW_CHAR);
  } else {
    error(ERR_INVALIDBASICTYPE, lookAhead->lineNo, lookAhead->colNo);
  }
}

/* Parameters */

void compileParams(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileParam();
    while (lookAhead->tokenType == SB_SEMICOLON) {
      eat(SB_SEMICOLON);
      compileParam();
    }
    eat(SB_RPAR);
  }
}

void compileParams2(void) {
  /* Not used in this simple grammar - kept for compatibility */
}

void compileParam(void) {
  /* format: ident {, ident} : basicType */
  eat(TK_IDENT);
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    eat(TK_IDENT);
  }
  eat(SB_COLON);
  compileBasicType();
}

/* Statements */

void compileStatements(void) {
  compileStatement();
  compileStatements2();
}

void compileStatements2(void) {
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    /* allow optional trailing semicolon before end */
    if (lookAhead->tokenType == KW_END) break;
    compileStatement();
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
  case KW_REPEAT:
    compileRepeatSt();
    break;
    /* EmptySt needs to check FOLLOW tokens */
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    /* Error occurs */
  default:
    error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

/* Assignment statement
   Support multi-variable assignment:
   x, y, z := expr1, expr2, expr3
*/
void compileAssignSt(void) {
  assert("Parsing an assign statement ....");
  /* left side: one or more identifiers (with optional indexes) */
  eat(TK_IDENT);
  compileIndexes();
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    eat(TK_IDENT);
    compileIndexes();
  }

  eat(SB_ASSIGN); /* := */

  /* right side: one or more expressions separated by comma */
  compileExpression();
  while (lookAhead->tokenType == SB_COMMA) {
    eat(SB_COMMA);
    compileExpression();
  }

  assert("Assign statement parsed ....");
}

/* Call statement: call ident ( arguments )? */
void compileCallSt(void) {
  assert("Parsing a call statement ....");
  eat(KW_CALL);
  eat(TK_IDENT);
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    if (lookAhead->tokenType != SB_RPAR) {
      compileExpression();
      while (lookAhead->tokenType == SB_COMMA) {
        eat(SB_COMMA);
        compileExpression();
      }
    }
    eat(SB_RPAR);
  }
  assert("Call statement parsed ....");
}

void compileGroupSt(void) {
  assert("Parsing a group statement ....");
  eat(KW_BEGIN);
  if (lookAhead->tokenType != KW_END) {
    compileStatements();
  }
  eat(KW_END);
  assert("Group statement parsed ....");
}

/* If statement */

void compileIfSt(void) {
  assert("Parsing an if statement ....");
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
  assert("If statement parsed ....");
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

/* While statement */

void compileWhileSt(void) {
  assert("Parsing a while statement ....");
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  assert("While statement pased ....");
}

/* For statement (simple version) */

void compileForSt(void) {
  assert("Parsing a for statement ....");
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
  assert("For statement parsed ....");
}

/* Repeat-Until statement (added) */

void compileRepeatSt(void) {
  assert("Parsing a repeat statement ....");
  eat(KW_REPEAT);
  /* repeat body: one or more statements separated by semicolons */
  compileStatements();
  eat(KW_UNTIL);
  compileCondition();
  assert("Repeat statement parsed ....");
}

/* Arguments for call (not used separately) */

void compileArguments(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileExpression();
    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      compileExpression();
    }
    eat(SB_RPAR);
  }
}

void compileArguments2(void) {
  /* left for compatibility */
}

/* Condition */

void compileCondition(void) {
  compileExpression();
  switch (lookAhead->tokenType) {
    case SB_EQ: case SB_NEQ: case SB_LT:
    case SB_LE: case SB_GT: case SB_GE:
      eat(lookAhead->tokenType);
      break;
    default:
      error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
  compileExpression();
}

void compileCondition2(void) {
  compileCondition();
  while (lookAhead->tokenType == KW_AND || lookAhead->tokenType == KW_OR) {
    eat(lookAhead->tokenType);
    compileCondition();
  }
}

/* Expression / Term / Factor */

void compileExpression(void) {
  assert("Parsing an expression");
  if (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS) {
    eat(lookAhead->tokenType);
  }
  compileTerm();
  compileExpression2();
  assert("Expression parsed");
}

void compileExpression2(void) {
  if (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS) {
    eat(lookAhead->tokenType);
    compileTerm();
    compileExpression2();
  }
}

void compileExpression3(void) {
  /* not used in this grammar */
}

void compileTerm(void) {
  compileFactor();
  compileTerm2();
}

void compileTerm2(void) {
  if (lookAhead->tokenType == SB_TIMES || lookAhead->tokenType == SB_SLASH) {
    eat(lookAhead->tokenType);
    compileFactor();
    compileTerm2();
  }
}

void compileFactor(void) {
  switch (lookAhead->tokenType) {
    case TK_NUMBER:
      eat(TK_NUMBER);
      break;
    case TK_CHAR:
      eat(TK_CHAR);
      break;
    case TK_IDENT:
      eat(TK_IDENT);
      compileIndexes();
      break;
    case SB_LPAR:
      eat(SB_LPAR);
      compileExpression();
      eat(SB_RPAR);
      break;
    default:
      error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
      break;
  }
}

/* Indexes: [ number ] ... (array indexing) */

void compileIndexes(void) {
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

/* Entry point */

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  compileProgram();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;
}
