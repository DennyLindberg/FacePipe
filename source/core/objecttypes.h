#pragma once

inline constexpr uint8_t ObjectTypeMax = ~uint8_t(0);
typedef uint8_t ObjectType;
typedef uint32_t ObjectId;

typedef int ScriptId;
#define INVALID_SCRIPT_ID -1

// Remember to update App::InitializePools() if a new type is added
// User defined ObjectTypes should be defined in the 100+ range (can't be larger than the max value of ObjectType)

// Core types are in the 0-19 range

#define ObjectType_Unknown 0
#define ObjectType_Object 1
#define ObjectType_Camera 2
#define ObjectType_Light 3

// GL primitives are in the 20-39 range

#define ObjectType_GLTexture 20
#define ObjectType_GLQuad 21
#define ObjectType_GLLine 22
#define ObjectType_GLLineStrips 23
#define ObjectType_GLTriangleMesh 24
#define ObjectType_GLBezierStrips 25

// Reserved range 26-99
