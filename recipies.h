#ifndef RECIPIES_H
#define RECIPIES_H
#include <arduino.h>

// Recommendations from ChefSteps:
//   http://www.chefsteps.com/activities/sous-vide-time-and-temperature-guide
struct Recipe {
  const char level[12] PROGMEM;
  const double temperature;
  const unsigned long ideal_minutes;
  const unsigned long last_call_minutes;
};

struct AnimalTypes {
  const char type[12] PROGMEM;
  const Recipe recipies[3] PROGMEM;
};

struct Animal {
  const char animal[12] PROGMEM;
  const AnimalTypes types[3] PROGMEM;
};

extern const Animal animals[];
#endif // RECIPIES_H
