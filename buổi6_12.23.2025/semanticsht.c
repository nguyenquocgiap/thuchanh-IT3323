#include <stdlib.h>
#include <string.h>
#include "semantics.h"
#include "error.h"

extern SymTab* symtab;
extern Token* currentToken;

// Tìm kiếm đối tượng trong toàn bộ phạm vi từ hiện tại ra ngoài [cite: 32, 43, 55]
Object* lookupObject(char *name) {
  Scope* scope = symtab->currentScope;
  Object* obj;

  while (scope != NULL) {
    obj = findObject(scope->objList, name);
    if (obj != NULL) return obj;
    scope = scope->outer; // Tìm kiếm ở những phạm vi rộng hơn [cite: 32]
  }
  return NULL;
}

// Kiểm tra tên có được khai báo mới trong phạm vi hiện tại hay không [cite: 13, 14]
void checkFreshIdent(char *name) {
  if (findObject(symtab->currentScope->objList, name) != NULL) {
    error(ERR_DUPLICATE_IDENT, currentToken->lineNo, currentToken->colNo); [cite: 89]
  }
}

// Kiểm tra định danh đã được khai báo hay chưa [cite: 10]
Object* checkDeclaredIdent(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo); [cite: 83]
  }
  return obj;
}

// Kiểm tra tham chiếu tới hằng số 
Object* checkDeclaredConstant(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo); [cite: 84]
  }
  if (obj->kind != OBJ_CONSTANT) {
    error(ERR_UNDECLARED_CONSTANT, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

// Kiểm tra tham chiếu tới kiểu dữ liệu [cite: 40, 41]
Object* checkDeclaredType(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo); [cite: 85]
  }
  if (obj->kind != OBJ_TYPE) {
    error(ERR_UNDECLARED_TYPE, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

// Kiểm tra tham chiếu tới biến [cite: 50, 51]
Object* checkDeclaredVariable(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo); [cite: 86]
  }
  if (obj->kind != OBJ_VARIABLE && obj->kind != OBJ_PARAMETER) {
    error(ERR_UNDECLARED_VARIABLE, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

// Kiểm tra tham chiếu tới hàm [cite: 66, 67]
Object* checkDeclaredFunction(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo); [cite: 87]
  }
  if (obj->kind != OBJ_FUNCTION) {
    error(ERR_UNDECLARED_FUNCTION, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

// Kiểm tra tham chiếu tới thủ tục [cite: 75, 76]
Object* checkDeclaredProcedure(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo); [cite: 88]
  }
  if (obj->kind != OBJ_PROCEDURE) {
    error(ERR_UNDECLARED_PROCEDURE, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}

// Kiểm tra định danh ở vế trái câu lệnh gán (L-Value) [cite: 106]
// Có thể là biến, tham số hoặc tên hàm hiện tại [cite: 59, 60, 61]
Object* checkDeclaredLValueIdent(char* name) {
  Object* obj = lookupObject(name);
  if (obj == NULL) {
    error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo); [cite: 83]
  }

  switch (obj->kind) {
    case OBJ_VARIABLE:
    case OBJ_PARAMETER:
      break;
    case OBJ_FUNCTION:
      // Chỉ hợp lệ nếu là tên của hàm hiện tại đang được định nghĩa 
      if (obj != symtab->currentScope->owner) {
        error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
      }
      break;
    default:
      error(ERR_UNDECLARED_IDENT, currentToken->lineNo, currentToken->colNo);
  }
  return obj;
}