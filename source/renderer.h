#pragma once

#include "Level.h"
// Used to print debug infomation from OpenGL, pulled straight from the official OpenGL wiki.
#ifndef NDEBUG
void MessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam) {
	if (GL_DEBUG_SEVERITY_NOTIFICATION == severity) return;
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
}
#endif

struct SCENE_DATA
{
	GW::MATH::GMATRIXF view_matrix;
	GW::MATH::GMATRIXF projection_matrix;
	GW::MATH::GVECTORF camera_pos;
};

struct MESH_DATA
{
	GW::MATH::GMATRIXF world_matrix;
	H2B::ATTRIBUTES material;
};

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GOpenGLSurface ogl;

	// what we need at a minimum to draw a triangle
	//GLuint vertex_array_object = 0;
	//GLuint vertex_buffer_object = 0;
	//GLuint index_buffer_object = 0;

	Level testLevel;

	MESH_DATA mesh_data;
	SCENE_DATA scene_data;

	GLuint vertex_shader = 0;
	GLuint fragment_shader = 0;
	GLuint shader_program = 0;
	GLuint mesh_data_buffer_object = 0;	// handle to uniform buffer for mesh data
	GLuint scene_data_buffer_object = 0; // handle to uniform buffer for scene data
	GLint mesh_data_index = 0;	// shader register index to the uniform buffer for mesh data
	GLint scene_data_index = 0; // shader register index to the uniform buffer for scene data

	//GLint vs_world_matrix_handle = 0;
	//GLint vs_view_matrix_handle = 0;
	//GLint vs_projection_matrix_handle = 0;

	GW::MATH::GMatrix proxy;
	GW::INPUT::GInput kbm_proxy;
	GW::INPUT::GController controller_proxy;

	GW::MATH::GMATRIXF view_matrix;
	GW::MATH::GMATRIXF camera_matrix;
	GW::MATH::GMATRIXF projection_matrix;

	GW::MATH::GMATRIXF right_handed =
	{
		-1.0f, 0.0f,  0.0f, 0.0f,
		 0.0f, 1.0f,  0.0f, 0.0f,
		 0.0f, 0.0f,  1.0f, 0.0f,
		 0.0f, 0.0f,  0.0f, 1.0f
	};


	// timing variables
	std::chrono::steady_clock::time_point last_update;

	// Load a shader file as a string of characters.
	std::string ShaderAsString(const char* shaderFilePath)
	{
		std::string output;
		unsigned int stringLength = 0;
		GW::SYSTEM::GFile file; file.Create();
		file.GetFileSize(shaderFilePath, stringLength);
		if (stringLength && +file.OpenBinaryRead(shaderFilePath)) {
			output.resize(stringLength);
			file.Read(&output[0], stringLength);
		}
		else
			std::cout << "ERROR: Shader Source File \"" << shaderFilePath << "\" Not Found!" << std::endl;
		return output;
	}

	void CreateLevelResources(Level& level)
	{
		for (int i = 0; i < level.models.size(); i++)
		{
			Model& current = level.models[i];
			glGenVertexArrays(1, &current.vertex_array_object);
			glBindVertexArray(current.vertex_array_object);
			{
				glGenBuffers(1, &current.vertex_buffer_object);
				glBindBuffer(GL_ARRAY_BUFFER, current.vertex_buffer_object);
				{
					glBufferData(GL_ARRAY_BUFFER, sizeof(H2B::VERTEX) * current.info.vertexCount, current.info.vertices.data(), GL_STATIC_DRAW);
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (void*)offsetof(H2B::VERTEX, pos));
					glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (void*)offsetof(H2B::VERTEX, uvw));
					glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(H2B::VERTEX), (void*)offsetof(H2B::VERTEX, nrm));
					glEnableVertexAttribArray(0);
					glEnableVertexAttribArray(1);
					glEnableVertexAttribArray(2);
				}

				glGenBuffers(1, &current.index_buffer_object);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, current.index_buffer_object);
				{
					glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * current.info.indexCount, current.info.indices.data(), GL_STATIC_DRAW);
				}
			}
			glBindVertexArray(0);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		}
	}

	void DrawModel(const Model& m)
	{
		glBindVertexArray(m.vertex_array_object);
		for (size_t j = 0; j < m.info.meshCount; j++)	// number of submeshes in current model
		{
			for (size_t k = 0; k < m.worldMatrix.size(); k++)	// number of instances of current model
			{
				MESH_DATA mesh_data =
				{
					(GW::MATH::GMATRIXF&)m.worldMatrix[k],
					m.info.materials[j].attrib
				};
				glBindBuffer(GL_UNIFORM_BUFFER, mesh_data_buffer_object);
				glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MESH_DATA), &mesh_data);
				glBindBuffer(GL_UNIFORM_BUFFER, 0);
				glDrawElements(GL_TRIANGLES, m.info.meshes[j].drawInfo.indexCount, GL_UNSIGNED_INT, (GLvoid*)(m.info.meshes[j].drawInfo.indexOffset * sizeof(GLuint)));
			}
		}
	}

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl)
	{
		win = _win;
		ogl = _ogl;

		proxy.Create();
		kbm_proxy.Create(win);
		controller_proxy.Create();

		// Link Needed OpenGL API functions
		LoadExtensions();
		// In debug mode we link openGL errors to the console
#ifndef NDEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
#endif

		FLOAT aspect_ratio = 0.0f;
		GW::MATH::GVECTORF eye = { 0.0f, 5.25f, -7.5f, 1.0f };
		GW::MATH::GVECTORF at = { 0.0f, 0.0f, 0.0f, 1.0f };
		GW::MATH::GVECTORF up = { 0.0f, 1.0f, 0.0f, 0.0f };
		proxy.LookAtLHF(eye, at, up, view_matrix);
		proxy.InverseF(view_matrix, camera_matrix);

		ogl.GetAspectRatio(aspect_ratio);
		proxy.ProjectionOpenGLLHF(G_DEGREE_TO_RADIAN_F(65.0f),
			aspect_ratio, 0.1f, 1000.0f,
			projection_matrix);

		{
			bool success = testLevel.LoadFromFile("../levels/Testing.txt");
			CreateLevelResources(testLevel);
			int debug = 0;
		}

		{
			scene_data.camera_pos = camera_matrix.row4;
			scene_data.view_matrix = view_matrix;
			scene_data.projection_matrix = projection_matrix;
		}

		{
			std::string vertexShaderSource = ShaderAsString("../shaders/vs.glsl");
			std::string fragmentShaderSource = ShaderAsString("../shaders/fs.glsl");

			// Create Vertex Shader
			const char* vssource = vertexShaderSource.c_str();
			vertex_shader = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex_shader, 1, &vssource, NULL);
			glCompileShader(vertex_shader);
			char errors[1024]; GLint result;
			glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &result);
			if (result == false)
			{
				glGetShaderInfoLog(vertex_shader, 1024, NULL, errors);
				std::cout << errors << std::endl;
			}
			// Create Fragment Shader
			const char* fssource = fragmentShaderSource.c_str();
			fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment_shader, 1, &fssource, NULL);
			glCompileShader(fragment_shader);
			glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &result);
			if (result == false)
			{
				glGetShaderInfoLog(fragment_shader, 1024, NULL, errors);
				std::cout << errors << std::endl;
			}
			// Create Executable Shader Program
			shader_program = glCreateProgram();
			glAttachShader(shader_program, vertex_shader);
			glAttachShader(shader_program, fragment_shader);
			glLinkProgram(shader_program);
			glGetProgramiv(shader_program, GL_LINK_STATUS, &result);
			if (result == false)
			{
				glGetProgramInfoLog(shader_program, 1024, NULL, errors);
				std::cout << errors << std::endl;
			}
			glUseProgram(shader_program);

			// Get handles to Vertex Shader uniforms (UBOs)
			glGenBuffers(1, &mesh_data_buffer_object);
			glBindBuffer(GL_UNIFORM_BUFFER, mesh_data_buffer_object);
			{
				mesh_data_index = glGetUniformBlockIndex(shader_program, "MESH_DATA");
				glBufferData(GL_UNIFORM_BUFFER, sizeof(MESH_DATA), nullptr, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, mesh_data_index, mesh_data_buffer_object);
			}
			glBindBuffer(GL_UNIFORM_BUFFER, 0);

			glGenBuffers(1, &scene_data_buffer_object);
			glBindBuffer(GL_UNIFORM_BUFFER, scene_data_buffer_object);
			{
				scene_data_index = glGetUniformBlockIndex(shader_program, "SCENE_DATA");
				glBufferData(GL_UNIFORM_BUFFER, sizeof(SCENE_DATA), &scene_data, GL_DYNAMIC_DRAW);
				glBindBufferBase(GL_UNIFORM_BUFFER, scene_data_index, scene_data_buffer_object);
			}
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}

		glUseProgram(0);
	}

	void Render()
	{
		// setup the pipeline
		glUseProgram(shader_program);
		glUniformBlockBinding(shader_program, mesh_data_index, 0);
		glUniformBlockBinding(shader_program, scene_data_index, 1);

		glBindBuffer(GL_UNIFORM_BUFFER, scene_data_buffer_object);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SCENE_DATA), &scene_data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		// now we can draw
		for (size_t i = 0; i < testLevel.models.size(); i++)	// number of models
		{
			const Model& current = testLevel.models[i];
			DrawModel(current);
		}

		glBindVertexArray(0);
		glUseProgram(0);
	}

	~Renderer()
	{
		// free resources
		for (size_t i = 0; i < testLevel.models.size(); i++)
		{
			Model& current = testLevel.models[i];
			glDeleteVertexArrays(1, &current.vertex_array_object);
			glDeleteBuffers(1, &current.vertex_buffer_object);
			glDeleteBuffers(1, &current.index_buffer_object);
		}

		// TODO: free resources (ubos, etc..)
		glDeleteBuffers(1, &mesh_data_buffer_object);
		glDeleteBuffers(1, &scene_data_buffer_object);

		glDeleteShader(vertex_shader);
		glDeleteShader(fragment_shader);
		glDeleteProgram(shader_program);
	}

	void Update()
	{
		auto now = std::chrono::steady_clock::now();
		float delta_time = std::chrono::duration_cast<std::chrono::microseconds>(now - last_update).count() / 100000.0f;
		last_update = now;

		HRESULT hr = E_NOTIMPL;
		GW::SYSTEM::UNIVERSAL_WINDOW_HANDLE wndHandle;
		win.GetWindowHandle(wndHandle);
		BOOL IsFocusWindow = GetFocus() == (HWND&)wndHandle;

		if (IsFocusWindow)
		{
			UpdateCamera(delta_time);
		}

		scene_data.camera_pos = camera_matrix.row4;
		scene_data.view_matrix = view_matrix;
		scene_data.projection_matrix = projection_matrix;

	}

	void UpdateCamera(float delta_time)
	{
		std::vector<FLOAT> kbmState(256);
		std::vector<FLOAT> controllerState(256);
		const FLOAT camera_speed = 0.15f;
		FLOAT per_frame_speed = camera_speed * delta_time;
		FLOAT thumb_speed = G_PI_F * delta_time * 0.05f;
		FLOAT aspect_ratio = 0.0f;
		UINT screen_width = 0;
		UINT screen_height = 0;
		FLOAT mouse_x_delta = 0.0f;
		FLOAT mouse_y_delta = 0.0f;

		win.GetWidth(screen_width);
		win.GetHeight(screen_height);
		ogl.GetAspectRatio(aspect_ratio);

		for (UINT i = 0; i < 256; i++)
		{
			kbm_proxy.GetState(i, kbmState[i]);
			controller_proxy.GetState(0, i, controllerState[i]);
		}

		GW::GReturn result = kbm_proxy.GetMouseDelta(mouse_x_delta, mouse_y_delta);
		bool move_up_changed = kbmState[G_KEY_SPACE] || controllerState[G_RIGHT_TRIGGER_AXIS];
		bool move_down_changed = kbmState[G_KEY_LEFTSHIFT] || controllerState[G_LEFT_TRIGGER_AXIS];
		bool move_forward_changed = kbmState[G_KEY_W] || controllerState[G_LY_AXIS];
		bool move_backward_changed = kbmState[G_KEY_S] || controllerState[G_LY_AXIS];
		bool move_left_changed = kbmState[G_KEY_A] || controllerState[G_LX_AXIS];
		bool move_right_changed = kbmState[G_KEY_D] || controllerState[G_LX_AXIS];
		bool mouse_r_button_pressed = kbmState[G_BUTTON_RIGHT];
		bool aim_up_down_changed = controllerState[G_RY_AXIS];
		bool aim_left_right_changed = controllerState[G_RX_AXIS];
		bool mouse_moved = G_PASS(result) && result != GW::GReturn::REDUNDANT && mouse_r_button_pressed;

		if (!mouse_moved)
		{
			mouse_x_delta = mouse_y_delta = 0.0f;
		}
		if (move_up_changed || move_down_changed)
		{
			FLOAT total_y_change = kbmState[G_KEY_SPACE] - kbmState[G_KEY_LEFTSHIFT] + controllerState[G_RIGHT_TRIGGER_AXIS] - controllerState[G_LEFT_TRIGGER_AXIS];
			camera_matrix.row4.y += total_y_change * per_frame_speed;
		}
		if (move_forward_changed || move_backward_changed || move_left_changed || move_right_changed)
		{
			FLOAT total_x_change = kbmState[G_KEY_D] - kbmState[G_KEY_A] + controllerState[G_LX_AXIS];
			FLOAT total_z_change = kbmState[G_KEY_W] - kbmState[G_KEY_S] + controllerState[G_LY_AXIS];
			GW::MATH::GVECTORF translation = { total_x_change * per_frame_speed, 0.0f, total_z_change * per_frame_speed, 1.0f };
			proxy.TranslateLocalF(camera_matrix, translation, camera_matrix);
		}
		if (mouse_moved || aim_up_down_changed)
		{
			FLOAT total_pitch = G_DEGREE_TO_RADIAN_F(65.0f) * (mouse_y_delta / static_cast<FLOAT>(screen_height)) + (controllerState[G_RY_AXIS] * thumb_speed);
			GW::MATH::GMATRIXF x_rotation = GW::MATH::GIdentityMatrixF;
			proxy.RotateXLocalF(x_rotation, total_pitch, x_rotation);
			proxy.MultiplyMatrixF(x_rotation, camera_matrix, camera_matrix);
		}
		if (mouse_moved || aim_left_right_changed)
		{
			FLOAT total_yaw = G_DEGREE_TO_RADIAN_F(65.0f) * aspect_ratio * (mouse_x_delta / static_cast<FLOAT>(screen_width)) + (controllerState[G_RX_AXIS] * thumb_speed);
			GW::MATH::GMATRIXF y_rotation = GW::MATH::GIdentityMatrixF;
			proxy.RotateYLocalF(y_rotation, total_yaw, y_rotation);
			GW::MATH::GVECTORF position = camera_matrix.row4;
			camera_matrix.row4 = { 0.0f, 0.0f, 0.0f, 1.0f };
			proxy.MultiplyMatrixF(camera_matrix, y_rotation, camera_matrix);
			camera_matrix.row4 = position;
		}

		// end of camera update
		proxy.InverseF(camera_matrix, view_matrix);
		proxy.ProjectionOpenGLLHF(G_DEGREE_TO_RADIAN_F(65.0f),
			aspect_ratio, 0.1f, 1000.0f,
			projection_matrix);
	}

