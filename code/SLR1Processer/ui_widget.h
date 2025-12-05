/********************************************************************************
** Form generated from reading UI file 'widget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.17
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_WIDGET_H
#define UI_WIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_Widget
{
public:
    QWidget *widget;
    QWidget *widget_3;
    QWidget *widget_10;
    QHBoxLayout *horizontalLayout_2;
    QSpacerItem *horizontalSpacer_3;
    QLabel *label_3;
    QPushButton *pushButton_7;
    QSpacerItem *horizontalSpacer_4;
    QPlainTextEdit *plainTextEdit_2;
    QWidget *widget_15;
    QHBoxLayout *horizontalLayout_5;
    QSpacerItem *horizontalSpacer_15;
    QPushButton *pushButton_3;
    QSpacerItem *horizontalSpacer_23;
    QPushButton *pushButton_4;
    QSpacerItem *horizontalSpacer_16;
    QWidget *widget_5;
    QWidget *widget_11;
    QHBoxLayout *horizontalLayout_7;
    QSpacerItem *horizontalSpacer_5;
    QLabel *label_4;
    QPushButton *pushButton_5;
    QSpacerItem *horizontalSpacer_8;
    QTableWidget *tableWidget_3;
    QWidget *widget_6;
    QWidget *widget_12;
    QHBoxLayout *horizontalLayout_8;
    QSpacerItem *horizontalSpacer_9;
    QLabel *label_5;
    QPushButton *pushButton_6;
    QSpacerItem *horizontalSpacer_10;
    QTableWidget *tableWidget_4;
    QWidget *widget_7;
    QWidget *widget_13;
    QHBoxLayout *horizontalLayout_6;
    QSpacerItem *horizontalSpacer_11;
    QLabel *label_6;
    QPushButton *pushButton;
    QSpacerItem *horizontalSpacer_12;
    QTableWidget *tableWidget;
    QWidget *widget_8;
    QWidget *widget_14;
    QHBoxLayout *horizontalLayout_4;
    QSpacerItem *horizontalSpacer_13;
    QLabel *label_7;
    QPushButton *pushButton_2;
    QSpacerItem *horizontalSpacer_26;
    QLabel *label_12;
    QPushButton *pushButton_10;
    QPushButton *pushButton_11;
    QSpacerItem *horizontalSpacer_14;
    QTabWidget *tabWidget_2;
    QWidget *tab_slr1;
    QTableWidget *tableWidget_2;
    QWidget *tab_lr1_dfa;
    QTableWidget *tableWidget_5;
    QWidget *tab_lr1_table;
    QTableWidget *tableWidget_6;
    QWidget *widget1;
    QTabWidget *tabWidget;
    QWidget *tab;
    QPlainTextEdit *codeText;
    QWidget *tab_2;
    QTreeWidget *treeWidget;
    QLabel *label_10;
    QPushButton *pushButton_8;
    QPushButton *pushButton_9;

    void setupUi(QWidget *Widget)
    {
        if (Widget->objectName().isEmpty())
            Widget->setObjectName(QString::fromUtf8("Widget"));
        Widget->resize(1773, 761);
        
        // 设置整体样式
        Widget->setStyleSheet(QString::fromUtf8(
            "QWidget { background-color: #f5f5f5; }"
            "QLabel { color: #333333; }"
            "QPushButton { background-color: #4a90d9; color: white; border: none; padding: 6px 12px; border-radius: 4px; }"
            "QPushButton:hover { background-color: #357abd; }"
            "QPushButton:pressed { background-color: #2a5f8f; }"
            "QTableWidget { background-color: white; border: 1px solid #ddd; gridline-color: #e0e0e0; }"
            "QTableWidget::item { padding: 4px; }"
            "QTableWidget QHeaderView::section { background-color: #4a90d9; color: white; padding: 6px; border: none; }"
            "QPlainTextEdit { background-color: white; border: 1px solid #ddd; border-radius: 4px; }"
            "QTabWidget::pane { border: 1px solid #ddd; background-color: white; }"
            "QTabBar::tab { background-color: #e0e0e0; padding: 8px 16px; margin-right: 2px; border-top-left-radius: 4px; border-top-right-radius: 4px; }"
            "QTabBar::tab:selected { background-color: #4a90d9; color: white; }"
            "QTreeWidget { background-color: white; border: 1px solid #ddd; }"
        ));
        
        widget = new QWidget(Widget);
        widget->setObjectName(QString::fromUtf8("widget"));
        widget->setGeometry(QRect(20, 10, 1731, 741));

        widget_3 = new QWidget(widget);
        widget_3->setObjectName(QString::fromUtf8("widget_3"));
        widget_3->setGeometry(QRect(0, 5, 251, 306));
        widget_3->setAutoFillBackground(false);
        widget_10 = new QWidget(widget_3);
        widget_10->setObjectName(QString::fromUtf8("widget_10"));
        widget_10->setGeometry(QRect(0, 0, 251, 41));
        horizontalLayout_2 = new QHBoxLayout(widget_10);
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        horizontalSpacer_3 = new QSpacerItem(72, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_3);

        label_3 = new QLabel(widget_10);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_2->addWidget(label_3);

        pushButton_7 = new QPushButton(widget_10);
        pushButton_7->setObjectName(QString::fromUtf8("pushButton_7"));

        horizontalLayout_2->addWidget(pushButton_7);

        horizontalSpacer_4 = new QSpacerItem(71, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_4);

        plainTextEdit_2 = new QPlainTextEdit(widget_3);
        plainTextEdit_2->setObjectName(QString::fromUtf8("plainTextEdit_2"));
        plainTextEdit_2->setGeometry(QRect(10, 80, 231, 216));
        widget_15 = new QWidget(widget_3);
        widget_15->setObjectName(QString::fromUtf8("widget_15"));
        widget_15->setGeometry(QRect(0, 40, 251, 41));
        horizontalLayout_5 = new QHBoxLayout(widget_15);
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        horizontalSpacer_15 = new QSpacerItem(72, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_15);

        pushButton_3 = new QPushButton(widget_15);
        pushButton_3->setObjectName(QString::fromUtf8("pushButton_3"));

        horizontalLayout_5->addWidget(pushButton_3);

        horizontalSpacer_23 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_23);

        pushButton_4 = new QPushButton(widget_15);
        pushButton_4->setObjectName(QString::fromUtf8("pushButton_4"));

        horizontalLayout_5->addWidget(pushButton_4);

        horizontalSpacer_16 = new QSpacerItem(71, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_16);

        widget_5 = new QWidget(widget);
        widget_5->setObjectName(QString::fromUtf8("widget_5"));
        widget_5->setGeometry(QRect(-1, 309, 251, 211));
        widget_11 = new QWidget(widget_5);
        widget_11->setObjectName(QString::fromUtf8("widget_11"));
        widget_11->setGeometry(QRect(-1, 0, 251, 41));
        horizontalLayout_7 = new QHBoxLayout(widget_11);
        horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
        horizontalSpacer_5 = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_5);

        label_4 = new QLabel(widget_11);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_7->addWidget(label_4);

        pushButton_5 = new QPushButton(widget_11);
        pushButton_5->setObjectName(QString::fromUtf8("pushButton_5"));

        horizontalLayout_7->addWidget(pushButton_5);

        horizontalSpacer_8 = new QSpacerItem(28, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_7->addItem(horizontalSpacer_8);

        tableWidget_3 = new QTableWidget(widget_5);
        tableWidget_3->setObjectName(QString::fromUtf8("tableWidget_3"));
        tableWidget_3->setGeometry(QRect(10, 40, 231, 171));
        widget_6 = new QWidget(widget);
        widget_6->setObjectName(QString::fromUtf8("widget_6"));
        widget_6->setGeometry(QRect(-1, 529, 251, 211));
        widget_12 = new QWidget(widget_6);
        widget_12->setObjectName(QString::fromUtf8("widget_12"));
        widget_12->setGeometry(QRect(-1, -1, 251, 41));
        horizontalLayout_8 = new QHBoxLayout(widget_12);
        horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
        horizontalSpacer_9 = new QSpacerItem(25, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_9);

        label_5 = new QLabel(widget_12);
        label_5->setObjectName(QString::fromUtf8("label_5"));

        horizontalLayout_8->addWidget(label_5);

        pushButton_6 = new QPushButton(widget_12);
        pushButton_6->setObjectName(QString::fromUtf8("pushButton_6"));

        horizontalLayout_8->addWidget(pushButton_6);

        horizontalSpacer_10 = new QSpacerItem(25, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_8->addItem(horizontalSpacer_10);

        tableWidget_4 = new QTableWidget(widget_6);
        tableWidget_4->setObjectName(QString::fromUtf8("tableWidget_4"));
        tableWidget_4->setGeometry(QRect(10, 40, 231, 161));

        // LR(0) DFA图区域 - 扩大表格
        widget_7 = new QWidget(widget);
        widget_7->setObjectName(QString::fromUtf8("widget_7"));
        widget_7->setGeometry(QRect(249, 5, 1051, 365));
        widget_13 = new QWidget(widget_7);
        widget_13->setObjectName(QString::fromUtf8("widget_13"));
        widget_13->setGeometry(QRect(-1, -1, 1051, 41));
        horizontalLayout_6 = new QHBoxLayout(widget_13);
        horizontalLayout_6->setObjectName(QString::fromUtf8("horizontalLayout_6"));
        horizontalSpacer_11 = new QSpacerItem(437, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_11);

        label_6 = new QLabel(widget_13);
        label_6->setObjectName(QString::fromUtf8("label_6"));

        horizontalLayout_6->addWidget(label_6);

        pushButton = new QPushButton(widget_13);
        pushButton->setObjectName(QString::fromUtf8("pushButton"));

        horizontalLayout_6->addWidget(pushButton);

        horizontalSpacer_12 = new QSpacerItem(437, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_6->addItem(horizontalSpacer_12);

        // DFA表格扩大
        tableWidget = new QTableWidget(widget_7);
        tableWidget->setObjectName(QString::fromUtf8("tableWidget"));
        tableWidget->setGeometry(QRect(10, 40, 1031, 315));

        // 分析表区域 - 扩大
        widget_8 = new QWidget(widget);
        widget_8->setObjectName(QString::fromUtf8("widget_8"));
        widget_8->setGeometry(QRect(249, 369, 1051, 371));
        widget_14 = new QWidget(widget_8);
        widget_14->setObjectName(QString::fromUtf8("widget_14"));
        widget_14->setGeometry(QRect(-1, -1, 1051, 41));
        horizontalLayout_4 = new QHBoxLayout(widget_14);
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        horizontalLayout_4->setContentsMargins(-1, 0, -1, 0);
        horizontalSpacer_13 = new QSpacerItem(200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_13);

        label_7 = new QLabel(widget_14);
        label_7->setObjectName(QString::fromUtf8("label_7"));

        horizontalLayout_4->addWidget(label_7);

        pushButton_2 = new QPushButton(widget_14);
        pushButton_2->setObjectName(QString::fromUtf8("pushButton_2"));

        horizontalLayout_4->addWidget(pushButton_2);

        horizontalSpacer_26 = new QSpacerItem(50, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_26);

        label_12 = new QLabel(widget_14);
        label_12->setObjectName(QString::fromUtf8("label_12"));

        horizontalLayout_4->addWidget(label_12);

        pushButton_10 = new QPushButton(widget_14);
        pushButton_10->setObjectName(QString::fromUtf8("pushButton_10"));

        horizontalLayout_4->addWidget(pushButton_10);

        pushButton_11 = new QPushButton(widget_14);
        pushButton_11->setObjectName(QString::fromUtf8("pushButton_11"));

        horizontalLayout_4->addWidget(pushButton_11);

        horizontalSpacer_14 = new QSpacerItem(200, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_14);

        // 分析表TabWidget扩大
        tabWidget_2 = new QTabWidget(widget_8);
        tabWidget_2->setObjectName(QString::fromUtf8("tabWidget_2"));
        tabWidget_2->setGeometry(QRect(10, 40, 1031, 321));

        tab_slr1 = new QWidget();
        tab_slr1->setObjectName(QString::fromUtf8("tab_slr1"));
        tableWidget_2 = new QTableWidget(tab_slr1);
        tableWidget_2->setObjectName(QString::fromUtf8("tableWidget_2"));
        tableWidget_2->setGeometry(QRect(0, 0, 1021, 291));
        tabWidget_2->addTab(tab_slr1, QString());

        tab_lr1_dfa = new QWidget();
        tab_lr1_dfa->setObjectName(QString::fromUtf8("tab_lr1_dfa"));
        tableWidget_5 = new QTableWidget(tab_lr1_dfa);
        tableWidget_5->setObjectName(QString::fromUtf8("tableWidget_5"));
        tableWidget_5->setGeometry(QRect(0, 0, 1021, 291));
        tabWidget_2->addTab(tab_lr1_dfa, QString());

        tab_lr1_table = new QWidget();
        tab_lr1_table->setObjectName(QString::fromUtf8("tab_lr1_table"));
        tableWidget_6 = new QTableWidget(tab_lr1_table);
        tableWidget_6->setObjectName(QString::fromUtf8("tableWidget_6"));
        tableWidget_6->setGeometry(QRect(0, 0, 1021, 291));
        tabWidget_2->addTab(tab_lr1_table, QString());

        // 右侧语法树生成区域
        widget1 = new QWidget(widget);
        widget1->setObjectName(QString::fromUtf8("widget1"));
        widget1->setGeometry(QRect(1299, 5, 431, 735));
        tabWidget = new QTabWidget(widget1);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setGeometry(QRect(6, 39, 421, 685));

        // 只保留代码生成和语法树展示两个tab
        tab = new QWidget();
        tab->setObjectName(QString::fromUtf8("tab"));
        codeText = new QPlainTextEdit(tab);
        codeText->setObjectName(QString::fromUtf8("codeText"));
        codeText->setGeometry(QRect(0, 0, 411, 655));
        tabWidget->addTab(tab, QString());

        tab_2 = new QWidget();
        tab_2->setObjectName(QString::fromUtf8("tab_2"));
        treeWidget = new QTreeWidget(tab_2);
        treeWidget->setObjectName(QString::fromUtf8("treeWidget"));
        treeWidget->setGeometry(QRect(0, 0, 421, 655));
        treeWidget->setHeaderHidden(true);
        tabWidget->addTab(tab_2, QString());

        label_10 = new QLabel(widget1);
        label_10->setObjectName(QString::fromUtf8("label_10"));
        label_10->setGeometry(QRect(130, 10, 60, 23));
        pushButton_8 = new QPushButton(widget1);
        pushButton_8->setObjectName(QString::fromUtf8("pushButton_8"));
        pushButton_8->setGeometry(QRect(200, 10, 75, 23));
        pushButton_9 = new QPushButton(widget1);
        pushButton_9->setObjectName(QString::fromUtf8("pushButton_9"));
        pushButton_9->setGeometry(QRect(280, 10, 101, 23));

        retranslateUi(Widget);

        tabWidget_2->setCurrentIndex(0);
        tabWidget->setCurrentIndex(0);

        QMetaObject::connectSlotsByName(Widget);
    } // setupUi

    void retranslateUi(QWidget *Widget)
    {
        Widget->setWindowTitle(QCoreApplication::translate("Widget", "LR(1)/SLR(1) \345\210\206\346\236\220\345\231\250", nullptr));
        label_3->setText(QCoreApplication::translate("Widget", "\346\226\207\346\263\225\350\276\223\345\205\245\347\274\226\350\276\221", nullptr));
        pushButton_7->setText(QCoreApplication::translate("Widget", "\346\237\245\347\234\213\350\276\223\345\205\245\350\247\204\345\210\231", nullptr));
        pushButton_3->setText(QCoreApplication::translate("Widget", "\346\211\223\345\274\200", nullptr));
        pushButton_4->setText(QCoreApplication::translate("Widget", "\344\277\235\345\255\230", nullptr));
        label_4->setText(QCoreApplication::translate("Widget", "First\351\233\206\345\220\210\346\261\202\350\247\243", nullptr));
        pushButton_5->setText(QCoreApplication::translate("Widget", "\346\261\202\350\247\243", nullptr));
        label_5->setText(QCoreApplication::translate("Widget", "Follow\351\233\206\345\220\210\346\261\202\350\247\243", nullptr));
        pushButton_6->setText(QCoreApplication::translate("Widget", "\346\261\202\350\247\243", nullptr));
        label_6->setText(QCoreApplication::translate("Widget", "LR(0)DFA\345\233\276", nullptr));
        pushButton->setText(QCoreApplication::translate("Widget", "\345\274\200\345\247\213\347\224\237\346\210\220", nullptr));
        label_7->setText(QCoreApplication::translate("Widget", "SLR(1)\346\226\207\346\263\225\345\210\206\346\236\220", nullptr));
        pushButton_2->setText(QCoreApplication::translate("Widget", "\345\274\200\345\247\213\345\210\206\346\236\220", nullptr));
        label_12->setText(QCoreApplication::translate("Widget", "LR(1)\346\226\207\346\263\225\345\210\206\346\236\220", nullptr));
        pushButton_10->setText(QCoreApplication::translate("Widget", "\347\224\237\346\210\220DFA", nullptr));
        pushButton_11->setText(QCoreApplication::translate("Widget", "\347\224\237\346\210\220\345\210\206\346\236\220\350\241\250", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_slr1), QCoreApplication::translate("Widget", "SLR(1)\345\210\206\346\236\220\350\241\250", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_lr1_dfa), QCoreApplication::translate("Widget", "LR(1) DFA\345\233\276", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_lr1_table), QCoreApplication::translate("Widget", "LR(1)\345\210\206\346\236\220\350\241\250", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("Widget", "\344\273\243\347\240\201\347\224\237\346\210\220", nullptr));
        QTreeWidgetItem *___qtreewidgetitem = treeWidget->headerItem();
        ___qtreewidgetitem->setText(0, QCoreApplication::translate("Widget", "\346\240\221\345\275\242\347\273\223\346\236\204", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab_2), QCoreApplication::translate("Widget", "\350\257\255\346\263\225\346\240\221\345\261\225\347\244\272", nullptr));
        label_10->setText(QCoreApplication::translate("Widget", "\350\257\255\346\263\225\346\240\221\347\224\237\346\210\220", nullptr));
        pushButton_8->setText(QCoreApplication::translate("Widget", "\344\273\243\347\240\201\347\224\237\346\210\220", nullptr));
        pushButton_9->setText(QCoreApplication::translate("Widget", "\345\217\257\350\247\206\345\214\226\350\257\255\346\263\225\346\240\221", nullptr));
    } // retranslateUi

};

namespace Ui {
    class Widget: public Ui_Widget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_WIDGET_H
