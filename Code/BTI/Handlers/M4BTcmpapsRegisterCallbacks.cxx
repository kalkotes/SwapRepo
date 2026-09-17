/**
 *@file M4BTcmpapsRegisterCallbacks.cxx
 *@brief This file contains registration of Handlers defined in TCMPAPS Template.
 */
#include "M4BTcmpapsRegisterCallbacks.hxx"

#include <stdlib.h>
#include <string.h>

#include <tccore/custom.h>

#include <AbstractException.hxx>
#include <MessageManager.hxx>
#include <TCException.hxx>
#include <TCLogger.hxx>

#include "M4BLoggerConstants.hxx"
#include "M4BTcmpapsAhAddCcFromDxOrderAsTargetHandler.hxx"
#include "M4BTcmpapsAhAddCheckoutTargetObjToReferenceFolderHandler.hxx"
#include "M4BTcmpapsAhAddStructureContextsHandler.hxx"
#include "M4BTcmpapsAhAttachTopNodesAsTargetsHandler.hxx"
#include "M4BTcmpapsAhBolzenApplyRevEffectivityHandler.hxx"
#include "M4BTcmpapsAhBolzenCreateApplyVariantConditionHandler.hxx"
#include "M4BTcmpapsAhChangeOccurrenceTypeHandler.hxx"
#include "M4BtcmpapsAhChangeProjectAccessHandler.hxx"
#include "M4BTcmpapsAhCheckEbomImportStatusAndNotifyHandler.hxx"
#include "M4BTcmpapsAhCheckInvalidStatusAndNotifyHandler.hxx"
#include "M4BTcmpapsAhCheckProjectOfLibraryAndNotifyHandler.hxx"
#include "M4BTcmpapsAhCheckStatusAndNotifyHandler.hxx"
#include "M4BtcmpapsAhCopyEquipmentHandler.hxx"
#include "M4BTcmpapsAhInitiateDispatcherJobHandler.hxx"
#include "M4BtcmpapsAhMigratePlnAsmFormsHandler.hxx"
#include "M4BTcmpapsAhNotifyHandler.hxx"
#include "M4BTcmpapsAhReviseFromCCStructuresHandler.hxx"
#include "M4BTcmpapsAhReviseReleasedObjectsHandler.hxx"
#include "M4BTcmpapsAhSendFailureNotificationHandler.hxx"
#include "M4BTcmpapsAhSetEffectivityDateAndStatusHandler.hxx"
#include "M4BTcmpapsAhSetProcLineProjInSessionHandler.hxx"
#include "M4BTcmpapsAhUpdateConfigContextOfCCStructuresHandler.hxx"
#include "M4BTcmpapsConstants.hxx"
#include "M4BTcmpapsErrorCodes.hxx"
#include "M4BTcmpapsAhPlanningDataValidationHandler.hxx"
#include "M4BTcmpapsAhBOPBOESyncValidationHandler.hxx"
#include "M4BTcmpapsAhCreateITSMTicketHandler.hxx"
#include "M4BTcmpapsAhAttachAssemblyComponentsHandler.hxx"
#include "M4BTcmpapsRhValidateNoClusteredMaterialHandler.hxx"

#include <bmw_itk_wrapper_epmtask.hxx>

using namespace bmw::tegggi::error;
using namespace bmw::tegggi::itkwrapper;
using namespace bmw::tegggi::logger;
using namespace std;

