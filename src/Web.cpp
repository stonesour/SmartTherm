/* Web.cpp  UTF-8  */

#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
using WiFiWebServer = ESP8266WebServer;
#define FORMAT_ON_FAIL
#elif defined(ARDUINO_ARCH_ESP32)
#include <WiFi.h>
#include <WebServer.h>
using WiFiWebServer = WebServer;
#define FORMAT_ON_FAIL  true
#endif

#include <time.h>
#include "OpenTherm.h"
#include "SmartDevice.hpp"
#include "SD_OpenTherm.hpp"

/*
  Include AutoConnectFS.h allows the sketch to retrieve the current file
  system that AutoConnect has selected. It derives a constant
  AUTOCONNECT_APPLIED_FILESYSTEM according to the definition state of
  AC_USE_SPIFFS or AC_USE_LITTLEFS in AutoConnectDefs.h.
  Also, the AutoConnectFS::FS class indicates either SPIFFS or LittleFS
  and will select the appropriate filesystem class depending on the file
  system applied to the sketch by the definition AC_USE_SPIFFS or
  AC_USE_LITTLEFS in AutoConnectDefs.h.
  You no need to change the sketch due to the file system change, declare
  the Filesystem object according to the following usage:
  
  #include <AutoConnectFS.h>
  AutoConnectFS::FS& name = AUTOCONNECT_APPLIED_FILESYSTEM;
  name.begin(AUTOCONECT_FS_INITIALIZATION);
*/

#include <AutoConnect.h>
#include <AutoConnectFS.h>
AutoConnectFS::FS& FlashFS = AUTOCONNECT_APPLIED_FILESYSTEM;

#if defined(ARDUINO_ARCH_ESP8266)
char SmartDevice::BiosDate[12]=__DATE__;   /* дата компиляции биоса */

#endif

extern  SD_Termo SmOT;
int WiFiDebugInfo[10] ={0,0,0,0,0, 0,0,0,0,0};
int OTDebugInfo[10] ={0,0,0,0,0, 0,0,0,0,0};
extern OpenThermID OT_ids[N_OT_NIDS];


/*********************************/
const char* INFO_URI = "/info";
const char* SETUP_URI = "/setup";
const char* ABOUT_URI = "/about";
const char* SET_T_URI =  "/set_t";
const char* SET_PAR_URI =  "/set_par";
const char* DEBUG_URI = "/debug";


/************* InfoPage ******************/
ACText(Caption, "<b>Статус OT: </b>", "", "", AC_Tag_DIV);
ACText(Info1, "it1", "", "", AC_Tag_DIV);
ACText(Info2, "it2", "", "", AC_Tag_DIV);
ACText(Info3, "it3", "", "", AC_Tag_DIV);
ACText(Info4, "it4", "", "", AC_Tag_DIV);
ACText(Info5, "it5", "", "", AC_Tag_DIV);
ACText(Info6, "it6", "", "", AC_Tag_DIV);
ACText(Info7, "it7", "", "", AC_Tag_DIV);
ACInput(SetBoilerTemp,"44", "Температура теплоносителя:<br>"); // Boiler Control setpoint
ACInput(SetDHWTemp,   "43", "Температура горячей воды:<br>");  // DHW Control setpoint
ACInput(SetBoilerTemp2,"44", "Температура CH2:<br>"); // Boiler CH2 Control setpoint

ACSubmit(Apply, "Обновить", INFO_URI, AC_Tag_DIV);
ACSubmit(SetNewBoilerTemp,"Задать", SET_T_URI, AC_Tag_DIV);

/************* SetupPage ***************/
ACText(Ctrl1, "Настройки котла:", "", "", AC_Tag_DIV);
ACText(Ctrl2, "Настройки 2", "", "", AC_Tag_DIV);
//ACCheckbox(CtrlChB1,"checkbox", "uniqueapid");
AutoConnectCheckbox CtrlChB1("CtrlChB1","1", "Отопление", false, AC_Behind , AC_Tag_BR);
AutoConnectCheckbox CtrlChB2("CtrlChB2","2", "Горячая вода", false, AC_Behind , AC_Tag_DIV);
AutoConnectCheckbox CtrlChB3("CtrlChB3","3", "Отопление 2", false, AC_Behind , AC_Tag_DIV);
  
