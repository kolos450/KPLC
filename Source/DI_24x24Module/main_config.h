#ifndef MAIN_CONFIG_H
#define MAIN_CONFIG_H

#define F_CPU 16000000UL
#define CAN_SPEED 125000

#define CANARD_NODESTATUS_PERIOD_MSEC    500
#define CANARD_NODE_ID 102

#define APP_VERSION_MAJOR                       1
#define APP_VERSION_MINOR                       0
#define APP_NODE_NAME                           "KPLC.DI_24x24Module"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#endif