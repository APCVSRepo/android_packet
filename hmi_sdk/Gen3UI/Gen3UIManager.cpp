﻿#include "Gen3UIManager.h"
//#include "AppData/AppList.h"
//#include <pthread.h>
#include <QThread>
#ifdef ANDROID
#include "android/log.h"
#include <unistd.h>
#endif

#if defined(WINCE)
//#include<sys/stat.h>
#include "Config/Config.h"
#else
#include<sys/stat.h>
#endif
#include <QDir>
#ifdef SDL_SUPPORT_LIB
#include "main.h"
//#include "AudioTrans/MspVRAudio.h"
#endif

#include "VideoStream/VideoStream.h"
#include "MainWindow/MainWindow.h"
#include "AppListView/AppListView.h"
//#include "Show/MainWidget.h"
#include "CommandView/CommandView.h"
#include "Show/MediaShow.h"
#include "Alert/AlertView.h"
#include "ChoiceSet/ChoiceSet.h"
#include <QApplication>
#include <QDesktopWidget>
#include "ScrollableMessage/ScollMsgView.h"
#include "SliderView/SliderView.h"

CGen3UIManager::CGen3UIManager(AppListInterface * pList, QWidget *parent) :
    QWidget(parent)
{
    for(int i = 0; i < ID_UI_MAX; i++)
    {
        m_vUIWidgets[i] = NULL;
    }
    m_pList = pList;
}

CGen3UIManager::~CGen3UIManager()
{
    for(int i = 0; i < ID_UI_MAX; i++)
    {
        if(m_vUIWidgets[i])
        {
            delete m_vUIWidgets[i];
            m_vUIWidgets[i] = NULL;
        }
    }


#ifdef SDL_SUPPORT_LIB
    //delete m_MspVR;
#endif
}

void CGen3UIManager::initAppHMI()
{
#ifdef ANDROID
    UIConfig::loadResolution(QApplication::desktop()->width(),QApplication::desktop()->height()-30);
#else
    UIConfig::loadResolution(800,480);
#endif

    MainWindow * pMain = new MainWindow(m_pList);
    QWidget* pParent = pMain->CenterWidget();
    m_vUIWidgets[ID_MAIN] = pMain;
    m_vUIWidgets[ID_APPLINK] = new CAppListView(m_pList, pParent);
    m_vUIWidgets[ID_ALERT]=new AlertView(m_pList, pParent);
    //m_vUIWidgets[ID_AUDIOPASSTHRU]=new CAudioPassThru(m_pList, pParent);
    //m_vUIWidgets[ID_CHOICESETVR]=new CChoicesetVR(m_pList, pParent);
    m_vUIWidgets[ID_CHOICESET] = new CChoiceSet(m_pList, pParent);
    m_vUIWidgets[ID_COMMAND]=new CCommandView(m_pList, pParent);
    m_vUIWidgets[ID_SCROLLMSG] = new CScollMsgView(m_pList, pParent);
    m_vUIWidgets[ID_SHOW] = new CMediaShow(m_pList,pParent);
    //m_vUIWidgets[ID_NOTIFY]=new Notify(pParent);
    m_vUIWidgets[ID_SLIDER] = new CSliderView(m_pList, pParent);
    m_vUIWidgets[ID_MEDIACLOCK] = NULL;
    m_vUIWidgets[ID_VIDEOSTREAM] = new VideoStream(m_pList,pMain);


    for(int i = 0; i < ID_UI_MAX; i++)
    {
        if(m_vUIWidgets[i] != NULL)
        {
            m_vUIWidgets[i]->hide();
        }
    }



    m_iCurUI = ID_MAIN;

#ifdef SDL_SUPPORT_LIB
    //m_MspVR = new msp_vr_audio;
#endif
//
    connect(this,SIGNAL(onAppShowSignal(int)),this,SLOT(AppShowSlot(int)));
    connect(this,SIGNAL(onVideoStartSignal()),this,SLOT(onVideoStartSlots()));
    connect(this,SIGNAL(onVideoStopSignal()),this,SLOT(onVideoStopSlots()));

    //emit finishMainHMI();
}

