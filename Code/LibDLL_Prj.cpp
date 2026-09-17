// LibDLL_Prj.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Header.h"

extern "C" DLLAPI int LibDLL_Prj_register_callbacks()
{
	int ifail = ITK_ok;
	CHECK_RETURN(CUSTOM_register_exit("LibDLL_Prj", USER_INIT_MODULE, (CUSTOM_EXIT_ftn_t)CUSTOM_register_extension));
	return ifail;
}
extern DLLAPI int CUSTOM_register_extension(METHOD_message_t* msg, va_list args)
{
	int ifail = ITK_ok;
	METHOD_id_t method_id;
	METHOD_id_t method_id2;
	METHOD_id_t method_id3;
	METHOD_id_t method_id4;
	METHOD_id_t method_id5;
	cout << "Registering LibDLL_Prj...." << endl;
	CHECK_RETURN(METHOD_find_method("EDACCABaseRevision", "ITEM_baseline_rev", &method_id));
	CHECK_RETURN(METHOD_find_method("M4MPcbRevision", "ITEM_baseline_rev", &method_id2));
	CHECK_RETURN(METHOD_find_method("EDASchem Revision", "ITEM_baseline_rev", &method_id3));
	CHECK_RETURN(METHOD_find_method("M4MPart", "ITEM_create", &method_id4));
	CHECK_RETURN(METHOD_find_method(IMANRESERVATION, RES_CHECKIN, &method_id5));
	CHECK_RETURN(METHOD_add_action(method_id, METHOD_post_action_type, (METHOD_function_t)Baseline_Post_action, NULL));
	CHECK_RETURN(METHOD_add_action(method_id2, METHOD_post_action_type, (METHOD_function_t)PCB_Baseline_Post_action, NULL));
	CHECK_RETURN(METHOD_add_action(method_id3, METHOD_post_action_type, (METHOD_function_t)Schematic_Baseline_Post_action, NULL));
	CHECK_RETURN(METHOD_add_action(method_id4, METHOD_post_action_type, (METHOD_function_t)CR402PostAction, NULL));
	CHECK_RETURN(METHOD_add_action(method_id5, METHOD_post_action_type, (METHOD_function_t)CR402Checkin_PostAction, NULL));
	return ifail;
}



