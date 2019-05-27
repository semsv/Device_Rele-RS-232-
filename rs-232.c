#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>  
#include <sys/ioctl.h>

int F_ID      = -1; /* Дескриптор порта*/
int debug_out = 0;  /* Выводить или нет дебаг информацию*/

/*----------------------------------------------------------------------------
 * Пример работы с платой реле по интерфейсу RS-232 (COM) или
 *                             через переходник USB -> RS-232
 * 
 * Автор: Севастьянов Семен Владимирович
 * 
 *  Открыть COM порт: 
 *          COM_name: путь к устройству, например: /dev/ttyS0 или  /dev/ttyUSB0 - для USB
 *             speed: скорость, например: B9600, B57600, B115200 
 * 
 *  Примеры команд:  rele 0 1 on    # Включение 1-го реле
 *                   rele 0 2 on    # Включение 2-го реле
 *                   rele 0 1 off   # Выключение 1-го реле
 *                   rele 0 2 off   # Выключение 2-го реле
 * 
 *  первый параметр: "0" или "1" выбирает порт 
 *                   /dev/ttyS0 или  /dev/ttyUSB0 соответственно
 * 
 *  второй параметр: "1" или "2" выбирает реле 
 *                   1-ое или 2-ое реле соответственно
 *
 *  третий параметр: "on" или "off" выбирает действие 
 *                   включить или выключить соответственно
 ----------------------------------------------------------------------------*/


int OpenPort(const char *COM_name,speed_t speed)
{
    F_ID = open(COM_name, O_RDWR | O_NOCTTY | O_SYNC); // 
    if(F_ID == -1)
    {
        char *errmsg = strerror(errno);
        printf("%s\n",errmsg);
        return 0;
    }
    struct termios options;        /*** структура для установки порта ***/
    tcgetattr(F_ID, &options);     /*** читает пораметры порта        ***/
    
    if (debug_out == 1)
    {
        printf("c_cflag = %i\n", options.c_cflag);
        printf("c_oflag = %i\n", options.c_oflag);
        printf("c_lflag = %i\n", options.c_lflag);
    }	
    
    options.c_cflag  &= ~CBAUD;
    cfsetispeed(&options, B9600);  /*** установка скорости порта ***/
    cfsetospeed(&options, B9600);  /*** установка скорости порта ***/

    if (debug_out)
    {
      printf("c_cflag = %i\n", options.c_cflag);
      printf("c_oflag = %i\n", options.c_oflag);
      printf("c_lflag = %i\n", options.c_lflag); 
    }  

    options.c_cc[VTIME]    = 25;   /*** Время ожидания байта 20*0.1 = 1 секунды ***/
    options.c_cc[VMIN]     = 0;    /*** минимальное число байт для чтения       ***/
    
    if (debug_out) printf("EOF = %i\n", options.c_cc[VEOF]);
    
    options.c_cc[VEOF]     = 0x04;
 
    options.c_cflag &= ~PARENB;    /*** бит четности не используется ***/
    options.c_cflag &= ~CSTOPB;    /*** 1 стоп бит                   ***/
    options.c_cflag &= ~CSIZE;     /*** Размер байта                 ***/
    options.c_cflag |= CS8;        /*** 8 бит                        ***/

    options.c_cflag &= ~CRTSCTS;
    options.c_iflag &= ~(INLCR | ICRNL);
    options.c_cflag |= (CREAD | CLOCAL);  
    
          
    options.c_oflag &= ~OPOST;     /*** Обязательно отключить постобработку ***/    
    options.c_lflag = 0x00;             
    
    tcsetattr(F_ID, TCSANOW | TCSAFLUSH, &options);      /*** сохронения параметров порта ***/
    // tcsetattr(F_ID, TCSADRAIN | TCSAFLUSH, &options); /*** сохронения параметров порта ***/
    // tcsetattr(F_ID, TCSADRAIN, &options);             /*** сохронения параметров порта ***/
    // tcsetattr(F_ID, TCSETSF, &options);               /*** сохронения параметров порта ***/
    // tcsetattr(F_ID, TCSETSW, &options);               /*** сохронения параметров порта ***/
    
    TIOCSBRK;
    TCSBRK;
    
    if (debug_out) 
    {
        printf("c_cflag = %i\n", options.c_cflag);
        printf("c_oflag = %i\n", options.c_oflag);
        printf("c_lflag = %i\n", options.c_lflag);
    }	
    
    int serial=0;
    ioctl(F_ID, TIOCMGET, &serial);
    serial |= TIOCM_DTR;  // HOST is ready 
    ioctl(F_ID, TIOCMSET, &serial);
    if (serial & TIOCM_DTR)
    {
      if (debug_out) printf("SET DTR SUCCESS\n");
    } else 
    { 
      printf("SET DTR FAULT\n"); 
    }
      
    return 1;
}

