// Include libraries
#include "U8glib.h"

// Construct object
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NO_ACK);

// Declare pins
const int ldrPin{ A0 };             // Pin for LDR sensor; no pinMode needed in setup

// Declare Global Constants and Variables
const int length{ 3 };             // Number of values to store for averaging
int lastReadings[length]{ 0 };  // Array to store recent sensor values
int currentIndex{ 0 };              // Tracks the index for the next reading
int sum{ 0 };                       // Sum of the current readings to quickly compute mean
int index{ 0 };

int allReadings[124]{ 0 };      // Array to store data for the OLED display

void setup() {
  Serial.begin(9600);              // Initialize Serial Monitor
}

void loop() {
  int raw = analogRead(ldrPin);    // Read the sensor value
  int mean = updateMean();         // Calculate the mean of the last readings

  updateOled(mean);                // Update OLED with the mean value

  Serial.println(String(mean) + "   " + String(raw));  // Print the mean and raw reading
}

/**
 * updateMean - Updates the average (mean) of the most recent sensor readings.
 * 
 * This function maintains a rolling average of the last `length` readings
 * from the sensor. It works by using a circular buffer to store readings
 * in the `lastReadings` array without shifting elements. The `sum` variable 
 * keeps a running total of the current values in the array, allowing us to 
 * efficiently compute the mean without re-summing all elements each time.
 */
int updateMean() {
  // Subtract the oldest reading from the sum
  sum -= lastReadings[currentIndex];

  // Read the new sensor value and update the array and sum
  lastReadings[currentIndex] = analogRead(ldrPin);
  sum += lastReadings[currentIndex];

  // Move to the next index in a circular manner
  currentIndex = (currentIndex + 1) % length;

  // Calculate and return the mean value
  return sum / length;
}

/**
 * updateOled - Updates the OLED display with a rolling graph of the mean sensor values.
 *
 * This function shifts the contents of the `allReadings` array to the left, adds the 
 * newest mean value to the end, and then maps the values to display as pixels on the 
 * OLED screen in a scrolling manner.
 * 
 * param m The latest mean sensor reading to be displayed.
 */
void updateOled(int m) {
  u8g.setFont(u8g_font_unifont);

  if (index < 124)
  {
    allReadings[index] = m;
    ++index;
    return;
  }
  
  // Shift all readings one step to the left to make space for the new reading
  for (int i{0}; i < 4; ++i)
  {
    for (int j{ 0 }; j < 124; ++j)
    {
      allReadings[j] = allReadings[j+1];
    }
    allReadings[123] = m;
    updateMean();
  }
    
  // Prepp draw
  u8g.firstPage();
  do {
    // Plot the values on the OLED display
    u8g.drawLine(1, 64, 1, 0);
    u8g.drawLine(128, 63, 1, 63);
    for (int i{ 0 }; i < 123; ++i)
    {
      int point1 = map(allReadings[i], 0, 1000, 64, 0);
      int point2 = map(allReadings[i + 1], 0, 1000, 64, 0);
      u8g.drawLine(i, point1, i+1, point2);
    }
  } while (u8g.nextPage());
}