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

// 电影详情显示与编辑对话框
class MovieDetailDialog : public QDialog {
    Q_OBJECT
public:
    explicit MovieDetailDialog(QWidget *parent = nullptr);
    
    // 设置电影信息(查看模式)
    void setMovieInfo(const QString &movieName, const QString &description);
    
    // 获取详情字段
    QString getDescription() const { return descriptionEdit->toPlainText(); }
    QString getDirector() const { return directorEdit->text(); }
    QString getActors() const { return actorsEdit->text(); }
    QString getGenre() const { return genreEdit->text(); }
    double getRating() const { return ratingSpin->value(); }
    
    // 获取合并的完整详情
    QString getFullDetails() const;
    
    // 切换编辑/查看模式
    void setEditMode(bool editMode);

private:
    QLabel *movieNameLabel;
    QTextEdit *descriptionEdit;
    QLineEdit *directorEdit;
    QLineEdit *actorsEdit;
    QLineEdit *genreEdit;
    QDoubleSpinBox *ratingSpin;
    QPushButton *okBtn;
    QPushButton *cancelBtn;
    
    bool m_editMode;
};