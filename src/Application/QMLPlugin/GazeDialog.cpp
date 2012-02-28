/**
 *  Copyright (c) 2011 CIE / University of Oulu, All Rights Reserved
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 */

#include "StableHeaders.h"
#include "GazeDialog.h"
#include <QLabel>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>


GazeDialog::GazeDialog(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    setModal(true);
    setWindowTitle(tr("Gaze Properties"));
    setAttribute(Qt::WA_DeleteOnClose);

    center_size_ = 0;
    points_ = 0;
    rect_size_ = 0;
    delta_mode_ = false;
    debug_mode_ = false;
    mouse_ = false;

    QPushButton *buttonOK = new QPushButton(tr("OK"));
    QPushButton *buttonCancel = new QPushButton(tr("Cancel"));

    buttonOK->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonCancel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    buttonOK->setDefault(true);
    buttonCancel->setAutoDefault(false);

    le_amount_of_points_ = new QLineEdit();
    le_amount_of_points_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    le_rect_= new QLineEdit();
    le_rect_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    le_center_= new QLineEdit();
    le_center_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    cb_delta_ = new QCheckBox(this);
    cb_delta_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    cb_debug_ = new QCheckBox(this);
    cb_debug_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    cb_mouse_ = new QCheckBox(this);
    cb_mouse_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    QLabel *lPoints = new QLabel(tr("Average points:"));
    QLabel *lRect = new QLabel(tr("Frustum rectangle size:"));
    QLabel *lCenter = new QLabel(tr("Center dead zone size:"));
    QLabel *lDelta = new QLabel(tr("Delta mode:"));
    QLabel *lDebug = new QLabel(tr("Debug mode:"));
    QLabel *lMouse = new QLabel(tr("Use mouse as gaze:"));


    QGridLayout *grid = new QGridLayout();
    grid->setVerticalSpacing(8);

    grid->addWidget(lPoints, 0, 0);
    grid->addWidget(le_amount_of_points_, 0, 1, Qt::AlignLeft, 1);

    grid->addWidget(lRect, 1, 0);
    grid->addWidget(le_rect_, 1, 1, Qt::AlignLeft, 1);

    grid->addWidget(lCenter, 2, 0);
    grid->addWidget(le_center_, 2, 1, Qt::AlignLeft, 1);

    grid->addWidget(lDelta, 3, 0);
    grid->addWidget(cb_delta_, 3, 1, Qt::AlignLeft, 1);

    grid->addWidget(lDebug, 4, 0);
    grid->addWidget(cb_debug_, 4, 1, Qt::AlignLeft, 1);

    grid->addWidget(lMouse, 5, 0);
    grid->addWidget(cb_mouse_, 5, 1, Qt::AlignLeft, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(buttonOK);
    buttonLayout->addWidget(buttonCancel);

    QVBoxLayout *vertLayout = new QVBoxLayout();
    vertLayout->addLayout(grid);
    vertLayout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    vertLayout->addLayout(buttonLayout);

    setLayout(vertLayout);

    connect(buttonOK, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

GazeDialog::~GazeDialog()
{

}

void GazeDialog::SetValues(float center_size, int points, int rect_size, bool delta_mode, bool debug_mode, bool mouse)
{
    center_size_ = center_size;
    points_ = points;
    rect_size_ = rect_size;
    delta_mode_ = delta_mode;
    debug_mode_ = debug_mode;
    mouse_ = mouse;

    le_amount_of_points_->setText(QString::number(points_));
    le_center_->setText(QString::number(center_size_));
    le_rect_->setText(QString::number(rect_size_));
    cb_delta_->setChecked(delta_mode_);
    cb_debug_->setChecked(debug_mode_);
    cb_mouse_->setChecked(mouse_);
}

void GazeDialog::accept()
{
    center_size_ = le_center_->text().toFloat();
    points_ = le_amount_of_points_->text().toInt();
    rect_size_ = le_rect_->text().toInt();
    delta_mode_ = cb_delta_->isChecked();
    debug_mode_ = cb_debug_->isChecked();
    mouse_ = cb_mouse_->isChecked();
    emit WindowAccepted(center_size_, points_, rect_size_, delta_mode_, debug_mode_, mouse_);
    this->close();
}
