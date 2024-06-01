/*
    Projet : CAFETIERE_AUTO
    Date Creation : 22/08/2023
    Date Revision : 30/05/2024
    Entreprise : 3SC4P3
    Auteur: Florian HOFBAUER
    Contact :
    But : Fichier unique
*/

/****************************************************************************
 *                           Libraries
 ****************************************************************************/

#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>

#include <LiquidCrystal_I2C.h>

#include <EEPROM.h>
#include <avr/wdt.h>

/****************************************************************************/

/****************************************************************************
 *                           Variables
 ****************************************************************************/
// PIN
int buttonUP = {2};
int buttonDOWN = {3};
int buttonRIGHT = {4};
int buttonLEFT = {5};
int buttonSELECT = {6};
int pinRelay = {7};

// Variables for pin
bool buttonUpState, buttonDownState, buttonRightState, buttonLeftState, buttonSelectState;

bool manual = 0;
bool test = 0;
int menu, sousMenu;
int chap;
int caseHour;
int activation = 1, durationHour = 0, durationMinute = 30, durationSecond = 0;
int alarmHour = 6, alarmMinute = 30, alarmSecond = 0;

/****************************************************************************
 *                           Structure for menu
 ****************************************************************************/
struct MenuItem {
    String name;
    MenuItem *subMenu;
    int subMenuLength;
};

MenuItem subMenuReglage[] = { {"ALARME", nullptr, 0}, {"DUREE", nullptr, 0} };
MenuItem mainMenu[] = { {"REGLAGES", subMenuReglage, 2}, {"ACTIVATION", nullptr, 0}, {"MANUEL", nullptr, 0} };

int mainMenuLength = sizeof(mainMenu);
MenuItem* currentMenu = mainMenu;
int currentMenuLength = mainMenuLength;
int currentLevel = 0; // Level of menu
int parentMenuIndex = 0; // Index of parent menu for sub menu back
/****************************************************************************/

LiquidCrystal_I2C Screen(0x3F, 16, 2); // Screen I2C config

void setup() {
    // Screen init
    Screen.init();
    Screen.backlight();
    delay(10);
    Screen.clear();
    Serial.begin(9600);

    // Init of pin
    pinMode(buttonUP, INPUT_PULLUP);
    pinMode(buttonDOWN, INPUT_PULLUP);
    pinMode(buttonRIGHT, INPUT_PULLUP);
    pinMode(buttonLEFT, INPUT_PULLUP);
    pinMode(buttonSELECT, INPUT_PULLUP);
    pinMode(pinRelay, OUTPUT);
    digitalWrite(pinRelay, LOW);

    // Read data from EEPROM
    activation = readIntFromEEPROM(0);
    durationHour = readIntFromEEPROM(2);
    durationMinute = readIntFromEEPROM(4);
    durationSecond = readIntFromEEPROM(6);
    alarmHour = readIntFromEEPROM(8);
    alarmMinute = readIntFromEEPROM(10);
    alarmSecond = readIntFromEEPROM(12);
}

void loop() {
    tmElements_t tm;

    checkButton();
    if (RTC.read(tm)) {
        Screen.print(printDay(tm.Wday));
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
        activateMenu();
        printManuel(manual);
        if (activation == 1) {
            Screen.setCursor(15, 0);
            Screen.print("*");
            alarm(tm.Hour, tm.Minute, tm.Second);
        }

        if (tm.Hour == 2 && tm.Minute == 0 && tm.Second == 0) {
            saveData();
            delay(100);
            reboot();
            // Reboot pour éviter les problèmes d'écran qui fige
        }
        if (tm.Hour == 14 && tm.Minute == 0 && tm.Second == 0) {
            saveData();
            delay(100);
            reboot();
            // Reboot pour éviter les problèmes d'écran qui fige
        }
    }
    delay(300);
}

void checkButton() {
    /* 
        Read state of all button 
    */
    buttonUpState = digitalRead(buttonUP);
    buttonDownState = digitalRead(buttonDOWN);
    buttonRightState = digitalRead(buttonRIGHT);
    buttonLeftState = digitalRead(buttonLEFT);
    buttonSelectState = digitalRead(buttonSELECT);
}

void displayMenu() {
    /*
        Function to display menu
    */
    Screen.clear();
    Screen.print(currentMenu[currentMenuIndex].name);
    delay(500);
    checkButton();
}

