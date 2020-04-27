#include <Game.h>
#include <DirectXMath.h>
#include <Windows.h>
#include <Application.h>
#include <Window.h>

Game::Game(const std::wstring& name, int width, int height, bool vSync)
{
}

Game::~Game()
{
}

bool Game::Initialize()
{

	// CPU 지원 확인
	if (!DirectX::XMVerifyCPUSupport()) {
		MessageBoxA(NULL, "Failed to verify DirectX Math library support.", "Error", MB_OK | MB_ICONERROR);
		return false;
	}

	m_pWindow = Application::Get().CreateRenderWindow(m_Name, m_Width, m_Height, m_vSync);
	m_pWindow->RegisterCallbacks(shared_from_this());
	m_pWindow->Show();
	return true;
}

void Game::Destroy()
{
	Application::Get().DestroyWindow(m_pWindow);
	m_pWindow.reset();
}
