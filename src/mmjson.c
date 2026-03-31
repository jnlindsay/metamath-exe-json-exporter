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
static yyjson_mut_val *g_mmJsonSteps = NULL;
static yyjson_mut_val *g_mmJsonCurrentArgs = NULL;

static void mmJsonResetState(void) {
  if (g_mmJsonDoc) {
    yyjson_mut_doc_free(g_mmJsonDoc);
  }
  g_mmJsonDoc = NULL;
  g_mmJsonSteps = NULL;
  g_mmJsonCurrentArgs = NULL;
}

void mmJsonProofStart(const char *theoremLabel) {
  yyjson_mut_val *root;

  mmJsonResetState();
  g_mmJsonDoc = yyjson_mut_doc_new(NULL);
  if (!g_mmJsonDoc) {
    print2("?JSON error: unable to allocate yyjson document.\n");
    return;
  }

  root = yyjson_mut_obj(g_mmJsonDoc);
  if (!root) {
    print2("?JSON error: unable to create root object.\n");
    mmJsonResetState();
    return;
  }
  yyjson_mut_doc_set_root(g_mmJsonDoc, root);
  g_mmJsonSteps = yyjson_mut_obj_add_arr(g_mmJsonDoc, root, theoremLabel);
  if (!g_mmJsonSteps) {
    print2("?JSON error: unable to create proof steps array.\n");
    mmJsonResetState();
  }
}

void mmJsonProofAddStepStart(long stepNum,
    const char *ref,
    const char *type,
    const char *expr) {
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
}

void mmJsonProofAddStepArg(long argStep, int isUnknown) {
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
}

void mmJsonProofAddStepEnd(void) {
  g_mmJsonCurrentArgs = NULL;
}

void mmJsonProofEnd(void) {
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
