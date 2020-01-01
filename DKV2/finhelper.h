#ifndef FINHELPER_H
#define FINHELPER_H
#include <QMap>
#include <QDate>
#include <QRegExpValidator>

double round(const double d, const int stellen = 2);
double round2(const double d);
double round6(const double d);

int TageBisJahresende_a(const QDate& d);
int TageBisJahresende(const QDate& d);
int TageSeitJahresAnfang_a(const QDate& d);
int TageSeitJahresAnfang(const QDate& d);

double ZinsesZins(const double zins, const double wert,const QDate von, const QDate bis, const bool tesa=true);

// taken from https://github.com/Al-/IbanValidator
class IbanValidator : public QRegExpValidator
{
public:
    explicit IbanValidator();
    virtual void fixup (QString& input) const;
    virtual State validate (QString& input, int& pos) const;
private:
    unsigned int mod97(const QString& input) const;
};


#endif // FINHELPER_H
