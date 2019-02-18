#include "Model.h"
#include <GL/freeglut.h>
#include <glm/gtc/type_ptr.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

const char *vertexShaderSource = "\
#version 330 core\n\
layout(location = 0) in vec3 position;\
layout(location = 1) in vec3 color;\
layout(location = 2) in vec3 normal;\
layout(location = 3) in vec2 uv;\
out Vertex {\
	vec3 position;\
	vec3 color;\
	vec3 normal;\
	vec2 uv;\
} vertex;\
uniform mat4 positionMatrix;\
uniform mat4 projection;\
void main() {\
	vec4 vertexPosition = positionMatrix * vec4(position, 1.0);\
	vertex.position = vertexPosition.xyz;\
	vertex.color = color;\
	vertex.normal = normal;\
	vertex.uv = uv;\
	gl_Position = projection * vertexPosition;\
}";

const char *fragmentShaderSource = "\
#version 330 core\n\
in Vertex {\
	vec3 position;\
	vec3 color;\
	vec3 normal;\
	vec2 uv;\
} vertex;\
out vec4 Color;\
uniform mat3 normalMatrix;\
uniform sampler2D sampler;\
uniform bool haveTexture;\
uniform bool useLight;\
void main() {\
	vec3 color = haveTexture ? texture(sampler, vertex.uv).rgb : vertex.color;\
	if(useLight) {\
		vec3 lightPosition = vec3(0.0, 0.0, 2.0); \
		vec3 ambient = color * 0.05;\
		vec3 normal = normalize(normalMatrix * vertex.normal);\
		vec3 lightDirection = normalize(lightPosition - vertex.position);\
		vec3 diffuse = max(dot(lightDirection, normal), 0.0) * color;\
		Color = vec4(ambient + diffuse, 1.0);\
	} else\
		Color = vec4(color,1.0);\
}";

const GLfloat step = 0.02f;
GLuint vertexShaderId, fragmentShaderId, programId;
bool useLight = false;
glm::vec3 translation;
GLint angleX = 0, angleY = 0, angleZ = 0;
int windowWidth, windowHeight;
bool leftButtonDown = false;
int oldX = 0, oldY = 0;

bool initialize();
void clear();
bool check(GLuint id, GLenum id_type, GLenum target_type);
void reshape(int w, int h);
void display();
void keyboard(unsigned char key, int, int);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);

Model * model_ptr = nullptr;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Usage : command model_filename" << std::endl;
		return 0;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
	glutInitWindowSize(500, 500);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow("OpenGL");
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	if (glewInit()) {
		std::cout << "Fail to initialize GLEW ." << std::endl;
		return 0;
	}

	if (!initialize()) {
		std::cout << "Fail to initialize OpenGL context ." << std::endl;
		return 0;
	}

	translation = glm::vec3(0.f, 0.f, -2.f);

	Model model(argv[1]);
	if (model.empty()) {
		std::cout << model.getErrorInfo() << std::endl;
		return 0;
	}

	model_ptr = &model;

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);

	glutMainLoop();
	clear();
	return 0;
}

bool initialize() {
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
	if (!vertexShaderId) {
		std::cout << "Fail to create vertex_shader ." << std::endl;
		return false;
	}
	glShaderSource(vertexShaderId, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShaderId);
	if (!check(vertexShaderId, GL_VERTEX_SHADER, GL_COMPILE_STATUS))
		return false;

	fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
	if (!fragmentShaderId) {
		std::cout << "Fail to create fragment_shader ." << std::endl;
		return false;
	}
	glShaderSource(fragmentShaderId, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShaderId);
	if (!check(fragmentShaderId, GL_FRAGMENT_SHADER, GL_COMPILE_STATUS))
		return false;

	programId = glCreateProgram();
	if (!programId) {
		std::cout << "Fail to create program ." << std::endl;
		return false;
	}
	glAttachShader(programId, vertexShaderId);
	glAttachShader(programId, fragmentShaderId);
	glLinkProgram(programId);
	if (!check(programId, 0, GL_LINK_STATUS))
		return false;

	glDeleteShader(vertexShaderId);
	glDeleteShader(fragmentShaderId);

	glUseProgram(programId);
	return true;
}

