#include<avr/io.h>
#include<stdint.h> //for int8_t
#include<String.h>
#include<stdlib.h>

#define F_CPU 16000000UL //clock frequency 16MHz

void i2c_init()
{
  TWSR=0x00; //prescaler bits set to zero
  TWBR=0x98; //SCL Frequency is 50kHz for clock frequency of 16MHz and bit rate 0x98, or 152 in decimal
  TWCR=(1<<TWEN); 
}

void i2c_start()
{
  TWCR=(1<<TWINT)|(1<<TWSTA)|(1<<TWEN);
  while((TWCR&(1<<TWINT))==0);
}

void i2c_write(char data)
{
  TWDR=data;
  TWCR=(1<<TWINT)|(1<<TWEN);
  while((TWCR&(1<<TWINT))==0);
}

char i2c_read(char isLast)
{
  if(isLast==0)
  {
    TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWEA);
  }
  else
  {
    TWCR=(1<<TWINT)|(1<<TWEN);
  }
  while((TWCR&(1<<TWINT))==0);
  return TWDR;
}

void i2c_stop()
{
  TWCR=(1<<TWINT)|(1<<TWEN)|(1<<TWSTO);
}

int main()
{
  int8_t temp=0;
  char s1[]="Room is safe. Temperature is ";
  char s2[]="Room is on fire!!! Temperature is ";
  char temp_string[4];
  char s3[]=" degrees C.";
  char serial_output[100];
  
  DDRD=(1<<PD2); //initializing the fan pin as output
  
  i2c_init();
  Serial.begin(9600);
  
  while(1)
  {    
    /*the following series of i2c function calls, from the first i2c_start()
    to the last i2c_stop() is done in accordance with the read format for 
    temperature, mentioned in datasheet of TC74*/
    
    i2c_start();
    i2c_write(0b10011010); //address of slave (TC74), SLA is 0b1001101 + W(0). Write, because we will be writing the command to read temperature
    i2c_write(0x00); //0x00 is the command for TC74 to read temperature
    i2c_start();
    i2c_write(0b10011011); //address of slave (TC74), SLA is 0b1001101 + R(1). Read, because we will be reading temperature
    temp=i2c_read(1);
    i2c_stop();
    
    if(temp>=50)
    {
      PORTD|=(1<<PD2); //turn fan on
      strcpy(serial_output,s2); //copying string s2 to serial_output
    }
    else
    {
      PORTD&=~(1<<PD2); //turn fan off
      strcpy(serial_output,s1); //copying string s1 to serial_output
    }
    
    itoa(temp, temp_string, 10); //convert temperature to string
    strcat(serial_output, temp_string); //concatenating strings
    strcat(serial_output, s3); //concatenating strings
    Serial.println(serial_output); //display output on virtual terminal
    strcpy(serial_output,""); //resetting output string to empty, for next iteration of the loop
  }
  
  return 0;
}
