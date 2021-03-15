/*
AUTHOR: Micah Black, A2D Electronics
DATE: Mar 11, 2021
PURPOSE: This example implements some SCPI commands
        (that don't completely follow the susbystem style standard)
        to control the Arduino's IO pins
CHANGELOG:
	Mar 11, 2021 - First commit
*/

#include <A2D_DAQ.h>

#define MANUFACTURER  ("A2D Electronics")
#define DESCRIPTION ("Arduino IO Module")
#define VERSION     ("V1.0.0")

#define LED_PIN 13

//SERIAL DEFINES
#define BAUDRATE    57600
#define SER_BUF_LEN   32
#define END_CHAR    '\n'
#define NO_CMD      ""

//Macro for finding commands - F to store string literal
//in flash instead of memory
#define CMDIS(i,c) (!strcmp_P(i, PSTR(c)))

//Function Prototypes:
void parse_serial(char ser_buf[], char command[], uint8_t* channel_num, uint8_t* value_int);
void(* resetFunc) (void) = 0;//declare reset function at address 0

void setup() {
  // put your setup code here, to run once:
  Serial.begin(BAUDRATE);

  //set all pins as inputs by default
  for (int i=2; i<14; i++){
    pinMode(i, INPUT);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  //Allocate memory for the serial buffer
  char ser_buf[SER_BUF_LEN];
  char command[SER_BUF_LEN];
  uint8_t chars_input = 0;
  uint8_t channel_num = 0;
  uint8_t value_int = 0;
  
  //if serial data is available
  if(Serial.available()){
    //Read until a full command is received
    chars_input = Serial.readBytesUntil(END_CHAR, ser_buf, SER_BUF_LEN);
    //terminate the input string with NULL
    ser_buf[chars_input] = '\0'; 
    //if(chars_input == 0);//TODO - set a default command to read
    //}
	  parse_serial(ser_buf, command, &channel_num, &value_int);
  }
  else{
    strcpy(command, "NOCMD");
  }


  //NOCMD?
  if(CMDIS(command, "NOCMD")){
    ;
  }
  
  //*IDN?
  else if(CMDIS(command, "*IDN?")){
    Serial.print(MANUFACTURER);
    Serial.print(" ");
    Serial.print(DESCRIPTION);
    Serial.print(" ");
    Serial.println(VERSION);
    Serial.flush();
  }
  
  //*RST
  else if (CMDIS(command, "*RST")){
    resetFunc();
  }
  
  //*CLS
  else if (CMDIS(command, "*CLS")){
    ; //nothing since we don't have errors yet
  }
  
  //CONF:IO:OUTP (@ch) //just work with 1 ch for now
  else if (CMDIS(command, "CONF:IO:OUTP")){
    pinMode(channel_num, OUTPUT);
  }
  
  //CONF:IO:INP (@ch) //configure all channels as input
  else if (CMDIS(command, "CONF:IO:INP")){
    pinMode(channel_num, INPUT);
  }
  
  //INSTR:IO:SET:DIG:OUTP (@ch),VAL //VAL is boolean 0 or 1
  else if (CMDIS(command, "INSTR:IO:SET:DIG:OUTP")){
    digitalWrite(channel_num, value_int);
  }

  //INSTR:IO:SET:PWM:OUTP (@ch),VAL //VAL is 0-255
  else if (CMDIS(command, "INSTR:IO:SET:PWM:OUTP")){
    analogWrite(channel_num, value_int);
  }
  
  //INSTR:IO:READ:ANA? (@ch) //read ADC ch_list and return
  else if (CMDIS(command, "INSTR:IO:READ:ANA?")){
    Serial.println(analogRead(channel_num));
    Serial.flush();
  }
  
  //INSTR:IO:READ:DIG? (@ch) //read digital input register and return
  else if (CMDIS(command, "INSTR:IO:READ:DIG?")){
    Serial.println(digitalRead(channel_num));
    Serial.flush();
  }
  
  //INSTR:IO:PULSE (@ch),val //val is boolean 0 or 1
  else if (CMDIS(command, "INSTR:IO:PULSE")){
    digitalWrite(channel_num, !value_int);//make sure pin is opposite to start
	  digitalWrite(channel_num, value_int);//pulse pin
	  digitalWrite(channel_num, !value_int);//return to start val
  }
  
  //INSTR:IO:SET:LED x VAL  //VAL is boolean 0 or 1
  else if (CMDIS(command, "INSTR:IO:SET:LED")){
    digitalWrite(LED_PIN, value_int);
  }
}

void parse_serial(char ser_buf[], char command[], uint8_t *channel_num, uint8_t *value_int){
  //All SCPI commands are terminated with newline '/n'
  //but the Serial.readBytesUntil discards the terminator
  //so do we need to add one to use strcmp?
  
  //we will assume only 1 command is sent at a time
  //so we don't have to deal with SCPI's ';' to send
  //multiple commands on the same line
  
  //split input string on space to extract the command
  //and the parameters
  //strtok replaces the delimeter with NULL to terminate
  //the string
  char delimeters[] = " ,";
  char* token;
  char channel_str[8];
  char value_str[8];
  
  token = strtok(ser_buf, delimeters);
  strcpy(command, token);
  token = strtok(NULL, delimeters);
  strcpy(channel_str, token);
  token = strtok(NULL, delimeters);
  strcpy(value_str, token);

  token = strtok(value_str, delimeters);
  *value_int = atoi(token);
  if(*value_int < 0)
    *value_int = 0;
  
  strcpy(delimeters, "@)");
  token = strtok(channel_str, delimeters); //get rid of "(@"
  token = strtok(NULL, delimeters);
  *channel_num = atoi(token);
  if(*channel_num > 63)
    *channel_num = 63;
  else if(*channel_num < 0)
    *channel_num = 0;
}
