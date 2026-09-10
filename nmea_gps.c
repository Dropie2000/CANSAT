/* #include "nmea_gps.h"
*
* Creada por: Ing. Abiezer Hernandez O.
* Fecha de creacion: 18/04/2022
* Electronica y Circuitos
*
*/

#define _XTAL_FREQ 20000000
#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "uart.h"
#include "nmea_gps.h"

char dato_rx;
char buffer[12];
char sentence[6];

uint8_t sentence_type = 0;
uint8_t cont_buf = 0;
uint8_t cont_rx = 0;
short GPRMC_flag = 0;
short GPGGA_flag = 0;

char rawTime[11], rawDate[7], rawSpeed[6], rawCourse[6],
     rawSatellites[3], rawLatitude[13], rawLongitude[13], rawAltitude[7];

short GPS_Get_Data(void)
{
    dato_rx = Uart_Read();
    
    switch(dato_rx)
    {
        case '$':
            cont_rx = 0;
            cont_buf = 0;
            break;

        case ',':
            buffer[cont_rx] = '\0';
            if(cont_buf == 0)
            {
                strcpy(sentence, buffer);
                if(!strcmp(sentence, "GPRMC")){
                    sentence_type = _GPRMC_;
                }
                else if(!strcmp(sentence, "GPGGA")){
                    sentence_type = _GPGGA_;
                }
                else{
                    sentence_type = _OTHER_;
                }
            }
            // Tiempo
            else if((cont_buf == 1) && (sentence_type == _GPRMC_)){
                strcpy(rawTime, buffer);
            }
            // Latitud
            else if((cont_buf == 3) && (sentence_type == _GPRMC_)){
                uint8_t chr = 0;
                do{
                    rawLatitude[chr+1] = buffer[chr];
                } while(buffer[chr++] != '\0');
            }
            // Latitud N/S
            else if((cont_buf == 4) && (sentence_type == _GPRMC_)){
                if(buffer[0] == 'N'){
                    rawLatitude[0] = '0';
                }else{
                    rawLatitude[0] = '-';
                }
            }
            // Longitud
            if((cont_buf == 5) && (sentence_type == _GPRMC_)){
                uint8_t chr = 0;
                do{
                    rawLongitude[chr+1] = buffer[chr];
                } while(buffer[chr++] != '\0');
            }
            // Longitud E/W
            else if((cont_buf == 6) && (sentence_type == _GPRMC_)){
                if(buffer[0] == 'E'){
                    rawLongitude[0] = '0';
                }else{
                    rawLongitude[0] = '-';
                }
            }
            // Velocidad
            else if((cont_buf == 7) && (sentence_type == _GPRMC_)){
                strcpy(rawSpeed, buffer);
            }
            // Curso
            else if((cont_buf == 8) && (sentence_type == _GPRMC_)){
                strcpy(rawCourse, buffer);
            }
            // Fecha
            else if((cont_buf == 9) && (sentence_type == _GPRMC_)){
                strcpy(rawDate, buffer);
            }
            // Satelites
            else if((cont_buf == 7) && (sentence_type == _GPGGA_)){
                strcpy(rawSatellites, buffer);
            }
            // Altitud
            else if((cont_buf == 9) && (sentence_type == _GPGGA_)){
                strcpy(rawAltitude, buffer);
            }
            cont_buf++;
            cont_rx = 0;
            break;

        case '\r':
            if(sentence_type == _GPRMC_){
                GPRMC_flag = 1;
            }
            if(sentence_type == _GPGGA_){
                GPGGA_flag = 1;
            }
            if(GPRMC_flag == 1 && GPGGA_flag == 1){
                GPRMC_flag = 0;
                GPGGA_flag = 0;
                return 1;
            }
            break;

        default:
            buffer[cont_rx++] = dato_rx;
            break;
    }
    return 0;
}

uint8_t GPS_Second(void)
{
    return ((rawTime[4] - '0')*10 + (rawTime[5] - '0'));
}

uint8_t GPS_Minute(void)
{
    return ((rawTime[2] - '0')*10 + (rawTime[3] - '0'));
}

uint8_t GPS_Hour(void)
{
    return ((rawTime[0] - '0')*10 + (rawTime[1] - '0'));
}

uint8_t GPS_Day(void)
{
    return ((rawDate[0] - '0')*10 + (rawDate[1] - '0'));
}

uint8_t GPS_Month(void)
{
    return ((rawDate[2] - '0')*10 + (rawDate[3] - '0'));
}

uint8_t GPS_Year(void)
{
    return ((rawDate[4] - '0')*10 + (rawDate[5] - '0'));
}

float GPS_Altitude(void)
{
    return (float)atof(rawAltitude);
}

uint8_t GPS_Satellites(void)
{
    return (uint8_t)atoi(rawSatellites);
}

float GPS_Latitude(void)
{
    float val = (float)(atof(rawLatitude)/100);
    int16_t dec = (int16_t)val;
    val -= dec;
    val  = val * 5/3 + dec;
    return (float)val;
}

float GPS_Longitude(void)
{
    float val = (float)(atof(rawLongitude)/100);
    int16_t dec = (int16_t)val;
    val -= dec;
    val  = val * 5/3 + dec;
    return (float)val;
}

float GPS_Speed(void)
{
    return (float)(atof(rawSpeed) * 1.852);
}

float GPS_Course(void)
{
    return (float)atof(rawCourse);
}