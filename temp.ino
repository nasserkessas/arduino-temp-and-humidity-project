#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHTPIN 7
#define BUTTONPIN 2
#define POTPIN A5
#define DHTTYPE DHT22
#define menuHoldTime 2000

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int buttonState = 0;
int selectedOption = 0;
int refresh_interval = 1000;
int idle_timeout_time = 10000;
bool lcd_on = false;
bool update_lcd = false;
bool button_off = true;
bool menu_open = false;
bool menu_toggled = false;
bool pot_changed = false;
bool button_held = false;
bool menu_click = false;
bool temp_scale_click = false;
bool temp_scale_toggled = false;
bool select_option_allowed = false;
bool show_temp_scale_menu = false;
bool refresh_click = false;
bool refresh_toggled = false;
bool show_refresh_menu = false;
bool timeout_click = false;
bool timeout_toggled = false;
bool show_timeout_menu = false;
int temp_scale = 0;
unsigned long start_lcd = 0;
unsigned long previousMillis = 0;
unsigned long start_hold = 0;
int i2c;
char output[17] = { 0 };

// struct option {
//   char *name;
//   char *type;
//   int start;
//   int end;
//   bool exit;
//   char **options;
//   int optionsLength;
// };

char *options[4] = { " Refresh time ", " Idle timeout ", "  Temp scale  ", "     Exit     " };
char *refreshOptions[5] = { " 0.5 seconds  ", "   1 second   ", "  2 seconds   ", "  5 seconds   ", "  10 seconds  " };
char *timeoutOptions[4] = { "  5 seconds   ", "  10 seconds  ", "  30 seconds  ", "   1 minute   " };
char *tempScaleOptions[3] = { "   Celsius    ", "  Fahrenheit  ", "    Kelvin    " };

void setup() {
  Serial.begin(9600);
  Wire.begin();

  i2c = detectI2CAddress();
  if (i2c == 0) {
    Serial.println("Error detecting I2C address");
    return;
  }
  Serial.print("Address: 0x");
  Serial.println(i2c, 16);

  dht.begin();
  lcd.begin(16, 2);

  pinMode(BUTTONPIN, INPUT);
}

void loop() {
  if (i2c == 0) return;

  unsigned long currentMillis = millis();

  buttonState = digitalRead(BUTTONPIN);

  if (buttonState == HIGH) {
    if (button_off) {
      start_hold = currentMillis;
    }

    if (currentMillis - start_hold >= menuHoldTime && !show_temp_scale_menu) {
      if (!menu_toggled && !menu_open) {
        menu_click = true;
        menu_open = true;
        menu_toggled = true;
      }
    }

    button_off = false;
    update_lcd = true;
    lcd_on = true;
    start_lcd = currentMillis;
  }

  if (buttonState == LOW) {
    if (!button_off) {
      button_off = true;
    }
  }

  if (currentMillis - start_lcd >= idle_timeout_time) {
    update_lcd = true;
    lcd_on = false;
  }

  if (menu_open) {

    if (buttonState == LOW && menu_click) {
      menu_click = false;
    }

    if (buttonState == HIGH && !menu_click) {
      select_option_allowed = true;
    }

    if (show_temp_scale_menu) {
      menu_click = true;
      show_temp_scale_menu = false;
      menu();
    }

    if (show_timeout_menu) {
      menu_click = true;
      show_timeout_menu = false;
      menu();
    }

    if (show_refresh_menu) {
      menu_click = true;
      show_refresh_menu = false;
      menu();
    }

    if (menu_toggled) {
      menu_click = true;
      menu();
    }

    if (selectedOption != analogRead(POTPIN) >> 8) {
      start_lcd = currentMillis;
      menu();
    }

    if (buttonState == HIGH) {
      start_lcd = currentMillis;
      if (!button_held) {
        button_held = true;
      }
    }
    if (buttonState == LOW && button_held && select_option_allowed) {
      if (selectedOption == 0) {
        show_refresh_menu = true;
        menu_open = false;
        refresh_click = true;
        refresh_toggled = true;
      }
      if (selectedOption == 1) {
        show_timeout_menu = true;
        menu_open = false;
        timeout_click = true;
        timeout_toggled = true;
      }
      if (selectedOption == 2) {
        show_temp_scale_menu = true;
        menu_open = false;
        temp_scale_click = true;
        temp_scale_toggled = true;
      }
      if (selectedOption == 3) {
        menu_open = false;
        select_option_allowed = false;
      }
      button_held = false;
    }
  }

  if (show_timeout_menu) {

    if (buttonState == LOW && timeout_click) {
      timeout_click = false;
    }

    if (buttonState == HIGH && !timeout_click) {
      select_option_allowed = true;
    }

    if (timeout_toggled) {
      timeout_toggled = false;
      timeout_click = true;
      timeoutMenu();
    }

    if (selectedOption != (analogRead(POTPIN) >> 8)) {
      start_lcd = currentMillis;
      timeoutMenu();
    }

    if (buttonState == HIGH) {
      start_lcd = currentMillis;
      if (!button_held) {
        button_held = true;
      }
    }
    if (buttonState == LOW && button_held && select_option_allowed) {
      menu_open = true;
      select_option_allowed = false;
      menu_click = true;
      menu_toggled = true;
      button_held = false;

      switch (selectedOption) {
        case 0:
          idle_timeout_time = 5000;
          break;
        case 1:
          idle_timeout_time = 10000;
          break;
        case 2:
          idle_timeout_time = 30000;
          break;
        case 3:
          idle_timeout_time = 60000;
          break;
      }
    }
  }

  if (show_temp_scale_menu) {

    if (buttonState == LOW && temp_scale_click) {
      temp_scale_click = false;
    }

    if (buttonState == HIGH && !temp_scale_click) {
      select_option_allowed = true;
    }

    if (temp_scale_toggled) {
      temp_scale_toggled = false;
      temp_scale_click = true;
      tempScaleMenu();
    }

    if (selectedOption != (analogRead(POTPIN) * 3) >> 10) {
      start_lcd = currentMillis;
      tempScaleMenu();
    }

    if (buttonState == HIGH) {
      start_lcd = currentMillis;
      if (!button_held) {
        button_held = true;
      }
    }
    if (buttonState == LOW && button_held && select_option_allowed) {
      menu_open = true;
      select_option_allowed = false;
      menu_click = true;
      menu_toggled = true;
      button_held = false;

      temp_scale = selectedOption;
    }
  }

  if (show_refresh_menu) {

    if (buttonState == LOW && refresh_click) {
      refresh_click = false;
    }

    if (buttonState == HIGH && !refresh_click) {
      select_option_allowed = true;
    }

    if (refresh_toggled) {
      refresh_toggled = false;
      refresh_click = true;
      refreshMenu();
    }

    if (selectedOption != (analogRead(POTPIN) * 5) >> 10) {
      start_lcd = currentMillis;
      refreshMenu();
    }

    if (buttonState == HIGH) {
      start_lcd = currentMillis;
      if (!button_held) {
        button_held = true;
      }
    }
    if (buttonState == LOW && button_held && select_option_allowed) {
      menu_open = true;
      select_option_allowed = false;
      menu_click = true;
      menu_toggled = true;
      button_held = false;

      switch (selectedOption) {
        case 0:
          refresh_interval = 500;
          break;
        case 1:
          refresh_interval = 1000;
          break;
        case 2:
          refresh_interval = 2000;
          break;
        case 3:
          refresh_interval = 5000;
          break;
        case 4:
          refresh_interval = 10000;
          break;
      }

      Serial.println(refreshOptions[selectedOption]);
    }
  }

  if (lcd_on && currentMillis - previousMillis >= refresh_interval && !menu_open && !show_temp_scale_menu && !show_refresh_menu && !show_timeout_menu) {
    previousMillis = currentMillis;
    lcdPrint(dht);
  }

  if (lcd_on && update_lcd) {
    update_lcd = false;
    lcd.backlight();
    lcd.display();
  }
  else if (!lcd_on && update_lcd) {
    update_lcd = false;
    menu_open = false;
    show_temp_scale_menu = false;
    show_timeout_menu = false;
    show_refresh_menu = false;
    lcd.noBacklight();
    lcd.noDisplay();
  }

  menu_toggled = false;

  delay(100);
}

