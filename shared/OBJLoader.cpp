#include "OBJLoader.h"

#include <fstream>
#include <iostream>
#include <sstream>

using namespace OBJLoader;

namespace
{
  // 2D/3D point data structures
  struct Point3D
  {
    Point3D() : x(0), y(0), z(0) {}

    float x,y,z;
  };

  struct Point2D
  {
    Point2D() : x(0), y(0) {}

    float x,y;
  };

  // Extract path from a string
  std::string extractPath(const std::string& filepathname)
  {
    std::size_t pos = filepathname.find_last_of("/\\");

    if (pos == std::string::npos)
      return std::string(".");

    return filepathname.substr(0, pos);
  }
}

//--------------------------------------------------------------------------------------------------
// Constructors / Destructors
Loader::Loader()
  : _isLoaded(false)
{}

Loader::Loader(const std::string& filename)
  : _isLoaded(false)
{
  loadFile(filename);
}

Loader::~Loader()
{}

//--------------------------------------------------------------------------------------------------
// Load file
bool Loader::loadFile(const std::string& filename)
{
  // Clear current data
  unload();

  // Open the input file
  std::ifstream file(filename.c_str(), std::ifstream::in);
  if (!file.is_open())
  {
    std::cout << "Error: Failed to open file " << filename << " for reading!" << std::endl;
    return false;
  }

  // Extract path. It will be useful later when loading the mtl file
  std::string path = extractPath(filename);

  // Create the default material
  Material defaultMat;
  defaultMat.Ka[0] = 1.0; defaultMat.Ka[1] = 1.0; defaultMat.Ka[2] = 1.0; defaultMat.Ka[3] = 1.0;
  defaultMat.Ke[0] = 0.0; defaultMat.Ke[1] = 0.0; defaultMat.Ke[2] = 0.0; defaultMat.Ke[3] = 1.0;
  defaultMat.Kd[0] = 1.0; defaultMat.Kd[1] = 1.0; defaultMat.Kd[2] = 1.0; defaultMat.Kd[3] = 1.0;
  defaultMat.Ks[0] = 1.0; defaultMat.Ks[1] = 1.0; defaultMat.Ks[2] = 1.0; defaultMat.Ks[3] = 1.0;
  defaultMat.Kn = 128;
  defaultMat.name = "(Default)";
  _materials.push_back(defaultMat);


  std::size_t currentMaterial = 0;

  // Create default mesh (default group)
  Mesh defaultMesh;
  _meshes.push_back(defaultMesh);

  std::size_t currentMesh = 0;

  // Create vertices' position, normal, and uv lists with default values
  std::vector<Point3D> vertices(1);
  std::vector<Point3D> normals(1);
  std::vector<Point2D> uvs(1);

  // Read file
  std::string line;
  while (std::getline(file, line))
  {
    if (line[0] == '#')
    {
      // Comments... just ignore the line
      continue;
    }
    else if (line[0] == 'v' && line[1] == ' ')
    {
      // Vertex! Add it to the list.
      Point3D v;
      std::stringstream ss(line.substr(2));
      ss >> v.x >> v.y >> v.z;
      vertices.push_back(v);

    }
    else if (line[0] == 'v' && line[1] == 'n')
    {
      // Normal! Add it to the list.
      Point3D n;
      std::stringstream ss(line.substr(3));
      ss >> n.x >> n.y >> n.z;
      normals.push_back(n);
    }
    else if (line[0] == 'v' && line[1] == 't')
    {
      // Tex coord! Add it to the list
      Point2D uv;
      std::stringstream ss(line.substr(3));
      ss >> uv.x >> uv.y;
      uvs.push_back(uv);
    }
    else if (line[0] == 'u')
    {
      // usemtl! First, get the material's name
      std::string name;
      std::string dummy;
      std::stringstream ss(line);
      ss >> dummy >> name;

      // Find it, and attach it to the current mesh
      currentMaterial = findMaterial(name);
      _meshes[currentMesh].materialID = currentMaterial;
    }
    else if (line[0] == 'g')
    {
      // Group! Set it as the current mesh
      std::string dummy;
      std::string name;
      std::stringstream ss(line);
      ss >> dummy >> name;

      currentMesh = getMesh(name);
      _meshes[currentMesh].materialID = currentMaterial;
    }
    else if (line[0] == 'f')
    {
      // Face! First, get its vertices data
      std::string vertexData;
      std::string dummy;
      std::stringstream ssLine(line.substr(2));
      std::vector<unsigned int> vertexIDs;
      std::vector<unsigned int> uvIDs;
      std::vector<unsigned int> normalIDs;
      while (std::getline(ssLine, vertexData, ' '))
      {
        std::size_t index = vertexIDs.size();
        vertexIDs.push_back(0);
        uvIDs.push_back(0);
        normalIDs.push_back(0);

        std::stringstream ss(vertexData);
        std::string stringVal;
        std::getline(ss, stringVal, '/');
        std::stringstream ss2(stringVal);
        ss2 >> vertexIDs[index];

        std::getline(ss, stringVal, '/');
        std::stringstream ss3(stringVal);
        ss3 >> uvIDs[index];

        std::getline(ss, stringVal, '/');
        std::stringstream ss4(stringVal);
        ss4 >> normalIDs[index];
      }

      // Create first triangle
      if (vertexIDs.size() < 3)
        continue;

      for (unsigned int i=0; i<3; ++i)
      {
        Vertex v;
        v.position[0] = vertices[vertexIDs[i]].x;
        v.position[1] = vertices[vertexIDs[i]].y;
        v.position[2] = vertices[vertexIDs[i]].z;

        v.normal[0] = normals[normalIDs[i]].x;
        v.normal[1] = normals[normalIDs[i]].y;
        v.normal[2] = normals[normalIDs[i]].z;

        v.uv[0] = uvs[uvIDs[i]].x;
        v.uv[1] = uvs[uvIDs[i]].y;

        _meshes[currentMesh].vertices.push_back(v);
      }

      // Create subsequent triangles (1 per additional vertices)
      // Note: These triangles are created using a triangle fan approach
      for (unsigned int i=3; i<vertexIDs.size(); ++i)
      {
        // First vertex of triangle is always the first vertex that has been specified
        Vertex v1;
        v1.position[0] = vertices[vertexIDs[0]].x;
        v1.position[1] = vertices[vertexIDs[0]].y;
        v1.position[2] = vertices[vertexIDs[0]].z;
        v1.normal[0] = normals[normalIDs[0]].x;
        v1.normal[1] = normals[normalIDs[0]].y;
        v1.normal[2] = normals[normalIDs[0]].z;
        v1.uv[0] = uvs[uvIDs[0]].x;
        v1.uv[1] = uvs[uvIDs[0]].y;

        // Second vertex is the previous vertex
        Vertex v2;
        v2.position[0] = vertices[vertexIDs[i-1]].x;
        v2.position[1] = vertices[vertexIDs[i-1]].y;
        v2.position[2] = vertices[vertexIDs[i-1]].z;
        v2.normal[0] = normals[normalIDs[i-1]].x;
        v2.normal[1] = normals[normalIDs[i-1]].y;
        v2.normal[2] = normals[normalIDs[i-1]].z;
        v2.uv[0] = uvs[uvIDs[i-1]].x;
        v2.uv[1] = uvs[uvIDs[i-1]].y;

        // Third vertex is the current vertex
        Vertex v3;
        v3.position[0] = vertices[vertexIDs[i]].x;
        v3.position[1] = vertices[vertexIDs[i]].y;
        v3.position[2] = vertices[vertexIDs[i]].z;
        v3.normal[0] = normals[normalIDs[i]].x;
        v3.normal[1] = normals[normalIDs[i]].y;
        v3.normal[2] = normals[normalIDs[i]].z;
        v3.uv[0] = uvs[uvIDs[i]].x;
        v3.uv[1] = uvs[uvIDs[i]].y;

        // Add the triangle
        _meshes[currentMesh].vertices.push_back(v1);
        _meshes[currentMesh].vertices.push_back(v2);
        _meshes[currentMesh].vertices.push_back(v3);
      }
    }
    else if (line[0] == 'm')
    {
      // Get file name
      std::string filename;
      std::string dummy;
      std::stringstream ss(line);
      ss >> dummy >> filename;

      // Add path to filename
      std::string pathname = path;
#ifdef Q_OS_WIN32
      pathname.append("\\");
#else
      pathname.append("/");
#endif
      pathname.append(filename);

      // Load file
      loadMtlFile(pathname);
    }
  }

  // Everything is loaded! Now remove empty meshes (this generally happens with the default group)
  std::vector<Mesh>::iterator it = _meshes.begin();
  while (it != _meshes.end())
  {
    if ((*it).vertices.size() == 0)
    {
      it = _meshes.erase(it);
    }
    else
    {
      ++it;
    }
  }


  // Close file
  file.close();
  _isLoaded = true;

  return true;
}

