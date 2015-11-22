#include <QApplication>

#include <QEvent>
#include <QClipboard>
#include <QTextStream>
#include <QtDebug>

#include <unistd.h>
#include <stdio.h>


class Clipper : public QObject
{
public:
    Clipper (bool in, FILE *file, bool closeIt)
        : mIn(in), mCloseFile(closeIt), mFile(file) { }
    ~Clipper ();

    virtual bool event (QEvent *e);

    bool mIn, mCloseFile;
    FILE *mFile;
};


Clipper::~Clipper ()
{
    if (mCloseFile)
        if (fclose(mFile) != 0)
            perror("fclose");
}


bool Clipper::event (QEvent *e)
{
    if (e->type() == QEvent::User)
    {
        e->accept();

        if (mIn)
        {
            QTextStream s(mFile, QIODevice::ReadOnly);
            QString input;
            input = s.readAll();
            if (!input.isEmpty())
                qApp->clipboard()->setText(input);
        }
        else
        {
            QTextStream s(mFile, QIODevice::WriteOnly);
            s << qApp->clipboard()->text();
        }

        qApp->quit();
        return true;
    }
    return QObject::event(e);
}


void usage ()
{
    QTextStream s(stdout);
    s << "\nUsage: qclip [options] [file]\n\n"

      << "Read or write the clipboard to standard I/O or file\n\n"

      << "Options:\n\n"

      << "   -i    Read text into the clipboard (the default).\n"
      << "   -o    Write clipboard text out.\n"
      << "   -h    Show this message.\n\n";
}


int main (int argc, char **argv)
{
    bool in = true;
    int res = -1;

    do
    {
        res = getopt(argc, argv, "ioh");
        switch(res)
        {
        case -1 :
            break;
        case 'i' :
            in = true;
            break;
        case 'o' :
            in = false;
            break;
        case 'h' :
            usage();
            break;
        default :
            return -1;
        }
    } while(res != -1);

    FILE *f;
    bool stdFile = true;
    if (optind < argc)
    {
        f = fopen(argv[optind], in ? "r" : "w");
        if (f == NULL)
        {
            perror("open");
            return -1;
        }
        stdFile = false;
    }
    else
    {
        f = in ? stdin : stdout;
    }

    QApplication a(argc, argv);

    Clipper c(in, f, stdFile);
    a.postEvent(&c, new QEvent(QEvent::User));

    return a.exec();
}
