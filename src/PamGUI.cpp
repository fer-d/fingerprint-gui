/*
 * Project "Fingerprint GUI": Services for fingerprint authentication on Linux
 * Module: PamGUI.cpp, PamGUI.h
 * Purpose: Main object for pam_fingerprint module for running in gui environments
 *
 * @author  Wolfgang Ullrich
 * Copyright (C) 2008-2013  Wolfgang Ullrich
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

#include <unistd.h>
#include "PamGUI.h"
#include "FingerprintHelper.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#include <X11/keysym.h>

PamGUI::PamGUI(FingerprintDevice *dev,const char *user,const char *finger)
    : QFrame(){
    setupUi(this);

    identifyData=NULL;
    startupGUI(dev);
    if(finger!=NULL){
        QString s("Swipe your ");
        s.append(finger);
        s.append(".");
        label->setText(s);
        showMessage(MSG_LABEL,s);
    }
    if(user!=NULL){
        QString s="Authenticating ";
        s.append(user);
        showMessage(MSG_NORMAL,s);
    }
}

PamGUI::PamGUI(FingerprintDevice *dev,FingerprintData *iData)
    : QFrame(){
    setupUi(this);

    showMessage(MSG_NORMAL,"Ready...");
    identifyData=iData;
    startupGUI(dev);
}

PamGUI::~PamGUI(){
    device->stop();
}

void PamGUI::startupGUI(FingerprintDevice *dev){
    UNDECORATED_MODAL_WINDOW_CENTERED
    show();
    repeatDelay=0;
    device=dev;
    device->setTimeout(false);
    connect(device,SIGNAL(verifyResult(int)),this,SLOT(newVerifyResult(int)));
    connect(device,SIGNAL(matchResult(int)),this,SLOT(matchResult(int)));
    device->start();
    releaseKeyboard();
    START_TIMER
    ANIMATION_MOVIE
}

void PamGUI::showMessage(const char *target,const QString msg){
    QFont font;
    if(target){
        if(strcmp(target,MSG_BOLD)==0){
            font.setPointSize(10);
            font.setBold(true);
            statusBar->setFont(font);
        }
        if(strcmp(target,MSG_NORMAL)==0){
            font.setPointSize(9);
            font.setBold(false);
            statusBar->setFont(font);
        }
    }
    statusBar->setText(msg);    // Show message on statusBar
    syslog(LOG_DEBUG,"Message: %s",msg.toStdString().data()); // Message to syslog
    
    // send message to plugin
    string fifoMsg(""); // Compose a message for fingerprint-plugin
    fifoMsg.append(target);
    fifoMsg.append(msg.toStdString());
    fifoMsg.append("\0");
    pluginMessage(fifoMsg.data());
}

// Helper function, required since Ubuntu 12.04 because unity-greeter releases the keyboard focus
// We set the focus to the greeter window again so the user can invoke his password
static bool focusIsSet=false;

void PamGUI::setFocusToUnityGreeter(){
    unsigned char *name;
    Window    wRoot;
    Window    wParent;
    Window   *wChild;
    unsigned  nChildren;
    Atom           type;
    int            format;
    unsigned long  nItems;
    unsigned long  bytesAfter;

    if(focusIsSet)return;	//Do it only once
    focusIsSet=true;
    Display *display=XOpenDisplay(getenv("DISPLAY"));
    if(display!=NULL){
        Window window=DefaultRootWindow(display);
    	Atom _atomPID=XInternAtom(display,"_NET_WM_PID",True);
    	if(_atomPID!=None){
    	    if(0!=XQueryTree(display,window,&wRoot,&wParent,&wChild,&nChildren)){
    		for(unsigned i=0;i<nChildren;i++){
		    if(Success==XGetWindowProperty(display,wChild[i],XInternAtom(display,"WM_NAME",False),0,1024,False,XA_STRING,&type,&format,&nItems,&bytesAfter,&name)){
			if(name!=NULL){
//			    syslog(LOG_DEBUG,"WINDOW: \"%ld\", NAME: %s.",wChild[i],name);
			    if(strcmp((const char *)name,"unity-greeter")==0){
				syslog(LOG_DEBUG,"Setting focus to window: %ld \"%s\", result: %d.",wChild[i],name,XSetInputFocus(display,wChild[i],RevertToNone,CurrentTime));
			    }
			    XFree(name);
			}
		    }
		}
    	    }
	}
    }
    show();
}



// slots -----------------------------------------------------------------------
void PamGUI::matchResult(int match){
    device->stop();
    if(match>=0){
        movie->setPaused(true);
        string message;
        if(identifyData!=NULL){
            FingerprintData *fingerprintData=identifyData;
            for(int i=0;i<match;i++)fingerprintData=fingerprintData->next;
            message.append("Identified: ");
            message.append(fingerprintData->getUserName()->data());
            message.append(" (");
            message.append(fingerprintData->getFingerName());
            message.append(")");
        }
        else{
            message.append("OK");
        }
        showMessage(MSG_BOLD,message.data());
	timer->stop();
        //exit with index (match) as exit code
        qApp->processEvents();
        device->wait(5000);
//        for(int i=SHOW_DELAY/1000;i>0;i--){ //let 'em see the result before exiting
        qApp->processEvents();
        usleep(SHOW_DELAY);
//            usleep(1000);
//        }
        qApp->exit(match);
        return;
    }
    showMessage(MSG_BOLD,"Not identified!");
    repeatDelay=3;   //let 'em see the result before restarting
}

void PamGUI::newVerifyResult(int result){
    switch(result){
        case RESULT_VERIFY_NO_MATCH:
            showMessage(MSG_NORMAL,"No match!");
            break;
        case RESULT_VERIFY_RETRY_TOO_SHORT:
            showMessage(MSG_NORMAL,"Swipe too short...");
            break;
        case RESULT_VERIFY_RETRY_CENTER:
            showMessage(MSG_NORMAL,"Please center...");
            break;
        case RESULT_VERIFY_RETRY:
        case RESULT_VERIFY_RETRY_REMOVE:
            showMessage(MSG_NORMAL,"Try again...");
            break;
        default:
            return;
    }
}

// Helper thread for restart
void PamGUI::timerTick(){
    setFocusToUnityGreeter();
    // send alive message to plugin
    pluginMessage(MSG_ALIVE);
    raise();
    switch(repeatDelay){
        case 0:
            if(!device->isRunning()){
                syslog(LOG_ERR,"ERROR: Fingerprint device not running.");
                qApp->exit(-1);
                return;
            }
            //do nothing
            break;
        case 1:
            syslog(LOG_INFO,"Waiting for device to stop...");
            device->wait(5000);
            syslog(LOG_INFO,"Stopped, restarting");
            //restart fingerprint scanner
            showMessage(MSG_NORMAL,"Ready...");
            device->start();
            repeatDelay--;
            break;
        default:
            repeatDelay--;  //still wait
    }
}
