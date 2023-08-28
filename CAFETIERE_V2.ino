"""
    Projet : CAFETIERE_AUTO
    Date Creation : 
    Date Revision : 22/08/2023
    Entreprise : 3SC4P3
    Auteur: Florian HOFBAUER
    Contact : 
    But : Fichier unique
"""

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <avr/wdt.h>

int bouton_Haut = {2};
int bouton_Bas = {3};
int bouton_Droite = {4};
int bouton_Gauche = {5};
int bouton_Select = {6};
int pinRelay = {7};
bool etatBoutH, etatBoutB, etatBoutD, etatBoutG, etatBoutS;
bool passage = 0;
bool manual = 0;
bool test = 0;
int menu, sousMenu;
int chap;
int Test_case;
int heure, minutes, seconde;
int activation = 1, dureeActiveH = 0, dureeActiveM = 30, dureeActiveS = 0;
int alarmeHeure = 6, alarmeMinute = 30, alarmeSeconde = 0;
String Menu[40];

LiquidCrystal_I2C Screen(0x3F, 16, 2);

void setup() {
  Menu[10] = "REGLAGES";
  Menu[11] = "ALARME";
  Menu[12] = "DUREE";
  Menu[20] = "ACTIVATION";
  Menu[30] = "MANUAL";
  Screen.init();
  Screen.backlight();
  delay(10);
  pinMode(bouton_Haut, INPUT_PULLUP);
  pinMode(bouton_Bas, INPUT_PULLUP);
  pinMode(bouton_Droite, INPUT_PULLUP);
  pinMode(bouton_Gauche, INPUT_PULLUP);
  pinMode(bouton_Select, INPUT_PULLUP);
  pinMode(pinRelay, OUTPUT);
  digitalWrite(pinRelay, LOW);
  Screen.clear();
  Serial.begin(9600);
  
  activation = readIntFromEEPROM(0);
  dureeActiveH = readIntFromEEPROM(2);
  dureeActiveM = readIntFromEEPROM(4);
  dureeActiveS = readIntFromEEPROM(6);
  alarmeHeure = readIntFromEEPROM(8);
  alarmeMinute = readIntFromEEPROM(10);
  alarmeSeconde = readIntFromEEPROM(12);
  
}

void etatBout() {
  etatBoutH = digitalRead(bouton_Haut);
  etatBoutB = digitalRead(bouton_Bas);
  etatBoutD = digitalRead(bouton_Droite);
  etatBoutG = digitalRead(bouton_Gauche);
  etatBoutS = digitalRead(bouton_Select);
}

void declenchementMenu() {
  etatBout();
  if (!etatBoutH) {
    Screen.clear();
    menu = menu - 10;
    equivalentM();
    Screen.print(menu);
    Screen.print(".");
    Screen.print(Menu[menu]);
    delay(500);
    etatBout();
    Navigation();
  }
  if (!etatBoutB) {
    Screen.clear();
    menu = menu + 10;
    equivalentM();
    Screen.print(menu);
    Screen.print(".");
    Screen.print(Menu[menu]);
    delay(500);
    etatBout();
    Navigation();
  }
  if (!etatBoutS) {
    Screen.clear();
    menu = menu + 10;
    equivalentM();
    Screen.print(menu);
    Screen.print(".");
    Screen.print(Menu[menu]);
    delay(500);
    etatBout();
    Navigation();
  }
  delay(10);
}

void equivalentSM() {
  if (sousMenu == 3) {
    sousMenu = 1;
  }
  if (sousMenu == 0) {
    sousMenu = 2;
  }
}

void equivalentM() {
  if (menu == 0) {
    menu = 30;
  }
  if (menu == 40) {
    menu = 10;
  }
  if (menu == -10) {
    menu = 30;
  }
}

