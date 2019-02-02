#include "stdafx.hxx"

#include "gfx-param-surface.hxx"
#include "pfm-def.h"
#include <glm/inc.hpp>
#include <list>
#include <fstream>
#include <assert.h>


#define Pi  glm::pi<float>()
#define TwoPi (2.f *  glm::pi<float>())

enum VertexFlags
{
   VertexFlagsNormals = 1 << 0,
   VertexFlagsTexCoords = 1 << 1,
};

struct ParametricInterval
{
   glm::ivec2 Divisions;
   glm::vec2 UpperBound;
   glm::vec2 TextureCount;
};

class ParametricSurface : public gfx_param_surface
{
public:
   int GetVertexCount() const;
   int GetLineIndexCount() const;
   int GetTriangleIndexCount() const;
   void GenerateVertices(std::vector<float>& vertices, unsigned char flags) const;
   void GenerateLineIndices(std::vector<unsigned short>& indices) const;
   void GenerateTriangleIndices(std::vector<unsigned short>& indices) const;
protected:
   void SetInterval(const ParametricInterval& interval);
   virtual glm::vec3 Evaluate(const glm::vec2& domain) const = 0;
   virtual bool InvertNormal(const glm::vec2& domain) const { return false; }
   virtual bool UseDomainCoords() const { return true; }
private:
   glm::vec2 ComputeDomain(float i, float j) const;
   glm::ivec2 m_slices;
   glm::ivec2 m_divisions;
   glm::vec2 m_upperBound;
   glm::vec2 m_textureCount;
};

inline void ParametricSurface::SetInterval(const ParametricInterval& interval)
{
   m_divisions = interval.Divisions;
   m_upperBound = interval.UpperBound;
   m_textureCount = interval.TextureCount;
   m_slices = m_divisions - glm::ivec2(1, 1);
}

inline int ParametricSurface::GetVertexCount() const
{
   return m_divisions.x * m_divisions.y;
}

inline int ParametricSurface::GetLineIndexCount() const
{
   return 4 * m_slices.x * m_slices.x;
}

inline int ParametricSurface::GetTriangleIndexCount() const
{
   return 6 * m_slices.x * m_slices.y;
}

inline glm::vec2 ParametricSurface::ComputeDomain(float x, float y) const
{
   return glm::vec2(x * m_upperBound.x / m_slices.x, y * m_upperBound.y / m_slices.y);
}

inline void ParametricSurface::GenerateVertices(std::vector<float>& vertices, unsigned char flags) const
{
   int floatsPerVertex = 3;
   if (flags & VertexFlagsNormals)
      floatsPerVertex += 3;
   if (flags & VertexFlagsTexCoords)
      floatsPerVertex += 2;

   vertices.resize(GetVertexCount() * floatsPerVertex);
   float* attribute = &vertices[0];

   for (int j = 0; j < m_divisions.y; j++) {
      for (int i = 0; i < m_divisions.x; i++) {

         // Compute position
         glm::vec2 domain = ComputeDomain(i, j);
         glm::vec3 range = Evaluate(domain);
         //attribute = range.Write(attribute);

         // Compute normal
         if (flags & VertexFlagsNormals) {
            float s = i, t = j;

            // Nudge the point if the normal is indeterminate.
            if (i == 0) s += 0.01f;
            if (i == m_divisions.x - 1) s -= 0.01f;
            if (j == 0) t += 0.01f;
            if (j == m_divisions.y - 1) t -= 0.01f;

            // Compute the tangents and their cross product.
            glm::vec3 p = Evaluate(ComputeDomain(s, t));
            glm::vec3 u = Evaluate(ComputeDomain(s + 0.01f, t)) - p;
            glm::vec3 v = Evaluate(ComputeDomain(s, t + 0.01f)) - p;
            glm::vec3 normal = glm::normalize(glm::cross(u, v));
            if (InvertNormal(domain))
               normal = -normal;
            //attribute = normal.Write(attribute);
         }

         // Compute Texture Coordinates
         if (flags & VertexFlagsTexCoords) {
            float s, t;
            if (UseDomainCoords()) {
               s = m_textureCount.x * i / m_slices.x;
               t = m_textureCount.y * j / m_slices.y;
            }
            else {
               s = 0.5 * range.x;
               t = 0.5 * range.z;
            }
            //attribute = glm::vec2(s, t).Write(attribute);
         }
      }
   }
}