int libM4B_tcmpaps_server_register_callbacks()
{
    int retCode = ITK_ok;
    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);

    try
    {
        logger.info(__FILE_LINE__, "Entering into custom register exit");
        CUSTOM_register_exit("libM4B_tcmpaps_server", "USER_init_module", (CUSTOM_EXIT_ftn_t)libM4BTcmpapsUserInitModule);
        logger.info(__FILE_LINE__, "Sucessfully Registered Library");
    }
    catch (TCException &ex)
    {
        logger.error(__FILE_LINE__, boost::format("TCException occurred while executing handler [%s]. Reason is %s") % __func__ % ex.askExceptionMessage().c_str());
        retCode = ex.askCode();
    }
    catch (AbstractException &ex)
    {
        std::string exceptionMsg = ex.askExceptionMessage() != "" ? ex.askExceptionMessage() : "unknown error";
        logger.error(__FILE_LINE__, boost::format("Exception occurred while executing handler [%s]. Reason is [%s]") % __func__ % exceptionMsg);
        MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, HANDLERS_ERROR_EXECUTING_HANDLER, __func__, exceptionMsg.c_str());
        retCode = HANDLERS_ERROR_EXECUTING_HANDLER;
    }
    catch (std::exception &ex)
    {
        string exceptionMsg = ex.what() ? ex.what() : "Unknown error";
        logger.error(__FILE_LINE__, boost::format("Exception occurred while executing handler [%s] . Reason is [%s]") % __func__ % exceptionMsg);
        MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, HANDLERS_ERROR_EXECUTING_HANDLER, __func__, exceptionMsg.c_str());
        retCode = HANDLERS_ERROR_EXECUTING_HANDLER;
    }

    return retCode;
}

