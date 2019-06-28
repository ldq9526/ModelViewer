#include "Mesh.h"

void Mesh::initialize() {
	glGenVertexArrays(1, &_VAO_ID);
	glGenBuffers(1, &_VBO_ID);
	glGenBuffers(1, &_EBO_ID);
	
	glBindVertexArray(_VAO_ID);
	
	glBindBuffer(GL_ARRAY_BUFFER, _VBO_ID);
	glBufferData(GL_ARRAY_BUFFER, 11 * _vertexNum * sizeof(GLfloat), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 3 * _vertexNum * sizeof(GLfloat), _vertices);
	glBufferSubData(GL_ARRAY_BUFFER, 3 * _vertexNum * sizeof(GLfloat), 3 * _vertexNum * sizeof(GLfloat), _colors);
	glBufferSubData(GL_ARRAY_BUFFER, 6 * _vertexNum * sizeof(GLfloat), 3 * _vertexNum * sizeof(GLfloat), _normals);
	glBufferSubData(GL_ARRAY_BUFFER, 9 * _vertexNum * sizeof(GLfloat), 2 * _vertexNum * sizeof(GLfloat), _texCoords);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO_ID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * _faceNum * sizeof(GLuint), _indices, GL_STATIC_DRAW);

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
	_exist = false;
	_vertices = _colors = _normals = _texCoords = nullptr;
	_VAO_ID = _VBO_ID = _EBO_ID = 0;

	_texture = texture;
	_haveTexture = (_texture > 0);
	_vertexNum = mesh->mNumVertices;
	_faceNum = mesh->mNumFaces;

	try {
		_vertices = new GLfloat[_vertexNum * 3];
		_normals = new GLfloat[_vertexNum * 3];
		_texCoords = new GLfloat[_vertexNum * 2];
		_colors = new GLfloat[_vertexNum * 3];
		_indices = new GLuint[_faceNum * 3];
	} catch(std::bad_alloc) {
		return;
	}

	for(unsigned int i = 0; i < _vertexNum; i++) {
		_vertices[3 * i] = mesh->mVertices[i].x;
		_vertices[3 * i + 1] = mesh->mVertices[i].y;
		_vertices[3 * i + 2] = mesh->mVertices[i].z;
		_normals[3 * i] = mesh->mNormals[i].x;
		_normals[3 * i + 1] = mesh->mNormals[i].y;
		_normals[3 * i + 2] = mesh->mNormals[i].z;
		if (!_haveTexture) {
			_colors[3 * i] = color.r;
			_colors[3 * i + 1] = color.g;
			_colors[3 * i + 2] = color.b;
			continue;
		}
		if (mesh->mTextureCoords[0]) {
			_texCoords[2 * i] = mesh->mTextureCoords[0][i].x;
			_texCoords[2 * i + 1] = mesh->mTextureCoords[0][i].y;
		} else
			_texCoords[2 * i] = _texCoords[2 * i + 1] = 0.f;
	}

	int k = 0;
	for (unsigned int i = 0; i < _faceNum; i++) {
		const aiFace face = mesh->mFaces[i];
		/* each face has 3 vertices */
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			_indices[k] = face.mIndices[j];
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
	if (_indices != nullptr)
		delete[] _indices;
	if (_VAO_ID) {
		glBindVertexArray(0);
		glDeleteVertexArrays(1, &_VAO_ID);
	}
	if (_VBO_ID) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &_VBO_ID);
	}
	if (_EBO_ID) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glDeleteBuffers(1, &_EBO_ID);
	}
}

void Mesh::draw(GLuint programId) {
	glUniform1i(glGetUniformLocation(programId, "haveTexture"), (int)_haveTexture);
	glBindVertexArray(_VAO_ID);
	glBindTexture(GL_TEXTURE_2D, _texture);
	//glDrawArrays(GL_TRIANGLES, 0, _vertexNum);
	glDrawElements(GL_TRIANGLES, 3 * _faceNum, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

bool Mesh::empty() const {
	return !_exist;
}