/**
 * @file M4BTcmpapsAhNotifyHandler.cxx
 * @brief This file contains the implementation for the custom rule handler M4B-TCMPAPS-AH-notify.
 */

#include "M4BTcmpapsAhNotifyHandler.hxx"

#include <AbstractException.hxx>
#include <Preconditions.hxx>
#include <StringUtil.hxx>
#include <TCException.hxx>
#include <TCLogger.hxx>
#include <boost/filesystem.hpp>
#include <InvalidArgumentException.hxx>

#include "bmw_itk_wrapper_ae.hxx"
#include "bmw_itk_wrapper_aom.hxx"
#include "bmw_itk_wrapper_date.hxx"
#include "bmw_itk_wrapper_epmtask.hxx"
#include "bmw_itk_wrapper_imf.hxx"
#include "bmw_itk_wrapper_pom.hxx"
#include "bmw_itk_wrapper_pref.hxx"
#include "bmw_itk_wrapper_qry.hxx"
#include "bmw_itk_wrapper_sa.hxx"
#include "bmw_itk_wrapper_tctype.hxx"

#include "M4BCommon.hxx"
#include "M4BLoggerConstants.hxx"
#include "M4BTcmpapsConstants.hxx"
#include "M4BTcmpapsErrorCodes.hxx"
#include "M4BTcmpapsHelperMacros.hxx"
#include "T4BTegggiEmailHelperService.hxx"

namespace fs = boost::filesystem;
using namespace bmw::m4b::commonutils;
using namespace bmw::tegggi::tegggiemailhelperservice;
using namespace bmw::tegggi::error;
using namespace bmw::tegggi::itkwrapper;
using namespace bmw::tegggi::logger;
using namespace bmw::tegggi::util;
using namespace std;

