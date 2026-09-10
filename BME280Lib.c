#include "BME280Lib.h"

void BME280_Write(uint8_t reg_addr, uint8_t _data)
{
  I2C_Start();
  I2C_Write(BME280_I2C_ADDRESS);
  I2C_Write(reg_addr);
  I2C_Write(_data);
  I2C_Stop();
}

// reads 8 bits from register 'reg_addr'
uint8_t BME280_Read8(uint8_t reg_addr)
{
  uint8_t ret;

  I2C_Start();
  I2C_Write(BME280_I2C_ADDRESS);    //Write mode
  I2C_Write(reg_addr);
  //I2C_Stop();
  //I2C_Start();
  I2C_Restart();
  I2C_Write(BME280_I2C_ADDRESS | 1);    //Read mode
  ret = I2C_Read();
  I2C_Nack();
  I2C_Stop();
  return ret;
}

// reads 16 bits from register 'reg_addr'
uint16_t BME280_Read16(uint8_t reg_addr)
{
  union
  {
    uint8_t  b[2];
    uint16_t w;
  } ret;

  I2C_Start();
  I2C_Write(BME280_I2C_ADDRESS);
  I2C_Write(reg_addr);
  //I2C_Stop();
  I2C_Start();
  I2C_Write(BME280_I2C_ADDRESS | 1);
  ret.b[0] = I2C_Read();
  I2C_Ack();
  ret.b[1] = I2C_Read();
  I2C_Nack();
  I2C_Stop();

  return(ret.w);
}

// initializes the BME280 sensor, returns 1 if OK and 0 if error
bool BME280_begin(enum bme280_mode mode,enum bme280_sampling T_sampling,enum bme280_sampling H_sampling,
                  enum bme280_sampling P_sampling, enum bme280_filter filter, enum standby_time  standby)
{
  if(BME280_Read8(BME280_REG_CHIPID) != CHIP_ID)
    return 0;

  // reset the BME280 with soft reset
  BME280_Write(BME280_REG_SOFTRESET, 0xB6);
  __delay_ms(100);

  // if NVM data are being copied to image registers, wait 100 ms
  while( (BME280_Read8(BME280_REG_STATUS) & 0x01) == 0x01 )
  __delay_ms(100);

  BME280_calib.dig_T1 = BME280_Read16(BME280_REG_DIG_T1);
  BME280_calib.dig_T2 = BME280_Read16(BME280_REG_DIG_T2);
  BME280_calib.dig_T3 = BME280_Read16(BME280_REG_DIG_T3);

  BME280_calib.dig_P1 = BME280_Read16(BME280_REG_DIG_P1);
  BME280_calib.dig_P2 = BME280_Read16(BME280_REG_DIG_P2);
  BME280_calib.dig_P3 = BME280_Read16(BME280_REG_DIG_P3);
  BME280_calib.dig_P4 = BME280_Read16(BME280_REG_DIG_P4);
  BME280_calib.dig_P5 = BME280_Read16(BME280_REG_DIG_P5);
  BME280_calib.dig_P6 = BME280_Read16(BME280_REG_DIG_P6);
  BME280_calib.dig_P7 = BME280_Read16(BME280_REG_DIG_P7);
  BME280_calib.dig_P8 = BME280_Read16(BME280_REG_DIG_P8);
  BME280_calib.dig_P9 = BME280_Read16(BME280_REG_DIG_P9);

  BME280_calib.dig_H1 = BME280_Read8(BME280_REG_DIG_H1);
  BME280_calib.dig_H2 = BME280_Read16(BME280_REG_DIG_H2);
  BME280_calib.dig_H3 = BME280_Read8(BME280_REG_DIG_H3);
  BME280_calib.dig_H4 = ((uint16_t)BME280_Read8(BME280_REG_DIG_H4) << 4) | (BME280_Read8(BME280_REG_DIG_H4 + 1) & 0x0F);
  if (BME280_calib.dig_H4 & 0x0800)    // if BME280_calib.dig_H4 < 0
      BME280_calib.dig_H4 |= 0xF000;
  BME280_calib.dig_H5 = ((uint16_t)BME280_Read8(BME280_REG_DIG_H5 + 1) << 4) | (BME280_Read8(BME280_REG_DIG_H5) >> 4);
  if (BME280_calib.dig_H5 & 0x0800)    // if BME280_calib.dig_H5 < 0
      BME280_calib.dig_H5 |= 0xF000;
  BME280_calib.dig_H6 = BME280_Read8(BME280_REG_DIG_H6);

 uint8_t _ctrl_hum, _ctrl_meas, _config;

  _ctrl_hum = H_sampling;
  _config = ((standby << 5) | (filter << 2)) & 0xFC;
  _ctrl_meas = (T_sampling << 5) | (P_sampling << 2) | mode;

  BME280_Write(BME280_REG_CTRLHUM, _ctrl_hum);
  BME280_Write(BME280_REG_CONFIG,  _config);
  BME280_Write(BME280_REG_CTRLMEAS, _ctrl_meas);

  return 1;
}

// takes a new measurement, for forced mode only!
// Returns 1 if ok and 0 if error (sensor is not in sleep mode)
bool BME280_ForcedMeasurement()
{
  uint8_t ctrl_meas_reg = BME280_Read8(BME280_REG_CTRLMEAS);

  if ( (ctrl_meas_reg & 0x03) != 0x00 )
    return 0;   // sensor is not in sleep mode

  // set sensor to forced mode
  BME280_Write(BME280_REG_CTRLMEAS, ctrl_meas_reg | 1);
  // wait for conversion complete
  while (BME280_Read8(BME280_REG_STATUS) & 0x08)
    __delay_ms(1);

  return 1;
}