//AutoConnectCheckbox checkbox("checkbox", "uniqueapid", "Use APID unique", false);
//ACCheckbox(CtrlChB2,"a2", "", true,  AC_Behind , AC_Tag_DIV);
ACSubmit(ApplyChB, "Задать", SET_PAR_URI, AC_Tag_DIV);

/************* SetTempPage ***************/
ACText(SetTemp_info1, "", "", "", AC_Tag_DIV); //Заданная температура:
ACSubmit(SetTemp_OK, "Ok", INFO_URI, AC_Tag_DIV);
/************* SetParPage ***************/
ACText(SetPar_info1, "", "", "", AC_Tag_DIV); //CH DHW CH2:
//ACText(SetPar_info2, " ", "", "", AC_Tag_DIV);
ACSubmit(SetPar_OK, "Ok", INFO_URI, AC_Tag_DIV);

/************* debugPage( ****************/
ACText(DebugInfo1, "D1", "", "", AC_Tag_DIV);
ACText(DebugInfo2, "D2", "", "", AC_Tag_DIV);
ACText(DebugInfo3, "D3", "", "", AC_Tag_DIV);
ACText(DebugInfo4, "D4", "", "", AC_Tag_DIV);
ACText(DebugInfo5, "D5", "", "", AC_Tag_DIV);
ACText(DebugInfo6, "D6", "", "", AC_Tag_DIV);
ACSubmit(DebugApply, "Обновить", DEBUG_URI, AC_Tag_DIV);
/************* AboutPage *****************/
ACText(About_0, "<b>About:</b>", "", "", AC_Tag_DIV);
ACText(About_1, "1", "", "", AC_Tag_DIV);
ACText(About_2, "2", "", "", AC_Tag_DIV);
ACText(About_3, "3", "", "", AC_Tag_DIV);
/*****************************************/

// AutoConnectAux for the custom Web page.
AutoConnectAux InfoPage(INFO_URI, "SmartTherm", true, { Caption, Info1, Info2, Info3, Info4, Info5, Info6, Info7,  Apply, SetBoilerTemp, SetDHWTemp, SetBoilerTemp2, SetNewBoilerTemp });
AutoConnectAux Setup_Page(SETUP_URI, "Setup", true, { Ctrl1, CtrlChB1, CtrlChB2, CtrlChB3, Ctrl2, ApplyChB});
AutoConnectAux SetTempPage(SET_T_URI, "SetTemp", false, { SetTemp_info1,  SetTemp_OK});
AutoConnectAux SetParPage(SET_PAR_URI, "SetPar", false, { SetPar_info1,   SetPar_OK});

AutoConnectAux debugPage(DEBUG_URI, "Debug", true, { DebugInfo1, DebugInfo2, DebugInfo3, DebugInfo4, DebugInfo5, DebugInfo6,  DebugApply});
AutoConnectAux AboutPage(ABOUT_URI, "About", true, { About_0, About_1, About_2, About_3});

AutoConnectConfig config;
AutoConnect portal;


/************************************/
//int test_fs(void);

void setup_web_common(void);
void loop_web(void);
void onRoot(void);
void loadParam(String fileName);
void onConnect(IPAddress& ipaddr);

String onInfo(AutoConnectAux& aux, PageArgument& args);
String on_Setup(AutoConnectAux& aux, PageArgument& args);
String onSetTemp(AutoConnectAux& aux, PageArgument& args);
String onSetPar(AutoConnectAux& aux, PageArgument& args);
String onDebug(AutoConnectAux& aux, PageArgument& args);
String onS(AutoConnectAux& aux, PageArgument& args);
String onAbout(AutoConnectAux& aux, PageArgument& args);

/************************************/
unsigned int /* AutoConnect:: */ _toWiFiQuality(int32_t rssi);

/************************************/