inline void ParametricSurface::GenerateLineIndices(std::vector<unsigned short>& indices) const
{
   indices.resize(GetLineIndexCount());
   std::vector<unsigned short>::iterator index = indices.begin();
   for (int j = 0, vertex = 0; j < m_slices.y; j++) {
      for (int i = 0; i < m_slices.x; i++) {
         int next = (i + 1) % m_divisions.x;
         *index++ = vertex + i;
         *index++ = vertex + next;
         *index++ = vertex + i;
         *index++ = vertex + i + m_divisions.x;
      }
      vertex += m_divisions.x;
   }
}

inline void ParametricSurface::GenerateTriangleIndices(std::vector<unsigned short>& indices) const
{
   indices.resize(GetTriangleIndexCount());
   std::vector<unsigned short>::iterator index = indices.begin();
   for (int j = 0, vertex = 0; j < m_slices.y; j++) {
      for (int i = 0; i < m_slices.x; i++) {
         int next = (i + 1) % m_divisions.x;
         *index++ = vertex + i;
         *index++ = vertex + next;
         *index++ = vertex + i + m_divisions.x;
         *index++ = vertex + next;
         *index++ = vertex + next + m_divisions.x;
         *index++ = vertex + i + m_divisions.x;
      }
      vertex += m_divisions.x;
   }
}

class Quad : public ParametricSurface
{
public:
   Quad(float width, float height) : m_size(width, height)
   {
      ParametricInterval interval = { glm::ivec2(2, 2), glm::vec2(1, 1), glm::vec2(16, 16) };
      SetInterval(interval);
   }
   glm::vec3 Evaluate(const glm::vec2& domain) const
   {
      float x = (domain.x - 0.5) * m_size.x;
      float y = (domain.y - 0.5) * m_size.y;
      float z = 0;
      return glm::vec3(x, y, z);
   }
private:
   glm::vec2 m_size;
};

class Sphere : public ParametricSurface
{
public:
   Sphere(float radius) : m_radius(radius)
   {
      ParametricInterval interval = { glm::ivec2(20, 20), glm::vec2(Pi, TwoPi), glm::vec2(40, 70) };
      SetInterval(interval);
   }
   glm::vec3 Evaluate(const glm::vec2& domain) const
   {
      float u = domain.x, v = domain.y;
      float x = m_radius * glm::sin(u) * glm::cos(v);
      float y = m_radius * glm::cos(u);
      float z = m_radius * -glm::sin(u) * glm::sin(v);
      return glm::vec3(-y, x, z);
   }
private:
   float m_radius;
};

class Torus : public ParametricSurface
{
public:
   Torus(float majorRadius, float minorRadius) :
      m_majorRadius(majorRadius),
      m_minorRadius(minorRadius)
   {
      ParametricInterval interval = { glm::ivec2(20, 20), glm::vec2(TwoPi, TwoPi), glm::vec2(40, 10) };
      SetInterval(interval);
   }
   glm::vec3 Evaluate(const glm::vec2& domain) const
   {
      const float major = m_majorRadius;
      const float minor = m_minorRadius;
      float u = domain.x, v = domain.y;
      float x = (major + minor * glm::cos(v)) * glm::cos(u);
      float y = (major + minor * glm::cos(v)) * glm::sin(u);
      float z = minor * glm::sin(v);
      return glm::vec3(x, y, z);
   }
private:
   float m_majorRadius;
   float m_minorRadius;
};

