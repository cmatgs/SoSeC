#pragma once
#include <wx/string.h>
namespace sosesta::util { inline wxString U(const char* s){ return wxString::FromUTF8(s); } }