void Navigation() {
  while (menu != 00) {
    etatBout();
    if (!etatBoutH) {
      Screen.clear();
      menu = menu - 10;
      equivalentM();
      Screen.print(menu);
      Screen.print(".");
      Screen.print(Menu[menu]);
      delay(500);
      etatBout();
    }
    if (!etatBoutB) {
      Screen.clear();
      menu = menu + 10;
      equivalentM();
      Screen.print(menu);
      Screen.print(".");
      Screen.print(Menu[menu]);
      delay(500);
      etatBout();
    }
    if (!etatBoutG) {
      menu = 0;
      delay(500);
      Screen.clear();
    }
    if (!etatBoutD || !etatBoutS) {
      Screen.clear();
      if (menu == 20) {
        chap = menu;
        delay(250);
        passage = 0;
        SousMenu(chap);
      }
      else if (menu == 30) {
        chap = menu;
        delay(250);
        passage = 0;
        SousMenu(chap);
      }
      else {
        sousMenu = 1;
        Screen.print(menu + sousMenu);
        Screen.print(".");
        Screen.print(Menu[menu + sousMenu]);
        delay(500);
        while (sousMenu != 0) {
          etatBout();
          if (!etatBoutH) {
            Screen.clear();
            sousMenu = sousMenu - 1;
            equivalentSM();
            Screen.print(menu + sousMenu);
            Screen.print(".");
            Screen.print(Menu[menu + sousMenu]);
            delay(500);
            etatBout();
          }
          if (!etatBoutB) {
            Screen.clear();
            sousMenu = sousMenu + 1;
            equivalentSM();
            Screen.print(menu + sousMenu);
            Screen.print(".");
            Screen.print(Menu[menu + sousMenu]);
            delay(500);
            etatBout();
          }
          if (!etatBoutG) {
            Screen.clear();
            sousMenu = 0;
            Screen.print(menu);
            Screen.print(".");
            Screen.print(Menu[menu]);
            delay(500);
          }
          etatBout();
          if (!etatBoutD || !etatBoutS) {
            chap = menu + sousMenu;
            delay(250);
            passage = 0;
            SousMenu(chap);
          }
        }
      }
    }
  }
}

