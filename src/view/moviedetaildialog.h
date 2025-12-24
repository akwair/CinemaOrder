#pragma once
#include <QDialog>
#include <QLabel>
#include <QTextEdit>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>

class MovieDetailDialog : public QDialog {
    Q_OBJECT
public:
    explicit MovieDetailDialog(QWidget *parent = nullptr);
    
    // 设置电影信息（查看模式）
    void setMovieInfo(const QString &movieName, const QString &description, 
                     const QString &director, const QString &actors,
                     const QString &genre, double rating, const QString &poster);
    
    // 获取电影信息（编辑模式）
    QString getDescription() const { return descriptionEdit->toPlainText(); }
    QString getDirector() const { return directorEdit->text(); }
    QString getActors() const { return actorsEdit->text(); }
    QString getGenre() const { return genreEdit->text(); }
    double getRating() const { return ratingSpin->value(); }
    QString getPoster() const { return posterEdit->text(); }
    
    // 设置编辑模式
    void setEditMode(bool editMode);

private slots:
    void onSelectPoster();

private:
    QLabel *movieNameLabel;
    QTextEdit *descriptionEdit;
    QLineEdit *directorEdit;
    QLineEdit *actorsEdit;
    QLineEdit *genreEdit;
    QDoubleSpinBox *ratingSpin;
    QLineEdit *posterEdit;
    QPushButton *posterBtn;
    QPushButton *okBtn;
    QPushButton *cancelBtn;
    
    bool m_editMode;
};