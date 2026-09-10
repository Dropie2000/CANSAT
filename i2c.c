/* #include "i2c.h"
*
* Creada por: Ing. Abiezer Hernandez O.
* Fecha de creacion: 25/11/2019
* Electronica y Circuitos
*
*/

#include "i2c.h"
#include "uart.h"

#ifdef I2C_MASTER_MODE
void I2C_Init_Master(unsigned char sp_i2c)
{
    ANSELB=0;
    TRIS_SCL = 1;
    TRIS_SDA = 1;
    SSP1STAT = sp_i2c;
	SSP1CON1 = 0x28;
    SSP1CON2 = 0x00;
    ///La frecuencia del I2C se establece según la pagina 250 de la hoja de datos
    //en donde Fbaud=Fosc/((SSPADD+1)4), por lo que 
    //SSPADD=Fosc/((Fbaud)4)-1
    //Para un cristal de 20MHz
    if(sp_i2c == I2C_100KHZ){
        SSP1ADD = 49;//49 //119
    }
    else if(sp_i2c == I2C_400KHZ){
        SSP1ADD = 12;//12   //29
    }
}

void I2C_Start(void)
{
    SSP1CON2bits.SEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

void I2C_Stop(void)
{
    SSP1CON2bits.PEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

void I2C_Restart(void)
{
    SSP1CON2bits.RSEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

void I2C_Ack(void)
{
    SSP1CON2bits.ACKDT = 0;
	SSP1CON2bits.ACKEN = 1;
	while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

void I2C_Nack(void)
{
    SSP1CON2bits.ACKDT = 1;
	SSP1CON2bits.ACKEN = 1;
	while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
}

short I2C_Write(char data)
{
    SSP1BUF = data;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
    short b_ok = SSP1CON2bits.ACKSTAT;
    return b_ok;
}

unsigned char I2C_Read(void)
{
    SSP1CON2bits.RCEN = 1;
    while(PIR1bits.SSPIF == 0);
    PIR1bits.SSPIF = 0;
    return SSP1BUF;
}
#endif

#ifdef I2C_SLAVE_MODE
void I2C_Init_Slave(unsigned char add_slave)
{
    TRIS_SCL = 1;
    TRIS_SDA = 1;
    SSPSTAT = 0x80;
    SSPADD = add_slave;
    SSPCON = 0x36;
    SSPCON2 = 0x01;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
    PIR1bits.SSPIF = 0;
    PIE1bits.SSPIE = 1;
}

short I2C_Error_Read(void)
{
    SSPCONbits.CKP = 0;
    return ((SSPCONbits.SSPOV) || (SSPCONbits.WCOL)) ? 1 : 0;
}

short I2C_Write_Mode(void)
{
    return(!SSPSTATbits.D_nA && !SSPSTATbits.R_nW) ? 1 : 0;
}
short I2C_Read_Mode(void)
{
    return (!SSPSTATbits.D_nA && SSPSTATbits.R_nW) ? 1: 0;
}

void I2C_Error_Data(void)
{
    short z;
    SSPCONbits.CKP = 0;
    z = SSPBUF;
    SSPCONbits.SSPOV = 0;
    SSPCONbits.WCOL = 0;
    SSPCONbits.CKP = 1;
    SSPCONbits.SSPM3 = 0;
}

unsigned char I2C_Read_Slave(void)
{
    short z;
    unsigned char dato_i2c;
    z = SSPBUF;
    while(!BF);
    dato_i2c = SSPBUF;
    SSPCONbits.CKP = 1;
    SSPCONbits.SSPM3 = 0;
    return dato_i2c;
}

void I2C_Write_Slave(char dato_i2c)
{
    short z;
    z = SSPBUF;
    BF = 0;
    SSPBUF = dato_i2c;
    SSPCONbits.CKP = 1;
    while(SSPSTATbits.BF == 1);
}
#endif