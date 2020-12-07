#include "../../../Source/Renderer.h"

#include <fstream>

using namespace Renderer;

int main()
{
	RenderResult rr = RenderResult::Create();

	std::ofstream ofs;
	ofs.open("output.ppm");
	ofs << "P6\n" << rr.Width() << " " << rr.Height() << "\n255\n";
	ofs.write(( char* ) rr.GetFrameBuffer(), rr.GetFrameBufferSize());
	ofs.close();

	return 0;
}
