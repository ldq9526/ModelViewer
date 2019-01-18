#include "Model.h"
#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/postprocess.h>
#include <opencv2/opencv.hpp>
#define __DEBUG__

void Model::loadTextures(const aiScene *scene, const std::string &path) {
	_textureNum = scene->mNumMaterials;
	_textures = new GLuint[_textureNum];
	_colors = new aiColor3D[_textureNum];
	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		const aiMaterial *material = scene->mMaterials[i];
		_colors[i] = aiColor3D(.5f, .5f, .5f);
		material->Get(AI_MATKEY_COLOR_DIFFUSE, _colors[i]);
		unsigned int diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
		if (!diffuseCount) {
			_textures[i] = 0;
			continue;
		}
		aiString texturePath;
		if (AI_SUCCESS != material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath, NULL, NULL, NULL, NULL, NULL)) {
			_textures[i] = 0;
			continue;
		}

		std::string fullPath;
		size_t pos;
		pos = path.find_last_of("/\\");
		if (pos != std::string::npos)
			fullPath = path.substr(0, pos + 1) + texturePath.data;
		else
			fullPath = texturePath.data;

		cv::Mat textureImage = cv::imread(fullPath, cv::IMREAD_UNCHANGED);
		if (textureImage.empty()) {
			_textures[i] = 0;
			continue;
		}

		GLenum format;
		if (textureImage.channels() == 1)
			format = GL_RED;
		else if (textureImage.channels() == 3) {
			cv::cvtColor(textureImage, textureImage, cv::COLOR_BGR2RGB);
			format = GL_RGB;
		}
		else {
			cv::cvtColor(textureImage, textureImage, cv::COLOR_BGRA2RGBA);
			format = GL_RGBA;
		}
		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, textureImage.cols, textureImage.rows, 0, format, GL_UNSIGNED_BYTE, textureImage.data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		_textures[i] = textureId;
	}
}

Model::Model(const std::string &path) {
	/* default value is important because initializer may be interrupted */
	_exist = false;
	_errorInfo = "";
	_textureNum = _vertexNum = 0;
	_textures = nullptr;
	_colors = nullptr;
	_center = glm::vec3(0.f, 0.f, 0.f);
	_maxDistance = 0.f;

	Assimp::Importer importer;
	unsigned int flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_RemoveRedundantMaterials;
	const aiScene *scene = importer.ReadFile(path, flags);/* aiProcessPreset_TargetRealtime_Quality */
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		_errorInfo = importer.GetErrorString();
		return;
	}

#ifdef __DEBUG__
	std::cout << "total " << scene->mNumMeshes << " meshes :" << std::endl;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh * mesh = scene->mMeshes[i];
		std::cout << "mesh[" << i << "] : " << mesh->mName.data <<" " <<mesh->mNumFaces << " faces, " << mesh->mNumVertices << " vertices, material[" << mesh->mMaterialIndex << "] ." << std::endl;
	}
	std::cout << "materials :" << std::endl;
	aiString texturePath;
	for (unsigned int i = 0; i < scene->mNumMaterials; i++) {
		const aiMaterial *material = scene->mMaterials[i];
		std::cout << "material[" << i << "] : diffuse[ ";
		unsigned int diffuseCount = material->GetTextureCount(aiTextureType_DIFFUSE);
		for (unsigned int j = 0; j < diffuseCount; j++)
			if (AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, j, &texturePath, NULL, NULL, NULL, NULL, NULL))
				std::cout << texturePath.data << " ";
		std::cout << "], specular[ ";

		unsigned int specularCount = material->GetTextureCount(aiTextureType_SPECULAR);
		for (unsigned int j = 0; j < specularCount; j++)
			if (AI_SUCCESS == material->GetTexture(aiTextureType_SPECULAR, j, &texturePath, NULL, NULL, NULL, NULL, NULL))
				std::cout << texturePath.data << " ";
		std::cout << "], ambient[ ";

		unsigned int ambientCount = material->GetTextureCount(aiTextureType_AMBIENT);
		for (unsigned int j = 0; j < ambientCount; j++)
			if (AI_SUCCESS == material->GetTexture(aiTextureType_AMBIENT, j, &texturePath, NULL, NULL, NULL, NULL, NULL))
				std::cout << texturePath.data << " ";
		std::cout << "]";

		aiColor3D color(0.f, 0.f, 0.f);
		if (AI_SUCCESS == material->Get(AI_MATKEY_COLOR_DIFFUSE, color)) {
			std::cout << ", color[" << color.r << "," << color.g << "," << color.b << "]";
		}
		std::cout << std::endl;
	}

#endif // __DEBUG__

	/* load all textures */
	loadTextures(scene, path);

	/* compute model center */
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh * mesh = scene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
			_center[0] += mesh->mVertices[j].x;
			_center[1] += mesh->mVertices[j].y;
			_center[2] += mesh->mVertices[j].z;
		}
		_vertexNum += mesh->mNumVertices;
	}
	_center /= _vertexNum;

	/* compute max distance from vertex to model center */
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh *mesh = scene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumVertices; j++) {
			mesh->mVertices[j].x -= _center[0];
			mesh->mVertices[j].y -= _center[1];
			mesh->mVertices[j].z -= _center[2];
			GLfloat x = glm::abs(mesh->mVertices[j].x), y = glm::abs(mesh->mVertices[j].y), z = glm::abs(mesh->mVertices[j].z);
			_maxDistance = glm::max(_maxDistance, glm::max(x, glm::max(y, z)));
		}
	}

	/* load mesh data */
	GLfloat scale = 1.f / _maxDistance;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		const aiMesh * mesh = scene->mMeshes[i];
		for (unsigned int j = 0; j < mesh->mNumVertices; j++)
			mesh->mVertices[j] *= scale;
		Mesh * pMesh = new Mesh(mesh, _textures[mesh->mMaterialIndex], _colors[mesh->mMaterialIndex]);
		if (!pMesh->empty())
			_meshes.push_back(pMesh);
		else
			delete pMesh;
	}

	if (!_meshes.size())
		_errorInfo = "Invalid vertices data .";
	else
		_exist = true;
}

Model::~Model() {
	for (size_t i = 0; i < _meshes.size(); i++)
		delete _meshes[i];
	_meshes.clear();
	if (_textures != nullptr) {
		glBindTexture(GL_TEXTURE_2D, 0);
		for (size_t i = 0; i < _textureNum; i++)
			if (_textures[i])
				glDeleteTextures(1, _textures + i);
		delete[]_textures;
	}
	if (_colors != nullptr)
		delete[] _colors;
}

void Model::draw(GLuint programId) {
	for (size_t i = 0; i < _meshes.size(); i++)
		if (!_meshes[i]->empty())
			_meshes[i]->draw(programId);
}

const std::string & Model::getErrorInfo() const { return _errorInfo; }

bool Model::empty() const { return !_exist; }