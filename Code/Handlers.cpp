#include "Hanler_header.h"

extern "C" DLLAPI int Handlers_register_callbacks()
{
	int ifail = ITK_ok;
	CHECK_RETURN(CUSTOM_register_exit("Handlers", USER_INIT_MODULE, (CUSTOM_EXIT_ftn_t)CUSTOM_register_extension));
	return ifail;
}
extern DLLAPI int CUSTOM_register_extension(int* decision, va_list argc)
{
	int ifail = ITK_ok;
	*decision = ALL_CUSTOMIZATIONS;

	printf("welcome to Handlers..\n");
	CHECK_RETURN(EPM_register_rule_handler("Check_clustered_material", "checking for clustered material on the basis of Property", (EPM_rule_handler_t)Check_clustered_material));

	CHECK_RETURN(EPM_register_action_handler("Replace_TargetAttchment_with_NewRev", "On the besis of decision if yes creates minor rev with status else create status", (EPM_action_handler_t)Replace_TargetAttchment));
	//CHECK_RETURN(EPM_register_rule_handler("Check_clustered_material", "checking for clustered material on the basis of Property", (EPM_Rule_handler_t)Check_clustered_material));
	return ifail;
}





	/*************************************************************************************
	Function Name         : Replace_TargetAttchment_with_NewRev
	Function Description  : On the basis of newly created Minor revision handler will replace Target attachment with new Minor revision, Major Rev not allowed.



	Requirement			  : Ajay Rajurker
	--------------------------------------------------------------------------------------
	History:
	--------------------------------------------------------------------------------------
	Date					Modified By					Versions
	5th Oct 2023		  SWAPNIL KALKOTE		initial creation



	*************************************************************************************/
