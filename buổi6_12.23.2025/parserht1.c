/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "semantics.h"
#include "error.h"
#include "debug.h"

Token *currentToken;
Token *lookAhead;

extern Type* intType;
extern Type* charType;
extern SymTab* symtab;

/* --------------------------------------------------- */

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType)
    scan();
  else
    missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

/* --------------------------------------------------- */

void compileProgram(void) {
  Object* program;

  eat(KW_PROGRAM);
  eat(TK_IDENT);

  program = createProgramObject(currentToken->string);
  enterBlock(program->progAttrs->scope);

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);

  exitBlock();
}

/* --------------------------------------------------- */

void compileBlock(void) {
  Object* constObj;
  ConstantValue* constValue;

  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    do {
      eat(TK_IDENT);
      checkFreshIdent(currentToken->string);

      constObj = createConstantObject(currentToken->string);
      eat(SB_EQ);

      constValue = compileConstant();
      constObj->constAttrs->value = constValue;
      declareObject(constObj);

      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);
  }
  compileBlock2();
}

void compileBlock2(void) {
  Object* typeObj;
  Type* actualType;

  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    do {
      eat(TK_IDENT);
      checkFreshIdent(currentToken->string);

      typeObj = createTypeObject(currentToken->string);
      eat(SB_EQ);

      actualType = compileType();
      typeObj->typeAttrs->actualType = actualType;
      declareObject(typeObj);

      eat(SB_SEMICOLON);
    } while (lookAhead->tokenType == TK_IDENT);
  }
  compileBlock3();
}

void compileBlock3(void) {
  Object* varObj;
  Type* varType;

  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    do {
      eat(TK_IDENT);
      checkFreshIdent(currentToken->string);

      varObj = createVariableObject(currentToken->string);
      eat(SB_COLON);

      varType = compileType();
      varObj->varAttrs->type = varType;
      declareObject(varObj);

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

/* --------------------------------------------------- */

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
  Object* funcObj;
  Type* returnType;

  eat(KW_FUNCTION);
  eat(TK_IDENT);
  checkFreshIdent(currentToken->string);

  funcObj = createFunctionObject(currentToken->string);
  declareObject(funcObj);
  enterBlock(funcObj->funcAttrs->scope);

  compileParams();
  eat(SB_COLON);

  returnType = compileBasicType();
  funcObj->funcAttrs->returnType = returnType;

  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

void compileProcDecl(void) {
  Object* procObj;

  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  checkFreshIdent(currentToken->string);

  procObj = createProcedureObject(currentToken->string);
  declareObject(procObj);
  enterBlock(procObj->procAttrs->scope);

  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);

  exitBlock();
}

/* --------------------------------------------------- */

ConstantValue* compileUnsignedConstant(void) {
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredConstantIdent(currentToken->string);
    constValue = obj->constAttrs->value;
    break;
  case TK_CHAR:
    eat(TK_CHAR);
    constValue = makeCharConstant(currentToken->string[0]);
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
  }
  return constValue;
}

ConstantValue* compileConstant(void) {
  ConstantValue* constValue;

  if (lookAhead->tokenType == SB_PLUS) {
    eat(SB_PLUS);
    constValue = compileConstant2();
  } else if (lookAhead->tokenType == SB_MINUS) {
    eat(SB_MINUS);
    constValue = compileConstant2();
    constValue->intValue = -constValue->intValue;
  } else if (lookAhead->tokenType == TK_CHAR) {
    eat(TK_CHAR);
    constValue = makeCharConstant(currentToken->string[0]);
  } else {
    constValue = compileConstant2();
  }
  return constValue;
}

ConstantValue* compileConstant2(void) {
  ConstantValue* constValue;
  Object* obj;

  switch (lookAhead->tokenType) {
  case TK_NUMBER:
    eat(TK_NUMBER);
    constValue = makeIntConstant(currentToken->value);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredConstantIdent(currentToken->string);
    constValue = obj->constAttrs->value;
    break;
  default:
    error(ERR_INVALID_CONSTANT, lookAhead->lineNo, lookAhead->colNo);
  }
  return constValue;
}

/* --------------------------------------------------- */

Type* compileType(void) {
  Type* type;
  Type* elementType;
  int arraySize;
  Object* obj;

  switch (lookAhead->tokenType) {
  case KW_INTEGER:
    eat(KW_INTEGER);
    type = makeIntType();
    break;
  case KW_CHAR:
    eat(KW_CHAR);
    type = makeCharType();
    break;
  case KW_ARRAY:
    eat(KW_ARRAY);
    eat(SB_LSEL);
    eat(TK_NUMBER);
    arraySize = currentToken->value;
    eat(SB_RSEL);
    eat(KW_OF);
    elementType = compileType();
    type = makeArrayType(arraySize, elementType);
    break;
  case TK_IDENT:
    eat(TK_IDENT);
    obj = checkDeclaredTypeIdent(currentToken->string);
    type = obj->typeAttrs->actualType;
    break;
  default:
    error(ERR_INVALID_TYPE, lookAhead->lineNo, lookAhead->colNo);
  }
  return type;
}

Type* compileBasicType(void) {
  Type* type;

  if (lookAhead->tokenType == KW_INTEGER) {
    eat(KW_INTEGER);
    type = makeIntType();
  } else if (lookAhead->tokenType == KW_CHAR) {
    eat(KW_CHAR);
    type = makeCharType();
  } else {
    error(ERR_INVALID_BASICTYPE, lookAhead->lineNo, lookAhead->colNo);
  }
  return type;
}

/* --------------------------------------------------- */

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
  Type* type;
  enum ParamKind paramKind;

  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    paramKind = PARAM_REFERENCE;
  } else {
    paramKind = PARAM_VALUE;
  }

  eat(TK_IDENT);
  checkFreshIdent(currentToken->string);

  param = createParameterObject(currentToken->string,
                                paramKind,
                                symtab->currentScope->owner);

  eat(SB_COLON);
  type = compileBasicType();
  param->paramAttrs->type = type;
  declareObject(param);
}

/* --------------------------------------------------- */

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

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