int m4bTcmpapsAhNotifyHandler(EPM_action_message_t msg)
{
    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);
    int retCode = ITK_ok;

    M4B_TRY
    M4BCommonUtils utils;
    utils.logSessionInfo();

    Preconditions::checkNotNull(__FILE__, __LINE__, msg.task, "msg.task");

    // Get root task.
    tag_t rootTaskTag = NULLTAG;
    TEGGGIITK_EPM_ask_root_task(msg.task, rootTaskTag);

    if (NULLTAG == rootTaskTag)
    {
        logger.error(__FILE_LINE__, "The root task is not found.");
        return retCode;
    }

    // Get the arguments given to the handler in the workflow.
    map<string, string> argNamesAndValues = utils.getArgumentListForActionHandler(msg);

    string targetObjType = "";
    string propNameForToRec = "";
    string propNameForCcRec = "";
    string propNameForMailSub = "";
    string mailSubject = "";
    string dsNameForMailBody = "";
    string targetObjDsRel = "";
    string dsTypeForAttachment = "";
    string targetObjRel = "";
    string relatedObjType = "";
    vector<string> relatedObjPropNames;

    map<string, string>::iterator itr = argNamesAndValues.begin();

    while (itr != argNamesAndValues.end())
    {
        // Check if the argument is equal to "-targetObjType".
        if (itr->first.compare(ARG_TARGET_OBJ_TYPE) == 0)
        {
            if (itr->second.empty())
            {
                logger.error(__FILE_LINE__, boost::format("M4B-TCMPAPS-AH-notify:: -targetObjType workflow handler argument is missing."));
                MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_WORKFLOW_ARGUMENT_MISSING);
                return M4B_WORKFLOW_ARGUMENT_MISSING;
            }
            targetObjType = itr->second;
        }
        // Check if the argument is equal to "-propNameForToRec".
        else if (itr->first.compare(ARG_PROP_NAME_FOR_TO_RECIPIENT) == 0)
        {
            if (itr->second.empty())
            {
                logger.error(__FILE_LINE__, boost::format("M4B-TCMPAPS-AH-notify::-propNameForToRec workflow handler argument is missing."));
                MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_WORKFLOW_ARGUMENT_MISSING);
                return M4B_WORKFLOW_ARGUMENT_MISSING;
            }
            propNameForToRec = itr->second;
        }
        // Check if the argument is equal to "-propNameForCcRec".
        else if (itr->first.compare(ARG_PROP_NAME_FOR_CC_RECIPIENT) == 0)
        {
            if (itr->second.empty())
            {
                logger.error(__FILE_LINE__, boost::format("M4B-TCMPAPS-AH-notify::-propNameForCcRec workflow handler argument is missing."));
                MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_WORKFLOW_ARGUMENT_MISSING);
                return M4B_WORKFLOW_ARGUMENT_MISSING;
            }
            propNameForCcRec = itr->second;
        }
        // Check if the argument is equal to "-propNameForMailSub".
        else if (itr->first.compare(ARG_PROP_NAME_FOR_MAIL_SUBJECT) == 0)
        {
            if (itr->second.empty())
            {
                logger.error(__FILE_LINE__, boost::format("M4B-TCMPAPS-AH-notify::-propNameForMailSub workflow handler argument is missing."));
                MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_WORKFLOW_ARGUMENT_MISSING);
                return M4B_WORKFLOW_ARGUMENT_MISSING;
            }
            propNameForMailSub = itr->second;
        }
        // Check if the argument is equal to "-mailSubject".
        else if (itr->first.compare(ARG_MAIL_SUBJECT) == 0)
        {
            if (itr->second.empty())
            {
                logger.error(__FILE_LINE__, boost::format("M4B-TCMPAPS-AH-notify::-mailSubject workflow handler argument is missing."));
                MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_WORKFLOW_ARGUMENT_MISSING);
                return M4B_WORKFLOW_ARGUMENT_MISSING;
            }
            mailSubject = itr->second;
        }
        // Check if the argument is equal to "-dsNameForMailBody".
        else if (itr->first.compare(ARG_DATASET_NAME_FOR_MAIL_BODY) == 0)
        {
            if (itr->second.empty())
            {
                logger.error(__FILE_LINE__, boost::format("M4B-TCMPAPS-AH-notify::-dsNameForMailBody workflow handler argument is missing."));
                MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_WORKFLOW_ARGUMENT_MISSING);
                return M4B_WORKFLOW_ARGUMENT_MISSING;
            }
            dsNameForMailBody = itr->second;
        }
        // Check if the argument is equal to "-targetObjDsRel".
        else if (itr->first.compare(ARG_TARGET_OBJ_DATASET_REL) == 0)
        {
            if (!itr->second.empty())
                targetObjDsRel = itr->second;
        }
        // Check if the argument is equal to "-dsTypeForAttachment".
        else if (itr->first.compare(ARG_DATASET_TYPE_FOR_ATTACHMENT) == 0)
        {
            if (!itr->second.empty())
                dsTypeForAttachment = itr->second;
        }
        // Check if the argument is equal to "-targetObjRel".
        else if (itr->first.compare(ARG_TARGET_OBJ_REL) == 0)
        {
            if (!itr->second.empty())
                targetObjRel = itr->second;
        }
        // Check if the argument is equal to "-relatedObjType".
        else if (itr->first.compare(ARG_RELATED_OBJ_TYPE) == 0)
        {
            if (!itr->second.empty())
                relatedObjType = itr->second;
        }
        // Check if the argument is equal to "-relatedObjPropNamesForMailSub".
        else if (itr->first.compare(ARG_REL_OBJ_PROP_NAMES_FOR_MAIL_SUB) == 0)
        {
            if (!itr->second.empty())
            {
                vector<string> relatedObjPropNamesList = split(itr->second, ",");

                for (auto propertyName : relatedObjPropNamesList)
                {
                    // Insert the value in the relObjPropNamesV vector.
                    if (!propertyName.empty())
                    {
                        relatedObjPropNames.push_back(propertyName);
                    }
                }
            }
        }

        ++itr;
    }

    // Get the target object from the root task.
    vector<string> objectTypes;
    objectTypes.push_back(targetObjType);
    tag_t targetObjTag = utils.getObjOfTypeFromRootTask(rootTaskTag, EPM_target_attachment, objectTypes);
    logger.info(__FILE_LINE__, boost::format("targetObjTag = [%s].") % utils.getObjectStringAndUid(targetObjTag));

    if (targetObjTag == NULLTAG)
    {
        logger.error(__FILE_LINE__, boost::format("Object of type [%s] is not found in the target of the workflow process.") % targetObjType);
        MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_WORKFLOW_INVALID_TARGET);
        return M4B_WORKFLOW_INVALID_TARGET;
    }

    if (targetObjTag != NULLTAG)
    {
        tag_t relObjTag = NULLTAG;
        // Get all the secondary objects realted to the workflow target object with the given relation.
        vector<tag_t> secondaryObjects = utils.getSecondaryObjectsWithRelationAndType(targetObjTag, targetObjRel, relatedObjType);

        if (secondaryObjects.size() > 0)
        {
            relObjTag = secondaryObjects[0];
        }

        notifyTargetObjInfo(propNameForToRec, propNameForCcRec, propNameForMailSub, mailSubject, dsNameForMailBody, targetObjDsRel, dsTypeForAttachment, targetObjTag, relObjTag, relatedObjPropNames);
    }

    M4B_SERVER_ACTION_HANDLER_CATCH
    return retCode;
}