int libM4BTcmpapsUserInitModule(int *decision, va_list args)
{
    int retCode = ITK_ok;
    *decision = ALL_CUSTOMIZATIONS;
    TCLogger logger(BMW_M4B_HANDLERS_LOGGER, __PRETTYFUNCTION__);

    try
    {
        // Action Handlers Registration.
        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-attach-top-nodes-as-targets", "M4B-TCMPAPS-AH-attach-top-nodes-as-targets", (EPM_action_handler_t)m4bTcmpapsAhAttachTopNodesAsTargetsHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-add-structure-contexts", "M4B-TCMPAPS-AH-add-structure-contexts", (EPM_action_handler_t)m4bTcmpapsAhAddStructureContextsHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-update-config-context-of-cc-structures", "M4B-TCMPAPS-AH-update-config-context-of-cc-structures", (EPM_action_handler_t)m4bTcmpapsAhUpdateConfigContextOfCCStructuresHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-check-status-and-notify", "M4B-TCMPAPS-AH-check-status-and-notify", (EPM_action_handler_t)m4bTcmpapsAhCheckStatusAndNotifyHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-revise-from-cc-structures", "M4B-TCMPAPS-AH-revise-from-cc-structures", (EPM_action_handler_t)m4bTcmpapsAhReviseFromCCStructuresHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-revise-released-objects", "M4B-TCMPAPS-AH-revise-released-objects", (EPM_action_handler_t)m4bTcmpapsAhReviseReleasedObjectsHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-add-cc-from-dx-order-as-target", "M4B-TCMPAPS-AH-add-cc-from-dx-order-as-target", (EPM_action_handler_t)m4bTcmpapsAhAddCcFromDxOrderAsTargetHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-add-checkout-target-obj-to-reference-folder", "M4B-TCMPAPS-AH-add-checkout-target-obj-to-reference-folder", (EPM_action_handler_t)m4bTcmpapsAhAddCheckoutTargetObjToReferenceFolderHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-check-invalid-status-and-notify", "M4B-TCMPAPS-AH-check-invalid-status-and-notify", (EPM_action_handler_t)m4bTcmpapsAhCheckInvalidStatusAndNotifyHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-set-proc-line-proj-in-session", "M4B-TCMPAPS-AH-set-proc-line-proj-in-session", (EPM_action_handler_t)m4bTcmpapsAhSetProcLineProjInSessionHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-notify", "M4B-TCMPAPS-AH-notify", (EPM_action_handler_t)m4bTcmpapsAhNotifyHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-set-effectivity-date-and-status", "M4B-TCMPAPS-AH-set-effectivity-date-and-status", (EPM_action_handler_t)m4bTcmpapsAhSetEffectivityDateAndStatusHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-check-project-of-library-and-notify", "M4B-TCMPAPS-AH-check-project-of-library-and-notify", (EPM_action_handler_t)m4bTcmpapsAhCheckProjectOfLibraryAndNotifyHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-bolzenApplyRevEffectivity", "M4B-TCMPAPS-AH-bolzenApplyRevEffectivity", (EPM_action_handler_t)m4bTcmpapsAhBolzenApplyRevEffectivityHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-copy-equipment", "M4B-TCMPAPS-AH-copy-equipment", (EPM_action_handler_t)m4btcmpapsAhCopyEquipmentHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-BOPBOESyncValidation", "M4B-TCMPAPS-AH-BOPBOESyncValidation", (EPM_action_handler_t)m4bTcmpapsAhBOPBOESyncValidationHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-initiate-dispatcher-job", "M4B-TCMPAPS-AH-initiate-dispatcher-job", (EPM_action_handler_t)m4bTcmpapsAhInitiateDispatcherJobHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-send-failure-notification", "M4B-TCMPAPS-AH-send-failure-notification", (EPM_action_handler_t)m4bTcmpapsAhSendFailureNotificationHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-bolzenCreateApplyVariantCondition", "M4B-TCMPAPS-AH-bolzenCreateApplyVariantCondition", (EPM_action_handler_t)m4bTcmpapsAhBolzenCreateApplyVariantConditionHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-migrate-pln-asm-forms", "M4B-TCMPAPS-AH-migrate-pln-asm-forms", (EPM_action_handler_t)m4btcmpapsAhMigratePlnAsmFormsHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-change-project-access", "M4B-TCMPAPS-AH-change-project-access", (EPM_action_handler_t)m4btcmpapsAhChangeProjectAccessHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-check-ebom-import-status-and-notify", "M4B-TCMPAPS-AH-check-ebom-import-status-and-notify", (EPM_action_handler_t)m4bTcmpapsAhCheckEbomImportStatusAndNotifyHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-PlanningDataValidation", "M4B-TCMPAPS-AH-PlanningDataValidation", (EPM_action_handler_t)m4bTcmpapsAhPlanningDataValidationHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-change-occurrence-type", "M4B-TCMPAPS-AH-change-occurrence-type", (EPM_action_handler_t)m4bTcmpapsAhChangeOccurrenceTypeHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-create-ITSM-ticket", "M4B-TCMPAPS-AH-create-ITSM-ticket", (EPM_action_handler_t)m4bTcmpapsAhCreateITSMTicketHandler);

        TEGGGIITK_EPM_register_action_handler("M4B-TCMPAPS-AH-attach-assembly-components", "M4B-TCMPAPS-AH-attach-assembly-components", (EPM_action_handler_t)m4bTcmpapsAhAttachAssemblyComponentsHandler);

        // Rule Handlers Registration
        TEGGGIITK_EPM_register_rule_handler("M4B-TCMPAPS-RH-validate-no-clustered-material", "Validates that no Clustered Material is present in Industrial Release", (EPM_rule_handler_t)m4bTcmpapsRhValidateNoClusteredMaterialHandler);
    }
    catch (TCException &ex)
    {
        logger.error(__FILE_LINE__, boost::format("TCException occurred while executing handler [%s]. Reason is %s") % __func__ % ex.askExceptionMessage().c_str());
        retCode = ex.askCode();
    }
    catch (AbstractException &ex)
    {
        std::string exceptionMsg = ex.askExceptionMessage() != "" ? ex.askExceptionMessage() : "unknown error";
        logger.error(__FILE_LINE__, boost::format("Exception occurred while executing handler [%s]. Reason is [%s]") % __func__ % exceptionMsg);
        MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, HANDLERS_ERROR_EXECUTING_HANDLER, __func__, exceptionMsg.c_str());
        retCode = HANDLERS_ERROR_EXECUTING_HANDLER;
    }
    catch (std::exception &ex)
    {
        string exceptionMsg = ex.what() ? ex.what() : "Unknown error";
        logger.error(__FILE_LINE__, boost::format("Exception occurred while executing handler [%s] . Reason is [%s]") % __func__ % exceptionMsg);
        MessageManager::getInstance().push(MessageManager::MESSAGE_SEVERITY_ERROR, HANDLERS_ERROR_EXECUTING_HANDLER, __func__, exceptionMsg.c_str());
        retCode = HANDLERS_ERROR_EXECUTING_HANDLER;
    }

    return retCode;
}
