#include "framework.h"
#include "Application.h"
#include "Demo.h"

int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, int nCmdShow)
{
	int retCode = 0;

	Application::Create(hInstance);
	{
		std::shared_ptr<Demo> demo = std::make_shared<Demo>(L"Learning DirectX12", 1280, 720);
		retCode = Application::Get().Run(demo);
	}
	Application::Destroy();

	return retCode;
}