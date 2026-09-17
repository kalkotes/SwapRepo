/**
 *@file M4BTcmpapsAhAddCheckoutTargetObjToReferenceFolderHandler.cxx
 *@brief This file contains the implementation for the custom handler M4B-TCMPAPS-AH-add-checkout-target-obj-to-reference-folder defined in TCMPAPS Template.
 */

#include <M4BTcmpapsAhAddCheckoutTargetObjToReferenceFolderHandler.hxx>

#include <AbstractException.hxx>
#include <Preconditions.hxx>
#include <TCException.hxx>
#include <TCLogger.hxx>

#include <bmw_itk_wrapper_aom.hxx>
#include <bmw_itk_wrapper_epmtask.hxx>

#include <M4BCommon.hxx>
#include <M4BLoggerConstants.hxx>
#include <M4BTcmpapsConstants.hxx>
#include "M4BTcmpapsErrorCodes.hxx"
#include <M4BTcmpapsHelperMacros.hxx>

using namespace bmw::m4b::commonutils;
using namespace bmw::tegggi::error;
using namespace bmw::tegggi::itkwrapper;
using namespace bmw::tegggi::logger;
using namespace std;

int m4bTcmpapsAhAddCheckoutTargetObjToReferenceFolderHandler(EPM_action_message_t msg)
{
    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);
    int retCode = ITK_ok;

    M4B_TRY
    M4BCommonUtils utils;
    utils.logSessionInfo();

    Preconditions::checkNotNull(__FILE__, __LINE__, msg.task, "task");

    // Get root task.
    tag_t rootTaskTag = NULLTAG;
    TEGGGIITK_EPM_ask_root_task(msg.task, rootTaskTag);

    if (rootTaskTag == NULLTAG)
    {
        logger.error(__FILE_LINE__, "Invalid root task tag. No further Processing.");
        return ITK_ok;
    }

    // Get reference attachments.
    std::vector<tag_t> referenceObjectTags;
    TEGGGIITK_EPM_ask_attachments(rootTaskTag, EPM_reference_attachment, referenceObjectTags);

    int numOfRefObjs = referenceObjectTags.size();
    logger.trace(__FILE_LINE__, boost::format("Reference Object count = [%d].") % numOfRefObjs);

    std::vector<int> attachmentTypes;

    if (numOfRefObjs > 0)
    {
        for (int index = 0; index < numOfRefObjs; index++)
        {
            attachmentTypes.push_back(EPM_reference_attachment);
        }

        // Remove reference attachments.
        TEGGGIITK_EPM_remove_attachments_from_att_type(rootTaskTag, referenceObjectTags, attachmentTypes);
    }

    attachmentTypes.clear();

    // Check reference attachments.
    std::vector<tag_t> checkReferenceObjectTags;
    TEGGGIITK_EPM_ask_attachments(rootTaskTag, EPM_reference_attachment, checkReferenceObjectTags);
    logger.trace(__FILE_LINE__, boost::format("Reference Object count after removal of reference object = [%d].") % checkReferenceObjectTags.size());

    // Get target attachments.
    std::vector<tag_t> targetObjectTags;
    TEGGGIITK_EPM_ask_attachments(rootTaskTag, EPM_target_attachment, targetObjectTags);
    logger.trace(__FILE_LINE__, boost::format("Target Object count = [%d].") % targetObjectTags.size());

    // Get Checked out target objects.
    std::vector<tag_t> checkedOutTargetObj;

    for (auto targetObjectTag : targetObjectTags)
    {
        if (targetObjectTag == NULLTAG)
        {
            continue;
        }

        std::string checkedOutString = "";
        TEGGGIITK_AOM_ask_value_string(targetObjectTag, TC_PROPERTY_CHECKED_OUT, checkedOutString);
        logger.trace(__FILE_LINE__, boost::format("checkedOutString = [%s].") % checkedOutString);

        if (checkedOutString.compare("Y") == 0)
        {
            checkedOutTargetObj.push_back(targetObjectTag);
        }
    }

    int numCheckedOutTarObjs = checkedOutTargetObj.size();

    logger.trace(__FILE_LINE__, boost::format("checkedOutTargetObj size = [%d].") % numCheckedOutTarObjs);

    // Attach checked out objects to reference and set task result.
    if (numCheckedOutTarObjs > 0)
    {
        for (int index = 0; index < numCheckedOutTarObjs; index++)
        {
            attachmentTypes.push_back(EPM_reference_attachment);
        }

        TEGGGIITK_EPM_add_attachments(rootTaskTag, checkedOutTargetObj, attachmentTypes);
    }

    tag_t checkForCheckedinObjectsTag = NULLTAG;
    TEGGGIITK_EPM_ask_sub_task(rootTaskTag, "Check for checked in objects", checkForCheckedinObjectsTag);

    std::string taskName = "";
    TEGGGIITK_EPM_ask_name(checkForCheckedinObjectsTag, taskName);
    logger.trace(__FILE_LINE__, boost::format("taskName = [%s].") % taskName);

    if (numCheckedOutTarObjs)
    {
        TEGGGIITK_EPM_set_task_result(checkForCheckedinObjectsTag, EPM_RESULT_False);
    }
    else
    {
        TEGGGIITK_EPM_set_task_result(checkForCheckedinObjectsTag, EPM_RESULT_True);
    }

    M4B_SERVER_ACTION_HANDLER_CATCH
    return retCode;
}
