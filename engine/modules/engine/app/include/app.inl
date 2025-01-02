namespace api {
	template<typename T>
	class AppImpl {
		void Setup() {
			static_cast<T*>(this)->Setup();
		}
		void CleanUp() {
			static_cast<T*>(this)->CleanUp();
		}
		void ImGui() {
			static_cast<T*>(this)->ImGui();
		}
		void PreRender() {
			static_cast<T*>(this)->PreRender();
		}
		void PostRender() {
			static_cast<T*>(this)->PostRender();
		}
	};
	template<typename T>
	inline void App::Run(EngineConfig config, AppImpl<T>& impl)
	{
		impl.Setup();
	}
}