void CGen3UIManager::onAppActive()
{
 //   QString qs = AppControl->getAppName().c_str();
    //((MainMenu *)m_vUIWidgets[ID_MAIN])->SetTitle(qs);
    //((MainWidget *)m_vUIWidgets[ID_MAIN])->SetAppName(qs);
}

void CGen3UIManager::onAppStop()
{

}

//show app
void CGen3UIManager::onAppShow(int type)
{
    if((type >= 0) && (type < ID_UI_MAX))
        emit onAppShowSignal(type);
}

void CGen3UIManager::onVideoStreamStart()
{
    fflush(stdout);
    emit onVideoStartSignal();
}

void CGen3UIManager::onVideoStartSlots()
{
    fflush(stdout);
    std::string str_url = AppControl->getUrlString();
    //_D("%s\n",str_url.data());
    VideoStream *pVideoStream = ((VideoStream *)m_vUIWidgets[ID_VIDEOSTREAM]);//->StartVideoStream(str_url.c_str());
    pVideoStream->setUrl(str_url.c_str());
    pVideoStream->startStream();
}

void CGen3UIManager::onVideoStreamStop()
{
    emit onVideoStopSignal();
}

void CGen3UIManager::onVideoStopSlots()
{
    ((VideoStream *)m_vUIWidgets[ID_VIDEOSTREAM])->stopStream();
}

void CGen3UIManager::AppShowSlot(int type)
{    
    if(m_vUIWidgets[m_iCurUI] == NULL)
    {
        return;
    }

    if(ID_MEDIACLOCK == type)
    {
        if(ID_SHOW == m_iCurUI)
        {
            CMediaShow *pShow = (CMediaShow *)m_vUIWidgets[ID_SHOW];
            pShow->UpdateMediaColckTimer();
        }
    }
    else
    {
        if(m_iCurUI != ID_MAIN)
        {
            m_vUIWidgets[m_iCurUI]->hide();
        }
        //m_vUIWidgets[m_iCurUI]->hide();
        m_iCurUI = type;
        m_vUIWidgets[m_iCurUI]->show();
    }
}

#include <QCoreApplication>
void CGen3UIManager::waitMSec(int ms)
{
    /*
    QTime t;
    t.start();
    while(t.elapsed()<ms)
        QCoreApplication::processEvents();
        */
}

void CGen3UIManager::tsSpeak(int VRID, std::string strText)
{
    /*
    bool ret = ts.speak(strText.c_str());


    switch(VRID)
    {
    case ID_DEFAULT:
        if(ret)
            AppControl->OnTTSSpeek(0);
        else
            AppControl->OnTTSSpeek(5);
        break;
    case ID_CANCEL:
        AppControl->OnPerformAudioPassThru(PERFORMAUDIOPASSTHRU_CANCEL);
        break;
    case ID_HELP:
        AppControl->OnPerformAudioPassThru(PERFORMAUDIOPASSTHRU_DONE);
        break;
    case ID_EXIT:
        AppControl->OnPerformAudioPassThru(PERFORMAUDIOPASSTHRU_DONE);
        m_pList->OnAppExit();
        break;
    case ID_SWITCHAPP:
        AppControl->OnPerformAudioPassThru(PERFORMAUDIOPASSTHRU_DONE);
        break;
    default:
        AppControl->OnPerformAudioPassThru(PERFORMAUDIOPASSTHRU_CANCEL);
        break;
    }
    */
}

void CGen3UIManager::OnEndAudioPassThru()
{
    //((CAudioPassThru *)m_vUIWidgets[ID_AUDIOPASSTHRU])->onButtonClickedSlots(PERFORMAUDIOPASSTHRU_CANCEL);
}
