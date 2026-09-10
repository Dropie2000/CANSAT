#ifndef INA219Lib_H
#define	INA219Lib_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include<stdbool.h>
#include "i2c.h"


#define INA219_ADDRESS 0x40<<1

#define INA_conf_reg        0x00
#define shunt_voltage_reg   0x01
#define bus_voltage_reg     0x02
#define power_reg           0x03
#define current_reg         0x04
#define INA_cal_reg         0x05

uint16_t reg_shunt, reg_bus, reg_current, reg_power,aux_current;

void INA_write(uint8_t reg_add, uint16_t data);
uint16_t INA_read(uint8_t reg);
bool INA_begin(uint16_t conf, uint16_t cal);
void INA_UPDATE(void);
double Get_bus_voltage(void);
double Get_current(void);
double Get_power(void);

#endif	/* XC_HEADER_TEMPLATE_H */

