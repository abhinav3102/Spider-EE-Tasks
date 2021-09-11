#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
int slave_address = 0b10011010; //SLA+W
int rslave_address = 0b10011011; // SLA+R
int status, flag= 2;
unsigned char hmessage[100] = "Temperature above 50 degrees!";
unsigned char lmessage[100] = "Temperature below 50 degrees";
unsigned int temperature = 0;
void SendData (unsigned char cnt);
unsigned char counter;
unsigned int ubrr = 51;
int main()
{
  TWSR = (0<<TWPS0) | (0<<TWPS1); //setting prescale
  TWBR = 0x30; //setting frequency 
  UCSR0B = (1<<TXEN0)|(1<<RXEN0); //enabling TXD and RXD pins
  UCSR0C = 0;
  UCSR0C = (1<<UCSZ00)|(1<<UCSZ01); // 8 bit data to be transferred followed by 1 stop bit
  UBRR0L = ubrr;
  UBRR0H = (ubrr>>8); // baud rate = 9600
  DDRB = 0xFF;
  while(1)
  {
    //sending START condition
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
    //waiting till start condition is transmitted
    while (!(TWCR & (1<<TWINT)));
    while(status != 0x08)
    {
      status = TWSR;
    }
    
    //loading SLA+W
    TWDR = slave_address;
    TWCR = (1<<TWINT)|(1 << TWEN);
    //waiting for completion of transmission of address
    while (!(TWCR & (1<<TWINT)));
    while(status != 0x18)
    {
      status = TWSR;
    }
    
    //sending command to access temperature register
    TWDR = 0x00;
    TWCR = (1<<TWINT)|(1 << TWEN);
    //waiting for completion of transmission of address and acknowledgments
    while (!(TWCR & (1<<TWINT)));
    while(status != 0x28)
    {
      status = TWSR;
    }
    
    //sending RESTART condition
    TWCR = (1<<TWINT)|(1<<TWSTA)|(1<<TWEN);//|(1<<TWEA);
    //waiting till start condition is transmitted
    while (!(TWCR & (1<<TWINT)));
    while(status != 0x10)
    {
      status = TWSR;
    }
    //loading SLA+R
    TWDR = rslave_address;
    TWCR = (1<<TWINT)|(1 << TWEN);
    //waiting for completion of transmission of address
    while (!(TWCR & (1<<TWINT)));
    while(status != 0x40)
    {
      status = TWSR;
    }
    TWCR = (1<<TWINT)|(0<<TWEA)|(1<<TWEN);
    
    //receiving data from sensor
    while(!(TWCR&(1<<TWINT)));
    while(status != 0x58)
    {
      status = TWSR;
    }
    temperature = TWDR;
    TWCR = (1<<TWINT)|(1<<TWSTO)|(1<<TWEN);

    //enabling fan DC if temperature is above 50 degrees
    if(temperature >=50)
    { PORTB = (1<<0);
      if((flag==0) || (flag==2))
      {
        SendData (hmessage);
        flag=1;
        while ((UCSR0A & (1<<UDRE0)) == 0);
        UDR0 = '\r';
      }
    }
    
    else if(temperature<50)
    {
      PORTB = 0;
      if((flag==1) || (flag==2))
      {
        SendData(lmessage);
        flag=0;
        while ((UCSR0A & (1<<UDRE0)) == 0);
        UDR0 = '\r';
      }
    }
    _delay_ms(500);
    }
  
}



void SendData (unsigned char *arr)
{
  unsigned char loop;
  
  int cnt;
  cnt = strlen(arr);
  for (loop = 0; loop < cnt; loop++)
  {
    while ((UCSR0A & (1<<UDRE0)) == 0);
    UDR0 = arr[loop];
  }
}