//--------------------------------------------------------------------------------------------------
// Load material file
void Loader::loadMtlFile(const std::string& filename)
{
  // Open the input file
  std::ifstream file(filename.c_str(), std::ifstream::in);
  if (!file.is_open())
  {
    std::cout << "Error: Failed to open material file " << filename << " for reading!" << std::endl;
    return;
  }

  // Read file
  std::string line;
  std::size_t currentMaterial = 0;
  while (std::getline(file, line))
  {
    if (line[0] == '#')
    {
      // Comments... just ignore this line
      continue;
    }
    else if (line[0] == 'n')
    {
      // newmtl! Create the new material
      Material newMtl;
      newMtl.Ka[0] = 0.0; newMtl.Ka[1] = 0.0; newMtl.Ka[2] = 0.0; newMtl.Ka[3] = 0.0;
      newMtl.Ke[0] = 0.0; newMtl.Ke[1] = 0.0; newMtl.Ke[2] = 0.0; newMtl.Ke[3] = 0.0;
      newMtl.Kd[0] = 0.0; newMtl.Kd[1] = 0.0; newMtl.Kd[2] = 0.0; newMtl.Kd[3] = 0.0;
      newMtl.Ks[0] = 0.0; newMtl.Ks[1] = 0.0; newMtl.Ks[2] = 0.0; newMtl.Ks[3] = 0.0;
      newMtl.Kn = 0;

      // Get its name
      std::string dummy;
      std::stringstream ss(line);
      ss >> dummy >> newMtl.name;

      // Add it to the list and set as current material
      currentMaterial = _materials.size();
      _materials.push_back(newMtl);
    }
    else if (line[0] == 'N')
    {
      // Shininess
      std::stringstream ss(line);
      std::string dummy;
      float shininess;
      ss >> dummy >> shininess;

      // TODO: The range convension is not enforced
      //    by modern OBJ exporter. 
      // Change from range [0, 1000] to range [0, 128]
      // shininess /= 1000.0;
      // shininess *= 128.0;

      _materials[currentMaterial].Kn = shininess;
    }
    else if (line[0] == 'K')
    {
      Material& mat = _materials[currentMaterial];
      std::string dummy;
      std::stringstream ss(line);

      if (line[1] == 'd')
      {
        // Diffuse coefficient
        ss >> dummy >> mat.Kd[0] >> mat.Kd[1] >> mat.Kd[2];
        mat.Kd[3] = 1.0f;
      }
      else if (line[1] == 's')
      {
        // Diffuse coefficient
        ss >> dummy >> mat.Ks[0] >> mat.Ks[1] >> mat.Ks[2];
        mat.Ks[3] = 1.0f;
      }
      else if (line[1] == 'a')
      {
        // Diffuse coefficient
        ss >> dummy >> mat.Ka[0] >> mat.Ka[1] >> mat.Ka[2];
        mat.Ka[3] = 1.0f;
      }
      else if (line[1] == 'e')
      {
        // Diffuse coefficient
        ss >> dummy >> mat.Ke[0] >> mat.Ke[1] >> mat.Ke[2];
        mat.Ke[3] = 1.0f;
      }
    }
  }

  // Close file
}

//--------------------------------------------------------------------------------------------------
// Find a material by its name
std::size_t Loader::findMaterial(const std::string& name)
{
  std::size_t id = 0;
  for (std::size_t i=0; i<_materials.size(); ++i)
  {
    const Material& mat = _materials[i];
    if (mat.name.compare(name) == 0)
    {
      id = i;
      break;
    }
  }

  return id;
}

//--------------------------------------------------------------------------------------------------
// Find a mesh by its name
std::size_t Loader::getMesh(const std::string& name)
{
  std::size_t id = 0;
  bool found = false;
  for (std::size_t i=0; i<_meshes.size(); ++i)
  {
    const Mesh& mesh = _meshes[i];
    if (mesh.name.compare(name) == 0)
    {
      id = i;
      found = true;
    }
  }

  if (!found)
  {
    Mesh newMesh;
    newMesh.name = name;

    id = _meshes.size();
    _meshes.push_back(newMesh);
  }

  return id;
}

//--------------------------------------------------------------------------------------------------
// Clear data
void Loader::unload()
{
  // Clear everything!
  _meshes.clear();
  _materials.clear();
  _isLoaded = false;
}
