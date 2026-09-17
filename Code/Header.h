#pragma once
#include<tccore\method.h>
#include<stdio.h>
#include<conio.h>
#include <iostream>
#include <vector>
#include<tc\tc.h>
#include<tccore\custom.h>
#include<tccore\aom_prop.h>
#include<tccore\aom.h>
#include<fclasses\tc_string.h>
#include<tccore\item.h>
#include<tccore\grm.h>
#include<tccore\workspaceobject.h>
#include<form\form.h>
#include <cstdarg>
#include<stdlib.h>
#include <user_exits/user_exits.h>
#include <tccore/tctype.h>
#include<epm\epm.h>
#include<bom\bom.h>
#include<string>  
#include<epm/epm_toolkit_tc_utils.h>
#include<IContentPrefetcherTaskTrigger.h>
#include<strstream>
#include<sstream>
#include<ae/dataset.h>
#include<map>
#include <fstream>
#include<tc/preferences.h>
#include <algorithm>

#define DLLAPI _declspec(dllexport)
#define USER_INIT_MODULE      "USER_init_module"
#define item_idPROP			  "item_id"
#define IMANRESERVATION       "ImanReservation"
#define RES_CHECKIN           "RES_checkin"
#define M4SALES_FORCE_INFO    "m4sales_force_info"
#define SOP                   "m4sop"
#define PROJID                "m4project_id"
#define ITEMID                "item_id"
#define REVID                 "item_revision_id"
#define PREFERENCEIPP         "Marelli_OnePLM_to_IPP_Pref"
#ifdef __cplusplus
extern "C" {
#endif
    using namespace std;
#define CHECK_RETURN(x) \
{ \
    if ( (ifail = (x)) != ITK_ok ) \
    { \
        char *_ems = NULL; \
        EMH_ask_error_text ( ifail, &_ems ); \
        TC_write_syslog ( "ERROR: %d, ERROR MSG: %s. at Line: %d in File: %s\n", ifail, _ems, __LINE__, __FILE__ ); \
        printf("ERROR: %d, ERROR MSG: %s. at Line: %d in File: %s\n", ifail, _ems, __LINE__, __FILE__ );  \
        MEM_free ( _ems ); \
        return ifail; \
    } \
}
#define RULE_HANDLER_CHECK(x) \
{ \
if ((ifail = (x)) != ITK_ok) \
{ \
char* _ems = NULL; \
EMH_ask_error_text(ifail, &_ems); \
TC_write_syslog("ERROR: %d, ERROR MSG: %s. at Line: %d in File: %s\n", ifail, _ems, __LINE__, __FILE__); \
printf("ERROR: %d, ERROR MSG: %s. at Line: %d in File: %s\n", ifail, _ems, __LINE__, __FILE__);  \
MEM_free(_ems); \
} \
}

#define MARELLI_LOG_ERROR_AND_RETURN { \
	if (ireturnStatus != ITK_ok)\
	{ \
		char* err_string;\
		EMH_ask_error_text(ireturnStatus, &err_string); \
		TC_write_syslog("\n[%s] at line:[%d]",err_string, __LINE__); \
		log_file <<err_string<<" at line:" <<__LINE__ <<endl; \
		if (err_string) MEM_free(err_string); \
		if(log_file.is_open()) log_file.close(); \
		if(json_file.is_open())json_file.close();\
		throw ireturnStatus;\
	} \
}
    extern "C" DLLAPI int LibDLL_Prj_register_callbacks();
    extern DLLAPI int CUSTOM_register_extension(METHOD_message_t* msg, va_list args);
    int Baseline_Post_action(METHOD_message_t* msg, va_list args);
    //int Baseline_dataset(tag_t* tSchematic_Objs, int iSchematic_count, tag_t* tpwb_Objs, int iPwb_count, const char* baselineRevID, tag_t tNew_SchemObj, tag_t tNew_revObj, tag_t* tNewRev);
    //int Baseline_dataset_Schematic(tag_t tSchematic_Objs,const char* baselineRevID, tag_t tNew_SchemObj);
    //int Baseline_dataset_PCB(tag_t tpwb_Objs, const char* baselineRevID, tag_t tNew_revObj);
    int PCB_Baseline_Post_action(METHOD_message_t* msg, va_list args);
    int Schematic_Baseline_Post_action(METHOD_message_t* msg, va_list args);
    int Release_datasets(tag_t tNew_objs);
    int CR402PostAction(METHOD_message_t* msg, va_list args);
    int CR402Checkin_PostAction(METHOD_message_t* msg, va_list args);
    int checkSFTableProperty(tag_t checked_in_object);
    string getLocalTime();
    int createJsonForTriggerZero(string jsonFName, map<tag_t, string>FG_mapInput, string project);
    int get_all_project_from_map(map<tag_t, string> FG_mapInput, string prj_or_pgt, string& json);
    int postTriggerZeroToMARC(string SFjson, string PGTjson, string resonceFromMarcFile);
    void getFinalFGstr(string project_name, map<tag_t, string>maps, vector<tag_t>& finalFGStr, bool& line_read);
    int M4_sort_tags_by_properties(
        int                n_unsorted_tags,         /**< (I) Number of unsorted tags */
        const tag_t* unsorted_tags,          /**< (I) n_unsorted_tags Input unsorted tags */
        int* n_sorted_tags,          /**< (O) Number of sorted tags */
        tag_t** sorted_tags            /**< (OF) n_sorted_tags Output sorted tags. NULLTAGS and duplicate tags are not removed. NULLTAGS are placed at the end. */
    );
    int find_FGWithCorrectProject(string comId,std::string& ERRstring, std::string SF_project, tag_t checked_in_object);
#ifdef __cplusplus
}
#endif

