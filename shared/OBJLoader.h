#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>

namespace OBJLoader
{
  // Structure used to store a material's properties
  struct Material
  {
    float Ka[4];  // Ambient color
    float Ke[4];  // Emissive color
    float Kd[4];  // Diffuse color
    float Ks[4];  // Specular color
    float Kn;     // Specular exponent

    std::string name; // Material's name
  };

  // Structure used to store a vertex
  struct Vertex
  {
    float position[3];
    float normal[3];
    float uv[2];
  };

  // Structure used to store a mesh data.
  // Each triplet of vertices forms a triangle.
  struct Mesh
  {
    Mesh() : materialID(0), name("") {}

    std::vector<Vertex> vertices;
    std::size_t  materialID;
    std::string   name;
  };

  // Class responsible for loading all the meshes included in an OBJ file
  class Loader
  {
  public:
    Loader();
    Loader(const std::string& filename);
    ~Loader();

    bool loadFile(const std::string& filename);
    bool isLoaded() const { return _isLoaded; }
    void unload();

    const std::vector<Mesh>& getMeshes() const { return _meshes; }
    const std::vector<Material>& getMaterials() const { return _materials; }

  private:
    void loadMtlFile(const std::string& filename);
    std::size_t findMaterial(const std::string& name);
    std::size_t getMesh(const std::string& name);

    std::vector<Mesh>     _meshes;
    std::vector<Material> _materials;

    bool                  _isLoaded;
  };
}

#endif // OBJLOADER_H
