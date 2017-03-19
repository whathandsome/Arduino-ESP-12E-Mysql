#include <SoftwareSerial.h>

char server[] = "192.168.1.20";   // server address

unsigned long lastConnectionTime = 0;          // last time you connected to the server, in milliseconds
boolean lastConnected = false;                 // state of the connection last time through the main loop
const unsigned long postingInterval = 30*1000; // delay between 2 datapoints, 30s

String WiFiSSID = "XXXXXX";//WiFiSSID
String WiFiPASSWORD = "XXXXXXXX";//WiFiPASSWORD
int chlID;

float Vol,Amp,Tkw,Nkw;
byte V[22]={0xFE,0xFE,0xFE,0xFE,0x68,0x67,0x40,0x05,0x31,0x37,0x00,0x68,0x11,0x04,0x33,0x34,0x34,0x35,0xC9,0x16,0x13,0x10};
byte A[22]={0xFE,0xFE,0xFE,0xFE,0x68,0x67,0x40,0x05,0x31,0x37,0x00,0x68,0x11,0x04,0x33,0x34,0x35,0x35,0xCA,0x16,0x13,0x10};
byte T[22]={0xFE,0xFE,0xFE,0xFE,0x68,0x67,0x40,0x05,0x31,0x37,0x00,0x68,0x11,0x04,0x33,0x33,0x33,0x33,0xC5,0x16,0x13,0x10};
byte N[22]={0xFE,0xFE,0xFE,0xFE,0x68,0x67,0x40,0x05,0x31,0x37,0x00,0x68,0x11,0x04,0x33,0x33,0x36,0x35,0xCA,0x16,0x13,0x10};

SoftwareSerial mySerial(10, 11); //software serial(RX, TX)
void setup() 
{
  begin1:
  delay(15000);
  Serial.begin(9600);
  Serial1.begin(2400,SERIAL_8E1); 
  Serial.println("DDZY ID:00 37 31 05 40 67");
  
  ESPbegin();
  bool b = Initialize(WiFiSSID,WiFiPASSWORD);
  if(!b)
  {
    Serial.println();
    Serial.println("Initialize Error");
    goto begin1;
  }
  delay(10000);  //make sure the module can have enough time to get an IP address 
  String ipstring  = showIP();
  Serial.println();
  Serial.println(ipstring);                //show the ip address of module
}

void loop() 
{
  delay(5000);
  char message[400];
  if((millis() - lastConnectionTime > postingInterval))
   {
     Vol=Comm(V,22,'1');
     Amp=Comm(A,23,'2');
     Tkw=Comm(T,24,'3');
     Nkw=Comm(N,23,'4');
     sendData();
   }
 
  if(ReceiveMessage(message)) 
   {
      Serial.println();
      Serial.println(message);
   }
  delay(10);
}

/*000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000*/
float Comm(byte* cmd,unsigned int lrsp,int value)
{
  String response="";
  bool found = false;
  char c;
  long timer_init;
  long timer;
  float Cal_L,Cal_M,Cal_H,Cal_HH,Cal;
  Cal=0;
  Serial1.write(&cmd[0],22);
  delay(100);  
  timer_init = millis();
  while (!found) 
  {
    timer = millis();
    if (((timer - timer_init) / 1000) > 1.5) 
    { 
      break;
      return 0;
     }
    if (Serial1.available()) 
     {
      c =  Serial1.read();
      response += c;
      if (response.length()>=lrsp) 
      {
        found = true;
        delay(10);
        Serial1.flush();
        if (value=='1')
          {
            Cal_L=dec2hex(byte(response[18])-51)/10.0;
            Cal_M=dec2hex(byte(response[19])-51)*10.0;
            Cal=Cal_L+Cal_M;
          }
        if (value=='2')
          {
            Cal_L=dec2hex(byte(response[18])-51)/1000.0;
            Cal_M=dec2hex(byte(response[19])-51)/10.0;
            Cal_H=dec2hex(byte(response[20])-51)*10.0;
            Cal=Cal_L+Cal_M+Cal_H;
          }
        if (value=='3')
          {
            Cal_L=dec2hex(byte(response[18])-51)/100.0;
            Cal_M=dec2hex(byte(response[19])-51)/1.0;
            Cal_H=dec2hex(byte(response[20])-51)*100.0;
            Cal_HH=dec2hex(byte(response[21])-51)*10000.0;
            Cal=Cal_L+Cal_M+Cal_H+Cal_HH;
          }
        if (value=='4')
          {
            Cal_L=dec2hex(byte(response[18])-51)/10000.0000;
            Cal_M=dec2hex(byte(response[19])-51)/100.00;
            Cal_H=dec2hex(byte(response[20])-51)*1.0;
            Cal=Cal_L+Cal_M+Cal_H;
          }
      }
    }
  }
  return Cal;
}
/*000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000*/
byte dec2hex(byte buf)
{
byte dec;
int a=0;
int b=0;
int c=0;
a=buf%16;
b=int(buf/16)*10;
c=a+b;
return c;
}

