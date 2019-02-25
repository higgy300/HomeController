/*
 * _data_pack_.h
 *
 *  Created on: Feb 23, 2019
 *      Author: juanh
 */

#ifndef DATA_PACK__H_
#define DATA_PACK__H_
#include <stdint.h>
#include <stdbool.h>

#pragma pack (push, 1)
typedef struct {
    bool ctrl_acknowledged;
    uint32_t ip_addr;
    uint16_t voltage;
    uint16_t curr1;
    uint16_t curr2;
}_eMeter_t;

typedef struct {
    bool ack_meter;
    bool ack_fireAlarm;
    bool meter_requesting;
    bool fireAlarm_requesting;
    uint32_t ip_addr;
}_controller_t;

typedef struct {
    bool ctrl_acknowledged;
    uint16_t ip_addr;
    uint16_t reading;
}_fireAlarm_t;
#pragma pack (pop)

//void establishConnection(uint8_t* ctrl, uint8_t* device);
//void meterToController(uint8_t* addr);
//void controllerToMeter(uint8_t* addr);

#endif /* DATA_PACK__H_ */
