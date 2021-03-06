#ifndef CAN_CONFIG_H
#define CAN_CONFIG_H

#include "main_config.h"

// -----------------------------------------------------------------------------
/* Global settings for building the can-lib and application program.
 *
 * The following two #defines must be set identically for the can-lib and
 * your application program. They control the underlying CAN struct. If the
 * settings disagree, the underlying CAN struct will be broken, with
 * unpredictable results.
 * If can.h detects that any of the #defines is not defined, it will set them
 * to the default values shown here, so it is in your own interest to have a
 * consistent setting. Ommiting the #defines in both can-lib and application
 * program will apply the defaults in a consistent way too.
 *
 * Select if you want to use 29 bit identifiers.
 */
#define SUPPORT_EXTENDED_CANID  1

// -----------------------------------------------------------------------------
/* Setting for MCP2515
 *
 * Declare which pins you are using for communication.
 * Remember NOT to use them in your application!
 * It is a good idea to use bits from the port that carries MOSI, MISO, SCK.
 */
#define MCP2515_CS                              B, 4
#define MCP2515_INT                             B, 2

#endif  // CAN_CONFIG_H
