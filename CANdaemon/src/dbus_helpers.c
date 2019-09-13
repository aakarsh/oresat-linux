#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#include "CANopen.h"
#include "CO_OD.h"
#include "CO_master.h"
#include "dbus_helpers.h"
#include "CO_comm_helpers.h"


#define FILENAME_MAX_LENGTH 20
#define TEST_NODE_ID 3


/* Variables */
static uint16_t             SDOtimeoutTime = 500; /* Timeout time for SDO transfer in milliseconds, if no response */
static uint8_t              blockTransferEnable = 1; /* SDO block transfer enabled? */

void dbusError(int r, char* err) {
    if (r < 0)
        fprintf(stderr, "%s %s\n", err, strerror(-r));
    return;
}


void dbusErrorExit(int r, char* err) {
    if (r < 0) {
        fprintf(stderr, "%s %s\n", err, strerror(-r));
        exit(0);
    }
    return;
}


int OD_add_file(const uint16_t idx, 
                const uint8_t subidx_name, 
                const uint8_t subidx_data,
                const char* file_path) {

    if(file_path == NULL)
        return 0;

    int file_size = get_file_size(file_path);
    char* file_name = remove_path(file_path);
    char* file_data = read_file(file_path, file_size);

    if((file_name == NULL) || (file_data == NULL)) {
        printf("Invalid file path\n");
        return -1;
    }

    send_SDO(idx, subidx_name, file_name, strlen(file_name)); // don't send '\0'
    send_SDO(idx, subidx_data, file_data, file_size);
    return 1; 
}

int OD_update(const uint16_t idx, 
              const uint8_t subidx,
              const int16_t data) {

    if(data >= 0)
        return 1;

    uint32_t length = sizeof(data);
    char* new_data = malloc(sizeof(char) * length);
    memcpy(&new_data, &data, length);

    send_SDO(idx, subidx, new_data, length);

    free(new_data);
    new_data = NULL;

    return 0;
}


unsigned int get_file_size(const char* file_path) {
    if(file_path == NULL)
        return 0;

    size_t file_size;
    FILE *f = fopen(file_path, "r");

    if(f == NULL) {
        printf("File not found: %s\n", file_path); 
        return 0;
    }

    // get file size
    fseek(f, 0, SEEK_END);
    file_size = ftell(f);
    fclose(f);

    return file_size;
}


char* read_file(const char* file_path, unsigned int file_size) {
    if(file_path == NULL)
        return NULL;

    char * file_data = NULL;
    FILE *f = fopen(file_path, "r");

    if(f == NULL) {
        printf("File not found: %s\n", file_path); 
        return NULL;
    }

    // read into buffer
    file_data = malloc((file_size + 1) * sizeof(*file_data));
    fread(file_data, file_size, 1, f);
    file_data[file_size] = '\0';

    fclose(f);

    return file_data;
}


char* remove_path(const char* file_path) {
    if(file_path == NULL)
        return NULL;

    char * file_name = NULL;
    unsigned int size;
    unsigned int size_new;
    unsigned int start;

    size = strlen(file_path) + 1;
    size_new = size;
    start = size-1;
    
    // find right most '/'
    while(start > 0) {
        if(file_path[start] == '/') {
            ++start;
            size_new = size - start;
            break;
        }
        --start;
    }

    if(size_new > FILENAME_MAX_LENGTH) 
        return NULL;

    // make the filename of all spaces
    file_name = (char *)malloc(FILENAME_MAX_LENGTH+1);
    for(unsigned int i=0; i<FILENAME_MAX_LENGTH; ++i) 
        file_name[i] = ' ';

    // copy only file name
    strncpy(file_name, &file_path[start], size_new-1);
    file_name[FILENAME_MAX_LENGTH] = '\0';

    printf("%s;", file_name);

    return file_name;
}


void send_SDO(uint16_t idx, uint8_t subidx, char* input_data, uint32_t len) {
    if(input_data == NULL) {
        printf("Input error\n");
        return;
    }

    int err = 0; /* syntax or other error, true or false */
    int emptyLine = 0;
    int respLen = 0;
    char resp[STRING_BUFFER_SIZE];
    respErrorCode_t respErrorCode = respErrorNone;
    uint32_t sequence = 0;
    uint32_t SDOabortCode = 1;
    uint8_t dataRx[SDO_BUFFER_SIZE]; /* SDO transmit buffer */
    uint32_t dataRxLen = 0;  /* Length of data to transmit. */

    dataRxLen = len;
    /* Length must match and must not be zero. */
    if(dataRxLen == 0 || dataRxLen >= SDO_BUFFER_SIZE) {
        printf("len error\n");
        err = 1;
    }
    memcpy(dataRx, input_data, dataRxLen);

    printf("sending SDO\n");

    /* Make CANopen SDO transfer */
    if(err == 0) {
        err = sdoClientUpload(
                CO->SDOclient[0],
                OD_CANNodeID,
                idx,
                subidx,
                dataRx,
                sizeof(dataRx),
                &dataRxLen,
                &SDOabortCode,
                SDOtimeoutTime,
                blockTransferEnable);

        if(err != 0){
            respErrorCode = respErrorInternalState;
            printf("SDO download err: %d\n", respErrorCode);
        }
    }

    /* output result */
    if(err == 0){
        if(SDOabortCode == 0) {
            respLen = sprintf(resp, "[%d] OK", sequence);
        }
        else{
            respLen = sprintf(resp, "[%d] ERROR: 0x%08X", sequence, SDOabortCode);
        }
    }

    /* Generate error response (or leave empty line response) */
    if(err != 0 && emptyLine == 0) {
        if(respErrorCode == respErrorNone) {
            respErrorCode = respErrorSyntax;
        }
        respLen = sprintf(resp, "[%d] ERROR: %d", sequence, respErrorCode);
    }

    /* Terminate string and send response */
    resp[respLen++] = '\r';
    resp[respLen++] = '\n';
    resp[respLen++] = '\0';

    /* TODO printf or log
    if(write(fd, resp, respLen) != respLen) {
        CO_error(0x15200000L);
    }
    */
}
