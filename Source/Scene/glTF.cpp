#include "glTF.h"

#include <assert.h>
#include <ctype.h>
#include <malloc.h>
#include <string.h>
#include <stdio.h>

#define VLEVEL (1)
#define VLOG(level, ...) do { if ((level) <= VLEVEL) printf(__VA_ARGS__); } while (0)

#define DEFINE_OBJECT_POOL(type, size)								 \
	struct type##Pool									 \
	{											 \
		int used;									 \
		int next;									 \
		unsigned char bitmap[ (size + 7) / 8 ];						 \
		type objects[ size ];								 \
	};											 \
	static struct type##Pool g##type##Pool;							 \
	static type## *	Get##type()								 \
	{											 \
		int byte;									 \
		int bit;									 \
		int msk;									 \
		type## * obj;									 \
												 \
		if ( g##type##Pool.used < size )						 \
		{										 \
			do									 \
			{									 \
				byte = g##type##Pool.next >> 3;					 \
				bit = g##type##Pool.next & 0x7;					 \
				msk = 1 << bit;							 \
				g##type##Pool.next = ( g##type##Pool.next + 1 ) & (size - 1);	 \
			}									 \
			while ( g##type##Pool.bitmap[ byte ] & msk );				 \
			VLOG(3, "Get " #type " %d:%d\n", byte, msk);				 \
			g##type##Pool.bitmap[ byte ] |= ( char ) msk;				 \
			++g##type##Pool.used;							 \
												 \
			assert(byte * 8 + bit < size);						 \
			obj = g##type##Pool.objects + byte * 8 + bit;				 \
			memset(obj, 0, sizeof(type));						 \
			return obj;								 \
		}										 \
		else										 \
		{										 \
			assert(false);								 \
			return NULL;								 \
		}										 \
	}											 \
	static void		Put##type(type * object)					 \
	{											 \
		int byte;									 \
		int msk;									 \
												 \
		assert(g##type##Pool.objects <= object &&					 \
			object < ( g##type##Pool.objects + size ));				 \
												 \
		byte = ( object - g##type##Pool.objects ) >> 3;					 \
		msk = 1 << (( object - g##type##Pool.objects ) & 0x7);				 \
												 \
		if ( g##type##Pool.bitmap[ byte ] & msk )					 \
		{										 \
			VLOG(3, "Put " #type " %d:%d\n", byte, msk);				 \
			g##type##Pool.bitmap[ byte ] &= ( char ) ~msk;				 \
			--g##type##Pool.used;							 \
		}										 \
	}

#define TOP()			(*(tokens) ? (*(tokens))->type : TokenType::UNKNOWN)
#define POP()			(Pop(tokens))
#define PEEK(type_)		(*(tokens) && (*(tokens))->type == type_)
#define PEEK_PUNC(punc)		(*(tokens) && (*(tokens))->type == TokenType::PUNCTUATOR && (*(tokens))->ival == (punc))
#define DROP(type)		(Skip(tokens, type))
#define DROP_PUNC(punc)		(SkipPunctuator(tokens, punc))
#define DROP_FIELD(field)	(SkipField(tokens, field))
#ifdef _DEBUG
#define SKIP(type)		assert(Skip(tokens, type))
#define SKIP_PUNC(punc)		assert(SkipPunctuator(tokens, punc))
#define READ_INT(i)		assert(ReadInt(tokens, i))
#define READ_STR(p, n)		assert(ReadString(tokens, p, n))
#else
#define SKIP(type)		do { if (!Skip(tokens, type)) return NULL; } while(0)
#define SKIP_PUNC(punc)		do { if (!SkipPunctuator(tokens, punc)) return NULL; } while(0)
#define READ_INT(i)		do { if (!ReadInt(tokens, i)) return NULL; } while (0)
#define READ_STR(p, n)		do { if (!ReadString(tokens, p, n)) return NULL; } while (0)
#endif

enum class TokenType
{
	UNKNOWN,

	SCENES,
	NODES,
	MESHES,
	BUFFERS,
	BUFFERVIEWS,
	ACCESSORS,
	FIELD,
	PRIMITIVES,
	ATTRIBUTES,

	PUNCTUATOR,
	INTEGER,
};

enum class TokenInfo
{
	UNKNOWN,

	STR,
	CHR,
	NUM,
};

struct Token
{
	struct Token * next;

	enum TokenType type;
	union
	{
		int ival;
		char cval;
		const char * pval;
	};
};

struct ScanContext
{
	const char * p;
	int line;
	int column;
};

DEFINE_OBJECT_POOL(Token,		65536);
DEFINE_OBJECT_POOL(glTF,		256);
DEFINE_OBJECT_POOL(glTFBuffer,		256);
DEFINE_OBJECT_POOL(glTFBufferView,	256);
DEFINE_OBJECT_POOL(glTFAccessor,	1024);
DEFINE_OBJECT_POOL(glTFMesh,		256);
DEFINE_OBJECT_POOL(glTFPrimitive,	256);
DEFINE_OBJECT_POOL(glTFAttributes,	256);

// helpers
static inline int		Min(int a, int b)
{
	return a <= b ? a : b;
}
static bool			ReadFile(const char * filename, char * buf, int cnt)
{
	FILE * file = NULL;
	int ret = 0;

	assert(filename);
	assert(buf);
	assert(cnt > 0);

	if ( !fopen_s(&file, filename, "rb") )
	{
		ret = fread(buf, 1, cnt, file);
		fclose(file);
	}

	return ret == cnt;
}

// parse tokens
static const char *		TokenBeg(const char * p, ScanContext * ctx)
{
	while ( isspace(*p) )
	{
		switch ( *p )
		{
			case '\r':
				if ( *(p + 1) == '\n' )
				{
					++p;
				}
			case '\n':
				++p;
				++ctx->line;
				ctx->column = 0;
				ctx->p = p;
				break;
			default:
				++ctx->column;
				++p;
				break;
		}
	}
	return p;
}
static const char *		TokenEnd(const char * p, enum TokenInfo * info)
{
	const char * end = p;
	enum TokenInfo inf = TokenInfo::UNKNOWN;

	switch ( *end )
	{
		case ',':
		case ':':
		case '[': case ']':
		case '{': case '}':
			inf = TokenInfo::CHR;
			++end;
			break;
		case '"':
			inf = TokenInfo::STR;
			end = strchr(end + 1, '"');
			if ( end ) ++end;
			else end = p;
			break;
		default:
			if ( *end == '-')
			{
				while ( isspace(*(++end)) );
			}
			if ( isdigit(*end) )
			{
				inf = TokenInfo::NUM;
				while ( isdigit(*(++end)) );
				if ( *end == '.' )
				{
					while ( isdigit(*(++end)) );
					if ( *end == 'e' )
					{
						++end;
						if ( *end == '-' ) ++end;
						while ( isdigit(*end) ) ++end;
					}
				}
			}
			else
			{
				end = p;
			}
			break;
	}

	*info = (end != p) ? inf : TokenInfo::UNKNOWN;
	return end;
}
static int			IdentifyInteger(const char * p)
{
	int value = 0;
	bool neg = false;

	if ( *p == '-' )
	{
		neg = true;
		++p;
	}
	do
	{
		value = value * 10 + (*p - '0');
	}
	while ( isdigit(*(++p)) );

	return neg ? -value : value;
}
static struct Token *		IdentifyToken(const char * begin, const char * end, const enum TokenInfo info)
{
	struct Token * token = GetToken();

	assert(token);

	switch ( info )
	{
		case TokenInfo::STR:
			++begin;
			if ( memcmp(begin, "scenes", 6) == 0 )			token->type = TokenType::SCENES;
			else if ( memcmp(begin, "nodes", 5) == 0 )		token->type = TokenType::NODES;
			else if ( memcmp(begin, "buffers", 7) == 0 )		token->type = TokenType::BUFFERS;
			else if ( memcmp(begin, "bufferViews", 11) == 0 )	token->type = TokenType::BUFFERVIEWS;
			else if ( memcmp(begin, "accessors", 9) == 0 )		token->type = TokenType::ACCESSORS;
			else if ( memcmp(begin, "meshes", 6) == 0 )		token->type = TokenType::MESHES;
			else if ( memcmp(begin, "primitives", 10) == 0 )	token->type = TokenType::PRIMITIVES;
			else if ( memcmp(begin, "attributes", 10) == 0 )	token->type = TokenType::ATTRIBUTES;
			else token->type = TokenType::FIELD, token->pval = begin;
			break;
		case TokenInfo::CHR:
			token->type = TokenType::PUNCTUATOR;
			token->ival = *begin;
			break;
		case TokenInfo::NUM:
			token->type = TokenType::INTEGER;
			token->ival = IdentifyInteger(begin);
			break;
		default:
			token->type = TokenType::UNKNOWN;
			break;
	}

	return token;
}
static const char *		FormatToken(const struct Token * token)
{
	static char buf[256];

	switch ( token->type )
	{
		case TokenType::PUNCTUATOR:	snprintf(buf, 256, "punc('%c')", token->ival); break;
		case TokenType::INTEGER:	snprintf(buf, 256, "int(%d)", token->ival); break;
		case TokenType::SCENES:		snprintf(buf, 256, "scenes"); break;
		case TokenType::NODES:		snprintf(buf, 256, "nodes"); break;
		case TokenType::BUFFERS:	snprintf(buf, 256, "buffers"); break;
		case TokenType::BUFFERVIEWS:	snprintf(buf, 256, "bufferviews"); break;
		case TokenType::ACCESSORS:	snprintf(buf, 256, "accessors"); break;
		case TokenType::MESHES:		snprintf(buf, 256, "meshes"); break;
		case TokenType::PRIMITIVES:	snprintf(buf, 256, "primitives"); break;
		case TokenType::ATTRIBUTES:	snprintf(buf, 256, "attributes"); break;
		case TokenType::FIELD:		snprintf(buf, 256, "field:");
						snprintf(buf + 6, strchr(token->pval, '"') - token->pval + 1, token->pval); break;
		default:			snprintf(buf, 256, "unknown"); break;
	}

	return buf;
}
static struct Token *		ParseTokens(const char * p)
{
	struct Token head;
	struct Token * token;
	enum TokenInfo info;
	struct ScanContext ctx;
	const char * beg;
	const char * end;

	token = &head;
	ctx.p = p;
	ctx.line = 1;
	ctx.column = 0;
	beg = p;

	while ( *beg )
	{
		beg = TokenBeg(beg, &ctx);
		end = TokenEnd(beg, &info);

		if ( !*beg )
		{
			break;
		}
		if ( end == beg )
		{
			fprintf(stderr, "Bad token at %d:%d\n", ctx.line, ctx.column);
			for ( const char * ch = ctx.p; *ch && *ch != '\r' && *ch != '\n'; ++ch ) fputc(*ch, stderr);
			fputc('\n', stderr);
			for ( int i = 0; i < ctx.column; ++i ) fputc(' ', stderr);
			fprintf(stderr, "^\n");
			break;
		}

		token->next = IdentifyToken(beg, end, info);
		token = token->next;
		beg = end;
		VLOG(2, "Token: %s\n", FormatToken(token));
	}

	token->next = NULL;

	return head.next;
}

// iterate tokens
static inline TokenType		Pop(struct Token ** tokens)
{
	struct Token * token = *tokens;

	if ( token )
	{
		*tokens = token->next;
		return token->type;
	}
	else
	{
		return TokenType::UNKNOWN;
	}
}
static inline bool		Skip(struct Token ** tokens, TokenType type)
{
	struct Token * token = *tokens;

	if ( token && token->type == type )
	{
		*tokens = token->next;
		return true;
	}
	else
	{
		return false;
	}
}
static inline bool		SkipPunctuator(struct Token ** tokens, char punc)
{
	struct Token * token = *tokens;

	if ( token && token->type == TokenType::PUNCTUATOR && token->ival == punc )
	{
		*tokens = token->next;
		return true;
	}
	else
	{
		return false;
	}
}
static inline bool		SkipField(struct Token ** tokens, const char * field)
{
	struct Token * token = *tokens;

	if ( token && token->type == TokenType::FIELD && strncmp(field, token->pval, strlen(field)) == 0 )
	{
		*tokens = token->next;
		return true;
	}
	else
	{
		return false;
	}
}
static inline bool		ReadInt(struct Token ** tokens, int * ival)
{
	struct Token * token = *tokens;

	if ( token && token->type == TokenType::INTEGER )
	{
		*ival = token->ival;
		*tokens = token->next;
		return true;
	}
	else
	{
		return false;
	}
}
static inline bool		ReadString(struct Token ** tokens, char * buf, int cnt)
{
	struct Token * token = *tokens;
	int len;

	assert(buf && cnt > 0);

	if ( token && token->type == TokenType::FIELD )
	{
		len = Min(cnt - 1, strchr(token->pval, '"') - token->pval);
		memcpy(buf, token->pval, len);
		buf[len] = '\0';
		*tokens = token->next;
		return true;
	}
	else
	{
		return false;
	}
}

// parse glTF elements
static struct glTFList *	ParseglTFList(struct Token ** tokens, struct glTFList * (*parse)(struct Token **))
{
	struct glTFList * head = NULL;
	struct glTFList * next = NULL;

	SKIP_PUNC('[');
	while ( DROP_PUNC('{') )
	{
		if ( next ) next = next->next = parse(tokens);
		else head = next = parse(tokens);

		if ( !next ) return NULL;

		SKIP_PUNC('}');
		DROP_PUNC(',');
	}
	SKIP_PUNC(']');

	return head;
}
static struct glTFList *	ParseglTFAccessor(struct Token ** tokens)
{
	struct glTFAccessor * accessor = GetglTFAccessor();

	do
	{
		if ( DROP_FIELD("bufferView") )
		{
			SKIP_PUNC(':');
			READ_INT(&accessor->view);
		}
		else if ( DROP_FIELD("byteOffset") )
		{
			SKIP_PUNC(':');
			READ_INT(&accessor->offset);
		}
		else if ( DROP_FIELD("count") )
		{
			SKIP_PUNC(':');
			READ_INT(&accessor->count);
		}
		else if ( DROP_FIELD("type") )
		{
			SKIP_PUNC(':');
			READ_STR(accessor->type, sizeof(accessor->type));
		}
		else if ( DROP(TokenType::FIELD) )
		{
			SKIP_PUNC(':');
			
			if (DROP_PUNC('['))
			{
				while (*tokens && !PEEK_PUNC(']')) POP();
				SKIP_PUNC(']');
			}
			else
			{
				SKIP(TokenType::INTEGER);
			}
		}
	}
	while ( DROP_PUNC(',') && !PEEK_PUNC('}') );

	return accessor;
}
static struct glTFAccessor *	ParseglTFAccessorList(struct Token ** tokens)
{
	SKIP(TokenType::ACCESSORS);
	SKIP_PUNC(':');
	return (struct glTFAccessor *) ParseglTFList(tokens, ParseglTFAccessor);
}
static struct glTFList *	ParseglTFBufferView(struct Token ** tokens)
{
	struct glTFBufferView * view = GetglTFBufferView();

	do
	{
		if ( DROP_FIELD("buffer") )
		{
			SKIP_PUNC(':');
			READ_INT(&view->buffer);
		}
		else if ( DROP_FIELD("byteOffset") )
		{
			SKIP_PUNC(':');
			READ_INT(&view->offset);
		}
		else if ( DROP_FIELD("byteLength") )
		{
			SKIP_PUNC(':');
			READ_INT(&view->length);
		}
		else if ( DROP_FIELD("byteStride") )
		{
			SKIP_PUNC(':');
			READ_INT(&view->stride);
		}
		else if ( PEEK(TokenType::FIELD) )
		{
			VLOG(1, "BufferView: drop field '%s'\n", FormatToken(*tokens));
			SKIP(TokenType::FIELD);
			
			SKIP_PUNC(':');
			if ( DROP(TokenType::INTEGER) );
			else SKIP(TokenType::FIELD);
		}
	}
	while ( DROP_PUNC(',') && !PEEK_PUNC('}') );

	return view;
}
static struct glTFBufferView *	ParseglTFBufferViewList(struct Token ** tokens)
{
	SKIP(TokenType::BUFFERVIEWS);
	SKIP_PUNC(':');
	return (struct glTFBufferView *) ParseglTFList(tokens, ParseglTFBufferView);
}
static struct glTFList *	ParseglTFBuffer(struct Token ** tokens)
{
	static char filename[256];
	struct glTFBuffer * buffer = GetglTFBuffer();

	memset(filename, 0, 256);

	do
	{
		if ( DROP_FIELD("uri") )
		{
			SKIP_PUNC(':');
			READ_STR(filename, 256);
		}
		else if ( DROP_FIELD("byteLength") )
		{
			SKIP_PUNC(':');
			READ_INT(&buffer->size);
		}
	}
	while ( DROP_PUNC(',') && !PEEK_PUNC('}') );

	buffer->data = (char *) malloc(buffer->size);
	if ( !ReadFile(filename, buffer->data, buffer->size) )
	{
		return NULL;
	}

	return buffer;
}
static struct glTFBuffer *	ParseglTFBufferList(struct Token ** tokens)
{
	SKIP(TokenType::BUFFERS);
	SKIP_PUNC(':');
	return (struct glTFBuffer *) ParseglTFList(tokens, ParseglTFBuffer);
}
static struct glTFAttributes *	ParseglTFAttributesList(struct Token ** tokens)
{
	struct glTFAttributes head;
	struct glTFAttributes * next;

	head.next = NULL;
	next = &head;
	do
	{
		next->next = GetglTFAttributes();
		next = (struct glTFAttributes *) next->next;
		READ_STR(next->name, sizeof(next->name));
		SKIP_PUNC(':');
		READ_INT(&next->values);
	}
	while ( DROP_PUNC(',') && !PEEK_PUNC('}') );

	return (struct glTFAttributes *)head.next;
}
static struct glTFList *	ParseglTFPrimitive(struct Token ** tokens)
{
	struct glTFPrimitive * primitives = GetglTFPrimitive();
	
	do
	{
		if ( DROP(TokenType::ATTRIBUTES) )
		{
			SKIP_PUNC(':');
			SKIP_PUNC('{');
			primitives->attribs = ParseglTFAttributesList(tokens);
			SKIP_PUNC('}');
		}
		else if ( DROP_FIELD("indices") )
		{
			SKIP_PUNC(':');
			READ_INT(&primitives->indices);
		}
		else if ( DROP_FIELD("material") )
		{
			SKIP_PUNC(':');
			READ_INT(&primitives->material);
		}
		else if ( DROP_FIELD("mode") )
		{
			SKIP_PUNC(':');
			READ_INT(&primitives->mode);
		}
	}
	while ( DROP_PUNC(',') && !PEEK_PUNC('}') );

	return primitives;
}
static struct glTFPrimitive *	ParseglTFPrimitiveList(struct Token ** tokens)
{
	SKIP(TokenType::PRIMITIVES);
	SKIP_PUNC(':');
	return (struct glTFPrimitive *) ParseglTFList(tokens, ParseglTFPrimitive);
}
static struct glTFList *	ParseglTFMesh(struct Token ** tokens)
{
	struct glTFMesh * mesh = GetglTFMesh();
	mesh->primitives = (struct glTFPrimitive *) ParseglTFPrimitiveList(tokens);
	return mesh;
}
static struct glTFMesh *	ParseglTFMeshList(struct Token ** tokens)
{
	SKIP(TokenType::MESHES);
	SKIP_PUNC(':');
	return (struct glTFMesh *) ParseglTFList(tokens, ParseglTFMesh);
}

struct glTF *			ParseglTF(const char * p)
{
	struct glTF * gltf = GetglTF();
	struct Token ** tokens;
	struct Token * tks;

	tokens = &tks;
	tks = ParseTokens(p);

	while ( tks )
	{
		switch ( tks->type )
		{
			case TokenType::BUFFERS: gltf->buffers = ParseglTFBufferList(tokens); break;
			case TokenType::BUFFERVIEWS: gltf->bufferviews = ParseglTFBufferViewList(tokens); break;
			case TokenType::ACCESSORS: gltf->accessors = ParseglTFAccessorList(tokens); break;
			case TokenType::MESHES: gltf->meshes = ParseglTFMeshList(tokens); break;
			default: tks = tks->next; break;
		}
	}

	return gltf;
}