/*----------------------------------------------------------------------------
 Прочитать данные из COM порта
 buff - буфер для принятых данных
 size - количество запрашиваемых байт
 ----------------------------------------------------------------------------*/
int readData(unsigned char *buff,int size)
{
    int n = read(F_ID, buff, size);
    if(n == -1)
    {
        char *errmsg = strerror(errno);
        printf("%s\n",errmsg);
    }
    return n;
}

/*----------------------------------------------------------------------------
 Отправить в COM порт данные
 buff - буфер данных для отправки
 size - количество отправляемых байт
 ----------------------------------------------------------------------------*/
int sendData(unsigned char* buff,int len)
{
    int n = write(F_ID, buff, len);
    if(n == -1)
    {
        char *errmsg = strerror(errno);
        printf("%s\n",errmsg);
    }
    

    return n;
}
/*----------------------------------------------------------------------------
 Закрыть COM порт
 ----------------------------------------------------------------------------*/
void ClosePort(void)
{
    close(F_ID);
    F_ID = -1; 
    return;
}
/*----------------------------------------------------------------------------
 Установить RTS
 ----------------------------------------------------------------------------*/
void setRTS(int direct)
{
  // TIOCM_LE	DSR (data set ready/line enable)
  // TIOCM_DTR	DTR (data terminal ready)
  // TIOCM_RTS	RTS (request to send)
  // TIOCM_ST	Secondary TXD (transmit)
  // TIOCM_SR	Secondary RXD (receive)
  // TIOCM_CTS	CTS (clear to send)
  // TIOCM_CAR	DCD (data carrier detect)
  // TIOCM_CD	Synonym for TIOCM_CAR
  // TIOCM_RNG	RNG (ring)
  // TIOCM_RI	Synonym for TIOCM_RNG
  // TIOCM_DSR	DSR (data set ready)

    int status;
    ioctl(F_ID, TIOCMGET, &status);
    if (status & TIOCM_CTS & debug_out) {
      printf("CTS\n"); 
    }
    if (status & TIOCM_RTS & debug_out) {
      printf("RTS\n"); 
    }
    if (status & TIOCM_DTR & debug_out) {
      printf("DTR\n"); 
    }
    if (status & TIOCM_LE & debug_out) {
      printf("LE\n"); 
    }
    if (status & TIOCM_DSR & debug_out) {
      printf("DSR\n"); 
    }
    if (status & TIOCM_CAR & debug_out) {
      printf("CAR\n"); 
    }
    if (status & TIOCM_RI & debug_out) {
      printf("RI\n"); 
    }
    if (status & TIOCM_ST & debug_out) {
      printf("ST\n"); 
    }
    if (status & TIOCM_SR & debug_out) {
      printf("SR\n"); 
    }
    if (status & TIOCM_CD & debug_out) {
      printf("CD\n"); 
    }
    if (status & TIOCM_RNG & debug_out) {
      printf("RNG\n"); 
    }
    
    if (direct == 0)
    {
      status &= ~TIOCM_CTS; // DEVICE READY = 0 (RECEIVER)
      status |= TIOCM_RTS;  // HOST   READY = 1 (TRANSFER)
      status |= TIOCM_DTR;  // HOST is ready 
      status |= TIOCM_DSR;      
    } else
    {
      status |= TIOCM_CTS;   // DEVICE READY = 1 (RECEIVER)
      status &= ~TIOCM_RTS;  // HOST   READY = 0 (TRANSFER)
      status |= TIOCM_DTR;   // HOST is ready 
    }
    
    ioctl(F_ID, TIOCMSET, &status);
    usleep(100);
    ioctl(F_ID, TIOCMGET, &status);
    
    if (debug_out) printf("SET DTR F_ID=%i STATUS = %i\n", F_ID, status);
}
/*----------------------------------------------------------------------------
Очистить RTS
 ----------------------------------------------------------------------------*/
