#include "services/CsvExporter.hpp"
#include <wx/filedlg.h>
#include <wx/ffile.h>
#include <wx/msgdlg.h>
#include <wx/wx.h> 

bool CsvExporter::ExportDataViewToCSV(wxWindow* parent,
                                      wxDataViewListCtrl* view,
                                      const wxString& default_filename)
{
    if (!view) return false;

    wxFileDialog dlg(parent,
                     wxString::FromUTF8("CSV exportieren"),
                     "",
                     default_filename,
                     "CSV Dateien (*.csv)|*.csv",
                     wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (dlg.ShowModal() != wxID_OK) return false;

    wxFFile f(dlg.GetPath(), "wb");
    if (!f.IsOpened()) {
        if (parent) {
            wxMessageBox(wxString::FromUTF8("Datei konnte nicht geschrieben werden."),
                         "Fehler", wxICON_ERROR | wxOK, parent);
        }
        logger_.Log("CSV", "Schreiben fehlgeschlagen: " + dlg.GetPath(), "ERROR");
        return false;
    }

    // UTF‑8 BOM (Excel freundlich)
    static const unsigned char bom[3] = {0xEF, 0xBB, 0xBF};
    f.Write(bom, 3);

    auto W = [&](const wxString& s){ f.Write(s.ToUTF8()); };

    // Header
    const unsigned cols = view->GetColumnCount();
    for (unsigned c = 0; c < cols; ++c) {
        if (c) W(",");
        W("\"" + view->GetColumn(c)->GetTitle() + "\"");
    }
    W("\n");

    // Rows
    const unsigned rows = view->GetItemCount();
    for (unsigned r = 0; r < rows; ++r) {
        for (unsigned c = 0; c < cols; ++c) {
            if (c) W(",");

            wxVariant v;
            view->GetValue(v, r, c);
            wxString cell = v.GetString();
            cell.Replace("\"", "\"\""); // CSV escaping
            W("\"" + cell + "\"");
        }
        W("\n");
    }

    logger_.Log("CSV", "Exportiert: " + dlg.GetPath(), "OK");
    return true;
}
