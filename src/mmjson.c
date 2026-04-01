/*****************************************************************************/
/*        Copyright (C) 2026  Jeremy Lindsay                                 */
/*            License terms:  GNU General Public License                     */
/*****************************************************************************/
/*34567890123456 (79-character line to adjust editor window) 2345678901234567*/

/* Fork module for JSON proof serialization output using yyjson. */

#include <stdlib.h>
#include "mmjson.h"
#include "mminou.h"
#include "third_party/yyjson/yyjson.h"

static yyjson_mut_doc *g_mmJsonDoc = NULL;
static yyjson_mut_val *g_mmJsonRoot = NULL;
static yyjson_mut_val *g_mmJsonSteps = NULL;
static yyjson_mut_val *g_mmJsonCurrentArgs = NULL;
static yyjson_mut_val *g_mmJsonCurrentArgTypecodes = NULL;
static yyjson_mut_val *g_mmJsonCurrentArgStatementTypes = NULL;
static int g_mmJsonFullMode = 0;
static int g_mmJsonBatchMode = 0;

static void mmJsonResetState(void) {
  if (g_mmJsonDoc) {
    yyjson_mut_doc_free(g_mmJsonDoc);
  }
  g_mmJsonDoc = NULL;
  g_mmJsonRoot = NULL;
  g_mmJsonSteps = NULL;
  g_mmJsonCurrentArgs = NULL;
  g_mmJsonCurrentArgTypecodes = NULL;
  g_mmJsonCurrentArgStatementTypes = NULL;
  g_mmJsonFullMode = 0;
  g_mmJsonBatchMode = 0;
}

static int mmJsonEnsureDoc(int fullJsonFlag) {
  if (g_mmJsonDoc) {
    return 1;
  }

  g_mmJsonFullMode = fullJsonFlag;
  g_mmJsonDoc = yyjson_mut_doc_new(NULL);
  if (!g_mmJsonDoc) {
    print2("?JSON error: unable to allocate yyjson document.\n");
    return 0;
  }

  g_mmJsonRoot = yyjson_mut_obj(g_mmJsonDoc);
  if (!g_mmJsonRoot) {
    print2("?JSON error: unable to create root object.\n");
    mmJsonResetState();
    return 0;
  }
  yyjson_mut_doc_set_root(g_mmJsonDoc, g_mmJsonRoot);
  return 1;
}

static void mmJsonWriteAndReset(void) {
  char *jsonOut;
  size_t jsonLen = 0;

  if (!g_mmJsonDoc) {
    return;
  }
  jsonOut = yyjson_mut_write(g_mmJsonDoc,
      YYJSON_WRITE_PRETTY_TWO_SPACES | YYJSON_WRITE_NEWLINE_AT_END,
      &jsonLen);
  if (!jsonOut) {
    print2("?JSON error: failed to serialize yyjson document.\n");
    mmJsonResetState();
    return;
  }
  if (jsonLen) {
    print2("%s", jsonOut);
  }
  free(jsonOut);
  mmJsonResetState();
}

void mmJsonBatchStart(int fullJsonFlag) {
  mmJsonResetState();
  g_mmJsonBatchMode = 1;
  if (!mmJsonEnsureDoc(fullJsonFlag)) {
    return;
  }
}

void mmJsonBatchEnd(void) {
  if (!g_mmJsonDoc) {
    return;
  }
  mmJsonWriteAndReset();
}

void mmJsonProofStart(const char *theoremLabel, int fullJsonFlag) {
  if (!mmJsonEnsureDoc(fullJsonFlag)) {
    return;
  }
  g_mmJsonSteps = yyjson_mut_obj_add_arr(g_mmJsonDoc, g_mmJsonRoot, theoremLabel);
  if (!g_mmJsonSteps) {
    print2("?JSON error: unable to create proof steps array.\n");
    if (!g_mmJsonBatchMode) {
      mmJsonResetState();
    }
  }
}