void clrRTS()
{
    int status;
    ioctl(F_ID, TIOCMGET, &status);
    status &= ~TIOCM_CTS; // DEVICE READY = 0
    status &= ~TIOCM_RTS; // HOST   READY = 0
    status &= ~TIOCM_DTR; // HOST is not ready
    status &= ~TIOCM_DSR;
    ioctl(F_ID, TIOCMSET, &status);
    usleep(100);
    ioctl(F_ID, TIOCMGET, &status);
  
    if (debug_out) printf("CLR DTR F_ID=%i STATUS = %i\n", F_ID, status);
}
/*----------------------------------------------------------------------------
Пример использования
 ----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    // ioperm(0, 0xFFFFFFFF); // io - port- granted access
    if (debug_out) printf("argc = %i\n", argc);
    
    int res = 0;
    if (argc > 0)
    if (strstr(argv[1], "0") != 0)
    {
      res = OpenPort("/dev/ttyS0",B9600);
      printf("/dev/ttyS0\n");
    } else  
    { 
      res = OpenPort("/dev/ttyUSB0",B9600);
      printf("/dev/ttyUSB0\n");
    }  
    
    if(res==0)
    {
        printf("Невозможно открыть COM порт\n");
        return 0;
    }
    
    unsigned char rele_1[8] = {0x55,0x56,0x00,0x00,0x00,0x01,0x01, 0xAD};
    unsigned char rele_2[8] = {0x55,0x56,0x00,0x00,0x00,0x02,0x01, 0xAE};
    unsigned char rele_3[8] = {0x55,0x56,0x00,0x00,0x00,0x03,0x01, 0xAF};
    unsigned char rele_4[8] = {0x55,0x56,0x00,0x00,0x00,0x04,0x01, 0xB0};
    
    
    if (argc > 3) {
     if (strstr(argv[3], "off") != 0) {
       rele_1[6]++;
       rele_1[7]++;
       rele_2[6]++;
       rele_2[7]++;
       rele_3[6]++;
       rele_3[7]++;
       rele_4[6]++;
       rele_4[7]++;
     }
    } else if (argc == 3)
    {
       if (strstr(argv[2], "off") != 0) {
         rele_1[6]++;
         rele_1[7]++;
         rele_2[6]++;
         rele_2[7]++;
	 rele_3[6]++;
         rele_3[7]++;
         rele_4[6]++;
         rele_4[7]++;
       }
    }     

    if (argc > 2) {
    if (strstr(argv[2], "1") != 0)
     { 
      sendData(rele_1, 8);
     } 
    if (strstr(argv[2], "2") != 0)  
     {
      sendData(rele_2, 8);
     }
    if (strstr(argv[2], "3") != 0)  
     {
      sendData(rele_3, 8);
     } 
    if (strstr(argv[2], "4") != 0)  
     {
      sendData(rele_4, 8);
     } 
    if (strstr(argv[2], "off") != 0 || strstr(argv[2], "on"))  
     {
      sendData(rele_1, 8);
      usleep(50); clrRTS(); usleep(50); setRTS(0);
      sendData(rele_2, 8);
     }
    } else
    {
     sendData(rele_1, 8);
     usleep(50); clrRTS(); usleep(50); setRTS(0);
     sendData(rele_2, 8);
    }
    
    usleep(50);
    
    printf("\n");
    unsigned char rbuff[8] = {0,0,0,0,0,0,0,0};
    int s = readData(rbuff, 8);
   
    if(s < 8)
    {
        printf("Not responding, receiving: %i byte \n", s);
        return 0;	
    } else
    {
        printf("%i, %i, %i, %i, %i, %i, %i, %i, получено байт: %i \n", 
	       rbuff[0],
	       rbuff[1],
	       rbuff[2],
	       rbuff[3],
	       rbuff[4],
	       rbuff[5],
	       rbuff[6],
	       rbuff[7],
	       s);
        return 0;	
    }
 /*
 Наш кодcloseCom
  */
 ClosePort();
    return 0;
}
 
