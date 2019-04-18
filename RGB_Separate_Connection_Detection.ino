//Pins to control LED brightness
const int LED_OUTPUT_RED = 3;
const int LED_OUTPUT_GREEN = 5;
const int LED_OUTPUT_BLUE = 6;

//Pins to check for closed circuit
const int LED_CHECK_RED = 9;
const int LED_CHECK_GREEN = 10;
const int LED_CHECK_BLUE = 8;

//Pins to check voltage for determining color intensity
const int LED_ANALOG_IN_RED = A0;
const int LED_ANALOG_IN_GREEN = A1;
const int LED_ANALOG_IN_BLUE = A2;

const int PHOTORESISTOR_IN = A5;

//array for storing persistent rgb values
int rgbVals[] = {100, 100, 50};
float hue = 0.5f;


void setup() {
  pinMode(LED_OUTPUT_RED, OUTPUT);
  pinMode(LED_OUTPUT_GREEN, OUTPUT);
  pinMode(LED_OUTPUT_BLUE, OUTPUT);

  pinMode(LED_CHECK_RED, INPUT);
  pinMode(LED_CHECK_GREEN, INPUT);
  pinMode(LED_CHECK_BLUE, INPUT);

  pinMode(LED_ANALOG_IN_RED, INPUT);
  pinMode(LED_ANALOG_IN_GREEN, INPUT);
  pinMode(LED_ANALOG_IN_BLUE, INPUT);

  pinMode(PHOTORESISTOR_IN, INPUT);

  Serial.begin(9600);
}

void loop() {
  //check if switch (connected magnets) for each color channel is enabled
  //range of values used to account for noise
  delay(50);
  int readRed = analogRead(LED_CHECK_RED);
  bool redConnect = readRed > 650 && readRed < 660;
  delay(50);
  int readGreen = analogRead(LED_CHECK_GREEN); 
  bool greenConnect = readGreen > 650 && readGreen < 660;
  delay(50);;
  int readBlue = analogRead(LED_CHECK_BLUE); 
  bool blueConnect = readBlue > 650 && readBlue < 660;

  //read in potentiometer values for each color
  int redIn = analogRead(LED_ANALOG_IN_RED);
  delay(10);
  int greenIn = analogRead(LED_ANALOG_IN_GREEN);
  delay(10);
  int blueIn = analogRead(LED_ANALOG_IN_BLUE);
  delay(10);

  //check if all 3 colors are maxed in which case do something special
  bool allConnect = redConnect && greenConnect && blueConnect;

  //if all colors are connected do a rainbow pattern
  if(allConnect) {
    hue += 0.01f;
    hue = hue > 1.0f ? 0.0f : hue; //loop hue value when maxed
  } else { 
    //use potentiometer values of each color and remap to 255 value range
    //exact mapping values set based on testing individual potentiometers
    rgbVals[0] = map(redIn, 30, 54, 150, 255);
    rgbVals[1] = map(greenIn, 30, 60, 0, 150);
    rgbVals[2] = map(blueIn, 50, 74, 100, 255);
    //generate hue value from generated rgb values
    hue = rgbToHue(rgbVals);
  }

  //get photoresistor voltage and convert to lightness value
  int photoIn = analogRead(PHOTORESISTOR_IN);
  //current lighting range based on well lit room to completely covered photocell
  int photoInClamp = map(photoIn, 730, 1000, 0, 100);
  
  float lightness = (float)photoInClamp / 100.0f;
  float saturation = 1.0f; //always use full saturation to reduce complexity of color space
  hslToRgb(hue, saturation, lightness, rgbVals);

  //check if each color channel is connected and toggle that color light off if not
  rgbVals[0] = redConnect ? rgbVals[0] : 255;
  rgbVals[1] = greenConnect ? rgbVals[1] : 255;
  rgbVals[2] = blueConnect ? rgbVals[2] : 255;

  //set each led value based on current value in RGB array, remap to account for differeny voltage needs
  analogWrite(LED_OUTPUT_RED, map(rgbVals[0], 0, 255, 50, 255));
  delay(10);
  analogWrite(LED_OUTPUT_GREEN, map(rgbVals[1], 0, 255, 20, 255));
  delay(10);
  analogWrite(LED_OUTPUT_BLUE, map(rgbVals[2], 0, 255, 100, 255));
}

//converts hsl to rgb values and places results in rgb array
//hsl to rgb function modified from http://axonflux.com/handy-rgb-to-hsl-and-rgb-to-hsv-color-model-c
//and found at https://stackoverflow.com/questions/2353211/hsl-to-rgb-color-conversion
void hslToRgb(float h, float s, float l, int* rgb){
  if(s == 0.0f) {
    rgb[0] = 255.0f;
    rgb[1] = 255.0f;
    rgb[2] = 255.0f;
  }
  else {
    float q = l < 0.5f ? l * (1.0f + s) : (l + s) - (l * s);
    float p = 2.0f * l - q;
    rgb[0] = (int)(hueToRgb(p, q, h + (1.0f/3.0f)) * 255.0f);
    rgb[1] = (int)(hueToRgb(p, q, h) * 255.0f);
    rgb[2] = (int)(hueToRgb(p, q, h - (1.0f/3.0f)) * 255.0f);
  } 
}

//converts hsl values to rgb and places results in rgb array
float hueToRgb(float p, float q, float t) {
  if(t < 0.0f){ t += 1.0f;}
  if(t > 1.0f){ t -= 1.0f;}
  if(t < 1.0f/6.0f){ return p + (q - p) * 6.0f * t;}
  if(t < 1.0f/2.0f){ return q;}
  if(t < 2.0f/3.0f){ return p + (q - p) * (2.0f/3.0f - t) * 6.0f;}
  return p;
}

//rgb to hue calculation modified from https://stackoverflow.com/questions/23090019/fastest-formula-to-get-hue-from-rgb
float rgbToHue(int* rgb) {
  float rS = (255.0f - (float)rgb[0]) / 255.0f;
  float gS = (255.0f - (float)rgb[1]) / 255.0f;
  float bS = (255.0f - (float)rgb[2]) / 255.0f;

  float maxC  = max(max(rS, gS), bS);
  float minC = min(min(rS, gS), bS);

  if(maxC == rS) {
    return (abs(gS - bS) / (maxC - minC)) / 6.0f;
  } else if(maxC == gS) {
    return (2.0f + (abs(bS - rS)/(maxC - minC))) / 6.0f;
  } else {
    return (4.0f + (abs(rS - gS)/(maxC - minC))) / 6.0f;
  }  
}
