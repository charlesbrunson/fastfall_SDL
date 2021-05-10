
#include <thread>
#include <iostream>

#include "imgui.h"
#include "backends/imgui_impl_sdl.h"
#include "backends/imgui_impl_opengl3.h"

#include "render/render.hpp"

#include "SDL_image.h"

int main(int argc, char* argv[])
{
	ff::init();


	ff::Window window{ "AAAAAAAAAAAA", 800, 600 };
	window.setWindowCentered();
	window.setWindowResizable();
	window.setVsyncEnabled();
	window.showWindow();
	window.setActive();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForOpenGL(window.getSDL_Window(), window.getSDLContext());
	ImGui_ImplOpenGL3_Init("#version 330");


	const ff::ShaderProgram* program = nullptr;
	try {
		program = &ff::ShaderProgram::getDefaultProgram();
	}
	catch (std::exception err) {
		std::cout << err.what();
		return EXIT_FAILURE;
	}

	std::chrono::system_clock clock;
	std::chrono::time_point start = clock.now();
	std::chrono::duration<float, std::ratio<1, 1>> elapsed;

	bool show_demo_window = false;
	ImVec4 clear_color = ImVec4(0.f, 0.f, 0.f, 1.00f);

	ff::Texture tex1{};
	tex1.loadFromFile("testimage1.png");
	assert(tex1.exists());

	ff::Texture tex2;
	tex2.loadFromFile("testimage2.bmp");
	assert(tex2.exists());


	ff::RenderTexture rtexture;
	rtexture.create(100, 100);
	rtexture.setDefaultView();
	rtexture.clear(ff::Color::Red);
	
	ff::Sprite obj1{ rtexture.getTexture(), 100.f, 100.f };
	obj1.setOrigin(50.f, 50.f);

	ff::Sprite obj2{ &tex2 };
	obj2.setPosition(200.f, -200.f);

	ff::ShapeLine line1{
		{50.f, 50.f},
		{-50.f, -50.f}
	};
	ff::ShapeLine line2{
		{-50.f, 50.f},
		{50.f, -50.f}
	};
	line1.setPosition(-150.f, -150.f);
	line2.setPosition(-150.f, -150.f);

	ff::ShapeRectangle rect1(ff::Rectf(100.f, 0.f, 100.f, 100.f), ff::Color::White().alpha(50), ff::Color::White);

	ff::ShapeCircle circle1{
		{0.f, 0.f},
		150.f,
		32u,
		ff::Color::Green().alpha(50),
		ff::Color::Red
	};
	circle1.setPosition(-150.f, 150.f);

	bool quit = false;
	while (!quit) {
		std::chrono::time_point now = clock.now();
		elapsed = (now - start);
		start = now;


		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_ESCAPE)
					quit = true;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
				case SDL_WINDOWEVENT_CLOSE:		quit = true; break;
				case SDL_WINDOWEVENT_RESIZED:
					ff::View view = window.getView();
					view.setViewport({ 0, 0, event.window.data1, event.window.data2 });
					window.setView(view);
					break;
				}
				break;
			case SDL_MOUSEBUTTONDOWN:
				if (event.button.windowID != window.getID())
					break;

				switch (event.button.button) {
				case SDL_BUTTON_LEFT:

					auto worldPos = window.windowCoordToWorld(glm::ivec2{ event.button.x, event.button.y });
					printf("%d, %d -> %f, %f\n",
						event.button.x, event.button.y,
						worldPos.x, worldPos.y
					);

					auto windowPos = window.worldCoordToWindow(worldPos);
					printf("window -> %d, %d\n",
						windowPos.x, windowPos.y
					);
					break;
				}
				break;
			}
		}

		rtexture.clear(ff::Color{ 100, 100, 100 });
		window.clear(ff::Color{ 10, 10, 10 });

		static glm::fvec2 circle1pos{ -10.f, -10.f };
		static glm::fvec2 circle2pos{ 25.f, 25.f };
		static glm::fvec2 circle3pos{ 25.f, -25.f };

		{
			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame(window.getSDL_Window());
			ImGui::NewFrame();

			if (show_demo_window)
				ImGui::ShowDemoWindow(&show_demo_window);


			// coordinate testing
			if (ImGui::Begin("Coordinate Testing")) {

				ff::View view = window.getView();
				glm::fvec2 view_pos = view.getCenter();
				glm::fvec2 view_size = view.getSize();
				float view_zoom = view.getZoom();

				if (ImGui::DragFloat2("View Position", (float*)&view_pos)) {
					view.setCenter(view_pos);
				}
				if (ImGui::DragFloat2("View Size", (float*)&view_size)) {
					view.setSize(view_size);
				}
				if (ImGui::DragFloat("View Zoom", (float*)&view_zoom, 0.05f, -100.f, 100.f)) {
					view.setZoom(view_zoom);
				}
				window.setView(view);

				glm::fvec2 pos = obj1.getPosition();
				glm::fvec2 origin = obj1.getOrigin();
				glm::fvec2 scale = obj1.getScale();
				float rot = glm::degrees(obj1.getRotation());

				if (ImGui::DragFloat2("Box Position", (float*)&pos)) {
					obj1.setPosition(pos);
				}
				if (ImGui::DragFloat2("Box Origin", (float*)&origin)) {
					obj1.setOrigin(origin);
				}
				if (ImGui::DragFloat2("Box Scale", (float*)&scale, 0.05f)) {
					obj1.setScale(scale);
				}
				if (ImGui::DragFloat("Box Rotation", &rot)) {
					obj1.setRotation(glm::radians(rot));
				}
				if (ImGui::Button("View Reset")) {
					window.setView(window.getDefaultView());
				}
				ImGui::SameLine();
				if (ImGui::Button("Box Reset")) {
					obj1.setPosition({ 0.f, 0.f });
					obj1.setOrigin({ 50.f, 50.f });
					obj1.setScale(1.f);
					obj1.setRotation(0.f);
				}
				ImGui::SameLine();
				static bool spin = false;
				ImGui::Checkbox("Spin", &spin);
				if (spin) {
					obj1.setRotation(obj1.getRotation() - elapsed.count());
				}
				static bool wireframe = false;
				if (ImGui::Checkbox("Wireframe", &wireframe)) {
					if (wireframe) {
						glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
					}
					else {
						glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					}
				}

				ImGui::Separator();

				ff::View rt_view = rtexture.getView();
				glm::fvec2 rt_view_pos = rt_view.getCenter();
				glm::fvec2 rt_view_size = rt_view.getSize();
				float rt_view_zoom = rt_view.getZoom();
				if (ImGui::DragFloat2("RT View Position", (float*)&rt_view_pos)) {
					rt_view.setCenter(rt_view_pos);
				}
				if (ImGui::DragFloat2("RT View Size", (float*)&rt_view_size)) {
					rt_view.setSize(rt_view_size);
				}
				if (ImGui::DragFloat("RT View Zoom", (float*)&rt_view_zoom, 0.05f, -100.f, 100.f)) {
					rt_view.setZoom(rt_view_zoom);
				}
				rtexture.setView(rt_view);
				ImGui::DragFloat2("Blue Circle Position", (float*)&circle1pos);
				ImGui::DragFloat2("Green Circle Position", (float*)&circle2pos);
				ImGui::DragFloat2("Magenta Circle Position", (float*)&circle3pos);

			}
			ImGui::End();

			ImGui::Render();
		}


		// update rendertarget
		rtexture.draw(
			ff::ShapeCircle{
				circle1pos,
				40.f,
				20u,
				ff::Color::Blue
			}, *program);
		rtexture.draw(
			ff::ShapeCircle{
				circle2pos,
				25.f,
				12u,
				ff::Color::Green
			}, *program);
		rtexture.draw(
			ff::ShapeCircle{
				circle3pos,
				15.f,
				12u,
				ff::Color::Magenta
			}, *program);

		window.draw(obj1, *program);
		window.draw(obj2, *program);

		window.draw(line1, *program);
		window.draw(line2, *program);
		window.draw(circle1, *program);
		window.draw(rect1, *program);


		{
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
			//  For this specific demo app we could also call SDL_GL_MakeCurrent(window, gl_context) directly)
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
				SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
			}
		}

		window.display();

		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}


	ff::quit();

	return EXIT_SUCCESS;
}
