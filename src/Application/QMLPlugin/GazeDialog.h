/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */

#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>

class GazeDialog : public QDialog
{
    Q_OBJECT

public:
    GazeDialog(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~GazeDialog();

    void SetValues(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode, bool mouse);

private:
    QLineEdit *le_amount_of_points_;
    QLineEdit *le_center_;
    QLineEdit *le_rect_;
    QCheckBox *cb_delta_;
    QCheckBox *cb_debug_;
    QCheckBox *cb_mouse_;

    float center_size_;
    int points_;
    int rect_size_;
    bool delta_mode_;
    bool debug_mode_;
    bool mouse_;

private slots:
    void accept();

signals:
    void WindowAccepted(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode, bool mouse);
    void closed();
protected:
    void showEvent(QShowEvent *event);
    void closeEvent(QCloseEvent *event);
};