void activateMenu() {
    /*
        Function to launch menu 
    */
    checkButton();
    if (!buttonUpState) {
        currentMenuIndex = (currentMenuIndex + 1) % currentMenuLength;
        displayMenu();
        changeMenu();
    }
    if (!buttonDownState) {
        currentMenuIndex = (currentMenuIndex - 1) % currentMenuLength;
        displayMenu();
        changeMenu();
    }
    if (!buttonSelectState) {
        currentMenuIndex = (currentMenuIndex + 1) % currentMenuLength;
        displayMenu();
        changeMenu();
    }
    delay(10);
}

void changeMenu() {
    /*
        Function to naviguate in the menu
    */
    while (true) {
        checkButton();
        if (!buttonUpState) {
            currentMenuIndex = (currentMenuIndex + 1) % currentMenuLength;
            displayMenu();
        }
        if (!buttonDownState) {
            currentMenuIndex = (currentMenuIndex - 1) % currentMenuLength;
            displayMenu();
        }
        if (!buttonLeftState) {
            if (currentLevel > 0) {
                currentMenu = mainMenu;

                for (int i = 0; i < currentLevel - 1; i++) {
                    currentMenu = currentMenu[parentMenuIndex].subMenu;
                }

                currentMenuLength = sizeof(currentMenu) / sizeof(currentMenu[0]);
                currentMenuIndex = parentMenuIndex;
                currentLevel--;
                displayMenu();
            }
            else {
                return;
            }
        }
        if (!buttonRightState || !buttonSelectState) {
            if (currentMenu[currentMenuIndex].subMenu != nullptr) {
                // Go to sub menu
                parentMenuIndex = currentMenuIndex;
                currentMenu = currentMenu[currentMenuIndex].subMenu;
                currentMenuLength = currentMenu[currentMenuIndex].subMenuLength;
                currentMenuIndex = 0;
                currentLevel++;
                displayMenu();
            }
            else {
                // Action à réaliser lors de la sélection d'un élément du menu
                Screen.clear();
                switch (currentMenu[currentMenuIndex].name) {
                    case "ALARME": {
                        setAlarm();
                        break;
                    }
                    case "DUREE": {
                        setDuration();
                        break;
                    }
                    case "ACTIVATION": {
                        activationFunction();
                        break;
                    }
                    case "MANUEL": {
                        manualFunction();
                        break;
                    }
                    default: {
                        return;
                    }
                }
            }
        } 
    }
}

void blinkingHour(int hour, int minute, int second) {
    /*
        Function to blink hour while change

        Args : 
            - hour : [int] 
            - minute : [int] 
            - second : [int] 
            - Screen : [LiquidCrystal_I2C]
    */
    Screen.setCursor(4, 1);
    print2digits(hour);
    Screen.print(":");
    print2digits(minute);
    Screen.print(":");
    print2digits(second);

    delay(10);

    switch (caseHour) { // Blink at editing position
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
        case 2: {
            Screen.setCursor(15, 0);
            Screen.print("S");
            Screen.setCursor(10, 1);
            Screen.print("    ");
            break;
        }
        default : {
            break;
        }
    }

    delay(10);

    Screen.setCursor(4, 1);
    print2digits(hour);
    Screen.print(":");
    print2digits(minute);
    Screen.print(":");
    print2digits(second);

    delay(10);
}

void setAlarm() {
    /*
        Function to set time alarm
    */
    caseHour = 0;
    Screen.clear();
    Screen.setCursor(3, 0);
    Screen.print("Set Alarme");

    while (true) {
        checkButton();
        if (!buttonRightState) {
            caseHour++;
            caseHour = (caseHour + 3) % 3;
            delay(500);
        }
        if (!buttonLeftState) {
            caseHour--;
            caseHour = (caseHour + 3) % 3;
            delay(500);
        }

        blinkingHour(alarmHour, alarmMinute, alarmSecond)

        
        if (!buttonUpState) {
            switch (caseHour) { // Blink at editing position
                case 0: {
                    alarmHour++;
                    if (alarmHour == 24) {
                        alarmHour = 00;
                    }
                    delay(150);
                    break;
                }
                case 1: {
                    alarmMinute++;
                    if (alarmMinute == 60) {
                        alarmMinute = 00;
                    }
                    delay(150);
                    break;
                }
                case 2: {
                    alarmSecond++;
                    if (alarmSecond == 60) {
                        alarmSecond = 00;
                    }
                    delay(150);
                    break;
                }
                default : {
                    break;
                }
            }
        }
        if (!buttonDownState) {
            switch (caseHour) { // Blink at editing position
                case 0: {
                    alarmHour--;
                    if (alarmHour == -1) {
                        alarmHour = 23;
                    }
                    delay(150);
                    break;
                }
                case 1: {
                    alarmMinute--;
                    if (alarmMinute == -1) {
                        alarmMinute = 59;
                    }
                    delay(150);
                    break;
                }
                case 2: {
                    alarmSecond--;
                    if (alarmSecond == -1) {
                        alarmSecond = 59;
                    }
                    delay(150);
                    break;
                }
                default : {
                    break;
                }
            }
        }
        if (!buttonSelectState) {
            Screen.clear();
            Screen.setCursor(1, 0);
            Screen.print("ALARME REGLEE");
            caseHour = 4;
            delay(1000);
            return;
        }
    }
}

