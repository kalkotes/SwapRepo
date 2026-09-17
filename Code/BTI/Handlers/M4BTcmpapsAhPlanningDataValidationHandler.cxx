/**
 * @file M4BTcmpapsAhPlanningDataValidationHandler.cxx
 * @brief This file contains the implementation for the custom action handler M4B-TCMPAPS-AH-PlanningDataValidation defined in TCMPAPS template.
 */

#include "M4BTcmpapsAhPlanningDataValidationHandler.hxx"
#include "M4BStructureValidationsService.hxx"

#include <fstream>
#include <iostream>
#include <chrono>

#include <boost/filesystem.hpp>
#include <ErrorMacros.hxx>
#include <Preconditions.hxx>
#include <StringUtil.hxx>
#include <TCException.hxx>
#include <TCLogger.hxx>

#include <bmw_itk_wrapper_aom.hxx>
#include <bmw_itk_wrapper_epmtask.hxx>

#include "M4BCommon.hxx"
#include "M4BLoggerConstants.hxx"
#include "M4BPruftoolCommon.hxx"
#include "M4BTcmpapsConstants.hxx"
#include "M4BTcmpapsErrorCodes.hxx"
#include "M4BTcmpapsHelperMacros.hxx"

namespace fs = boost::filesystem;
using namespace bmw::tegggi::error;
using namespace bmw::tegggi::itkwrapper;
using namespace bmw::tegggi::logger;
using namespace bmw::tegggi::util;
using namespace std;

int m4bTcmpapsAhPlanningDataValidationHandler(EPM_action_message_t msg)
{
   TC_write_syslog("\n[M4B DEBUG] Entered M4B-TCMPAPS-AH-PlanningDataValidation handler\n");

    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);
    int retCode = ITK_ok;
    M4BPruftoolCommon prufToolObj;
    M4B_TRY
    
    //Preconditions::checkNotNull(__FILE__, __LINE__, msg.task, "msg.task");

    // Start time
    auto t0 = std::chrono::high_resolution_clock::now();

    // Get root task.
   // tag_t rootTaskTag = NULLTAG;
  //  TEGGGIITK_EPM_ask_root_task(msg.task, rootTaskTag);

  logger.info(__FILE_LINE__, "Action handler M4B-TCMPAPS-AH-PlanningDataValidation started.");
    
   
    // Get root task.
    tag_t rootTaskTag = NULLTAG;
    TEGGGIITK_EPM_ask_root_task(msg.task, rootTaskTag);

    if (NULLTAG == rootTaskTag)
    {
        logger.error(__FILE_LINE__, "rootTaskTag is NULLTAG. Cannot retrieve task template.");
        return ITK_ok; // Return success to prevent workflow failure
    }
    
    logger.info(__FILE_LINE__, "rootTaskTag retrieved successfully.");

    if (NULLTAG != rootTaskTag)
    {
        tag_t taskTemplete = NULLTAG;
        TEGGGIITK_AOM_ask_value_tag(rootTaskTag, TC_PROPERTY_TASK_TEMPLATE, taskTemplete);
        string taskTempleteName = "";
        TEGGGIITK_AOM_ask_value_string(taskTemplete, TC_PROPERTY_OBJECT_NAME, taskTempleteName);
        logger.trace(__FILE_LINE__, boost::format("Template name [%s].") % taskTempleteName);

        // Read Workflow initiator
        tag_t workFlowInitiator = NULLTAG;
        TEGGGIITK_AOM_ask_value_tag(rootTaskTag, TC_PROPERTY_FND0WORKFLOWINITIATOR, workFlowInitiator);

        // Step 1. Read workflow description
        tag_t parentJob = NULLTAG;
        TEGGGIITK_EPM_ask_job(rootTaskTag, parentJob);
        string workflowDesc = "";
        TEGGGIITK_AOM_ask_value_string(parentJob, TC_PROPERTY_OBJECT_DESC, workflowDesc);

        if (workflowDesc.empty())
        {
            logger.info(__FILE_LINE__, "No validation names present in the workflow description.");
            return retCode;
        }
        logger.info(__FILE_LINE__, boost::format("Workflow description : [%s].") % workflowDesc);
        // Get PrufTool validation mentioned in the
        vector<string> validationOptionList;

        prufToolObj.getPrufValidationList(workflowDesc, validationOptionList);
        // Get target objects.
        vector<tag_t> refObjTags;
        TEGGGIITK_EPM_ask_attachments(rootTaskTag, EPM_reference_attachment, refObjTags);
        logger.trace(__FILE_LINE__, boost::format("refObjCount = [%d].") % refObjTags.size());

        // Initialize variables
        if (validationOptionList.size())
        {
            if (refObjTags.size() && NULLTAG != refObjTags[0])
            {
                bool isValidationPassed = false;
                prufToolObj.performPrufToolValidations(taskTempleteName, refObjTags[0], workFlowInitiator, validationOptionList, isValidationPassed);
            }
        }
    }
    logger.trace(__FILE_LINE__, boost::format("retCode : [%d].") % retCode);
    // Set return code to 0 as currently it is not catched in workflow. Remove this if it is catched in workflow
    retCode = 0;

    // End time
    auto t1 = std::chrono::high_resolution_clock::now();
    // Calculate the time difference in seconds
    std::chrono::duration<double> delta = t1 - t0;

    logger.info(__FILE_LINE__, boost::format("Total time for execution =  [%d secs].") % delta.count());
    M4B_SERVER_ACTION_HANDLER_CATCH
    return retCode;
}