int notifyTargetObjInfo(const string &propNameForToRec,
                        const string &propNameForCcRec,
                        const string &propNameForMailSub,
                        const string &mailSubject,
                        const string &dsNameForMailBody,
                        const string &targetObjDsRel,
                        const string &dsTypeForAttachment,
                        const tag_t &targetObjTag,
                        const tag_t &relObjTag,
                        const vector<string> &relatedObjPropNames)
{
    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);
    int retCode = ITK_ok;

    M4BCommonUtils utils;

    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::propNameForToRec = [%s].") % propNameForToRec);
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::propNameForCcRec = [%s].") % propNameForCcRec);
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::propNameForMailSub = [%s].") % propNameForMailSub);
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::mailSubject = [%s].") % mailSubject);
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::dsNameForMailBody = [%s].") % dsNameForMailBody);
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::targetObjDsRel = [%s].") % targetObjDsRel);
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::dsTypeForAttachment = [%s].") % dsTypeForAttachment);
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::targetObjTag = [%s]") % utils.getObjectStringAndUid(targetObjTag));
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::relObjTag = [%s]") % utils.getObjectStringAndUid(relObjTag));
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::Number of properties on related objects = [%d]") % relatedObjPropNames.size());

    M4B_TRY
    Preconditions::checkNotEmpty(__FILE_LINE__, propNameForToRec, "propNameForToRec");
    Preconditions::checkNotEmpty(__FILE_LINE__, propNameForCcRec, "propNameForCcRec");
    Preconditions::checkNotEmpty(__FILE_LINE__, propNameForMailSub, "propNameForMailSub");
    Preconditions::checkNotEmpty(__FILE_LINE__, mailSubject, "mailSubject");
    Preconditions::checkNotEmpty(__FILE_LINE__, dsNameForMailBody, "dsNameForMailBody");
    Preconditions::checkNotNull(__FILE_LINE__, targetObjTag, "targetObjTag");

    // Get the logged in user tag, user name, person and mail address.
    tag_t currUserTag = NULLTAG;
    string currUserName = "";
    TEGGGIITK_POM_get_user(currUserName, currUserTag);
    tag_t personTag = NULLTAG;
    TEGGGIITK_SA_ask_user_person(currUserTag, personTag);
    string senderMailId = "";
    TEGGGIITK_SA_ask_person_email_address(personTag, senderMailId);

    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::currUserTag = [%s]") % utils.getObjectStringAndUid(currUserTag));
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::currUserName = [%s]") % utils.getObjectStringAndUid(personTag));
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::senderMailId = [%s]") % senderMailId);

    if (senderMailId.empty())
    {
        /* Retrieve the preference value.Read the preference when defined */
        TEGGGIITK_PREF_ask_char_value_at_location(PREF_EMAIL_FROM_ADDRESS, TC_preference_group, 0, senderMailId);
        logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::senderMailId from the preference = [%s].") % senderMailId);
    }

    if (senderMailId.empty())
    {
        logger.error(__FILE_LINE__, "Recipient mail ID is not found.");
        MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_SENDER_MAIL_ID_NOT_FOUND);
        return M4B_SENDER_MAIL_ID_NOT_FOUND;
    }

    // Get the type of the object.
    vector<string> toMailIds;
    tag_t objectType = NULLTAG;
    TEGGGIITK_TCTYPE_ask_object_type(targetObjTag, objectType);

    // Check if the property 'propNameForToRec' exists on the target object.
    if (utils.isPropertyExist(objectType, propNameForToRec))
    {
        // Get the value of the propNameForToRec property.
        string propValue = "";
        TEGGGIITK_AOM_ask_value_string(targetObjTag, propNameForToRec, propValue);

        if (!propValue.empty())
        {
            vector<string> values = split(propValue, ",");
            for (auto value : values)
            {
                if (!value.empty())
                {
                    toMailIds.push_back(value);
                }
            }
        }
        else
        {
            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::The value for property [%s] is empty.") % propNameForToRec);
        }
    }
    else
    {
        logger.error(__FILE_LINE__, boost::format("notifyTargetObjInfo::The property [%s] is not found on the target object.") % propNameForToRec);
    }

    // Check if the property 'propNameForCcRec' exists on the target object.
    vector<string> ccMailIds;

    if (utils.isPropertyExist(objectType, propNameForCcRec))
    {
        // Get the value of the propNameForCcRec property.
        string propValue = "";
        TEGGGIITK_AOM_ask_value_string(targetObjTag, propNameForCcRec, propValue);

        if (!propValue.empty())
        {
            vector<string> values = split(propValue, ",");
            for (auto value : values)
            {
                if (!value.empty())
                {
                    ccMailIds.push_back(value);
                }
            }
        }
        else
        {
            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::The value for property [%s] is empty.") % propNameForCcRec);
        }
    }
    else
    {
        logger.error(__FILE_LINE__, boost::format("notifyTargetObjInfo::The property [%s] is not found on the target object.") % propNameForCcRec);
    }

    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::Number of toMailIds = [%d]") % toMailIds.size());
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::Number of ccMailIds = [%d]") % ccMailIds.size());

    if (toMailIds.size() == 0 && ccMailIds.size() == 0)
    {
        logger.error(__FILE_LINE__, "Recipient mail ID is not found.");
        MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, M4B_RECIPIENT_MAIL_ID_NOT_FOUND);
        return M4B_RECIPIENT_MAIL_ID_NOT_FOUND;
    }

    // Check if the property 'propNameForMailSub' exists on the target object.
    string mailSubjectStr(mailSubject);

    if (utils.isPropertyExist(objectType, propNameForMailSub))
    {
        // Get the type of the property.
        string propTypeName = "";
        PROP_value_type_t propType;
        TEGGGIITK_AOM_ask_value_type(targetObjTag, propNameForMailSub, propType, propTypeName);

        string mailSubPropVal = "";

        switch (propType)
        {
            case PROP_string:
            {
                // Get the value of the property.
                TEGGGIITK_AOM_ask_value_string(targetObjTag, propNameForMailSub, mailSubPropVal);
                break;
            }

            case PROP_int:
            {
                int propValInt = 0;

                // Get the value of the property and convert it to string.
                TEGGGIITK_AOM_ask_value_int(targetObjTag, propNameForMailSub, propValInt);
                mailSubPropVal = to_string(propValInt);
                break;
            }

            // Below cases to be handled later when needed.
            case PROP_untyped:
            case PROP_char:
            case PROP_date:
            case PROP_double:
            case PROP_float:
            case PROP_logical:
            case PROP_short:
            case PROP_typed_reference:
            case PROP_untyped_reference:
            case PROP_external_reference:
            case PROP_note:
            case PROP_typed_relation:
            case PROP_untyped_relation:

            default:
            {
                logger.error(__FILE_LINE__, boost::format("notifyTargetObjInfo::Unhandled property type [%s] for the property [%s].") % propType % propTypeName);
                break;
            }
        }

        logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::mailSubPropVal =[%s].") % mailSubPropVal);

        if (!mailSubPropVal.empty())
        {
            // Replace the '$PROP' in the mail subject with the property value.
            string strToReplace(PROP);

            if (mailSubjectStr.find(strToReplace) != std::string::npos)
            {
                mailSubjectStr.replace(mailSubjectStr.find(strToReplace), strToReplace.length(), mailSubPropVal);
            }
        }

        logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::mailSubjectStr after replacing $PROP = [%s].") % mailSubjectStr);
    }
    else
    {
        logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::The property [%s] is not found on the target object.") % propNameForMailSub);
    }

    if (relObjTag != NULLTAG && relatedObjPropNames.size() > 0)
    {
        // Loop over each property name for the related object.
        // Get the property value and replace the constants
        // ($RELOBJPROPx where x = 1,2,3,..) from the mail subject
        // with the values.
        for (int index = 0; index < relatedObjPropNames.size(); index++)
        {
            string relObjPropVal = "";

            // Get the property value for the related object.
            TEGGGIITK_AOM_ask_value_string(relObjTag, relatedObjPropNames[index], relObjPropVal);

            if (relObjPropVal.empty())
            {
                relObjPropVal = "UNDEFINED";
            }

            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::relObjPropNamesV[%d] = [%s].") % index % relatedObjPropNames[index]);
            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::relObjPropVal = [%s].") % relObjPropVal);

            string strToReplace = RELOBJPROP + to_string(index + 1);

            if (mailSubjectStr.find(strToReplace) != std::string::npos)
            {
                mailSubjectStr.replace(mailSubjectStr.find(strToReplace), strToReplace.length(), relObjPropVal);
            }

            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::mailSubjectStr after replacing [%s] = [%s].") % relatedObjPropNames[index] % mailSubjectStr);
        }
    }

    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::mailSubjectStr = [%s].") % mailSubjectStr);

    // Query the dataset with given name for the mail body content.
    /*
    tag_t dataset = executeDatasetQuery(dsNameForMailBody, "HTML");

    string mailBodyFilePath = "";
    if (dataset != NULLTAG)
    {
        // Export the named reference file to local directory and get the path.
        mailBodyFilePath = getDsExportedFilePath(dataset);
    }
    */

    logical isAttachmentActivated = false;
    TEGGGIITK_PREF_ask_logical_value("Mail_send_file_attachments_activated", 0, isAttachmentActivated);
    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::Mail_send_file_attachments_activated = [%d]") % isAttachmentActivated);

    string attachmentListFilePath = "";

    if (isAttachmentActivated)
    {
        vector<tag_t> datasetTags;

        // Get the datasets attached to the target object with given relation.
        if (!dsTypeForAttachment.empty() && !targetObjDsRel.empty())
        {
            datasetTags = utils.getSecondaryObjectsWithRelationAndType(targetObjTag, targetObjDsRel, dsTypeForAttachment);
        }

        logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::Number of secondary datasets = [%d]") % datasetTags.size());

        string listFileContent = "";

        // Loop over all the datasets.
        for (auto datasetTag : datasetTags)
        {
            // Get the dataset format name.
            string dsFormatName = "";
            TEGGGIITK_AE_ask_dataset_format(datasetTag, dsFormatName);
            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::dsFormatName = [%s].") % dsFormatName);

            // Get all the named reference files associated with the current dataset.
            set<string> dsFileExportPaths = getAllNamedRefFileExportPaths(datasetTag);
            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::Size of dsFileExportPaths = [%d]") % dsFileExportPaths.size());

            set<string> alreadyExportedPath;

            // Loop over the dsFileExportPaths.
            for (set<string>::iterator dsFileExportPathsItr = dsFileExportPaths.begin(); dsFileExportPathsItr != dsFileExportPaths.end(); dsFileExportPathsItr++)
            {
                // Check if the current named reference file path is present in alreadyExportedPath.
                if (alreadyExportedPath.find(*dsFileExportPathsItr) == alreadyExportedPath.end())
                {
                    // Append the path string and the format to the listFileContent.
                    listFileContent.append(*dsFileExportPathsItr);
                    listFileContent.append("=");

                    if (dsFormatName.compare("TEXT") || dsFormatName.compare("ASCII"))
                    {
                        listFileContent.append("T");
                    }
                    else if (dsFormatName.compare("BINARY"))
                    {
                        listFileContent.append("B");
                    }

                    alreadyExportedPath.insert(*dsFileExportPathsItr);
                    listFileContent.append("\n");
                }
            }

            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::listFileContent = [%s].") % listFileContent);
        }

        logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::After loop, listFileContent = [%s].") % listFileContent);

        if (!listFileContent.empty())
        {
            date_t currTimeStamp = utils.getNow();
            std::string currentDateTime = "";
            TEGGGIITK_DATE_date_to_string(currTimeStamp, "%d-%m-%Y %H:%M:%S", currentDateTime);
            logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::currTimeStamp = [%s].") % currentDateTime);

            // Generate the attachmentListFilePath.
            utils.getEnvVarPath(attachmentListFilePath);

            attachmentListFilePath += fs::path::preferred_separator;
            attachmentListFilePath.append(currentDateTime);
            attachmentListFilePath.append(".txt");

            // Open the file and write the content in the file.
            ofstream outdata;
            outdata.open(attachmentListFilePath);

            if (!outdata)
            {
                logger.info(__FILE_LINE__, "sendMail::Failed to write in file.");
                cout << "Failed to write in file" << endl;
                return retCode;
            }

            outdata << listFileContent;
            outdata.close();
        }
    }

    logger.info(__FILE_LINE__, boost::format("notifyTargetObjInfo::attachmentListFilePath = [%s].") % attachmentListFilePath);

    vector<string> bccMailIds;

    // Send the mail.
    // utils.sendMail(senderMailId, toMailIds, ccMailIds, bccMailIds, mailSubStr, mailBodyFilePath, attachmentListFilePath);

    string sTemplateArg("");
    sTemplateArg.assign(dsNameForMailBody);
    std::map<std::string, std::string> m_keyNamesVsValues;
    TegggiEmail tEmail = TegggiEmail( mailSubjectStr, sTemplateArg, attachmentListFilePath, toMailIds, ccMailIds, senderMailId, m_keyNamesVsValues);
    tEmail.sendMail3();

    M4B_SERVER_COMMON_CATCH
    return retCode;
}

