#ifndef MESH_HPP_
#define MESH_HPP_

#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>
#include <iostream>
#include <algorithm>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <shader.hpp>

constexpr size_t kVerticesPerQuad{ 6 };

struct Vertex {
	glm::vec3 position{};
	glm::vec3 normal{};
	glm::vec2 tex_coords{};
};

// this struct might be moved somewhere else
struct Color {
	GLfloat r;
	GLfloat g;
	GLfloat b;
	GLfloat a;
};

enum class ObjLoadingType {
	TRIANGLES = 3,
	QUADS = 4
};

enum class FaceType {
	V,
	VVN,
	VVTVN,
	NOT_DEFINED
};

// inherit from abstract class
class MeshRenderer {
private:
	ObjLoadingType obj_loading_type_;
	std::shared_ptr<Shader> shader_;
	std::vector<Vertex> vertices_;
	std::vector<GLuint> indices_;
	GLuint vbo_, vao_, ibo_;
	size_t number_quads_;
	bool loaded_;

	static FaceType EvalSplitRes(
		const std::vector<GLuint>& input_vec,
		const ObjLoadingType& olt = ObjLoadingType::QUADS
	);
	static std::vector<GLuint> Split(
		std::string value,
		char delimiter = '/'
	);
	void InitIBO();
	void InitBuffers();
public:
	MeshRenderer(std::shared_ptr<Shader>);
	~MeshRenderer();

	bool LoadObj(
		const std::string& filename,
		const ObjLoadingType& obj_loading_type = ObjLoadingType::QUADS
	);
	void Draw(std::shared_ptr<Shader> shader = std::shared_ptr<Shader>{nullptr}) const;
};

#endif //MESH_HPP_