void mmJsonProofAddStepStart(long stepNum,
    const char *ref,
    const char *type,
  const char *expr,
  const char *refStatementType,
  const char *refTypecode) {
  yyjson_mut_val *stepObj;

  if (!g_mmJsonDoc || !g_mmJsonSteps) {
    return;
  }

  stepObj = yyjson_mut_arr_add_obj(g_mmJsonDoc, g_mmJsonSteps);
  if (!stepObj) {
    print2("?JSON error: unable to add proof step object.\n");
    return;
  }
  if (!yyjson_mut_obj_add_int(g_mmJsonDoc, stepObj, "step", stepNum)
      || !yyjson_mut_obj_add_strcpy(g_mmJsonDoc, stepObj, "ref", ref)
      || !yyjson_mut_obj_add_strcpy(g_mmJsonDoc, stepObj, "type", type)
      || !yyjson_mut_obj_add_strcpy(g_mmJsonDoc, stepObj, "expr", expr)) {
    print2("?JSON error: unable to add proof step fields.\n");
    return;
  }

  g_mmJsonCurrentArgs = yyjson_mut_obj_add_arr(g_mmJsonDoc, stepObj, "args");
  if (!g_mmJsonCurrentArgs) {
    print2("?JSON error: unable to add args array.\n");
  }

  if (g_mmJsonFullMode) {
    if (refStatementType) {
      if (!yyjson_mut_obj_add_strcpy(g_mmJsonDoc,
          stepObj,
          "refStatementType",
          refStatementType)) {
        print2("?JSON error: unable to add refStatementType.\n");
      }
    } else {
      if (!yyjson_mut_obj_add_null(g_mmJsonDoc, stepObj, "refStatementType")) {
        print2("?JSON error: unable to add null refStatementType.\n");
      }
    }

    if (refTypecode) {
      if (!yyjson_mut_obj_add_strcpy(g_mmJsonDoc,
          stepObj,
          "refTypecode",
          refTypecode)) {
        print2("?JSON error: unable to add refTypecode.\n");
      }
    } else {
      if (!yyjson_mut_obj_add_null(g_mmJsonDoc, stepObj, "refTypecode")) {
        print2("?JSON error: unable to add null refTypecode.\n");
      }
    }

    g_mmJsonCurrentArgTypecodes = yyjson_mut_obj_add_arr(g_mmJsonDoc,
        stepObj,
        "argTypecodes");
    if (!g_mmJsonCurrentArgTypecodes) {
      print2("?JSON error: unable to add argTypecodes array.\n");
    }

    g_mmJsonCurrentArgStatementTypes = yyjson_mut_obj_add_arr(g_mmJsonDoc,
        stepObj,
        "argStatementTypes");
    if (!g_mmJsonCurrentArgStatementTypes) {
      print2("?JSON error: unable to add argStatementTypes array.\n");
    }
  }
}

void mmJsonProofAddStepArg(long argStep,
    int isUnknown,
    const char *argTypecode,
    const char *argStatementType) {
  if (!g_mmJsonDoc || !g_mmJsonCurrentArgs) {
    return;
  }
  if (isUnknown) {
    if (!yyjson_mut_arr_add_null(g_mmJsonDoc, g_mmJsonCurrentArgs)) {
      print2("?JSON error: unable to append null arg.\n");
    }
  } else {
    if (!yyjson_mut_arr_add_int(g_mmJsonDoc, g_mmJsonCurrentArgs, argStep)) {
      print2("?JSON error: unable to append numeric arg.\n");
    }
  }

  if (g_mmJsonFullMode && g_mmJsonCurrentArgTypecodes
      && g_mmJsonCurrentArgStatementTypes) {
    if (argTypecode) {
      if (!yyjson_mut_arr_add_strcpy(g_mmJsonDoc,
          g_mmJsonCurrentArgTypecodes,
          argTypecode)) {
        print2("?JSON error: unable to append arg typecode.\n");
      }
    } else {
      if (!yyjson_mut_arr_add_null(g_mmJsonDoc, g_mmJsonCurrentArgTypecodes)) {
        print2("?JSON error: unable to append null arg typecode.\n");
      }
    }

    if (argStatementType) {
      if (!yyjson_mut_arr_add_strcpy(g_mmJsonDoc,
          g_mmJsonCurrentArgStatementTypes,
          argStatementType)) {
        print2("?JSON error: unable to append arg statement type.\n");
      }
    } else {
      if (!yyjson_mut_arr_add_null(g_mmJsonDoc,
          g_mmJsonCurrentArgStatementTypes)) {
        print2("?JSON error: unable to append null arg statement type.\n");
      }
    }
  }
}

void mmJsonProofAddStepEnd(void) {
  g_mmJsonCurrentArgs = NULL;
  g_mmJsonCurrentArgTypecodes = NULL;
  g_mmJsonCurrentArgStatementTypes = NULL;
}

void mmJsonProofEnd(void) {
  if (!g_mmJsonDoc || g_mmJsonBatchMode) {
    g_mmJsonSteps = NULL;
    g_mmJsonCurrentArgs = NULL;
    g_mmJsonCurrentArgTypecodes = NULL;
    g_mmJsonCurrentArgStatementTypes = NULL;
    return;
  }
  mmJsonWriteAndReset();
}