private:
	// Modern OpenGL API Functions must be queried before use
	PFNGLCREATESHADERPROC				glCreateShader = nullptr;
	PFNGLSHADERSOURCEPROC				glShaderSource = nullptr;
	PFNGLCOMPILESHADERPROC				glCompileShader = nullptr;
	PFNGLGETSHADERIVPROC				glGetShaderiv = nullptr;
	PFNGLGETSHADERINFOLOGPROC			glGetShaderInfoLog = nullptr;
	PFNGLATTACHSHADERPROC				glAttachShader = nullptr;
	PFNGLDETACHSHADERPROC				glDetachShader = nullptr;
	PFNGLDELETESHADERPROC				glDeleteShader = nullptr;
	PFNGLCREATEPROGRAMPROC				glCreateProgram = nullptr;
	PFNGLLINKPROGRAMPROC				glLinkProgram = nullptr;
	PFNGLUSEPROGRAMPROC					glUseProgram = nullptr;
	PFNGLGETPROGRAMIVPROC				glGetProgramiv = nullptr;
	PFNGLGETPROGRAMINFOLOGPROC			glGetProgramInfoLog = nullptr;
	PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays = nullptr;
	PFNGLBINDVERTEXARRAYPROC			glBindVertexArray = nullptr;
	PFNGLGENBUFFERSPROC					glGenBuffers = nullptr;
	PFNGLBINDBUFFERPROC					glBindBuffer = nullptr;
	PFNGLBUFFERDATAPROC					glBufferData = nullptr;
	PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray = nullptr;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray = nullptr;
	PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer = nullptr;
	PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation = nullptr;
	PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv = nullptr;
	PFNGLDELETEBUFFERSPROC				glDeleteBuffers = nullptr;
	PFNGLDELETEPROGRAMPROC				glDeleteProgram = nullptr;
	PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays = nullptr;
	PFNGLDEBUGMESSAGECALLBACKPROC		glDebugMessageCallback = nullptr;

	// TODO: Part 2d
	PFNGLGETUNIFORMBLOCKINDEXPROC		glGetUniformBlockIndex = nullptr;
	PFNGLBINDBUFFERBASEPROC				glBindBufferBase = nullptr;
	PFNGLUNIFORMBLOCKBINDINGPROC		glUniformBlockBinding = nullptr;
	PFNGLMAPBUFFERPROC					glMapBuffer = nullptr;
	PFNGLUNMAPBUFFERPROC				glUnmapBuffer = nullptr;
	PFNGLBUFFERSUBDATAPROC				glBufferSubData = nullptr;		// alternative to map / unmap

	// Modern OpenGL API functions need to be queried
	void LoadExtensions()
	{
		ogl.QueryExtensionFunction(nullptr, "glCreateShader", (void**)&glCreateShader);
		ogl.QueryExtensionFunction(nullptr, "glShaderSource", (void**)&glShaderSource);
		ogl.QueryExtensionFunction(nullptr, "glCompileShader", (void**)&glCompileShader);
		ogl.QueryExtensionFunction(nullptr, "glGetShaderiv", (void**)&glGetShaderiv);
		ogl.QueryExtensionFunction(nullptr, "glGetShaderInfoLog", (void**)&glGetShaderInfoLog);
		ogl.QueryExtensionFunction(nullptr, "glAttachShader", (void**)&glAttachShader);
		ogl.QueryExtensionFunction(nullptr, "glDetachShader", (void**)&glDetachShader);
		ogl.QueryExtensionFunction(nullptr, "glDeleteShader", (void**)&glDeleteShader);
		ogl.QueryExtensionFunction(nullptr, "glCreateProgram", (void**)&glCreateProgram);
		ogl.QueryExtensionFunction(nullptr, "glLinkProgram", (void**)&glLinkProgram);
		ogl.QueryExtensionFunction(nullptr, "glUseProgram", (void**)&glUseProgram);
		ogl.QueryExtensionFunction(nullptr, "glGetProgramiv", (void**)&glGetProgramiv);
		ogl.QueryExtensionFunction(nullptr, "glGetProgramInfoLog", (void**)&glGetProgramInfoLog);
		ogl.QueryExtensionFunction(nullptr, "glGenVertexArrays", (void**)&glGenVertexArrays);
		ogl.QueryExtensionFunction(nullptr, "glBindVertexArray", (void**)&glBindVertexArray);
		ogl.QueryExtensionFunction(nullptr, "glGenBuffers", (void**)&glGenBuffers);
		ogl.QueryExtensionFunction(nullptr, "glBindBuffer", (void**)&glBindBuffer);
		ogl.QueryExtensionFunction(nullptr, "glBufferData", (void**)&glBufferData);
		ogl.QueryExtensionFunction(nullptr, "glEnableVertexAttribArray", (void**)&glEnableVertexAttribArray);
		ogl.QueryExtensionFunction(nullptr, "glDisableVertexAttribArray", (void**)&glDisableVertexAttribArray);
		ogl.QueryExtensionFunction(nullptr, "glVertexAttribPointer", (void**)&glVertexAttribPointer);
		ogl.QueryExtensionFunction(nullptr, "glGetUniformLocation", (void**)&glGetUniformLocation);
		ogl.QueryExtensionFunction(nullptr, "glUniformMatrix4fv", (void**)&glUniformMatrix4fv);
		ogl.QueryExtensionFunction(nullptr, "glDeleteBuffers", (void**)&glDeleteBuffers);
		ogl.QueryExtensionFunction(nullptr, "glDeleteProgram", (void**)&glDeleteProgram);
		ogl.QueryExtensionFunction(nullptr, "glDeleteVertexArrays", (void**)&glDeleteVertexArrays);
		ogl.QueryExtensionFunction(nullptr, "glDebugMessageCallback", (void**)&glDebugMessageCallback);
		
		// TODO: Part 2d
		ogl.QueryExtensionFunction(nullptr, "glGetUniformBlockIndex", (void**)&glGetUniformBlockIndex);
		ogl.QueryExtensionFunction(nullptr, "glBindBufferBase", (void**)&glBindBufferBase);
		ogl.QueryExtensionFunction(nullptr, "glUniformBlockBinding", (void**)&glUniformBlockBinding);
		ogl.QueryExtensionFunction(nullptr, "glMapBuffer", (void**)&glMapBuffer);
		ogl.QueryExtensionFunction(nullptr, "glUnmapBuffer", (void**)&glUnmapBuffer);
		ogl.QueryExtensionFunction(nullptr, "glBufferSubData", (void**)&glBufferSubData);

	}
};
