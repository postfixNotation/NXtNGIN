#include <text.hpp>

void Text::LoadFonts() {
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		std::cerr << "ERROR::FREETYPE: COULD NOT INIT FREETYPE LIBRARY" << std::endl;
	}
	FT_Face face;
	if (FT_New_Face(ft, filename_.c_str(), 0, &face)) {
		std::cerr << "ERROR::FREETYPE: FAILED TO LOAD FONT" << std::endl;
	}
	FT_Set_Pixel_Sizes(face, 0, default_pixel_size_);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// more than ASCII set to also get further glyphs
	for (GLubyte c = 0; c < 170; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			std::cerr << "ERROR::FREETYTPE: FAILED TO LOAD GLYPH" << std::endl;
			continue;
		}

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			GL_RED,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			0,
			GL_RED,
			GL_UNSIGNED_BYTE,
			face->glyph->bitmap.buffer
		);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			static_cast<GLuint>(face->glyph->advance.x)
		};
		characters_.insert(std::pair<GLchar, Character>(c, character));
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
}

void Text::InitBuffers() {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glGenVertexArrays(1, &vao_);
	glGenBuffers(1, &vbo_);
	glBindVertexArray(vao_);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(GLfloat) * kVerticesPerQuad * kPositionAndTexture,
		nullptr,
		GL_DYNAMIC_DRAW
	);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		kPositionAndTexture,
		GL_FLOAT,
		GL_FALSE,
		kPositionAndTexture * sizeof(GLfloat),
		0
	);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void Text::UseProjection() const {
	glUseProgram(shader_handle_);
	// change to shader method
	glUniformMatrix4fv(
		glGetUniformLocation(shader_handle_, "projection"),
		1,
		GL_FALSE,
		glm::value_ptr(projection_)
	);
}

Text::Text(GLuint shader_handle, size_t width, size_t height) {
	shader_handle_ = shader_handle;
	projection_ = glm::ortho(
		0.0f,
		static_cast<GLfloat>(width),
		0.0f,
		static_cast<GLfloat>(height)
	);
	InitBuffers();
}

Text::~Text() {
	for (std::map<GLchar, Character>::iterator it{begin(characters_)}; it != end(characters_); ++it) {
		glDeleteTextures(1, &(it->second.texture_id));
	}
}

void Text::SetFileName(std::string filename, size_t pixel_size) {
	default_pixel_size_ = pixel_size;
	filename_ = filename;
	LoadFonts();
}

void Text::RenderText(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) {
	UseProjection();
	glUniform3f(
		glGetUniformLocation(shader_handle_, "text_color"),
		color.x,
		color.y,
		color.z
	);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(vao_);

	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++) {
		Character ch = characters_[*c];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.character_size.y - ch.bearing.y) * scale;

		GLfloat w = ch.character_size.x * scale;
		GLfloat h = ch.character_size.y * scale;

		GLfloat vertices[kVerticesPerQuad][kPositionAndTexture] = {
			{ xpos,     ypos + h, 0.0f, 0.0f },
			{ xpos,     ypos,     0.0f, 1.0f },
			{ xpos + w, ypos,     1.0f, 1.0f },
			{ xpos,     ypos + h, 0.0f, 0.0f },
			{ xpos + w, ypos,     1.0f, 1.0f },
			{ xpos + w, ypos + h, 1.0f, 0.0f }
		};
		glBindTexture(GL_TEXTURE_2D, ch.texture_id);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_);
		glBufferSubData(
			GL_ARRAY_BUFFER,
			0,
			sizeof(vertices),
			vertices
		);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6); // add ibo -> glDrawElements(...)
		x += (ch.advance >> 6) * scale;
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
