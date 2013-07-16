#include "qt/rogue-gui.cpp"

int main(int argc, char *argv[]) {
    QFile file("src/qt/stylesheet.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());
    QApplication app(argc, argv);
    app.setStyleSheet(styleSheet);

    rogueGui window;
    //window.resize(900, 500);
    window.setWindowTitle("rogue");
    window.show();
    return app.exec();
}
