#include "engine/api.h"
#include "os/file_manager.h"
#include "xmalloc_new_delete.h"
namespace api {
    class ENGINE_API EngineModule : public IDynamicModule
    {
    public:
        void OnLoad(int argc, char** argv) override {
#ifdef ENGINE_ROOT
            api::FileManager::Ptr()->Mount("engine", TOSTRING(ENGINE_ROOT));
#endif
        };
        void Initialize() override {
            IDynamicModule::Initialize();
        }
        void OnUnload() override {
        };
        void InitMetaData(void) override {
            mInfo.dependencies = {
                {"app","1.0.1", "static"},
                {"core", "1.0.1", "static" },
                {"asset", "1.0.1", "static" },
                {"render", "1.0.1", "static" },
                {"ui", "1.0.1", "static" },
            };
        };
    };
    IMPLEMENT_DYNAMIC_MODULE(ENGINE_API, EngineModule, engine)
}