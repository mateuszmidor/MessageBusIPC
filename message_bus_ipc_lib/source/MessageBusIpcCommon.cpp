/**
 *   @file: MessageBusIpcCommon.cpp
 *
 *   @date: Mar 16, 2017
 * @author: Mariusz Midor
 */

#include "MessageBusIpcCommon.h"

namespace messagebusipc {

/**
 * message names
 */
static const char * MessageBusMessages_Names[] = { MBIPC_MESSAGES(ENUM_NAME1_OPERATOR, ENUM_NAME2_OPERATOR, ENUM_COMMENT_OPERATOR) NULL };

/**
 * message values in the same order as names
 */
static const MessageBusMessage MessageBusMessages_Values[] = { MBIPC_MESSAGES(ENUM_ENUMVAL1_OPERATOR, ENUM_ENUMVAL2_OPERATOR, ENUM_COMMENT_OPERATOR) (MessageBusMessage)(uint32_t)-1 };

/**
 * @brief find message string representation using MessageBusMessages_Values::MessageBusMessages_Names like map
 * @param Msg
 * @return message name od "ID_?" if id not found
 */
const char * GetMessageName(MessageBusMessage Msg)
{
   for (uint32_t i = 0; MessageBusMessages_Values[i] != (uint32_t)-1; ++i)
      if (MessageBusMessages_Values[i] == Msg)
         return MessageBusMessages_Names[i];
   return "ID_?";
}

} // messagebusipc