int Replace_TargetAttchment(EPM_action_message_t msg)
{
	int ifail                      = ITK_ok;
	int iCount                     = 0;
	int   iLvCount                 = 0;
	int iCnt                       = 0;
	tag_t troot_task               = NULLTAG;
	tag_t task                     = NULLTAG;
	tag_t tEDAComPartTag           = NULLTAG;
	tag_t tEDAComPartLatest_RevTag = NULLTAG;
	char* cTaskName                = NULL;
	char* cattachmentType          = NULL;
	char* cEDAComPart_RevID        = NULL;
	char* cEDAComPart_ID           = NULL;
	char* cEDAComPart_LatestRevID  = NULL;
	char* cName                    = NULL;
	char* cType                    = NULL;
	char* cItem_id                 = NULL;
	char* cSubProcess_name         = NULL;
	tag_t* tattachment             = NULL;
	char** spEntries               = NULL;
	char** spValues                = NULL;
	const char* itemType           = "EDAComPart";
	tag_t tQry                     = NULLTAG;
	tag_t troot_task_Parent        = NULLTAG;
	tag_t tSubProcess              = NULLTAG;
	tag_t tParentTask              = NULLTAG;
	tag_t* tQueryResult            = NULL;
	tag_t* tParentProcess          = NULL;
	size_t iLength_ID              = 0;
	vector<tag_t>vAttach_to_Target_obj;
	vector<tag_t>vRemove_from_Target_obj;
	cout << "Entered in Replace_TargetAttchment" << endl;
	TC_write_syslog("\n Entered in Replace_TargetAttchment.. \n");
	CHECK_RETURN(EPM_ask_root_task(msg.task, &troot_task));
	task = msg.task;
	CHECK_RETURN(EPM_ask_job(troot_task, &tSubProcess));
		

	CHECK_RETURN(EPM_ask_parent_processes(tSubProcess, &iCnt, &tParentProcess));
	
	//char* cSubProcessName = NULL;
	//CHECK_RETURN(EPM_ask_procedure_name2(tSubProcess, &cSubProcessName));
	//cout << "cSubProcessName:" << cSubProcessName << endl;

	//char* cParentProcess_Name = NULL;
	//CHECK_RETURN(EPM_ask_procedure_name2(tParentProcess[0], &cParentProcess_Name));
	//cout << "cParentProcess_Name:" << cParentProcess_Name << endl;

	if (iCnt > 0)
	{
		CHECK_RETURN(EPM_ask_root_task(tParentProcess[0], &troot_task_Parent));
	}
	
	

	CHECK_RETURN(EPM_ask_attachments(troot_task, EPM_target_attachment, &iCount, &tattachment));//find target attachment
	//cout << iCount << endl;
	for (int i = 0; i < iCount; i++) //loop for target attachments
	{
		CHECK_RETURN(WSOM_ask_object_type2(tattachment[i], &cattachmentType));
		cout << cattachmentType << endl;
		TC_write_syslog("\n ObjectType: %s \n", cattachmentType);
		if (tc_strcmp(cattachmentType, "EDAComPart Revision") != 0)//Bug Fix
		{

			CHECK_RETURN(AOM_ask_value_string(tattachment[i], "item_revision_id", &cEDAComPart_RevID));//get Rev Id
			//cout << "rev_id=" << cEDAComPart_RevID << endl;
			//TC_write_syslog("rev_id: %s \n", cEDAComPart_RevID);
			CHECK_RETURN(AOM_ask_value_string(tattachment[i], "item_id", &cEDAComPart_ID));//get Item Id
			//cout << "item_id=" << cEDAComPart_ID << endl;
			//TC_write_syslog("item_id: %s \n", cEDAComPart_ID);

			//Query for EDAComPart tag
			spEntries = (char**)MEM_alloc(sizeof(char*) * 2);
			spValues = (char**)MEM_alloc(sizeof(char*) * 2);

			spEntries[0] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen("Item ID") + 1));
			spValues[0] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(cEDAComPart_ID) + 1));

			spEntries[1] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen("Type") + 1));
			spValues[1] = (char*)MEM_alloc(sizeof(char) * ((int)tc_strlen(itemType) + 1));

			tc_strcpy(spEntries[0], "");
			tc_strcpy(spEntries[0], "Item ID");

			tc_strcpy(spValues[0], "");
			tc_strcpy(spValues[0], cEDAComPart_ID);

			tc_strcpy(spEntries[1], "");
			tc_strcpy(spEntries[1], "Type");

			tc_strcpy(spValues[1], "");
			tc_strcpy(spValues[1], itemType);
			CHECK_RETURN(QRY_find2("Item...", &tQry));
			CHECK_RETURN(QRY_execute(tQry, 2, spEntries, spValues, &iLvCount, &tQueryResult));
			if (iLvCount > 0)
			{
				CHECK_RETURN(WSOM_ask_object_type2(tQueryResult[0], &cType));
				//cout << "cType:" << cType << endl;
				CHECK_RETURN(ITEM_ask_latest_rev(tQueryResult[0], &tEDAComPartLatest_RevTag));
				CHECK_RETURN(AOM_ask_value_string(tEDAComPartLatest_RevTag, "item_revision_id", &cEDAComPart_LatestRevID));//get Rev Id
				//cout << "LatestRev_id=" << cEDAComPart_LatestRevID << endl;
				TC_write_syslog("LatestRev_id: %s \n", cEDAComPart_LatestRevID);
				iLength_ID = strlen(cEDAComPart_LatestRevID);
				//cout << "IDLength=" << iLength_ID << endl;
				TC_write_syslog("IDLength: %d \n", iLength_ID);
				if (tc_strcmp(cEDAComPart_RevID, cEDAComPart_LatestRevID) != 0)
				{
					if (iLength_ID > 2)
					{
						vAttach_to_Target_obj.push_back(tEDAComPartLatest_RevTag);
						vRemove_from_Target_obj.push_back(tattachment[i]);
					}


				}
			}
		}
	}
	
	int* iAttachmentType = NULL;
	int* iAttachmentType_Ref = NULL;
	counted_tag_list_t  tlNewTargets = { 0 };
	char* cObj_name = NULL;
	int initial_tag_list_size = vAttach_to_Target_obj.size();
	tlNewTargets.list = (tag_t*)MEM_alloc(initial_tag_list_size * (sizeof(tag_t)));
	//cout <<"Before Add:" << "tlNewTargets.count:" << tlNewTargets.count << endl;
	if (vAttach_to_Target_obj.size() > 0)
	{
		for (int Vi = 0; Vi < vAttach_to_Target_obj.size(); Vi++)
		{
			CHECK_RETURN(EPM__add_to_tag_list(vAttach_to_Target_obj[Vi], &tlNewTargets));
			CHECK_RETURN(AOM_ask_value_string(tlNewTargets.list[Vi], "object_name", &cObj_name));
			//cout << "cObj_name:" << cObj_name << endl;

		}
		TC_write_syslog("tlNewTargets.count:: %d \n", tlNewTargets.count);
		//cout << "tlNewTargets.count:" << tlNewTargets.count << endl;
		iAttachmentType = (int*)MEM_alloc(tlNewTargets.count * sizeof(int));
		iAttachmentType_Ref = (int*)MEM_alloc(tlNewTargets.count * sizeof(int));
		for (int ii = 0; ii < tlNewTargets.count; ii++)
		{
			iAttachmentType[ii] = EPM_target_attachment;
			iAttachmentType_Ref[ii] = EPM_reference_attachment;
		}

		CHECK_RETURN(EPM_add_attachments(troot_task, tlNewTargets.count, tlNewTargets.list, iAttachmentType));
		CHECK_RETURN(EPM_add_attachments(troot_task, tlNewTargets.count, tlNewTargets.list, iAttachmentType_Ref));
		if (iCnt > 0)
		{
			CHECK_RETURN(EPM_add_attachments(troot_task_Parent, tlNewTargets.count, tlNewTargets.list, iAttachmentType_Ref));
		}
		
	}

