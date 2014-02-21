/*
 * Project "Fingerprint GUI": Services for fingerprint authentication on Linux
 * Module: AboutImpl.cpp, AboutImpl.h
 * Purpose: About dialog
 *
 * @author  Wolfgang Ullrich
 * Copyright (C) 2008-2012  Wolfgang Ullrich
 */

/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "../include/AboutImpl.h"

AboutImpl::AboutImpl(QWidget *parent,Qt::WFlags f)
    : QDialog(parent,f){
    setupUi(this);

    string s="Version: ";
    s.append(VERSION);
    versionLabel->setText(QApplication::translate("AboutDialog", s.data(),0,QApplication::UnicodeUTF8));
    copyrightLabel->setText(QApplication::translate("AboutDialog", COPYRIGHT,0,QApplication::UnicodeUTF8));
}
