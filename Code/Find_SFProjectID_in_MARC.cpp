#include "Header.h"
ofstream log_file;

int CR402Checkin_PostAction(METHOD_message_t* msg, va_list args)
{
	cout << "CR402Checkin_PostAction...." << endl;
	int ifail = ITK_ok;

	tag_t checked_in_object = va_arg(args, tag_t);

	if (checked_in_object != NULLTAG)
	{
		tag_t object_type_tag = NULLTAG;
		char* cobject_type_name = NULL;
		char* cClassValue = NULL;
		CHECK_RETURN(TCTYPE_ask_object_type(checked_in_object, &object_type_tag));
		CHECK_RETURN(TCTYPE_ask_name2(object_type_tag, &cobject_type_name));
		cout << "cobject_type_name:" << cobject_type_name << endl;
		//For MPart Rev checkin
		if (strcmp(cobject_type_name, "M4MPartRevision") == 0)
		{
			CHECK_RETURN(AOM_ask_value_string(checked_in_object, "m4class", &cClassValue));
			cout << "cClassValue:" << cClassValue << endl;
			if (tc_strcmp(cClassValue, "Finished Product") == 0)//Awarded Finished Good /Finished Product
			{
				CHECK_RETURN(checkSFTableProperty(checked_in_object));
			}
		}
		//For MPart checkin
		if (strcmp(cobject_type_name, "M4MPart") == 0)
		{
			CHECK_RETURN(AOM_ask_value_string(checked_in_object, "m4class", &cClassValue));
			cout << "cClassValue:" << cClassValue << endl;
			if (tc_strcmp(cClassValue, "Finished Product") == 0)//Awarded Finished Good /Finished Product
			{
				tag_t tMPartRev = NULLTAG;
				CHECK_RETURN(ITEM_ask_latest_rev(checked_in_object, &tMPartRev));
				if (tMPartRev != NULLTAG)
				{
					CHECK_RETURN(checkSFTableProperty(tMPartRev));
				}

			}
		}
	}
	else
	{
		char* cError = NULL;
		CHECK_RETURN(EMH_ask_error_text(ifail, &cError));
		printf("error:%s", cError);
		TC_write_syslog("ERROR: %d, ERROR MSG: %s. at Line: %d in File: %s\n", ifail, cError, __LINE__, __FILE__);
	}


	//MEM_free(object_type_name);
	return ifail;
}
int checkSFTableProperty(tag_t checked_in_object)
{
	cout << "checkSFTableProperty...." << endl;
	int ifail = ITK_ok;
	int iSFRowCount = 0;
	tag_t* tSFRow = NULL;
	char* cPGT_ProjectID = NULL;
	char* cSF_ProjectID = NULL;
	vector<std::string>vAllPGT_ID;
	vector<std::string>vAllSFProject_ID;
	std::string SF_project;
	string error_msg;
	error_msg.assign("\n");
	char* filepath = NULL;
	ifail = PREF_ask_char_value(PREFERENCEIPP, 0, &filepath);
	CHECK_RETURN(AOM_ask_table_rows(checked_in_object, M4SALES_FORCE_INFO, &iSFRowCount, &tSFRow));
	cout << "iSFRowCount:" << iSFRowCount << endl;
	if (iSFRowCount > 0)
	{
		for (int inx = 0; inx < iSFRowCount; inx++)
		{

			CHECK_RETURN(AOM_ask_value_string(tSFRow[inx], "m4PGT_ID", &cPGT_ProjectID));
			if (tc_strcmp(cPGT_ProjectID, "") != 0)
			{
				vAllPGT_ID.push_back(cPGT_ProjectID);
			}

		}
		cout << "vAllPGT_ID:" << vAllPGT_ID.size() << endl;
		// First  case if PGT_ID  is NULL in all row of SF Table then check for SF_Project ID 
		if (vAllPGT_ID.size() == 0)
		{
			for (int inx = 0; inx < iSFRowCount; inx++)
			{
				CHECK_RETURN(AOM_ask_value_string(tSFRow[inx], "m4project_id", &cSF_ProjectID));
				if (tc_strcmp(cSF_ProjectID, "") != 0)
				{
					vAllSFProject_ID.push_back(cPGT_ProjectID);
				}

			}
			cout << "vAllSFProject_ID:" << vAllSFProject_ID.size() << endl;

			// Third case if PGT_ID and Project_ID is NULL in all row of SF Table
			if (vAllSFProject_ID.size() == 0)
			{
				cout << "PGTID & cSF_ProjectID not found...." << endl;
				error_msg.append("Please fill in PGT Project ID for Awarded Finished Good");
				CHECK_RETURN(EMH_store_error_s1(EMH_severity_warning, -1, error_msg.c_str()));
			}
			// Second  case if PGT_ID is NULL and Project_ID is found  in any of Row check PGT_ID in MARC
			else
			{
				//char* filepath = NULL;
				cout << "PcSF_ProjectID  found...." << endl;
				std::string ERRstring;
				char* itemId = NULL;
				char* fgProj = NULL;
				char* fgSOP = NULL;
				char* itemID = NULL;
				char* revID = NULL;
				int sortedRowsCount = 0;
				tag_t* sortedTableRows = NULL;
				ifail = AOM_ask_value_string(checked_in_object, ITEMID, &itemId);
				cout << "itemId:" << itemId << endl;
				string fgID = itemId;
				CHECK_RETURN(M4_sort_tags_by_properties(iSFRowCount, tSFRow, &sortedRowsCount, &sortedTableRows));
				//MARELLI_LOG_ERROR_AND_RETURN;
				if (sortedRowsCount != 0)
				{
					for (int index = 0; index < sortedRowsCount; index++)
					{					
						ifail = AOM_UIF_ask_value(sortedTableRows[index], SOP, &fgSOP);
						cout << "fgSOP:" << fgSOP << endl;
						//MARELLI_LOG_ERROR_AND_RETURN;
						if (fgSOP == NULL || tc_strcmp("", fgSOP) == 0) {
							//attributeCheckList.push_back(" SOP date is not filed under Saleforce info Tab of FG" + (string)fgID + "\n ");
							ERRstring.append(" SOP date is not filed under Saleforce info Tab of FG" + (string)fgID + "\n ");
						}
						ifail = AOM_ask_value_string(sortedTableRows[index], PROJID, &fgProj);
						//MARELLI_LOG_ERROR_AND_RETURN;
						cout << "fgProj:" << fgProj << endl;
						//ireturnStatus = AOM_ask_value_string(sortedTableRows[index], "m4PGT_ID", &fgPGT_Proj);
						//MARELLI_LOG_ERROR_AND_RETURN;
						// can we check direly the 0 position of SF and PGT present or not ?
						if ((fgProj == NULL || tc_strcmp("", fgProj) == 0)) 
						{
							//attributeCheckList.push_back(" Project is not filed under Saleforce info Tab of FG " + (string)fgID + "\n ");
							ERRstring.append(" Project is not filed under Saleforce info Tab of FG " + (string)fgID + "\n ");
						}
					}
					
						ifail = AOM_ask_value_string(sortedTableRows[0], PROJID, &fgProj);
						//MARELLI_LOG_ERROR_AND_RETURN;
						SF_project.append(fgProj);
						ifail = AOM_ask_value_string(checked_in_object, ITEMID, &itemID);
						//MARELLI_LOG_ERROR_AND_RETURN;
						ifail = AOM_ask_value_string(checked_in_object, REVID, &revID);
						//MARELLI_LOG_ERROR_AND_RETURN;
						string id = itemID; id.append("_"); id.append(revID);
						find_FGWithCorrectProject(id,ERRstring, SF_project, checked_in_object);
						// SF PGT ID 
						//ifail = AOM_ask_value_string(sortedTableRows[0], "m4PGT_ID", &fgPGT_Proj);
						//MARELLI_LOG_ERROR_AND_RETURN;
						//PGT_Project.append(fgPGT_Proj);
					
				}

				//SF_project.append(cSF_ProjectID);
			}
		}
		// First  case if PGT_ID  found  in any of the Row check in succesfull
		else
		{
			cout << "PGTID found checkin successfull...." << endl;
		}
	}

	return ifail;
}
int find_FGWithCorrectProject(string comId,std::string& ERRstring,std::string SF_project, tag_t checked_in_object)
{
	try
	{
		cout << "inside find_FGWithCorrectProject..." << endl;
		int ireturnStatus = ITK_ok;
		map<tag_t, string>FG_SF_mapInput;
		map<tag_t, string>FG_PGT_mapInput;
		FG_SF_mapInput[checked_in_object] = SF_project;
		cout << "SF_project:" << SF_project << endl;
		if (ERRstring.length() > 1)
		{
			ireturnStatus = EMH_store_error_s1(EMH_severity_error, -1, ERRstring.c_str());
			log_file << "\nError in Attribute Check List Trigger Zero: " << ERRstring;
			cout << "ERRstring:" << ERRstring << endl;
			//MARELLI_LOG_ERROR_AND_RETURN;
			return 1;
		}
		ofstream Json_trigger0;
		string Ltime = getLocalTime();
		cout << "Ltime:" << Ltime << endl;
		//string jsonFileNametrigger0SF = string(filepath) + comId + "_SF_Trigger0_" + Ltime + ".json";
		string jsonFileNametrigger0SF = "D:\\" + comId + "_SF_Trigger0_" + Ltime + ".json";
		//string jsonFileNametrigger0PGT = string(filepath) + comId + "_PGT_Trigger0_" + Ltime + ".json";
		//string resonceFromMarcFile = string(filepath) + comID + "_MARC_Responce_" + Ltime + ".txt";
		//string resonceFromMarcFile = "C:\\Users\\aii96002\\Desktop\\ipp_Jar\\New Text Document.txt";
		createJsonForTriggerZero(jsonFileNametrigger0SF, FG_SF_mapInput, "SF");
		//createJsonForTriggerZero(jsonFileNametrigger0PGT, FG_PGT_mapInput, "PGT");
		
			// this use to remove the file from loglocation
			//remove(resonceFromMarcFile.c_str());
		
	}

	catch (const std::exception&)
	{
	}
	
}
int createJsonForTriggerZero(string jsonFName, map<tag_t, string>FG_mapInput, string project) {
	//int output = 10;
	try
	{
		string service = "";
		if (tc_strcmp(project.c_str(), "SF") == 0)
		{
			service = "OnePLM-CheckProjectsBySFProjectID";
		}
		else if (tc_strcmp(project.c_str(), "PGT") == 0)
		{
			service = "OnePLM-CheckProjectsByPGTProjectID";
		}
		string json = "";
		json = json + "{  \"header\": { \"clientKey\": \"TataTec\"  },  \"services\": [    {      \"name\": \"" + service + "\",      \"dataIO\": {        \"meta_data\": {          \"version\": \"1.0\",          \"description\": \"Integration Test\"        },        \"projects\": [{";
		if (FG_mapInput.size() > 0)
		{
			get_all_project_from_map(FG_mapInput, project, json);
		}
		/*json = json + "],";
		json = json + "\"PGT_INFO\": [{";
		if (FG_PGT_mapInput.size() > 0)
		{
			get_all_project_from_map(FG_PGT_mapInput, "PGT", json);
		}*/
		json = json + "] } } ]}";
		ofstream Json_trigger0;
		Json_trigger0.open(jsonFName, std::ios_base::app);
		if (Json_trigger0.is_open())
		{
			Json_trigger0 << json;
		}

		//output =postTriggerZeroToMARC(jsonFName, resonceFromMarcFile);
		Json_trigger0.close();
	}
	catch (const std::exception&)
	{
	}
	return ITK_ok;
}
int get_all_project_from_map(map<tag_t, string> FG_mapInput, string prj_or_pgt, string& json) {
	try
	{
		int mapSize = 0;
		//map<tag_t, string>::iterator itr;
		for (const auto& pair : FG_mapInput) {
			string projectID = pair.second;
			mapSize++;
			if (tc_strcmp(prj_or_pgt.c_str(), "SF") == 0)
			{
				json = json + "  \"sf_project_id\": \"" + projectID + "\"";
			}
			else if (tc_strcmp(prj_or_pgt.c_str(), "PGT") == 0)
			{
				json = json + "  \"pgt_project_id\": \"" + projectID + "\"";
			}

			json = json + "}";
			//if (!m.size() - 1)
			if (mapSize != FG_mapInput.size())
			{
				json = json + ",";
				json = json + "{";
			}
		}
		//for (itr = FG_mapInput.begin(); itr!= FG_mapInput.end(); ++itr) {
		//	string projectID = itr->second;
		//	mapSize++;
		//	if (tc_strcmp(prj_or_pgt.c_str(), "SF") == 0)
		//	{
		//		json = json + "  \"sf_project_id\": \"" + projectID + "\"";
		//	}
		//	else if (tc_strcmp(prj_or_pgt.c_str(), "PGT") == 0)
		//	{
		//		json = json + "  \"pgt_project_id\": \"" + projectID + "\"";
		//	}

		//	json = json + "}";
		//	//if (!m.size() - 1)
		//	if (itr != FG_mapInput.end())
		//	{
		//		json = json + ",";
		//	}
		//}
	}
	catch (const std::exception&)
	{
	}
}
int postTriggerZeroToMARC(string SFjson, string PGTjson, string resonceFromMarcFile)
{
	int ireturnStatus = 0;
	char* zeroTrgJar = NULL;
	ireturnStatus = PREF_ask_char_value(PREFERENCEIPP, 3, &zeroTrgJar);
	//MARELLI_LOG_ERROR_AND_RETURN;
	if (zeroTrgJar == NULL || tc_strcmp("", zeroTrgJar) == 0)
	{
		ireturnStatus = EMH_store_error_s1(EMH_severity_error, 919015, "Trigger Zero path is invalid in preference Marelli_OnePLM_to_IPP_Pref");
		//MARELLI_LOG_ERROR_AND_RETURN;
	}
	//string zeroTrgJar = "C:\\Ashish_Sample\\IPP\\TRGZERO\\TRGZERO.jar";
	string jarArgument = "java -jar " + (string)zeroTrgJar + " -SFjson=" + SFjson + " -PGTjson=" + PGTjson + " -responceLogPath=" + resonceFromMarcFile + " -trigNo=0";
	int output = system(jarArgument.c_str());

	// Need to what we can paas from SOA code and how to check 
	return output;

}
int M4_sort_tags_by_properties(
	int                n_unsorted_tags,         /**< (I) Number of unsorted tags */
	const tag_t* unsorted_tags,          /**< (I) n_unsorted_tags Input unsorted tags */
	int* n_sorted_tags,          /**< (O) Number of sorted tags */
	tag_t** sorted_tags            /**< (OF) n_sorted_tags Output sorted tags. NULLTAGS and duplicate tags are not removed. NULLTAGS are placed at the end. */
)
{
	int ireturnStatus = ITK_ok;
	cout << "inside M4_sort_tags_by_properties..." << endl;
	//*n_sorted_tags = 0;
	//**sorted_tags = NULL;

	if (n_unsorted_tags > 0)
	{
		tag_t	propDescriptor = NULLTAG;
		ireturnStatus = AOM_ask_descriptor(unsorted_tags[0], SOP, &propDescriptor);
		//MARELLI_LOG_ERROR_AND_RETURN;

		int* order = (int*)MEM_alloc(sizeof(int));
		order[0] = 0;
		ireturnStatus = AOM_sort_tags_by_properties(n_unsorted_tags, unsorted_tags, 1, &propDescriptor, order, n_sorted_tags, sorted_tags);
		//MARELLI_LOG_ERROR_AND_RETURN;
	}

	return ireturnStatus;
}
string getLocalTime() {

	time_t nowtime = time(nullptr);
	std::stringstream ss;
	ss << nowtime;
	std::string ts = ss.str();
	return ts;
}
void getFinalFGstr(string project_name, map<tag_t, string>maps, vector<tag_t>& finalFGStr, bool& line_read) {
	for (auto& it : maps) {
		// If mapped value is prj then print the key value
		//here we have add new function with different logioc as we are pass all the poj comma seperated and the same we have to chek in map 
		if (it.second == project_name) {
			finalFGStr.push_back(it.first);
			line_read = true;
		}
	}
}