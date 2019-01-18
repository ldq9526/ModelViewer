#include "Mesh.h"

void Mesh::initialize() {
	glGenVertexArrays(1, &_VAO_ID);
	glGenBuffers(1, &_VBO_ID);
	
	glBindVertexArray(_VAO_ID);
	
	glBindBuffer(GL_ARRAY_BUFFER, _VBO_ID);
	glBufferData(GL_ARRAY_BUFFER, 11 * _vertexNum * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * _vertexNum * sizeof(GLfloat), _vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * _vertexNum * sizeof(GLfloat), 3 * _vertexNum * sizeof(GLfloat), _colors);
	glBufferSubData(GL_ARRAY_BUFFER, 6 * _vertexNum * sizeof(GLfloat), 3 * _vertexNum * sizeof(GLfloat), _normals);
	glBufferSubData(GL_ARRAY_BUFFER, 9 * _vertexNum * sizeof(GLfloat), 2 * _vertexNum * sizeof(GLfloat), _texCoords);

	/* vertex positions */
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	/* vertex colors */
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)(3 * _vertexNum * sizeof(GLfloat)));
	/* vertex normals */
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(6 * _vertexNum * sizeof(GLfloat)));
	/* vertex texture coords */
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)(9 * _vertexNum * sizeof(GLfloat)));

	glBindVertexArray(0);
}

Mesh::Mesh(const aiMesh * mesh, GLuint texture, const aiColor3D & color) {
	/* default value is important because initializer may be interrupted */
	_exist = _haveTexture = false;
	_vertexNum = 0;
	_texture = 0;
	_vertices = _colors = _normals = _texCoords = nullptr;
	_VAO_ID = _VBO_ID = 0;

	if (mesh->mNumVertices != 3 * mesh->mNumFaces) return;
	_texture = texture;
	_haveTexture = (_texture > 0);
	_vertexNum = mesh->mNumVertices;

	_vertices = new GLfloat[_vertexNum * 3];
	if (_vertices == nullptr)
		return;
	_normals = new GLfloat[_vertexNum * 3];
	if (_normals == nullptr)
		return;
	_texCoords = new GLfloat[_vertexNum * 2];
	if (_texCoords == nullptr)
		return;
	_colors = new GLfloat[_vertexNum * 3];
	if (_colors == nullptr)
		return;

	int k = 0;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		const aiFace face = mesh->mFaces[i];
		/* each face has 3 vertices */
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			_vertices[3 * k] = mesh->mVertices[face.mIndices[j]].x;
			_vertices[3 * k + 1] = mesh->mVertices[face.mIndices[j]].y;
			_vertices[3 * k + 2] = mesh->mVertices[face.mIndices[j]].z;
			_normals[3 * k] = mesh->mNormals[face.mIndices[j]].x;
			_normals[3 * k + 1] = mesh->mNormals[face.mIndices[j]].y;
			_normals[3 * k + 2] = mesh->mNormals[face.mIndices[j]].z;
			if (!_haveTexture) {
				_colors[3 * k] = color.r;
				_colors[3 * k + 1] = color.g;
				_colors[3 * k + 2] = color.b;
				++k;
				continue;
			}
			if (mesh->mTextureCoords[0]) {
				_texCoords[2 * k] = mesh->mTextureCoords[0][face.mIndices[j]].x;
				_texCoords[2 * k + 1] = mesh->mTextureCoords[0][face.mIndices[j]].y;
			} else
				_texCoords[2 * k] = _texCoords[2 * k + 1] = 0.f;
			++k;
		}
	}
	initialize();
	_exist = true;
}


Mesh::~Mesh() {
	if(_vertices != nullptr)
		delete[] _vertices;
	if (_colors != nullptr)
		delete[] _colors;
	if (_normals != nullptr)
		delete[] _normals;
	if (_texCoords != nullptr)
		delete[] _texCoords;
	if (_VAO_ID) {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &_VAO_ID);
	}
	if (_VBO_ID) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &_VBO_ID);
	}
}

void Mesh::draw(GLuint programId) {
	glUniform1i(glGetUniformLocation(programId, "haveTexture"), (int)_haveTexture);
	glBindVertexArray(_VAO_ID);
	glBindTexture(GL_TEXTURE_2D, _texture);
	glDrawArrays(GL_TRIANGLES, 0, _vertexNum);
	glBindVertexArray(0);
}

bool Mesh::empty() const {
	return !_exist;
}