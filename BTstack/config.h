#define EMBEDDED
#define HAVE_TICK

#define HAVE_INIT_SCRIPT
#define HAVE_BZERO
#define HAVE_EHCILL
//#define HAVE_TIME

// #define ENABLE_LOG_INFO
// #define ENABLE_LOG_ERROR

#define HCI_ACL_PAYLOAD_SIZE 52

//
#define MAX_SPP_CONNECTIONS 1

#define MAX_NO_HCI_CONNECTIONS MAX_SPP_CONNECTIONS
#define MAX_NO_L2CAP_SERVICES  2
#define MAX_NO_L2CAP_CHANNELS  (1+MAX_SPP_CONNECTIONS)
#define MAX_NO_RFCOMM_MULTIPLEXERS MAX_SPP_CONNECTIONS
#define MAX_NO_RFCOMM_SERVICES 1
#define MAX_NO_RFCOMM_CHANNELS MAX_SPP_CONNECTIONS
#define MAX_NO_DB_MEM_DEVICE_LINK_KEYS  2
#define MAX_NO_DB_MEM_DEVICE_NAMES 0
#define MAX_NO_DB_MEM_SERVICES 1

