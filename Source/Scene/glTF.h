#pragma once

struct glTFList
{
	struct glTFList * next;
};
struct glTFTree
{
	struct glTFTree * next;
	struct glTFTree * child;
};

struct glTF
{
	struct glTFScene * scenes;
	struct glTFBuffer * buffers;
	struct glTFBufferView * bufferviews;
	struct glTFAccessor * accessors;
	struct glTFMesh * meshes;
	struct glTFNode * nodes;
};

struct glTFScene : glTFList
{
	struct glTFNode * node;
};

struct glTFNode : glTFTree
{
	struct glTFMatrix * matrix;
	struct glTFCamera * camera;
	struct glTFMesh * mesh;
};

struct glTFBuffer : glTFList
{
	char * data;
	int size;
};

struct glTFBufferView : glTFList
{
	int buffer;
	int offset;
	int length;
	int stride;
};

struct glTFAccessor : glTFList
{
	int view;
	int offset;
	int count;
	char type[12];
};

struct glTFMatrix
{
	float m[16];
};

struct glTFCamera
{
};

struct glTFMesh : glTFList
{
	struct glTFPrimitive * primitives;
};

struct glTFPrimitive : glTFList
{
	struct glTFAttributes * attribs;
	int indices;
	int material;
	int mode;
};

struct glTFAttributes : glTFList
{
	char name[16];
	int values;
};

struct glTF * ParseglTF(const char * p);