void setup_web_common(void)
{
//  Serial.println();
   Serial.println("setup_web_common");
  FlashFS.begin(AUTOCONNECT_FS_INITIALIZATION);
//  FlashFS.begin(FORMAT_ON_FAIL); //AUTOCONNECT_FS_INITIALIZATION);


#if defined(ARDUINO_ARCH_ESP8266)
   Serial.printf("OT_ids[0].used =%d\n", OT_ids[0].used);
#elif defined(ARDUINO_ARCH_ESP32)
   Serial.printf("OT_ids[0].used =%d %s\n", OT_ids[0].used,  OT_ids[0].descript);
#endif


    { int tBytes, uBytes; 
#if defined(ARDUINO_ARCH_ESP8266)
      FSInfo info;
      FlashFS.info(info);
      tBytes  = info.totalBytes;
      uBytes = info.usedBytes;
#else
      tBytes  = FlashFS.totalBytes();
      uBytes = FlashFS.usedBytes();
#endif      
      Serial.printf("FlashFS tBytes = %d used = %d\n", tBytes, uBytes);
    }

  SmOT.Read_ot_fs();
   
  {  char str[40];
     sprintf(str,"%.1f",SmOT.Tset);
     SetBoilerTemp.value = str;
     sprintf(str,"%.1f",SmOT.TdhwSet);
     SetDHWTemp.value = str;
  }

  InfoPage.on(onInfo);      // Register the attribute overwrite handler.
  Setup_Page.on(on_Setup);
  SetTempPage.on(onSetTemp);
  SetParPage.on(onSetPar);
  debugPage.on(onDebug);
  AboutPage.on(onAbout);
/**/  
  portal.join({InfoPage, Setup_Page, SetTempPage, SetParPage, debugPage,  AboutPage});     // Join pages.
//  portal.join({InfoPage, Setup_Page, SetTempPage});     // Join pages.
  config.ota = AC_OTA_BUILTIN;
  config.portalTimeout = 1; 
  config.retainPortal = true; 
  config.autoRise = true;
  // Enable saved past credential by autoReconnect option,
  // even once it is disconnected.
//  config.autoReconnect = true;
//  config.reconnectInterval = 1;

#if SERIAL_DEBUG 
  Serial.println("1");
#endif  
  portal.config(config);
  portal.onConnect(onConnect);  // Register the ConnectExit function
#if SERIAL_DEBUG 
  Serial.println("2");
#endif  
  portal.begin();
#if SERIAL_DEBUG 
  Serial.println("3");
#endif  

  WiFiWebServer&  webServer = portal.host();

  webServer.on("/", onRoot);  // Register the root page redirector.
//  Serial.println("Web server started:" +WiFi.localIP().toString());
  if (WiFi.status() != WL_CONNECTED)  {
    Serial.println("WiFi Not connected");
  }
  else {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.println("WiFiOn");
    WiFi.setAutoReconnect(true);
  }  

/* get my MAC*/
#if defined(ARDUINO_ARCH_ESP8266)
//    WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3
    if(WiFi.getMode() == WIFI_OFF)
    {
      wifi_get_macaddr(STATION_IF, SmOT.Mac);

    } else {
      wifi_get_macaddr(STATION_IF, SmOT.Mac);
    }
    Serial.printf("MAC: %02x %02x %02x %02x %02x %02x\n",SmOT.Mac[0],SmOT.Mac[1],SmOT.Mac[2],SmOT.Mac[3],SmOT.Mac[4],SmOT.Mac[5]);
#elif defined(ARDUINO_ARCH_ESP32)
    if(WiFi.getMode() == WIFI_MODE_NULL){
        esp_read_mac(SmOT.Mac, ESP_MAC_WIFI_STA);
    }
    else{
        esp_wifi_get_mac(WIFI_IF_STA, SmOT.Mac);
    }  
#endif //

}

void onConnect(IPAddress& ipaddr) {
  Serial.print("onConnect:WiFi connected with ");
  Serial.print(WiFi.SSID());
  Serial.print(", IP:");
  Serial.println(ipaddr.toString());
}

// Redirects from root to the info page.
void onRoot() {
  WiFiWebServer&  webServer = portal.host();
  webServer.sendHeader("Location", String("http://") + webServer.client().localIP().toString() + String(INFO_URI));
  webServer.send(302, "text/plain", "");
  webServer.client().flush();
  webServer.client().stop();
}

float mRSSi = 0.;
int WiFists = -1;

