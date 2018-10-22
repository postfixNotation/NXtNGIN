#include <context.hpp>

Context::Context() {
	if (!glfwInit()) {
		std::cerr << "ERROR: COULD NOT START GLFW3" << std::endl;
	}
	if (!Init()) {
		std::cerr << "ERROR: COULD NOT INITIALIZE CONTEXT" << std::endl;
	}
}

Context::~Context() { Terminate(); }

void Context::Terminate() {
	glfwDestroyWindow(window_);
	window_ = nullptr;
	glfwTerminate();
}

bool Context::Init() {
	monitor_ = glfwGetPrimaryMonitor();
	UpdateVideoMode();

	width_ = video_mode_->width;
	height_ = video_mode_->height;
	ratio_ = static_cast<float>(width_) / height_;

	if (monitor_ && video_mode_) return true;
	return false;
}

void Context::UpdateVideoMode() { video_mode_ = const_cast<GLFWvidmode*>(glfwGetVideoMode(monitor_)); }

void Context::SetHints(
	OpenGLMajor major,
	OpenGLMinor minor,
	NumberOfSamples samples,
	bool resizable,
	bool debug) const {
	glfwWindowHint(GLFW_SAMPLES, samples);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_RED_BITS, video_mode_->redBits);
	glfwWindowHint(GLFW_GREEN_BITS, video_mode_->greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, video_mode_->blueBits);
	glfwWindowHint(GLFW_REFRESH_RATE, video_mode_->refreshRate);

	glfwWindowHint(GLFW_RESIZABLE, static_cast<size_t>(resizable));
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, static_cast<size_t>(debug));
}

void Context::SetCursorMode(ContextStates state) {
	if (state == ContextStates::ENABLED) {
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
	else if (state == ContextStates::DISABLED) {
		glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

void Context::Create(const std::string &title, ContextSize size) {
	switch (size) {
		case ContextSize::DEBUG:
			contextsize = size;
			window_ = glfwCreateWindow(
				width_ / 2,
				height_ / 2,
				title.c_str(),
				nullptr,
				nullptr
			);
			break;
		case ContextSize::MAXIMIZED:
			contextsize = size;
			window_ = glfwCreateWindow(
				width_,
				height_,
				title.c_str(),
				nullptr,
				nullptr
			);
			break;
		case ContextSize::FULLSCREEEN:
			contextsize = size;
			window_ = glfwCreateWindow(
				width_,
				height_,
				title.c_str(),
				monitor_,
				nullptr
			);
			break;
	}

	if (!window_) {
		std::cerr << "ERROR: COULD NOT OPEN WINDOW WITH GLFW3" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window_);
}

void Context::UpdateDimensions() {
	UpdateVideoMode();
	width_ = video_mode_->width;
	height_ = video_mode_->height;
	ratio_ = static_cast<float>(width_) / height_;
}

float Context::GetFrameRate(size_t precision) const {
	static size_t number_of_frames;
	static double time_elapsed;
	static double time_previous;
	static double fps;

	++number_of_frames;
	time_elapsed = glfwGetTime();

	if ((time_elapsed - time_previous) > 0.5) {
		fps = number_of_frames / (time_elapsed - time_previous);
		number_of_frames = 0;
		time_previous = time_elapsed;
	}
	fps = static_cast<int>(fps * pow(10, precision) + 0.5) / pow(10, precision);
	return static_cast<float>(fps);
}
