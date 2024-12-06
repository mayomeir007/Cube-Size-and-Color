#include <deque>
#include <iostream>
#include <memory>
#include <vector>

#include "gl.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "imgui/imgui_impl_sdl.h"
#include <SDL.h>

#include "Camera.h"
#include "Cube.h"
#include "Grid.h"
#include "Input.h"
#include "Model.h"
#include "Quad.h"
#include "Screen.h"
#include "Shader.h"
#include "Utility.h"

auto isLit = false;
auto isAppRunning = true;

const auto SCREEN_WIDTH = 1920;
const auto SCREEN_HEIGHT = 1080;
const auto CONSOLE_WINDOW_HEIGHT = 250;
const auto PROPERTIES_WINDOW_WIDTH = 400;

std::deque<std::string> messages;
std::vector<std::unique_ptr<Object>> objects;

void RenderConsoleWindow()
{
	ImGui::Begin("Output console", nullptr,
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);
	
	auto windowPos = ImVec2(0, SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT - 25);
	auto windowSize = ImVec2(SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH, CONSOLE_WINDOW_HEIGHT);

	ImGui::SetWindowPos("Output console", windowPos);
	ImGui::SetWindowSize("Output console", windowSize);

	auto message = Utility::ReadMessage();
	
	if (!message.empty())
	{
		messages.push_front(message);
	}

	for (const auto& message : messages)
	{
		ImGui::Text(message.c_str());
	}

	ImGui::End();
}

void RenderPropertiesWindow(Cube& cube)
{
	ImGui::Begin("Properties", nullptr,
		ImGuiWindowFlags_::ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse);

	auto windowPos = ImVec2(SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH, 0);
	auto windowSize = ImVec2(PROPERTIES_WINDOW_WIDTH, SCREEN_HEIGHT);

	ImGui::SetWindowPos("Properties", windowPos);
	ImGui::SetWindowSize("Properties", windowSize);



	auto scale = cube.GetTransform().GetScale();
	ImGui::SliderFloat("Scale", &scale.x, 0.001f, 10.0f, "%.2f");
	cube.GetTransform().SetScale(scale.x, scale.x, scale.x);

	ImGui::Separator();

	auto isTextured = cube.IsTextured();
	ImGui::Checkbox("Textured", &isTextured);
	cube.IsTextured(isTextured);

	if (ImGui::Button("Crate 1 texture"))
	{
		cube.UpdateTexture("crate_1.png");
	}

	if (ImGui::Button("Crate 2 texture"))
	{
		cube.UpdateTexture("crate_2.png");
	}
	
	ImGui::Separator();

	float PI = (float)M_PI;
	float hue = PI*4.f/30 * scale.x;
	glm::vec4 color(1.0f);
	float inten = 1.f / 3;
	float sat = 1;
	if (hue <= PI*2.f/3)
	{
		color.b = inten * (1 - sat);
		color.r = inten * (1 + sat * cos(hue) / (cos(PI*1.f/3 - hue)));
		color.g = 3*inten - (color.r + color.b);
	}
	else
	{
		hue -= PI * 2.f / 3;
		color.r = inten*(1 - sat);
		color.g = inten*(1+sat*cos(hue)/ (cos(PI * 1.f / 3 - hue)));
		color.b = 3 * inten - (color.r + color.g);
	}
	//ImGui::Value("red", color.r);
	//ImGui::Value("green", color.g);
	//ImGui::Value("blue", color.b);
	//auto color = cube.GetColor();

	//ImGui::ColorEdit4("Color", &color.r);
	cube.SetColor(color);

	ImGui::End();
}

int main(int argc, char* argv[])
{
	if (!Screen::Instance()->Initialize())
	{
		return 0;
	}
	
	if (!Shader::Initialize())
	{
		return 0;
	}

	Shader defaultShader;
	defaultShader.Create("Shaders/Default.vert", "Shaders/Default.frag");

	//================================================================

	Grid grid;
	
	//objects.push_back(std::make_unique<Quad>("Floor.jpg", &grid));
	objects.push_back(std::make_unique<Cube>("Crate_1.png", &grid));
	Cube cube("Crate_1.png", &grid);
	//objects.push_back(std::make_unique<Cube>("Crate_2.png", &grid));
	//objects.push_back(std::make_unique<Model>("Models/Armchair.obj", &grid));
	
	Camera camera;
	camera.Set3DView();
	camera.SetSpeed(0.5f);
	camera.SetViewport(0,
			           CONSOLE_WINDOW_HEIGHT,
			           SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH,
			           SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT);
		


	ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/Arial.ttf", 16.0f);
	ImGui::GetIO().Fonts->Build();

	SDL_Rect mouseCollider = { 0 };
	SDL_Rect sceneCollider = { 0,
							   0,
							   SCREEN_WIDTH - PROPERTIES_WINDOW_WIDTH,
							   SCREEN_HEIGHT - CONSOLE_WINDOW_HEIGHT };

	//================================================================

	while (isAppRunning)
	{
		Screen::Instance()->ClearScreen();
		Input::Instance()->Update();

		mouseCollider = { static_cast<int>(Input::Instance()->GetMousePosition().x),
						  static_cast<int>(Input::Instance()->GetMousePosition().y),
						  1,
						  1 };

		bool isMouseColliding = SDL_HasIntersection(&mouseCollider, &sceneCollider);

		if (Input::Instance()->GetMouseWheel() > 0)
		{
			camera.MoveForward();
		}

		else if (Input::Instance()->GetMouseWheel() < 0)
		{
			camera.MoveBackward();
		}


		//==================================================================
		
		if (isMouseColliding && Input::Instance()->IsLeftButtonClicked())
		{
			auto rotation = grid.GetTransform().GetRotation();
			rotation.x += Input::Instance()->GetMouseMotion().x;
			rotation.y += Input::Instance()->GetMouseMotion().y;
			grid.GetTransform().SetRotation(rotation.x, rotation.y, rotation.z);
		}

		isAppRunning = !Input::Instance()->IsXClicked();

		defaultShader.Use();
		camera.SendToShader(defaultShader);
		
		//grid.Render(defaultShader);

		defaultShader.Use();
		camera.SendToShader(defaultShader);
		cube.Render(defaultShader);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		RenderConsoleWindow();
		RenderPropertiesWindow(cube);

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		Screen::Instance()->Present();
	}

	defaultShader.Destroy();	

	Shader::Shutdown();
	Screen::Instance()->Shutdown();	
	
	return 0;
}