int CR402PostAction(METHOD_message_t* msg, va_list args)
{
	TC_write_syslog("\nEnter in CR402PostAction.... \n");
	int ifail = ITK_ok;
	va_list largs;
	va_copy(largs, args);
	char* item_id               = va_arg(largs, char*);
	char* item_name             = va_arg(largs, char*);
	char* type_name             = va_arg(largs, char*);
	char* rev_id                = va_arg(largs, char*);
	tag_t* new_item             = va_arg(largs, tag_t*);
	tag_t* new_rev	            = va_arg(largs, tag_t*);
	tag_t  item_master_form     = va_arg(largs, tag_t);
	tag_t  item_rev_master_form = va_arg(largs, tag_t);
	
	va_end(largs);
	char* cMainCategory = NULL;
	
	CHECK_RETURN(AOM_ask_value_string(new_item[0], "item_id", &cMainCategory));//MainCategory
	//cout << "cMainCategory:" << cMainCategory[0] << endl;
	TC_write_syslog("\n cMainCategory: %s \n", cMainCategory);
	if(cMainCategory[0]=='A')
	{
		CHECK_RETURN(AOM_refresh(new_item[0], true));
		CHECK_RETURN(AOM_set_value_string(new_item[0], "m4class", "Component"));//Not Awarded Finished Good/Component
		CHECK_RETURN(AOM_refresh(new_item[0], false));
		
	}
	//cout << "Exit in CR402PostAction...." << endl;
	return ifail;
}
int Baseline_Post_action(METHOD_message_t* msg, va_list args)
{
	int ifail = ITK_ok;
	int retCode = ITK_ok;

	va_list largs;

	va_copy(largs, args);

	tag_t tSrcRev = va_arg(largs, tag_t);

	const char* baselineRevID = va_arg(largs, const char*);

	const char* baselineDesc = va_arg(largs, const char*);

	const char* processname = va_arg(largs, const char*);

	const char* jobname = va_arg(largs, const char*);

	const char* jobdesc = va_arg(largs, const char*);

	logical dryrun = va_arg(largs, logical);

	tag_t* tNewRev = va_arg(largs, tag_t*);

	va_end(largs);
	
	tag_t tSchematic_Rel = NULLTAG;
	tag_t tPWB_Rel = NULLTAG;
	int iPwb_count = 0;
	tag_t* tpwb_Objs = NULL;
	int iSchematic_count = 0;
	tag_t* tSchematic_Objs = NULL;
	char* cSec_type = NULL;
	char* cNew_revObjName = NULL;
	char* cItem_id = NULL;
	tag_t tNew_revObj = NULLTAG;
	tag_t tNewRel = NULLTAG;
	int iCount = 0;
	tag_t* tDeep_copied_objects = NULL;
	
	//Added for datasetlogiv
	tag_t DerivedRelTag = NULLTAG;
	char* className = NULL;
	tag_t class_id = NULLTAG;
	char* itemID = NULL;
	int secCount = 0;
	tag_t* secTags = NULL;
	tag_t newRelTag = NULLTAG;
	tag_t tNewDataset = NULLTAG;
	tag_t tStatus = NULLTAG;
	char* datasetName = NULL;
	cout << "Enter Baseline_Post_action..." << endl;

	cout << "baselineRevID:" << baselineRevID << endl;
	cout << "processname:" << processname << endl;
	cout << "jobname:" << jobname << endl;
	cout << "jobdesc:" << jobdesc << endl;
	if (tc_strlen(baselineRevID) > 4 && tc_strstr(baselineRevID, ".") != NULL && *tNewRev != NULLTAG)

	{
		GRM_find_relation_type("EDAHasDerivedDataset", &DerivedRelTag);

		if (DerivedRelTag != NULLTAG)
		{

			//for MElectric assembly with EDAHasDerivedDataset relation
			GRM_list_secondary_objects_only(tSrcRev, DerivedRelTag, &secCount, &secTags);

			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							//char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(secTags[secIdx], "object_name", &itemID);

								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);

									//Attach dataset to target rev if excel type...
									AE_copy_dataset_with_id(secTags[secIdx],
										newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);

									if (tNewDataset != NULLTAG)
									{
										//tag_t newRelTag = NULLTAG;

										//create relation with new dataset on new revision...													
										GRM_create_relation(*tNewRev, tNewDataset, DerivedRelTag, NULL, &newRelTag);

										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}

								if (itemID != NULL) { MEM_free(itemID); }

							}

							if (datasetName != NULL) { MEM_free(datasetName); }


						}

						if (className != NULL) { MEM_free(className); }
					}
				}
			}

			if (secTags != NULL) { MEM_free(secTags); }
		}

		//for specification relation
		tag_t specificationRelTag = NULLTAG;
		GRM_find_relation_type("IMAN_specification", &specificationRelTag);		
		if (specificationRelTag != NULLTAG)
		{


			GRM_list_secondary_objects_only(tSrcRev, specificationRelTag, &secCount, &secTags);

			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							//char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(secTags[secIdx], "object_name", &itemID);

								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);

									//Attach dataset to target rev if excel type...
									AE_copy_dataset_with_id(secTags[secIdx],
										newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);

									if (tNewDataset != NULLTAG)
									{
										//tag_t newRelTag = NULLTAG;

										//create relation with new dataset on new revision...													
										GRM_create_relation(*tNewRev, tNewDataset, specificationRelTag, NULL, &newRelTag);

										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}

								if (itemID != NULL) { MEM_free(itemID); }

							}

							if (datasetName != NULL) { MEM_free(datasetName); }


						}

						if (className != NULL) { MEM_free(className); }
					}
				}
			}

			if (secTags != NULL) { MEM_free(secTags); }
		}


		//for PCB & schematic Object
		CHECK_RETURN(GRM_find_relation_type("EDAHasSchematic", &tSchematic_Rel));
		CHECK_RETURN(GRM_find_relation_type("EDAHasPWB", &tPWB_Rel));
		CHECK_RETURN(GRM_list_secondary_objects_only(tSrcRev, tPWB_Rel, &iPwb_count, &tpwb_Objs));
		if (iPwb_count > 0 && tpwb_Objs != NULL)
		{
			for (int secIdx = 0; secIdx < iPwb_count; secIdx++)
			{
				CHECK_RETURN(AOM_ask_value_string(tpwb_Objs[secIdx], "object_type", &cSec_type));
				cout << "cSec_type:" << cSec_type << endl;
				TC_write_syslog("\n cSec_type: %s \n", cSec_type);
				if (tc_strcmp(cSec_type, "M4MPcbRevision") == 0)
				{
					//create baseline rev pcb
					//newbaseline rev will attach to tNewRev
					
					CHECK_RETURN(ITEM_baseline_rev(tpwb_Objs[secIdx], baselineRevID, baselineDesc, processname, jobname, jobdesc,&tNew_revObj,&iCount,&tDeep_copied_objects));
					
					//Added method for release status on datasets		
					Release_datasets(tNew_revObj);
					//Added logic for release status on new baseline rev for PCB object
					tag_t* tNew_revObjs = NULL;
					tNew_revObjs = &tNew_revObj;
					CHECK_RETURN(RELSTAT_create_release_status("TC Baselined", &tStatus));
					CHECK_RETURN(RELSTAT_add_release_status(tStatus, 1, tNew_revObjs, 0));
					
				}

			}
		}
		tag_t tNew_SchemObj = NULLTAG;
		CHECK_RETURN(GRM_list_secondary_objects_only(tSrcRev, tSchematic_Rel, &iSchematic_count, &tSchematic_Objs));
		if (iSchematic_count > 0 && tSchematic_Objs != NULL)
		{
			for (int j = 0; j < iSchematic_count; j++)
			{
				CHECK_RETURN(AOM_ask_value_string(tSchematic_Objs[j], "object_type", &cSec_type));
				cout << "cSec_type:" << cSec_type << endl;
				if (tc_strcmp(cSec_type, "EDASchem Revision") == 0)
				{
					CHECK_RETURN(ITEM_baseline_rev(tSchematic_Objs[j], baselineRevID, baselineDesc, processname, jobname, jobdesc, &tNew_SchemObj, &iCount, &tDeep_copied_objects));
					//Added method for release status on datasets		
					Release_datasets(tNew_SchemObj);

					//Added logic for release status on new baseline rev for schematic object
					tag_t* tNew_SchemObjs = NULL;
					tNew_SchemObjs = &tNew_SchemObj;
					CHECK_RETURN(RELSTAT_create_release_status("TC Baselined", &tStatus));
					CHECK_RETURN(RELSTAT_add_release_status(tStatus, 1, tNew_SchemObjs, 0));
					
				}
			}
			
		}

		/*if (tpwb_Objs != NULL || tSchematic_Objs != NULL)
		{
			Baseline_dataset(tSchematic_Objs, iSchematic_count, tpwb_Objs, iPwb_count, baselineRevID, tNew_SchemObj, tNew_revObj, tNewRev);
		}*/
		
	}
	SAFE_SM_FREE(cSec_type);
	SAFE_SM_FREE(tpwb_Objs);
	SAFE_SM_FREE(cNew_revObjName);
	return ifail;
}
/*int Baseline_dataset_PCB(tag_t tpwb_Objs, const char* baselineRevID, tag_t tNew_revObj)
{
	int ifail = ITK_ok;
	tag_t specificationRelTag = NULLTAG;
	tag_t DerivedRelTag = NULLTAG;
	int secCount = 0;
	tag_t* secTags = NULL;
	tag_t newRelTag = NULLTAG;
	tag_t class_id = NULLTAG;
	tag_t tNewDataset = NULLTAG;
	char* datasetName = NULL;
	char* itemID = NULL;
	char* className = NULL;
	tag_t* tNewDatasetList = NULL;
	char* cReleaseStatus = NULL;
	tag_t tStatus = NULLTAG;
	int iRelStatusCount = 0;
	tag_t* tReleaseStatus = NULL;
	GRM_find_relation_type("IMAN_specification", &specificationRelTag);
	GRM_find_relation_type("EDAHasDerivedDataset", &DerivedRelTag);
	if (specificationRelTag != NULLTAG)
	{
		GRM_list_secondary_objects_only(tpwb_Objs, specificationRelTag, &secCount, &secTags);
		if (secCount > 0 && secTags != NULL)
		{
			for (int secIdx = 0; secIdx < secCount; secIdx++)
			{
				if (secTags[secIdx] != NULLTAG)
				{
					//char* className = NULL;
					//tag_t class_id = NULLTAG;

					//check object Type
					POM_class_of_instance(secTags[secIdx], &class_id);
					POM_name_of_class(class_id, &className);
					cout << "className:" << className << endl;
					TC_write_syslog("\n className: %s \n", className);

					if (tc_strcmp(className, "Dataset") == 0)
					{
						//char* datasetName = NULL;

						AOM_ask_name(secTags[secIdx], &datasetName);
						cout << "datasetName:" << datasetName << endl;
						TC_write_syslog("\n datasetName: %s \n", datasetName);

						if (datasetName != NULL && tc_strlen(datasetName) > 0)
						{
							//char* itemID = NULL;

							//tag_t tNewDataset = NULLTAG;

							//get Item ID
							AOM_ask_value_string(tpwb_Objs, "object_name", &itemID);
							cout << "itemID:" << itemID << endl;
							TC_write_syslog("\n itemID: %s \n", itemID);
							if (itemID != NULL && tc_strlen(itemID) > 0)
							{
								string newDatasetName = "";
								newDatasetName.assign(itemID);
								newDatasetName.append("/");
								newDatasetName.append(baselineRevID);


								CHECK_RETURN(AE_copy_dataset_with_id(secTags[secIdx], newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset));
								if (tNewDataset != NULLTAG)
								{
									//tag_t newRelTag = NULLTAG;

									//create relation with new dataset on new revision...													
									GRM_create_relation(tNew_revObj, tNewDataset, specificationRelTag, NULL, &newRelTag);

									if (newRelTag != NULLTAG)
									{
										GRM_save_relation(newRelTag);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (DerivedRelTag != NULLTAG)
	{
		GRM_list_secondary_objects_only(tpwb_Objs, DerivedRelTag, &secCount, &secTags);
		if (secCount > 0 && secTags != NULL)
		{
			for (int secIdx = 0; secIdx < secCount; secIdx++)
			{
				if (secTags[secIdx] != NULLTAG)
				{
					//char* className = NULL;
					//tag_t class_id = NULLTAG;

					//check object Type
					POM_class_of_instance(secTags[secIdx], &class_id);
					POM_name_of_class(class_id, &className);
					cout << "className:" << className << endl;
					TC_write_syslog("\n className: %s \n", className);

					if (tc_strcmp(className, "Dataset") == 0)
					{
						//char* datasetName = NULL;

						AOM_ask_name(secTags[secIdx], &datasetName);
						cout << "datasetName:" << datasetName << endl;
						TC_write_syslog("\n datasetName: %s \n", datasetName);

						if (datasetName != NULL && tc_strlen(datasetName) > 0)
						{
							//char* itemID = NULL;

							//tag_t tNewDataset = NULLTAG;

							//get Item ID
							AOM_ask_value_string(tpwb_Objs, "object_name", &itemID);
							cout << "itemID:" << itemID << endl;
							TC_write_syslog("\n itemID: %s \n", itemID);
							if (itemID != NULL && tc_strlen(itemID) > 0)
							{
								string newDatasetName = "";
								newDatasetName.assign(itemID);
								newDatasetName.append("/");
								newDatasetName.append(baselineRevID);


								AE_copy_dataset_with_id(secTags[secIdx], newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);
								if (tNewDataset != NULLTAG)
								{
									//tag_t newRelTag = NULLTAG;

									//create relation with new dataset on new revision...													
									GRM_create_relation(tNew_revObj, tNewDataset, DerivedRelTag, NULL, &newRelTag);

									if (newRelTag != NULLTAG)
									{
										GRM_save_relation(newRelTag);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (itemID != NULL) { MEM_free(itemID); }
	if (datasetName != NULL) { MEM_free(datasetName); }
	if (className != NULL) { MEM_free(className); }
	if (secTags != NULL) { MEM_free(secTags); }
	return ifail;
}*/

