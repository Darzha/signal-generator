/*
 * Signal Generator v1.0
 * Arduino-based DDS function generator with AD9833
 * 
 * Features:
 *   - Sine, triangle, and square wave output
 *   - Frequency range: 1 Hz to 2 MHz
 *   - Joystick-controlled menu system
 *   - Contrast and brightness control
 *   - Output on/off via joystick button
 * 
 * Hardware:
 *   - Arduino Nano/Uno
 *   - AD9833 DDS module (SPI)
 *   - 16x2 LCD (4-bit mode)
 *   - Analog joystick (KY-023 or similar)
 * 
 * Author: Asma Shateri
 * Date: June 2026
 * License: MIT
 */
// -------- DDS --------
#define MASTER_CLOCK 25000000UL
#define WAVE_SINE     0x2000
#define WAVE_TRIANGLE 0x2002
#define WAVE_SQUARE   0x2028
#define FSYNC_PIN 10
uint16_t currentWave = WAVE_SQUARE;
//----------------------
#define brightnessPin  3
#define contrastPin 9
#define VRx A0
#define VRy A1
#define SW A2

//libraries
  #include <LiquidCrystal.h>
  LiquidCrystal lcd(2, 8, 4, 5, 6, 7);
  //LiquidCrystal(rs, enable, d4, d5, d6, d7);
  
  #include <SPI.h>

//variables
    static int brightness = 255;
    static int contrast = 48;
  //joy stick value variables
    int xVal;
    int yVal;
    static int yValLowTres = 400;
    static int yValHighTres = 600;

    static int xValLowDoubleTres = 100; 
    static int xValLowTres = 400; 
    static int xValHighTres = 600;
    static int xValHighDoubleTres = 900;
  //menu index variables
    static int menuIndex = 0;
    static int old_menuIndex = 1;
    static int menuMax = 4;
    static int menuDebounceTime = 200;
  //signal generator variables
    static int signalShape = 0;
    static uint32_t frequency = 440;
    static uint32_t decade = 1;
    static uint32_t frequencyMultiplier = 440;
    
    static bool outputState = false;
    static bool lastSWState = HIGH;
    static unsigned long lastSWDebounce = 0;

//functions
  //signal generator
void writeAD9833(uint16_t data){
  digitalWrite(FSYNC_PIN, LOW);
  SPI.transfer16(data);
  digitalWrite(FSYNC_PIN, HIGH);
}
void setFrequency(double freq){
  uint32_t freqWord = (uint32_t)((freq * 268435456UL) / MASTER_CLOCK);
  writeAD9833(0x2100); // RESET + B28
  writeAD9833(0x4000 | (freqWord & 0x3FFF));
  writeAD9833(0x4000 | ((freqWord >> 14) & 0x3FFF));
  writeAD9833(currentWave); // re-apply waveform
}

void outputOff() {
  writeAD9833(0x2000 | 0x40);  // SLEEP12 + SLEEP1 (turn off DAC and internal clock)
}

void outputOn() {
  writeAD9833(currentWave);     // Restore the current waveform (exits sleep)
  setFrequency(frequency);
}

  //display   
void percentageBar(int x,int column,int row){ // Percentage Bar
  lcd.setCursor(column,row);
  lcd.print("          ");
  lcd.setCursor(column,row);
  int bars = map(x,0,100,0,10);
  bars = constrain(bars,0,10);
  for(int i = 1 ; i <= bars ; i++){
    lcd.write(byte(255));
  }
}



//setup
void setup() {
  //screen init
  lcd.begin(16, 2); // 16 characters, 2 lines
  //backlight pin
  analogWrite(brightnessPin,brightness);  // for  initial brightness
  analogWrite(contrastPin, contrast);  // for initial contrast
  
  //basically the start display intro
  lcd.home();
  lcd.print("Asma Shateri"); //
  lcd.setCursor(0, 1);
  lcd.print("Signal Generator");
  delay(1500);
  lcd.clear();
  lcd.setCursor(9, 0);
  lcd.print("%");
  for(int i = 0 ; i <= 100 ; i++){
    percentageBar(i,3,1);
    delay(25);
    lcd.setCursor(6, 0);
    lcd.print(i);
  }
  delay(700);
  lcd.clear();
  //setting up signal
  pinMode(FSYNC_PIN, OUTPUT);
  digitalWrite(FSYNC_PIN, HIGH);
  SPI.begin();
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE2));
  writeAD9833(0x2100);        // RESET + B28

  //switch
  pinMode(SW, INPUT_PULLUP);
}