/*111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111111*/
void ESPbegin()
{
  boolean result = false;
  mySerial.begin(9600);  //The default baud rate of ESP8266 is 115200
  mySerial.flush();
  //mySerial.setTimeout(10000);
  mySerial.print("+++");
  Serial.print("+++");
  Serial.println("Initalizing ESP Wifi Module...");
  Serial.println("WIFI RESET");
  mySerial.flush();
  ESPsendCommand("AT","OK",1);
  Serial.println();
  ESPsendCommand("AT+GMR", "OK", 5); // ESP Wifi module RESET
  Serial.println();
  ESPsendCommand("AT+RST", "OK", 3); // ESP Wifi module RESET
  Serial.println();
  result =ESPwait("ready", 6);
  if(result)
    {
      Serial.println();
      Serial.println("Module is ready");
    }
   else
   {
     Serial.println();
     Serial.println("Module have no response");
     Serial.println("Check Module And Restart!");
     while(1);
   }
}

/*222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222222*/
int ESPwait(String stopstr, int timeout_secs)
{
  String response;
  bool found = false;
  char c;
  long timer_init;
  long timer;
  timer_init = millis();
  while (!found) {
    timer = millis();
    if (((timer - timer_init) / 1000) > timeout_secs) 
    { 
      Serial.println("!Timeout!");
      return 0;
    }
    if (mySerial.available()) {
      c = mySerial.read();
      Serial.print(c);
      response += c;
      if (response.endsWith(stopstr)) {
        found = true;
        delay(10);
        mySerial.flush();
      }
    } 
  } 
  return 1;
}

/*333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333333*/
int ESPsendCommand(char *command, String stopstr, int timeout_secs)
{
  int result=0;
  mySerial.println(command);
  result=ESPwait(stopstr, timeout_secs);
  delay(250);
  return result;
}


/*444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444444*/
bool Initialize(String ssid, String pwd)
{
    bool b = confMode();
    if (!b)
     {
      return false;
     }
    Serial.println();
    Serial.println("Quit Counter AP");
    b = ESPsendCommand("AT+CWQAP", "OK", 3);
    if (!b)
     {
      return false;
     }
    Serial.println();
    delay(5000);
    Serial.println("Connect to router!");
    b = confJAP(ssid, pwd);
    if (!b)
     {
      return false;
     }
 return true;
}

/*555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555*/
bool confMode()
{
    String data;
    mySerial.print("AT+CWMODE=");  
    mySerial.println("1");
    unsigned long start1;
    start1 = millis();
    while (millis()-start1<2000) 
    {
      if(mySerial.available()>0)
      {
      char a =mySerial.read();
      data=data+a;
      }
      //Serial.print(data);
      if (data.indexOf("OK")!=-1 || data.indexOf("no change")!=-1)
      {
         return true;
      }
     if (data.indexOf("ERROR")!=-1 || data.indexOf("busy")!=-1)
      {
        return false;
      }
   }
}

/*666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666666*/

boolean confJAP(String ssid , String pwd)
{
    int result=0;
    mySerial.print("AT+CWJAP=");
    mySerial.print("\"");     //"ssid"
    mySerial.print(ssid);
    mySerial.print("\"");
    mySerial.print(",");
    mySerial.print("\"");      //"pwd"
    mySerial.print(pwd);
    mySerial.println("\"");
    result =ESPwait("OK", 30);
    return result;
}

/*777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777777*/

String showIP(void)
{
  String data;
  unsigned long start1;
  for(int a=0; a<3;a++)
   {
     mySerial.println("AT+CIFSR");  
     start1 = millis();
     while (millis()-start1<10000) 
       {
         while(mySerial.available()>0)
          {
           char a =mySerial.read();
           data=data+a;
          }
        if (data.indexOf("AT+CIFSR")!=-1)
         {
           break;
         }
       }
     if(data.indexOf(".") != -1)
       {
        break;
       }
     data = "";
  }
    char head[4] = {0x0D,0x0A};   
    char tail[7] = {0x0D,0x0D,0x0A};        
    data.replace("AT+CIFSR","");
    data.replace(tail,"");
    data.replace(head,"");
    return data;
}

