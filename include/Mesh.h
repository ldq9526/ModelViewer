#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <assimp/scene.h>
#include <vector>

class Mesh {
private:
	bool _exist;
	bool _haveTexture;/* has texture or not */
	GLuint _VAO_ID, _VBO_ID;/* array and buffer object ids */
	unsigned int _vertexNum;/* vertices number */
	GLfloat *_vertices, *_colors, *_normals, *_texCoords;
	GLuint _texture;/* texture id */

	/* initialize array and buffer objects */
	void initialize();
public:
	Mesh(const aiMesh * mesh, GLuint texture, const aiColor3D & color);
	~Mesh();

	/* draw mesh */
	void draw(GLuint programId);

	/* data load completely or not */
	bool empty() const;
};

#endif