void SousMenu(int chap) {
  switch (chap) {
    case 11:
      while (!passage) {
        Test_case = 0;
        Screen.clear();
        Screen.setCursor(3, 0);
        Screen.print("Set Alarme");
        while (Test_case < 3) {
          etatBout();
          if (!etatBoutD) {
            Test_case ++;
            if (Test_case == 3) {
              Test_case = 0;
            }
            delay(500);
          }
          if (!etatBoutG) {
            Test_case --;
            if (Test_case == -1) {
              Test_case = 2;
            }
            delay(500);
          }
          Screen.setCursor(4, 1);
          print2digits(alarmeHeure);
          Screen.print(":");
          print2digits(alarmeMinute);
          Screen.print(":");
          print2digits(alarmeSeconde);
          switch (Test_case) { // Blink at editing position
            case 0: {
                Screen.setCursor(15, 0);
                Screen.print("H");
                Screen.setCursor(4, 1);
                Screen.print("  ");
                break;
              }
            case 1: {
                Screen.setCursor(15, 0);
                Screen.print("M");
                Screen.setCursor(7, 1);
                Screen.print("  ");
                break;
              }
            default: {
                Screen.setCursor(15, 0);
                Screen.print("S");
                Screen.setCursor(10, 1);
                Screen.print("    ");
                break;
              }
          }
          Screen.setCursor(4, 1);
          print2digits(alarmeHeure);
          Screen.print(":");
          print2digits(alarmeMinute);
          Screen.print(":");
          print2digits(alarmeSeconde);
          if (!etatBoutH) {
            switch (Test_case) { // Blink at editing position
              case 0: {
                  alarmeHeure ++;
                  if (alarmeHeure == 24) {
                    alarmeHeure = 00;
                  }
                  delay(150);
                  break;
                }
              case 1: {
                  alarmeMinute ++;
                  if (alarmeMinute == 60) {
                    alarmeMinute = 00;
                  }
                  delay(150);
                  break;
                }
              default: {
                  alarmeSeconde ++;
                  if (alarmeSeconde == 60) {
                    alarmeSeconde = 00;
                  }
                  delay(150);
                  break;
                }
            }
          }
          if (!etatBoutB) {
            switch (Test_case) { // Blink at editing position
              case 0: {
                  alarmeHeure --;
                  if (alarmeHeure == -1) {
                    alarmeHeure = 23;
                  }
                  delay(150);
                  break;
                }
              case 1: {
                  alarmeMinute --;
                  if (alarmeMinute == -1) {
                    alarmeMinute = 59;
                  }
                  delay(150);
                  break;
                }
              default: {
                  alarmeSeconde --;
                  if (alarmeSeconde == -1) {
                    alarmeSeconde = 59;
                  }
                  delay(150);
                  break;
                }
            }
          }
          if (!etatBoutS) {
            Screen.clear();
            Screen.setCursor(1, 0);
            Screen.print("ALARME REGLEE");
            Test_case = 4;
            delay(1000);
            passage += 1;
          }
        }
        etatBout();
        delay(250);
        Screen.clear();
      }
      break;

    case 12:
      while (!passage) {
        Test_case = 0;
        Screen.clear();
        Screen.setCursor(3, 0);
        Screen.print("Set Duree");
        while (Test_case < 3) {
          etatBout();
          if (!etatBoutD) {
            Test_case ++;
            if (Test_case == 3) {
              Test_case = 0;
            }
            delay(500);
          }
          if (!etatBoutG) {
            Test_case --;
            if (Test_case == -1) {
              Test_case = 2;
            }
            delay(500);
          }
          Screen.setCursor(4, 1);
          print2digits(dureeActiveH);
          Screen.print(":");
          print2digits(dureeActiveM);
          Screen.print(":");
          print2digits(dureeActiveS);
          switch (Test_case) { // Blink at editing position
            case 0: {
                Screen.setCursor(15, 0);
                Screen.print("H");
                Screen.setCursor(4, 1);
                Screen.print("  ");
                break;
              }
            case 1: {
                Screen.setCursor(15, 0);
                Screen.print("M");
                Screen.setCursor(7, 1);
                Screen.print("  ");
                break;
              }
            default: {
                Screen.setCursor(15, 0);
                Screen.print("S");
                Screen.setCursor(10, 1);
                Screen.print("    ");
                break;
              }
          }
          Screen.setCursor(4, 1);
          print2digits(dureeActiveH);
          Screen.print(":");
          print2digits(dureeActiveM);
          Screen.print(":");
          print2digits(dureeActiveS);
          if (!etatBoutH) {
            switch (Test_case) { // Blink at editing position
              case 0: {
                  dureeActiveH ++;
                  delay(150);
                  break;
                }
              case 1: {
                  dureeActiveM ++;
                  if (dureeActiveM == 60) {
                    dureeActiveM = 00;
                    dureeActiveH ++;
                  }
                  delay(150);
                  break;
                }
              default: {
                  dureeActiveS ++;
                  if (dureeActiveS == 60) {
                    dureeActiveS = 00;
                    dureeActiveM ++;
                  }
                  delay(150);
                  break;
                }
            }
          }
          if (!etatBoutB) {
            switch (Test_case) { // Blink at editing position
              case 0: {
                  dureeActiveH --;
                  if (dureeActiveH == -1) {
                    dureeActiveH = 00;
                    dureeActiveM = 59;
                  }
                  delay(150);
                  break;
                }
              case 1: {
                  dureeActiveM --;
                  if (dureeActiveM == -1) {
                    dureeActiveM = 59;
                    dureeActiveH --;
                  }
                  delay(150);
                  break;
                }
              default: {
                  dureeActiveS --;
                  if (dureeActiveS == -1) {
                    dureeActiveS = 59;
                    dureeActiveM --;
                  }
                  delay(150);
                  break;
                }
            }
          }
          etatBout();
          if (!etatBoutS) {
            Screen.clear();
            Screen.setCursor(2, 0);
            Screen.print("DUREE REGLEE");
            Test_case = 4;
            delay(1000);
            passage += 1;
          }
        }
        etatBout();
        delay(250);
        Screen.clear();
      }
      break;

    case 20:
      Screen.clear();
      while (!passage) {
        Screen.setCursor(2, 0);
        Screen.print("PRESS SELECT");
        etatBout();
        if (activation == 0) {
          if (!etatBoutS) {
            Screen.clear();
            activation = 1;
            Screen.setCursor(5, 0);
            Screen.print("ALARME");
            Screen.setCursor(4, 1);
            Screen.print("ACTIVEE");
            passage += 1;
            delay(1000);
          }
        }
        etatBout();
        if (activation == 1) {
          if (!etatBoutS) {
            Screen.clear();
            activation = 0;
            Screen.setCursor(5, 0);
            Screen.print("ALARME");
            Screen.setCursor(3, 1);
            Screen.print("DESACTIVEE");
            passage += 1;
            delay(1000);
          }
        }
        etatBout();
      }
      break;
    case 30:
      while (!passage) {
        test = 0;
        while (test == 0) {
          if (manual == 0) {
            Screen.setCursor(5, 0);
            Screen.print("MANUAL");
            Screen.setCursor(7, 1);
            Screen.print("OFF");
            etatBout();
            if (!etatBoutS) {
              manual = 1;
              digitalWrite(pinRelay, HIGH);
              Screen.clear();
              Screen.setCursor(5, 0);
              Screen.print("MANUAL");
              Screen.setCursor(7, 1);
              Screen.print("ON");
              passage += 1;
              test += 1;
              delay(1000);
            }
          }
          else {
            Screen.setCursor(5, 0);
            Screen.print("MANUAL");
            Screen.setCursor(7, 1);
            Screen.print("ON");
            etatBout();
            if (!etatBoutS) {
              manual = 0;
              digitalWrite(pinRelay, LOW);
              Screen.clear();
              Screen.setCursor(5, 0);
              Screen.print("MANUAL");
              Screen.setCursor(7, 1);
              Screen.print("OFF");
              passage += 1;
              test += 1;
              delay(1000);
            }
          }
        }
      }
      break;
  }
  menu = 0;
  sousMenu = 0;
}


