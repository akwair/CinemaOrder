#include "moviedetaildialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>
#include <QHBoxLayout>

MovieDetailDialog::MovieDetailDialog(QWidget *parent)
    : QDialog(parent), m_editMode(false)
{
    setWindowTitle("电影详情");
    setModal(true);
    resize(600, 500);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 电影名称
    movieNameLabel = new QLabel(this);
    movieNameLabel->setStyleSheet("font-size: 18px; font-weight: bold; margin-bottom: 10px;");
    mainLayout->addWidget(movieNameLabel);
    
    // 基本信息组
    QGroupBox *basicGroup = new QGroupBox("基本信息", this);
    QFormLayout *basicLayout = new QFormLayout(basicGroup);
    
    directorEdit = new QLineEdit(this);
    basicLayout->addRow("导演:", directorEdit);
    
    actorsEdit = new QLineEdit(this);
    basicLayout->addRow("主演:", actorsEdit);
    
    genreEdit = new QLineEdit(this);
    basicLayout->addRow("类型:", genreEdit);
    
    ratingSpin = new QDoubleSpinBox(this);
    ratingSpin->setRange(0.0, 10.0);
    ratingSpin->setSingleStep(0.1);
    ratingSpin->setDecimals(1);
    basicLayout->addRow("评分:", ratingSpin);
    
    mainLayout->addWidget(basicGroup);
    
    // 海报组
    QGroupBox *posterGroup = new QGroupBox("海报", this);
    QHBoxLayout *posterLayout = new QHBoxLayout(posterGroup);
    
    posterEdit = new QLineEdit(this);
    posterEdit->setPlaceholderText("选择海报文件...");
    posterLayout->addWidget(posterEdit);
    
    posterBtn = new QPushButton("选择文件", this);
    connect(posterBtn, &QPushButton::clicked, this, &MovieDetailDialog::onSelectPoster);
    posterLayout->addWidget(posterBtn);
    
    mainLayout->addWidget(posterGroup);
    
    // 剧情简介组
    QGroupBox *plotGroup = new QGroupBox("剧情简介", this);
    QVBoxLayout *plotLayout = new QVBoxLayout(plotGroup);
    
    descriptionEdit = new QTextEdit(this);
    descriptionEdit->setMinimumHeight(100);
    plotLayout->addWidget(descriptionEdit);
    
    mainLayout->addWidget(plotGroup);
    
    // 按钮
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    okBtn = new QPushButton("确定", this);
    connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okBtn);
    
    cancelBtn = new QPushButton("取消", this);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelBtn);
    
    mainLayout->addLayout(buttonLayout);
    
    setEditMode(false);
}

void MovieDetailDialog::setMovieInfo(const QString &movieName, const QString &description,
                                   const QString &director, const QString &actors,
                                   const QString &genre, double rating, const QString &poster)
{
    movieNameLabel->setText(movieName);
    descriptionEdit->setPlainText(description);
    directorEdit->setText(director);
    actorsEdit->setText(actors);
    genreEdit->setText(genre);
    ratingSpin->setValue(rating);
    posterEdit->setText(poster);
}

void MovieDetailDialog::setEditMode(bool editMode)
{
    m_editMode = editMode;
    
    directorEdit->setReadOnly(!editMode);
    actorsEdit->setReadOnly(!editMode);
    genreEdit->setReadOnly(!editMode);
    ratingSpin->setReadOnly(!editMode);
    descriptionEdit->setReadOnly(!editMode);
    posterEdit->setReadOnly(!editMode);
    posterBtn->setEnabled(editMode);
    
    if (editMode) {
        setWindowTitle("编辑电影详情");
        okBtn->setText("保存");
    } else {
        setWindowTitle("电影详情");
        okBtn->setText("确定");
    }
}

void MovieDetailDialog::onSelectPoster()
{
    QString fileName = QFileDialog::getOpenFileName(this, "选择海报",
                                                   "",
                                                   "图片文件 (*.png *.jpg *.jpeg *.bmp *.gif)");
    if (!fileName.isEmpty()) {
        posterEdit->setText(fileName);
    }
}