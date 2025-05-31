#pragma once
#include "af_xloper_data.h"

namespace autotelica {
	namespace xloper {
	namespace xl_util {

		// called_from_wizard detects if a function was invoked from Excel function wizard
		struct called_from_wizard {
		private:
			// nasty but standard way to stop functions evaluating in the wizard dialog
			struct wizard_detection_impl {
				// detecting function wizard calls, taken from: https://learn.microsoft.com/en-us/office/client-developer/excel/how-to-call-xll-functions-from-the-function-wizard-or-replace-dialog-boxes
				// Data structure used as input to xldlg_enum_proc(), called by
				// called_from_paste_fn_dlg(), called_from_replace_dlg(), and
				// called_from_Excel_dlg(). These functions tell the caller whether
				// the current worksheet function was called from one or either of
				// these dialog boxes.
				typedef struct
				{
					bool is_dlg;
					short low_hwnd;
					char const* const window_title_text; // set to NULL if don't care
				}
				xldlg_enum_struct;

				// The callback function called by Windows for every top-level window.
				static BOOL CALLBACK xldlg_enum_proc(HWND hwnd, xldlg_enum_struct* p_enum)
				{
					auto constexpr CLASS_NAME_BUFFSIZE = 50;
					auto constexpr WINDOW_TEXT_BUFFSIZE = 50;
					// Check if the parent window is Excel.
					// Note: Because of the change from MDI (Excel 2010)
					// to SDI (Excel 2013), comment out this step in Excel 2013.
					//if (LOWORD((DWORD)GetParent(hwnd)) != p_enum->low_hwnd)
					//	return TRUE; // keep iterating
					char class_name[CLASS_NAME_BUFFSIZE + 1];
					//  Ensure that class_name is always null terminated for safety.
					class_name[CLASS_NAME_BUFFSIZE] = 0;
					GetClassNameA(hwnd, class_name, CLASS_NAME_BUFFSIZE);
					//  Do a case-insensitve comparison for the Excel dialog window
					//  class name with the Excel version number truncated.
					size_t len; // The length of the window's title text
					if (_strnicmp(class_name, "bosa_sdm_xl", 11) == 0)
					{
						// Check if a searching for a specific title string
						if (p_enum->window_title_text)
						{
							// Get the window's title and see if it matches the given text.
							char buffer[WINDOW_TEXT_BUFFSIZE + 1];
							buffer[WINDOW_TEXT_BUFFSIZE] = 0;
							len = GetWindowTextA(hwnd, buffer, WINDOW_TEXT_BUFFSIZE);
							if (len == 0) // No title
							{
								if (p_enum->window_title_text[0] != 0)
									return TRUE; // No match, so keep iterating
							}
							// Window has a title so do a case-insensitive comparison of the
							// title and the search text, if provided.
							else if (p_enum->window_title_text[0] != 0
								&& _stricmp(buffer, p_enum->window_title_text) != 0)
								return TRUE; // Keep iterating
						}
						p_enum->is_dlg = true;
						return FALSE; // Tells Windows to stop iterating.
					}
					return TRUE; // Tells Windows to continue iterating.
				}
				static bool called_from_paste_fn_dlg(void)
				{
					// commented out parts are not actually used in versions of excel higher than 10
					// leaving them in because this piece of code is very often cited in documentation, books, forums ... 
					//XLOPER xHwnd;
					// Calls Excel12, which only returns the low part of the Excel
					// main window handle. This is OK for the search however.

					//if (Excel12(xlGetHwnd, &xHwnd, 0))
					//	return false; // Couldn't get it, so assume not
					// Search for bosa_sdm_xl* dialog box with title Function Arguments.
					//xldlg_enum_struct es = { FALSE, xHwnd.val.w, "Function Arguments" };

					wizard_detection_impl::xldlg_enum_struct es = { FALSE, 0, "Function Arguments" };
					EnumWindows((WNDENUMPROC)wizard_detection_impl::xldlg_enum_proc, (LPARAM)&es);
					return es.is_dlg;
				}
			};
		public:
			static bool check() {
				return wizard_detection_impl::called_from_paste_fn_dlg();
			}
			operator bool() const {
				return check();
			}
		};

		// display a message box
		inline void alert(std::string const& text, std::string const& title = "Warning") {
			MessageBoxA(NULL, text.c_str(), title.c_str(), MB_ICONEXCLAMATION | MB_OK | MB_SETFOREGROUND);
		}

		// transposing linear and simple grids
		template< typename TValue>
		struct xl_transposed {
			using value_type = TValue;
			using stripped_value_type = typename std::remove_reference<typename TValue>::type;

			stripped_value_type const _value;
			xl_transposed(stripped_value_type const& value_ = stripped_value_type()) :_value(value_) {}
			stripped_value_type const& value() const { return _value; }
			operator stripped_value_type const& () const { return _value; }
		};

		template<typename TValue>
		inline xl_transposed<TValue> xl_transpose(TValue const& value) { return xl_transposed<TValue>{value}; }

		// sometimes you may want to transpose an XLOPER12 itself
		static LPXLOPER12 xl_transpose(LPXLOPER12 in) {
			if (!xl_inner::xl_type_ops::is_xl_type(*in, xltypeMulti))
				return in;
			if (((in->val.array.rows) * (in->val.array.columns)) == 0)
				return in;

			LPXLOPER12 out = xl_inner::xl_memory::new_xloper12();
			xl_inner::xl_type_ops::set_xl_type(*out, xltypeMulti);
			out->val.array.columns = in->val.array.rows;
			out->val.array.rows = in->val.array.columns;
			for (size_t r = 0; r < out->val.array.rows; ++r) {
				for (size_t c = 0; c < out->val.array.columns; ++c) {
					size_t i_out = c + r * out->val.array.columns;
					size_t i_in = r + c * in->val.array.columns;
					xl_inner::xl_memory::clone_value(in->val.array.lparray[i_in], out->val.array.lparray[i_out]);
				}
			}
			return in;
		}
	}
	}
}