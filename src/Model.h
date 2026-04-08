#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "stb_image.h"
#include "Mesh.h"
#include <vector>
#include <unordered_map>
#include <string>

class Model
{
public:
	Model(const std::string& path);
	void Draw(Shader& shader);

private:
	std::vector<Mesh> meshes;
	std::string directory;
	std::unordered_map<std::string, Texture> textures_loaded;

	void loadModel(const std::string& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
											  std::string typeName, const aiScene* scene);
};

