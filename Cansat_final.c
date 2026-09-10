/*
 * File:   Cansat_final.c
 * Author: 52442
 *
 * Created on November 22, 2023, 3:46 PM
 */

#include <xc.h>
#include "configuracion.h"
#define _XTAL_FREQ 20000000
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "BME280Lib.h"
#include "INA219Lib.h"
#include "i2c.h"
#include "uart.h"
#include "nmea_gps.h"
#include <string.h>

double voltaje, corriente, potencia, t_bateria,voltaje_ADC;
int32_t temp;
uint32_t hum;
uint32_t pres;
uint8_t ID;
uint8_t adress;
int16_t lectura_adc;
int STATBME;
char datos_rx[30];

void main(void) {
    // Configuración del pin E0 como entrada analogica
    ANSELE=0xFF;        //Colocar todo el puerto E como analógico
    TRISEbits.TRISE0=1;
    // Configurar la conversión analógica
    ADCON0=0b00010100;  //Configurar el canal AN5, conservar el canal de conversión apagado
    ADCON1=0b00000000;  //Configurar la referencia de voltaje como VDD y VSS
    ADCON2=0b10011011;  //Justificar la medición a la derecha, con 6 TAD y el reloj derivado del oscilador interno
    //Configurar el puerto UART
    Uart_Init(9600);
    __delay_ms(20);
    //Configurar el I2C
    I2C_Init_Master(I2C_100KHZ);
    __delay_ms(10);
    //Inicializar el BME
    STATBME=BME280_begin(MODE_NORMAL,SAMPLING_X1,SAMPLING_X1,SAMPLING_X1, FILTER_OFF,STANDBY_0_5);
    __delay_ms(60);
    //Inicializar el INA
    INA_begin(0x119F,0x346D); //Configuración del INA, lectura de 12 bits, PGA=4, modo continuo
    //y calibración, con un valor de 26841, para una corriente máxima esperada de 0.5A
    __delay_ms(20);
    //Habilitar el ADC
    ADCON0bits.ADON=1;
    __delay_ms(10);
    while(1)
    {
        ADCON0bits.GODONE=1;            //Iniciar la conversión
        while(ADCON0bits.GODONE==1);     //Esperar que se termine la conversión
        lectura_adc=(ADRESH << 8)+ ADRESL;  //Guardar la lectura del ADC
        __delay_ms(10);
        voltaje_ADC=lectura_adc*0.00488;    //Obtener la lectura del voltaje en E0
        t_bateria=voltaje_ADC*10000;     //Se obtiene la temperatura multiplicada por 10000 para procesar el dato en labview
                
        while(GPS_Get_Data()<1);
        BME280_readTemperature(&temp);
        BME280_readHumidity(&hum);
        BME280_readPressure(&pres);
        voltaje=Get_bus_voltage()*100;
        corriente=Get_current()*1000;
        potencia=Get_power()*1000;
        // Impresion datos INA
        Uart_Send_String(":");
        sprintf(datos_rx,"%.1f",voltaje);
        Uart_Send_String(datos_rx);
        Uart_Send_String(",");
        sprintf(datos_rx,"%.1f",corriente);
        Uart_Send_String(datos_rx);
        Uart_Send_String(":");
        sprintf(datos_rx,"%.1f",potencia);
        Uart_Send_String(datos_rx);
        Uart_Send_String(",");
    //Impresion datos BME
        sprintf(datos_rx,"%02u",temp);
        Uart_Send_String(datos_rx);
        Uart_Send_String(":");
        sprintf(datos_rx,"%02u",hum);
        Uart_Send_String(datos_rx);
        Uart_Send_String(",");
        sprintf(datos_rx,"%02u",pres);
        Uart_Send_String(datos_rx);
        Uart_Send_String(":");
    // Impresión datos GPS
        sprintf(datos_rx,"%02u,%02u:%02u",GPS_Hour(),GPS_Minute(),GPS_Second());
        Uart_Send_String(datos_rx);
        Uart_Send_String(",");
        sprintf(datos_rx,"%02u:%.2f,%.2f:%.2f",GPS_Satellites(),GPS_Latitude(),GPS_Longitude(),GPS_Altitude());
        Uart_Send_String(datos_rx);
        Uart_Send_String(",");
        sprintf(datos_rx,"%02u:%02u,%02u",GPS_Day(),GPS_Month(),GPS_Year());
        Uart_Send_String(datos_rx);
        Uart_Send_String(":");
        sprintf(datos_rx,"%.2f,%.2f:",GPS_Speed(),GPS_Course());
        Uart_Send_String(datos_rx);
        sprintf(datos_rx,"%.2f,",t_bateria);
        Uart_Send_String(datos_rx);
        Uart_Send_String("\r\n");
    }
}
