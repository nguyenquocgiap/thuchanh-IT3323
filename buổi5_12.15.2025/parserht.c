/*
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"
#include "debug.h"
#include "symtab.h"
#include "object.h"
#include "type.h"

Token *currentToken;
Token *lookAhead;

extern Type* intType;
extern Type* charType;
extern SymTab* symtab;

/* ===================== TOKEN HANDLING ===================== */

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  if (tmp != NULL) free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

/* ===================== PROGRAM ===================== */

void compileProgram(void) {
  Object* program;

  eat(KW_PROGRAM);
  eat(TK_IDENT);

  program = createObject(currentToken->string, OBJ_PROGRAM);
  declareObject(program);
  enterBlock(program);

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);

  exitBlock();
}

/* ===================== BLOCK ===================== */

void compileBlock(void) {
  Object* obj;

  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    do {
      eat(TK_IDENT);
      obj = createObject(currentToken->string, OBJ_CONSTANT);
      eat(SB_EQ);
      obj->constValue = *compileConstant();
      declareObject(obj);
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);
  }
  compileBlock2();
}

void compileBlock2(void) {
  Object* obj;

  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    do {
      eat(TK_IDENT);
      obj = createObject(currentToken->string, OBJ_TYPE);
      eat(SB_EQ);
      obj->type = compileType();
      declareObject(obj);
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);
  }
  compileBlock3();
}

void compileBlock3(void) {
  Object* obj;

  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    do {
      eat(TK_IDENT);
      obj = createObject(currentToken->string, OBJ_VARIABLE);
      eat(SB_COLON);
      obj->type = compileType();
      declareObject(obj);
      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);
  }
  compileBlock4();
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

/* ===================== SUB DECL ===================== */

void compileSubDecls(void) {
  while (lookAhead->tokenType == KW_FUNCTION ||
         lookAhead->tokenType == KW_PROCEDURE) {
    if (lookAhead->tokenType == KW_FUNCTION)
      compileFuncDecl();
    else
      compileProcDecl();
  }
}

void compileFuncDecl(void) {
  Object* func;

  eat(KW_FUNCTION);
  eat(TK_IDENT);
  func = createObject(currentToken->string, OBJ_FUNCTION);
  declareObject(func);
  enterBlock(func);

  compileParams();
  eat(SB_COLON);
  func->type = compileBasicType();
  eat(SB_SEMICOLON);

  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

void compileProcDecl(void) {
  Object* proc;

  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  proc = createObject(currentToken->string, OBJ_PROCEDURE);
  declareObject(proc);
  enterBlock(proc);

  compileParams();
  eat(SB_SEMICOLON);

  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

/* ===================== CONSTANT ===================== */

ConstantValue* compileUnsignedConstant(void) {
  ConstantValue* v = (ConstantValue*)malloc(sizeof(ConstantValue));

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    v->intValue = atoi(currentToken->string);
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    v->charValue = currentToken->string[0];
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
  }
  return v;
}

ConstantValue* compileConstant(void) {
  ConstantValue* v;

  switch (lookAhead->tokenType) {
  case SB_PLUS:
    eat(SB_PLUS);
    v = compileConstant2();
    break;
  case SB_MINUS:
    eat(SB_MINUS);
    v = compileConstant2();
    v->intValue = -v->intValue;
    break;
  default:
    v = compileConstant2();
  }
  return v;
}

ConstantValue* compileConstant2(void) {
  ConstantValue* v = (ConstantValue*)malloc(sizeof(ConstantValue));

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    v->intValue = atoi(currentToken->string);
    break;
  case TK_IDENT: {
    eat(TK_IDENT);
    Object* obj = lookupObject(currentToken->string);
    *v = obj->constValue;
    break;
  }
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
  }
  return v;
}

/* ===================== TYPE ===================== */

Type* compileType(void) {
  Type* type;

  switch (lookAhead->tokenType) {
  case KW_INTEGER:
    eat(KW_INTEGER);
    type = intType;
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    type = charType;
    break;
  case KW_ARRAY: {
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);
    int size = atoi(currentToken->string);
    eat(SB_RSEL);
    eat(KW_OF);
    type = makeArrayType(size, compileType());
    break;
  }
  case TK_IDENT: {
    eat(TK_IDENT);
    Object* obj = lookupObject(currentToken->string);
    type = obj->type;
    break;
  }
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
  }
  return type;
}

