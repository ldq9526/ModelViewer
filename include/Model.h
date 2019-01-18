#ifndef MODEL_H
#define MODEL_H

#include "Mesh.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Model
{
private:
	bool _exist;
	std::string _errorInfo;
	std::vector<Mesh *> _meshes;
	GLsizei _textureNum;
	GLuint *_textures;
	aiColor3D *_colors;
	GLsizei _vertexNum;
	glm::vec3 _center;/* model center */
	GLfloat _maxDistance;/* max distance from vertex to model center */

	/* load textures */
	void loadTextures(const aiScene *scene, const std::string &path);

public:

	Model(const std::string &path);
	~Model();

	/* draw */
	void draw(GLuint programId);

	/* get error information */
	const std::string & getErrorInfo() const;

	/* succeed in loading or not */
	bool empty() const;
};

#endif