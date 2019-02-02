#include "stdafx.hxx"

#include "palette.hxx"

//using namespace std;

#include "pfm.hxx"

#define FORMAT_ABGR

Palette::Palette()
{
    clearColors();
}

void Palette::clearColors()
{
    pos[0] = 0.0f;
    rgb[0] = 0;
    pos[1] = 1.0f;
    rgb[1] = 0xffffff;
    posCount = 2;
}

int Palette::parseColor(std::string& color0)
  {
    if (color0[0] == '#')
    {
      // Hex encoding. Skip '#' and parse the rest of the string as a hex number.
      return 0xff000000 ;//| Integer.parseInt(color0.substring(1), 16);
    }
    else
    {
      // RGB encoding in the 'r,g,b' format, where 0 <= r,g,b <= 255.
      int r=0, g=0, b=0;
      int sidx, eidx;
      std::string str;

//      sidx = 0;
//      eidx = color0.find(',');
//      str = color0.substr(sidx, eidx);
//      str = str.trim();
//      r = Integer.parseInt(str);
//      // Skip over first ',' char.
//      sidx = eidx;
//      eidx = color0.find(',', sidx + 1);
//      str = color0.substr(sidx + 1, eidx);
//      str = str.trim();
//      g = Integer.parseInt(str);
//      // Skip over second ',' char.
//      str = color0.substring(eidx + 1);
//      str = str.trim();
//      b = Integer.parseInt(str);
      return 0xff000000 | (r << 16) | (g << 8) | b;
    }
  }

  void Palette::setParams(std::string& position0, std::string& color0)
  {
    // Remove blank characters.
    //position0 = position0.trim();
    //color0 = color0.trim();

    float position = 0;//Float.parseFloat(position0);
    int r, g, b;

    setColor(position, parseColor(color0));
  }

  int Palette::setColor(float position, int r, int g, int b)
  {
    int argb = 0xff000000 | (r << 16) | (g << 8) | b;
    return setColor(position, argb);
  }

  int Palette::setColor(float position, int argb)
  {
    if(posCount + 2 >= MAX_COLORS) return -1;
    if(position < 0.f || position > 1.f) return -1;

    int idx = -1;
    //System.err.println(" p " + position + " r " + r + " g " + g + " b " + b);
#ifdef FORMAT_ABGR
    argb = (argb & 0xff00ff00) | ((argb >> 16) & 0xff) | ((argb << 16) & 0xff0000);
#endif

    for(int i = 0; i < posCount; i++)
    {
      if (position == pos[i])
      {
        pos[i] = position;
        rgb[i] = argb;
        return i;
      }
      else if(position < pos[i])
      {
        for (int k = posCount; k > i; k--)
        {
          pos[k] = pos[k - 1];
          rgb[k] = rgb[k - 1];
        }

        idx = i;
        pos[i] = position;
        rgb[i] = argb;

        posCount++;
        break;
      }
    }

    return idx;
  }

  bool Palette::removeColor(int idx)
  {
    if (idx <= 0 || idx >= posCount - 1)
    {
      return false;
    }

    posCount--;

    for (int k = idx; k < posCount; k++)
    {
      pos[k] = pos[k + 1];
      rgb[k] = rgb[k + 1];
    }

    return true;
  }

  int Palette::getColor(float f)
  {
    float d,s;

    if(f <= 0) return rgb[0];
    if(f >= 1) return rgb[posCount-1];

    for(int i = 0; i < posCount; i++)
    {
      if(f >= pos[i] && f < pos[i+1])
      {
        d = pos[i+1] - pos[i];
        s = (f-pos[i]) / d;
        int is = (int)(s * 256);

        int rb = rgb[i] & 0xff00ff;
        int g  = rgb[i] & 0x00ff00;
        rb += ((is * ((rgb[i+1] & 0xff00ff) - rb)) >> 8);
        g  += ((is * ((rgb[i+1] & 0x00ff00) - g)) >> 8);
        rb &= 0xff00ff;
        g  &= 0x00ff00;

        return 0xff000000 | rb | g;
      }
    }

    return 0xff000000;
  }

  // Returns an index into the pos[] array if it contains p2,
  // so that abs(p2 - p) <= maxDelta.
  // Returns -1 otherwise.
  // 0 <= p, maxDelta <= 1.
  int Palette::indexOfMatch(float p, float maxDelta)
  {
    if (p < 0.f || p > 1.f || maxDelta < 0.f || maxDelta > 1.f)
    {
      return -1;
    }

    // Put an initial value greater than 1.
    float minDelta = 2.f;
    float delta = 0.f;
    // The index for the match.
    int idx = -1;

    for (int k = 0; k < posCount; k++)
    {
      delta = fabs(p - pos[k]);

      if (delta < minDelta)
      {
        minDelta = delta;
        idx = k;
      }
    }

    if (minDelta <= maxDelta)
    {
      return idx;
    }

    return -1;
  }
