#pragma once
#include "af_xloper/af_xloper_data.h"
#include <map>
#include <unordered_map>

namespace autotelica {
	namespace xloper {

	namespace xl_object_caches {

		// A cache for storing objects and accessing them by a versioned tag
		// A tag is the user given name for the object followed by ":" and a version number. 
		// The version number starts at 0 when you first instert the object into cache, then gets incremented when the object is updated.
		struct xl_objects_cache_base {
			virtual size_t size() const = 0;
			virtual void clear_cache() = 0;
			virtual std::list<std::string> list_cache() const = 0;
		};

		// Caches are typed
		template<typename ObjectT>
		class xl_objects_cache : public xl_objects_cache_base {
			struct tagged_object {
				std::shared_ptr<ObjectT> _object;
				std::string _name;
				size_t _version;
				const char* const _version_separator = ":";
				std::string tag() const {
					char t[10];
					itoa(_version, t, 10);
					return _name + _version_separator + t;
				}
				static std::string name_from_tag(std::string tag_) {
					std::size_t found = tag_.find_last_of(_version_separator);
					if (found == std::string::npos)
						return tag_;
					return tag_.substr(0, found);

				}
			};
			template<typename ObjecT>
			struct xl_objects_cache_impl {
				std::unordered_map<std::string, tagged_object> _cache;
			};
			static xl_objects_cache_impl<ObjectT>& instance() {
				static xl_objects_cache_impl<ObjectT> _inst;
				return _inst;
			}
		public:
			std::shared_ptr<ObjectT> get(std::string const& name_or_tag) {
				std::string name = tagged_object::name_from_tag(name_or_tag);
				auto it = instance()._cache.find(name);
				if (it == instance()._cache.end())
					return nullptr;
				return it->second._object;
			}

			std::string add(std::string const& name_or_tag, std::shared_ptr<ObjectT> object) {
				std::string name = tagged_object::name_from_tag(name_or_tag);
				auto it = instance()._cache.find(name);
				if (it == instance()._cache.end()) {
					tagged_object entry{ object, name, 0 };
					instance()._cache[name] = entry;
				}
				else {
					tagged_object entry(it->second);
					entry._version++;
					entry._object = object;
					instance()._cache[name] = entry;
				}
			}

			std::string update(std::string const& name_or_tag, std::shared_ptr<ObjectT> object) {
				std::string name = tagged_object::name_from_tag(name_or_tag);
				auto it = instance()._cache.find(name);
				if (it == instance()._cache.end()) {
					throw std::runtime_error(std::string("Object ") + name + " doesn't exist when trying to update it.");
				}
				else {
					tagged_object entry(it->second);
					entry._version++;
					entry._object = object;
					instance()._cache[name] = entry;
				}
			}
			void clear() {
				instance()._cache.clear();
			}

			size_t size() const override { return instance()->_cache.size(); }
			void clear_cache() override { clear(); }
			std::list<std::string> list_cache() const override {
				std::list<std::string> ret;
				for (auto const& c : instance()->_cache)
					ret.push_back(c.first);
				return ret;
			}
		};

		// to provide simple ways to list and clear caches
		// we have cache of caches
		class xl_objects_caches {
			using cache_cache_t = std::unordered_map<std::string, std::shared_ptr<xl_objects_cache_base>>;
			static cache_cache_t& cache_cache() {
				static cache_cache_t inst;
				return inst;
			}
		public:
			static bool exists(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				return it != cache_cache().end();
			}
			template<typename ObjectT>
			static xl_objects_cache<ObjectT>& get(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				if (it == cache_cache().end()) {
					std::shared_ptr<xl_objects_cache<ObjectT>> ret(new xl_objects_cache<ObjectT>());
					cache_cache()[cache_name] = ret;
					return ret;
				}
				else {
					std::shared_ptr<xl_objects_cache<ObjectT>> ret(std::dynamic_pointer_cast<xl_objects_cache<ObjectT>>(it->second));
					if (!ret)
						throw std::runtime_error(std::string("Cache ") + cache_name + (" is of wrong type."));
					return ret;
				}
			}
			template<typename ObjectT>
			static std::shared_ptr<ObjectT> get(std::string const& cache_name, std::string const& object_name_or_tag) {
				return get<ObjectT>(cache_name).get(object_name_or_tag);
			}

			template<typename ObjectT>
			static std::string add(std::string const& cache_name, std::string const& object_name_or_tag, std::shared_ptr<ObjectT> object) {
				return get<ObjectT>(cache_name).add(object_name_or_tag, object);
			}
			template<typename ObjectT>
			static std::string update(std::string const& cache_name, std::string const& object_name_or_tag, std::shared_ptr<ObjectT> object) {
				return get<ObjectT>(cache_name).update(object_name_or_tag, object);
			}

			static size_t cache_size(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				if (it == cache_cache().end())
					return 0;
				else
					return it->second->size();
			}
			static void clear_cache(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				if (it != cache_cache().end())
					it->second->clear_cache();
			}
			static std::list<std::string> list_cache(std::string const& cache_name) {
				auto it = cache_cache().find(cache_name);
				if (it != cache_cache().end())
					return it->second->list_cache();
				return {};
			}
			static std::unordered_map<std::string, size_t> cache_sizes() {
				std::unordered_map<std::string, size_t> ret;
				for (auto const& c : cache_cache())
					ret[c.first] = c.second->size();
				return ret;
			}
			static void clear_all_caches() {
				for (auto const& c : cache_cache())
					c.second->clear_cache();
			}
			static std::unordered_map<std::string, std::list<std::string>> list_all_caches() {
				std::unordered_map<std::string, std::list<std::string>> ret;
				for (auto const& c : cache_cache())
					ret[c.first] = c.second->list_cache();
				return ret;
			}
		};
		// these are exposed to XLLs by default
		inline size_t af_xl_object_cache_size(std::string const& cache_name) { return xl_objects_caches::cache_size(cache_name); }

		inline std::unordered_map<std::string, size_t> af_xl_object_cache_sizes() { return xl_objects_caches::cache_sizes(); }

		inline bool af_xl_clear_object_cache(std::string const& cache_name) { xl_objects_caches::clear_cache(cache_name); return true; }

		inline bool af_xl_clear_all_object_caches() { xl_objects_caches::clear_all_caches(); return true; }

		inline std::list<std::string> af_xl_list_objects_cache(std::string const& cache_name) { return xl_objects_caches::list_cache(cache_name); }

		inline std::unordered_map<std::string, std::list<std::string>> af_xl_list_all_objects_caches() { return xl_objects_caches::list_all_caches(); }

	
	}
	
}
}