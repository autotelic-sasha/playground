#pragma once
#include <chrono>
#include <iostream>
#include <iomanip>
#include "asserts.h"
#include "std_pretty_printing.h"

namespace autotelica {
	namespace timing {
		class timer {
			using clock_t = std::chrono::high_resolution_clock;
			using duration_t = std::chrono::nanoseconds;
			using time_t = std::chrono::time_point<clock_t, duration_t>;

			time_t _start;
			time_t _end;

		public:

			inline void start() { _start = clock_t::now(); }
			inline void stop() { _end = clock_t::now(); }

			inline duration_t period() const { return std::chrono::duration_cast<duration_t>(_end - _start); }
		};

		class duration_formatter {
			using duration_t = std::chrono::nanoseconds;

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
			duration_formatter(duration_t const& duration_) : _duration(duration_){}
			template<typename in_duration_t>
			static duration_formatter create(in_duration_t const& duration_) {
				return duration_formatter(std::chrono::duration_cast<duration_t>(duration_));
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

			std::ostream& to_stream(std::ostream& out) const {
				out << std::setw(3) << std::setfill('0') <<
					hours() << "h" <<
					minutes() << "m" <<
					seconds() << "s" <<
					milliseconds() << "ms" <<
					microseconds() << "us" <<
					nanoseconds() << "ns";
				return out;
			}

			void add_to_row(std::vector<std::string>& out) const {
				out.push_back(std::to_string(hours()));
				out.push_back(std::to_string(minutes()));
				out.push_back(std::to_string(seconds()));
				out.push_back(std::to_string(milliseconds()));
				out.push_back(std::to_string(microseconds()));
				out.push_back(std::to_string(nanoseconds()));
			}
		};
		std::ostream& operator<<(std::ostream& out, duration_formatter const& pf) {
			return pf.to_stream(out);
		}

		class timers {
			std::map<std::string, timer> _timers;
			static std::vector<std::string> const& headings() {
				static std::vector<std::string> _headings{ "timer", "hours", "minutes", "seconds",
					"milliseconds", "microseconds", "nanoseconds" };
				return _headings;
			}
		public:
			timer& add(std::string const& name) {
				AF_ASSERT(_timers.find(name) == _timers.end(), "Timer % already exists", name);
				_timers[name] = timer();
				return _timers[name];
			}
			std::map<std::string, timer> const& get_timers() const { return _timers; }
			
			timer& get_timer(std::string const& name) {
				AF_ASSERT(_timers.find(name) != _timers.end(), "Timer % does not exists", name);
				return _timers[name];
			}

			std::vector<std::vector<std::string>> to_table() const {
				std::vector<std::vector<std::string>> out;
				out.push_back(headings());
				for (auto const& e : _timers) {
					std::vector<std::string> row{ e.first };
					duration_formatter(e.second.period()).add_to_row(row);
					out.push_back(row);
				}
				return out;
			}
			std::string pretty() const {
				using namespace autotelica::std_pretty_printing;
				return table_s(to_table(), true);
			}
		};
		std::ostream& operator<<(std::ostream& out, timers const& ts) {
			using namespace autotelica::std_pretty_printing;
			out << table_s(ts.to_table(), true);
			return out;
		}
	}
}
