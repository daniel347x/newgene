#ifndef IMPORTDIALOGHELPER_H
#define IMPORTDIALOGHELPER_H

namespace ImportDialogHelper
{

    void AddFileChooserBlock(QFormLayout & form, QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> const & fileChooserStrings);
    bool ValidateFileChooserBlock(QList<QLineEdit *> & fieldsFileChooser, std::vector<std::string> & dataFileChooser, std::string & errorMsg);

    void AddTimeRangeSelectorBlock(QFormLayout & form, QList<QLineEdit *> & fieldsTimeRange);
    bool ValidateTimeRangeBlock(QList<QLineEdit *> & fieldsTimeRange, std::vector<std::string> & dataTimeRange, std::string & errorMsg);

}

#endif // IMPORTDIALOGHELPER_H