Type* compileBasicType(void) {
  if (lookAhead->tokenType == KW_INTEGER) {
    eat(KW_INTEGER);
    return intType;
  }
  if (lookAhead->tokenType == KW_CHAR) {
    eat(KW_CHAR);
    return charType;
  }
  error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
  return NULL;
}

/* ===================== PARAM ===================== */

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

void compileParam(void) {
  Object* param;

  if (lookAhead->tokenType == TK_IDENT) {
    eat(TK_IDENT);
    param = createObject(currentToken->string, OBJ_PARAMETER);
    eat(SB_COLON);
    param->type = compileBasicType();
    declareObject(param);
  } else if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    eat(TK_IDENT);
    param = createObject(currentToken->string, OBJ_REF_PARAMETER);
    eat(SB_COLON);
    param->type = compileBasicType();
    declareObject(param);
  } else {
    error(ERR_INVALID_PARAMETER, lookAhead->lineNo, lookAhead->colNo);
  }
}

/* ===================== STATEMENTS ===================== */

void compileStatements(void) {
  compileStatement();
  while (lookAhead->tokenType == SB_SEMICOLON) {
    eat(SB_SEMICOLON);
    compileStatement();
  }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT: compileAssignSt(); break;
  case KW_CALL:  compileCallSt(); break;
  case KW_BEGIN: compileGroupSt(); break;
  case KW_IF:    compileIfSt(); break;
  case KW_WHILE: compileWhileSt(); break;
  case KW_FOR:   compileForSt(); break;
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
  default:
    error(ERR_INVALID_STATEMENT, lookAhead->lineNo, lookAhead->colNo);
  }
}

/* ===================== EXPRESSIONS & OTHERS ===================== */
/* (Giữ nguyên cú pháp – không semantic sâu) */

void compileLValue(void) { eat(TK_IDENT); compileIndexes(); }
void compileAssignSt(void) { compileLValue(); eat(SB_ASSIGN); compileExpression(); }
void compileCallSt(void) { eat(KW_CALL); eat(TK_IDENT); compileArguments(); }
void compileGroupSt(void) { eat(KW_BEGIN); compileStatements(); eat(KW_END); }
void compileIfSt(void) { eat(KW_IF); compileCondition(); eat(KW_THEN); compileStatement(); if (lookAhead->tokenType == KW_ELSE) compileElseSt(); }
void compileElseSt(void) { eat(KW_ELSE); compileStatement(); }
void compileWhileSt(void) { eat(KW_WHILE); compileCondition(); eat(KW_DO); compileStatement(); }
void compileForSt(void) { eat(KW_FOR); eat(TK_IDENT); eat(SB_ASSIGN); compileExpression(); eat(KW_TO); compileExpression(); eat(KW_DO); compileStatement(); }

void compileArgument(void) { compileExpression(); }

void compileArguments(void) {
  if (lookAhead->tokenType == SB_LPAR) {
    eat(SB_LPAR);
    compileArgument();
    while (lookAhead->tokenType == SB_COMMA) {
      eat(SB_COMMA);
      compileArgument();
    }
    eat(SB_RPAR);
  }
}

void compileCondition(void) {
  compileExpression();
  eat(lookAhead->tokenType);
  compileExpression();
}

void compileExpression(void) {
  if (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS)
    eat(lookAhead->tokenType);
  compileTerm();
}

void compileTerm(void) { compileFactor(); }
void compileFactor(void) {
  if (lookAhead->tokenType == TK_NUMBER) eat(TK_NUMBER);
  else if (lookAhead->tokenType == TK_CHAR) eat(TK_CHAR);
  else if (lookAhead->tokenType == TK_IDENT) eat(TK_IDENT);
  else error(ERR_INVALID_FACTOR, lookAhead->lineNo, lookAhead->colNo);
}

void compileIndexes(void) {
  while (lookAhead->tokenType == SB_LSEL) {
    eat(SB_LSEL);
    compileExpression();
    eat(SB_RSEL);
  }
}

/* ===================== ENTRY ===================== */

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR) return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  initSymTab();
  compileProgram();
  printObject(symtab->program, 0);
  cleanSymTab();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;
}
