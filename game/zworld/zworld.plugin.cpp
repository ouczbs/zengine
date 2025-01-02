#include "src/zworld.h"
void __zworld__module::InitMetaData(void){
	mInfo.name = "zworld";
	mInfo.dependencies =  {
 		{"engine", "1.0.1", "static" },
		{"editor", "1.0.1", "static" },
		{"vulkan", "1.0.1", "shared" }
	};
};