//Remove objects from Target attachments

	counted_tag_list_t  tlNewTargets_remove = { 0 };
	char* cObj_name_remove = NULL;
	int initial_tag_list_size_remove = vRemove_from_Target_obj.size();
	tlNewTargets_remove.list = (tag_t*)MEM_alloc(initial_tag_list_size_remove * (sizeof(tag_t)));
	if (vRemove_from_Target_obj.size() > 0)
	{
		for (int Vr = 0; Vr < vRemove_from_Target_obj.size(); Vr++)
		{
			CHECK_RETURN(EPM__add_to_tag_list(vRemove_from_Target_obj[Vr], &tlNewTargets_remove));
			CHECK_RETURN(AOM_ask_value_string(tlNewTargets_remove.list[Vr], "object_name", &cObj_name_remove));
			//cout << "cObj_name_remove:" << cObj_name_remove << endl;
			
		}
		CHECK_RETURN(EPM_remove_attachments(troot_task, tlNewTargets_remove.count, tlNewTargets_remove.list));
	}
	
	
	SAFE_SM_FREE(tattachment);
	SAFE_SM_FREE(cattachmentType);
	SAFE_SM_FREE(cEDAComPart_RevID);
	SAFE_SM_FREE(cEDAComPart_ID);
	SAFE_SM_FREE(cEDAComPart_LatestRevID);
	SAFE_SM_FREE(tQueryResult);
	SAFE_SM_FREE(cObj_name);
	SAFE_SM_FREE(cObj_name_remove);
	return ifail;
}


/*************************************************************************************
	Function Name         : Check_clustered_material
	Function Description  : checking for clustered material on the basis of Property if Clustered Material attribute is TRUE 
							throw error for list of Clustered materials.(CR354)



	Requirement			  : Ravi Verma
	--------------------------------------------------------------------------------------
	History:
	--------------------------------------------------------------------------------------
	Date					Modified By					Versions
	1st Dec 2023		  SWAPNIL KALKOTE		initial creation



	*************************************************************************************/


