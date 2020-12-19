#include "../../../Source/Renderer.h"

#include <fstream>

using namespace Renderer;

int main()
{
	auto renderResult = RenderResult::Create();
	RenderResult & rr = *renderResult;

	rr.Draw();

	std::ofstream ofs;
	ofs.open("output.ppm");
	ofs << "P6\n" << rr.Width() << " " << rr.Height() << "\n255\n";
	ofs.write(( char* ) rr.GetFrontBuffer(), rr.Width() * rr.Height() * 3);
	ofs.close();

	return 0;
}
