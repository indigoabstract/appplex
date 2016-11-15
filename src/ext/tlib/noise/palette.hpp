#pragma once

#include "pfm.hpp"

#include <string>

#define MAX_COLORS 100

class Palette
{
public:

  Palette();
  void clearColors();
  static int parseColor(std::string& color0);
  void setParams(std::string& position0, std::string& color0);
  int setColor(float position, int r, int g, int b);
  int setColor(float position, int argb);
  bool removeColor(int idx);
  int getColor(float f);


  // Returns an index into the pos[] array if it contains p2,
  // so that abs(p2 - p) <= maxDelta.
  // Returns -1 otherwise.
  // 0 <= p, maxDelta <= 1.
  int indexOfMatch(float p, float maxDelta);
  float pos[MAX_COLORS];
  int rgb[MAX_COLORS];
  int posCount;
  std::string name;
};