EPM_decision_t Check_clustered_material(EPM_rule_message_t msg)
{
	TC_write_syslog("********  Check_clustered_material is trigger ********");
	EPM_decision_t decision = EPM_go;
	int ifail               = ITK_ok;
	int k                   = 0;
	int piAttachTypes       = EPM_target_attachment;
	vector<tag_t>failedtMaterials;
	int iTargetCount        = 0;
	tag_t* tTargets         = NULL;
	tag_t root_task         = NULLTAG;
	char* value             = NULL;
	string error_msg;
	error_msg.assign("\n");
	vector <tag_t> itemRevVector;
	RULE_ERROR_CHECK(EPM_ask_root_task(msg.task, &root_task));
	RULE_ERROR_CHECK(EPM_ask_attachments(root_task, EPM_target_attachment, &iTargetCount, &tTargets));

	for (int tarNum = 0; tarNum < iTargetCount; tarNum++)
	{
		char* targetObjType = NULL;

		RULE_ERROR_CHECK(AOM_ask_value_string(tTargets[tarNum], "object_type", &targetObjType));

			int secCount            = 0;
			tag_t relation_type_tag = NULLTAG;
			tag_t* secObjects       = NULL;

			RULE_ERROR_CHECK(GRM_find_relation_type("CMHasSolutionItem", &relation_type_tag));

			RULE_ERROR_CHECK(GRM_list_secondary_objects_only(tTargets[tarNum], relation_type_tag, &secCount, &secObjects));

			TC_write_syslog("CMHasSolutionItem found - %d ", secCount);
			for (int sCnt = 0; sCnt < secCount; sCnt++)
			{
				char* itemID = NULL;
				char* objtype = NULL;
				char* itemRevName = NULL;
				char* cMaterialRevName = NULL;
				tag_t item_tag = NULLTAG;
				tag_t* bv_list = NULL;
				logical lClusteredMaterial;
				int bvCount = 0;
				
				vector<tag_t> materialTags;
				RULE_ERROR_CHECK(Get_Material_Tags(secObjects[sCnt], materialTags));
				RULE_ERROR_CHECK(AOM_ask_value_string(secObjects[sCnt], "object_string", &itemRevName));

				if (materialTags.size() > 0)
				{
					for (size_t VM = 0; VM < materialTags.size(); VM++)
					{
						RULE_ERROR_CHECK(AOM_ask_value_string(materialTags[VM], "object_string", &cMaterialRevName));
						RULE_ERROR_CHECK(AOM_ask_value_logical(materialTags[VM], "m4is_migrated_item", &lClusteredMaterial));//m4isClustered_material

						if (lClusteredMaterial == true)
						{
							
							failedtMaterials.push_back(materialTags[VM]);
							error_msg.append("Clustered Material is not allowed in Industrial Release, please remove");
							error_msg.append(" ");
							error_msg.append(cMaterialRevName);
							error_msg.append(" ");
							error_msg.append("from");
							error_msg.append(" ");
							error_msg.append(itemRevName);
							error_msg.append("\n");
						}
					}
					
				}


				RULE_ERROR_CHECK(ITEM_ask_item_of_rev(secObjects[sCnt], &item_tag));

				RULE_ERROR_CHECK(ITEM_list_bom_views(item_tag, &bvCount, &bv_list));

				//cout << "Bom view Count - " << bvCount << endl;
				for (int bCnt = 0; bCnt < bvCount; bCnt++)
				{
					tag_t   tag_windows = NULLTAG;
					tag_t   topLine = NULLTAG;
					int		lineCount = 0;
					tag_t* lineList = NULL;

					RULE_ERROR_CHECK(BOM_create_window(&tag_windows));
					RULE_ERROR_CHECK(BOM_set_window_pack_all(tag_windows, TRUE));
					RULE_ERROR_CHECK(BOM_set_window_top_line(tag_windows, item_tag, secObjects[sCnt], NULLTAG, &topLine));
					RULE_ERROR_CHECK(BOM_line_ask_all_child_lines(topLine, &lineCount, &lineList));

					

					for (int cCnt = 0; cCnt < lineCount; cCnt++)
					{
						M4_Get_ItemRevisions_Of_BOM(lineList[cCnt], itemRevVector);
					}

					if (itemRevVector.size() > 0)
					{
						for (int vCnt = 0; vCnt < itemRevVector.size(); vCnt++)
						{
							char* objname       = NULL;
							//char* cParent_item  = NULL;
							tag_t* statusesTags = NULL;
							int statusCnt       = 0;
							int revFound        = 0;
							/*vector<tag_t> chilmaterialTags;
							RULE_ERROR_CHECK(Get_Material_Tags(itemRevVector.at(vCnt), chilmaterialTags));
							if (chilmaterialTags.size() > 0) {
								RULE_ERROR_CHECK(Material_Status_Validation(chilmaterialTags, failedtMaterialsID, statusArgVector));
							}*/
							//RULE_ERROR_CHECK(AOM_ask_value_string(secObjects[sCnt], "object_string", &cParent_item));
							//cout << "cParent_item:" << cParent_item << endl;
							RULE_ERROR_CHECK(AOM_ask_value_string(itemRevVector[vCnt], "object_string", &cMaterialRevName));
							RULE_ERROR_CHECK(AOM_ask_value_logical(itemRevVector[vCnt], "m4is_migrated_item", &lClusteredMaterial));//m4isClustered_material
							if (lClusteredMaterial == true)
							{
										
								failedtMaterials.push_back(itemRevVector[vCnt]);
								error_msg.append("Clustered Material is not allowed in Industrial Release, please remove");
								error_msg.append(" ");
								error_msg.append(cMaterialRevName);
								error_msg.append(" ");
								error_msg.append("from");
								error_msg.append(" ");
								error_msg.append("BOM");
								error_msg.append(" ");
								error_msg.append(itemRevName);
								error_msg.append("\n");
							}
															
						}
					}

					itemRevVector.clear();
					RULE_ERROR_CHECK(BOM_close_window(tag_windows));

					SAFE_SM_FREE(lineList);
					
	
				}
				SAFE_SM_FREE(cMaterialRevName);
				SAFE_SM_FREE(itemRevName);
			}
			
	}
	if (failedtMaterials.size() > 0)
	{
		RULE_ERROR_CHECK(EMH_store_error_s1(EMH_severity_error, -1, error_msg.c_str()));
		decision = EPM_nogo;
	}
	
	return decision;
}