void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Screen.write('0');
  }
  Screen.print(number);
}
void manuel(int manual) {
  if (manual == 1) {
    Screen.setCursor(14, 0);
    Screen.print("ON");
  }
}

void alarme() {
  tmElements_t tm;
  if (RTC.read(tm)) {
    heure = tm.Hour;
    minutes = tm.Minute;
    seconde = tm.Second;
  }
  int finH = alarmeHeure + dureeActiveH;
  int finM = alarmeMinute + dureeActiveM;
  int finS = alarmeSeconde + dureeActiveS;
  if (finS >= 60) {
    finM ++;
    finS = finS - 60;
  }
  if (finM >= 60) {
    finH ++;
    finM = finM - 60;
  }
  delay(100);
  if (heure >= alarmeHeure && heure < finH) {
    if (minutes >= alarmeMinute && minutes < finM) {
      if (seconde >= alarmeSeconde && seconde < finS) {
        digitalWrite(pinRelay, HIGH);
        Screen.setCursor(13, 0);
        Screen.print("=>");
        delay(100);
      }
    }
  }
  if (heure == finH) {
    if (minutes == finM) {
      if (seconde == finS) {
        Screen.clear();
        digitalWrite(pinRelay, LOW);
        activation = 0;
        delay(100);
      }
    }
  }
}

void loop() {
  tmElements_t tm;

  etatBout();
  if (RTC.read(tm)) {
    Screen.setCursor(4, 0);
    print2digits(tm.Hour);
    Screen.write(':');
    print2digits(tm.Minute);
    Screen.write(':');
    print2digits(tm.Second);
    Screen.setCursor(3, 1);
    print2digits(tm.Day);
    Screen.write('/');
    print2digits(tm.Month);
    Screen.write('/');
    Screen.print(tmYearToCalendar(tm.Year));
    declenchementMenu();
    manuel(manual);
    if (activation == 1) {
      Screen.setCursor(15, 0);
      Screen.print("*");
      alarme();
    }

    saveData();
    
    if (tm.Hour == 2 && tm.Minute == 0 && tm.Second == 0) {
      saveData();
      delay(100);
      reboot();
      //Reboot pour éviter les problèmes d'écran qui fige
    }
    if (tm.Hour == 14 && tm.Minute == 0 && tm.Second == 0) {
      saveData();
      delay(100);
      reboot();
      //Reboot pour éviter les problèmes d'écran qui fige
    }
  }
  delay(100);
}

void writeIntIntoEEPROM(int address, int number) { 
  byte byte1 = number & 0xFF;
  byte byte2 = number >> 8;
  EEPROM.update(address, byte1);
  EEPROM.update(address + 1, byte2);
}

int readIntFromEEPROM(int address) {
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte2 << 8) + byte1;
}

void saveData() {
  writeIntIntoEEPROM(0, activation);
  writeIntIntoEEPROM(2, dureeActiveH);
  writeIntIntoEEPROM(4, dureeActiveM);
  writeIntIntoEEPROM(6, dureeActiveS);
  writeIntIntoEEPROM(8, alarmeHeure);
  writeIntIntoEEPROM(10, alarmeMinute);
  writeIntIntoEEPROM(12, alarmeSeconde);
}

void reboot() {
  wdt_enable(WDTO_15MS); // activer le watchdog
  while (1) {};          // et attendre ...
}