bool check(GLuint id, GLenum id_type, GLenum target_type) {
	int success;
	char infoLog[1024];
	if (target_type == GL_COMPILE_STATUS) {/* check shaders compile */
		glGetShaderiv(id, target_type, &success);
		if (!success) {
			std::cout << "Fail to compile ";
			if (id_type == GL_VERTEX_SHADER)
				std::cout << "vertex_shader :" << std::endl;
			else if (id_type == GL_FRAGMENT_SHADER)
				std::cout << "fragment_shader :" << std::endl;
			else if (id_type == GL_GEOMETRY_SHADER)
				std::cout << "geometry_shader :" << std::endl;
			glGetShaderInfoLog(id, 1024, NULL, infoLog);
			std::cout << infoLog << std::endl;
			return false;
		}
	} else if (target_type == GL_LINK_STATUS) {/* check program link */
		glGetProgramiv(id, target_type, &success);
		if (!success) {
			std::cout << "Fail to link program :" << std::endl;
			glGetProgramInfoLog(id, 1024, NULL, infoLog);
			std::cout << infoLog << std::endl;
			return false;
		}
	}
	return true;
}

void reshape(int w, int h) {
	glViewport(0, 0, windowWidth = w, windowHeight = h);
}

void display() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 view = glm::translate(glm::mat4(1.f), translation);
	glm::mat4 model = glm::mat4(1.f);
	model = glm::rotate(model, glm::radians(1.f*angleX), glm::vec3(1.f, 0.f, 0.f));
	model = glm::rotate(model, glm::radians(1.f*angleY), glm::vec3(0.f, 1.f, 0.f));
	model = glm::rotate(model, glm::radians(1.f*angleZ), glm::vec3(0.f, 0.f, 1.f));

	glm::mat4 positionMatrix = view*model;
	glUniformMatrix4fv(glGetUniformLocation(programId, "positionMatrix"), 1, GL_FALSE, glm::value_ptr(positionMatrix));

	glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(model)));
	glUniformMatrix3fv(glGetUniformLocation(programId, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

	glm::mat4 projection = glm::perspective(glm::radians(45.f), 1.f*windowWidth / windowHeight, .1f, 1000000.f);
	glUniformMatrix4fv(glGetUniformLocation(programId, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform1i(glGetUniformLocation(programId, "useLight"), (int)useLight);

	model_ptr->draw(programId);

	glutSwapBuffers();
}

void keyboard(unsigned char key, int, int) {
	if (key == 'f' || key == 'F')
		translation.z -= step;
	else if (key == 'n' || key == 'N')
		translation.z += step;
	else if (key == 'w' || key == 'W')
		translation.y += step;
	else if (key == 's' || key == 'S')
		translation.y -= step;
	else if (key == 'a' || key == 'A')
		translation.x -= step;
	else if (key == 'd' || key == 'D')
		translation.x += step;
	else if (key == 'q' || key == 'Q')
		angleZ = (angleZ + 2) % 360;
	else if (key == 'e' || key == 'E')
		angleZ = (angleZ - 2) % 360;
	else if (key == 'r' || key == 'R') {
		useLight = false;
		translation = glm::vec3(0.f, 0.f, -2.f);
		angleX = angleY = angleZ = 0;
		oldX = oldY = 0;
	} else if (key == 'l' || key == 'L')
		useLight = !useLight;
	else if(key == 'p' || key == 'P') {
		cv::Mat image(windowHeight, windowWidth, CV_8UC4);
		glReadPixels(0, 0, windowWidth, windowHeight, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
		cv::flip(image, image, 0);
		cv::imwrite("preview.png", image);
	} else
		return;
	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		leftButtonDown = true;
		oldX = x;
		oldY = y;
	}
	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		leftButtonDown = false;
}

void motion(int x, int y) {
	int dx = x - oldX, dy = y - oldY;
	if(!dx && !dy) return;
	int adx = abs(dx), ady = abs(dy);
	if (leftButtonDown) {
		if (adx > ady)
			angleY = (angleY + 2 * dx / adx) % 360;
		else
			angleX = (angleX + 2 * dy / ady) % 360;
		oldX = x;
		oldY = y;
		glutPostRedisplay();
	}
}

void clear() {
	glDeleteProgram(programId);
}