String onDebug(AutoConnectAux& aux, PageArgument& args)
{  char str[120];
extern int minRamFree;

//WiFiDebugInfo
   sprintf(str,"WiFi statistics:");
   DebugInfo1.value = str;
   sprintf(str,"%d %d  %d %d  %d %d  %d %d", 
      WiFiDebugInfo[0],WiFiDebugInfo[1],WiFiDebugInfo[2],WiFiDebugInfo[3],WiFiDebugInfo[4],WiFiDebugInfo[5],WiFiDebugInfo[6],WiFiDebugInfo[7]);
   DebugInfo2.value = str;
   if(WiFists == WL_CONNECTED)
   {  sprintf(str,"RSSI: %d dBm (%i%%), среднее за 10 мин %.1f",WiFi.RSSI(),_toWiFiQuality(WiFi.RSSI()), mRSSi);
      DebugInfo3.value = str;
   } else 
      DebugInfo3.value = "";
   sprintf(str,"OpenTherm statistics:");
   DebugInfo4.value = str;
   sprintf(str,"%d %d  %d %d  %d %d  %d %d  %d %d", 
      OTDebugInfo[0], OTDebugInfo[1], OTDebugInfo[2], OTDebugInfo[3], OTDebugInfo[4], OTDebugInfo[5], OTDebugInfo[6],OTDebugInfo[7], OTDebugInfo[8],OTDebugInfo[9]);
   DebugInfo5.value = str;
   sprintf(str,"min free RAM %d", minRamFree);
   DebugInfo6.value = str;
#if 0   
   {  int i;
      extern char ot_data_used[60];
      extern int ot_nids;

      for(i=0;i<ot_nids; i++)
      {    sprintf(str,"%d ", ot_data_used[i]); 
        if(i == 0)
           DebugInfo7.value = str;
        else
           DebugInfo7.value += str;
        if(i > 0 && (i%10 == 9))
            DebugInfo7.value += "<br>";
      }

   }
#endif //0   
  return String();
}

String onSetTemp(AutoConnectAux& aux, PageArgument& args)
{  char str[40];
   float  v, v1;
   int isChange=0;


    v = SetBoilerTemp.value.toFloat();
    if(SmOT.enable_CentralHeating && v != SmOT.Tset) isChange = 1;
    
    SmOT.Tset = v;

    v1 = SetDHWTemp.value.toFloat();
    if(SmOT.enable_HotWater && v1 != SmOT.TdhwSet) isChange = 1;
    SmOT.TdhwSet = v1;
    str[0] = 0;
    if(SmOT.enable_CentralHeating)
     {  sprintf(str,"Отопление %.1f",v);
        SmOT.need_set_T = 1;
     }
    SetTemp_info1.value =   str;
    if(SmOT.enable_HotWater)
    {   sprintf(str,"Горячая вода %.1f",v1);
        SmOT.need_set_dhwT = 1;
       SetTemp_info1.value += " ";
       SetTemp_info1.value +=  str;
    }

    if(SmOT.enable_CentralHeating2)
    {    v = SetBoilerTemp2.value.toFloat();
         if(v != SmOT.Tset2) isChange = 1;
         SmOT.Tset2 = v;
       sprintf(str,"CH2 %.1f",v);
        SmOT.need_set_dhwT = 1;
       SetTemp_info1.value += " ";
       SetTemp_info1.value +=  str;
    }

    if(isChange)
        SmOT.need_write_f = 1;

  return String();
}

String onSetPar(AutoConnectAux& aux, PageArgument& args)
{  int isChange=0;
   bool check;

  if( CtrlChB1.checked) check = true;
  else                  check = false;
  if(check != SmOT.enable_CentralHeating)
  { isChange++;
    SmOT.enable_CentralHeating = check;
  }

  if( CtrlChB2.checked) check = true;
  else                  check = false;
  if(check != SmOT.enable_HotWater)
  { isChange++;
    SmOT.enable_HotWater = check;
  }
  
  if( CtrlChB3.checked) check = true;
  else                  check = false;
  if(check != SmOT.enable_CentralHeating2)
  { isChange++;
    SmOT.enable_CentralHeating2 = check;
  }

  if(isChange)
        SmOT.need_write_f = 1;  //need write changes to FS

   SetPar_info1.value = "Отопление ";

    if(SmOT.enable_CentralHeating)
    {   SetPar_info1.value += "Вкл";
        SmOT.need_set_T = 1;
    } else {
       SetPar_info1.value += "вЫкл";
    }

    SetPar_info1.value += " Горячая вода ";
    if(SmOT.enable_HotWater)
    {   SetPar_info1.value += "Вкл";
        SmOT.need_set_dhwT = 1;
    } else {
       SetPar_info1.value += "вЫкл";
    }

    SetPar_info1.value += " Отопление2 ";
    if(SmOT.enable_CentralHeating2)
    {   SetPar_info1.value += "Вкл";
        SmOT.need_set_T2 = 1;
    } else {
       SetPar_info1.value += "вЫкл";
    }

  return String();
}