int Get_Material_Tags(tag_t objecttag, vector<tag_t>& materialTags)
{
	TC_write_syslog(" Get_Material_Tags Finding material tags found ");
	int ifail               = ITK_ok;
	tag_t tAlternatMaterial = NULLTAG;
	tag_t tMaterial         = NULLTAG;
	//cout << "----------------- getting materials --------------------" << endl;
	RULE_ERROR_CHECK(GRM_find_relation_type("IMAN_specification", &tAlternatMaterial));//M4alternate_materials
	RULE_ERROR_CHECK(GRM_find_relation_type("Mat1UsesMaterial", &tMaterial));
	
	char* objtype = NULL;
	RULE_ERROR_CHECK(AOM_ask_value_string(objecttag, "object_type", &objtype));
	//cout << "Material object type - " << objtype << endl;
	//if (tc_strcmp(objtype, "M4alternate_materials") == 0 || tc_strcmp(objtype, "Mat1UsesMaterial") == 0 )
	if (tc_strcmp(objtype, "EDAComPart Revision") == 0)//Mat1MaterialRevision
		//tc_strcmp(objtype, "M4MDocumentRevision") == 0)
	{
		materialTags.push_back(objecttag);
	}

	if (tAlternatMaterial != NULLTAG)
	{
		tag_t* psecObjects = NULL;
		int psecCount      = 0;

		RULE_ERROR_CHECK(GRM_list_secondary_objects_only(objecttag, tAlternatMaterial, &psecCount, &psecObjects));

		if (psecCount > 0) {
			for (size_t i = 0; i < psecCount; i++)
			{

				materialTags.push_back(psecObjects[i]);

			}
		}
		SAFE_SM_FREE(psecObjects);
	}

	if (tMaterial != NULLTAG)
	{
		tag_t* tmaterial = NULL;
		int matCount     = 0;
		RULE_ERROR_CHECK(GRM_list_secondary_objects_only(objecttag, tMaterial, &matCount, &tmaterial));
		//cout << "No pf Materials present in \"Material\" - " << matCount << endl;
		if (matCount > 0) {
			for (size_t i = 0; i < matCount; i++)
			{
				materialTags.push_back(tmaterial[i]);
			}
		}
		SAFE_SM_FREE(tmaterial);
	}
	SAFE_SM_FREE(objtype);

	return ifail;
	
}

int M4_Get_ItemRevisions_Of_BOM(tag_t bomLinetag, vector<tag_t>& itemRevVector)
{
	int ifail             = ITK_ok;
	int	    attribute_id;
	tag_t   blRevTag      = NULLTAG;
	int		lineCount     = 0;
	tag_t* lineList       = NULL;
	int		iSublineCount = 0;
	tag_t* tSublineList   = NULL;

	RULE_ERROR_CHECK(BOM_line_look_up_attribute(bomAttr_lineItemRevTag, &attribute_id));
	RULE_ERROR_CHECK(BOM_line_ask_attribute_tag(bomLinetag, attribute_id, &blRevTag));

	char* childobjecttype     = NULL;
	tag_t* childobjecttypeTag = NULL;
	int objectChildCount      = 0;
	RULE_ERROR_CHECK(AOM_ask_value_string(blRevTag, "object_type", &childobjecttype));	

		if (strcmp(childobjecttype, "EDAComPart Revision") == 0)//Mat1MaterialRevision
		{
			itemRevVector.push_back(blRevTag);
		}

	SAFE_SM_FREE(childobjecttype);
	SAFE_SM_FREE(childobjecttypeTag);
	return ifail;
}