tag_t executeDatasetQuery(const string &datasetName, const string &datasetType)
{
    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);
    tag_t dataset = NULLTAG;

    M4B_TRY
    Preconditions::checkNotEmpty(__FILE_LINE__, datasetName, "datasetName");
    Preconditions::checkNotEmpty(__FILE_LINE__, datasetType, "datasetType");

    M4BCommonUtils utils;
    vector<string> entryNames = {"Name", "Dataset Type"};
    vector<string> entryValues = {datasetName, datasetType};
    vector<tag_t> datasetTags;

    // Get query "Dataset..."
    tag_t datasetQryTag = NULLTAG;
    TEGGGIITK_QRY_find("Dataset...", datasetQryTag);

    logger.info(__FILE_LINE__, boost::format("executeDatasetQuery::datasetQryTag = [%s], datasetName = [%s], datasetType = [%s]") % utils.getObjectStringAndUid(datasetQryTag) % datasetName % datasetType);

    TEGGGIITK_QRY_execute(datasetQryTag, entryNames, entryValues, datasetTags);

    if (datasetTags.size())
    {
        dataset = datasetTags[0];
    }
    else
    {
        logger.error(__FILE_LINE__, boost::format("executeDatasetQuery:: No dataset found with name = [%s] and type = [%s].") % datasetName % datasetType);
    }

    M4B_SERVER_COMMON_CATCH
    return dataset;
}

