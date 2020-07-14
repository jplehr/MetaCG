#ifndef CGCOLLECTOR_HELPER_ASTHELPER_H
#define CGCOLLECTOR_HELPER_ASTHELPER_H

#include "clang/AST/Stmt.h"
#include "clang/AST/StmtCXX.h"

int getNumStmtsInStmt(clang::Stmt *d);
int getNumStmtsInCompoundStmt(clang::CompoundStmt *cpst);
int getNumStmtsInIfStmt(clang::IfStmt *is);
int getNumStmtsInForStmt(clang::ForStmt *fs);
int getNumStmtsInWhileStmt(clang::WhileStmt *ws);
int getNumStmtsInCXXForRangeStmt(clang::CXXForRangeStmt *frs);
int getNumStmtsInDoStmt(clang::DoStmt *ds);
int getNumStmtsInTryStmt(clang::CXXTryStmt *tryst);
int getNumStmtsInCatchStmt(clang::CXXCatchStmt *catchst);
int getNumStmtsInSwitchCase(clang::SwitchStmt *scStmt);
int getNumStmtsInCaseStmt(clang::CaseStmt *cStmt);

#endif
