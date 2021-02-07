#pragma once

#include "../Includes/RendererApi.h"

namespace Graphics
{
	Ptr<IScene>	CreateTestScene_Effects();
	Ptr<IScene>	CreateTestScene_Minecraft();
	Ptr<IScene>	CreateTestScene_Mirror();
	Ptr<IScene>	CreateTestScene_Water();
}

void	Test(int argc, char * argv[]);
void	TestNative_Callbacks();
void	TestNative_Blit();
void	TestNative_MultipleWindow();