float c2f(float c) {
  return 32 + c * 9 / 5;
}

float c2k(float c) {
  return c + 273.15;
}

void tempScaleMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("** Temp Scale **");

  selectedOption = (analogRead(POTPIN) * 3) >> 10;
  lcd.setCursor(0, 1);
  lcd.print(selectedOption == 0 ? " " : "<");
  lcd.print(tempScaleOptions[selectedOption]);
  lcd.print(selectedOption == 2 ? " " : ">");
}

void refreshMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("* Refresh time *");

  selectedOption = (analogRead(POTPIN) * 5) >> 10;
  lcd.setCursor(0, 1);
  lcd.print(selectedOption == 0 ? " " : "<");
  lcd.print(refreshOptions[selectedOption]);
  lcd.print(selectedOption == 4 ? " " : ">");
}

void timeoutMenu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("* Idle Timeout *");

  selectedOption = (analogRead(POTPIN) >> 8);
  lcd.setCursor(0, 1);
  lcd.print(selectedOption == 0 ? " " : "<");
  lcd.print(timeoutOptions[selectedOption]);
  lcd.print(selectedOption == 3 ? " " : ">");
}

int detectI2CAddress() {

  byte error, address;

  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      return address;
    }
  }
  return 0;
}

void menu() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("*** Settings ***");


  selectedOption = analogRead(POTPIN) >> 8;
  lcd.setCursor(0, 1);
  lcd.print(selectedOption == 0 ? " " : "<");
  lcd.print(options[selectedOption]);
  lcd.print(selectedOption == 3 ? " " : ">");
}

void lcdPrint(DHT sensor) {
  char tempS[5];
  char humS[5];
  char output[34];
  lcd.clear();

  float temp = sensor.readTemperature();

  switch (temp_scale) {
    case 0:
      dtostrf(temp, 5, 1, tempS);
      sprintf(output, "Temp:%s%c%c", tempS, 223, 'C');
      break;
    case 1:
      dtostrf(c2f(temp), 5, 1, tempS);
      sprintf(output, "Temp:%s%c%c", tempS, 223, 'F');
      break;
    case 2:
      dtostrf(c2k(temp), 5, 1, tempS);
      sprintf(output, "Temp: %s%c", tempS, 'K');
      break;
  }

  lcd.setCursor(0, 0);
  lcd.print(output);

  dtostrf(sensor.readHumidity(), 5, 1, humS);
  sprintf(output, "Hum: %s %%", humS);
  lcd.setCursor(0, 1);
  lcd.print(output);
}