// read (updates) adc_P, adc_T and adc_H from BME280 sensor
void BME280_Update()
{
  union
  {
    uint8_t  b[4];
    uint32_t dw;
  } ret;
  ret.b[3] = 0x00;

  I2C_Start();
  I2C_Write(BME280_I2C_ADDRESS);
  I2C_Write(BME280_REG_PRESS_MSB);
  I2C_Stop();
  I2C_Start();
  I2C_Write(BME280_I2C_ADDRESS | 1);
  ret.b[2] = I2C_Read();
  I2C_Ack();
  ret.b[1] = I2C_Read();
  I2C_Ack();
  ret.b[0] = I2C_Read();
  I2C_Ack();

  adc_P = (ret.dw >> 4) & 0xFFFFF;

  ret.b[2] = I2C_Read();
  I2C_Ack();
  ret.b[1] = I2C_Read();
  I2C_Ack();
  ret.b[0] = I2C_Read();
  I2C_Ack();

  adc_T = (ret.dw >> 4) & 0xFFFFF;

  ret.b[2] = 0x00;
  ret.b[1] = I2C_Read();
  I2C_Ack();
  ret.b[0] = I2C_Read();
  I2C_Nack();
  I2C_Stop();

  adc_H = ret.dw  & 0xFFFF;
}

bool BME280_readTemperature(int32_t *temp)
{
int32_t var1,var2;
  BME280_Update();

  // calculate temperature
  var1 = ((((adc_T /8) - ((int32_t)BME280_calib.dig_T1 *2))) *
         ((int32_t)BME280_calib.dig_T2))/2048;

  var2 = (((((adc_T /16) - ((int32_t)BME280_calib.dig_T1)) *
         ((adc_T /16) - ((int32_t)BME280_calib.dig_T1))) /4096) *
         ((int32_t)BME280_calib.dig_T3)) / 16384;

  t_fine = var1 + var2;

  *temp = (t_fine * 5 + 128)/256;

  return 1;
}

// Reads humidity from BME280 sensor.
// Humidity is stored in relative humidity percent in 1024 steps
// (output value of "47445" represents 47445/1024 = 46.333 %RH).
// Humidity value is saved to *humi, returns 1 if OK and 0 if error.
bool BME280_readHumidity(uint32_t *humi)
{
  int32_t v_x1_u32r;
  uint32_t H;

  v_x1_u32r = (t_fine - ((int32_t)76800));

  v_x1_u32r = (((((adc_H * 16384) - (((int32_t)BME280_calib.dig_H4) * 1048576) - (((int32_t)BME280_calib.dig_H5) * v_x1_u32r)) +
      ((int32_t)16384))/ 32768) * (((((((v_x1_u32r * ((int32_t)BME280_calib.dig_H6))/ 1024) * (((v_x1_u32r *
      ((int32_t)BME280_calib.dig_H3)) / 2048) + ((int32_t)32768))) / 1024) + ((int32_t)2097152)) *
      ((int32_t)BME280_calib.dig_H2) + 8192) / 16384));
      
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r / 32768) * (v_x1_u32r / 32768)) / 128) * ((int32_t)BME280_calib.dig_H1)) / 16));
  if(v_x1_u32r<0)
      v_x1_u32r=0;
  if(v_x1_u32r>419430400)
      v_x1_u32r=419430400;
   
  H = (uint32_t)(v_x1_u32r / 4096);
  *humi = H;

  return 1;
}

// Reads pressure from BME280 sensor.
// Pressure is stored in Pa (output value of "96386" equals 96386 Pa = 963.86 hPa).
// Pressure value is saved to *pres, returns 1 if OK and 0 if error.
bool BME280_readPressure(uint32_t *pres)
{
  int32_t var1, var2;
  uint32_t p;

  // calculate pressure 24 integer bits and 8 fractional bits
  var1 = (((int32_t)t_fine) / 2) - 64000;
  
  var2 = (((var1 / 4) * (var1/4)) /2048 ) * ((int32_t)BME280_calib.dig_P6);
  var2 = var2 + ((var1 * ((int32_t)BME280_calib.dig_P5)) *2);
  var2 = (var2/4) + (((int32_t)BME280_calib.dig_P4) * 65536);

  var1 = ((((int32_t)BME280_calib.dig_P3 * (((var1/4) * (var1/4)) /8192 )) /8) +
         ((((int32_t)BME280_calib.dig_P2) * var1)/2)) /262144;
  var1 =((((32768 + var1)) * ((int32_t)BME280_calib.dig_P1)) / 32768);
  
  if (var1 == 0)
    return 0; // avoid exception caused by division by zero

  p = (((uint32_t)((1048576) - adc_P) - (uint32_t)(var2 /4096))) * 3125;

  if (p < 0x80000000)
    p = (p * 2 ) / ((uint32_t)var1);

  else
    p = (p / (uint32_t)var1) * 2;

  var1 = (((int32_t)BME280_calib.dig_P9) * ((int32_t)(((p/8) * (p/8)) /8192))) /4096;
  var2 = (((int32_t)(p/4)) * ((int32_t)BME280_calib.dig_P8)) /8192;

  p = (uint32_t)((int32_t)p + ((var1 + var2 + (int32_t)BME280_calib.dig_P7) /16));

  *pres = p;

  return 1;
}