class TrefoilKnot : public ParametricSurface
{
public:
   TrefoilKnot(float scale) : m_scale(scale)
   {
      ParametricInterval interval = { glm::ivec2(60, 15), glm::vec2(TwoPi, TwoPi), glm::vec2(100, 8) };
      SetInterval(interval);
   }
   glm::vec3 Evaluate(const glm::vec2& domain) const
   {
      const float a = 0.5f;
      const float b = 0.3f;
      const float c = 0.5f;
      const float d = 0.1f;
      float u = (TwoPi - domain.x) * 2;
      float v = domain.y;

      float r = a + b * glm::cos(1.5f * u);
      float x = r * glm::cos(u);
      float y = r * glm::sin(u);
      float z = c * glm::sin(1.5f * u);

      glm::vec3 dv;
      dv.x = -1.5f * b * glm::sin(1.5f * u) * glm::cos(u) -
         (a + b * glm::cos(1.5f * u)) * glm::sin(u);
      dv.y = -1.5f * b * glm::sin(1.5f * u) * glm::sin(u) +
         (a + b * glm::cos(1.5f * u)) * glm::cos(u);
      dv.z = 1.5f * c * glm::cos(1.5f * u);

      glm::vec3 q = glm::normalize(dv);
      glm::vec3 qvn = glm::normalize(glm::vec3(q.y, -q.x, 0));
      glm::vec3 ww = glm::cross(q, qvn);

      glm::vec3 range;
      range.x = x + d * (qvn.x * glm::cos(v) + ww.x * glm::sin(v));
      range.y = y + d * (qvn.y * glm::cos(v) + ww.y * glm::sin(v));
      range.z = z + d * ww.z * glm::sin(v);
      return range * m_scale;
   }
private:
   float m_scale;
};

class Cone : public ParametricSurface
{
public:
   Cone(float height, float radius) : m_height(height), m_radius(radius)
   {
      ParametricInterval interval = { glm::ivec2(20, 20), glm::vec2(TwoPi, 1), glm::vec2(1, 1) };
      SetInterval(interval);
   }
   glm::vec3 Evaluate(const glm::vec2& domain) const
   {
      float u = domain.x, v = domain.y;
      float x = m_radius * (1 - v) * glm::cos(u);
      float y = m_height * (v - 0.5f);
      float z = m_radius * (1 - v) * -glm::sin(u);
      return glm::vec3(x, y, z);
   }
   bool UseDomainCoords() const { return false; }
private:
   float m_height;
   float m_radius;
};

class MobiusStrip : public ParametricSurface
{
public:
   MobiusStrip(float scale) : m_scale(scale)
   {
      ParametricInterval interval = { glm::ivec2(40, 20), glm::vec2(TwoPi, TwoPi), glm::vec2(40, 15) };
      SetInterval(interval);
   }
   glm::vec3 Evaluate(const glm::vec2& domain) const
   {
      float u = domain.x;
      float t = domain.y;
      float major = 1.25;
      float a = 0.125f;
      float b = 0.5f;
      float phi = u / 2;

      // General equation for an ellipse where phi is the angle
      // between the major axis and the X axis.
      float x = a * glm::cos(t) * glm::cos(phi) - b * glm::sin(t) * glm::sin(phi);
      float y = a * glm::cos(t) * glm::sin(phi) + b * glm::sin(t) * glm::cos(phi);

      // Sweep the ellipse along a circle, like a torus.
      glm::vec3 range;
      range.x = (major + x) * glm::cos(u);
      range.y = (major + x) * glm::sin(u);
      range.z = y;
      return range * m_scale;
   }
private:
   float m_scale;
};

class KleinBottle : public ParametricSurface
{
public:
   KleinBottle(float scale) : m_scale(scale)
   {
      ParametricInterval interval = { glm::ivec2(20, 30), glm::vec2(TwoPi, TwoPi), glm::vec2(15, 50) };
      SetInterval(interval);
   }
   glm::vec3 Evaluate(const glm::vec2& domain) const
   {
      float v = 1 - domain.x;
      float u = domain.y;

      float x0 = 3 * glm::cos(u) * (1 + glm::sin(u)) +
         (2 * (1 - glm::cos(u) / 2)) * glm::cos(u) * glm::cos(v);

      float y0 = 8 * glm::sin(u) + (2 * (1 - glm::cos(u) / 2)) * glm::sin(u) * glm::cos(v);

      float x1 = 3 * glm::cos(u) * (1 + glm::sin(u)) +
         (2 * (1 - glm::cos(u) / 2)) * glm::cos(v + Pi);

      float y1 = 8 * glm::sin(u);

      glm::vec3 range;
      range.x = u < Pi ? x0 : x1;
      range.y = u < Pi ? -y0 : -y1;
      range.z = (-2 * (1 - glm::cos(u) / 2)) * glm::sin(v);
      return range * m_scale;
   }
   bool InvertNormal(const glm::vec2& domain) const
   {
      return domain.y > 3 * Pi / 2;
   }
private:
   float m_scale;
};

