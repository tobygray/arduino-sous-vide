#include "recipes.h"
#include <avr/pgmspace.h>

// Recommendations from ChefSteps:
//   http://www.chefsteps.com/activities/sous-vide-time-and-temperature-guide
struct Recipe {
  const char level[RECIPE_NAME_MAX];
  const double temperature;
  const unsigned long ideal_minutes;
  const unsigned long last_call_minutes;
};

struct AnimalTypes {
  const char type[RECIPE_NAME_MAX];
  const Recipe recipies[3];
};

struct Animal {
  const char animal[RECIPE_NAME_MAX];
  const AnimalTypes types[3];
};


const PROGMEM Animal animals[] = {
  {"Beef", {
    {"Steak", {
      {"Rare",        54, 90, 180},
      {"Medium Rare", 58, 90, 180},
      {"Well Done",   70, 90, 180}}},
    {"Roast", {
      {"Rare",        56, 420, 960},
      {"Medium Rare", 60, 360, 840},
      {"Well Done",   70, 300, 660}}},
    {"Tough", {
      {"Rare",        58, 1440, 2880},
      {"Medium Rare", 65, 960, 1440},
      {"Well Done",   85, 480, 960}}}
    }
  },
  {"Pork", {
    {"Chop", {
      {"Rare",        58, 60, 150},
      {"Medium Rare", 62, 60, 105},
      {"Well Done",   70, 60,  90}}},
    {"Roast", {
      {"Rare",        58, 180, 330},
      {"Medium Rare", 62, 180, 240},
      {"Well Done",   70, 180, 210}}},
    {"Tough", {
      {"Rare",        62, 960, 1440},
      {"Medium Rare", 65, 720, 1440},
      {"Well Done",   85, 480, 960}}}
    }
  },
  {"Chicken", {
    {"Light Meat", {
      {"S.-Supple",   60, 120, 210},
      {"Juicy",       65, 60, 120},
      {"Well Done",   75, 60, 90}}},
    {"Dark Meat", {
      {"Tender",      65, 90, 270},
      {"Off bone",    75, 90, 180}}}
    }
  },
  {"Other", {
    {"Fish", {
      {"Tender",      40, 40, 60},
      {"Flaky",       58, 40, 60},
      {"Well Done",   60, 40, 60}}},
    {"Vegetables", {
      {"Green Veg.",  85, 5, 20},
      {"Squash",      85, 60, 180},
      {"Root Veg.",   85, 60, 180}}},
    {"Fruit", {
      {"Warm & Ripe",  68, 105, 150},
      {"Well Done",    85, 30, 90}}}
    }
  },
};


void recipes_get_animal_name(byte animal_idx, char* buffer)
{
  strcpy_P(buffer, animals[animal_idx].animal);
}

void recipes_get_type_name(byte animal_idx, byte type_idx, char* buffer)
{
  strcpy_P(buffer, animals[animal_idx].types[type_idx].type);
}

void recipes_get_level_name(byte animal_idx, byte type_idx, byte level_idx, char* buffer)
{
  strcpy_P(buffer, animals[animal_idx].types[type_idx].recipies[level_idx].level);
}

double recipes_get_temperature(byte animal_idx, byte type_idx, byte level_idx)
{
  return pgm_read_float((&(animals[animal_idx].types[type_idx].recipies[level_idx].temperature)));
}

unsigned long recipes_get_ideal_minutes(byte animal_idx, byte type_idx, byte level_idx)
{
  return pgm_read_dword((&(animals[animal_idx].types[type_idx].recipies[level_idx].ideal_minutes)));
}

unsigned long recipes_get_last_call_minutes(byte animal_idx, byte type_idx, byte level_idx)
{
  return pgm_read_dword((&(animals[animal_idx].types[type_idx].recipies[level_idx].last_call_minutes)));
}

bool recipes_is_valid(byte animal_idx, byte type_idx, byte level_idx)
{
  return recipes_get_temperature(animal_idx, type_idx, level_idx) != 0;
}