/*int Baseline_dataset(tag_t* tSchematic_Objs, int iSchematic_count, tag_t* tpwb_Objs, int iPwb_count, const char* baselineRevID, tag_t tNew_SchemObj, tag_t tNew_revObj, tag_t* tNewRev)
{
	int ifail = ITK_ok;
	tag_t specificationRelTag = NULLTAG;
	tag_t DerivedRelTag = NULLTAG;
	int secCount = 0;
	tag_t* secTags = NULL;
	tag_t newRelTag = NULLTAG;
	tag_t class_id = NULLTAG;
	tag_t tNewDataset = NULLTAG;
	char* datasetName = NULL;
	char* itemID = NULL;
	char* className = NULL;
	tag_t* tNewDatasetList = NULL;
	char* cReleaseStatus = NULL;
	tag_t tStatus = NULLTAG;
	int iRelStatusCount = 0;
	tag_t* tReleaseStatus = NULL;
	GRM_find_relation_type("IMAN_specification", &specificationRelTag);
	GRM_find_relation_type("EDAHasDerivedDataset", &DerivedRelTag);
	if (specificationRelTag != NULLTAG)
	{
		WSOM_ask_release_status_list(tNew_SchemObj, &iRelStatusCount, &tReleaseStatus);
		//CHECK_RETURN(AOM_ask_value_tags(tNew_SchemObj, "release_status_list", &iRelStatusCount, &tReleaseStatus));
		cout << "iRelStatusCount1:" << iRelStatusCount << endl;
		for (int i = 0; i < iSchematic_count; i++)
		{
			
			//WSOM_ask_release_status_list(tSchematic_Objs[i], &iRelStatusCount, &tReleaseStatus);
			GRM_list_secondary_objects_only(tSchematic_Objs[i], specificationRelTag, &secCount, &secTags);
			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);
						cout << "className:" << className << endl;
						TC_write_syslog("\n className: %s \n", className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							//char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);
							cout << "datasetName:" << datasetName << endl;
							TC_write_syslog("\n datasetName: %s \n", datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(tSchematic_Objs[i], "object_name", &itemID);
								cout << "itemID:" << itemID << endl;
								TC_write_syslog("\n itemID: %s \n", itemID);
								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);

									
									CHECK_RETURN(AE_copy_dataset_with_id(secTags[secIdx], newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset));
									if (tNewDataset != NULLTAG)
									{
										
										
										//tag_t newRelTag = NULLTAG;
										tNewDatasetList =&tNewDataset;
										cout << "111..." << endl;
										if (iRelStatusCount > 0)
										{
											cout << "before cReleaseStatus..." << endl;
											CHECK_RETURN(AOM_ask_value_string(tReleaseStatus[0], "object_name", &cReleaseStatus));
											cout << "cReleaseStatus:" << cReleaseStatus << endl;
											TC_write_syslog("\n cReleaseStatus: %s \n", cReleaseStatus);
											CHECK_RETURN(RELSTAT_create_release_status(cReleaseStatus, &tStatus));
											CHECK_RETURN(RELSTAT_add_release_status(tReleaseStatus[0], 1, tNewDatasetList, 0));
											//create relation with new dataset on new revision...													
											GRM_create_relation(tNew_SchemObj, tNewDataset, specificationRelTag, NULL, &newRelTag);
										}
										
										
										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		for (int ip = 0; ip < iPwb_count; ip++)
		{
			GRM_list_secondary_objects_only(tpwb_Objs[ip], specificationRelTag, &secCount, &secTags);
			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);
						cout << "className:" << className << endl;
						TC_write_syslog("\n className: %s \n", className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							//char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);
							cout << "datasetName:" << datasetName << endl;
							TC_write_syslog("\n datasetName: %s \n", datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(tpwb_Objs[ip], "object_name", &itemID);
								cout << "itemID:" << itemID << endl;
								TC_write_syslog("\n itemID: %s \n", itemID);
								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);


									CHECK_RETURN(AE_copy_dataset_with_id(secTags[secIdx], newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset));
									if (tNewDataset != NULLTAG)
									{
										//tag_t newRelTag = NULLTAG;

										//create relation with new dataset on new revision...													
										GRM_create_relation(tNew_revObj, tNewDataset, specificationRelTag, NULL, &newRelTag);

										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (DerivedRelTag != NULLTAG)
	{

		for (int i = 0; i < iSchematic_count; i++)
		{
			GRM_list_secondary_objects_only(tSchematic_Objs[i], DerivedRelTag, &secCount, &secTags);
			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);
						cout << "className:" << className << endl;
						TC_write_syslog("\n className: %s \n", className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							//char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);
							cout << "datasetName:" << datasetName << endl;
							TC_write_syslog("\n datasetName: %s \n", datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(tSchematic_Objs[i], "object_name", &itemID);
								cout << "itemID:" << itemID << endl;
								TC_write_syslog("\n itemID: %s \n", itemID);
								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);


									AE_copy_dataset_with_id(secTags[secIdx], newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);
									if (tNewDataset != NULLTAG)
									{
										//tag_t newRelTag = NULLTAG;

										//create relation with new dataset on new revision...													
										GRM_create_relation(tNew_SchemObj, tNewDataset, DerivedRelTag, NULL, &newRelTag);

										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}
							}
						}
					}
				}
			}
		}

		for (int ip = 0; ip < iPwb_count; ip++)
		{
			GRM_list_secondary_objects_only(tpwb_Objs[ip], DerivedRelTag, &secCount, &secTags);
			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);
						cout << "className:" << className << endl;
						TC_write_syslog("\n className: %s \n", className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							//char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);
							cout << "datasetName:" << datasetName << endl;
							TC_write_syslog("\n datasetName: %s \n", datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(tpwb_Objs[ip], "object_name", &itemID);
								cout << "itemID:" << itemID << endl;
								TC_write_syslog("\n itemID: %s \n", itemID);
								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);


									AE_copy_dataset_with_id(secTags[secIdx], newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);
									if (tNewDataset != NULLTAG)
									{
										//tag_t newRelTag = NULLTAG;

										//create relation with new dataset on new revision...													
										GRM_create_relation(tNew_revObj, tNewDataset, DerivedRelTag, NULL, &newRelTag);

										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	if (itemID != NULL) { MEM_free(itemID); }
	if (datasetName != NULL) { MEM_free(datasetName); }
	if (className != NULL) { MEM_free(className); }
	if (secTags != NULL) { MEM_free(secTags); }
	return ifail;
}*/

