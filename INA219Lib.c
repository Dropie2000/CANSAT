#include "INA219Lib.h"

void INA_write(uint8_t reg_add, uint16_t data)
{
    uint8_t d1=data>>8;
    uint8_t d2=data & 0x00FF;
    I2C_Start();
    I2C_Write(INA219_ADDRESS);
    I2C_Write(reg_add);
    I2C_Write(d1);
    I2C_Write(d2);
    I2C_Stop();
}

bool INA_begin(uint16_t conf, uint16_t cal)
{
    INA_write(INA_conf_reg, 0xB99F);    //Reset
    __delay_ms(100);
    
    INA_write(INA_conf_reg, conf);    //Configurar el sensor
    INA_write(INA_cal_reg, cal);     //Calibrar el sensor
    return 1;
}

uint16_t INA_read(uint8_t reg)
{
  union
  {
    uint8_t  b[2];
    uint16_t w;
  } ret;
  
  I2C_Start();
  I2C_Write(INA219_ADDRESS);
  I2C_Write(reg);
  I2C_Restart();
  I2C_Write(INA219_ADDRESS | 1);
  ret.b[1] = I2C_Read();
  I2C_Ack();
  ret.b[0] = I2C_Read();
  I2C_Nack();
  I2C_Stop();

  return(ret.w);
  
}

void INA_UPDATE(void)
{

    reg_shunt=INA_read(shunt_voltage_reg);       // Lectura del voltaje shunt
    reg_bus=INA_read(bus_voltage_reg);         // Lectura del voltaje del bus
    reg_power=INA_read(power_reg);       // Lectura de la potencia
    reg_current=INA_read(current_reg);     // Lectura de la corriente
  
}

double Get_bus_voltage(void)
{
    double val;
    INA_UPDATE();
    val=(reg_bus>>3)*0.004;
    return val;
}

double Get_current(void)
{
    double val;
    val=(reg_current & 0x7FFF)*0.00003052;
    return val;
}
double Get_power(void)
{
    double val;
    val=reg_power*0.00061;
    return val;
}