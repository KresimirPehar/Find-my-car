#include<SoftwareSerial.h>

extern uint8_t SmallFont[];

#define rxPin 7
#define txPin 8

SoftwareSerial mySerial(rxPin, txPin);
char response[200];
char latitude[15];
char longitude[15];

void setup(){
    mySerial.begin(9600);
    Serial.begin(9600); 

    Serial.println("Starting...");
    power_on();
    while (sendATcommand("AT+CREG?", "+CREG: 0,1", 2000) == 0);
    //SMS TEXT MODE - ON
    sendATcommand("AT+CMGF=1", "OK", 2000);
}

void loop(){
    on_demand();
    
    start_GPS();
    
    get_GPS();
    
    send_coordinates();         
}

//WAIT FOR THE USER DEMAND
void on_demand(){
  uint8_t answer= 0;
answer=sendATcommand("AT+CMGL=\"ALL\"", "+CMGL:", 5000);

  
    if (answer==0)
    {
        on_demand();  
    }
    
}

//GPS - ON
int8_t start_GPS(){
  
    while(sendATcommand("AT+CGPSPWR=1", "OK", 2000)==0);
    while(sendATcommand("AT+CGPSRST=0", "OK", 2000)==0);

    // WAIT FOR THE SIGNAL
    while(( (sendATcommand("AT+CGPSSTATUS?", "2D Fix", 5000) || 
        sendATcommand("AT+CGPSSTATUS?", "3D Fix", 5000)) == 0 ) );

    return 1;
}

    // GET GPS DATA
int8_t get_GPS(){
    
    int8_t answer;
    char * auxChar;
    
    sendATcommand("AT+CGPSINF=0", "O", 8000);
 
    auxChar = strstr(response, "+CGPSINF:");
    if (auxChar != NULL)    
    {
         
      memset(longitude, '\0', 15);
      memset(latitude, '\0', 15);
      strcpy (response, auxChar);
      Serial.println(response);
      
      strtok(response, ",");
      strcpy(longitude,strtok(NULL, ",")); // GET LONGITUDE
      strcpy(latitude,strtok(NULL, ",")); // GET LATITUDE
     
      answer = 1;
    }
    else
      answer = 0;

    return answer;
}

 // SEND COORDINATES ON MOBILE PHONE
void send_coordinates(){
     
    char frame[200];
            memset(frame, '\0', 200);
            sprintf(frame, "https://www.google.hr/maps/place//@%s,%s,12z/data=!3m1!4b1!4m5!3m4!1s0x0:0x0!8m2!3d%s!4d%s", longitude, latitude, longitude, latitude);
          
            sendNMEALocation("+38763408699",frame); //put your number here!
}

void sendNMEALocation(char * cellPhoneNumber, char * message) 
{ 
    char ctrlZString[2];  
    char sendSMSString[100];    
    sendATcommand("AT+CMGF=1", "OK", 2000);
    
    memset(ctrlZString, '\0', 2);
    ctrlZString[0] = 26;  
    
    memset(sendSMSString, '\0', 100); 
    sprintf(sendSMSString,"AT+CMGS=\"%s\"",cellPhoneNumber);            
     
    
    sendATcommand(sendSMSString, ">", 2000);
    mySerial.println(message);
    sendATcommand(ctrlZString, "OK", 6000); 
    //DELETE USER MESSAGE
    sendATcommand("AT+CMGDA=\"DEL ALL\"","OK",5000);
    //TURN OFF THE DEVICE
    sendATcommand("AT+CPOWD=1", "OK", 2000);
} 

    //CHECK OUT IF THE MODULE IS ON
void power_on(){

    uint8_t answer=0;

    answer = sendATcommand("AT", "OK", 2000);
    if (answer == 0)
    {
        // CHECK FOR THE ANSWER
        while(answer == 0){  
            //SEND "AT" EVERY 2 SECONDS AND WAIT FOR THE ANSWER
            answer = sendATcommand("AT", "OK", 2000);    
        }
    }

}

int8_t sendATcommand(char* ATcommand, char* expected_answer1, unsigned int timeout){

    uint8_t x=0,  answer=0;
    unsigned long previous;
    char readVar[200];
    char * auxChar;
    
    memset(response, '\0', 200);    
    memset(readVar, '\0', 200);    

    while( mySerial.available() > 0) mySerial.read();   
    while( Serial.available() > 0) Serial.read();    
 
    mySerial.write(ATcommand);     
    mySerial.write("\r\n\r\n");    
    
    Serial.println(ATcommand);
    
 
    x = 0;
    previous = millis();

    
    do{
        if(mySerial.available() != 0){    
            readVar[x] = mySerial.read();
            x++;
           
            auxChar = strstr(readVar, expected_answer1);
            if (auxChar != NULL)    
            {
                if( strstr(readVar, "+CGPSINF:") == NULL)
                  strcpy (response, auxChar);
                else
                  strcpy (response, readVar);
    
                answer = 1;
            }
        }
       
    }
    while((answer == 0) && ((millis() - previous) < timeout));  

    if(auxChar == NULL)
      Serial.println(readVar);
    
    return answer;
}

