/*****************************************************************************/
/*        Copyright (C) 2026  Jeremy Lindsay                                 */
/*            License terms:  GNU General Public License                     */
/*****************************************************************************/
/*34567890123456 (79-character line to adjust editor window) 2345678901234567*/

/* Fork module for JSON proof serialization output using yyjson. */

#ifndef METAMATH_MMJSON_H_
#define METAMATH_MMJSON_H_

void mmJsonProofStart(const char *theoremLabel);
void mmJsonProofAddStepStart(long stepNum,
	const char *ref,
	const char *type,
	const char *expr);
void mmJsonProofAddStepArg(long argStep, int isUnknown);
void mmJsonProofAddStepEnd(void);
void mmJsonProofEnd(void);

#endif // METAMATH_MMJSON_H_
