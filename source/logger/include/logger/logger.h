#ifndef LOGGER_H
#define LOGGER_H

#include <QWidget>
#include <logger/logger_api.h>

namespace Ui {
class Logger;
}

class LOGGER_API Logger : public QWidget
{
    Q_OBJECT

public:
    static Logger& getInstance()
    {
        static Logger    instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }
    ~Logger();

    Logger(Logger const&)               = delete;
    void operator=(Logger const&)  = delete;
    void log(std::string logMessage);

private:
    explicit Logger(QWidget *parent = nullptr);


private:
    Ui::Logger *ui;
};

#endif // LOGGER_H
