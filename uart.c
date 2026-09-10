/* #include "uart.h"
*
* Creada por: Ing. Abiezer Hernandez O.
* Fecha de creacion: 22/11/2019
* Electronica y Circuitos
*
*/

#include "uart.h"

void Uart_Init(unsigned long baud)                  // Inicialzia el modulo USART
{
    ANSELC=0x00;
    TRISCbits.TRISC6 = 0;                              // Pin TX como salida
    TRISCbits.TRISC7 = 1;                              // Pin RX como entrada
    BAUDCON1bits.BRG16=1;                            //Configurar baud rate de 16 bits   
    TXSTA1bits.BRGH=1;                               //Configurar baud rate de alta velocidad
    // Para el calculo del baud rate, Baud=Fosc/(4(n+1)) en donde n es el valor en 
    //SPBRG, si se resuleve para n -> n= (Fosc/Baud*4)-1
    unsigned int vx = (_XTAL_FREQ/(baud*4))-1;
    SPBRG1 = vx & 0x00FF;                            // Carga los baudios de la parte baja
    SPBRGH1 = vx >> 8;                               // Carga los baudios de la parte alta
    TXSTA1bits.SYNC=0;                               //Configurar modulo eusart en modo asincrono
    RCSTA1bits.SPEN=1;                               //Habilitar el puerto serial
    RCSTA1bits.CREN=1;                               //Habilitar el receptor serial
    TXSTA1bits.TXEN=1;                               //Habilitar la transmision
}

void Uart_Send_Char(char txData)         			// Funcion para transmitir caracteres
{
    while(TXSTA1bits.TRMT == 0);
    TXREG1 = txData;
}

void Uart_Send_String(char *info)          			// Funcion para transmitir una cadena de caracteres
{
    while(*info)
    {
        Uart_Send_Char(*info++);
    }
}

char Uart_Read(void)                        		// Funcion para recibir caracteres
{
    while(PIR1bits.RCIF == 0);
    if(RCSTA1bits.OERR == 1)
    {
        RCSTA1bits.CREN = 0;
        RCSTA1bits.CREN = 1;
    }
    PIR1bits.RCIF = 0;
    return RCREG1;
}
                                                    // Funcion para recibir cadenas de caracteres
/*void Uart_Read_String(char* Buf, unsigned int s_buf)
{
    unsigned int cont_buf = 0;
    char c;
    do
    {
        c = Uart_Read();
        Buf[cont_buf++] = c;
        if(cont_buf >= s_buf) break;
    }while(c != '\n');
    Buf[cont_buf-2] = '\0';
    cont_buf = 0;
}
                                                    // Funcion para descomponer cadenas de caracteres
void Uart_Read_Substring(char* cp, char* vc, int inc, int fn)
{
    int pt = 0;
    for(int lt=inc; lt<=fn; lt++)
    {
        vc[pt] = cp[lt];
        pt++;
    }
    vc[pt] = '\0';
    pt = 0;
}*/
                                                    // Funcion para descomponer cadenas de caracteres, bit stop
/*void Uart_Read_StringUntil(char stop_c, char* buf, unsigned int st_size)
{
    unsigned int rx_cont = 0;
    while(*(buf-1) != stop_c)
    {
        rx_cont++;
        *buf++ = Uart_Read();
        if(rx_cont >= st_size) break;
    }
    *--buf = 0;
}

void Uart_Read_Plot(char b_ini, char b_fin, char* tr_or, char* n_str, int s_buf)
{
    int conta_ini = 0;
    int conta_fin = 0;
    for(int i=0; i<s_buf; i++)
    {
        if(tr_or[i] == b_ini){
            conta_ini = i;
            break;
        }
    }
    for(int j=0; j<s_buf; j++)
    {
        if(tr_or[j] == b_fin){
            conta_fin = j;
            break;
        }
    }
    Uart_Read_Substring(tr_or, n_str, conta_ini+1, conta_fin-1);
}*/

/*short Uart_Available(void)                          // Funcion de verificacion si ha recibido datos
{
    return PIR1bits.RCIF ? 1 : 0;
}*/