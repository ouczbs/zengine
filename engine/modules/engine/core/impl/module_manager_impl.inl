#include "module/module_manager.h"
#include "os/file_manager.h"
namespace api { 
	class ModuleManagerImpl
	{
		friend struct IModule;
	public:
		struct ModuleBlock {
			bool isLoad;
			bool isInit;
		};
		struct ExecuteInfo {
			Name name;
			int argc;
			char** argv;
			bool isActive;
		};
		ExecuteInfo          mInfo;
		SharedLibrary        mProcessLib;
		pmr::table<Name, ModuleBlock>        mModuleBlocks;
		pmr::table<Name, IModule*>           mModuleTable;
		pmr::table<Name, IModule::CreatePFN> mInitializeTable;
		pmr::table<Name, pmr::vector<ISubSystem::CreatePFN>> mInitializeSubSystems;
	public:
		ModuleManagerImpl();
		~ModuleManagerImpl();
		IModule* GetModule(Name name);
		void RegisterModule(Name name, IModule::CreatePFN fn);
		void CreateModule(Name name, bool shared);
		void DestroyModule(Name name);
		void InitModule(Name name);
		void ShutModule(Name name);
		void MakeGraph(Name name, bool shared, int argc, char** argv);
		void MainLoop();
		void DestroyGraph();
	public:
		IModule* spawnDynamicModule(Name name, bool hotfix);
		IModule* spawnStaticModule(Name name);
	};
	ModuleManagerImpl::ModuleManagerImpl()
	{
		mProcessLib.Load();
	}
	ModuleManagerImpl::~ModuleManagerImpl()
	{
		DestroyGraph();
	}
	void ModuleManagerImpl::CreateModule(Name name, bool is_shared)
	{
		bool hotfix = true;
		IModule* module = is_shared ?
			spawnDynamicModule(name, hotfix) :
			spawnStaticModule(name);
		if (!module) { return; }
		mModuleBlocks[name] = ModuleBlock{false, false};
		ModuleBlock& block = mModuleBlocks[name];
		auto& moduleInfo = module->mInfo;
		Name shared("shared");
		for (auto& dep : moduleInfo.dependencies) {
			if(auto it = mModuleBlocks.find(dep.name); it == mModuleBlocks.end())
				CreateModule(dep.name, dep.kind == shared);
		}
		module->OnLoad(mInfo.argc, mInfo.argv);
		block.isLoad = true;
	}
	void ModuleManagerImpl::DestroyModule(Name name)
	{
		auto it = mModuleBlocks.find(name);
		if (it == mModuleBlocks.end() || !it->second.isLoad) {
			return;
		}
		IModule* module = mModuleTable[name];
		auto& moduleInfo = module->mInfo;
		for (auto& dep : moduleInfo.dependencies) {
			DestroyModule(dep.name);
		}
		module->OnUnload();
		it->second.isLoad = false;
	}
	void ModuleManagerImpl::InitModule(Name name)
	{
		auto it = mModuleBlocks.find(name);
		if (it == mModuleBlocks.end() || it->second.isInit) {
			return;
		}
		IModule* module = mModuleTable[name];
		auto& moduleInfo = module->mInfo;
		for (auto& dep : moduleInfo.dependencies) {
			InitModule(dep.name);
		}
		module->Initialize();
		it->second.isInit = true;
	}
	void ModuleManagerImpl::ShutModule(Name name)
	{
		auto it = mModuleBlocks.find(name);
		if (it == mModuleBlocks.end() || !it->second.isInit) {
			return;
		}
		IModule* module = mModuleTable[name];
		auto& moduleInfo = module->mInfo;
		for (auto& dep : moduleInfo.dependencies) {
			ShutModule(dep.name);
		}
		module->Finalize();
		it->second.isInit = false;
	}
	void ModuleManagerImpl::MakeGraph(Name name, bool shared, int argc, char** argv)
	{
		mInfo = { name, argc, argv , true};
		CreateModule(name, shared);
		InitModule(name);
	}
	inline void ModuleManagerImpl::MainLoop()
	{
		auto it = mModuleTable.find(mInfo.name);
		dynamic_cast<IMainModule*>(it->second)->MainLoop();
	}
	void ModuleManagerImpl::DestroyGraph()
	{
		if(mInfo.isActive){
			ShutModule(mInfo.name);
			DestroyModule(mInfo.name);
		}
	}
	IModule* ModuleManagerImpl::spawnDynamicModule(Name name, bool hotfix)
	{
		if (auto it = mModuleTable.find(name); it != mModuleTable.end()) {
			return it->second;
		}
		SharedLibrary sharedLib;
		string_view name_view = name.ToStringView();
		pmr::string newFuncName("__newDynamicModule__");
		newFuncName.append(name_view);
		void* newFuncAddr = mProcessLib.GetSymbol(newFuncName.data());
		if (!newFuncAddr) {
			pmr::string libPath("/exe/");
			libPath.reserve(10 + name_view.size());
			libPath.append(name == mInfo.name ? "mgame" : name_view);
			libPath.append(SharedLibrary::GetExtensionName());
			if (sharedLib.Load(libPath)) {
				newFuncAddr = sharedLib.GetSymbol(newFuncName.data());
			}
		}
		IDynamicModule* module = newFuncAddr ? (IDynamicModule*)((IModule::CreatePFN)newFuncAddr)() : new(GlobalPool()) DefaultDynamicModule(name);
		mModuleTable[name] = module;
		module->mSharedLib = sharedLib;
		module->InitMetaData();
		return module;
	}
	IModule* ModuleManagerImpl::spawnStaticModule(Name name)
	{
		if (auto it = mModuleTable.find(name); it != mModuleTable.end()) {
			return it->second;
		}
		auto it = mInitializeTable.find(name);
		if (it == mInitializeTable.end()) {
			return nullptr;
		}
		IModule* module = it->second();
		module->InitMetaData();
		mModuleTable[name] = module;
		return module;
	}
	IModule* ModuleManagerImpl::GetModule(Name name)
	{
		auto it = mModuleTable.find(name);
		if (it == mModuleTable.end()) {
			return nullptr;
		}
		return it->second;
	}
	void ModuleManagerImpl::RegisterModule(Name name, IModule::CreatePFN fn)
	{
		mInitializeTable[name] = fn;
	}
	ModuleManager* ModuleManager::Ptr()
	{
		static ModuleManager* ptr = new(GlobalPool()) ModuleManager();
		return ptr;
	}

