#include "recipies.h"

const Animal animals[] = {
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