// Load the attribute of th
String onInfo(AutoConnectAux& aux, PageArgument& args) {
  char str0[80];
extern OpenTherm ot;

   switch(SmOT.stsOT)
   {  case -1:
        Info1.value =  String(SmOT.stsOT) + ": <b>Ошибка:</b> OT не инициализирован";
        SetDHWTemp.enable = false;
        SetBoilerTemp.enable = false;
        SetBoilerTemp2.enable = false;
        SetNewBoilerTemp.enable = false;
        break;
      case 0:
      {  char str[40];
        sprintf(str," (%8x)",SmOT.BoilerStatus );
        Info1.value =  String(SmOT.stsOT) +  str;
        if(SmOT.BoilerStatus & 0x01)
          Info1.value += "<br>Ошибка";
        if(SmOT.BoilerStatus & 0x02)
          Info1.value += "<br>Отопление Вкл";
        else  
          Info1.value += "<br>Отопление вЫкл";

        if(SmOT.HotWater_present)
        { if(SmOT.BoilerStatus & 0x04)
            Info1.value += "<br>Горячая вода Вкл";
          else  
            Info1.value += "<br>Горячая вода вЫкл";
        }

        if(SmOT.BoilerStatus & 0x08)
          Info1.value += "<br>Горелка Вкл";
        else  
          Info1.value += "<br>Горелка вЫкл";

        if(SmOT.CH2_present && SmOT.enable_CentralHeating2)
        {
          if(SmOT.BoilerStatus & 0x20)
            Info1.value += "<br>CH2 Вкл";
          else  
            Info1.value += "<br>CH2 вЫкл";
        }

        if(SmOT.BoilerStatus & 0x40)
          Info1.value += "<br>Diag";

      }
        break;
      case 1:
       Info1.value =  String(SmOT.stsOT) + " Invalid response";
        break;
      case 2:
      {  time_t now = time(nullptr);
        double dt;
        dt = difftime(now,SmOT.t_lastwork);
        if(dt < 3600.)
        {   sprintf(str0, "Потеря связи с котлом %.f сек назад", dt);

        } else {        
            sprintf(str0, "Потеря связи связи с котлом %.1f час(ов) назад", dt);
        }
        Info1.value =  String(SmOT.stsOT) + " : <b>Ошибка:</b> ";
        Info1.value +=  str0;
      }
        break;
   }

//  Serial.printf("Info1.value length=%i\n ", strlen(Info1.value.c_str()));

    if(SmOT.stsT1 >= 0 || SmOT.stsT2 >= 0)
    {   Info3.value = " Температура помещения ";
        if(SmOT.stsT1 >= 0)
          Info3.value += "T1 " + String(SmOT.t1) + " ";
        if(SmOT.stsT2 >= 0)
          Info3.value += "T2 " + String(SmOT.t2) ;
        Info3.value += "<br>";
    } else {
        Info3.value = "";
    }
    if(ot.OTid_used(OpenThermMessageID::Toutside))
    {   Info3.value += "Text " + String(SmOT.Toutside) + "<br>";
    }

  if(SmOT.stsOT != -1)
  {
   Info2.value = " Выходная температура  "  + String(SmOT.BoilerT);
      if(ot.OTid_used(OpenThermMessageID::Tret))
      { Info2.value +=  " Обратка " + String(SmOT.RetT);
      }
      if(ot.OTid_used(OpenThermMessageID::Texhaust))
      { sprintf(str0," Выхлоп %.0f", SmOT.Texhaust);
        Info2.value +=  str0;
      }

      if(SmOT.HotWater_present) 
      { Info2.value +=  " Горячая вода " + String(SmOT.dhw_t);
      }

      Info2.value += "<br>";

      Info4.value = "";
    
      if(ot.OTid_used(OpenThermMessageID::RelModLevel))
      {  Info4.value += " Flame "  + String(SmOT.FlameModulation) ;
      }

      if(ot.OTid_used(OpenThermMessageID::CHPressure))
      {
           Info4.value += " Pressure " + String(SmOT.Pressure);
      }
      Info4.value += "<br>";

      if(SmOT.enable_CentralHeating2)
      {  Info4.value += "T CH2 " +  String(SmOT.BoilerT2) + "<br>";
      }

// Info5.value = " MaxRelModLevel "  + String(SmOT.MaxRelModLevelSetting) + "<br>" + "Ts="+ String(SmOT.Tset) + "Tsr="+ String(SmOT.Tset_r) + "<br>";
   Info5.value = "Ts "+ String(SmOT.Tset) + "Tsr "+ String(SmOT.Tset_r) + "<br>";

    if(SmOT.OEMDcode || SmOT.Fault)
    {  sprintf(str0, "%x %x", SmOT.Fault, SmOT.OEMDcode);
//      Info5.value = "Fault = " + str0 + "<br>";
      Info6.value = "Fault = ";
      Info6.value += str0;
      Info6.value += "<br>";
    } else {
      Info6.value = "";
    }
//    Info7.value = " MinModLevel="  + String(SmOT.MinModLevel) + "<br>"  + " MaxCapacity="  + String(SmOT.MaxCapacity) + "<br>";
    Info7.value = "";

/******************************/  
    if(SmOT.enable_CentralHeating)
      SetBoilerTemp.enable = true;
    else 
      SetBoilerTemp.enable = false;

    if(SmOT.CH2_present && SmOT.enable_CentralHeating2)
        SetBoilerTemp2.enable = true;
    else 
        SetBoilerTemp2.enable = false;

    if( SmOT.enable_HotWater)
      SetDHWTemp.enable = true;
    else
      SetDHWTemp.enable = false;

    if( SmOT.enable_HotWater || SmOT.enable_CentralHeating||SmOT.enable_CentralHeating2)
        SetNewBoilerTemp.enable = true;
    else 
        SetNewBoilerTemp.enable = false;
  } else {
        Info2.value = "";
        Info4.value = "";
        Info5.value = "";
        Info6.value = "";
        Info7.value = "";
  }

 
/********************/
  return String();
}