string getDsExportedFilePath(const tag_t datasetTag)
{
    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);
    string dsFileExportPath = "";

    M4B_TRY
    Preconditions::checkNotNull(__FILE_LINE__, datasetTag, "datasetTag");

    // Assign the value of environment variable for TC_TMP_DIR/HOME to the directory path variable.
    M4BCommonUtils utils;
    utils.getEnvVarPath(dsFileExportPath);
    dsFileExportPath += fs::path::preferred_separator;
    logger.info(__FILE_LINE__, boost::format("getDsExportedFilePath::Export Path for file = [%s]") % dsFileExportPath);

    // Get the named reference tags attached to the dataset tag.
    vector<tag_t> namedRefTags;
    TEGGGIITK_AE_ask_dataset_named_refs(datasetTag, namedRefTags);
    logger.info(__FILE_LINE__, boost::format("getDsExportedFilePath::Number of Named Reference found = [%d]") % namedRefTags.size());

    if (namedRefTags.size())
    {
        // Get the file name associated with the named reference tag.
        string fileName = "";
        TEGGGIITK_IMF_ask_original_file_name(namedRefTags[0], fileName);
        logger.info(__FILE_LINE__, boost::format("getDsExportedFilePath::File found = [%s]") % fileName);
        dsFileExportPath.append(fileName);

        if (fs::exists(dsFileExportPath))
        {
            fs::remove(dsFileExportPath);
            logger.info(__FILE_LINE__, "getDsExportedFilePath::Removing existing file");
        }

        // Export the file to the export path.
        TEGGGIITK_IMF_export_file(namedRefTags[0], dsFileExportPath);
        logger.info(__FILE_LINE__, boost::format("getDsExportedFilePath::File export path = [%s]") % dsFileExportPath);
    }

    M4B_SERVER_COMMON_CATCH
    return dsFileExportPath;
}