	inline ModuleManager::ModuleManager()
	{
		new(GlobalPool()) FileManager();
		impl = new(GlobalPool()) ModuleManagerImpl();
	}

	inline ModuleManager::~ModuleManager()
	{
		impl->~ModuleManagerImpl();
		FileManager::Ptr()->~FileManager();
	}

	inline IModule* ModuleManager::GetModule(Name name)
	{
		return impl->GetModule(name);
	}

	inline void ModuleManager::RegisterModule(Name name, IModule::CreatePFN fn)
	{
		impl->RegisterModule(name, fn);
	}

	inline void ModuleManager::CreateModule(Name name, bool shared)
	{
		impl->CreateModule(name, shared);
	}

	inline void ModuleManager::DestroyModule(Name name)
	{
		impl->DestroyModule(name);
	}

	inline void ModuleManager::InitModule(Name name)
	{
		impl->InitModule(name);
	}

	inline void ModuleManager::ShutModule(Name name)
	{
		impl->ShutModule(name);
	}

	inline void ModuleManager::MakeGraph(Name name, bool shared, int argc, char** argv)
	{
		impl->MakeGraph(name, shared, argc, argv);
	}

	inline void ModuleManager::MainLoop()
	{
		impl->MainLoop();
	}

	inline void ModuleManager::DestroyGraph()
	{
		impl->DestroyGraph();
	}

	inline IModule* ModuleManager::spawnDynamicModule(Name name, bool hotfix)
	{
		return impl->spawnDynamicModule(name, hotfix);
	}

	inline IModule* ModuleManager::spawnStaticModule(Name name)
	{
		return impl->spawnStaticModule(name);
	}
}