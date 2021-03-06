#include "TestCases.h"
#include "../Scene/glTF.h"

extern void		TestglTF_Basics(int argc, char * argv[]);

static TestCase		cases[] =
{
	{"basics",	TestglTF_Basics},
};
TestSuitEntry(glTF);

const char *		LoadglTF(const char * filename)
{
	FILE * file = NULL;
	char * buffer = NULL;
	long cnt = 0;

	if ( fopen_s(&file, filename, "r") )
		return NULL;
	
	if ( fseek(file, 0, SEEK_END) )
		return NULL;
	cnt = ftell(file);
	
	if ( fseek(file, 0, SEEK_SET) )
		return NULL;

	buffer = (char *)malloc(cnt + 1);
	if (buffer) cnt = fread_s(buffer, cnt, 1, cnt, file);
	else cnt = 0;
	buffer[cnt] = '\0';

	fclose(file);

	return buffer;
}

void			TestglTF_Basics(int argc, char * argv[])
{
	struct glTF * gltf;
	const char * gltfContent;

	assert(argc >= 1);

	gltfContent = LoadglTF(argv[0]);
	if ( !gltfContent )
		return;

	gltf = ParseglTF(gltfContent);
	if ( !gltf )
		return;

	for ( struct glTFBuffer * buffer = (struct glTFBuffer *) gltf->buffers; buffer; buffer = (struct glTFBuffer *) buffer->next )
	{
		printf("buffer:\n");
		printf("  size: %d\n", buffer->size);
	}
	for ( struct glTFBufferView * view = (struct glTFBufferView *) gltf->bufferviews; view; view = (struct glTFBufferView *) view->next )
	{
		printf("buffer view:\n");
		printf("  buffer: %d\n", view->buffer);
		printf("  offset: %d\n", view->offset);
		printf("  length: %d\n", view->length);
	}
	for ( struct glTFAccessor * accessor = (struct glTFAccessor *) gltf->accessors; accessor; accessor = (struct glTFAccessor *) accessor->next )
	{
		printf("buffer accessor:\n");
		printf("  view: %d\n", accessor->view);
		printf("  offset: %d\n", accessor->offset);
		printf("  count: %d\n", accessor->count);
		printf("  type: %s\n", accessor->type);
	}
	for ( struct glTFMesh * mesh = (struct glTFMesh *) gltf->meshes; mesh; mesh = (struct glTFMesh *) mesh->next )
	{
		printf("mesh:\n");
		for ( struct glTFPrimitive * primitive = mesh->primitives; primitive; primitive = (struct glTFPrimitive *) primitive->next )
		{
			printf("  primitives:\n");
			printf("    indices: %d\n", primitive->indices);
			printf("    material: %d\n", primitive->material);
			printf("    mode: %d\n", primitive->mode);
			printf("    attributes:\n");
			for ( struct glTFAttributes * attrib = primitive->attribs; attrib; attrib = (struct glTFAttributes *) attrib->next )
			{
				printf("      %s: %d\n", attrib->name, attrib->values);
			}
		}
	}
}