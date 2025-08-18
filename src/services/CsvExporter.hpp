#pragma once
#include <wx/window.h>
#include <wx/dataview.h>
#include "services/LoggerService.hpp"

// Exportiert beliebige wxDataViewListCtrl-Inhalte als CSV
class CsvExporter {
public:
    explicit CsvExporter(LoggerService& logger) : logger_(logger) {}

    // Öffnet Dateidialog, schreibt UTF-8 mit BOM (Excel-freundlich).
    // default_filename z.B. "ereignis_log.csv"
    bool ExportDataViewToCSV(wxWindow* parent,
                             wxDataViewListCtrl* view,
                             const wxString& default_filename = wxString("ereignis_log.csv"));

private:
    LoggerService& logger_;
};
