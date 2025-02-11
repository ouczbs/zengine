#include "serialize.h"
namespace api {
	inline void YamlArchive::InitSerde()
	{
		using std::string_view;
#define RegisterAny(T) Register<T>();
#include "../register.inl"
#undef RegisterAny
	}
	inline Name VYamlSerdeRead() {
		return Name("YamlSerdeRead");
	}
	inline Name VYamlSerdeWrite() {
		return Name("YamlSerdeWrite");
	}
	template<typename T>
	inline void YamlArchive::Register()
	{
		auto uclass = meta_info<T>();
		auto [bfind, it] = uclass->vtable.FindLast(VYamlSerdeRead());
		if (!bfind && it) {
			it = it->Insert(VYamlSerdeRead(), (void*)&gen::YamlSerde<T>::Read);
			it = it->Insert(VYamlSerdeWrite(), (void*)&gen::YamlSerde<T>::Write);
		}
	}
	inline YAML::Node YamlArchive::Serialize(Any any)
	{
		if (!any) {
			return YAML::Node();
		}
		auto it_func = any.FindVtable<YamlVTable::Write>(VYamlSerdeWrite());
		if (it_func) {
			return it_func(any.ptr);
		}
		if (any.IsEnum()) {
			return Serialize(any.Parent());
		}
		YAML::Node result;
		if (any.Check(meta_info<Any>())) {
			Any obj = any.CastTo<Any>();
			if (obj) {
				result[CLASS_KEY_NAME] = obj.cls->name.ToStringView();
				result[DATA_KEY_NAME] = Serialize(obj);
			}
			return result;
		}
		if (any.IsContainer()) {
			refl::Container docker(any);
			bool isMap = any.IsMap();
			auto fieldList = any.ContainerType()->GetFields(refl::FIND_ALL_MEMBER, Name(""));
			for (auto obj : docker) {
				if (isMap) {
					Any first = obj.Member(fieldList[0]);
					Any second = obj.Member(fieldList[1]);
					result[Serialize(first)] = Serialize(second);
				}
				else {
					result.push_back(Serialize(obj));
				}
			}
			return result;
		}
		if (any.IsArray()) {
			int n = any.ArraySize();
			for (int i = 0; i < n; i++) {
				result.push_back(Serialize(any.Member(i)));
			}
			return result;
		}
		if (Any p = any.Parent()) {
			result[PARENT_KEY_NAME] = Serialize(p);
		}
		auto fieldList = any.cls->GetFields(refl::FIND_ALL_MEMBER, Name(""));
		for (auto& field : fieldList) {
			YAML::Node child = Serialize(any.Member(field));
			result[field.name.ToStringView()] = child;
		}
		return result;
	}
	inline bool YamlArchive::Deserialize(const YAML::Node& res, Any any)
	{
		if (!any) {
			return false;
		}
		auto it_func = any.FindVtable<YamlVTable::Read>(VYamlSerdeRead());
		if (it_func) {
			return it_func(res, any.ptr);
		}
		if (any.IsEnum()) {
			return Deserialize(res, any.Parent());
		}
		if (any.Check(meta_info<Any>()) && res) {
			auto __class = res[CLASS_KEY_NAME];
			if (!__class) {
				return false;
			}
			auto name = __class.Scalar();
			auto cls = refl::find_info(name);
			if (cls) {
				Any obj = cls->New(FramePool());
				*any.CastTo<Any*>() = obj;
				return Deserialize(res[DATA_KEY_NAME], obj);
			}
			*any.CastTo<Any*>() = {};
			return false;
		}
		if (any.IsContainer()) {
			refl::Container docker(any);
			docker.construct();
			Any any = docker.malloc(FramePool());
			bool isMap = res.IsMap() && any.IsMap();
			Any first, second;
			if (isMap) {
				auto fieldList = any.ContainerType()->GetFields(refl::FIND_ALL_MEMBER, Name(""));
				first = any.Member(fieldList[0]);
				second = any.Member(fieldList[1]);
			}
			for (auto it : res) {
				if (isMap) {
					Deserialize(it.first, first);
					Deserialize(it.second, second);
				}
				else {
					Deserialize(it, any);
				}
				docker.insert(any);
				any.Destruct();
			}
			return true;
		}
		if (res.IsSequence() && any.IsArray()) {
			for (std::size_t i = 0; i < res.size(); i++) {
				Deserialize(res[i], any.Member(i));
			}
			return true;
		}
		if (res.IsMap() && any.IsObject()) {
			auto fieldList = any.cls->GetFields(refl::FIND_ALL_MEMBER, Name(""));
			auto rt = res.begin();
			int diff = res.size() - fieldList.size();
			if (diff > 1 || (diff == 1 && !any.HasParent())) {
				return false;
			}
			if (diff == 1) {
				Deserialize(rt++->second, any.Parent());
			}
			auto ft = fieldList.begin();
			for (; ft != fieldList.end() && rt != res.end(); ++ft, ++rt) {
				if (ft->name != Name(rt->first.Scalar())) {
					return false;
				}
				Deserialize(rt->second, any.Member(*ft));
			}
			return true;
		}
		if (res.IsScalar() && any.IsObject()) {
			const std::string& str = res.Scalar();
			return any.Construct(Any{ str });
		}
		return false;
	}
}