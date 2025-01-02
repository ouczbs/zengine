#pragma once
#include "json.h"
#include "yaml.h"
#if 0
#define TextSerialize   JsonSerialize
#define TextDeserialize JsonDeserialize
#define TextArchive     JsonArchive
#else 
#define TextSerialize   YamlSerialize
#define TextDeserialize YamlDeserialize
#define TextArchive     YamlArchive
#endif