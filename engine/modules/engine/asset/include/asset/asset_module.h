#include "module/module_manager.h"
namespace api {
    class ASSET_API AssetModule : public IStaticModule
    {
    public:
        void OnLoad(int argc, char** argv) override;
        void OnUnload() override;
        void InitMetaData(void) override {};
        void InitResourceType();
    };
}