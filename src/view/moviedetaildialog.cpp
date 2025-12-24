#include "moviedetaildialog.h"
#include <QMessageBox>

// 构造函数：创建电影详情对话框
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

// 设置电影信息（从合并字段解析）
void MovieDetailDialog::setMovieInfo(const QString &movieName, const QString &description)
{
    movieNameLabel->setText(movieName);
    
    // 解析movie_details格式的字符串
    QStringList lines = description.split('\n');
    QString desc;
    QString dir;
    QString act;
    QString gen;
    double rat = 0.0;
    
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("导演: ")) {
            dir = trimmed.mid(4).trimmed();
        } else if (trimmed.startsWith("主演: ")) {
            act = trimmed.mid(4).trimmed();
        } else if (trimmed.startsWith("类型: ")) {
            gen = trimmed.mid(4).trimmed();
        } else if (trimmed.startsWith("评分: ")) {
            rat = trimmed.mid(4).trimmed().toDouble();
        } else if (!trimmed.isEmpty()) {
            // 不是特定字段，当作剧情简介
            if (!desc.isEmpty()) desc += "\n";
            desc += trimmed;
        }
    }
    
    descriptionEdit->setPlainText(desc);
    directorEdit->setText(dir);
    actorsEdit->setText(act);
    genreEdit->setText(gen);
    ratingSpin->setValue(rat);
}

// 切换编辑模式（输入或只读）
void MovieDetailDialog::setEditMode(bool editMode)
{
    m_editMode = editMode;
    
    directorEdit->setReadOnly(!editMode);
    actorsEdit->setReadOnly(!editMode);
    genreEdit->setReadOnly(!editMode);
    ratingSpin->setReadOnly(!editMode);
    descriptionEdit->setReadOnly(!editMode);
    
    if (editMode) {
        setWindowTitle("编辑电影详情");
        okBtn->setText("保存");
    } else {
        setWindowTitle("电影详情");
        okBtn->setText("确定");
    }
}

// 获取合并字段（用于保存）
QString MovieDetailDialog::getFullDetails() const
{
    QStringList parts;
    
    // 剧情简介
    QString desc = descriptionEdit->toPlainText().trimmed();
    if (!desc.isEmpty()) {
        parts << desc;
    }
    
    // 导演
    QString director = directorEdit->text().trimmed();
    if (!director.isEmpty()) {
        parts << ("导演: " + director);
    }
    
    // 主演
    QString actors = actorsEdit->text().trimmed();
    if (!actors.isEmpty()) {
        parts << ("主演: " + actors);
    }
    
    // 类型
    QString genre = genreEdit->text().trimmed();
    if (!genre.isEmpty()) {
        parts << ("类型: " + genre);
    }
    
    // 评分
    double rating = ratingSpin->value();
    if (rating > 0.0) {
        parts << ("评分: " + QString::number(rating, 'f', 1));
    }
    
    return parts.join("\n");
}