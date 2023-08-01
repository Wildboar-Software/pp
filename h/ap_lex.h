/* ap_lex.h: lexical symbols for address parser */

/*
 * @(#) $Header: /xtel/pp/pp-beta/h/RCS/ap_lex.h,v 6.0 1991/12/18 20:42:44 jpo Rel $
 *
 * $Log: ap_lex.h,v $
 * Revision 6.0  1991/12/18  20:42:44  jpo
 * Release 6.0
 *
 *
 */



#ifndef _H_AP_LEX
#define _H_AP_LEX


#define LT_EOL          0         /* New-Line                             */
#define LT_SPC          0         /* Space and tab                        */
#define LT_ERR          1         /* Illegal chars (control chars)        */
#define LT_EOD          2         /* End of Data (null)                   */
#define LT_COM          3         /* ,                                    */
#define LT_AT           4         /* @                                    */
#define LT_COL          5         /* :                                    */
#define LT_SEM          6         /* ;                                    */
#define LT_LES          7         /* <                                    */
#define LT_GTR          8         /* >                                    */
#define LT_SQT          9         /* \ (only in quoted strings)           */
#define LT_LTR         10         /* alphabetics, numbers, and others     */
#define LT_XTR         10         /* alphabetics, numbers, and others     */
#define LT_NUM         10         /* alphabetics, numbers, and others     */
#define LT_LPR         11         /* (                                    */
#define LT_RPR         12         /* )                                    */
#define LT_QOT         13         /* "                                    */
#define LT_LSQ         14         /* [                                    */
#define LT_RSQ         15         /* ]                                    */


#define LV_EOD          0         /* End of Data                          */
#define LV_ERROR        1         /* These Values go with the above Types */
#define LV_COMMA        2         /* ,                                    */
#define LV_AT           3         /* @                                    */
#define LV_COLON        4         /* :                                    */
#define LV_SEMI         5         /* ;                                    */
#define LV_COMMENT      6         /* (text text text)                     */
#define LV_LESS         7         /* <                                    */
#define LV_GRTR         8         /* >                                    */
#define LV_WORD         9         /* atom / string                        */
#define LV_FROM        10         /* <<                                   */
#define LV_DLIT        11         /* [text text text]                     */


#endif