void setDuration() {
    /*
        Function to set the duration of "ON" mode
    */
    caseHour = 0;
    Screen.clear();
    Screen.setCursor(3, 0);
    Screen.print("Set Duree");

    while (true) {
        checkButton();
        if (!buttonRightState) {
            caseHour++;
            caseHour = (caseHour + 3) % 3;
            delay(500);
        }
        if (!buttonLeftState) {
            caseHour--;
            caseHour = (caseHour + 3) % 3;
            delay(500);
        }
        
        blinkingHour(durationHour, durationMinute, durationSecond);

        if (!buttonUpState) {
            switch (caseHour) { // Blink at editing position
                case 0: {
                    durationHour++;
                    delay(150);
                    break;
                }
                case 1: {
                    durationMinute++;
                    if (durationMinute == 60) {
                        durationMinute = 00;
                        durationHour++;
                    }
                    delay(150);
                    break;
                }
                case 2: {
                    durationSecond++;
                    if (durationSecond == 60) {
                        durationSecond = 00;
                        durationMinute++;
                    }
                    delay(150);
                    break;
                }
                default : {
                    break;
                }
            }
        }
        if (!buttonDownState) {
            switch (caseHour) { // Blink at editing position
                case 0: {
                    durationHour--;
                    if (durationHour == -1) {
                        durationHour = 00;
                        durationMinute = 59;
                    }
                    delay(150);
                    break;
                }
                case 1: {
                    durationMinute--;
                    if (durationMinute == -1) {
                        durationMinute = 59;
                        durationHour--;
                    }
                    delay(150);
                    break;
                }
                case 2: {
                    durationSecond--;
                    if (durationSecond == -1) {
                        durationSecond = 59;
                        durationMinute--;
                    }
                    delay(150);
                    break;
                }
                default : {
                    break;
                }
            }
        }
        checkButton();
        if (!buttonSelectState) {
            Screen.clear();
            Screen.setCursor(2, 0);
            Screen.print("DUREE REGLEE");
            caseHour = 4;
            delay(1000);
            return;
        }
    }
}

void activationFunction() {
    /*
        Function to activate alarm
    */
    while (true) {
        Screen.setCursor(2, 0);
        Screen.print("PRESS SELECT");
        checkButton();
        if (activation == 0) {
            if (!buttonSelectState) {
                Screen.clear();
                activation = 1;
                Screen.setCursor(5, 0);
                Screen.print("ALARME");
                Screen.setCursor(4, 1);
                Screen.print("ACTIVEE");
                delay(1000);
                return;
            }
            if (!buttonLeftState) {
                Screen.clear();
                Screen.setCursor(5, 0);
                Screen.print("ALARME");
                Screen.setCursor(3, 1);
                Screen.print("DESACTIVEE");
                delay(1000);
                return;
            }
        }
        checkButton();
        if (activation == 1) {
            if (!buttonSelectState) {
                Screen.clear();
                activation = 0;
                Screen.setCursor(5, 0);
                Screen.print("ALARME");
                Screen.setCursor(3, 1);
                Screen.print("DESACTIVEE");
                delay(1000);
                return;
            }
            if (!buttonLeftState) {
                Screen.clear();
                Screen.setCursor(5, 0);
                Screen.print("ALARME");
                Screen.setCursor(3, 1);
                Screen.print("ACTIVEE");
                delay(1000);
                return;
            }
        }
        checkButton();
    }

}