// SmOT.OTmemberCode
String on_Setup(AutoConnectAux& aux, PageArgument& args)
{  
  if( SmOT.enable_CentralHeating)
      CtrlChB1.checked = true;
  else
      CtrlChB1.checked = false;

  if(SmOT.HotWater_present) 
  { CtrlChB2.enable  =  true;    
    if(SmOT.enable_HotWater)
      CtrlChB2.checked = true;
    else
      CtrlChB2.checked = false;
  } else {
     CtrlChB2.enable  = false;
  }

  if(SmOT.CH2_present) 
     CtrlChB3.enable  = true;
  else
     CtrlChB3.enable  = false;

  Ctrl2.value = "Котёл: "; 
  
/*
Buderus	8
Ferrolli 	9
Remeha	11
Viessmann  VITOPEND 	33
Navinien 	148

*/
  if(SmOT.OTmemberCode == 8)
      Ctrl2.value += "Buderus"; 
  else if(SmOT.OTmemberCode == 9)
      Ctrl2.value += "Ferrolli"; 
  else if(SmOT.OTmemberCode == 11)
      Ctrl2.value += "Remeha"; 
  else if(SmOT.OTmemberCode == 27)
      Ctrl2.value += "Baxi"; 
  else if(SmOT.OTmemberCode == 33)
      Ctrl2.value += "Viessmann"; 
  else if(SmOT.OTmemberCode == 148)
      Ctrl2.value += "Navinien"; 
  else 
      Ctrl2.value +=  "код " + String(SmOT.OTmemberCode);

  //Serial.printf("onControl checked=%i %i\n", CtrlChB1.checked, CtrlChB2.checked);

  return String();
}