/*int Baseline_dataset_Schematic(tag_t tSchematic_Objs, const char* baselineRevID, tag_t tNew_SchemObj)
{
	int ifail = ITK_ok;
	tag_t specificationRelTag = NULLTAG;
	tag_t DerivedRelTag = NULLTAG;
	int secCount = 0;
	tag_t* secTags = NULL;
	tag_t newRelTag = NULLTAG;
	tag_t class_id = NULLTAG;
	tag_t tNewDataset = NULLTAG;
	char* datasetName = NULL;
	char* itemID = NULL;
	char* className = NULL;
	tag_t* tNewDatasetList = NULL;
	char* cReleaseStatus = NULL;
	tag_t tStatus = NULLTAG;
	int iRelStatusCount = 0;
	tag_t* tReleaseStatus = NULL;
	GRM_find_relation_type("IMAN_specification", &specificationRelTag);
	GRM_find_relation_type("EDAHasDerivedDataset", &DerivedRelTag);
	if (specificationRelTag != NULLTAG)
	{
		GRM_list_secondary_objects_only(tSchematic_Objs, specificationRelTag, &secCount, &secTags);
		if (secCount > 0 && secTags != NULL)
		{
			for (int secIdx = 0; secIdx < secCount; secIdx++)
			{
				if (secTags[secIdx] != NULLTAG)
				{
					POM_class_of_instance(secTags[secIdx], &class_id);
					POM_name_of_class(class_id, &className);
					cout << "className:" << className << endl;
					TC_write_syslog("\n className: %s \n", className);
					if (tc_strcmp(className, "Dataset") == 0)
					{
						//char* datasetName = NULL;

						AOM_ask_name(secTags[secIdx], &datasetName);
						cout << "datasetName:" << datasetName << endl;
						TC_write_syslog("\n datasetName: %s \n", datasetName);
						if (datasetName != NULL && tc_strlen(datasetName) > 0)
						{
							//char* itemID = NULL;

							//tag_t tNewDataset = NULLTAG;

							//get Item ID
							AOM_ask_value_string(tSchematic_Objs, "object_name", &itemID);
							cout << "itemID:" << itemID << endl;
							TC_write_syslog("\n itemID: %s \n", itemID);
							if (itemID != NULL && tc_strlen(itemID) > 0)
							{
								string newDatasetName = "";
								newDatasetName.assign(itemID);
								newDatasetName.append("/");
								newDatasetName.append(baselineRevID);


								CHECK_RETURN(AE_copy_dataset_with_id(secTags[secIdx], newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset));
								if (tNewDataset != NULLTAG)
								{


									//tag_t newRelTag = NULLTAG;
									tNewDatasetList = &tNewDataset;
									cout << "111..." << endl;
									
										//cout << "before cReleaseStatus..." << endl;
										//CHECK_RETURN(AOM_ask_value_string(tReleaseStatus[0], "object_name", &cReleaseStatus));
										//cout << "cReleaseStatus:" << cReleaseStatus << endl;
										//TC_write_syslog("\n cReleaseStatus: %s \n", cReleaseStatus);
										CHECK_RETURN(RELSTAT_create_release_status("TC Baselined", &tStatus));
										CHECK_RETURN(RELSTAT_add_release_status(tStatus, 1, tNewDatasetList, 0));
										//create relation with new dataset on new revision...													
										GRM_create_relation(tNew_SchemObj, tNewDataset, specificationRelTag, NULL, &newRelTag);
									


									if (newRelTag != NULLTAG)
									{
										GRM_save_relation(newRelTag);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	if (DerivedRelTag != NULLTAG)
	{


		GRM_list_secondary_objects_only(tSchematic_Objs, DerivedRelTag, &secCount, &secTags);
		if (secCount > 0 && secTags != NULL)
		{
			for (int secIdx = 0; secIdx < secCount; secIdx++)
			{
				if (secTags[secIdx] != NULLTAG)
				{
					//char* className = NULL;
					//tag_t class_id = NULLTAG;

					//check object Type
					POM_class_of_instance(secTags[secIdx], &class_id);
					POM_name_of_class(class_id, &className);
					cout << "className:" << className << endl;
					TC_write_syslog("\n className: %s \n", className);

					if (tc_strcmp(className, "Dataset") == 0)
					{
						//char* datasetName = NULL;

						AOM_ask_name(secTags[secIdx], &datasetName);
						cout << "datasetName:" << datasetName << endl;
						TC_write_syslog("\n datasetName: %s \n", datasetName);

						if (datasetName != NULL && tc_strlen(datasetName) > 0)
						{
							//char* itemID = NULL;

							//tag_t tNewDataset = NULLTAG;

							//get Item ID
							AOM_ask_value_string(tSchematic_Objs, "object_name", &itemID);
							cout << "itemID:" << itemID << endl;
							TC_write_syslog("\n itemID: %s \n", itemID);
							if (itemID != NULL && tc_strlen(itemID) > 0)
							{
								string newDatasetName = "";
								newDatasetName.assign(itemID);
								newDatasetName.append("/");
								newDatasetName.append(baselineRevID);


								AE_copy_dataset_with_id(secTags[secIdx], newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);
								if (tNewDataset != NULLTAG)
								{
									//tag_t newRelTag = NULLTAG;

									//create relation with new dataset on new revision...													
									GRM_create_relation(tNew_SchemObj, tNewDataset, DerivedRelTag, NULL, &newRelTag);

									if (newRelTag != NULLTAG)
									{
										GRM_save_relation(newRelTag);
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return ifail;

}*/