void manualFunction() {
    /*
        Function to activate manually
    */
    // While we don't use button, we wait
    while (true) {
        if (manual) {
            Screen.setCursor(5, 0);
            Screen.print("MANUAL");
            Screen.setCursor(7, 1);
            Screen.print("ON");
            checkButton();

            if (!buttonSelectState) {
                manual = 0;
                digitalWrite(pinRelay, LOW);
                Screen.clear();
                Screen.setCursor(5, 0);
                Screen.print("MANUAL");
                Screen.setCursor(7, 1);
                Screen.print("OFF");
                delay(1000);
                return;
            }
            if (!buttonLeftState) {
                delay(50);
                return;
            }
        }
        else {
            Screen.setCursor(5, 0);
            Screen.print("MANUAL");
            Screen.setCursor(7, 1);
            Screen.print("OFF");
            checkButton();

            if (!buttonSelectState) {
                manual = 1;
                digitalWrite(pinRelay, HIGH);
                Screen.clear();
                Screen.setCursor(5, 0);
                Screen.print("MANUAL");
                Screen.setCursor(7, 1);
                Screen.print("ON");
                delay(1000);
                return;
            }
            if (!buttonLeftState) {
                delay(50);
                return;
            }
        }
    }
}

void print2digits(int number) {
    /*
        Function to print 2 digits number in screen with 0 if it's neccessary

        Args : 
            - number : [int] number to print with two digit
            - Screen : [LiquidCrystal_I2C]
    */
    if (number >= 0 && number < 10) {
        Screen.write('0');
    }
    Screen.print(number);
}

void printManuel(int manual) {
    /*
        Function to print ON in case of it's manually set to ON
    */
    if (manual == 1) {
        Screen.setCursor(14, 0);
        Screen.print("ON");
    }
}

void alarm(int hour, int minute, int second) {
    /*
        Activation and desactivation of module with time if it's set

        Args : 
            - hour : [int] hour time
            - minute : [int] minute time
            - second : [int] second time
    */

    // Calculation of time to cut 
    int endHour = alarmHour + durationHour;
    int endMinute = alarmMinute + durationMinute;
    int endSecond = alarmSecond + durationSecond;

    if (endSecond >= 60) {
        endMinute++;
        endSecond = endSecond - 60;
    }
    if (endMinute >= 60) {
        endHour++;
        endMinute = endMinute - 60;
    }
    if (endHour >= 24) {
        endHour = endHour - 24;
    }

    delay(100);

    // Set ON if we pass alarm time 
    if (hour >= alarmHour && hour < endHour) {
        if (minute >= alarmMinute && minute < endMinute) {
            if (second >= alarmSecond && second < endSecond) {
                digitalWrite(pinRelay, HIGH);
                Screen.setCursor(13, 0);
                Screen.print("=>");
                delay(100);
            }
        }
    }

    // Set OFF at the end of duration
    if (hour == endHour) {
        if (minute == endMinute) {
            if (second == endSecond) {
                Screen.clear();
                digitalWrite(pinRelay, LOW);
                activation = 0; // set off activation
                delay(100);
            }
        }
    }
}

String printDay(int wday) {
    /*
        Function to print Week day

        Args : 
            - wday : [int] number between 1 and 7 with 1 = Sunday
        
        Out : 
            - week[wday - 1] : [String] day in the week
    */
    String week[7] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

    return week[wday - 1];
}

void writeIntIntoEEPROM(int address, int number) {
    /*
        Function to write int number in EEPROM

        Args : 
            - adress : adress of EEPROM
            - number : [int] int to save
    */
    byte byte1 = number & 0xFF;
    byte byte2 = number >> 8;
    EEPROM.update(address, byte1);
    EEPROM.update(address + 1, byte2);
}

int readIntFromEEPROM(int address) {
    /*
        Function to write int number in EEPROM

        Args : 
            - adress : adress of EEPROM
        
        Out : 
            - (byte2 << 8) + byte1 : [int] int to read
    */
    byte byte1 = EEPROM.read(address);
    byte byte2 = EEPROM.read(address + 1);
    return (byte2 << 8) + byte1;
}

void saveData() {
    /*
        Function to save important data in EEPROM
    */
    writeIntIntoEEPROM(0, activation);
    writeIntIntoEEPROM(2, durationHour);
    writeIntIntoEEPROM(4, durationMinute);
    writeIntIntoEEPROM(6, durationSecond);
    writeIntIntoEEPROM(8, alarmHour);
    writeIntIntoEEPROM(10, alarmMinute);
    writeIntIntoEEPROM(12, alarmSecond);
}

void reboot() {
    /*
        Reboot function
    */
    wdt_enable(WDTO_15MS); // activate watchdog
    while (1) {}; // and wait ...
}