//расшифровка битов статуса бойлера для бестолковых
//decoding the status bits of the boiler for dummies
/*  LB: Slave status   
bit: description [ clear/0, set/1]
0: fault indication [ no fault, fault ]
1: CH mode [CH not active, CH active]
2: DHW mode [ DHW not active, DHW active]
3: Flame status [ flame off, flame on ]
4: Cooling status [ cooling mode not active, cooling mode active ]
5: CH2 mode [CH2 not active, CH2 active]
6: diagnostic indication [no diagnostics, diagnostic event]
7: reserved 

E = Error 0/1
H = CH = CentralHeating 0/1
W = DHW = HotWater 0/1
F = Flame status  0/1
D =  diagnostic indication 0/1

Tb = BoilerT
Tr = RetT
Th = dhw_t
T1 = T_ds18b20(1)
T2 = T_ds18b20(2)
*/   

char SM_OT_HomePage[]=  "https://www.umkikit.ru/index.php?route=product/product&path=67&product_id=103";

String onAbout(AutoConnectAux& aux, PageArgument& args)
{ char str[80];
  About_1.value = IDENTIFY_TEXT;
  sprintf(str, "Vers %d.%d build %s\n",SmOT.Vers, SmOT.SubVers, SmOT.BiosDate);
  About_2.value = str;
  if (WiFi.status() == WL_CONNECTED)
  {   About_3.value = "<a href=";
      About_3.value += SM_OT_HomePage;
      About_3.value += ">Домашняя страница проекта</a>\n";
  } else 
    About_3.value ="";
    
  return String();
}

int sts = 0;
int sRSSI = 0;
int razRSSI = 0;
extern int LedSts; 

void loop_web()
{  int rc,  dt;
static unsigned long t0=0; // t1=0;

  portal.handleClient();
  if(sts == 0)
  {   Serial.println("loop_web()");
       sts = 1;
  }
  /* 3->0->3->7->1->7->1 //изменения статуса при коннекте-реконнекте
  typedef enum {
    WL_NO_SHIELD        = 255,   // for compatibility with WiFi Shield library
    WL_IDLE_STATUS      = 0,
    WL_NO_SSID_AVAIL    = 1,
    WL_SCAN_COMPLETED   = 2,
    WL_CONNECTED        = 3,
    WL_CONNECT_FAILED   = 4,
    WL_CONNECTION_LOST  = 5,
    WL_WRONG_PASSWORD   = 6,
    WL_DISCONNECTED     = 7
} wl_status_t; 
  */
  rc = WiFi.status();
  if(rc != WiFists)
  { Serial.printf("WiFi.status=%i\n", rc);
    if(rc == WL_CONNECTED)
    {  LedSts = 0;
 //     digitalWrite(LED_BUILTIN, LedSts);   
      Serial.printf("RSSI: %d dBm (%i%%)\n", WiFi.RSSI(),_toWiFiQuality(WiFi.RSSI()));
      
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    } else {
      LedSts = 1;
//      digitalWrite(LED_BUILTIN, LedSts);   
    }

    if( rc >=0 && rc <=7)
        WiFiDebugInfo[rc]++;
    WiFists = rc;
  }
  if(rc ==  WL_CONNECTED)
  {  dt = millis() - t0;
     if(dt > 10000)
     {   t0 = millis();
         razRSSI++;
         sRSSI += WiFi.RSSI();
  //Serial.printf(" WiFi.RSSI()=%i %i %i\n",  WiFi.RSSI(), dt, razRSSI);

         if(razRSSI > 6*10)
         {  mRSSi =  float(sRSSI)/float(razRSSI);
            razRSSI = 0;
            sRSSI = 0;
         }
    }
  }

//  time_t now = time(nullptr);
//  Serial.println(ctime(&now));
//  ba = Serial.available();
//  Serial.printf("a=%i ", ba);

//  Serial.printf("water_count=%i\n", water_count);
}

/**
 *  Convert dBm to the wifi signal quality.
 *  @param  rssi  dBm.
 *  @return A signal quality percentage.
 */
unsigned int /* AutoConnect:: */ _toWiFiQuality(int32_t rssi) {
  unsigned int  qu;
  if (rssi == 31)   // WiFi signal is weak and RSSI value is unreliable.
    qu = 0;
  else if (rssi <= -100)
    qu = 0;
  else if (rssi >= -50)
    qu = 100;
  else
    qu = 2 * (rssi + 100);
  return qu;
}

