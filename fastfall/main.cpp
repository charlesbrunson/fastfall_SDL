
#include <thread>
#include <iostream>

#include "fastfall/util.hpp"
#include "fastfall/render.hpp"

using namespace std::chrono;

bool handleInputs(ff::Window& window) {

	bool quit = false;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYUP:
			if (event.key.keysym.sym == SDLK_ESCAPE)
				quit = true;
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event) {
			case SDL_WINDOWEVENT_CLOSE:	
				quit = true; 
				break;
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
	return quit;
}

int main(int argc, char* argv[])
{

	using namespace ff;

	LOG_INFO("Hello World!");

	ff::FFinit();

	// make a window

	ff::Window window{ "AAAAAAAAAAAA", 800, 600 };
	window.setWindowCentered();
	window.setWindowResizable();
	window.setVsyncEnabled();
	window.showWindow();
	window.setActive();

	// use default shader

	const ff::ShaderProgram& shader = ff::ShaderProgram::getDefaultProgram();

	// load a couple textures

	ff::Texture tex1{};
	tex1.loadFromFile("data/testimage1.png");
	assert(tex1.exists());

	ff::Texture tex2;
	tex2.loadFromFile("data/testimage2.bmp");
	assert(tex2.exists());

	// make a render texture

	ff::RenderTexture rtexture;
	rtexture.create(100, 100);

	// make a bunch of drawable stuff

	ff::Sprite obj1{ rtexture.getTexture(), 100.f, 100.f };
	obj1.setOrigin(50.f, 50.f);

	ff::Sprite obj2{ &tex1 };
	obj2.setPosition(200.f, -200.f);
	obj2.setOrigin(10.f, 15.f);
	obj2.setScale(3.f);

	ff::Sprite obj3{ &tex2 };
	obj3.setPosition(50.f, -220.f);
	obj3.setOrigin(50.f, 50.f);

	ff::ShapeLine line1{{ 50.f, 50.f}, {-50.f, -50.f}};
	ff::ShapeLine line2{{-50.f, 50.f}, { 50.f, -50.f}};
	line1.setPosition(-150.f, -150.f);
	line2.setPosition(-150.f, -150.f);

	ff::ShapeRectangle rect1{ ff::Rectf{100.f, 0.f, 100.f, 100.f}, ff::Color::White().alpha(50), ff::Color::White };

	ff::ShapeCircle circle1{{0.f, 0.f},	150.f, 32u, ff::Color::Green().alpha(50), ff::Color::Red};
	circle1.setPosition(-150.f, 150.f);

	ff::ShapeCircle rt1{{ -10.f, -10.f }, 40.f, 20u, ff::Color::Blue };
	ff::ShapeCircle rt2{{  25.f,  25.f }, 25.f, 12u, ff::Color::Green};
	ff::ShapeCircle rt3{{  25.f, -25.f }, 15.f, 12u, ff::Color::Magenta};


	system_clock clock;
	time_point start = clock.now();
	duration<float, std::ratio<1, 1>> elapsed;

	bool quit = false;
	while (!quit) {
		time_point now = clock.now();
		elapsed = (now - start);
		start = now;

		quit = handleInputs(window);

		rtexture.clear(ff::Color{ 100, 100, 100 });
		window.clear(ff::Color{ 0x060608FF });

		{

			// do imgui stuff

			static bool show_demo_window = false;

			ff::ImGuiNewFrame(window);

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
					obj2.setRotation(obj2.getRotation() - elapsed.count());
					obj3.setRotation(obj3.getRotation() - elapsed.count());
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

				glm::fvec2 rt1pos = rt1.getPosition();
				glm::fvec2 rt2pos = rt2.getPosition();
				glm::fvec2 rt3pos = rt3.getPosition();
				ImGui::DragFloat2("Blue Circle Position", (float*)&rt1pos);
				ImGui::DragFloat2("Green Circle Position", (float*)&rt2pos);
				ImGui::DragFloat2("Magenta Circle Position", (float*)&rt3pos);
				rt1.setPosition(rt1pos);
				rt2.setPosition(rt2pos);
				rt3.setPosition(rt3pos);

			}
			ImGui::End();

		}

		// update rendertarget
		rtexture.draw(rt1, shader);
		rtexture.draw(rt2, shader);
		rtexture.draw(rt3, shader);

		window.draw(obj1,    shader);
		window.draw(obj2,    shader);
		window.draw(obj3,    shader);
		window.draw(line1,   shader);
		window.draw(line2,   shader);
		window.draw(circle1, shader);
		window.draw(rect1,   shader);

		ff::ImGuiRender();

		window.display();

		std::this_thread::sleep_for(milliseconds(1));
	}


	ff::FFquit();

	return EXIT_SUCCESS;
}