int Release_datasets(tag_t tNew_objs)
{
	//cout << "inside Release_datasets..."<< endl;
	TC_write_syslog("\n inside Release_datasets........ \n");
	int ifail = ITK_ok;
	tag_t specificationRelTag = NULLTAG;
	tag_t DerivedRelTag = NULLTAG;
	int secCount = 0;
	tag_t* secTags = NULL;
	tag_t tStatus = NULLTAG;
	GRM_find_relation_type("IMAN_specification", &specificationRelTag);
	GRM_find_relation_type("EDAHasDerivedDataset", &DerivedRelTag);
	if (specificationRelTag != NULLTAG)
	{
		CHECK_RETURN(GRM_list_secondary_objects_only(tNew_objs, specificationRelTag, &secCount, &secTags));
		if (secCount > 0 && secTags != NULL)
		{
			CHECK_RETURN(RELSTAT_create_release_status("TC Baselined", &tStatus));
			CHECK_RETURN(RELSTAT_add_release_status(tStatus, 1, secTags, 0));
		}
	}
	if (DerivedRelTag != NULLTAG)
	{
		CHECK_RETURN(GRM_list_secondary_objects_only(tNew_objs, DerivedRelTag, &secCount, &secTags));
		if (secCount > 0 && secTags != NULL)
		{
			CHECK_RETURN(RELSTAT_create_release_status("TC Baselined", &tStatus));
			CHECK_RETURN(RELSTAT_add_release_status(tStatus, 1, secTags, 0));
		}
	}
	TC_write_syslog("\n End Release_datasets..... \n");
	//cout << "End Release_datasets..." << endl;
	return ifail;
}
int PCB_Baseline_Post_action(METHOD_message_t* msg, va_list args)
{
	int ifail = ITK_ok;
	tag_t specificationRelTag = NULLTAG;
	tag_t DerivedRelTag = NULLTAG;
	int secCount = 0;
	tag_t* secTags = NULL;
	tag_t newRelTag = NULLTAG;
	char* datasetName = NULL;
	char* className = NULL;
	tag_t class_id = NULLTAG;
	char* itemID = NULL;
	tag_t tNewDataset = NULLTAG;
	va_list largs;
	va_copy(largs, args);
	tag_t tSrcRev = va_arg(largs, tag_t);
	const char* baselineRevID = va_arg(largs, const char*);
	const char* baselineDesc = va_arg(largs, const char*);
	const char* processname = va_arg(largs, const char*);
	const char* jobname = va_arg(largs, const char*);
	const char* jobdesc = va_arg(largs, const char*);
	logical dryrun = va_arg(largs, logical);
	tag_t* tNewRev = va_arg(largs, tag_t*);
	va_end(largs);
	char* cItem_id = NULL;
	cout << "baselineRevID:" << baselineRevID << endl;
	cout << "processname:" << processname << endl;
	cout << "jobname:" << jobname << endl;
	cout << "jobdesc:" << jobdesc << endl;
	CHECK_RETURN(AOM_ask_value_string(tSrcRev, "item_id", &cItem_id));
	cout << "cItem_id:" << cItem_id << endl;
	//CHECK_RETURN(AOM_ask_value_string(tNewRev[0], "item_id", &cItem_id));
	//cout << "cItem_id_tNewRev:" << cItem_id << endl;
	if (tc_strlen(baselineRevID) > 4
		&& tc_strstr(baselineRevID, ".") != NULL && *tNewRev != NULLTAG)
	{
		tag_t referenceRelTag = NULLTAG;

	
		GRM_find_relation_type("IMAN_specification", &specificationRelTag);
		GRM_find_relation_type("EDAHasDerivedDataset", &DerivedRelTag);

		//for specification relation
		if (specificationRelTag != NULLTAG)
		{
			

			GRM_list_secondary_objects_only(tSrcRev, specificationRelTag, &secCount, &secTags);

			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);

						if (tc_strcmp(className, "Dataset") == 0)
						{														
									//char* datasetName = NULL;

									AOM_ask_name(secTags[secIdx], &datasetName);

									if (datasetName != NULL && tc_strlen(datasetName) > 0)
									{
										//char* itemID = NULL;

										//tag_t tNewDataset = NULLTAG;

										//get Item ID
										AOM_ask_value_string(secTags[secIdx], "object_name", &itemID);

										if (itemID != NULL && tc_strlen(itemID) > 0)
										{
											string newDatasetName = "";
											newDatasetName.assign(itemID);
											newDatasetName.append("/");
											newDatasetName.append(baselineRevID);

											//Attach dataset to target rev if excel type...
											AE_copy_dataset_with_id(secTags[secIdx],
												newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);

											if (tNewDataset != NULLTAG)
											{
												//tag_t newRelTag = NULLTAG;

												//create relation with new dataset on new revision...													
												GRM_create_relation(*tNewRev, tNewDataset, specificationRelTag, NULL, &newRelTag);

												if (newRelTag != NULLTAG)
												{
													GRM_save_relation(newRelTag);
												}
											}
										}

										if (itemID != NULL) { MEM_free(itemID); }

									}

									if (datasetName != NULL) { MEM_free(datasetName); }
															

						}

						if (className != NULL) { MEM_free(className); }
					}
				}
			}

			if (secTags != NULL) { MEM_free(secTags); }
		}

		//for EDA has serived datasetrelation  relation
		if (DerivedRelTag != NULLTAG)
		{


			GRM_list_secondary_objects_only(tSrcRev, DerivedRelTag, &secCount, &secTags);

			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(secTags[secIdx], "object_name", &itemID);

								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);

									//Attach dataset to target rev if excel type...
									AE_copy_dataset_with_id(secTags[secIdx],
										newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);

									if (tNewDataset != NULLTAG)
									{
										//tag_t newRelTag = NULLTAG;

										//create relation with new dataset on new revision...													
										GRM_create_relation(*tNewRev, tNewDataset, DerivedRelTag, NULL, &newRelTag);

										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}

								if (itemID != NULL) { MEM_free(itemID); }

							}

							if (datasetName != NULL) { MEM_free(datasetName); }


						}

						if (className != NULL) { MEM_free(className); }
					}
				}
			}

			if (secTags != NULL) { MEM_free(secTags); }
		}
	}
	return ifail;
}
int Schematic_Baseline_Post_action(METHOD_message_t* msg, va_list args)
{
	int ifail = ITK_ok;
	tag_t specificationRelTag = NULLTAG;
	tag_t DerivedRelTag = NULLTAG;
	int secCount = 0;
	tag_t* secTags = NULL;
	tag_t newRelTag = NULLTAG;
	char* datasetName = NULL;
	char* className = NULL;
	tag_t class_id = NULLTAG;
	char* itemID = NULL;
	tag_t tNewDataset = NULLTAG;
	va_list largs;
	va_copy(largs, args);
	tag_t tSrcRev = va_arg(largs, tag_t);
	const char* baselineRevID = va_arg(largs, const char*);
	const char* baselineDesc = va_arg(largs, const char*);
	const char* processname = va_arg(largs, const char*);
	const char* jobname = va_arg(largs, const char*);
	const char* jobdesc = va_arg(largs, const char*);
	logical dryrun = va_arg(largs, logical);
	tag_t* tNewRev = va_arg(largs, tag_t*);
	va_end(largs);
	char* cItem_id = NULL;
	cout << "baselineRevID:" << baselineRevID << endl;
	cout << "processname:" << processname << endl;
	cout << "jobname:" << jobname << endl;
	cout << "jobdesc:" << jobdesc << endl;
	CHECK_RETURN(AOM_ask_value_string(tSrcRev, "item_id", &cItem_id));
	cout << "cItem_id:" << cItem_id << endl;
	CHECK_RETURN(AOM_ask_value_string(tNewRev[0], "item_id", &cItem_id));
	cout << "cItem_id_tNewRev:" << cItem_id << endl;
	if (tc_strlen(baselineRevID) > 4
		&& tc_strstr(baselineRevID, ".") != NULL && *tNewRev != NULLTAG)
	{
		tag_t referenceRelTag = NULLTAG;


		GRM_find_relation_type("IMAN_specification", &specificationRelTag);
		GRM_find_relation_type("EDAHasDerivedDataset", &DerivedRelTag);

		//for specification relation
		if (specificationRelTag != NULLTAG)
		{


			GRM_list_secondary_objects_only(tSrcRev, specificationRelTag, &secCount, &secTags);

			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							//char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(secTags[secIdx], "object_name", &itemID);

								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);

									//Attach dataset to target rev if excel type...
									AE_copy_dataset_with_id(secTags[secIdx],
										newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);

									if (tNewDataset != NULLTAG)
									{
										//tag_t newRelTag = NULLTAG;

										//create relation with new dataset on new revision...													
										GRM_create_relation(*tNewRev, tNewDataset, specificationRelTag, NULL, &newRelTag);

										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}

								if (itemID != NULL) { MEM_free(itemID); }

							}

							if (datasetName != NULL) { MEM_free(datasetName); }


						}

						if (className != NULL) { MEM_free(className); }
					}
				}
			}

			if (secTags != NULL) { MEM_free(secTags); }
		}

		//for EDA has serived datasetrelation  relation
		if (DerivedRelTag != NULLTAG)
		{


			GRM_list_secondary_objects_only(tSrcRev, DerivedRelTag, &secCount, &secTags);

			if (secCount > 0 && secTags != NULL)
			{
				for (int secIdx = 0; secIdx < secCount; secIdx++)
				{
					if (secTags[secIdx] != NULLTAG)
					{
						//char* className = NULL;
						//tag_t class_id = NULLTAG;

						//check object Type
						POM_class_of_instance(secTags[secIdx], &class_id);
						POM_name_of_class(class_id, &className);

						if (tc_strcmp(className, "Dataset") == 0)
						{
							char* datasetName = NULL;

							AOM_ask_name(secTags[secIdx], &datasetName);

							if (datasetName != NULL && tc_strlen(datasetName) > 0)
							{
								//char* itemID = NULL;

								//tag_t tNewDataset = NULLTAG;

								//get Item ID
								AOM_ask_value_string(secTags[secIdx], "object_name", &itemID);

								if (itemID != NULL && tc_strlen(itemID) > 0)
								{
									string newDatasetName = "";
									newDatasetName.assign(itemID);
									newDatasetName.append("/");
									newDatasetName.append(baselineRevID);

									//Attach dataset to target rev if excel type...
									AE_copy_dataset_with_id(secTags[secIdx],
										newDatasetName.c_str(), newDatasetName.c_str(), "", &tNewDataset);

									if (tNewDataset != NULLTAG)
									{
										//tag_t newRelTag = NULLTAG;

										//create relation with new dataset on new revision...													
										GRM_create_relation(*tNewRev, tNewDataset, DerivedRelTag, NULL, &newRelTag);

										if (newRelTag != NULLTAG)
										{
											GRM_save_relation(newRelTag);
										}
									}
								}

								if (itemID != NULL) { MEM_free(itemID); }

							}

							if (datasetName != NULL) { MEM_free(datasetName); }


						}

						if (className != NULL) { MEM_free(className); }
					}
				}
			}

			if (secTags != NULL) { MEM_free(secTags); }
		}
	}
	return ifail;
}