//loop
void loop() {
  
  // Read joystick button inputs
  bool SWState = digitalRead(SW);

  // Read joystick analog inputs
  xVal = analogRead(VRx);
  yVal = analogRead(VRy);
  // Update menu index with some debounce time
  static unsigned long last_menuIndexUpdate = 0;
  unsigned long now = millis();

  if (now - last_menuIndexUpdate > menuDebounceTime) {
    if (yVal < yValLowTres) {
      menuIndex--;
      last_menuIndexUpdate = now;
    } else if (yVal > yValHighTres) {
      menuIndex++;
      last_menuIndexUpdate = now;
    }
    menuIndex = constrain(menuIndex, 0, menuMax);
  }
  // Check if menu changed this loop
    bool menuChanged = (menuIndex != old_menuIndex);

  //menu switch case base in menuindex
  switch (menuIndex){//------------------------------------------------------------------------------------------------------
    case 0: {//menu contrast
      static int old_contrastLevel = -1; // track last printed brightnessLevel
      if (menuChanged) {
        lcd.clear();
        lcd.home();
        lcd.print("1/5 >Contrast");
      }
      // Update brightnessLevel and print if changed
      int contrastLevel = map(contrast, 0, 255, 0, 100);
      contrastLevel = constrain(contrastLevel, 0, 100);
      if (menuChanged || contrastLevel != old_contrastLevel) {
        lcd.setCursor(12, 1);
        lcd.print("    ");      // clear old brightness
        lcd.setCursor(12, 1);
        lcd.print(contrastLevel);
        lcd.print("%");
        percentageBar(contrastLevel, 0, 1);
        // color range control  
        analogWrite(contrastPin,contrast);
        old_contrastLevel = contrastLevel;
      }
      // Read X axis for brightness change every 50ms
      static unsigned long lastContrastUpdate = 0;
      if (now - lastContrastUpdate > 50) {
        if (xVal < xValLowDoubleTres) contrast -= 4;
        else if (xVal < xValLowTres) contrast -= 1;
        else if (xVal > xValHighDoubleTres) contrast += 4;
        else if (xVal > xValHighTres) contrast += 1;
        contrast = constrain(contrast, 0, 255);
        lastContrastUpdate = now;
      }
      break;
    } //case 0 end



    case 1: {//menu brightness
      static int old_brightnessLevel = -1; // track last printed brightnessLevel
      if (menuChanged) {
        lcd.clear();
        lcd.home();
        lcd.print("2/5 >Light");
      }
      // Update brightnessLevel and print if changed
      int brightnessLevel = map(brightness, 0, 255, 0, 100);
      brightnessLevel = constrain(brightnessLevel, 0, 100);
      if (menuChanged || brightnessLevel != old_brightnessLevel) {
        lcd.setCursor(12, 1);
        lcd.print("    ");      // clear old brightness
        lcd.setCursor(12, 1);
        lcd.print(brightnessLevel);
        lcd.print("%");
        percentageBar(brightnessLevel, 0, 1);
        // color range control  
        analogWrite(brightnessPin,brightness);
        old_brightnessLevel = brightnessLevel;
      }
      // Read X axis for brightness change every 50ms
      static unsigned long lastBrightnessUpdate = 0;
      if (now - lastBrightnessUpdate > 50) {
        if (xVal < xValLowDoubleTres) brightness -= 4;
        else if (xVal < xValLowTres) brightness -= 1;
        else if (xVal > xValHighDoubleTres) brightness += 4;
        else if (xVal > xValHighTres) brightness += 1;
        brightness = constrain(brightness, 0, 255);
        lastBrightnessUpdate = now;
      }
      break;
    } //case 1 end


    case 2: {//shape
      static int old_signalShape = -1; // track last printed signal shape
      if (menuChanged) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("3/5 >Shape");
      }

      //signal gen word update
      if (signalShape != old_signalShape && outputState){
        switch(signalShape){
          case 0:{
            writeAD9833(WAVE_SQUARE);
            break;
          }
          case 1:{
            writeAD9833(WAVE_TRIANGLE);
            break;
          }
          case 2:{
            writeAD9833(WAVE_SINE);
            break;
          }
        }
      }
      //just the lcd update
      if (menuChanged || signalShape != old_signalShape){
        lcd.setCursor(0, 1);
        lcd.print("                ");
        lcd.setCursor(0, 1);
        switch(signalShape){
          case 0:{
            lcd.print("Square   -   1/3");
            currentWave = WAVE_SQUARE;
            break;
          }
          case 1:{
            lcd.print("Triangle  -  2/3");
            currentWave = WAVE_TRIANGLE;
            break;
          }
          case 2:{
            lcd.print("Sine    -    3/3");
            currentWave = WAVE_SINE;
            break;
          }
        }
      old_signalShape = signalShape;
      }

      
      // Read X axis for shape change every 200ms
      static unsigned long lastSignalShapeUpdate = 0;
      if (now - lastSignalShapeUpdate > 200) {
        if (xVal < xValLowTres) signalShape -= 1;
        else if (xVal > xValHighTres) signalShape += 1;
        signalShape = constrain(signalShape, 0, 2);
        lastSignalShapeUpdate = now;
      }
      break;
    } // case 2 end 
    
    
    case 3: {//frequncy
      static uint32_t old_frequency = -1; // track last printed signal frequency
      if (menuChanged) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("4/5 >Frequency");
        lcd.setCursor(14, 1);
        lcd.print("Hz");  
      }
      //just the lcd update
      if (menuChanged || frequency != old_frequency){
        lcd.setCursor(0, 1);
        lcd.print("              ");
        lcd.setCursor(0, 1);
        lcd.print(decade);
        lcd.print("X-");
        lcd.print(frequency);
      }
      //signal frequency word update      
      if (frequency != old_frequency){
        old_frequency = frequency;
        if (outputState){
          setFrequency(frequency);
        }
      }
      // Read X axis for frequeny change every 50ms
      static unsigned long lastFrequencyUpdate = 0;
      if (now - lastFrequencyUpdate > 50) {
          if (xVal < xValLowDoubleTres) frequencyMultiplier -= 10;
          else if (xVal < xValLowTres) frequencyMultiplier -= 1;
          else if (xVal > xValHighDoubleTres) frequencyMultiplier += 10;
          else if (xVal > xValHighTres) frequencyMultiplier += 1;

        frequencyMultiplier = constrain(frequencyMultiplier, 10, 999);
        frequency = frequencyMultiplier * decade;
        frequency = constrain(frequency, 1, 2000000);
        lastFrequencyUpdate = now;
      }
      break;
    }// case 3 end
    
    
    case 4: {//decade set
      static uint32_t old_decade = -1; // track last printed signal frequency
      if (menuChanged) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("5/5 >Decade");
        lcd.setCursor(14, 1);
        lcd.print("Hz");  
      }
      //just the lcd update
      if (menuChanged || decade != old_decade){
        lcd.setCursor(0, 1);
        lcd.print("              ");
        lcd.setCursor(0, 1);
        lcd.print(decade);
        lcd.print("X-");
        lcd.print(frequency);
      }
      //signal frequency word update
      if (decade != old_decade){
        old_decade = decade;
        if (outputState){
          setFrequency(frequency);
        }
      }
      // Read X axis for frequeny change every 200ms
      static unsigned long lastDecadeUpdate = 0;
      if (now - lastDecadeUpdate > 200) {
          if (xVal < xValLowTres) decade = decade / 10;
          else if (xVal > xValHighTres) decade = decade * 10;
        decade = constrain(decade, 1, 10000);
        frequency = frequencyMultiplier * decade;
        frequency = constrain(frequency, 1, 2000000);
        lastDecadeUpdate = now;
      }
      break;
    }//case 4 end

  }//switch end

  old_menuIndex = menuIndex;

  if (now - lastSWDebounce > 100) {
    if (SWState == LOW && lastSWState == HIGH) {
      // Button pressed: toggle output
      outputState = !outputState;
      if (outputState) {
        outputOn();
      } else {
        outputOff();
      }
    lastSWDebounce = now;
    }
  }
  lastSWState = SWState;
  
  static bool old_outputState = true; // track last printed brightnessLevel

  if (menuChanged || outputState != old_outputState){
    if (outputState) {
      lcd.setCursor(15, 0);
      lcd.print(" ");
      lcd.setCursor(15, 0);
      lcd.write(255);  // Filled rectangle (ON)
    } else {
      lcd.setCursor(15, 0);
      lcd.print(" ");
      lcd.setCursor(15, 0);
      lcd.write(219);  // Empty rectangle (OFF)
    }
  old_outputState = outputState;
  }
}
