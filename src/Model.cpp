#include "Model.h"

unsigned int TextureFromFile(const std::string& path, const std::string& directory);
unsigned int TextureFromAssImp(const aiTexture* aiTex);


Model::Model(const std::string& path)
{
	loadModel(path);
}

void Model::Draw(Shader& shader)
{
	for (Mesh& mesh : meshes)
		mesh.Draw(shader);
}

void Model::loadModel(const std::string& path)
{
	Assimp::Importer importer;
	//										convert all primitives into triangles
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout<< "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// process all meshes in current node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// recusive
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{	
		processNode(node->mChildren[i], scene);
	}

}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex>		  vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture>	  textures;

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vtx;

		vtx.Position = { 
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		};
		vtx.Normal = {
			mesh->mNormals[i].x,
			mesh->mNormals[i].y,
			mesh->mNormals[i].z
		};
		//			0: diffuse/specualr channel
		if (mesh->mTextureCoords[0])
		{
			vtx.TexCoords = {
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y,
			};
		}
		else vtx.TexCoords = glm::vec2(0.0);
		vertices.push_back(vtx);
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		std::vector<Texture> diffuseMaps = loadMaterialTextures(material,
			aiTextureType_DIFFUSE, "texture_diffuse", scene);
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		
		std::vector<Texture> specularMaps = loadMaterialTextures(material,
			aiTextureType_SPECULAR, "texture_specular", scene);
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}


	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName, const aiScene* scene)
{
	std::vector<Texture> textures;
	for (unsigned i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString texPath;
		mat->GetTexture(type, i, &texPath);
		auto cacheFind = textures_loaded.find(texPath.C_Str());
		if (cacheFind != textures_loaded.end())
		{
			//std::cout << "texture" + cacheFind->first + " cache hit" << std::endl;
			textures.push_back(cacheFind->second);
		}
		else
		{
			Texture texture;

			auto aitexture = scene->GetEmbeddedTexture(texPath.C_Str());

			// import embeded textures
			if (aitexture != nullptr) {
				//std::cout << "detect embeded texture" << std::endl;
				texture.id = TextureFromAssImp(aitexture);
			}
			else
			{
				texture.id = TextureFromFile(texPath.C_Str(), directory);
			}

			texture.type = typeName;
			texture.path = texPath.C_Str();
			textures.push_back(texture);

			textures_loaded[texPath.C_Str()] = texture;
		}

	}

	return textures;
}

// read textures from file and 
unsigned int TextureFromFile(const std::string& path, const std::string& directory) {
	std::string filename = directory + "/" + path;

	// generate texture
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int texWidth, texHeight, nrChannels;
	unsigned char* texData = stbi_load(filename.c_str(), &texWidth, &texHeight, &nrChannels, 0);
	if (texData)
	{
		GLenum format = GL_RGB;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;
		//std::cout << nrChannels << std::endl;
		glBindTexture(GL_TEXTURE_2D, textureID);
		// store (copy) texure data in bound texture object
		glTexImage2D(GL_TEXTURE_2D, 0, format, texWidth, texHeight, 0, format, GL_UNSIGNED_BYTE, texData);
		glGenerateMipmap(GL_TEXTURE_2D);

		// set warpping/filtering options (for current texture)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	else
	{
		std::cout << "Failed to load texture " << filename << std::endl;
	}
	// free data pointer
	stbi_image_free(texData);
	return textureID;
}

unsigned int TextureFromAssImp(const aiTexture* aiTex)
{
	unsigned int textureID = 0;
	glGenTextures(1, &textureID);

	int width, height, nrChannels;
	unsigned char* texData = nullptr;
	if (aiTex->mHeight == 0)
	{
		texData = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTex->pcData), aiTex->mWidth, &width, &height, &nrChannels, 0);
	}
	else
	{
		texData = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTex->pcData), aiTex->mWidth * aiTex->mHeight, &width, &height, &nrChannels, 0);
	}

	if (texData != nullptr)
	{
		GLenum format = GL_RGB;
		if (nrChannels == 1)
			format = GL_RED;
		else if (nrChannels == 3)
			format = GL_RGB;
		else if (nrChannels == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texData);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
	else
	{
		std::cout << "Failed to load texture " << aiTex << std::endl;
	}
	return textureID;
}