set<string> getAllNamedRefFileExportPaths(const tag_t datasetTag)
{
    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);
    set<string> dsFileExportPaths;

    M4B_TRY
    Preconditions::checkNotNull(__FILE_LINE__, datasetTag, "datasetTag");

    // Assign the value of environment variable for TC_TMP_DIR/HOME to the directory path variable.
    string directoryPath = "";
    M4BCommonUtils utils;
    utils.getEnvVarPath(directoryPath);
    directoryPath += fs::path::preferred_separator;
    logger.info(__FILE_LINE__, boost::format("getAllNamedRefFileExportPaths::Directory for file export = [%s]") % directoryPath);

    // Get the named reference tags attached to the dataset tag.
    vector<tag_t> namedRefTags;
    TEGGGIITK_AE_ask_dataset_named_refs(datasetTag, namedRefTags);
    logger.info(__FILE_LINE__, boost::format("getAllNamedRefFileExportPaths::Number of Named Reference found = [%d]") % namedRefTags.size());

    // Loop over all the named references.
    for (auto namedRef : namedRefTags)
    {
        // Get the file name associated with the named reference tag.
        string fileName = "";
        TEGGGIITK_IMF_ask_original_file_name(namedRef, fileName);
        logger.info(__FILE_LINE__, boost::format("getAllNamedRefFileExportPaths::File found = [%s]") % fileName);
        string dsFileExportPath = directoryPath;
        dsFileExportPath.append(fileName);

        if (fs::exists(dsFileExportPath))
        {
            fs::remove(dsFileExportPath);
            logger.info(__FILE_LINE__, "getDsExportedFilePath::Removing existing file");
        }

        // Export the file to the export path.
        TEGGGIITK_IMF_export_file(namedRef, dsFileExportPath);
        logger.info(__FILE_LINE__, boost::format("getDsExportedFilePath::File export path = [%s]") % dsFileExportPath);

        dsFileExportPaths.insert(dsFileExportPath);
    }

    logger.info(__FILE_LINE__, boost::format("dsFileExportPaths.size() = [%d]") % dsFileExportPaths.size());
    M4B_SERVER_COMMON_CATCH
    return dsFileExportPaths;
}