class ObjSurface : public gfx_param_surface
{
public:
   ObjSurface(const std::string& name);
   int GetVertexCount() const;
   int GetLineIndexCount() const { return 0; }
   int GetTriangleIndexCount() const;
   void GenerateVertices(std::vector<float>& vertices, unsigned char flags) const;
   void GenerateLineIndices(std::vector<unsigned short>& indices) const {}
   void GenerateTriangleIndices(std::vector<unsigned short>& indices) const;
private:
   std::string m_name;
   std::vector<glm::ivec3> m_faces;
   mutable size_t m_faceCount;
   mutable size_t m_vertexCount;
   static const int MaxLineSize = 128;
};

ObjSurface::ObjSurface(const std::string& name) :
   m_name(name),
   m_faceCount(0),
   m_vertexCount(0)
{
   m_faces.resize(GetTriangleIndexCount() / 3);
   std::ifstream objFile(m_name.c_str());
   std::vector<glm::ivec3>::iterator face = m_faces.begin();
   while (objFile) {
      char c = objFile.get();
      if (c == 'f') {
         assert(face != m_faces.end() && "parse error");
         objFile >> face->x >> face->y >> face->z;
         *face++ -= glm::ivec3(1, 1, 1);
      }
      objFile.ignore(MaxLineSize, '\n');
   }
   assert(face == m_faces.end() && "parse error");
}

int ObjSurface::GetVertexCount() const
{
   if (m_vertexCount != 0)
      return m_vertexCount;

   std::ifstream objFile(m_name.c_str());
   while (objFile) {
      char c = objFile.get();
      if (c == 'v')
         m_vertexCount++;
      objFile.ignore(MaxLineSize, '\n');
   }
   return m_vertexCount;
}

int ObjSurface::GetTriangleIndexCount() const
{
   if (m_faceCount != 0)
      return m_faceCount * 3;

   std::ifstream objFile(m_name.c_str());
   while (objFile) {
      char c = objFile.get();
      if (c == 'f')
         m_faceCount++;
      objFile.ignore(MaxLineSize, '\n');
   }
   return m_faceCount * 3;
}

void ObjSurface::GenerateVertices(std::vector<float>& floats, unsigned char flags) const
{
   assert(flags == VertexFlagsNormals && "Unsupported flags.");

   struct Vertex {
      glm::vec3 position;
      glm::vec3 normal;
   };

   // Read in the vertex positions and initialize lighting normals to (0, 0, 0).
   floats.resize(GetVertexCount() * 6);
   std::ifstream objFile(m_name.c_str());
   Vertex* vertex = (Vertex*)&floats[0];
   while (objFile) {
      char c = objFile.get();
      if (c == 'v') {
         vertex->normal = glm::vec3(0, 0, 0);
         glm::vec3& position = (vertex++)->position;
         objFile >> position.x >> position.y >> position.z;
      }
      objFile.ignore(MaxLineSize, '\n');
   }

   vertex = (Vertex*)&floats[0];
   for (size_t faceIndex = 0; faceIndex < m_faces.size(); ++faceIndex) {
      glm::ivec3 face = m_faces[faceIndex];

      // Compute the facet normal.
      glm::vec3 a = vertex[face.x].position;
      glm::vec3 b = vertex[face.y].position;
      glm::vec3 c = vertex[face.z].position;
      glm::vec3 facetNormal = glm::cross(b - a, c - a);

      // Add the facet normal to the lighting normal of each adjoining vertex.
      vertex[face.x].normal += facetNormal;
      vertex[face.y].normal += facetNormal;
      vertex[face.z].normal += facetNormal;
   }

   // Normalize the normals.
   for (int v = 0; v < GetVertexCount(); ++v)
      vertex[v].normal = glm::normalize(vertex[v].normal);
}

void ObjSurface::GenerateTriangleIndices(std::vector<unsigned short>& indices) const
{
   indices.resize(GetTriangleIndexCount());
   std::vector<unsigned short>::iterator index = indices.begin();
   for (std::vector<glm::ivec3>::const_iterator f = m_faces.begin(); f != m_faces.end(); ++f) {
      *index++ = f->x;
      *index++ = f->y;
      *index++ = f->z;
   }
}
