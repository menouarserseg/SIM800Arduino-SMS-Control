#include <gprs.h>
#include <SoftwareSerial.h>
 
#define TIMEOUT    2000
const int LED_PIN = 3; 
 
bool ledStatus;
GPRS gprs;
 
void setup() {
  pinMode(LED_PIN, OUTPUT); 
  digitalWrite(LED_PIN, HIGH); 
  Serial.begin(9600);
  while(!Serial);
  
  Serial.println("Starting SIM800 SMS Command Processor");
  gprs.preInit();
  delay(1000);

  while(0 != gprs.init()) {
      delay(1000);
      Serial.print("init error\r\n");
  } 
 
  //Set SMS mode to ASCII
  if(0 != gprs.sendCmdAndWaitForResp("AT+CMGF=1\r\n", "OK", TIMEOUT)) {
    ERROR("ERROR:CNMI");
    return;
  }
   
  //Start listening to New SMS Message Indications
  if(0 != gprs.sendCmdAndWaitForResp("AT+CNMI=1,2,0,0,0\r\n", "OK", TIMEOUT)) {
    ERROR("ERROR:CNMI");
    return;
  }
 
  Serial.println("Init success");
}
 
//Variable to hold last line of serial output from SIM800
char currentLine[500] = "";
int currentLineIndex = 0;
 
//Boolean to be set to true if message notificaion was found and next
//line of serial output is the actual SMS message content
bool nextLineIsMessage = false;
 
void loop() {
   
  //If there is serial output from SIM800
  if(gprs.serialSIM800.available()){
    char lastCharRead = gprs.serialSIM800.read();
    //Read each character from serial output until \r or \n is reached (which denotes end of line)
    if(lastCharRead == '\r' || lastCharRead == '\n'){
        String lastLine = String(currentLine);
         
        //If last line read +CMT, New SMS Message Indications was received.
        //Hence, next line is the message content.
        if(lastLine.startsWith("+CMT:")){
           
          Serial.println(lastLine);
          nextLineIsMessage = true;
        } else if (lastLine.length() > 0) {
          if(nextLineIsMessage) {
            Serial.println(lastLine);
             
            //Read message content and set status according to SMS content
            if(lastLine.indexOf("ON 123") >= 0){
              digitalWrite(LED_PIN, LOW);
            } else if(lastLine.indexOf("OFF 123") >= 0) {
              digitalWrite(LED_PIN, HIGH);
            }
             
            nextLineIsMessage = false;
          }
           
        }
         
        //Clear char array for next line of read
        for( int i = 0; i < sizeof(currentLine);  ++i ) {
         currentLine[i] = (char)0;
        }
        currentLineIndex = 0;
    } else {
      currentLine[currentLineIndex++] = lastCharRead;
    }
  } else {
gprs.sendCmdAndWaitForResp("AT+CMGF=1\r\n", "OK", TIMEOUT);
gprs.sendCmdAndWaitForResp("AT+CNMI=1,2,0,0,0\r\n", "OK", TIMEOUT);
  }
}