/*888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888*/
void sendData()
{
  if (ipConfig('TCP',server,80)) 
  {
    Serial.println();
    Serial.println("connecting...");
    String val;
    String cmd;
    val = "VALUE=";
    val += String(Vol);
    val +="','";
    val += String(Amp);
    val +="','";
    val += String(Tkw);
    val +="','";
    val += String(Nkw);
    
    cmd = "POST /add.php? HTTP/1.1\r\n";
    cmd += "Host:192.168.1.20\r\n";
    cmd += "Content-Type: application/x-www-form-urlencoded\r\n";
    cmd += "Content-Length: ";
    int thisLength = val.length();
    cmd += String(thisLength);
    cmd += "\r\n";
    cmd += "Connection: close\r\n";
    cmd += "\r\n";
    cmd += val;
    Serial.println(cmd);
    Send(cmd);
    lastConnectionTime = millis();
  } 
  else 
  {
    Serial.println();
    Serial.println("Connection Failed");
    Serial.println("Disconnecting.");
    closeMux();
  }
}

/*999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999999*/
boolean ipConfig(byte type, String addr, int port)
{
  boolean result = false;
    confMux(0);
    delay(5000);
    result = newMux(type, addr, port);
  return result;
}

/*AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA*/
boolean confMux(boolean a)
{
  int result=0;
  mySerial.print("AT+CIPMUX=");
  mySerial.println(a);
  result=ESPwait("OK",3);
  return result;
}

/*BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB*/
boolean newMux(byte type, String addr, int port)
{
  String data;
  mySerial.print("AT+CIPSTART=");
  if(type>0)
    {
      mySerial.print("\"TCP\"");
    }
  else
    {
      mySerial.print("\"UDP\"");
    }
  mySerial.print(",");
  mySerial.print("\"");
  mySerial.print(addr);
  mySerial.print("\"");
  mySerial.print(",");
  mySerial.println(String(port));
  unsigned long start1;
  start1 = millis();
  while (millis()-start1< 3000)
   { 
     if(mySerial.available()>0)
       {
         char a =mySerial.read();
         data=data+a;
       }
       Serial.print("newMux:");
       Serial.println(data);
     if (data.indexOf("OK")!=-1 || data.indexOf("ALREAY CONNECT")!=-1 || data.indexOf("ERROR")!=-1)
       {
         return true;
       }
   }
  return false;
}

/*CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC*/
int getLength(float someValue)
{ 
  int digits = 1;
  long dividend = someValue /10;
  while (dividend > 0) 
   {
    dividend = dividend /10;
    digits++;
   }
   digits=digits+3;
  return digits;
}

/*DDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDDD*/
boolean Send(String str)
{
  mySerial.print("AT+CIPSEND=");
  mySerial.println(str.length());
  unsigned long start1;
  start1 = millis();
  bool found;
  while (millis()-start1<5000) 
    {                            
     if(mySerial.find(">")==true )
        {
           found = true;
           break;
        }
     }
   if(found)
    {
      mySerial.print(str);
    }
  else
    {
      closeMux();
      return false;
    }
  String data;
  start1 = millis();
  while (millis()-start1<5000) 
  {
    if(mySerial.available()>0)
      {
        char a =mySerial.read();
        data=data+a;
      }
    if (data.indexOf("SEND OK")!=-1)
      {
         return true;
      }
   }
  return false;
}

/*EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE*/
void closeMux(void)
{
  mySerial.println("AT+CIPCLOSE");
  String data;
  unsigned long start1;
  start1 = millis();
  while (millis()-start1<3000) 
  {
    if(mySerial.available()>0)
     {
       char a =mySerial.read();
       data=data+a;
     }
    if (data.indexOf("Linked")!=-1 || data.indexOf("ERROR")!=-1 || data.indexOf("we must restart")!=-1)
     {
       break;
     }
  }
}

/*FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF*/
int ReceiveMessage(char *buf)
{
  String data = "";
  if (mySerial.available()>0)
  {
    unsigned long start;
    start = millis();
    char c0 = mySerial.read();
    if (c0 == '+')
    {
      while (millis()-start<5000) 
      {
        if (mySerial.available()>0)
        {
          char c = mySerial.read();
          data += c;
        }
        if (data.indexOf("\nOK")!=-1)
        {
          break;
        }
      }
      int sLen = strlen(data.c_str());
      int i,j;
      for (i = 0; i <= sLen; i++)
      {
        if (data[i] == ':')
        {
          break;
        }
        
      }
      boolean found = false;
      for (j = 4; j <= i; j++)
      {
        if (data[j] == ',')
        {
          found = true;
          break;
        }
      }
      int iSize;
      if(found ==true)
      {
      String _id = data.substring(4, j);
      chlID = _id.toInt();
      String _size = data.substring(j+1, i);
      iSize = _size.toInt();
      String str = data.substring(i+1, i+1+iSize);
      strcpy(buf, str.c_str());          
      }
      else
      {     
      String _size = data.substring(4, i);
      iSize = _size.toInt();
      String str = data.substring(i+1, i+1+iSize);
      strcpy(buf, str.c_str());
      }
      return iSize;
    }
  }
  return 0;
}


//////////////////////////////////////////////////////////////////////////END////////////////////////////////////////////////////////////////////////////////

