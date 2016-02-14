#include <Adafruit_NeoPixel.h>
// Pattern types supported:
enum  pattern { 
  NONE, RAINBOW_CYCLE, THEATER_CHASE, COLOR_WIPE, SCANNER, FADE, LOADER };
// Patern directions supported:
enum  direction { 
  FORWARD, REVERSE };

#include <CapacitiveSensor.h>
#include <Math.h>

CapacitiveSensor capativeSensorHalf = CapacitiveSensor(4,7); 
CapacitiveSensor capativeSensorFull = CapacitiveSensor(8,11);

// NeoPattern Class - derived from the Adafruit_NeoPixel class
class NeoPatterns : 
public Adafruit_NeoPixel
{
public:

  // Member Variables:  
  pattern  ActivePattern;  // which pattern is running
  direction Direction;     // direction to run the pattern

    unsigned long Interval;   // milliseconds between updates
  unsigned long lastUpdate; // last update of position

  uint32_t Color1, Color2;  // What colors are in use
  uint16_t TotalSteps;  // total number of steps in the pattern
  uint16_t Index;  // current step within the pattern

  void (*OnComplete)();  // Callback on completion of pattern
  
  boolean AllFadeInIsDone;
  int Counter;
  
  // Constructor - calls base-class constructor to initialize strip
  NeoPatterns(uint16_t pixels, uint8_t pin, uint8_t type, void (*callback)())
:
    Adafruit_NeoPixel(pixels, pin, type)
    {
      OnComplete = callback;
    }

  // Update the pattern
  void Update()
  {
    if((millis() - lastUpdate) > Interval) // time to update
    {
      lastUpdate = millis();
      switch(ActivePattern)
      {
      case RAINBOW_CYCLE:
        RainbowCycleUpdate();
        break;
      case LOADER:
        LoaderCycleUpdate();
        break;
      case THEATER_CHASE:
        TheaterChaseUpdate();
        break;
      case COLOR_WIPE:
        ColorWipeUpdate();
        break;
      case SCANNER:
        ScannerUpdate();
        break;
      case FADE:
        FadeUpdate();
        break;
      default:
        break;
      }
    }
  }

  // Increment the Index and reset at the end
  void Increment()
  {
    
    if (Direction == FORWARD)
    {
      Index++;
      if (Index >= TotalSteps)
      {
        Index = 0;
        if (OnComplete != NULL)
        {
          OnComplete(); // call the comlpetion callback
        }
      }
    }
    if (Direction == REVERSE)
    {
      --Index;
      if (Index <= 0)
      {
        Index = TotalSteps-1;
        if (OnComplete != NULL)
        {
          OnComplete(); // call the comlpetion callback
        }
      }
    }
  }

  // Reverse pattern direction
  void Reverse()
  {
    if (Direction == FORWARD)
    {
      Direction = REVERSE;
      Index = TotalSteps-1;
    }
    else
    {
      Direction = FORWARD;
      Index = 0;
    }
  }

  // Initialize for a RainbowCycle
  void RainbowCycle(uint8_t interval, direction dir = FORWARD)
  {
    ActivePattern = RAINBOW_CYCLE;
    Interval = interval;
    TotalSteps = 255;
    Index = 0;
    Direction = dir;
  }

  // Update the Rainbow Cycle Pattern
  void RainbowCycleUpdate()
  {
    for(int i=0; i< numPixels(); i++)
    {
      setPixelColor(i, Wheel(((i * 256 / numPixels()) + Index) & 255));
    }
    show();
    Increment();
  }

  // Initialize for a Theater Chase
  void TheaterChase(uint32_t color1, uint32_t color2, uint8_t interval, direction dir = FORWARD)
  {
    ActivePattern = THEATER_CHASE;
    Interval = interval;
    TotalSteps = numPixels();
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
  }

  // Update the Theater Chase Pattern
  void TheaterChaseUpdate()
  {

    for(int i=0; i< numPixels(); i++)
    {
      if ((i + Index) % 3 == 0)
      {
        setPixelColor(i, Color1);
      }
      else
      {
        setPixelColor(i, Color2);
      }
    }
    show();
    Increment();
  }

  // Initialize for a ColorWipe
  void ColorWipe(uint32_t color, uint8_t interval, direction dir = FORWARD)
  {
    ActivePattern = COLOR_WIPE;
    Interval = interval;
    TotalSteps = numPixels();
    Color1 = color;
    Index = 0;
    Direction = dir;
  }

  // Update the Color Wipe Pattern
  void ColorWipeUpdate()
  {
    setPixelColor(Index, Color1);
    show();
    Increment();
  }

  void Loader(uint32_t color1, uint8_t interval)
  {
    ActivePattern = SCANNER;
    Interval = interval;
    TotalSteps = numPixels();
    Index = 0;
  }
  
  uint32_t blendRGBColors( uint8_t Color1, uint8_t Color2, float Percent ) {
    // Calculate linear interpolation between Color1 and Color2
    // Optimise order of operations to minimize truncation error
    uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
    uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
    uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;
    
    return Color(red,green,blue);
  }
  
  void LoaderCycleUpdate()
  {      
    for (int i = 0; i < numPixels(); i++)
    {
      if (i == Index)  // Scan Pixel to the right
      {
        setPixelColor(i, Color1);
      }
      else if (i == TotalSteps - Index) // Scan Pixel to the left
      {
        setPixelColor(i, Color1);
      }
      else // Fading tail
      {
        setPixelColor(i, DimColor(getPixelColor(i)) );
      }
    }
    show();
    Increment();
  }

  // Initialize for a SCANNNER
  void Scanner(uint32_t color1, uint8_t interval)
  {
    ActivePattern = SCANNER;
    Interval = interval;
    TotalSteps = (numPixels() - 1) * 2;
    Color1 = color1;
    Index = 0;
  }

  // Update the Scanner Pattern
  void ScannerUpdate()
  { 
    for (int i = 0; i < numPixels(); i++)
    {
      if (i == Index)  // Scan Pixel to the right
      {
        setPixelColor(i, Color1);
      }
      else if (i == TotalSteps - Index) // Scan Pixel to the left
      {
        setPixelColor(i, Color1);
      }
      else // Fading tail
      {
        setPixelColor(i, DimColor(getPixelColor(i)) );
      }
    }
    show();
    Increment();
  }

  // Initialize for a Fade
  void Fade(uint32_t color1, uint32_t color2, uint16_t steps, uint8_t interval, direction dir = FORWARD)
  {
    ActivePattern = FADE;
    Interval = interval;
    TotalSteps = steps;
    Color1 = color1;
    Color2 = color2;
    Index = 0;
    Direction = dir;
  }

  // Update the Fade Pattern
  void FadeUpdate()
  {
    // Calculate linear interpolation between Color1 and Color2
    // Optimise order of operations to minimize truncation error
    uint8_t red = ((Red(Color1) * (TotalSteps - Index)) + (Red(Color2) * Index)) / TotalSteps;
    uint8_t green = ((Green(Color1) * (TotalSteps - Index)) + (Green(Color2) * Index)) / TotalSteps;
    uint8_t blue = ((Blue(Color1) * (TotalSteps - Index)) + (Blue(Color2) * Index)) / TotalSteps;

    ColorSet(Color(red, green, blue));
    show();
    Increment();
  }
  
  // Calculate 50% dimmed version of a color (used by ScannerUpdate)
  uint32_t DimColor(uint32_t color)
  {
    // Shift R, G and B components one bit to the right
    uint32_t dimColor = Color(Red(color) >> 1, Green(color) >> 1, Blue(color) >> 1);
    return dimColor;
  }

  // Set all pixels to a color (synchronously)
  void ColorSet(uint32_t color)
  {
    for (int i = 0; i < numPixels(); i++)
    {
      setPixelColor(i, color);
    }
    show();
  }

  // Returns the Red component of a 32-bit color
  uint8_t Red(uint32_t color)
  {
    return (color >> 16) & 0xFF;
  }

  // Returns the Green component of a 32-bit color
  uint8_t Green(uint32_t color)
  {
    return (color >> 8) & 0xFF;
  }

  // Returns the Blue component of a 32-bit color
  uint8_t Blue(uint32_t color)
  {
    return color & 0xFF;
  }

  // Input a value 0 to 255 to get a color value.
  // The colours are a transition r - g - b - back to r.
  uint32_t Wheel(byte WheelPos)
  {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85)
    {
      return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    else if(WheelPos < 170)
    {
      WheelPos -= 85;
      return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    else
    {
      WheelPos -= 170;
      return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
    }
  }
};

// Define some NeoPatterns for the two rings and the stick
//  as well as some completion routines

#include <FastLED.h>

CRGB Ring[NUM_LEDS];

FastLED.addLeds<WS2811, 3, GRB>(Ring, 16).setCorrection( TypicalLEDStrip );
FastLED.setBrightness(  BRIGHTNESS );

FastLED.addLeds<WS2811, 5, GRB>(Ring, 16).setCorrection( TypicalLEDStrip );
FastLED.setBrightness(  BRIGHTNESS );

boolean buttonPushedFull = false; 
boolean buttonPushedHalf = false;

#define RING_FULL_ID 0
#define RING_HALF_ID 1

#define ANIMATION_START 1
#define ANIMATION_BUTTON_IDLE 2
#define ANIMATION_BUTTON_KEYDOWN 3
#define ANIMATION_BUTTON_KEYUP 4
#define ANIMATION_BUTTON_FLUSH 5
#define ANIMATION_BUTTON_FAIL 99

int currentHalfAnimation = ANIMATION_START;
int currentFullAnimation = ANIMATION_START;

#define output_full A3
#define output_half A2

// RingHalf Completion Callback
void RingHalfComplete()
{
  switch(currentHalfAnimation)
  {
    case ANIMATION_START:
       
    //  Serial.println("RingFullComplete ANIMATION_START set animation to ANIMATION_BUTTON_IDLE");   
      currentHalfAnimation = ANIMATION_BUTTON_IDLE; 
      
    break;

    case ANIMATION_BUTTON_IDLE:  
  
  //  Serial.println("RingHalfComplete ANIMATION_BUTTON_IDLE reverse");           
  //  RingHalf.Reverse();    

    break;

    case ANIMATION_BUTTON_KEYDOWN:

      // sync
      //RingHalf.Color1 = RingFull.Color2;          
  
      //Serial.println("RingHalfComplete ANIMATION_BUTTON_KEYDOWN set animation to ANIMATION_BUTTON_KEYDOWN");
      RingHalf.Index = 0;
      currentHalfAnimation = ANIMATION_BUTTON_FLUSH; 
      
    break;
    
    case ANIMATION_BUTTON_FLUSH:

      // sync
      //RingHalf.Color1 = RingFull.Color2;          
  
      //Serial.println("RingHalfComplete ANIMATION_BUTTON_KEYUP set animation to ANIMATION_BUTTON_IDLE");
      currentHalfAnimation = ANIMATION_BUTTON_KEYUP; 
      
    break;
    
    case ANIMATION_BUTTON_KEYUP:

      // sync
      //RingHalf.Color1 = RingFull.Color2;          
  
      //Serial.println("RingHalfComplete ANIMATION_BUTTON_KEYUP set animation to ANIMATION_BUTTON_IDLE");
      currentHalfAnimation = ANIMATION_BUTTON_IDLE; 
      
    break;


    case ANIMATION_BUTTON_FAIL:
  
      //Serial.println("RingHalfComplete ANIMATION_BUTTON_KEYDOWN set animation to ANIMATION_BUTTON_IDLE");
      currentHalfAnimation = ANIMATION_BUTTON_IDLE; 
      currentFullAnimation = ANIMATION_BUTTON_IDLE; 

    break;
  }
}

// Ring 2 Completion Callback
void RingFullComplete()
{    
  switch(currentFullAnimation)
  {
    case ANIMATION_START:     

//      Serial.println("RingFullComplete ANIMATION_START set animation to ANIMATION_BUTTON_IDLE");   
      currentFullAnimation = ANIMATION_BUTTON_IDLE; 

    break;

    case ANIMATION_BUTTON_IDLE:   

//     Serial.println("RingFullComplete ANIMATION_BUTTON_IDLE stop");           
//    RingFull.Reverse();

    break;

    case ANIMATION_BUTTON_KEYDOWN:

      // sync
      //RingHalf.Color1 = RingFull.Color2;          
  
//      Serial.println("currentFullAnimation ANIMATION_BUTTON_KEYDOWN set animation to ANIMATION_BUTTON_KEYDOWN");
      RingFull.Index = 0;
      currentFullAnimation = ANIMATION_BUTTON_FLUSH; 
      
    break;
    
    case ANIMATION_BUTTON_FLUSH:

      // sync
      //RingHalf.Color1 = RingFull.Color2;          
  
//      Serial.println("RingHalfComplete ANIMATION_BUTTON_FLUSH set animation to ANIMATION_BUTTON_KEYUP");     
      currentFullAnimation = ANIMATION_BUTTON_KEYUP; 
      
    break;

    case ANIMATION_BUTTON_KEYUP:

      // sync
      //RingHalf.Color1 = RingFull.Color2;          
  
//      Serial.println("currentFullAnimation ANIMATION_BUTTON_KEYUP set animation to ANIMATION_BUTTON_IDLE");
      currentFullAnimation = ANIMATION_BUTTON_IDLE; 
      
    break;


    case ANIMATION_BUTTON_FAIL:
  
//      Serial.println("RingHalfComplete ANIMATION_BUTTON_KEYDOWN set animation to ANIMATION_BUTTON_IDLE");
      currentHalfAnimation = ANIMATION_BUTTON_IDLE; 
      currentFullAnimation = ANIMATION_BUTTON_IDLE; 
      
    break;
  }    
}

long input_full;
long input_half;  

boolean start = true;

// sensor definitions
#define SENSOR_THRESHOLD 300

void FillLEDsFromPaletteColors( int ring, uint8_t colorIndex)
{
  uint8_t brightness = 255;
  
  for( int i = 0; i < NUM_LEDS; i++) {
    leds[ring][i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}


// RingFull Completion Callback
void Animate( int animation, int ring )
{    
  switch( animation )
  {
    case ANIMATION_START:
       
      Ring[ring].Interval = 10;     
    
      Ring[ring].ActivePattern = FADE;    
      Ring[ring].Color1 = Ring[ring].Color(0,0,0); 
      Ring[ring].Color2 = Ring[ring].Color(0,0,255);  
      Ring[ring].TotalSteps = 100;

    break;
    
    case ANIMATION_BUTTON_KEYDOWN:           
    
    //reset interval
    Ring[ring].Interval = 20;          

    Ring[ring].ActivePattern = FADE;    
    Ring[ring].Color1 = Ring[ring].Color(0,0,255);  
    Ring[ring].Color2 = Ring[ring].Color(255,0,0);  
    Ring[ring].TotalSteps = 100;

    break;
      
  case ANIMATION_BUTTON_FLUSH:      
    
    digitalWrite(output_half, HIGH);    
    if( ring == RING_FULL_ID ){
      digitalWrite(output_full, HIGH);
    }   
    
    //reset interval
    Ring[ring].Interval = 20;       

    Ring[ring].ActivePattern = FADE;    
    Ring[ring].Color1 = Ring[ring].Color(255,0,0);  
    Ring[ring].Color2 = Ring[ring].Color(0,255,0);  
    Ring[ring].TotalSteps = 100;

  break;
    
  case ANIMATION_BUTTON_KEYUP:       
    
    digitalWrite(output_half, LOW);
    if( ring == RING_FULL_ID ){
      digitalWrite(output_full, LOW);
    }  
    
    //reset interval
    Ring[ring].Interval = 20;       

    Ring[ring].ActivePattern = FADE;    
    Ring[ring].Color1 = Ring[ring].Color(0,255,0);  
    Ring[ring].Color2 = Ring[ring].Color(0,0,255);  
    Ring[ring].TotalSteps = 100;

    break;
    
  case ANIMATION_BUTTON_IDLE:
      
      Ring[ring].Interval = 10;  
      
      Ring[ring].ActivePattern = FADE;    
      Ring[ring].Color1 = Ring[ring].Color(0,0,255);  
      Ring[ring].Color2 = Ring[ring].Color(0,0,255);     
   
      buttonPushedFull = false;   
      buttonPushedHalf = false;  

    break;

  case ANIMATION_BUTTON_FAIL:

    //reset interval
    Ring[ring].Interval = 10;     
    
    Ring[ring].ActivePattern = FADE;    
    Ring[ring].Color1 = Ring[ring].Color(0,255,0); 
    Ring[ring].Color2 = Ring[ring].Color(0,255,0);  

    break;

  }

  // Update the rings.
  Ring[ring].Update(); 
}

// Initialize everything and prepare to start
void setup()
{
  Serial.begin(9600);
  
  pinMode(output_full, OUTPUT);
  pinMode(output_half, OUTPUT);

  currentFullAnimation = ANIMATION_START;   
  currentHalfAnimation = ANIMATION_START;  

  // Initialize all the pixelStrips
  Ring[RING_FULL_ID].begin();
  Ring[RING_HALF_ID].begin();

  // Kick off a pattern
  Ring[RING_FULL_ID].Direction = FORWARD;
  //Ring[RING_HALF_ID].Direction = REVERSE;    
  
  // Push animation to ring
  Animate(currentFullAnimation,RING_FULL_ID);
  Animate(currentHalfAnimation,RING_HALF_ID);
}

// Main loop
void loop()
{  
  input_full = 0;
  input_half = 0;

  // only sense if not in animation
  if( currentFullAnimation == ANIMATION_BUTTON_IDLE && currentHalfAnimation == ANIMATION_BUTTON_IDLE && !buttonPushedHalf && !buttonPushedFull ){

    input_full = capativeSensorFull.capacitiveSensor(20);
    input_half = capativeSensorHalf.capacitiveSensor(20);

    if(false){
      Serial.print(input_full);
      Serial.print("\t");
      Serial.println(input_half);
    }

    if( input_half > SENSOR_THRESHOLD ){
      //Serial.println("Push half");    
      buttonPushedHalf = true;   
    }

    if( input_full > SENSOR_THRESHOLD ){      
      //Serial.println("Push full");    
      buttonPushedFull = true; 
    }

    if( buttonPushedHalf ){      
      currentHalfAnimation = ANIMATION_BUTTON_KEYDOWN;   
    }

    if( buttonPushedFull ){          
      currentFullAnimation = ANIMATION_BUTTON_KEYDOWN;
    } 

    if( buttonPushedHalf && buttonPushedFull ){      
      currentHalfAnimation = ANIMATION_BUTTON_FAIL;        
      currentFullAnimation = ANIMATION_BUTTON_FAIL;
    }
  }

    if(false){
      Serial.print(currentHalfAnimation);
      Serial.print("\t");
      Serial.println(currentFullAnimation);
    }
    
  // Push animation to ring
  Animate(currentHalfAnimation,RING_FULL_ID);
  Animate(currentFullAnimation,RING_HALF_ID);              
}

