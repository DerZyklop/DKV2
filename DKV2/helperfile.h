#ifndef FILEHELPER_H
#define FILEHELPER_H
#include <QString>

QString getUniqueTempFilename(QString templateFileName);

bool moveToBackup(QString fn);

bool backupFile(const QString& filename, const QString& subfolder=QString());

void showFileInFolder(const QString &fullPath);

void printHtmlToPdf( const QString html, const QString fn);

QString absoluteCanonicalPath(const QString path);

#endif // FILEHELPER_H
