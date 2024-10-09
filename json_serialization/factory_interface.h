#pragma once
#include "type_description.h"

struct factory_base {

};

struct factory_example {
	
	template<typename target_t>
	static default_value_p<target_t> make_default_handler(target_t const* default_value_){
		if (!default_value_) return nullptr;
		return std::make_shared<default_value_t<target_t>>(default_value_);
	}

	template<typename target_t>
	static serialization_handler_p make_member_handler(
		target_t* target_,
		target_t const* default_value_,
		target_t const* contained_default_value_
	) {

		return serialization_handler_p(
			create_example_handler(
				target_,
				make_default_handler(default_value_),
				make_default_handler(contained_default_value_));
	}

	template<typename object_t>
	static serialization_handler_p make_object_handler(
		object_t& object_,
		object_t* default_,
		std::vector<std::pair<traits::key_t, serialization_handler_p>> const& handlers_,
		std::function<void()> pre_load_f_,
		std::function<void()> post_load_f_,
		std::function<void()> pre_save_f_,
		std::function<void()> post_save_f_
	) {

	}
};


