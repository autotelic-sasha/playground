#pragma once
#include <chrono>
#include <iostream>
#include <iomanip>
#include "asserts.h"
#include "std_pretty_printing.h"

namespace autotelica {
	namespace timing {
		template<typename char_t>
		struct timing_strings;

		template<>
		struct timing_strings<char> {
			static constexpr auto zero = '0';
			static constexpr auto h = "h";
			static constexpr auto m = "m";
			static constexpr auto s = "s";
			static constexpr auto ms = "ms";
			static constexpr auto us = "us";
			static constexpr auto ns = "ns";
			static constexpr auto timer = "timer";
			static constexpr auto hours = "hours";
			static constexpr auto minutes = "minutes";
			static constexpr auto seconds = "seconds";
			static constexpr auto milliseconds = "milliseconds";
			static constexpr auto microseconds = "microseconds";
			static constexpr auto nanoseconds = "nanoseconds";
			static constexpr auto already_exists = "Timer % already exists";
			static constexpr auto does_not_exists = "Timer % does not exists";
		};
		template<>
		struct timing_strings<wchar_t> {
			static constexpr auto zero = L'0';
			static constexpr auto h = L"h";
			static constexpr auto m = L"m";
			static constexpr auto s = L"s";
			static constexpr auto ms = L"ms";
			static constexpr auto us = L"us";
			static constexpr auto ns = L"ns";
			static constexpr auto timer = L"timer";
			static constexpr auto hours = L"hours";
			static constexpr auto minutes = L"minutes";
			static constexpr auto seconds = L"seconds";
			static constexpr auto milliseconds = L"milliseconds";
			static constexpr auto microseconds = L"microseconds";
			static constexpr auto nanoseconds = L"nanoseconds";
			static constexpr auto already_exists = L"Timer % already exists";
			static constexpr auto does_not_exists = L"Timer % does not exists";
		};

		template<typename string_t = std::string>
		class timer_ {
			using char_t = typename string_t::value_type;
			using stream_t = std::basic_ostream<char_t>;
			using clock_t = std::chrono::high_resolution_clock;
			using duration_t = std::chrono::nanoseconds;
			using time_t = std::chrono::time_point<clock_t, duration_t>;
			using strings = timing_strings<char_t>;
			time_t _start;
			time_t _end;

		public:

			inline void start() { _start = clock_t::now(); }
			inline void stop() { _end = clock_t::now(); }

			inline duration_t period() const { return std::chrono::duration_cast<duration_t>(_end - _start); }
		};
		using timer = timer_<std::string>;
		using wtimer = timer_<std::wstring>;
		
		template<typename string_t = std::string>
		class duration_formatter_ {
			using duration_t = std::chrono::nanoseconds;
			using char_t = typename string_t::value_type;
			using stream_t = std::basic_ostream<char_t>;
			using strings = timing_strings<char_t>;

			const duration_t _duration;

			template< typename target_res_t, typename previous_res_t>
			short only() const{
				using namespace std::chrono;
				return static_cast<short>(
					(duration_cast<target_res_t>(_duration) -
						duration_cast<target_res_t>(
							duration_cast<previous_res_t>(_duration))).count());
			}
		public:
			duration_formatter_(duration_t const& duration_) : _duration(duration_){}
			template<typename in_duration_t>
			static duration_formatter_ create(in_duration_t const& duration_) {
				return duration_formatter_(std::chrono::duration_cast<duration_t>(duration_));
			}

			template< typename resolution>
			inline int64_t as() const {
				using namespace std::chrono;
				return duration_cast<resolution>(_duration).count();
			}

			inline short nanoseconds() const {
				return only<std::chrono::nanoseconds, std::chrono::microseconds>();
			}
			inline short microseconds() const {
				return only<std::chrono::microseconds, std::chrono::milliseconds>();
			}
			inline short milliseconds() const {
				return only<std::chrono::milliseconds, std::chrono::seconds>();
			}
			inline short seconds() const {
				return only<std::chrono::seconds, std::chrono::minutes>();
			}
			inline short minutes() const {
				return only<std::chrono::minutes, std::chrono::hours>();
			}
			inline short hours() const {
				return static_cast<short>(as<std::chrono::hours>());
			}

			stream_t& to_stream(stream_t& out) const {
				out << std::setw(3) << std::setfill(strings::zero) <<
					hours() << strings::h <<
					minutes() << strings::m <<
					seconds() << strings::s <<
					milliseconds() << strings::ms <<
					microseconds() << strings::us <<
					nanoseconds() << strings::ns;
				return out;
			}

			void add_to_row(std::vector<string_t>& out) const {
				using namespace string_util;
				out.push_back(to_string_t<string_t>::convert(hours()));
				out.push_back(to_string_t<string_t>::convert(minutes()));
				out.push_back(to_string_t<string_t>::convert(seconds()));
				out.push_back(to_string_t<string_t>::convert(milliseconds()));
				out.push_back(to_string_t<string_t>::convert(microseconds()));
				out.push_back(to_string_t<string_t>::convert(nanoseconds()));
			}
		};
		using duration_formatter = duration_formatter_<std::string>;
		using wduration_formatter = duration_formatter_<std::wstring>;
		
		std::ostream& operator<<(std::ostream& out, duration_formatter_<std::string> const& pf) {
			return pf.to_stream(out);
		}
		std::wostream& operator<<(std::wostream& out, duration_formatter_<std::wstring> const& pf) {
			return pf.to_stream(out);
		}

		template<typename string_t = std::string>
		class timers_ {
			using char_t = typename string_t::value_type;
			using stream_t = std::basic_ostream<char_t>;
			using strings = timing_strings<char_t>;

			std::map<string_t, timer_<string_t>> _timers;
			static std::vector<string_t> const& headings() {
				static std::vector<string_t> _headings{ strings::timer, strings::hours, strings::minutes, strings::seconds,
					strings::milliseconds, strings::microseconds, strings::nanoseconds };
				return _headings;
			}
		public:
			timer_<string_t>& add(string_t const& name) {
				AF_ASSERT(_timers.find(name) == _timers.end(), strings::already_exists, name);
				_timers[name] = timer_<string_t>();
				return _timers[name];
			}
			std::map<string_t, timer_<string_t>> const& get_timers() const { return _timers; }
			
			timer_<string_t>& get_timer(string_t const& name) {
				AF_ASSERT(_timers.find(name) != _timers.end(), strings::does_not_exist, name);
				return _timers[name];
			}

			std::vector<std::vector<string_t>> to_table() const {
				std::vector<std::vector<string_t>> out;
				out.push_back(headings());
				for (auto const& e : _timers) {
					std::vector<string_t> row{ e.first };
					duration_formatter_<string_t>(e.second.period()).add_to_row(row);
					out.push_back(row);
				}
				return out;
			}
			string_t pretty() const {
				using namespace autotelica::std_pretty_printing;
				return table_s(to_table(), true);
			}
		};

		using timers = timers_<std::string>;
		using wtimers = timers_<std::wstring>;
		
		std::ostream& operator<<(std::ostream& out, timers_<std::string> const& ts) {
			using namespace autotelica::std_pretty_printing;
			out << table_s(ts.to_table(), true);
			return out;
		}
		std::wostream& operator<<(std::wostream& out, timers_<std::wstring> const& ts) {
			using namespace autotelica::std_pretty_printing;
			out << table_w(ts.to_table(), true);
			return out;
		}
	}
}
