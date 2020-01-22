/*
 *
 */


#include "CANopen.h"
#include "CO_driver.h"
#include "error_assert_handlers.h"
#include "systemd_ODF.h"
#include <systemd/sd-bus.h>


#define BUS_NAME  "org.freedesktop.systemd1"
#define INTERFACE_NAME  BUS_NAME".Manager"
#define OBJECT_PATH     "/org/freedesktop/systemd1"


void systemd_ODF_configure(void){

    CO_OD_configure(CO->SDO[0], 0x3000, SYSTEMD_ODF, NULL, 0, 0U);

    return;
}


CO_SDO_abortCode_t SYSTEMD_ODF(CO_ODF_arg_t *ODF_arg) {
    CO_SDO_abortCode_t ret = CO_SDO_AB_NONE;
    sd_bus_error error = SD_BUS_ERROR_NULL;
    sd_bus *bus = NULL;
    int r;

    /* Connect to the bus */
    if(sd_bus_open_system(&bus) <= 0)
        return CO_SDO_AB_GENERAL;


    /* can't read parameters, write only */
    if(ODF_arg->reading == true)
        return CO_SDO_AB_WRITEONLY;

    switch(ODF_arg->subIndex) {
        case 1 : /* reboot Linux system */
            r = sd_bus_call_method(bus,
                                   BUS_NAME,
                                   OBJECT_PATH,
                                   INTERFACE_NAME,
                                   "Reboot",
                                   &error,
                                   NULL,
                                   NULL);
            if(r < 0)
                ret = CO_SDO_AB_GENERAL;
            
            break;

        case 2 : /* poweroff Linux system */
            r = sd_bus_call_method(bus,
                                   BUS_NAME,
                                   OBJECT_PATH,
                                   INTERFACE_NAME,
                                   "PowerOff",
                                   &error,
                                   NULL,
                                   NULL);
            if(r < 0)
                ret = CO_SDO_AB_GENERAL;

            break;

        default :
            ret = CO_SDO_AB_SUB_UNKNOWN; 
    }
    
    sd_bus_error_free(&error);
    sd_bus_unref(bus);
    return ret;
}


